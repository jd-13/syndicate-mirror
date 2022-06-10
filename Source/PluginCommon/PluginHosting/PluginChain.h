/*
  ==============================================================================

    PluginChain.h
    Created: 28 May 2021 9:17:35pm
    Author:  Jack Devlin

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ChainSlotPlugin.h"
#include "ChainSlotGainStage.h"
#include "LatencyListener.h"
#include "General/AudioSpinMutex.h"

class PluginChain : public juce::AudioProcessor, public LatencyListener {
public:
    PluginChain(std::function<float(int, MODULATION_TYPE)> getModulationValueCallback);
    ~PluginChain() = default;

    /**
     * Inserts a plugin at the given position, or at the end if that position doesn't exist.
     */
    void insertPlugin(std::shared_ptr<juce::AudioPluginInstance> plugin, int position);

    /**
     * Replaces an existing plugin in the chain.
     */
    void replacePlugin(std::shared_ptr<juce::AudioPluginInstance> plugin, int position);

    /**
     * Removes the plugin or gain stage at the given position in the chain.
     */
    bool removeSlot(int position);

    /**
     * Inserts a gain stage at the given position, or at the end if that position doesn't exist.
     */
    void insertGainStage(int position, const juce::AudioProcessor::BusesLayout& busesLayout);

    /**
     * Returns a pointer to the plugin at the given position.
     */
    std::shared_ptr<juce::AudioPluginInstance> getPlugin(int position) const;

    /**
     * Set the modulation config for the given plugin to the one provided.
     */
    bool setPluginModulationConfig(PluginModulationConfig config, int position);

    /**
     * Returns the modulation config for the given plugin.
     */
    PluginModulationConfig getPluginModulationConfig(int position) const;

    /**
     * Returns the number of plugins and gain stages in this chain.
     */
    size_t getNumSlots() const { return _chain.size(); }

    /**
     * Bypasses or enables the slot at the given position.
     */
    void setSlotBypass(int position, bool isBypassed);

    /**
     * Returns true if the slot is bypassed.
     */
    bool getSlotBypass(int position);

    /**
     * Bypasses the entire chain if set to true.
     */
    void setChainBypass(bool val);

    /**
     * Mutes the entire chain if set to true.
     */
    void setChainMute(bool val);

    /**
     * Sets the gain for the gain stage at the given position.
     */
    bool setGainLinear(int position, float gain);

    /**
     * Returns the gain for the gain stage at the given position.
     */
    float getGainLinear(int position);

    /**
     * If this position refers to a gain stage returns a levels provider for it, otherwise an empty
     * optional.
     */
    std::optional<GainStageLevelsProvider> getGainStageLevelsProvider(int position);

    /**
     * Sets the pan/balance for the gain stage at the given position.
     */
    bool setPan(int position, float pan);

    /**
     * Returns the pan/balance for the gain stage at the given position.
     */
    float getPan(int position);

    /**
     * @see setChainBypass
     */
    bool getChainBypass() { return _isChainBypassed; }

    /**
     * @see setChainMute
     */
    bool getChainMute() { return _isChainMuted; }

    /**
     * Sets the total amount of latency this chain should aim for to keep it inline with other
     * chains.
     *
     * The chain can't reduce its latency below the total of the plugins it hosts, but it can
     * increase its latency to match slower chains.
     */
    void setRequiredLatency(int numSamples);

    /**
     * Returns a pointer to the bounds for this plugin's editor. Will pointer to an empty optional
     * if there isn't a plugin at the given position.
     */
    std::shared_ptr<PluginEditorBounds> getPluginEditorBounds(int position) const;

    void restoreFromXml(juce::XmlElement* element,
                        HostConfiguration configuration,
                        const PluginConfigurator& pluginConfigurator,
                        std::function<void(juce::String)> onErrorCallback);
    void writeToXml(juce::XmlElement* element);

    // AudioProcessor methods
    virtual const juce::String getName() const override;
    virtual void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    virtual void releaseResources() override;
    virtual void reset() override;
    virtual void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
    virtual double getTailLengthSeconds() const override;
    virtual bool acceptsMidi() const override;
    virtual bool producesMidi() const override;
    virtual juce::AudioProcessorEditor* createEditor() override;
    virtual bool hasEditor() const override;
    virtual int getNumPrograms() override;
    virtual int getCurrentProgram() override;
    virtual void setCurrentProgram(int index) override;
    virtual const juce::String getProgramName(int index) override;
    virtual void changeProgramName(int index, const juce::String& newName) override;
    virtual void getStateInformation(juce::MemoryBlock& destData) override;
    virtual void setStateInformation(const void* data, int sizeInBytes) override;

private:
    // Use vector rather than linked list
    // (contiguous memory should be faster to iterate through in the processing loop)
    // (insertion may be slower this way but that'll be on a less important thread)
    std::vector<std::unique_ptr<ChainSlotBase>> _chain;

    bool _isChainBypassed;
    bool _isChainMuted;

    std::function<float(int, MODULATION_TYPE)> _getModulationValueCallback;

    std::unique_ptr<juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::None>> _latencyCompLine;
    WECore::AudioSpinMutex _latencyCompLineMutex;

    void _onLatencyChange() override;
};

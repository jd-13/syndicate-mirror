/*
  ==============================================================================

    PluginSplitter.h
    Created: 29 May 2021 2:35:45pm
    Author:  Jack Devlin

  ==============================================================================
*/

#pragma once

#include <vector>
#include <JuceHeader.h>

#include "PluginChain.h"
#include "LatencyListener.h"
#include "SplitTypes.h"

/**
 * Stores a plugin chain and any associated data.
 */
struct PluginChainWrapper {
    PluginChainWrapper(std::unique_ptr<PluginChain> newChain, bool newIsSoloed)
            : chain(std::move(newChain)), isSoloed(newIsSoloed) {}

    std::unique_ptr<PluginChain> chain;
    bool isSoloed;
};

/**
 * Base class which provides the audio splitting functionality.
 *
 * Each derived class contains one or more plugin chains (one for each split).
 *
 * A splitter may contain more chains than it can actually use if they have been carried over from
 * a previous splitter that could handle more. In this case its processBlock will just ignore the
 * extra chains.
 *
 * TODO: Ideally the state of the plugins in the unused chains would be stored and the plugin
 * instances removed until they are eventually needed, but for now we just keep them.
 */
class PluginSplitter : public juce::AudioProcessor, public LatencyListener {
public:
    PluginSplitter(int defaultNumChains,
                   std::function<float(int, MODULATION_TYPE)> _getModulationValueCallback);
    PluginSplitter(std::vector<PluginChainWrapper>& chains,
                   int defaultNumChains,
                   std::function<float(int, MODULATION_TYPE)> _getModulationValueCallback);

    ~PluginSplitter() = default;

    bool insertPlugin(std::shared_ptr<juce::AudioPluginInstance> plugin, int chainNumber, int positionInChain);
    bool replacePlugin(std::shared_ptr<juce::AudioPluginInstance> plugin, int chainNumber, int positionInChain);
    bool removeSlot(int chainNumber, int positionInChain);
    bool insertGainStage(int chainNumber, int positionInChain, const juce::AudioProcessor::BusesLayout& busesLayout);

    std::shared_ptr<juce::AudioPluginInstance> getPlugin(int chainNumber, int positionInChain);

    bool setPluginModulationConfig(PluginModulationConfig config, int chainNumber, int positionInChain);
    PluginModulationConfig getPluginModulationConfig(int chainNumber, int positionInChain) const;

    std::unique_ptr<PluginChain>& getChain(int chainNumber) { return _chains[chainNumber].chain; }
    std::vector<PluginChainWrapper>& getChains() { return _chains; }
    std::vector<PluginChainWrapper>& releaseChains();
    size_t getNumChains() { return _chains.size(); }

    void setSlotBypass(int chainNumber, int positionInChain, bool isBypassed);
    bool getSlotBypass(int chainNumber, int positionInChain);

    virtual void setChainSolo(int chainNumber, bool val);
    virtual bool getChainSolo(int chainNumber);

    bool setGainLinear(int chainNumber, int positionInChain, float gain);
    float getGainLinear(int chainNumber, int positionInChain);
    std::optional<GainStageLevelsProvider> getGainStageLevelsProvider(int chainNumber, int positionInChain);

    bool setPan(int chainNumber, int positionInChain, float pan);
    float getPan(int chainNumber, int positionInChain);

    virtual SPLIT_TYPE getSplitType() = 0;

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

protected:
    std::vector<PluginChainWrapper> _chains;
    size_t _numChainsSoloed;
    std::function<float(int, MODULATION_TYPE)> _getModulationValueCallback;

    /**
     * Called when restoring from XML and a chain needs to be added
     * Inheriting classes can override it if they need to setup other things for each chain
     */
    virtual void _onChainRestored() { _chains.emplace_back(std::make_unique<PluginChain>(_getModulationValueCallback), false); }

    // Helper methods for subclasses that need to use multiple buffers
    void _copyBuffer(juce::AudioBuffer<float>& source, juce::AudioBuffer<float>& destination);
    void _addBuffers(juce::AudioBuffer<float>& source, juce::AudioBuffer<float>& destination);

    void _onLatencyChange() override;
};

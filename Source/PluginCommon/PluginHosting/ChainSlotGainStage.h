#pragma once

#include <JuceHeader.h>

#include "ChainSlotBase.h"
#include "WEFilters/AREnvelopeFollowerSquareLaw.h"

/**
 * Represents a gain stage in a slot in a processing chain.
 */
class ChainSlotGainStage : public ChainSlotBase {
public:
    // Linear 0 to 1 (or a little more) values
    float gain;

    // -1 to 1 values
    float pan;

    ChainSlotGainStage(float newGain, float newPan, bool newIsBypassed, const juce::AudioProcessor::BusesLayout& busesLayout);
    virtual ~ChainSlotGainStage() = default;

    float getOutputAmplitude(int channel) const { return static_cast<float>(_meterEnvelopes[channel].getLastOutput()); }

    int getNumChannels() const { return _numMainChannels; }

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void reset() override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    static std::unique_ptr<ChainSlotGainStage> restoreFromXml(juce::XmlElement* element, const juce::AudioProcessor::BusesLayout& busesLayout);
    void writeToXml(juce::XmlElement* element) override;

private:
    int _numMainChannels;
    std::array<WECore::AREnv::AREnvelopeFollowerSquareLaw, 2> _meterEnvelopes;
};

/**
 * Provides an interface for the UI to access values needed to draw the level meter for this gain stage.
 */
class GainStageLevelsProvider {
public:
    explicit GainStageLevelsProvider(const ChainSlotGainStage& gainStage) : _gainStage(gainStage) {}
    ~GainStageLevelsProvider() = default;

    int getNumChannels() const { return _gainStage.getNumChannels(); }
    float getOutputAmplitude(int channel) const { return _gainStage.getOutputAmplitude(channel); }

private:
    const ChainSlotGainStage& _gainStage;
};

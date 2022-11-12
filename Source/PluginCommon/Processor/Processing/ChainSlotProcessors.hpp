#pragma once

#include <JuceHeader.h>
#include "ChainSlots.hpp"

namespace ChainProcessor {
    void prepareToPlay(ChainSlotGainStage& gainStage, double sampleRate);
    void releaseResources(ChainSlotGainStage& gainStage);
    void reset(ChainSlotGainStage& gainStage);
    void processBlock(ChainSlotGainStage& gainStage, juce::AudioBuffer<float>& buffer);

    void prepareToPlay(ChainSlotPlugin& slot, double sampleRate, int samplesPerBlock);
    void releaseResources(ChainSlotPlugin& slot);
    void reset(ChainSlotPlugin& slot);
    void processBlock(ChainSlotPlugin& slot, juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);
}

#pragma once

#include "CrossoverState.hpp"

namespace CrossoverProcessors {
    void prepareToPlay(CrossoverState& state, double sampleRate, int samplesPerBlock, juce::AudioProcessor::BusesLayout layout);
    void releaseResources(CrossoverState& state);
    void reset(CrossoverState& state);
    void processBlock(CrossoverState& state, juce::AudioBuffer<float>& buffer);
}

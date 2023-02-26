#pragma once

#include "DataModelInterface.hpp"

namespace SplitterInterface {
    void prepareToPlay(Splitter& splitter, double sampleRate, int samplesPerBlock, juce::AudioProcessor::BusesLayout layout);
    void releaseResources(Splitter& splitter);
    void reset(Splitter& splitter);
    void processBlock(Splitter& splitter, juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);
}
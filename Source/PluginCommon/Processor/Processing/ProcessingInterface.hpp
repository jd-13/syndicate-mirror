#pragma once

#include "DataModelInterface.hpp"

namespace SplitterInterface {
    void prepareToPlay(Splitter& splitter, double sampleRate, int samplesPerBlock, juce::AudioProcessor::BusesLayout layout);
    void releaseResources(Splitter& splitter);
    void reset(Splitter& splitter);
    void processBlock(Splitter& splitter, juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages, juce::AudioPlayHead* newPlayHead);

}

namespace ModulationInterface {
    void prepareToPlay(ModulationSourcesState& modulationSources, double sampleRate, int samplesPerBlock, juce::AudioProcessor::BusesLayout layout);
    void reset(ModulationSourcesState& modulationSources);
    void processBlock(ModulationSourcesState& modulationSources, juce::AudioBuffer<float>& buffer, juce::AudioPlayHead::CurrentPositionInfo tempoInfo);

    double getLfoModulationValue(ModulationSourcesState& modulationSources, int lfoNumber);
    double getEnvelopeModulationValue(ModulationSourcesState& modulationSources, int envelopeNumber);
}
#pragma once

#include <JuceHeader.h>
#include "DataModelInterface.hpp"

namespace ModulationProcessors {
    void prepareToPlay(ModulationInterface::ModulationSourcesState& state, double sampleRate, int samplesPerBlock, juce::AudioProcessor::BusesLayout layout);
    void releaseResources(ModulationInterface::ModulationSourcesState& state);
    void reset(ModulationInterface::ModulationSourcesState& state);
    void processBlock(ModulationInterface::ModulationSourcesState& state, juce::AudioBuffer<float>& buffer, juce::AudioPlayHead::CurrentPositionInfo tempoInfo);

    double getLfoModulationValue(ModulationInterface::ModulationSourcesState& state, int lfoNumber);
    double getEnvelopeModulationValue(ModulationInterface::ModulationSourcesState& state, int envelopeNumber);
}

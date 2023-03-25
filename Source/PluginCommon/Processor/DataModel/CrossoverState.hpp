#pragma once

#include <JuceHeader.h>
#include "PluginChain.hpp"

struct BandState {
    bool isSoloed;
    std::shared_ptr<PluginChain> chain;

    BandState() : isSoloed(false) {
    }
};

class CrossoverState {
public:
    // Num low/highpass filters = num bands - 1 (= num crossovers)
    std::vector<juce::dsp::LinkwitzRileyFilter<float>> lowpassFilters;
    std::vector<juce::dsp::LinkwitzRileyFilter<float>> highpassFilters;

    // Num allpass filters = num bands - 2
    std::vector<juce::dsp::LinkwitzRileyFilter<float>> allpassFilters;

    // Num buffers = num bands - 1 (= num crossovers)
    std::vector<juce::AudioBuffer<float>> buffers;

    std::vector<BandState> bands;

    HostConfiguration config;

    // We only need to implement solo at this level - chains handle bypass and mute themselves
    int numBandsSoloed;

    CrossoverState() : numBandsSoloed(0) {}
};

inline std::shared_ptr<CrossoverState> createDefaultCrossoverState() {
    auto state = std::make_shared<CrossoverState>();

    // Initialise configuration for two bands
    constexpr int DEFAULT_FREQ {1000};

    state->lowpassFilters.emplace_back();
    state->lowpassFilters[0].setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    state->lowpassFilters[0].setCutoffFrequency(DEFAULT_FREQ);

    state->highpassFilters.emplace_back();
    state->highpassFilters[0].setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    state->highpassFilters[0].setCutoffFrequency(DEFAULT_FREQ);

    state->buffers.emplace_back();

    state->bands.emplace_back();
    state->bands.emplace_back();

    return state;
}

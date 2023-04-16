#include "CrossoverMutators.hpp"
#include "CrossoverProcessors.hpp"

namespace {
    constexpr int MAX_FREQ {20000};
}

namespace CrossoverMutators {
    void setIsSoloed(std::shared_ptr<CrossoverState> state, size_t bandNumber, bool isSoloed) {
        if (state->bands.size() > bandNumber) {
            if (state->bands[bandNumber].isSoloed != isSoloed) {
                state->bands[bandNumber].isSoloed = isSoloed;

                if (isSoloed) {
                    state->numBandsSoloed++;
                } else {
                    state->numBandsSoloed--;
                }
            }
        }
    }

    void setCrossoverFrequency(std::shared_ptr<CrossoverState> state, size_t crossoverNumber, double val) {
        if (state->lowpassFilters.size() > crossoverNumber) {
            state->lowpassFilters[crossoverNumber].setCutoffFrequency(val);
            state->highpassFilters[crossoverNumber].setCutoffFrequency(val);

            // We might also have an allpass filter to set
            if (state->allpassFilters.size() > crossoverNumber - 1) {
                state->allpassFilters[crossoverNumber - 1].setCutoffFrequency(val);
            }


            // Make sure the crossover frequencies are still in the correct order
            for (size_t otherCrossoverIndex {0}; otherCrossoverIndex < state->lowpassFilters.size(); otherCrossoverIndex++) {
                const double otherCrossoverFrequency {getCrossoverFrequency(state, otherCrossoverIndex)};

                const bool needsCrossoverUpdate {
                    // We've moved the crossover frequency of index below another one that should be
                    // below it - move the other one to the new value
                    (val < otherCrossoverFrequency && otherCrossoverIndex < crossoverNumber) ||

                    // We've moved the crossover frequency of index above another one that should be
                    // above it - move the other one to the new value
                    (otherCrossoverFrequency > val && crossoverNumber > otherCrossoverIndex)
                };

                if (needsCrossoverUpdate) {
                    // We've moved the crossover frequency of index below another one that should be
                    // below it - move the other one to the new value
                    setCrossoverFrequency(state, otherCrossoverIndex, val);
                }
            }
        }
    }

    void setPluginChain(std::shared_ptr<CrossoverState> state, size_t bandNumber, std::shared_ptr<PluginChain> chain) {
        if (state->bands.size() > bandNumber) {
            state->bands[bandNumber].chain = chain;
        }
    }

    bool getIsSoloed(std::shared_ptr<CrossoverState> state, size_t bandNumber) {
        if (state->bands.size() > bandNumber) {
            return state->bands[bandNumber].isSoloed;
        }

        return false;
    }

    double getCrossoverFrequency(std::shared_ptr<CrossoverState> state, size_t crossoverNumber) {
        if (state->lowpassFilters.size() > crossoverNumber) {
            return state->lowpassFilters[crossoverNumber].getCutoffFrequency();
        }

        return 0;
    }

    size_t getNumBands(std::shared_ptr<CrossoverState> state) {
        return state->bands.size();
    }

    void addBand(std::shared_ptr<CrossoverState> state) {
        const double oldHighestCrossover {CrossoverMutators::getCrossoverFrequency(state, state->lowpassFilters.size() - 1)};

        // Add all the new state
        state->lowpassFilters.emplace_back();
        state->lowpassFilters[state->lowpassFilters.size() - 1].setType(juce::dsp::LinkwitzRileyFilterType::lowpass);

        state->highpassFilters.emplace_back();
        state->highpassFilters[state->highpassFilters.size() - 1].setType(juce::dsp::LinkwitzRileyFilterType::highpass);

        state->allpassFilters.emplace_back();
        state->allpassFilters[state->allpassFilters.size() - 1].setType(juce::dsp::LinkwitzRileyFilterType::allpass);

        state->buffers.emplace_back();

        state->bands.emplace_back();

        // Set the crossover frequency of the new band and make room for it if needed
        if (oldHighestCrossover < MAX_FREQ) {
            // The old highest crossover frequency is below the maximum, insert the new one halfway
            // between it and the maximum
            const double topBandWidth {MAX_FREQ - oldHighestCrossover};
            const double newCrossoverFreq {oldHighestCrossover + (topBandWidth / 2)};
            setCrossoverFrequency(state, state->lowpassFilters.size() - 1, newCrossoverFreq);
        } else {
            // The old highest crossover is at the maximum, move it halfway down to the one below
            // and place the new one at the maximum
            const double thirdHighestCrossover {CrossoverMutators::getCrossoverFrequency(state, state->lowpassFilters.size() - 3)};
            const double bandWidth {MAX_FREQ - thirdHighestCrossover};
            const double adjustedCrossoverFreq {thirdHighestCrossover + (bandWidth / 2)};

            setCrossoverFrequency(state, state->lowpassFilters.size() - 2, adjustedCrossoverFreq);
            setCrossoverFrequency(state, state->lowpassFilters.size() - 1, MAX_FREQ);
        }

        CrossoverProcessors::prepareToPlay(*state.get(), state->config.sampleRate, state->config.blockSize, state->config.layout);
        CrossoverProcessors::reset(*state.get());
    }

    void removeBand(std::shared_ptr<CrossoverState> state, size_t crossoverNumber) {
        if (state->lowpassFilters.size() > crossoverNumber && state->lowpassFilters.size() > 1) {
            state->lowpassFilters.erase(state->lowpassFilters.begin() + crossoverNumber);
            state->highpassFilters.erase(state->highpassFilters.begin() + crossoverNumber);

            // We might also have an allpass filter to delete
            if (state->allpassFilters.size() > crossoverNumber - 1) {
                state->allpassFilters.erase(state->allpassFilters.begin() + crossoverNumber - 1);
            }

            state->buffers.erase(state->buffers.begin() + crossoverNumber);

            state->bands.erase(state->bands.begin() + crossoverNumber);

            CrossoverProcessors::reset(*state.get());
        }
    }
}
#include "ModulationProcessors.hpp"

namespace Mi = ModelInterface;

namespace ModulationProcessors {
    void prepareToPlay(Mi::ModulationSourcesState& state, double sampleRate, int samplesPerBlock, juce::AudioProcessor::BusesLayout layout) {
        state.hostConfig.sampleRate = sampleRate;
        state.hostConfig.blockSize = samplesPerBlock;
        state.hostConfig.layout = layout;

        for (std::shared_ptr<Mi::CloneableLFO>& lfo : state.lfos) {
            lfo->setSampleRate(sampleRate);
        }

        for (std::shared_ptr<Mi::EnvelopeWrapper>& env : state.envelopes) {
            env->envelope->setSampleRate(sampleRate);
        }
    }

    void reset(Mi::ModulationSourcesState& state) {
        for (std::shared_ptr<Mi::CloneableLFO>& lfo : state.lfos) {
            lfo->reset();
        }

        for (std::shared_ptr<Mi::EnvelopeWrapper>& env : state.envelopes) {
            env->envelope->reset();
        }
    }

    void processBlock(Mi::ModulationSourcesState& state, juce::AudioBuffer<float>& buffer, juce::AudioPlayHead::CurrentPositionInfo tempoInfo) {
        for (std::shared_ptr<Mi::CloneableLFO>& lfo : state.lfos) {
            lfo->prepareForNextBuffer(tempoInfo.bpm, tempoInfo.timeInSeconds);
        }

        for (std::shared_ptr<Mi::CloneableLFO>& lfo : state.lfos) {
            for (int sampleIndex {0}; sampleIndex < buffer.getNumSamples(); sampleIndex++) {
                lfo->getNextOutput(0);
            }
        }

        // TODO this could be faster
        const int totalNumInputChannels = buffer.getNumChannels();
        for (std::shared_ptr<Mi::EnvelopeWrapper> env : state.envelopes) {
            // Figure out which channels we need to be looking at
            int startChannel {0};
            int endChannel {0};

            if (env->useSidechainInput) {
                startChannel = state.hostConfig.layout.getMainInputChannels();
                endChannel = totalNumInputChannels;
            } else {
                startChannel = 0;
                endChannel = state.hostConfig.layout.getMainInputChannels();
            }

            for (int sampleIndex {0}; sampleIndex < buffer.getNumSamples(); sampleIndex++) {
                // Average the samples across all channels
                float averageSample {0};
                for (int channelIndex {startChannel}; channelIndex < endChannel; channelIndex++) {
                    averageSample += buffer.getReadPointer(channelIndex)[sampleIndex];
                }
                averageSample /= (endChannel - startChannel);

                env->envelope->getNextOutput(averageSample);
            }
        }
    }

    double getLfoModulationValue(Mi::ModulationSourcesState& state, int lfoNumber) {
        const int index {lfoNumber - 1};
        if (state.lfos.size() > index) {
            return state.lfos[index]->getLastOutput();
        }

        return 0;
    }

    double getEnvelopeModulationValue(Mi::ModulationSourcesState& state, int envelopeNumber) {
        const int index {envelopeNumber - 1};
        if (state.envelopes.size() > index) {
            return state.envelopes[index]->envelope->getLastOutput() * state.envelopes[index]->amount;
        }

        return 0;
    }
}

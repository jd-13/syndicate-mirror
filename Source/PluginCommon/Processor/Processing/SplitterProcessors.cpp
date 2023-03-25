#include "SplitterProcessors.hpp"
#include "ChainProcessors.hpp"

namespace {
        void copyBuffer(juce::AudioBuffer<float>& source, juce::AudioBuffer<float>& destination) {
        if (source.getNumSamples() == destination.getNumSamples()) {

            int channelsToCopy {std::min(source.getNumChannels(), destination.getNumChannels())};

            // For each channel, copy each sample
            for (int channelIndex {0}; channelIndex < channelsToCopy; channelIndex++) {
                const float* readPointer = source.getReadPointer(channelIndex);
                float* writePointer = destination.getWritePointer(channelIndex);

                juce::FloatVectorOperations::copy(writePointer, readPointer, source.getNumSamples());
            }
        }
    }

    void addBuffers(juce::AudioBuffer<float>& source, juce::AudioBuffer<float>& destination) {
        if (source.getNumSamples() == destination.getNumSamples()) {

            int channelsToCopy {std::min(source.getNumChannels(), destination.getNumChannels())};

            // For each channel, add each sample
            for (int channelIndex {0}; channelIndex < channelsToCopy; channelIndex++) {
                const float* readPointer = source.getReadPointer(channelIndex);
                float* writePointer = destination.getWritePointer(channelIndex);

                juce::FloatVectorOperations::add(writePointer, readPointer, source.getNumSamples());
            }
        }
    }

    void processBlockSeries(PluginSplitterSeries& splitter, juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
        ChainProcessors::processBlock(*(splitter.chains[0].chain.get()), buffer, midiMessages);
    }

    void processBlockParallel(PluginSplitterParallel& splitter, juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
        splitter.outputBuffer->clear();

        for (PluginChainWrapper& chain : splitter.chains) {

            // Only process if no bands are soloed or this one is soloed
            if (splitter.numChainsSoloed == 0 || chain.isSoloed) {
                // Make a copy of the input buffer for us to process, preserving the original for the other
                // chains
                // TODO: do the same for midi
                copyBuffer(buffer, *(splitter.inputBuffer.get()));

                // Process the newly copied buffer
                ChainProcessors::processBlock(*(chain.chain.get()), *(splitter.inputBuffer.get()), midiMessages);

                // Add the output of this chain to the output buffer
                addBuffers(*(splitter.inputBuffer.get()), *(splitter.outputBuffer.get()));
            }
        }

        // Overwrite the original buffer with our own output
        copyBuffer(*(splitter.outputBuffer.get()), buffer);
    }

    void processBlockMultiband(PluginSplitterMultiband& splitter, juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
        splitter.fftProvider.processBlock(buffer);
        CrossoverProcessors::processBlock(*splitter.crossover.get(), buffer);
    }

    void processBlockLeftRight(PluginSplitterLeftRight& splitter, juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
        // TODO: maybe this should be done using mono buffers? (depends if plugins can handle it reliably)

        // Make sure to clear the buffers each time, as on a previous call the plugins may have left
        // data in the unused channel of each buffer, and since it's not used it won't get implicitly
        // overwritten (but it'll still be copied to the output)
        splitter.leftBuffer->clear();
        splitter.rightBuffer->clear();

        // Copy the left and right channels to separate buffers
        if (splitter.numChainsSoloed == 0 || splitter.chains[0].isSoloed) {
            const float* leftRead {buffer.getReadPointer(0)};
            float* leftWrite {splitter.leftBuffer->getWritePointer(0)};
            juce::FloatVectorOperations::copy(leftWrite, leftRead, buffer.getNumSamples());
        }

        if (splitter.numChainsSoloed == 0 || splitter.chains[1].isSoloed) {
            const float* rightRead {buffer.getReadPointer(1)};
            float* rightWrite {splitter.rightBuffer->getWritePointer(1)};
            juce::FloatVectorOperations::copy(rightWrite, rightRead, buffer.getNumSamples());
        }

        // Now the input has been copied we can clear the original
        buffer.clear();

        // Process the left chain
        if (splitter.numChainsSoloed == 0 || splitter.chains[0].isSoloed) {
            ChainProcessors::processBlock(*(splitter.chains[0].chain.get()), *(splitter.leftBuffer.get()), midiMessages);
            addBuffers(*(splitter.leftBuffer.get()), buffer);
        }

        // Process the right chain
        if (splitter.numChainsSoloed == 0 || splitter.chains[1].isSoloed) {
            ChainProcessors::processBlock(*(splitter.chains[1].chain.get()), *(splitter.rightBuffer.get()), midiMessages);
            addBuffers(*(splitter.rightBuffer.get()), buffer);
        }
    }

    void processBlockMidSide(PluginSplitterMidSide& splitter, juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
        // TODO: check if this can be done using mono buffers that refer to the original buffer rather
        // than copying to new ones (guest plugins don't seem to like mono buffers)

        // Make sure to clear the buffers each time, as on a previous call the plugins may have left
        // data in the unused channel of each buffer, and since it's not used it won't get implicitly
        // overwritten (but it'll still be copied to the output)
        splitter.midBuffer->clear();
        splitter.sideBuffer->clear();

        // Convert the left/right buffer to mid/side
        const float* leftRead {buffer.getReadPointer(0)};
        const float* rightRead {buffer.getReadPointer(1)};
        float* midWrite {splitter.midBuffer->getWritePointer(0)};
        float* sideWrite {splitter.sideBuffer->getWritePointer(0)};

        const int numSamples {buffer.getNumSamples()};

        // Add the right channel to get the mid, subtract it to get the side
        juce::FloatVectorOperations::add(midWrite, leftRead, rightRead, numSamples);
        juce::FloatVectorOperations::subtract(sideWrite, leftRead, rightRead, numSamples);

        // Multiply both by 0.5
        juce::FloatVectorOperations::multiply(midWrite, 0.5, numSamples);
        juce::FloatVectorOperations::multiply(sideWrite, 0.5, numSamples);

        // Process the buffers
        if (splitter.numChainsSoloed == 0 || splitter.chains[0].isSoloed) {
            ChainProcessors::processBlock(*(splitter.chains[0].chain.get()), *(splitter.midBuffer.get()), midiMessages);
        } else {
            // Mute the mid channel if only the other one is soloed
            juce::FloatVectorOperations::fill(midWrite, 0, numSamples);
        }

        if (splitter.numChainsSoloed == 0 || splitter.chains[1].isSoloed) {
            ChainProcessors::processBlock(*(splitter.chains[1].chain.get()), *(splitter.sideBuffer.get()), midiMessages);

        } else {
            // Mute the side channel if only the other one is soloed
            juce::FloatVectorOperations::fill(sideWrite, 0, numSamples);
        }

        // Convert from mid/side back to left/right, overwrite the original buffer with our own output
        float* leftWrite {buffer.getWritePointer(0)};
        float* rightWrite {buffer.getWritePointer(1)};
        const float* midRead {splitter.midBuffer->getReadPointer(0)};
        const float* sideRead {splitter.sideBuffer->getReadPointer(0)};

        // Add mid and side to get the left buffer, subtract them to get the right buffer
        juce::FloatVectorOperations::add(leftWrite, midRead, sideRead, numSamples);
        juce::FloatVectorOperations::subtract(rightWrite, midRead, sideRead, numSamples);
    }
}

namespace SplitterProcessors {
    void prepareToPlay(PluginSplitter& splitter, double sampleRate, int samplesPerBlock, juce::AudioProcessor::BusesLayout layout) {
        splitter.config.sampleRate = sampleRate;
        splitter.config.blockSize = samplesPerBlock;
        splitter.config.layout = layout;

        if (auto parallelSplitter = dynamic_cast<PluginSplitterParallel*>(&splitter)) {
            parallelSplitter->inputBuffer.reset(new juce::AudioBuffer<float>(getTotalNumInputChannels(layout), samplesPerBlock));
            parallelSplitter->outputBuffer.reset(new juce::AudioBuffer<float>(2, samplesPerBlock)); // stereo main
        } else if (auto multibandSplitter = dynamic_cast<PluginSplitterMultiband*>(&splitter)) {
            CrossoverProcessors::prepareToPlay(*multibandSplitter->crossover.get(), sampleRate, samplesPerBlock, layout);
            CrossoverProcessors::reset(*multibandSplitter->crossover.get());
            multibandSplitter->fftProvider.reset();
            multibandSplitter->fftProvider.setSampleRate(sampleRate);
            multibandSplitter->fftProvider.setIsStereo(canDoStereoSplitTypes(layout));
        } else if (auto leftRightSplitter = dynamic_cast<PluginSplitterLeftRight*>(&splitter)) {
            leftRightSplitter->leftBuffer.reset(new juce::AudioBuffer<float>(getTotalNumInputChannels(layout), samplesPerBlock));
            leftRightSplitter->rightBuffer.reset(new juce::AudioBuffer<float>(getTotalNumInputChannels(layout), samplesPerBlock));
        } else if (auto midSideSplitter = dynamic_cast<PluginSplitterMidSide*>(&splitter)) {
            midSideSplitter->midBuffer.reset(new juce::AudioBuffer<float>(getTotalNumInputChannels(layout), samplesPerBlock));
            midSideSplitter->sideBuffer.reset(new juce::AudioBuffer<float>(getTotalNumInputChannels(layout), samplesPerBlock));
        }

        for (PluginChainWrapper& chainWrapper : splitter.chains) {
            ChainProcessors::prepareToPlay(*chainWrapper.chain.get(), splitter.config);
        }
    }
    void releaseResources(PluginSplitter& splitter) {
        for (PluginChainWrapper& chainWrapper : splitter.chains) {
            ChainProcessors::releaseResources(*chainWrapper.chain.get());
        }
    }

    void reset(PluginSplitter& splitter) {
        for (PluginChainWrapper& chainWrapper : splitter.chains) {
            ChainProcessors::reset(*chainWrapper.chain.get());
        }
    }

    void processBlock(PluginSplitter& splitter, juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
        if (auto seriesSplitter = dynamic_cast<PluginSplitterSeries*>(&splitter)) {
            processBlockSeries(*seriesSplitter, buffer, midiMessages);
        } else if (auto parallelSplitter = dynamic_cast<PluginSplitterParallel*>(&splitter)) {
            processBlockParallel(*parallelSplitter, buffer, midiMessages);
        } else if (auto multibandSplitter = dynamic_cast<PluginSplitterMultiband*>(&splitter)) {
            processBlockMultiband(*multibandSplitter, buffer, midiMessages);
        } else if (auto leftRightSplitter = dynamic_cast<PluginSplitterLeftRight*>(&splitter)) {
            processBlockLeftRight(*leftRightSplitter, buffer, midiMessages);
        } else if (auto midSideSplitter = dynamic_cast<PluginSplitterMidSide*>(&splitter)) {
            processBlockMidSide(*midSideSplitter, buffer, midiMessages);
        }
    }
}

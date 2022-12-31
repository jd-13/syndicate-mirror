#include "ChainProcessors.hpp"

#include "ChainSlotProcessors.hpp"

namespace ChainProcessor {
    void prepareToPlay(PluginChain& chain, HostConfiguration config) {
        chain.latencyCompLine->prepare({
            config.sampleRate,
            static_cast<juce::uint32>(config.blockSize),
            static_cast<juce::uint32>(getTotalNumInputChannels(config.layout))
        });

        for (std::shared_ptr<ChainSlotBase> slot : chain.chain) {
            if (auto gainStage = std::dynamic_pointer_cast<ChainSlotGainStage>(slot)) {
                ChainProcessor::prepareToPlay(*gainStage.get(), config);
            } else if (auto pluginSlot = std::dynamic_pointer_cast<ChainSlotPlugin>(slot)) {
                ChainProcessor::prepareToPlay(*pluginSlot.get(), config);
            }
        }
    }

    void releaseResources(PluginChain& chain) {
        for (std::shared_ptr<ChainSlotBase> slot : chain.chain) {
            if (auto gainStage = std::dynamic_pointer_cast<ChainSlotGainStage>(slot)) {
                ChainProcessor::releaseResources(*gainStage.get());
            } else if (auto pluginSlot = std::dynamic_pointer_cast<ChainSlotPlugin>(slot)) {
                ChainProcessor::releaseResources(*pluginSlot.get());
            }
        }
    }

    void reset(PluginChain& chain) {
        for (std::shared_ptr<ChainSlotBase> slot : chain.chain) {
            if (auto gainStage = std::dynamic_pointer_cast<ChainSlotGainStage>(slot)) {
                ChainProcessor::reset(*gainStage.get());
            } else if (auto pluginSlot = std::dynamic_pointer_cast<ChainSlotPlugin>(slot)) {
                ChainProcessor::reset(*pluginSlot.get());
            }
        }
    }

    void processBlock(PluginChain& chain, juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
        // Add the latency compensation
        juce::dsp::AudioBlock<float> bufferBlock(buffer);
        juce::dsp::ProcessContextReplacing<float> context(bufferBlock);

        {
            WECore::AudioSpinTryLock lock(chain.latencyCompLineMutex);
            if (lock.isLocked()) {
                chain.latencyCompLine->process(context);
            }
        }

        // Mute gets priority over bypass
        if (chain.isChainMuted) {
            // Muted - return empty buffers (including sidechains since we don't need them)
            for (int channelIndex {0}; channelIndex < buffer.getNumChannels(); channelIndex++) {
                juce::FloatVectorOperations::fill(buffer.getWritePointer(channelIndex), 0, buffer.getNumSamples());
            }
        } else if (chain.isChainBypassed) {
            // Bypassed - do nothing
        } else {
            // Chain is active - process as normal
            for (std::shared_ptr<ChainSlotBase> slot : chain.chain) {
                if (auto gainStage = std::dynamic_pointer_cast<ChainSlotGainStage>(slot)) {
                    ChainProcessor::processBlock(*gainStage.get(), buffer);
                } else if (auto pluginSlot = std::dynamic_pointer_cast<ChainSlotPlugin>(slot)) {
                    ChainProcessor::processBlock(*pluginSlot.get(), buffer, midiMessages);
                }
            }
        }
    }
}

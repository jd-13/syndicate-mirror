#include "ChainMutators.hpp"
#include "ChainSlotProcessors.hpp"

namespace ChainMutators {
    void insertPlugin(std::shared_ptr<PluginChain> chain, std::shared_ptr<juce::AudioPluginInstance> plugin, int position) {
        if (chain->chain.size() > position) {
            chain->chain.insert(chain->chain.begin() + position, std::make_shared<ChainSlotPlugin>(plugin, false, chain->getModulationValueCallback));
        } else {
            // If the position is bigger than the chain just add it to the end
            chain->chain.push_back(std::make_shared<ChainSlotPlugin>(plugin, false, chain->getModulationValueCallback));
        }

        plugin->addListener(&chain->latencyListener);
        chain->latencyListener.onPluginChainUpdate();
    }

    void replacePlugin(std::shared_ptr<PluginChain> chain, std::shared_ptr<juce::AudioPluginInstance> plugin, int position) {
        if (chain->chain.size() > position) {
            // If it's a plugin remove the listener so we don't continue getting updates if it's kept
            // alive somewhere else
            if (const auto oldPluginSlot = std::dynamic_pointer_cast<ChainSlotPlugin>(chain->chain[position])) {
                oldPluginSlot->plugin->removeListener(&chain->latencyListener);
            }

            chain->chain[position] = std::make_unique<ChainSlotPlugin>(plugin, false, chain->getModulationValueCallback);
        } else {
            // If the position is bigger than the chain just add it to the end
            chain->chain.push_back(std::make_unique<ChainSlotPlugin>(plugin, false, chain->getModulationValueCallback));
        }

        plugin->addListener(&chain->latencyListener);
        chain->latencyListener.onPluginChainUpdate();
    }

    bool removeSlot(std::shared_ptr<PluginChain> chain, int position) {
        if (chain->chain.size() > position) {
            // If it's a plugin remove the listener so we don't continue getting updates if it's kept
            // alive somewhere else
            if (const auto oldPluginSlot = std::dynamic_pointer_cast<ChainSlotPlugin>(chain->chain[position])) {
                oldPluginSlot->plugin->removeListener(&chain->latencyListener);
            }

            chain->chain.erase(chain->chain.begin() + position);
            chain->latencyListener.onPluginChainUpdate();
            return true;
        }

        return false;
    }

    void insertGainStage(std::shared_ptr<PluginChain> chain, int position, HostConfiguration config) {
        auto gainStage = std::make_shared<ChainSlotGainStage>(1, 0, false, config.layout);

        ChainProcessors::prepareToPlay(*gainStage.get(), config);

        if (chain->chain.size() > position) {
            chain->chain.insert(chain->chain.begin() + position, std::move(gainStage));
        } else {
            // If the position is bigger than the chain just add it to the end
            chain->chain.push_back(std::move(gainStage));
        }
    }

    std::shared_ptr<juce::AudioPluginInstance> getPlugin(std::shared_ptr<PluginChain> chain, int position) {
        if (chain->chain.size() > position) {
            if (const auto pluginSlot = std::dynamic_pointer_cast<ChainSlotPlugin>(chain->chain[position])) {
                return pluginSlot->plugin;
            }
        }

        return nullptr;
    }

    bool setPluginModulationConfig(std::shared_ptr<PluginChain> chain,
                                   PluginModulationConfig config,
                                   int position) {
        if (chain->chain.size() > position) {
            if (const auto pluginSlot = std::dynamic_pointer_cast<ChainSlotPlugin>(chain->chain[position])) {
                pluginSlot->modulationConfig = std::make_shared<PluginModulationConfig>(config);
                return true;
            }
        }

        return false;
    }

    PluginModulationConfig getPluginModulationConfig(std::shared_ptr<PluginChain> chain, int position) {
        PluginModulationConfig retVal;

        if (chain->chain.size() > position) {
            if (const auto pluginSlot = std::dynamic_pointer_cast<ChainSlotPlugin>(chain->chain[position])) {
                retVal = *pluginSlot->modulationConfig.get();
            }
        }

        return retVal;
    }

    void setSlotBypass(std::shared_ptr<PluginChain> chain, int position, bool isBypassed) {
        if (chain->chain.size() > position) {
            if (chain->chain[position]->isBypassed != isBypassed) {
                chain->chain[position]->isBypassed = isBypassed;

                // Trigger an update to the latency compensation
                chain->latencyListener.onPluginChainUpdate();
            }
        }
    }

    bool getSlotBypass(std::shared_ptr<PluginChain> chain, int position) {
        if (chain->chain.size() > position) {
            return chain->chain[position]->isBypassed;
        }

        return false;
    }

    void setChainBypass(std::shared_ptr<PluginChain> chain, bool val) {
        chain->isChainBypassed = val;

        // Trigger an update to the latency compensation
        chain->latencyListener.onPluginChainUpdate();
    }

    void setChainMute(std::shared_ptr<PluginChain> chain, bool val) {
        chain->isChainMuted = val;
    }

    bool setGainLinear(std::shared_ptr<PluginChain> chain, int position, float gain) {
        if (chain->chain.size() > position) {
            if (const auto gainStage = std::dynamic_pointer_cast<ChainSlotGainStage>(chain->chain[position])) {
                // TODO bounds check
                gainStage->gain = gain;
                return true;
            }
        }

        return false;
    }

    float getGainLinear(std::shared_ptr<PluginChain> chain, int position) {
        if (chain->chain.size() > position) {
            if (const auto gainStage = std::dynamic_pointer_cast<ChainSlotGainStage>(chain->chain[position])) {
                return gainStage->gain;
            }
        }

        return 0.0f;
    }

    std::optional<GainStageLevelsInterface> getGainStageLevelsInterface(std::shared_ptr<PluginChain> chain, int position) {
        std::optional<GainStageLevelsInterface> retVal;

        if (chain->chain.size() > position) {
            if (const auto gainStage = std::dynamic_pointer_cast<ChainSlotGainStage>(chain->chain[position])) {
                retVal.emplace(gainStage);
            }
        }

        return retVal;
    }

    bool setPan(std::shared_ptr<PluginChain> chain, int position, float pan) {
        if (chain->chain.size() > position) {
            if (const auto gainStage = std::dynamic_pointer_cast<ChainSlotGainStage>(chain->chain[position])) {
                // TODO bounds check
                gainStage->pan = pan;
                return true;
            }
        }

        return false;
    }

    float getPan(std::shared_ptr<PluginChain> chain, int position) {
        if (chain->chain.size() > position) {
            if (const auto gainStage = std::dynamic_pointer_cast<ChainSlotGainStage>(chain->chain[position])) {
                return gainStage->pan;
            }
        }

        return 0.0f;
    }

    void setRequiredLatency(std::shared_ptr<PluginChain> chain, int numSamples, HostConfiguration config) {
        // The compensation is the amount of latency we need to add artificially to the latency of the
        // plugins in this chain in order to meet the required amount
        // If this is the slowest chain owned by the splitter this should be 0
        const int compensation {std::max(numSamples - chain->latencyListener.calculatedTotalPluginLatency, 0)};

        WECore::AudioSpinLock lock(chain->latencyCompLineMutex);
        chain->latencyCompLine.reset(new juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::None>(compensation));
        chain->latencyCompLine->prepare({
            config.sampleRate,
            static_cast<juce::uint32>(config.blockSize),
            static_cast<juce::uint32>(getTotalNumInputChannels(config.layout))
        });
        chain->latencyCompLine->setDelay(compensation);
    }

    std::shared_ptr<PluginEditorBounds> getPluginEditorBounds(std::shared_ptr<PluginChain> chain, int position) {
        std::shared_ptr<PluginEditorBounds> retVal(new PluginEditorBounds());

        if (chain->chain.size() > position) {
            if (const auto pluginSlot = std::dynamic_pointer_cast<ChainSlotPlugin>(chain->chain[position])) {
                retVal = pluginSlot->editorBounds;
            }
        }

        return retVal;
    }
}

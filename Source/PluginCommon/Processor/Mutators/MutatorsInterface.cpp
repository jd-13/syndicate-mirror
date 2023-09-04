#include "MutatorsInterface.hpp"

#include <assert.h>

#include "SplitterMutators.hpp"
#include "ModulationMutators.hpp"
#include "XmlConsts.hpp"
#include "XmlReader.hpp"
#include "XmlWriter.hpp"

namespace SplitterInterface {
    bool setSplitType(Splitter& splitter, SPLIT_TYPE splitType, HostConfiguration config) {
        std::scoped_lock lock(splitter.mutatorsMutex);
        WECore::AudioSpinLock sharedLock(splitter.sharedMutex);

        const SPLIT_TYPE previousSplitType = SplitterMutators::getSplitType(splitter.splitter);

        if (splitType != previousSplitType) {
            if (auto multibandSplitter = std::dynamic_pointer_cast<PluginSplitterMultiband>(splitter.splitter)) {
                splitter.cachedcrossoverFrequencies = std::vector<float>();

                for (int index {0}; index < CrossoverMutators::getNumBands(multibandSplitter->crossover); index++) {
                    splitter.cachedcrossoverFrequencies.value().push_back(CrossoverMutators::getCrossoverFrequency(multibandSplitter->crossover, index));
                }
            }

            switch (splitType) {
                case SPLIT_TYPE::SERIES:
                    splitter.splitter.reset(new PluginSplitterSeries(splitter.splitter));
                    break;
                case SPLIT_TYPE::PARALLEL:
                    splitter.splitter.reset(new PluginSplitterParallel(splitter.splitter));
                    break;
                case SPLIT_TYPE::MULTIBAND:
                    splitter.splitter.reset(new PluginSplitterMultiband(splitter.splitter, splitter.cachedcrossoverFrequencies));
                    break;
                case SPLIT_TYPE::LEFTRIGHT:
                    if (canDoStereoSplitTypes(config.layout)) {
                        splitter.splitter.reset(new PluginSplitterLeftRight(splitter.splitter));
                    } else {
                        juce::Logger::writeToLog("SyndicateAudioProcessor::setSplitType: Attempted to use left/right split while not in 2in2out configuration");
                        assert(false);
                    }
                    break;
                case SPLIT_TYPE::MIDSIDE:
                    if (canDoStereoSplitTypes(config.layout)) {
                        splitter.splitter.reset(new PluginSplitterMidSide(splitter.splitter));
                    } else {
                        juce::Logger::writeToLog("SyndicateAudioProcessor::setSplitType: Attempted to use mid/side split while not in 2in2out configuration");
                        assert(false);
                    }
                    break;
            }

            // Make sure prepareToPlay has been called on the splitter as we don't actually know if the host
            // will call it via the PluginProcessor
            if (splitter.splitter != nullptr) {
                SplitterProcessors::prepareToPlay(*splitter.splitter.get(), config.sampleRate, config.blockSize, config.layout);
            }

            return true;
        }

        return false;
    }

    SPLIT_TYPE getSplitType(Splitter& splitter) {
        std::scoped_lock lock(splitter.mutatorsMutex);
        if (splitter.splitter != nullptr) {
            return SplitterMutators::getSplitType(splitter.splitter);
        }

        return SPLIT_TYPE::SERIES;
    }

    bool insertPlugin(Splitter& splitter, std::shared_ptr<juce::AudioPluginInstance> plugin, int chainNumber, int positionInChain) {
        std::scoped_lock lock(splitter.mutatorsMutex);
        WECore::AudioSpinLock sharedLock(splitter.sharedMutex);
        if (splitter.splitter != nullptr) {
            return SplitterMutators::insertPlugin(splitter.splitter, plugin, chainNumber, positionInChain);;
        }

        return false;
    }

    bool replacePlugin(Splitter& splitter, std::shared_ptr<juce::AudioPluginInstance> plugin, int chainNumber, int positionInChain) {
        std::scoped_lock lock(splitter.mutatorsMutex);
        WECore::AudioSpinLock sharedLock(splitter.sharedMutex);
        if (splitter.splitter != nullptr) {
            return SplitterMutators::replacePlugin(splitter.splitter, std::move(plugin), chainNumber, positionInChain);
        }

        return false;
    }

    bool removeSlot(Splitter& splitter, int chainNumber, int positionInChain) {
        std::scoped_lock lock(splitter.mutatorsMutex);
        WECore::AudioSpinLock sharedLock(splitter.sharedMutex);
        if (splitter.splitter != nullptr) {
            return SplitterMutators::removeSlot(splitter.splitter, chainNumber, positionInChain);
        }

        return false;
    }

    bool insertGainStage(Splitter& splitter, int chainNumber, int positionInChain) {
        std::scoped_lock lock(splitter.mutatorsMutex);
        WECore::AudioSpinLock sharedLock(splitter.sharedMutex);
        if (splitter.splitter != nullptr) {
            return SplitterMutators::insertGainStage(splitter.splitter, chainNumber, positionInChain);
        }

        return false;
    }

    std::shared_ptr<juce::AudioPluginInstance> getPlugin(Splitter& splitter, int chainNumber, int positionInChain) {
        std::scoped_lock lock(splitter.mutatorsMutex);
        if (splitter.splitter != nullptr) {
            return SplitterMutators::getPlugin(splitter.splitter, chainNumber, positionInChain);
        }

        return nullptr;
    }

    bool setGainLinear(Splitter& splitter, int chainNumber, int positionInChain, float gain) {
        std::scoped_lock lock(splitter.mutatorsMutex);
        if (splitter.splitter != nullptr) {
            return SplitterMutators::setGainLinear(splitter.splitter, chainNumber, positionInChain, gain);
        }

        return false;
    }

    bool setPan(Splitter& splitter, int chainNumber, int positionInChain, float pan) {
        std::scoped_lock lock(splitter.mutatorsMutex);
        if (splitter.splitter != nullptr) {
            return SplitterMutators::setPan(splitter.splitter, chainNumber, positionInChain, pan);
        }

        return false;
    }

    std::tuple<float, float> getGainLinearAndPan(Splitter& splitter, int chainNumber, int positionInChain) {
        std::scoped_lock lock(splitter.mutatorsMutex);
        if (splitter.splitter != nullptr) {
            const float gain {SplitterMutators::getGainLinear(splitter.splitter, chainNumber, positionInChain)};
            const float pan {SplitterMutators::getPan(splitter.splitter, chainNumber, positionInChain)};

            return std::make_tuple(gain, pan);
        }

        return std::make_tuple<float, float>(0, 0);
    }

    bool setPluginModulationConfig(Splitter& splitter, PluginModulationConfig config, int chainNumber, int positionInChain) {
        std::scoped_lock lock(splitter.mutatorsMutex);
        WECore::AudioSpinLock sharedLock(splitter.sharedMutex);
        if (splitter.splitter != nullptr) {
            return SplitterMutators::setPluginModulationConfig(splitter.splitter, config, chainNumber, positionInChain);
        }

        return false;
    }

    PluginModulationConfig getPluginModulationConfig(Splitter& splitter, int chainNumber, int positionInChain) {
        std::scoped_lock lock(splitter.mutatorsMutex);
        if (splitter.splitter != nullptr) {
            return SplitterMutators::getPluginModulationConfig(splitter.splitter, chainNumber, positionInChain);
        }

        return PluginModulationConfig();
    }

    void removeModulationSource(Splitter& splitter, ModulationSourceDefinition definition) {
        std::scoped_lock lock(splitter.mutatorsMutex);
        WECore::AudioSpinLock sharedLock(splitter.sharedMutex);

        // Iterate through each plugin, remove the source if it has been assigned and renumber ones that
        // are numbered higher
        for (PluginChainWrapper& chain : splitter.splitter->chains) {
            for (int slotIndex {0}; slotIndex < ChainMutators::getNumSlots(chain.chain); slotIndex++) {
                PluginModulationConfig thisPluginConfig = ChainMutators::getPluginModulationConfig(chain.chain, slotIndex);

                // Iterate through each configured parameter
                for (std::shared_ptr<PluginParameterModulationConfig> parameterConfig : thisPluginConfig.parameterConfigs) {
                    bool needsToDelete {false};
                    int indexToDelete {0};

                    // Iterate through each configured source
                    for (int sourceIndex {0}; sourceIndex < parameterConfig->sources.size(); sourceIndex++) {
                        std::shared_ptr<PluginParameterModulationSource> thisSource = parameterConfig->sources[sourceIndex];

                        if (thisSource->definition == definition) {
                            // We need to come back and delete this one
                            needsToDelete = true;
                            indexToDelete = sourceIndex;
                        } else if (thisSource->definition.type == definition.type &&
                                thisSource->definition.id > definition.id) {
                            // We need to renumber this one
                            thisSource->definition.id--;
                        }
                    }

                    if (needsToDelete) {
                        parameterConfig->sources.erase(parameterConfig->sources.begin() + indexToDelete);
                    }
                }

                ChainMutators::setPluginModulationConfig(chain.chain, thisPluginConfig, slotIndex);
            }
        }
    }

    void setSlotBypass(Splitter& splitter, int chainNumber, int positionInChain, bool isBypassed) {
        std::scoped_lock lock(splitter.mutatorsMutex);
        if (splitter.splitter != nullptr) {
            SplitterMutators::setSlotBypass(splitter.splitter, chainNumber, positionInChain, isBypassed);
        }
    }

    bool getSlotBypass(Splitter& splitter, int chainNumber, int positionInChain) {
        std::scoped_lock lock(splitter.mutatorsMutex);
        if (splitter.splitter != nullptr) {
            return SplitterMutators::getSlotBypass(splitter.splitter, chainNumber, positionInChain);
        }

        return false;
    }

    void setChainBypass(Splitter& splitter, int chainNumber, bool val) {
        std::scoped_lock lock(splitter.mutatorsMutex);
        if (splitter.splitter != nullptr) {
            ChainMutators::setChainBypass(splitter.splitter->chains[chainNumber].chain, val);
        }
    }

    void setChainMute(Splitter& splitter, int chainNumber, bool val) {
        std::scoped_lock lock(splitter.mutatorsMutex);
        if (splitter.splitter != nullptr) {
            ChainMutators::setChainMute(splitter.splitter->chains[chainNumber].chain, val);
        }
    }

    void setChainSolo(Splitter& splitter, int chainNumber, bool val) {
        std::scoped_lock lock(splitter.mutatorsMutex);
        if (splitter.splitter != nullptr) {
            SplitterMutators::setChainSolo(splitter.splitter, chainNumber, val);
        }
    }

    bool getChainBypass(Splitter& splitter, int chainNumber) {
        std::scoped_lock lock(splitter.mutatorsMutex);
        if (splitter.splitter != nullptr) {
            return splitter.splitter->chains[chainNumber].chain->isChainBypassed;
        }

        return false;
    }

    bool getChainMute(Splitter& splitter, int chainNumber) {
        std::scoped_lock lock(splitter.mutatorsMutex);
        if (splitter.splitter != nullptr) {
            return splitter.splitter->chains[chainNumber].chain->isChainMuted;
        }

        return false;
    }

    bool getChainSolo(Splitter& splitter, int chainNumber) {
        std::scoped_lock lock(splitter.mutatorsMutex);
        if (splitter.splitter != nullptr) {
            return SplitterMutators::getChainSolo(splitter.splitter, chainNumber);
        }

        return false;
    }

    size_t getNumChains(Splitter& splitter) {
        std::scoped_lock lock(splitter.mutatorsMutex);
        if (splitter.splitter != nullptr) {
            return SplitterMutators::getNumChains(splitter.splitter);
        }

        return 0;
    }

    bool addParallelChain(Splitter& splitter) {
        std::scoped_lock lock(splitter.mutatorsMutex);
        WECore::AudioSpinLock sharedLock(splitter.sharedMutex);
        auto parallelSplitter = std::dynamic_pointer_cast<PluginSplitterParallel>(splitter.splitter);

        if (parallelSplitter != nullptr) {
            SplitterMutators::addChain(parallelSplitter);
            return true;
        }

        return false;
    }

    bool removeParallelChain(Splitter& splitter, int chainNumber) {
        std::scoped_lock lock(splitter.mutatorsMutex);
        WECore::AudioSpinLock sharedLock(splitter.sharedMutex);
        auto parallelSplitter = std::dynamic_pointer_cast<PluginSplitterParallel>(splitter.splitter);

        if (parallelSplitter != nullptr) {
            return SplitterMutators::removeChain(parallelSplitter, chainNumber);
        }

        return false;
    }

    void addCrossoverBand(Splitter& splitter) {
        std::scoped_lock lock(splitter.mutatorsMutex);
        WECore::AudioSpinLock sharedLock(splitter.sharedMutex);
        auto multibandSplitter = std::dynamic_pointer_cast<PluginSplitterMultiband>(splitter.splitter);

        if (multibandSplitter != nullptr) {
            SplitterMutators::addBand(multibandSplitter);
        }
    }

    bool removeCrossoverBand(Splitter& splitter, int bandNumber) {
        std::scoped_lock lock(splitter.mutatorsMutex);
        WECore::AudioSpinLock sharedLock(splitter.sharedMutex);
        auto multibandSplitter = std::dynamic_pointer_cast<PluginSplitterMultiband>(splitter.splitter);

        if (multibandSplitter != nullptr) {
            return SplitterMutators::removeBand(multibandSplitter, bandNumber);
        }

        return false;
    }

    bool setCrossoverFrequency(Splitter& splitter, size_t index, float val) {
        std::scoped_lock lock(splitter.mutatorsMutex);
        auto multibandSplitter = std::dynamic_pointer_cast<PluginSplitterMultiband>(splitter.splitter);

        if (multibandSplitter != nullptr) {
            if (index < SplitterMutators::getNumBands(multibandSplitter) - 1) {
                SplitterMutators::setCrossoverFrequency(multibandSplitter, index, val);
                return true;
            }
        }

        return false;
    }

    float getCrossoverFrequency(Splitter& splitter, size_t index) {
        std::scoped_lock lock(splitter.mutatorsMutex);
        auto multibandSplitter = std::dynamic_pointer_cast<PluginSplitterMultiband>(splitter.splitter);

        if (multibandSplitter != nullptr) {
            if (index < SplitterMutators::getNumBands(multibandSplitter) - 1) {
                return SplitterMutators::getCrossoverFrequency(multibandSplitter, index);
            }
        }

        return 0.0f;
    }

    std::pair<std::array<float, FFTProvider::NUM_OUTPUTS>, float> getFFTOutputs(Splitter& splitter) {
        std::scoped_lock lock(splitter.mutatorsMutex);
        auto multibandSplitter = std::dynamic_pointer_cast<PluginSplitterMultiband>(splitter.splitter);

        if (multibandSplitter != nullptr) {
            std::array<float, FFTProvider::NUM_OUTPUTS> bins;
            const float* outputs = multibandSplitter->fftProvider.getOutputs();

            std::copy(outputs, outputs + FFTProvider::NUM_OUTPUTS, bins.begin());

            return std::make_pair(bins, multibandSplitter->fftProvider.getBinWidth());
        }

        std::array<float, FFTProvider::NUM_OUTPUTS> bins;
        std::fill(bins.begin(), bins.end(), 0.0f);

        return std::make_pair(bins, 0);
    }

    std::shared_ptr<PluginEditorBounds> getPluginEditorBounds(Splitter& splitter, int chainNumber, int positionInChain) {
        std::scoped_lock lock(splitter.mutatorsMutex);
        if (splitter.splitter != nullptr) {
            return SplitterMutators::getPluginEditorBounds(splitter.splitter, chainNumber, positionInChain);
        }

        return nullptr;
    }

    std::optional<GainStageLevelsInterface> getGainStageLevelsInterface(Splitter& splitter, int chainNumber, int positionInChain) {
        std::scoped_lock lock(splitter.mutatorsMutex);
        if (splitter.splitter != nullptr) {
            return SplitterMutators::getGainStageLevelsInterface(splitter.splitter, chainNumber, positionInChain);
        }

        return std::optional<GainStageLevelsInterface>();
    }

    void forEachChain(Splitter& splitter, std::function<void(int, std::shared_ptr<PluginChain>)> callback) {
        std::scoped_lock lock(splitter.mutatorsMutex);
        if (splitter.splitter != nullptr) {
            for (int chainNumber {0}; chainNumber < splitter.splitter->chains.size(); chainNumber++) {
                callback(chainNumber, splitter.splitter->chains[chainNumber].chain);
            }
        }
    }

    void forEachCrossover(Splitter& splitter, std::function<void(float)> callback) {
        std::scoped_lock lock(splitter.mutatorsMutex);
        auto multibandSplitter = std::dynamic_pointer_cast<PluginSplitterMultiband>(splitter.splitter);

        if (multibandSplitter != nullptr) {
            for (size_t crossoverNumber {0}; crossoverNumber < SplitterMutators::getNumBands(multibandSplitter) - 1; crossoverNumber++) {
                callback(SplitterMutators::getCrossoverFrequency(multibandSplitter, crossoverNumber));
            }
        }
    }

    void writeToXml(Splitter& splitter, juce::XmlElement* element) {
        std::scoped_lock lock(splitter.mutatorsMutex);

        XmlWriter::write(splitter.splitter, element);

        // Store the cached crossover frequencies
        if (splitter.cachedcrossoverFrequencies.has_value()) {
            juce::XmlElement* frequenciesElement = element->createNewChildElement(XML_CACHED_CROSSOVER_FREQUENCIES_STR);
            for (int index {0}; index < splitter.cachedcrossoverFrequencies.value().size(); index++) {
                frequenciesElement->setAttribute(getCachedCrossoverFreqXMLName(index), splitter.cachedcrossoverFrequencies.value()[index]);
            }
        }
    }

    void restoreFromXml(Splitter& splitter,
                        juce::XmlElement* element,
                        std::function<float(int, MODULATION_TYPE)> getModulationValueCallback,
                        std::function<void(int)> latencyChangeCallback,
                        HostConfiguration config,
                        const PluginConfigurator& pluginConfigurator,
                        std::function<void(juce::String)> onErrorCallback) {
        std::scoped_lock lock(splitter.mutatorsMutex);
        WECore::AudioSpinLock sharedLock(splitter.sharedMutex);

        // Restore the cached crossover frequencies first, we need to allow for them to be overwritten
        // by the later call to setSplitType() in the case that we're restoring a multiband split
        juce::XmlElement* frequenciesElement = element->getChildByName(XML_CACHED_CROSSOVER_FREQUENCIES_STR);
        if (frequenciesElement != nullptr) {
            splitter.cachedcrossoverFrequencies = std::vector<float>();
            const int numFrequencies {frequenciesElement->getNumAttributes()};
            for (int index {0}; index < numFrequencies; index++) {
                if (frequenciesElement->hasAttribute(getCachedCrossoverFreqXMLName(index))) {
                    splitter.cachedcrossoverFrequencies.value().push_back(
                        frequenciesElement->getDoubleAttribute(getCachedCrossoverFreqXMLName(index)));
                }
            }
        }

        splitter.splitter = XmlReader::restoreSplitterFromXml(
            element,
            getModulationValueCallback,
            latencyChangeCallback,
            config,
            pluginConfigurator,
            onErrorCallback);

        // Make sure prepareToPlay has been called on the splitter as we don't actually know if the host
        // will call it via the PluginProcessor
        if (splitter.splitter != nullptr) {
            SplitterProcessors::prepareToPlay(*splitter.splitter.get(), config.sampleRate, config.blockSize, config.layout);
        }
    }
}

namespace ModulationInterface {
    void addLfo(ModulationSourcesState& state) {
        std::scoped_lock lock(state.mutatorsMutex);
        WECore::AudioSpinLock sharedLock(state.sharedMutex);
        ModulationMutators::addLfo(state);
    }

    void addEnvelope(ModulationSourcesState& state) {
        std::scoped_lock lock(state.mutatorsMutex);
        WECore::AudioSpinLock sharedLock(state.sharedMutex);
        ModulationMutators::addEnvelope(state);
    }

    void removeModulationSource(ModulationSourcesState& state, ModulationSourceDefinition definition) {
        std::scoped_lock lock(state.mutatorsMutex);
        WECore::AudioSpinLock sharedLock(state.sharedMutex);
        ModulationMutators::removeModulationSource(state, definition);
    }

    std::shared_ptr<WECore::Richter::RichterLFO> getLfo(ModulationSourcesState& state, int lfoNumber) {
        std::scoped_lock lock(state.mutatorsMutex);

        const int index {lfoNumber - 1};
        if (state.lfos.size() > index) {
            return state.lfos[index];
        }

        return nullptr;
    }

    std::shared_ptr<EnvelopeWrapper> getEnvelope(ModulationSourcesState& state, int envelopeNumber) {
        std::scoped_lock lock(state.mutatorsMutex);

        const int index {envelopeNumber - 1};
        if (state.envelopes.size() > index) {
            return state.envelopes[index];
        }

        return nullptr;
    }

    void forEachLfo(ModulationSourcesState& state, std::function<void(int)> callback) {
        std::scoped_lock lock(state.mutatorsMutex);
        for (int lfoIndex {0}; lfoIndex < state.lfos.size(); lfoIndex++) {
            callback(lfoIndex + 1);
        }
    }

    void forEachEnvelope(ModulationSourcesState& state, std::function<void(int)> callback) {
        std::scoped_lock lock(state.mutatorsMutex);
        for (int envIndex {0}; envIndex < state.envelopes.size(); envIndex++) {
            callback(envIndex + 1);
        }
    }

    void writeToXml(ModulationSourcesState& state, juce::XmlElement* element) {
        std::scoped_lock lock(state.mutatorsMutex);
        XmlWriter::write(state, element);
    }

    void restoreFromXml(ModulationSourcesState& state, juce::XmlElement* element, HostConfiguration config) {
        std::scoped_lock lock(state.mutatorsMutex);
        WECore::AudioSpinLock sharedLock(state.sharedMutex);

        XmlReader::restoreModulationSourcesFromXml(state, element, config);
    }
}

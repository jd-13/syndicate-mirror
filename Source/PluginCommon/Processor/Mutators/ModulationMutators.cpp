#include "ModulationMutators.hpp"

namespace {
    std::vector<WECore::ModulationSourceWrapper<double>> deleteSourceFromTargetSources(
            std::vector<WECore::ModulationSourceWrapper<double>> sources,
            ModulationSourceDefinition definition,
            std::function<float(int, MODULATION_TYPE)> getModulationValueCallback) {
        bool needsToDelete {false};
        int indexToDelete {0};

        // Iterate through each configured source
        for (int sourceIndex {0}; sourceIndex < sources.size(); sourceIndex++) {
            auto thisSource = std::dynamic_pointer_cast<ModulationSourceProvider>(sources[sourceIndex].source);

            if (thisSource == nullptr) {
                continue;
            }

            if (thisSource->definition == definition) {
                // We need to come back and delete this one
                needsToDelete = true;
                indexToDelete = sourceIndex;
            } else if (thisSource->definition.type == definition.type &&
                       thisSource->definition.id > definition.id) {
                // We need to renumber this one since a source below it is being deleted
                // We create a new source rather than modifying the existing one, since that would break undo/redo
                ModulationSourceDefinition newDefinition(thisSource->definition.id - 1, thisSource->definition.type);
                WECore::ModulationSourceWrapper<double> newSource;

                newSource.source = std::make_shared<ModulationSourceProvider>(
                    newDefinition, getModulationValueCallback);

                newSource.amount = sources[sourceIndex].amount;

                sources[sourceIndex] = newSource;
            }
        }

        if (needsToDelete) {
            sources.erase(sources.begin() + indexToDelete);
        }

        return sources;
    }
}

namespace ModulationMutators {
    //
    // LFOs
    //
    void addLfo(std::shared_ptr<ModelInterface::ModulationSourcesState> sources) {
        std::shared_ptr<ModelInterface::CloneableLFO> newLfo {new ModelInterface::CloneableLFO()};
        newLfo->setBypassSwitch(true);
        newLfo->setSampleRate(sources->hostConfig.sampleRate);
        sources->lfos.push_back(newLfo);
    }

    bool setLfoTempoSyncSwitch(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex, bool val) {
        if (sources->lfos.size() > lfoIndex) {
            sources->lfos[lfoIndex]->setTempoSyncSwitch(val);
            return true;
        }

        return false;
    }

    bool setLfoInvertSwitch(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex, bool val) {
        if (sources->lfos.size() > lfoIndex) {
            sources->lfos[lfoIndex]->setInvertSwitch(val);
            return true;
        }

        return false;
    }

    bool setLfoOutputMode(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex, int val) {
        if (sources->lfos.size() > lfoIndex) {
            sources->lfos[lfoIndex]->setOutputMode(val);
            return true;
        }

        return false;
    }

    bool setLfoWave(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex, int val) {
        if (sources->lfos.size() > lfoIndex) {
            sources->lfos[lfoIndex]->setWave(val);
            return true;
        }

        return false;
    }

    bool setLfoTempoNumer(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex, int val) {
        if (sources->lfos.size() > lfoIndex) {
            sources->lfos[lfoIndex]->setTempoNumer(val);
            return true;
        }

        return false;
    }

    bool setLfoTempoDenom(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex, int val) {
        if (sources->lfos.size() > lfoIndex) {
            sources->lfos[lfoIndex]->setTempoDenom(val);
            return true;
        }

        return false;
    }

    bool setLfoFreq(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex, double val) {
        if (sources->lfos.size() > lfoIndex) {
            sources->lfos[lfoIndex]->setFreq(val);
            return true;
        }

        return false;
    }

    bool setLfoDepth(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex, double val) {
        if (sources->lfos.size() > lfoIndex) {
            sources->lfos[lfoIndex]->setDepth(val);
            return true;
        }

        return false;
    }

    bool setLfoManualPhase(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex, double val) {
        if (sources->lfos.size() > lfoIndex) {
            sources->lfos[lfoIndex]->setManualPhase(val);
            return true;
        }

        return false;
    }

    bool addSourceToLFOFreq(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex, ModulationSourceDefinition source) {
        if (sources->lfos.size() <= lfoIndex) {
            return false;
        }

        auto sourceProvider = std::make_shared<ModulationSourceProvider>(source, sources->getModulationValueCallback);
        return sources->lfos[lfoIndex]->addFreqModulationSource(std::dynamic_pointer_cast<WECore::ModulationSource<double>>(sourceProvider));
    }

    bool removeSourceFromLFOFreq(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex, ModulationSourceDefinition source) {
        if (sources->lfos.size() <= lfoIndex) {
            return false;
        }

        std::vector<WECore::ModulationSourceWrapper<double>> existingSources = sources->lfos[lfoIndex]->getFreqModulationSources();
        for (const auto& existingSource : existingSources) {
            auto thisSource = std::dynamic_pointer_cast<ModulationSourceProvider>(existingSource.source);
            if (thisSource != nullptr && thisSource->definition == source) {
                return sources->lfos[lfoIndex]->removeFreqModulationSource(existingSource.source);
            }
        }

        return false;
    }

    bool setLFOFreqModulationAmount(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex, int sourceIndex, double val) {
        if (sources->lfos.size() > lfoIndex) {
            return sources->lfos[lfoIndex]->setFreqModulationAmount(sourceIndex, val);
        }

        return false;
    }

    std::vector<std::shared_ptr<PluginParameterModulationSource>> getLFOFreqModulationSources(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex) {
        // Use std::shared_ptr to be consistent with usage in PluginParameterModulationConfig
        std::vector<std::shared_ptr<PluginParameterModulationSource>> retVal;

        if (sources->lfos.size() > lfoIndex) {
            for (const auto& source : sources->lfos[lfoIndex]->getFreqModulationSources()) {
                auto thisSource = std::dynamic_pointer_cast<ModulationSourceProvider>(source.source);
                retVal.emplace_back(new PluginParameterModulationSource(thisSource->definition, source.amount));
            }
        }

        return retVal;
    }

    bool addSourceToLFODepth(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex, ModulationSourceDefinition source) {
        if (sources->lfos.size() <= lfoIndex) {
            return false;
        }

        auto sourceProvider = std::make_shared<ModulationSourceProvider>(source, sources->getModulationValueCallback);
        return sources->lfos[lfoIndex]->addDepthModulationSource(std::dynamic_pointer_cast<WECore::ModulationSource<double>>(sourceProvider));
    }

    bool removeSourceFromLFODepth(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex, ModulationSourceDefinition source) {
        if (sources->lfos.size() <= lfoIndex) {
            return false;
        }

        std::vector<WECore::ModulationSourceWrapper<double>> existingSources = sources->lfos[lfoIndex]->getDepthModulationSources();
        for (const auto& existingSource : existingSources) {
            auto thisSource = std::dynamic_pointer_cast<ModulationSourceProvider>(existingSource.source);
            if (thisSource != nullptr && thisSource->definition == source) {
                return sources->lfos[lfoIndex]->removeDepthModulationSource(existingSource.source);
            }
        }

        return false;
    }

    bool setLFODepthModulationAmount(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex, int sourceIndex, double val) {
        if (sources->lfos.size() > lfoIndex) {
            return sources->lfos[lfoIndex]->setDepthModulationAmount(sourceIndex, val);
        }

        return false;
    }

    std::vector<std::shared_ptr<PluginParameterModulationSource>> getLFODepthModulationSources(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex) {
        // Use std::shared_ptr to be consistent with usage in PluginParameterModulationConfig
        std::vector<std::shared_ptr<PluginParameterModulationSource>> retVal;

        if (sources->lfos.size() > lfoIndex) {
            for (const auto& source : sources->lfos[lfoIndex]->getDepthModulationSources()) {
                auto thisSource = std::dynamic_pointer_cast<ModulationSourceProvider>(source.source);
                retVal.emplace_back(new PluginParameterModulationSource(thisSource->definition, source.amount));
            }
        }

        return retVal;
    }

    bool addSourceToLFOPhase(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex, ModulationSourceDefinition source) {
        if (sources->lfos.size() <= lfoIndex) {
            return false;
        }

        auto sourceProvider = std::make_shared<ModulationSourceProvider>(source, sources->getModulationValueCallback);
        return sources->lfos[lfoIndex]->addPhaseModulationSource(std::dynamic_pointer_cast<WECore::ModulationSource<double>>(sourceProvider));
    }

    bool removeSourceFromLFOPhase(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex, ModulationSourceDefinition source) {
        if (sources->lfos.size() <= lfoIndex) {
            return false;
        }

        std::vector<WECore::ModulationSourceWrapper<double>> existingSources = sources->lfos[lfoIndex]->getPhaseModulationSources();
        for (const auto& existingSource : existingSources) {
            auto thisSource = std::dynamic_pointer_cast<ModulationSourceProvider>(existingSource.source);
            if (thisSource != nullptr && thisSource->definition == source) {
                return sources->lfos[lfoIndex]->removePhaseModulationSource(existingSource.source);
            }
        }

        return false;
    }

    bool setLFOPhaseModulationAmount(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex, int sourceIndex, double val) {
        if (sources->lfos.size() > lfoIndex) {
            return sources->lfos[lfoIndex]->setPhaseModulationAmount(sourceIndex, val);
        }

        return false;
    }

    std::vector<std::shared_ptr<PluginParameterModulationSource>> getLFOPhaseModulationSources(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex) {
        // Use std::shared_ptr to be consistent with usage in PluginParameterModulationConfig
        std::vector<std::shared_ptr<PluginParameterModulationSource>> retVal;

        if (sources->lfos.size() > lfoIndex) {
            for (const auto& source : sources->lfos[lfoIndex]->getPhaseModulationSources()) {
                auto thisSource = std::dynamic_pointer_cast<ModulationSourceProvider>(source.source);
                retVal.emplace_back(new PluginParameterModulationSource(thisSource->definition, source.amount));
            }
        }

        return retVal;
    }

    bool getLfoTempoSyncSwitch(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex) {
        if (sources->lfos.size() > lfoIndex) {
            return sources->lfos[lfoIndex]->getTempoSyncSwitch();
        }

        return false;
    }

    bool getLfoInvertSwitch(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex) {
        if (sources->lfos.size() > lfoIndex) {
            return sources->lfos[lfoIndex]->getInvertSwitch();
        }

        return false;
    }

    int getLfoOutputMode(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex) {
        if (sources->lfos.size() > lfoIndex) {
            return sources->lfos[lfoIndex]->getOutputMode();
        }

        return 0;
    }

    int getLfoWave(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex) {
        if (sources->lfos.size() > lfoIndex) {
            return sources->lfos[lfoIndex]->getWave();
        }

        return 0;
    }

    double getLfoTempoNumer(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex) {
        if (sources->lfos.size() > lfoIndex) {
            return sources->lfos[lfoIndex]->getTempoNumer();
        }

        return 0;
    }

    double getLfoTempoDenom(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex) {
        if (sources->lfos.size() > lfoIndex) {
            return sources->lfos[lfoIndex]->getTempoDenom();
        }

        return 0;
    }

    double getLfoFreq(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex) {
        if (sources->lfos.size() > lfoIndex) {
            return sources->lfos[lfoIndex]->getFreq();
        }

        return 0;
    }

    double getLFOModulatedFreqValue(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex) {
        if (sources->lfos.size() > lfoIndex) {
            return sources->lfos[lfoIndex]->getModulatedFreqValue();
        }

        return 0;
    }

    double getLfoDepth(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex) {
        if (sources->lfos.size() > lfoIndex) {
            return sources->lfos[lfoIndex]->getDepth();
        }

        return 0;
    }

    double getLFOModulatedDepthValue(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex) {
        if (sources->lfos.size() > lfoIndex) {
            return sources->lfos[lfoIndex]->getModulatedDepthValue();
        }

        return 0;
    }

    double getLfoManualPhase(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex) {
        if (sources->lfos.size() > lfoIndex) {
            return sources->lfos[lfoIndex]->getManualPhase();
        }

        return 0;
    }

    double getLFOModulatedPhaseValue(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex) {
        if (sources->lfos.size() > lfoIndex) {
            return sources->lfos[lfoIndex]->getModulatedPhaseValue();
        }

        return 0;
    }

    //
    // Envelopes
    //
    void addEnvelope(std::shared_ptr<ModelInterface::ModulationSourcesState> sources) {
        std::shared_ptr<ModelInterface::EnvelopeWrapper> newEnv(new ModelInterface::EnvelopeWrapper());
        newEnv->envelope->setSampleRate(sources->hostConfig.sampleRate);
        sources->envelopes.push_back(newEnv);
    }

    bool setEnvAttackTimeMs(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int envIndex, double val) {
        if (sources->envelopes.size() > envIndex) {
            sources->envelopes[envIndex]->envelope->setAttackTimeMs(val);
            return true;
        }

        return false;
    }

    bool setEnvReleaseTimeMs(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int envIndex, double val) {
        if (sources->envelopes.size() > envIndex) {
            sources->envelopes[envIndex]->envelope->setReleaseTimeMs(val);
            return true;
        }

        return false;
    }

    bool setEnvFilterEnabled(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int envIndex, bool val) {
        if (sources->envelopes.size() > envIndex) {
            sources->envelopes[envIndex]->envelope->setFilterEnabled(val);
            return true;
        }

        return false;
    }

    bool setEnvFilterHz(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int envIndex, double lowCut, double highCut) {
        if (sources->envelopes.size() > envIndex) {
            sources->envelopes[envIndex]->envelope->setLowCutHz(lowCut);
            sources->envelopes[envIndex]->envelope->setHighCutHz(highCut);
            return true;
        }

        return false;
    }

    bool setEnvAmount(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int envIndex, float val) {
        if (sources->envelopes.size() > envIndex) {
            sources->envelopes[envIndex]->amount = val;
            return true;
        }

        return false;
    }

    bool setEnvUseSidechainInput(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int envIndex, bool val) {
        if (sources->envelopes.size() > envIndex) {
            sources->envelopes[envIndex]->useSidechainInput = val;
            return true;
        }

        return false;
    }

    double getEnvAttackTimeMs(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int envIndex) {
        if (sources->envelopes.size() > envIndex) {
            return sources->envelopes[envIndex]->envelope->getAttackTimeMs();
        }

        return 0;
    }

    double getEnvReleaseTimeMs(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int envIndex) {
        if (sources->envelopes.size() > envIndex) {
            return sources->envelopes[envIndex]->envelope->getReleaseTimeMs();
        }

        return 0;
    }

    bool getEnvFilterEnabled(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int envIndex) {
        if (sources->envelopes.size() > envIndex) {
            return sources->envelopes[envIndex]->envelope->getFilterEnabled();
        }

        return false;
    }

    double getEnvLowCutHz(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int envIndex) {
        if (sources->envelopes.size() > envIndex) {
            return sources->envelopes[envIndex]->envelope->getLowCutHz();
        }

        return 0;
    }

    double getEnvHighCutHz(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int envIndex) {
        if (sources->envelopes.size() > envIndex) {
            return sources->envelopes[envIndex]->envelope->getHighCutHz();
        }

        return 0;
    }

    float getEnvAmount(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int envIndex) {
        if (sources->envelopes.size() > envIndex) {
            return sources->envelopes[envIndex]->amount;
        }

        return 0;
    }

    bool getEnvUseSidechainInput(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int envIndex) {
        if (sources->envelopes.size() > envIndex) {
            return sources->envelopes[envIndex]->useSidechainInput;
        }

        return false;
    }

    double getEnvLastOutput(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int envIndex) {
        if (sources->envelopes.size() > envIndex) {
            return sources->envelopes[envIndex]->envelope->getLastOutput();
        }

        return 0;
    }

    //
    // Random
    //
    void addRandom(std::shared_ptr<ModelInterface::ModulationSourcesState> sources) {
        std::shared_ptr<WECore::Perlin::PerlinSource> newRandom(new WECore::Perlin::PerlinSource());
        newRandom->setSampleRate(sources->hostConfig.sampleRate);
        sources->randomSources.push_back(newRandom);
    }

    bool setRandomOutputMode(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int randomIndex, int val) {
        if (sources->randomSources.size() > randomIndex) {
            sources->randomSources[randomIndex]->setOutputMode(val);
            return true;
        }

        return false;
    }

    bool setRandomFreq(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int randomIndex, double val) {
        if (sources->randomSources.size() > randomIndex) {
            sources->randomSources[randomIndex]->setFreq(val);
            return true;
        }

        return false;
    }

    bool setRandomDepth(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int randomIndex, double val) {
        if (sources->randomSources.size() > randomIndex) {
            sources->randomSources[randomIndex]->setDepth(val);
            return true;
        }

        return false;
    }

    bool addSourceToRandomFreq(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int randomIndex, ModulationSourceDefinition source) {
        if (sources->randomSources.size() <= randomIndex) {
            return false;
        }

        auto sourceProvider = std::make_shared<ModulationSourceProvider>(source, sources->getModulationValueCallback);
        return sources->randomSources[randomIndex]->addFreqModulationSource(std::dynamic_pointer_cast<WECore::ModulationSource<double>>(sourceProvider));
    }

    bool removeSourceFromRandomFreq(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int randomIndex, ModulationSourceDefinition source) {
        if (sources->randomSources.size() <= randomIndex) {
            return false;
        }

        std::vector<WECore::ModulationSourceWrapper<double>> existingSources = sources->randomSources[randomIndex]->getFreqModulationSources();
        for (const auto& existingSource : existingSources) {
            auto thisSource = std::dynamic_pointer_cast<ModulationSourceProvider>(existingSource.source);
            if (thisSource != nullptr && thisSource->definition == source) {
                return sources->randomSources[randomIndex]->removeFreqModulationSource(existingSource.source);
            }
        }

        return false;
    }

    bool setRandomFreqModulationAmount(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int randomIndex, int sourceIndex, double val) {
        if (sources->randomSources.size() > randomIndex) {
            return sources->randomSources[randomIndex]->setFreqModulationAmount(sourceIndex, val);
        }

        return false;
    }

    std::vector<std::shared_ptr<PluginParameterModulationSource>> getRandomFreqModulationSources(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int randomIndex) {
        // Use std::shared_ptr to be consistent with usage in PluginParameterModulationConfig
        std::vector<std::shared_ptr<PluginParameterModulationSource>> retVal;

        if (sources->randomSources.size() > randomIndex) {
            for (const auto& source : sources->randomSources[randomIndex]->getFreqModulationSources()) {
                auto thisSource = std::dynamic_pointer_cast<ModulationSourceProvider>(source.source);
                retVal.emplace_back(new PluginParameterModulationSource(thisSource->definition, source.amount));
            }
        }

        return retVal;
    }

    bool addSourceToRandomDepth(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int randomIndex, ModulationSourceDefinition source) {
        if (sources->randomSources.size() <= randomIndex) {
            return false;
        }

        auto sourceProvider = std::make_shared<ModulationSourceProvider>(source, sources->getModulationValueCallback);
        return sources->randomSources[randomIndex]->addDepthModulationSource(std::dynamic_pointer_cast<WECore::ModulationSource<double>>(sourceProvider));
    }

    bool removeSourceFromRandomDepth(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int randomIndex, ModulationSourceDefinition source) {
        if (sources->randomSources.size() <= randomIndex) {
            return false;
        }

        std::vector<WECore::ModulationSourceWrapper<double>> existingSources = sources->randomSources[randomIndex]->getDepthModulationSources();
        for (const auto& existingSource : existingSources) {
            auto thisSource = std::dynamic_pointer_cast<ModulationSourceProvider>(existingSource.source);
            if (thisSource != nullptr && thisSource->definition == source) {
                return sources->randomSources[randomIndex]->removeDepthModulationSource(existingSource.source);
            }
        }

        return false;
    }

    bool setRandomDepthModulationAmount(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int randomIndex, int sourceIndex, double val) {
        if (sources->randomSources.size() > randomIndex) {
            return sources->randomSources[randomIndex]->setDepthModulationAmount(sourceIndex, val);
        }

        return false;
    }

    std::vector<std::shared_ptr<PluginParameterModulationSource>> getRandomDepthModulationSources(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int randomIndex) {
        // Use std::shared_ptr to be consistent with usage in PluginParameterModulationConfig
        std::vector<std::shared_ptr<PluginParameterModulationSource>> retVal;

        if (sources->randomSources.size() > randomIndex) {
            for (const auto& source : sources->randomSources[randomIndex]->getDepthModulationSources()) {
                auto thisSource = std::dynamic_pointer_cast<ModulationSourceProvider>(source.source);
                retVal.emplace_back(new PluginParameterModulationSource(thisSource->definition, source.amount));
            }
        }

        return retVal;
    }

    int getRandomOutputMode(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int randomIndex) {
        if (sources->randomSources.size() > randomIndex) {
            return sources->randomSources[randomIndex]->getOutputMode();
        }

        return 0;
    }

    double getRandomFreq(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int randomIndex) {
        if (sources->randomSources.size() > randomIndex) {
            return sources->randomSources[randomIndex]->getFreq();
        }

        return 0;
    }

    double getRandomModulatedFreqValue(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int randomIndex) {
        if (sources->randomSources.size() > randomIndex) {
            return sources->randomSources[randomIndex]->getModulatedFreqValue();
        }

        return 0;
    }

    double getRandomDepth(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int randomIndex) {
        if (sources->randomSources.size() > randomIndex) {
            return sources->randomSources[randomIndex]->getDepth();
        }

        return 0;
    }

    double getRandomModulatedDepthValue(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int randomIndex) {
        if (sources->randomSources.size() > randomIndex) {
            return sources->randomSources[randomIndex]->getModulatedDepthValue();
        }

        return 0;
    }

    double getRandomLastOutput(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int randomIndex) {
        if (sources->randomSources.size() > randomIndex) {
            return sources->randomSources[randomIndex]->getLastOutput();
        }

        return 0;
    }

    //
    // Step Sequencers
    //
    void addStepSequencer(std::shared_ptr<ModelInterface::ModulationSourcesState> sources) {
        std::shared_ptr<WECore::StepSeq::StepSequencer> newSeq {new WECore::StepSeq::StepSequencer()};
        newSeq->setSampleRate(sources->hostConfig.sampleRate);
        sources->stepSequencers.push_back(newSeq);
    }

    bool addSourceToStepSeqFreq(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int seqIndex, ModulationSourceDefinition source) {
        if (sources->stepSequencers.size() <= seqIndex) {
            return false;
        }

        auto sourceProvider = std::make_shared<ModulationSourceProvider>(source, sources->getModulationValueCallback);
        return sources->stepSequencers[seqIndex]->addFreqModulationSource(std::dynamic_pointer_cast<WECore::ModulationSource<double>>(sourceProvider));
    }

    bool removeSourceFromStepSeqFreq(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int seqIndex, ModulationSourceDefinition source) {
        if (sources->stepSequencers.size() <= seqIndex) {
            return false;
        }

        std::vector<WECore::ModulationSourceWrapper<double>> existingSources = sources->stepSequencers[seqIndex]->getFreqModulationSources();
        for (const auto& existingSource : existingSources) {
            auto thisSource = std::dynamic_pointer_cast<ModulationSourceProvider>(existingSource.source);
            if (thisSource != nullptr && thisSource->definition == source) {
                return sources->stepSequencers[seqIndex]->removeFreqModulationSource(existingSource.source);
            }
        }

        return false;
    }

    bool setStepSeqFreqModulationAmount(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int seqIndex, int sourceIndex, double val) {
        if (sources->stepSequencers.size() > seqIndex) {
            return sources->stepSequencers[seqIndex]->setFreqModulationAmount(sourceIndex, val);
        }

        return false;
    }

    std::vector<std::shared_ptr<PluginParameterModulationSource>> getStepSeqFreqModulationSources(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int seqIndex) {
        // Use std::shared_ptr to be consistent with usage in PluginParameterModulationConfig
        std::vector<std::shared_ptr<PluginParameterModulationSource>> retVal;

        if (sources->stepSequencers.size() > seqIndex) {
            for (const auto& source : sources->stepSequencers[seqIndex]->getFreqModulationSources()) {
                auto thisSource = std::dynamic_pointer_cast<ModulationSourceProvider>(source.source);
                retVal.emplace_back(new PluginParameterModulationSource(thisSource->definition, source.amount));
            }
        }

        return retVal;
    }

    bool addSourceToStepSeqDepth(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int seqIndex, ModulationSourceDefinition source) {
        if (sources->stepSequencers.size() <= seqIndex) {
            return false;
        }

        auto sourceProvider = std::make_shared<ModulationSourceProvider>(source, sources->getModulationValueCallback);
        return sources->stepSequencers[seqIndex]->addDepthModulationSource(std::dynamic_pointer_cast<WECore::ModulationSource<double>>(sourceProvider));
    }

    bool removeSourceFromStepSeqDepth(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int seqIndex, ModulationSourceDefinition source) {
        if (sources->stepSequencers.size() <= seqIndex) {
            return false;
        }

        std::vector<WECore::ModulationSourceWrapper<double>> existingSources = sources->stepSequencers[seqIndex]->getDepthModulationSources();
        for (const auto& existingSource : existingSources) {
            auto thisSource = std::dynamic_pointer_cast<ModulationSourceProvider>(existingSource.source);
            if (thisSource != nullptr && thisSource->definition == source) {
                return sources->stepSequencers[seqIndex]->removeDepthModulationSource(existingSource.source);
            }
        }

        return false;
    }

    bool setStepSeqDepthModulationAmount(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int seqIndex, int sourceIndex, double val) {
        if (sources->stepSequencers.size() > seqIndex) {
            return sources->stepSequencers[seqIndex]->setDepthModulationAmount(sourceIndex, val);
        }

        return false;
    }

    std::vector<std::shared_ptr<PluginParameterModulationSource>> getStepSeqDepthModulationSources(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int seqIndex) {
        // Use std::shared_ptr to be consistent with usage in PluginParameterModulationConfig
        std::vector<std::shared_ptr<PluginParameterModulationSource>> retVal;

        if (sources->stepSequencers.size() > seqIndex) {
            for (const auto& source : sources->stepSequencers[seqIndex]->getDepthModulationSources()) {
                auto thisSource = std::dynamic_pointer_cast<ModulationSourceProvider>(source.source);
                retVal.emplace_back(new PluginParameterModulationSource(thisSource->definition, source.amount));
            }
        }

        return retVal;
    }

    bool setStepSeqTempoSyncSwitch(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int seqIndex, bool val) {
        if (sources->stepSequencers.size() > seqIndex) {
            sources->stepSequencers[seqIndex]->setTempoSyncSwitch(val);
            return true;
        }
        return false;
    }

    bool setStepSeqTempoNumer(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int seqIndex, int val) {
        if (sources->stepSequencers.size() > seqIndex) {
            sources->stepSequencers[seqIndex]->setTempoNumer(val);
            return true;
        }
        return false;
    }

    bool setStepSeqTempoDenom(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int seqIndex, int val) {
        if (sources->stepSequencers.size() > seqIndex) {
            sources->stepSequencers[seqIndex]->setTempoDenom(val);
            return true;
        }
        return false;
    }

    bool setStepSeqFreq(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int seqIndex, double val) {
        if (sources->stepSequencers.size() > seqIndex) {
            sources->stepSequencers[seqIndex]->setFreq(val);
            return true;
        }
        return false;
    }

    bool setStepSeqDepth(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int seqIndex, double val) {
        if (sources->stepSequencers.size() > seqIndex) {
            sources->stepSequencers[seqIndex]->setDepth(val);
            return true;
        }
        return false;
    }

    bool addStepSeqStep(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int seqIndex, int patternIndex) {
        if (sources->stepSequencers.size() > seqIndex) {
            sources->stepSequencers[seqIndex]->addStep(patternIndex);
            return true;
        }
        return false;
    }

    bool removeStepSeqStep(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int seqIndex, int patternIndex) {
        if (sources->stepSequencers.size() > seqIndex) {
            sources->stepSequencers[seqIndex]->removeStep(patternIndex);
            return true;
        }
        return false;
    }

    int getStepSeqNumSteps(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int seqIndex, int patternIndex) {
        if (sources->stepSequencers.size() > seqIndex) {
            return sources->stepSequencers[seqIndex]->getNumSteps(patternIndex);
        }
        return 0;
    }

    bool setStepSeqStepValue(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int seqIndex, int patternIndex, int stepIndex, double val) {
        if (sources->stepSequencers.size() > seqIndex) {
            sources->stepSequencers[seqIndex]->setStepValue(patternIndex, stepIndex, val);
            return true;
        }
        return false;
    }

    bool setStepSeqStepShape(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int seqIndex, int patternIndex, int stepIndex, int shape) {
        if (sources->stepSequencers.size() > seqIndex) {
            sources->stepSequencers[seqIndex]->setStepShape(patternIndex, stepIndex, static_cast<WECore::StepSeq::StepShape>(shape));
            return true;
        }
        return false;
    }

    bool setStepSeqStepReverse(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int seqIndex, int patternIndex, int stepIndex, bool val) {
        if (sources->stepSequencers.size() > seqIndex) {
            sources->stepSequencers[seqIndex]->setStepReverse(patternIndex, stepIndex, val);
            return true;
        }
        return false;
    }

    bool setStepSeqStepRepeat(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int seqIndex, int patternIndex, int stepIndex, int val) {
        if (sources->stepSequencers.size() > seqIndex) {
            sources->stepSequencers[seqIndex]->setStepRepeat(patternIndex, stepIndex, val);
            return true;
        }
        return false;
    }

    bool setStepSeqStepLengthMultiplier(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int seqIndex, int patternIndex, int stepIndex, double val) {
        if (sources->stepSequencers.size() > seqIndex) {
            sources->stepSequencers[seqIndex]->setStepLengthMultiplier(patternIndex, stepIndex, val);
            return true;
        }
        return false;
    }

    bool getStepSeqTempoSyncSwitch(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int seqIndex) {
        if (sources->stepSequencers.size() > seqIndex) {
            return sources->stepSequencers[seqIndex]->getTempoSyncSwitch();
        }
        return false;
    }

    int getStepSeqTempoNumer(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int seqIndex) {
        if (sources->stepSequencers.size() > seqIndex) {
            return sources->stepSequencers[seqIndex]->getTempoNumer();
        }
        return 0;
    }

    int getStepSeqTempoDenom(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int seqIndex) {
        if (sources->stepSequencers.size() > seqIndex) {
            return sources->stepSequencers[seqIndex]->getTempoDenom();
        }
        return 0;
    }

    double getStepSeqFreq(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int seqIndex) {
        if (sources->stepSequencers.size() > seqIndex) {
            return sources->stepSequencers[seqIndex]->getFreq();
        }
        return 0;
    }

    double getStepSeqDepth(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int seqIndex) {
        if (sources->stepSequencers.size() > seqIndex) {
            return sources->stepSequencers[seqIndex]->getDepth();
        }
        return 0;
    }

    double getStepSeqStepValue(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int seqIndex, int patternIndex, int stepIndex) {
        if (sources->stepSequencers.size() > seqIndex) {
            return sources->stepSequencers[seqIndex]->getStepValue(patternIndex, stepIndex);
        }
        return 0;
    }

    int getStepSeqStepShape(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int seqIndex, int patternIndex, int stepIndex) {
        if (sources->stepSequencers.size() > seqIndex) {
            return static_cast<int>(sources->stepSequencers[seqIndex]->getStepShape(patternIndex, stepIndex));
        }
        return 0;
    }

    bool getStepSeqStepReverse(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int seqIndex, int patternIndex, int stepIndex) {
        if (sources->stepSequencers.size() > seqIndex) {
            return sources->stepSequencers[seqIndex]->getStepReverse(patternIndex, stepIndex);
        }
        return false;
    }

    int getStepSeqStepRepeat(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int seqIndex, int patternIndex, int stepIndex) {
        if (sources->stepSequencers.size() > seqIndex) {
            return sources->stepSequencers[seqIndex]->getStepRepeat(patternIndex, stepIndex);
        }
        return 1;
    }

    double getStepSeqStepLengthMultiplier(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int seqIndex, int patternIndex, int stepIndex) {
        if (sources->stepSequencers.size() > seqIndex) {
            return sources->stepSequencers[seqIndex]->getStepLengthMultiplier(patternIndex, stepIndex);
        }
        return 1.0;
    }

    double getStepSeqModulatedFreqValue(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int seqIndex) {
        if (sources->stepSequencers.size() > seqIndex) {
            return sources->stepSequencers[seqIndex]->getModulatedFreqValue();
        }

        return 0;
    }

    double getStepSeqModulatedDepthValue(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int seqIndex) {
        if (sources->stepSequencers.size() > seqIndex) {
            return sources->stepSequencers[seqIndex]->getModulatedDepthValue();
        }

        return 0;
    }

    double getStepSeqLastOutput(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int seqIndex) {
        if (sources->stepSequencers.size() > seqIndex) {
            return sources->stepSequencers[seqIndex]->getLastOutput();
        }
        return 0;
    }

    int getStepSeqCurrentStep(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int seqIndex) {
        if (sources->stepSequencers.size() > seqIndex) {
            return sources->stepSequencers[seqIndex]->getCurrentStep();
        }
        return -1;
    }

    bool removeModulationSource(ModelInterface::ModulationSourcesState& state, ModulationSourceDefinition definition) {
        // First remove/renumber any modulation sources that reference this one
        for (std::shared_ptr<ModelInterface::CloneableLFO> lfo : state.lfos) {
            // Freq
            std::vector<WECore::ModulationSourceWrapper<double>> lfoFreqSources = lfo->getFreqModulationSources();
            lfoFreqSources = deleteSourceFromTargetSources(lfoFreqSources, definition, state.getModulationValueCallback);
            lfo->setFreqModulationSources(lfoFreqSources);

            // Depth
            std::vector<WECore::ModulationSourceWrapper<double>> lfoDepthSources = lfo->getDepthModulationSources();
            lfoDepthSources = deleteSourceFromTargetSources(lfoDepthSources, definition, state.getModulationValueCallback);
            lfo->setDepthModulationSources(lfoDepthSources);

            // Phase
            std::vector<WECore::ModulationSourceWrapper<double>> lfoPhaseSources = lfo->getPhaseModulationSources();
            lfoPhaseSources = deleteSourceFromTargetSources(lfoPhaseSources, definition, state.getModulationValueCallback);
            lfo->setPhaseModulationSources(lfoPhaseSources);
        }

        for (std::shared_ptr<WECore::Perlin::PerlinSource> random : state.randomSources) {
            // Freq
            std::vector<WECore::ModulationSourceWrapper<double>> randomFreqSources = random->getFreqModulationSources();
            randomFreqSources = deleteSourceFromTargetSources(randomFreqSources, definition, state.getModulationValueCallback);
            random->setFreqModulationSources(randomFreqSources);

            // Depth
            std::vector<WECore::ModulationSourceWrapper<double>> randomDepthSources = random->getDepthModulationSources();
            randomDepthSources = deleteSourceFromTargetSources(randomDepthSources, definition, state.getModulationValueCallback);
            random->setDepthModulationSources(randomDepthSources);
        }

        for (std::shared_ptr<WECore::StepSeq::StepSequencer> seq : state.stepSequencers) {
            // Freq
            std::vector<WECore::ModulationSourceWrapper<double>> seqFreqSources = seq->getFreqModulationSources();
            seqFreqSources = deleteSourceFromTargetSources(seqFreqSources, definition, state.getModulationValueCallback);
            seq->setFreqModulationSources(seqFreqSources);

            // Depth
            std::vector<WECore::ModulationSourceWrapper<double>> seqDepthSources = seq->getDepthModulationSources();
            seqDepthSources = deleteSourceFromTargetSources(seqDepthSources, definition, state.getModulationValueCallback);
            seq->setDepthModulationSources(seqDepthSources);
        }

        const int index {definition.id - 1};
        if (definition.type == MODULATION_TYPE::LFO) {
            if (state.lfos.size() > index) {
                state.lfos.erase(state.lfos.begin() + index);
                return true;
            }
        } else if (definition.type == MODULATION_TYPE::ENVELOPE) {
            if (state.envelopes.size() > index) {
                state.envelopes.erase(state.envelopes.begin() + index);
                return true;
            }
        } else if (definition.type == MODULATION_TYPE::RANDOM) {
            if (state.randomSources.size() > index) {
                state.randomSources.erase(state.randomSources.begin() + index);
                return true;
            }
        } else if (definition.type == MODULATION_TYPE::STEP_SEQUENCER) {
            if (state.stepSequencers.size() > index) {
                state.stepSequencers.erase(state.stepSequencers.begin() + index);
                return true;
            }
        }

        return false;
    }
}

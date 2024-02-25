#include "ModulationMutators.hpp"

namespace ModulationMutators {
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

    double getLfoDepth(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex) {
        if (sources->lfos.size() > lfoIndex) {
            return sources->lfos[lfoIndex]->getDepth();
        }

        return 0;
    }

    double getLfoManualPhase(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex) {
        if (sources->lfos.size() > lfoIndex) {
            return sources->lfos[lfoIndex]->getManualPhase();
        }

        return 0;
    }

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

    bool removeModulationSource(ModelInterface::ModulationSourcesState& state, ModulationSourceDefinition definition) {
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
        }

        return false;
    }
}

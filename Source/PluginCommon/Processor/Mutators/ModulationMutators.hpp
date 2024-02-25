#pragma once

#include "DataModelInterface.hpp"

namespace ModulationMutators {
    void addLfo(std::shared_ptr<ModelInterface::ModulationSourcesState> sources);
    bool setLfoTempoSyncSwitch(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex, bool val);
    bool setLfoInvertSwitch(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex, bool val);
    bool setLfoWave(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex, int val);
    bool setLfoTempoNumer(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex, int val);
    bool setLfoTempoDenom(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex, int val);
    bool setLfoFreq(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex, double val);
    bool setLfoDepth(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex, double val);
    bool setLfoManualPhase(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex, double val);

    bool getLfoTempoSyncSwitch(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex);
    bool getLfoInvertSwitch(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex);
    int getLfoWave(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex);
    double getLfoTempoNumer(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex);
    double getLfoTempoDenom(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex);
    double getLfoFreq(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex);
    double getLfoDepth(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex);
    double getLfoManualPhase(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int lfoIndex);

    void addEnvelope(std::shared_ptr<ModelInterface::ModulationSourcesState> sources);
    bool setEnvAttackTimeMs(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int envIndex, double val);
    bool setEnvReleaseTimeMs(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int envIndex, double val);
    bool setEnvFilterEnabled(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int envIndex, bool val);
    bool setEnvFilterHz(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int envIndex, double lowCut, double highCut);
    bool setEnvAmount(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int envIndex, float val);
    bool setEnvUseSidechainInput(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int envIndex, bool val);

    double getEnvAttackTimeMs(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int envIndex);
    double getEnvReleaseTimeMs(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int envIndex);
    bool getEnvFilterEnabled(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int envIndex);
    double getEnvLowCutHz(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int envIndex);
    double getEnvHighCutHz(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int envIndex);
    float getEnvAmount(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int envIndex);
    bool getEnvUseSidechainInput(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int envIndex);
    double getEnvLastOutput(std::shared_ptr<ModelInterface::ModulationSourcesState> sources, int envIndex);

    bool removeModulationSource(ModelInterface::ModulationSourcesState& state, ModulationSourceDefinition definition);
}

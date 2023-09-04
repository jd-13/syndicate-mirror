#include "ModulationMutators.hpp"

namespace ModulationMutators {
    void addLfo(ModulationInterface::ModulationSourcesState& state) {
        std::shared_ptr<WECore::Richter::RichterLFO> newLfo {new WECore::Richter::RichterLFO()};
        newLfo->setBypassSwitch(true);
        newLfo->setSampleRate(state.hostConfig.sampleRate);
        state.lfos.push_back(newLfo);
    }

    void addEnvelope(ModulationInterface::ModulationSourcesState& state) {
        std::shared_ptr<ModulationInterface::EnvelopeWrapper> newEnv {new ModulationInterface::EnvelopeWrapper()};
        newEnv->envelope.setSampleRate(state.hostConfig.sampleRate);
        state.envelopes.push_back(newEnv);
    }

    void removeModulationSource(ModulationInterface::ModulationSourcesState& state, ModulationSourceDefinition definition) {
        const int index {definition.id - 1};
        if (definition.type == MODULATION_TYPE::LFO) {
            if (state.lfos.size() > index) {
                state.lfos.erase(state.lfos.begin() + index);
            }
        } else if (definition.type == MODULATION_TYPE::ENVELOPE) {
            if (state.envelopes.size() > index) {
                state.envelopes.erase(state.envelopes.begin() + index);
            }
        }
    }
}

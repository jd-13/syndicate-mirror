#pragma once

#include "DataModelInterface.hpp"

namespace ModulationMutators {
    void addLfo(ModulationInterface::ModulationSourcesState& state);
    void addEnvelope(ModulationInterface::ModulationSourcesState& state);
    void removeModulationSource(ModulationInterface::ModulationSourcesState& state, ModulationSourceDefinition definition);
}

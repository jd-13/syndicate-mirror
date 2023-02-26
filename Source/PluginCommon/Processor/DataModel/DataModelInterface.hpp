#pragma once

#include "PluginSplitter.hpp"
#include "General/AudioSpinMutex.h"
#include "SplitterProcessors.hpp"

namespace SplitterInterface {
    struct Splitter {
        // Internal representation of the data model
        std::shared_ptr<PluginSplitter> splitter;

        // This mutex must be locked by all mutators before attempting to read or write to or from
        // data model. Its purpose is to stop a mutator being called on one thread from changing the
        // data model in a way that would crash a mutator being called on another thread.
        //
        // Recursive because the application code may call something via the mutator forEach methods
        // that tries to take the lock on the same thread as the forEach method itself
        std::recursive_mutex mutatorsMutex;

        // This mutex must be locked by mutators which change the structure of the data model, and
        // also by the processors. Its purpose is to stop a mutator being called on one thread from
        // changing the data model in a way that would crash a processor being called on another
        // thread.
        //
        // Mutators reading from the data model or writing only primitive values don't need to lock
        // this
        WECore::AudioSpinMutex sharedMutex;

        // We store the crossover frequencies so they can be restored if the user switches from a
        // multiband split to another type and back again
        std::optional<std::vector<float>> cachedcrossoverFrequencies;

        Splitter(HostConfiguration config,
                 std::function<float(int, MODULATION_TYPE)> getModulationValueCallback,
                 std::function<void(int)> latencyChangeCallback) {
            splitter.reset(
                new PluginSplitterSeries(config, getModulationValueCallback, latencyChangeCallback)
            );

            SplitterProcessor::prepareToPlay(*splitter.get(), config.sampleRate, config.blockSize, config.layout);
        }
    };
}

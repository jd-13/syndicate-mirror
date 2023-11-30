#pragma once

#include <JuceHeader.h>

#include "ChainMutators.hpp"
#include "PluginChain.hpp"
#include "FFTProvider.hpp"
#include "CrossoverState.hpp"
#include "CrossoverMutators.hpp"
#include "CrossoverProcessors.hpp"

/**
 * Stores a plugin chain and any associated data.
 */
struct PluginChainWrapper {
    PluginChainWrapper(std::shared_ptr<PluginChain> newChain, bool newIsSoloed)
            : chain(newChain), isSoloed(newIsSoloed) {}

    std::shared_ptr<PluginChain> chain;
    bool isSoloed;
};

/**
 * Base class which provides the audio splitting functionality.
 *
 * Each derived class contains one or more plugin chains (one for each split).
 *
 * A splitter may contain more chains than it can actually use if they have been carried over from
 * a previous splitter that could handle more. In this case its processBlock will just ignore the
 * extra chains.
 */
class PluginSplitter {
public:
    std::vector<PluginChainWrapper> chains;
    size_t numChainsSoloed;
    HostConfiguration config;
    std::function<float(int, MODULATION_TYPE)> getModulationValueCallback;
    std::function<void(int)> notifyProcessorOnLatencyChange;

    PluginSplitter(int defaultNumChains,
                   HostConfiguration newConfig,
                   std::function<float(int, MODULATION_TYPE)> newGetModulationValueCallback,
                   std::function<void(int)> latencyChangeCallback)
                   : numChainsSoloed(0),
                     config(newConfig),
                     getModulationValueCallback(newGetModulationValueCallback),
                     notifyProcessorOnLatencyChange(latencyChangeCallback) {

        // Set up the default number of chains
        for (int idx {0}; idx < defaultNumChains; idx++) {
            chains.emplace_back(std::make_shared<PluginChain>(getModulationValueCallback), false);
            chains[chains.size() - 1].chain->latencyListener.setSplitter(this);
        }
        onLatencyChange();
    }

    PluginSplitter(std::shared_ptr<PluginSplitter> otherSplitter, int defaultNumChains)
                   : chains(otherSplitter->chains),
                     numChainsSoloed(otherSplitter->numChainsSoloed),
                     config(otherSplitter->config),
                     getModulationValueCallback(otherSplitter->getModulationValueCallback),
                     notifyProcessorOnLatencyChange(otherSplitter->notifyProcessorOnLatencyChange) {

        // Move the latency listeners for the existing chains to point to this splitter
        for (auto& chain : chains) {
            chain.chain->latencyListener.setSplitter(this);
        }

        // Add chains if we still need to reach the default
        while (defaultNumChains > chains.size()) {
            chains.emplace_back(std::make_shared<PluginChain>(getModulationValueCallback), false);
            chains[chains.size() - 1].chain->latencyListener.setSplitter(this);
        }
        onLatencyChange();
    }

    virtual ~PluginSplitter() = default;

    void onLatencyChange() {
        // The latency of the splitter is the latency of the slowest chain, so iterate through each
        // chain and report the highest latency
        int highestLatency {0};

        for (const PluginChainWrapper& chain : chains) {
            const int thisLatency {chain.chain->latencyListener.calculatedTotalPluginLatency};
            if (highestLatency < thisLatency) {
                highestLatency = thisLatency;
            }
        }

        // Tell each chain the latency of the slowest chain, so they can all add compensation to match
        // it
        for (PluginChainWrapper& chain : chains) {
            ChainMutators::setRequiredLatency(chain.chain, highestLatency, config);
        }

        notifyProcessorOnLatencyChange(highestLatency);
    }
};

/**
 * Contains a single plugin graph for plugins arranged in series.
 */
class PluginSplitterSeries : public PluginSplitter {
public:
    static constexpr int DEFAULT_NUM_CHAINS {1};

    PluginSplitterSeries(HostConfiguration newConfig,
                         std::function<float(int, MODULATION_TYPE)> getModulationValueCallback,
                         std::function<void(int)> latencyChangeCallback)
                         : PluginSplitter(DEFAULT_NUM_CHAINS, newConfig, getModulationValueCallback, latencyChangeCallback) {
        juce::Logger::writeToLog("Constructed PluginSplitterSeries");
    }

    PluginSplitterSeries(std::shared_ptr<PluginSplitter> otherSplitter)
                         : PluginSplitter(otherSplitter, DEFAULT_NUM_CHAINS) {
        juce::Logger::writeToLog("Converted to PluginSplitterSeries");

        // We only have one active chain in the series splitter, so it can't be muted or soloed
        ChainMutators::setChainMute(chains[0].chain, false);
    }
};

/**
 * Contains a single plugin graph for plugins arranged in parallel.
 */
class PluginSplitterParallel : public PluginSplitter {
public:
    static constexpr int DEFAULT_NUM_CHAINS {1};

    std::unique_ptr<juce::AudioBuffer<float>> inputBuffer;
    std::unique_ptr<juce::AudioBuffer<float>> outputBuffer;

    PluginSplitterParallel(HostConfiguration newConfig,
                           std::function<float(int, MODULATION_TYPE)> getModulationValueCallback,
                           std::function<void(int)> latencyChangeCallback)
                           : PluginSplitter(DEFAULT_NUM_CHAINS, newConfig, getModulationValueCallback, latencyChangeCallback) {
        juce::Logger::writeToLog("Constructed PluginSplitterParallel");
    }

    PluginSplitterParallel(std::shared_ptr<PluginSplitter> otherSplitter)
                           : PluginSplitter(otherSplitter, DEFAULT_NUM_CHAINS) {
        juce::Logger::writeToLog("Converted to PluginSplitterParallel");
    }
};

/**
 * Contains a single plugin graph for plugins arranged in a multiband split.
 */
class PluginSplitterMultiband : public PluginSplitter {
public:
    static constexpr int DEFAULT_NUM_CHAINS {2};

    std::shared_ptr<CrossoverState> crossover;
    FFTProvider fftProvider;

    PluginSplitterMultiband(HostConfiguration newConfig,
                            std::function<float(int, MODULATION_TYPE)> getModulationValueCallback,
                            std::function<void(int)> latencyChangeCallback)
                            : PluginSplitter(DEFAULT_NUM_CHAINS, newConfig, getModulationValueCallback, latencyChangeCallback),
                              crossover(createDefaultCrossoverState(config)) {
        juce::Logger::writeToLog("Constructed PluginSplitterMultiband");

        CrossoverProcessors::prepareToPlay(*crossover.get(), config.sampleRate, config.blockSize, config.layout);
    }

    PluginSplitterMultiband(std::shared_ptr<PluginSplitter> otherSplitter, std::optional<std::vector<float>> crossoverFrequencies)
                            : PluginSplitter(otherSplitter, DEFAULT_NUM_CHAINS),
                              crossover(createDefaultCrossoverState(config)) {
        juce::Logger::writeToLog("Converted to PluginSplitterMultiband");

        // Set the crossover to have the correct number of bands (this will also default the frequencies)
        while (chains.size() > CrossoverMutators::getNumBands(crossover)) {
            CrossoverMutators::addBand(crossover);
        }

        // Restore the crossover frequencies if there are previous ones
        if (crossoverFrequencies.has_value()) {
            const size_t numCrossovers {std::min(crossoverFrequencies.value().size(), CrossoverMutators::getNumBands(crossover))};
            for (int index {0}; index < numCrossovers; index++) {
                CrossoverMutators::setCrossoverFrequency(crossover, index, crossoverFrequencies.value()[index]);
            }
        }

        // Set the processors
        for (size_t bandIndex {0}; bandIndex < CrossoverMutators::getNumBands(crossover); bandIndex++) {
            std::shared_ptr<PluginChain> newChain {chains[bandIndex].chain};
            CrossoverMutators::setPluginChain(crossover, bandIndex, newChain);
            CrossoverMutators::setIsSoloed(crossover, bandIndex, chains[bandIndex].isSoloed);
        }

        CrossoverProcessors::prepareToPlay(*crossover.get(), config.sampleRate, config.blockSize, config.layout);
    }
};

/**
 * Contains a single plugin graph for plugins arranged in a left right split.
 */
class PluginSplitterLeftRight : public PluginSplitter {
public:
    static constexpr int DEFAULT_NUM_CHAINS {2};

    std::unique_ptr<juce::AudioBuffer<float>> leftBuffer;
    std::unique_ptr<juce::AudioBuffer<float>> rightBuffer;

    PluginSplitterLeftRight(HostConfiguration newConfig,
                            std::function<float(int, MODULATION_TYPE)> getModulationValueCallback,
                            std::function<void(int)> latencyChangeCallback)
                            : PluginSplitter(DEFAULT_NUM_CHAINS, newConfig, getModulationValueCallback, latencyChangeCallback) {
        juce::Logger::writeToLog("Constructed PluginSplitterLeftRight");
    }

    PluginSplitterLeftRight(std::shared_ptr<PluginSplitter> otherSplitter)
                            : PluginSplitter(otherSplitter, DEFAULT_NUM_CHAINS) {
        juce::Logger::writeToLog("Converted to PluginSplitterLeftRight");
    }
};

/**
 * Contains a single plugin graph for plugins arranged in a mid side split.
 */
class PluginSplitterMidSide : public PluginSplitter {
public:
    static constexpr int DEFAULT_NUM_CHAINS {2};

    std::unique_ptr<juce::AudioBuffer<float>> midBuffer;
    std::unique_ptr<juce::AudioBuffer<float>> sideBuffer;

    PluginSplitterMidSide(HostConfiguration newConfig,
                          std::function<float(int, MODULATION_TYPE)> getModulationValueCallback,
                          std::function<void(int)> latencyChangeCallback)
                          : PluginSplitter(DEFAULT_NUM_CHAINS, newConfig, getModulationValueCallback, latencyChangeCallback) {
        juce::Logger::writeToLog("Constructed PluginSplitterMidSide");
    }

    PluginSplitterMidSide(std::shared_ptr<PluginSplitter> otherSplitter)
                          : PluginSplitter(otherSplitter, DEFAULT_NUM_CHAINS) {
        juce::Logger::writeToLog("Converted to PluginSplitterMidSide");
    }
};

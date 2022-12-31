#include "SplitterMutators.hpp"
#include "ChainMutators.hpp"
#include "ChainProcessors.hpp"
#include "MONSTRFilters/MONSTRParameters.h"

namespace SplitterMutators {
    bool insertPlugin(std::shared_ptr<PluginSplitter> splitter, std::shared_ptr<juce::AudioPluginInstance> plugin, int chainNumber, int positionInChain) {
        if (splitter->chains.size() > chainNumber) {
            ChainMutators::insertPlugin(splitter->chains[chainNumber].chain, plugin, positionInChain);
            return true;
        }

        return false;
    }

    bool replacePlugin(std::shared_ptr<PluginSplitter> splitter, std::shared_ptr<juce::AudioPluginInstance> plugin, int chainNumber, int positionInChain) {
        if (splitter->chains.size() > chainNumber) {
            ChainMutators::replacePlugin(splitter->chains[chainNumber].chain, plugin, positionInChain);
            return true;
        }

        return false;
    }

    bool removeSlot(std::shared_ptr<PluginSplitter> splitter, int chainNumber, int positionInChain) {
        if (splitter->chains.size() > chainNumber) {
            return ChainMutators::removeSlot(splitter->chains[chainNumber].chain, positionInChain);
        }

        return false;
    }

    bool insertGainStage(std::shared_ptr<PluginSplitter> splitter, int chainNumber, int positionInChain) {
        if (splitter->chains.size() > chainNumber) {
            ChainMutators::insertGainStage(splitter->chains[chainNumber].chain, positionInChain, splitter->config);
            return true;
        }

        return false;
    }

    std::shared_ptr<juce::AudioPluginInstance> getPlugin(std::shared_ptr<PluginSplitter> splitter, int chainNumber, int positionInChain) {
        if (splitter->chains.size() > chainNumber) {
            return ChainMutators::getPlugin(splitter->chains[chainNumber].chain, positionInChain);
        }

        return nullptr;
    }

    bool setPluginModulationConfig(std::shared_ptr<PluginSplitter> splitter, PluginModulationConfig config, int chainNumber, int positionInChain) {
        if (chainNumber < splitter->chains.size()) {
            return ChainMutators::setPluginModulationConfig(splitter->chains[chainNumber].chain, config, positionInChain);
        }

        return false;
    }

    PluginModulationConfig getPluginModulationConfig(std::shared_ptr<PluginSplitter> splitter, int chainNumber, int positionInChain) {
        if (splitter->chains.size() > chainNumber) {
            return ChainMutators::getPluginModulationConfig(splitter->chains[chainNumber].chain, positionInChain);
        }

        return PluginModulationConfig();
    }

    void setSlotBypass(std::shared_ptr<PluginSplitter> splitter, int chainNumber, int positionInChain, bool isBypassed) {
        if (splitter->chains.size() > chainNumber) {
            ChainMutators::setSlotBypass(splitter->chains[chainNumber].chain, positionInChain, isBypassed);
        }
    }

    bool getSlotBypass(std::shared_ptr<PluginSplitter> splitter, int chainNumber, int positionInChain) {
        if (splitter->chains.size() > chainNumber) {
            return ChainMutators::getSlotBypass(splitter->chains[chainNumber].chain, positionInChain);
        }

        return false;
    }

    void setChainSolo(std::shared_ptr<PluginSplitter> splitter, int chainNumber, bool val) {
        // The multiband crossover can handle soloed bands, so let it do that first
       if (auto multibandSplitter = std::dynamic_pointer_cast<PluginSplitterMultiband>(splitter)) {
           multibandSplitter->crossover.setIsSoloed(chainNumber, val);
       }

        if (chainNumber < splitter->chains.size()) {
            // If the new value is different to the existing one, update it and the counter
            if (val != splitter->chains[chainNumber].isSoloed) {

                splitter->chains[chainNumber].isSoloed = val;

                if (val) {
                    splitter->numChainsSoloed++;
                } else {
                    splitter->numChainsSoloed--;
                }
            }
        }
    }

    bool getChainSolo(std::shared_ptr<PluginSplitter> splitter, int chainNumber) {
        if (auto multibandSplitter = std::dynamic_pointer_cast<PluginSplitterMultiband>(splitter)) {
            return multibandSplitter->crossover.getIsSoloed(chainNumber);
        }

        if (chainNumber < splitter->chains.size()) {
            return splitter->chains[chainNumber].isSoloed;
        }

        return false;
    }

    bool setGainLinear(std::shared_ptr<PluginSplitter> splitter, int chainNumber, int positionInChain, float gain) {
        if (chainNumber < splitter->chains.size()) {
            return ChainMutators::setGainLinear(splitter->chains[chainNumber].chain, positionInChain, gain);
        }

        return false;
    }

    float getGainLinear(std::shared_ptr<PluginSplitter> splitter, int chainNumber, int positionInChain) {
        if (chainNumber < splitter->chains.size()) {
            return ChainMutators::getGainLinear(splitter->chains[chainNumber].chain, positionInChain);
        }

        return 0.0f;
    }

    std::optional<GainStageLevelsInterface> getGainStageLevelsInterface(std::shared_ptr<PluginSplitter> splitter, int chainNumber, int positionInChain) {
        if (chainNumber < splitter->chains.size()) {
            return ChainMutators::getGainStageLevelsInterface(splitter->chains[chainNumber].chain, positionInChain);
        }

        return std::optional<GainStageLevelsInterface>();
    }

    bool setPan(std::shared_ptr<PluginSplitter> splitter, int chainNumber, int positionInChain, float pan) {
        if (chainNumber < splitter->chains.size()) {
            return ChainMutators::setPan(splitter->chains[chainNumber].chain, positionInChain, pan);
        }

        return false;
    }

    float getPan(std::shared_ptr<PluginSplitter> splitter, int chainNumber, int positionInChain) {
        if (chainNumber < splitter->chains.size()) {
            return ChainMutators::getPan(splitter->chains[chainNumber].chain, positionInChain);
        }

        return 0.0f;
    }

    std::shared_ptr<PluginEditorBounds> getPluginEditorBounds(std::shared_ptr<PluginSplitter> splitter, int chainNumber, int positionInChain) {
        if (chainNumber < splitter->chains.size()) {
            return ChainMutators::getPluginEditorBounds(splitter->chains[chainNumber].chain, positionInChain);
        }

        return std::make_shared<PluginEditorBounds>();
    }

    SPLIT_TYPE getSplitType(const std::shared_ptr<PluginSplitter> splitter) {
        if (std::dynamic_pointer_cast<PluginSplitterSeries>(splitter)) {
            return SPLIT_TYPE::SERIES;
        }

        if (std::dynamic_pointer_cast<PluginSplitterParallel>(splitter)) {
            return SPLIT_TYPE::PARALLEL;
        }

        if (std::dynamic_pointer_cast<PluginSplitterMultiband>(splitter)) {
            return SPLIT_TYPE::MULTIBAND;
        }

        if (std::dynamic_pointer_cast<PluginSplitterLeftRight>(splitter)) {
            return SPLIT_TYPE::LEFTRIGHT;
        }

        if (std::dynamic_pointer_cast<PluginSplitterMidSide>(splitter)) {
            return SPLIT_TYPE::MIDSIDE;
        }

        return SPLIT_TYPE::SERIES;
    }

    bool addChain(std::shared_ptr<PluginSplitterParallel> splitter) {
        // Number of parallel chains is limited to the same as the crossover to avoid weird edge cases
        // when switching bands
        if (splitter->chains.size() < WECore::MONSTR::Parameters::NUM_BANDS.maxValue) {
            splitter->chains.emplace_back(std::make_shared<PluginChain>(splitter->getModulationValueCallback), false);
            ChainProcessor::prepareToPlay(*(splitter->chains[splitter->chains.size() - 1].chain.get()), splitter->config);
            splitter->chains[splitter->chains.size() - 1].chain->latencyListener.setSplitter(splitter.get());

            splitter->onLatencyChange();
            return true;
        }

        return false;
    }

    bool removeChain(std::shared_ptr<PluginSplitterParallel> splitter, int chainNumber) {
        if (splitter->chains.size() > 1 && chainNumber < splitter->chains.size()) {
            splitter->chains[chainNumber].chain->latencyListener.removeSplitter();
            splitter->chains.erase(splitter->chains.begin() + chainNumber);

            splitter->onLatencyChange();
            return true;
        }

        return false;
    }

    bool addBand(std::shared_ptr<PluginSplitterMultiband> splitter) {
        if (splitter->crossover.getNumBands() < WECore::MONSTR::Parameters::NUM_BANDS.maxValue) {
            // Create the chain first, then add the band and set the processor
            splitter->chains.emplace_back(std::make_unique<PluginChain>(splitter->getModulationValueCallback), false);
            splitter->crossover.addBand();

            PluginChain* newChain {splitter->chains[splitter->chains.size() - 1].chain.get()};
            ChainProcessor::prepareToPlay(*newChain, splitter->config);
            splitter->crossover.setPluginChain(splitter->crossover.getNumBands() - 1, newChain);

            newChain->latencyListener.setSplitter(splitter.get());
            splitter->onLatencyChange();
            return true;
        }

        return false;
    }

    bool removeBand(std::shared_ptr<PluginSplitterMultiband> splitter) {
        if (splitter->crossover.getNumBands() > WECore::MONSTR::Parameters::NUM_BANDS.minValue) {
            // Remove the band first, then the chain
            splitter->crossover.removeBand();
            splitter->chains[splitter->chains.size() - 1].chain->latencyListener.removeSplitter();
            splitter->chains.erase(splitter->chains.begin() + splitter->chains.size() - 1);

            splitter->onLatencyChange();
            return true;
        }

        return false;
    }

    void setCrossoverFrequency(std::shared_ptr<PluginSplitterMultiband> splitter, size_t index, double val) {
        splitter->crossover.setCrossoverFrequency(index, val);
    }

    double getCrossoverFrequency(std::shared_ptr<PluginSplitterMultiband> splitter, size_t index) {
        return splitter->crossover.getCrossoverFrequency(index);
    }
}

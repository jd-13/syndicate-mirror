#pragma once

#include <JuceHeader.h>
#include "PluginSplitter.hpp"
#include "SplitTypes.hpp"

namespace SplitterMutators {
    // PluginSplitter
    bool insertPlugin(std::shared_ptr<PluginSplitter> splitter, std::shared_ptr<juce::AudioPluginInstance> plugin, int chainNumber, int positionInChain);
    bool replacePlugin(std::shared_ptr<PluginSplitter> splitter, std::shared_ptr<juce::AudioPluginInstance> plugin, int chainNumber, int positionInChain);
    bool removeSlot(std::shared_ptr<PluginSplitter> splitter, int chainNumber, int positionInChain);
    bool insertGainStage(std::shared_ptr<PluginSplitter> splitter, int chainNumber, int positionInChain);

    std::shared_ptr<juce::AudioPluginInstance> getPlugin(std::shared_ptr<PluginSplitter> splitter, int chainNumber, int positionInChain);

    bool setPluginModulationConfig(std::shared_ptr<PluginSplitter> splitter, PluginModulationConfig config, int chainNumber, int positionInChain);
    PluginModulationConfig getPluginModulationConfig(std::shared_ptr<PluginSplitter> splitter, int chainNumber, int positionInChain);

    inline size_t getNumChains(std::shared_ptr<PluginSplitter> splitter) { return splitter->chains.size(); }

    void setSlotBypass(std::shared_ptr<PluginSplitter> splitter, int chainNumber, int positionInChain, bool isBypassed);
    bool getSlotBypass(std::shared_ptr<PluginSplitter> splitter, int chainNumber, int positionInChain);

    void setChainSolo(std::shared_ptr<PluginSplitter> splitter, int chainNumber, bool val);
    bool getChainSolo(std::shared_ptr<PluginSplitter> splitter, int chainNumber);

    bool setGainLinear(std::shared_ptr<PluginSplitter> splitter, int chainNumber, int positionInChain, float gain);
    float getGainLinear(std::shared_ptr<PluginSplitter> splitter, int chainNumber, int positionInChain);
    std::optional<GainStageLevelsInterface> getGainStageLevelsInterface(std::shared_ptr<PluginSplitter> splitter, int chainNumber, int positionInChain);

    bool setPan(std::shared_ptr<PluginSplitter> splitter, int chainNumber, int positionInChain, float pan);
    float getPan(std::shared_ptr<PluginSplitter> splitter, int chainNumber, int positionInChain);

    std::shared_ptr<PluginEditorBounds> getPluginEditorBounds(std::shared_ptr<PluginSplitter> splitter, int chainNumber, int positionInChain);

    SPLIT_TYPE getSplitType(const std::shared_ptr<PluginSplitter> splitter);

    // PluginSplitterParallel
    void addChain(std::shared_ptr<PluginSplitterParallel> splitter);
    bool removeChain(std::shared_ptr<PluginSplitterParallel> splitter, int chainNumber);

    // PluginSplitterMultiband
    void addBand(std::shared_ptr<PluginSplitterMultiband> splitter);
    bool removeBand(std::shared_ptr<PluginSplitterMultiband> splitter, int bandNumber);
    size_t getNumBands(std::shared_ptr<PluginSplitterMultiband> splitter);
    void setCrossoverFrequency(std::shared_ptr<PluginSplitterMultiband> splitter, size_t index, double val);
    double getCrossoverFrequency(std::shared_ptr<PluginSplitterMultiband> splitter, size_t index);
}

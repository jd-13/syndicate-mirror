#pragma once

#include "DataModelInterface.hpp"

namespace SplitterInterface {
    bool setSplitType(Splitter& splitter, SPLIT_TYPE splitType, HostConfiguration config);
    SPLIT_TYPE getSplitType(Splitter& splitter);

    bool insertPlugin(Splitter& splitter, std::shared_ptr<juce::AudioPluginInstance> plugin, int chainNumber, int positionInChain);
    bool replacePlugin(Splitter& splitter, std::shared_ptr<juce::AudioPluginInstance> plugin, int chainNumber, int positionInChain);
    bool removeSlot(Splitter& splitter, int chainNumber, int positionInChain);
    bool insertGainStage(Splitter& splitter, int chainNumber, int positionInChain);

    std::shared_ptr<juce::AudioPluginInstance> getPlugin(Splitter& splitter, int chainNumber, int positionInChain);

    bool setGainLinear(Splitter& splitter, int chainNumber, int positionInChain, float gain);
    bool setPan(Splitter& splitter, int chainNumber, int positionInChain, float pan);
    std::tuple<float, float> getGainLinearAndPan(Splitter& splitter, int chainNumber, int positionInChain);

    // TODO change usage of this get/set pattern to make it more atomic
    bool setPluginModulationConfig(Splitter& splitter, PluginModulationConfig config, int chainNumber, int positionInChain);
    PluginModulationConfig getPluginModulationConfig(Splitter& splitter, int chainNumber, int positionInChain);
    void removeModulationSource(Splitter& splitter, ModulationSourceDefinition definition);

    void setSlotBypass(Splitter& splitter, int chainNumber, int positionInChain, bool isBypassed);
    bool getSlotBypass(Splitter& splitter, int chainNumber, int positionInChain);

    void setChainBypass(Splitter& splitter, int chainNumber, bool val);
    void setChainMute(Splitter& splitter, int chainNumber, bool val);
    void setChainSolo(Splitter& splitter, int chainNumber, bool val);
    bool getChainBypass(Splitter& splitter, int chainNumber);
    bool getChainMute(Splitter& splitter, int chainNumber);
    bool getChainSolo(Splitter& splitter, int chainNumber);

    size_t getNumChains(Splitter& splitter);

    bool addParallelChain(Splitter& splitter);
    bool removeParallelChain(Splitter& splitter, int chainNumber);

    void addCrossoverBand(Splitter& splitter);
    bool removeCrossoverBand(Splitter& splitter, int bandNumber);
    bool setCrossoverFrequency(Splitter& splitter, size_t index, float val);
    float getCrossoverFrequency(Splitter& splitter, size_t index);

    std::pair<std::array<float, FFTProvider::NUM_OUTPUTS>, float> getFFTOutputs(Splitter& splitter);

    std::shared_ptr<PluginEditorBounds> getPluginEditorBounds(Splitter& splitter, int chainNumber, int positionInChain);

    std::optional<GainStageLevelsInterface> getGainStageLevelsInterface(Splitter& splitter, int chainNumber, int positionInChain);

    void forEachChain(Splitter& splitter, std::function<void(int, std::shared_ptr<PluginChain>)> callback);
    void forEachCrossover(Splitter& splitter, std::function<void(float)> callback);

    void writeToXml(Splitter& splitter, juce::XmlElement* element);
    void restoreFromXml(
        Splitter& splitter, juce::XmlElement* element,
        std::function<float(int, MODULATION_TYPE)> getModulationValueCallback,
        std::function<void(int)> latencyChangeCallback,
        HostConfiguration config,
        const PluginConfigurator& pluginConfigurator,
        std::function<void(juce::String)> onErrorCallback);
}

namespace ModulationInterface {
    void addLfo(ModulationSourcesState& state);
    void addEnvelope(ModulationSourcesState& state);
    void removeModulationSource(ModulationSourcesState& state, ModulationSourceDefinition definition);

    std::shared_ptr<WECore::Richter::RichterLFO> getLfo(ModulationSourcesState& state, int lfoNumber);
    std::shared_ptr<EnvelopeWrapper> getEnvelope(ModulationSourcesState& state, int envelopeNumber);

    void forEachLfo(ModulationSourcesState& state, std::function<void(int)> callback);
    void forEachEnvelope(ModulationSourcesState& state, std::function<void(int)> callback);

    void writeToXml(ModulationSourcesState& state, juce::XmlElement* element);
    void restoreFromXml(ModulationSourcesState& state, juce::XmlElement* element, HostConfiguration config);
}
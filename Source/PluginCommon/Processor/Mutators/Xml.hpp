#pragma once

#include <JuceHeader.h>
#include "ChainSlots.hpp"

// TODO lock on entry so UI can't make changes

namespace XmlReader {
    bool XmlElementIsPlugin(juce::XmlElement* element);
    bool XmlElementIsGainStage(juce::XmlElement* element);

    std::unique_ptr<ChainSlotGainStage> restoreChainSlotGainStage(
        juce::XmlElement* element, const juce::AudioProcessor::BusesLayout& busesLayout);

    typedef std::function<
        std::tuple<std::unique_ptr<juce::AudioPluginInstance>, juce::String>(
            const juce::PluginDescription&, const HostConfiguration&)> LoadPluginFunction;

    std::unique_ptr<ChainSlotPlugin> restoreChainSlotPlugin(
            juce::XmlElement* element,
            std::function<float(int, MODULATION_TYPE)> getModulationValueCallback,
            HostConfiguration configuration,
            const PluginConfigurator& pluginConfigurator,
            LoadPluginFunction loadPlugin,
            std::function<void(juce::String)> onErrorCallback);
    std::unique_ptr<PluginModulationConfig> restorePluginModulationConfig(juce::XmlElement* element);
    std::unique_ptr<PluginParameterModulationConfig> restorePluginParameterModulationConfig(juce::XmlElement* element);
    std::unique_ptr<PluginParameterModulationSource> restorePluginParameterModulationSource(juce::XmlElement* element);
}

namespace XmlWriter {
    void write(std::shared_ptr<ChainSlotGainStage> gainStage, juce::XmlElement* element);

    void write(std::shared_ptr<ChainSlotPlugin> chainSlot, juce::XmlElement* element);
    void write(std::shared_ptr<PluginModulationConfig> config, juce::XmlElement* element);
    void write(std::shared_ptr<PluginParameterModulationConfig> config, juce::XmlElement* element);
    void write(std::shared_ptr<PluginParameterModulationSource> source, juce::XmlElement* element);
}

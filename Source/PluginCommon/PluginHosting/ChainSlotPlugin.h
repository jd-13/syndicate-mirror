#pragma once

#include <JuceHeader.h>

#include "ChainSlotBase.h"
#include "ModulationSourceDefinition.h"
#include "PluginConfigurator.h"

struct PluginParameterModulationSource {
    PluginParameterModulationSource() : definition(0, MODULATION_TYPE::MACRO), modulationAmount(0) { }

    PluginParameterModulationSource(ModulationSourceDefinition newDefinition,
                                    float newModulationAmount) :
            definition(newDefinition),
            modulationAmount(newModulationAmount) { }

    // Definition of the modulation source
    ModulationSourceDefinition definition;

    // Amount of modulation to be applied (-1 : 1)
    float modulationAmount;

    void restoreFromXml(juce::XmlElement* element);
    void writeToXml(juce::XmlElement* element);
};

struct PluginParameterModulationConfig {
    // Name of the parameter being modulated
    juce::String targetParameterName;

    // Parameter value without modulation applied (0 : 1)
    float restValue;

    // All the sources being provided for this parameter
    std::vector<PluginParameterModulationSource> sources;

    // Used when retrieving the parameter name from a juce::AudioProcessorParameter
    static constexpr int PLUGIN_PARAMETER_NAME_LENGTH_LIMIT {30};

    void restoreFromXml(juce::XmlElement* element);
    void writeToXml(juce::XmlElement* element);
};

struct PluginModulationConfig {
    bool isActive;
    std::vector<PluginParameterModulationConfig> parameterConfigs;

    PluginModulationConfig() : isActive(false) {}

    PluginModulationConfig& operator=(const PluginModulationConfig& other) {
        isActive = other.isActive;
        parameterConfigs = other.parameterConfigs;

        return *this;
    }

    void restoreFromXml(juce::XmlElement* element);
    void writeToXml(juce::XmlElement* element);
};

typedef std::optional<juce::Rectangle<int>> PluginEditorBounds;

/**
 * Represents a plugin in a slot in a processing chain.
 */
class ChainSlotPlugin : public ChainSlotBase {
public:
    std::shared_ptr<juce::AudioPluginInstance> plugin;
    PluginModulationConfig modulationConfig;
    std::shared_ptr<PluginEditorBounds> editorBounds;

    ChainSlotPlugin(std::shared_ptr<juce::AudioPluginInstance> newPlugin,
                    bool newIsBypassed,
                    std::function<float(int, MODULATION_TYPE)> getModulationValueCallback)
        : ChainSlotBase(newIsBypassed), plugin(newPlugin),
          _getModulationValueCallback(getModulationValueCallback),
          editorBounds(new PluginEditorBounds()) {}

    virtual ~ChainSlotPlugin() = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void reset() override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    static std::unique_ptr<ChainSlotPlugin> restoreFromXml(
        juce::XmlElement* element,
        std::function<float(int, MODULATION_TYPE)> getModulationValueCallback,
        HostConfiguration configuration,
        const PluginConfigurator& pluginConfigurator,
        std::function<void(juce::String)> onErrorCallback);
    void writeToXml(juce::XmlElement* element) override;

private:
    std::function<float(int, MODULATION_TYPE)> _getModulationValueCallback;

    void _applyModulationForParamter(juce::AudioProcessorParameter* targetParameter,
                                     const PluginParameterModulationConfig& parameterConfig);
};

#pragma once

#include <JuceHeader.h>
#include "WEFilters/AREnvelopeFollowerSquareLaw.h"
#include "ModulationSourceDefinition.hpp"
#include "PluginConfigurator.hpp"

struct ChainSlotBase {
    bool isBypassed;

    ChainSlotBase(bool newIsBypassed) : isBypassed(newIsBypassed) {}
    virtual ~ChainSlotBase() = default;
};

/**
 * Represents a gain stage in a slot in a processing chain.
 */
struct ChainSlotGainStage : ChainSlotBase {
    // Linear 0 to 1 (or a little more) values
    float gain;

    // -1 to 1 values
    float pan;

    int numMainChannels;
    std::array<WECore::AREnv::AREnvelopeFollowerSquareLaw, 2> meterEnvelopes;

    ChainSlotGainStage(float newGain, float newPan, bool newIsBypassed, const juce::AudioProcessor::BusesLayout& busesLayout)
        : ChainSlotBase(newIsBypassed), gain(newGain), pan(newPan), numMainChannels(busesLayout.getMainInputChannels()) {

        for (auto& env : meterEnvelopes) {
            env.setAttackTimeMs(1);
            env.setReleaseTimeMs(50);
            env.setFilterEnabled(false);
        }
    }

    virtual ~ChainSlotGainStage() = default;
};

/**
 * Provides an interface for the UI to access values needed to draw the level meter for this gain stage.
 */
class GainStageLevelsInterface {
public:
    explicit GainStageLevelsInterface(const std::shared_ptr<ChainSlotGainStage> gainStage) : _gainStage(gainStage) {}
    ~GainStageLevelsInterface() = default;

    int getNumChannels() const { return _gainStage->numMainChannels; }
    float getOutputAmplitude(int channel) const { return static_cast<float>(_gainStage->meterEnvelopes[channel].getLastOutput()); }

private:
    const std::shared_ptr<ChainSlotGainStage> _gainStage;
};

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
};

struct PluginParameterModulationConfig {
    // Name of the parameter being modulated
    juce::String targetParameterName;

    // Parameter value without modulation applied (0 : 1)
    float restValue;

    // All the sources being provided for this parameter
    std::vector<std::shared_ptr<PluginParameterModulationSource>> sources;

    // Used when retrieving the parameter name from a juce::AudioProcessorParameter
    static constexpr int PLUGIN_PARAMETER_NAME_LENGTH_LIMIT {30};
};

struct PluginModulationConfig {
    bool isActive;
    std::vector<std::shared_ptr<PluginParameterModulationConfig>> parameterConfigs;

    PluginModulationConfig() : isActive(false) {}

    PluginModulationConfig& operator=(const PluginModulationConfig& other) {
        isActive = other.isActive;
        parameterConfigs = other.parameterConfigs;

        return *this;
    }
};

typedef std::optional<juce::Rectangle<int>> PluginEditorBounds;

/**
 * Represents a plugin in a slot in a processing chain.
 */
struct ChainSlotPlugin : ChainSlotBase {
    std::shared_ptr<juce::AudioPluginInstance> plugin;
    std::shared_ptr<PluginModulationConfig> modulationConfig;
    std::function<float(int, MODULATION_TYPE)> getModulationValueCallback;
    std::shared_ptr<PluginEditorBounds> editorBounds;

    ChainSlotPlugin(std::shared_ptr<juce::AudioPluginInstance> newPlugin,
                    bool newIsBypassed,
                    std::function<float(int, MODULATION_TYPE)> newGetModulationValueCallback)
        : ChainSlotBase(newIsBypassed),
          plugin(newPlugin),
          modulationConfig(std::make_shared<PluginModulationConfig>()),
          getModulationValueCallback(newGetModulationValueCallback),
          editorBounds(new PluginEditorBounds()) {}

    virtual ~ChainSlotPlugin() = default;
};

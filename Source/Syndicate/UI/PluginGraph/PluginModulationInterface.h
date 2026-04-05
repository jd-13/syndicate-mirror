#pragma once

#include <JuceHeader.h>
#if JUCE_IOS
#include "PluginParameterSelectorOverlay.h"
#else
#include "PluginParameterSelectorWindow.h"
#endif

class SyndicateAudioProcessor;
struct PluginModulationConfig;
class ModulationSourceDefinition;
class GraphViewComponent;

/**
 * The interface between the processor and parts of the UI that control modulation.
 */
class PluginModulationInterface {
public:
    PluginModulationInterface(SyndicateAudioProcessor& processor);
    ~PluginModulationInterface() = default;

    PluginModulationConfig getPluginModulationConfig(int chainNumber, int pluginNumber);
    void togglePluginModulationActive(int chainNumber, int pluginNumber);
    void addSourceToTarget(int chainNumber, int pluginNumber, int targetNumber, ModulationSourceDefinition source);
    void removeSourceFromTarget(int chainNumber, int pluginNumber, int targetNumber, ModulationSourceDefinition source);
    void setModulationTargetValue(int chainNumber, int pluginNumber, int targetNumber, float val);
    void setModulationSourceValue(int chainNumber, int pluginNumber, int targetNumber, int sourceNumber, float val);
    void selectModulationTarget(int chainNumber, int pluginNumber, int targetNumber);
    void removeModulationTarget(int chainNumber, int pluginNumber, int targetNumber);
    juce::AudioProcessorParameter* getPluginParameterForTarget(int chainNumber, int pluginNumber, int targetNumber);

private:
    SyndicateAudioProcessor& _processor;
#if JUCE_IOS
    std::unique_ptr<PluginParameterSelectorOverlay> _parameterSelectorOverlay;
#else
    std::unique_ptr<PluginParameterSelectorWindow> _parameterSelectorWindow;
#endif

    void _onPluginParameterSelected(juce::AudioProcessorParameter* parameter, int chainNumber, int pluginNumber, int targetNumber, bool shouldClose);
};

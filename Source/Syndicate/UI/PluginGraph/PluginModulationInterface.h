#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PluginParameterSelectorWindow.h"

class GraphViewComponent;

/**
 * The interface between the processor and parts of the UI that control modulation.
 */
class PluginModulationInterface {
public:
    PluginModulationInterface(SyndicateAudioProcessor& processor, GraphViewComponent* graphView);
    ~PluginModulationInterface() = default;

    PluginModulationConfig getPluginModulationConfig(int chainNumber, int pluginNumber);
    bool setPluginModulationConfig(PluginModulationConfig config, int chainNumber, int pluginNumber);
    bool togglePluginModulationActive(int chainNumber, int pluginNumber);
    void selectModulationTarget(int chainNumber, int pluginNumber, int targetNumber);
    void removeModulationTarget(int chainNumber, int pluginNumber, int targetNumber);
    juce::AudioProcessorParameter* getPluginParameterForTarget(int chainNumber, int pluginNumber, int targetNumber);

private:
    SyndicateAudioProcessor& _processor;
    GraphViewComponent* _graphView;
    std::unique_ptr<PluginParameterSelectorWindow> _parameterSelectorWindow;

    void _onPluginParameterSelected(juce::AudioProcessorParameter* parameter, int chainNumber, int pluginNumber, int targetNumber);
};

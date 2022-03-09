#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PluginSlotComponent.h"
#include "ChainViewComponent.h"

class GraphViewComponent  : public juce::Component {
public:
    GraphViewComponent(SyndicateAudioProcessor& processor);
    ~GraphViewComponent() = default;

    void onParameterUpdate();

private:
    SyndicateAudioProcessor& _processor;
    std::vector<std::unique_ptr<ChainViewComponent>> _chainViews;
    PluginSelectionInterface _pluginSelectionInterface;
    PluginModulationInterface _pluginModulationInterface;
};

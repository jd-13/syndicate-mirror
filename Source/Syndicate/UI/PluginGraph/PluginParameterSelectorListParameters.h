#pragma once

#include <JuceHeader.h>

struct PluginParameterSelectorState;

struct PluginParameterSelectorListParameters {
    PluginParameterSelectorState& state;
    const juce::Array<juce::AudioProcessorParameter*> fullParameterList;
    std::function<void(juce::AudioProcessorParameter*, bool)> parameterSelectedCallback;
    bool isReplacingParameter;
};

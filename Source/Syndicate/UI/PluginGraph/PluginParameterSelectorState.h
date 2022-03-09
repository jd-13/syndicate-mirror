#pragma once

#include <JuceHeader.h>

struct PluginParameterSelectorState {
    juce::String filterString;

    PluginParameterSelectorState() : filterString("") { }
};
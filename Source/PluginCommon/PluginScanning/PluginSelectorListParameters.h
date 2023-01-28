/*
  ==============================================================================

    PluginSelectorListParameters.h
    Created: 28 May 2021 11:53:23pm
    Author:  Jack Devlin

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "PluginScanClient.h"
#include "PluginSelectorState.h"

struct PluginSelectorListParameters {
    PluginScanClient& scanner;
    PluginSelectorState& state;
    juce::AudioPluginFormatManager& formatManager;
    juce::AudioPluginFormat::PluginCreationCallback pluginCreationCallback;
    std::function<double()> getSampleRate;
    std::function<int()> getBlockSize;
};

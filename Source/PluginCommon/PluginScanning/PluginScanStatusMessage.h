/*
  ==============================================================================

    PluginScanStatusMessage.h
    Created: 15 Mar 2021 11:19:44pm
    Author:  Jack Devlin

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class PluginScanStatusMessage : public juce::Message {
public:
    const int numPluginsScanned;
    const bool isScanRunning;
    const juce::String errorText;
    const juce::String currentPluginName;

    PluginScanStatusMessage(int newNumPluginsScanned,
                            bool newIsScanRunning,
                            juce::String newErrorText,
                            juce::String newCurrentPluginName) :
            numPluginsScanned(newNumPluginsScanned),
            isScanRunning(newIsScanRunning),
            errorText(newErrorText),
            currentPluginName(newCurrentPluginName) {}
};

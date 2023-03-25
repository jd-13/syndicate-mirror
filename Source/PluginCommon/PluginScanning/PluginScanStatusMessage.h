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

    PluginScanStatusMessage(int newNumPluginsScanned,
                            bool newIsScanRunning) :
            numPluginsScanned(newNumPluginsScanned),
            isScanRunning(newIsScanRunning) {}
};
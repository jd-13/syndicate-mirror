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
    const bool scanStartedByAnotherInstance;
    const bool hasPreviousScan;

    PluginScanStatusMessage(int newNumPluginsScanned,
                            bool newIsScanRunning,
                            bool newScanStartedByAnotherInstance,
                            bool newHasPreviousScan) :
            numPluginsScanned(newNumPluginsScanned),
            isScanRunning(newIsScanRunning),
            scanStartedByAnotherInstance(newScanStartedByAnotherInstance),
            hasPreviousScan(newHasPreviousScan) {}
};
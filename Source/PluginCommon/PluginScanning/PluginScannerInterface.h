#pragma once

#include <JuceHeader.h>

class PluginScannerInterface {
public:
    virtual ~PluginScannerInterface() = default;

    virtual juce::Array<juce::PluginDescription> getPluginTypes() const = 0;
    virtual void restore() = 0;
    virtual void startScan() = 0;
    virtual void stopScan() = 0;
    virtual void clearMissingPlugins() = 0;
    virtual void rescanAllPlugins() = 0;
    virtual void rescanCrashedPlugins() = 0;
    virtual void scanFile(juce::File file) = 0;
    virtual void addListener(juce::MessageListener* listener) = 0;
    virtual void removeListener(juce::MessageListener* listener) = 0;
};

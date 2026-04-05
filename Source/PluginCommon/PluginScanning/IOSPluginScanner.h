#pragma once

#if JUCE_IOS

#include <JuceHeader.h>
#include "PluginScannerInterface.h"

class IOSPluginScanner : public PluginScannerInterface,
                        private juce::Thread {
public:
    IOSPluginScanner();
    ~IOSPluginScanner() override;

    juce::Array<juce::PluginDescription> getPluginTypes() const override;
    void restore() override;
    void startScan() override;
    void stopScan() override;
    void clearMissingPlugins() override;
    void rescanAllPlugins() override;
    void rescanCrashedPlugins() override;
    void scanFile(juce::File file) override;
    void addListener(juce::MessageListener* listener) override;
    void removeListener(juce::MessageListener* listener) override;

private:
    juce::AudioUnitPluginFormat _auFormat;
    juce::KnownPluginList _knownPluginList;
    std::vector<juce::MessageListener*> _listeners;
    std::mutex _listenersMutex;
    std::atomic<bool> _isScanning {false};

    void run() override;
    void _notifyAllListeners(bool isRunning);
    void _notifyListener(juce::MessageListener* listener, bool isRunning);
};

#endif // JUCE_IOS

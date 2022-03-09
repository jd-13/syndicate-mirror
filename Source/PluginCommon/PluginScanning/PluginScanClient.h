#pragma once

#include <JuceHeader.h>
#include "AllUtils.h"
#include "PluginScanStatusMessage.h"

class PluginScanClient : public juce::Thread,
                         public juce::Timer {
public:
    PluginScanClient();

    juce::Array<juce::PluginDescription> getPluginTypes() const { return _pluginList.getTypes(); }

    /**
     * Called to update the internal list of plugins to match what is on disk. Public so that a
     * restore can be done without needing to start a scan.
     */
    void restore();

    /**
     * Called when the user starts a scan.
     */
    void startScan();

    /**
     * Called when the user has initiated the stop.
     */
    void stopScan();

    /**
     * Called when the user has requested a full rescan.
     */
    void rescanAllPlugins();

    /**
     * Called when the user has requested a rescan of crashed plugins
     */
    void rescanCrashedPlugins();

    void addListener(juce::MessageListener* listener);

    void removeListener(juce::MessageListener* listener);

    /**
     * Performs actions as needed - don't call this manually
     */
    void run() override;

    /**
     * Checks if there are any updates from the plugin scan process
     */
    void timerCallback() override;

private:
    class PluginScanProcessClient : public juce::ChildProcessMaster {
    public:
        PluginScanProcessClient(std::function<void()> onConnectionLost) :
                _onConnectionLost(onConnectionLost) {
        }

        void handleMessageFromSlave(const juce::MemoryBlock& /*message*/) override {
            // Do nothing - we never receive messages
        }

        void handleConnectionLost() override {
            _onConnectionLost();
        }

    private:
        std::function<void()> _onConnectionLost;
    };

    std::unique_ptr<PluginScanProcessClient> _processClient;
    juce::KnownPluginList _pluginList;
    std::vector<juce::MessageListener*> _listeners;
    std::mutex _listenersMutex;
    juce::WaitableEvent _messageEvent;
    std::queue<std::function<void()>> _callbacksToHandle;
    juce::File _isAliveFile;

    // True if was able to restore from previous scan
    bool _hasPreviousScan;

    // True if an attempt has been made to restore from previous scan (whether successful or not)
    bool _hasAttemptedRestore;

    // True if the scan process should be restarted in the event that it exits
    bool _shouldRestart;

    // True if there is a scan process running that is owned by another instance
    bool _scanStartedByAnotherInstance;

    juce::Time _lastUpdateTime;

    void _notifyListener(juce::MessageListener* listener);

    void _onConnectionLost();

    void _readScannerFilesForUpdates();
};

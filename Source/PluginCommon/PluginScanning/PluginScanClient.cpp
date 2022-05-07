#include "PluginScanClient.h"

PluginScanClient::PluginScanClient() : juce::Thread("Scan Client"),
                                       _hasPreviousScan(false),
                                       _hasAttemptedRestore(false),
                                       _shouldRestart(false),
                                       _scanStartedByAnotherInstance(false) {

    _isAliveFile = Utils::DataDirectory.getChildFile(Utils::SCAN_IS_ALIVE_FILE_NAME);
}

void PluginScanClient::restore() {
    _hasAttemptedRestore = true;

    const juce::File scannedPluginsFile(Utils::DataDirectory.getChildFile(Utils::SCANNED_PLUGINS_FILE_NAME));

    if (scannedPluginsFile.existsAsFile()) {
        if (scannedPluginsFile.getLastModificationTime() > _lastUpdateTime) {
            std::unique_ptr<juce::XmlElement> pluginsXml = juce::parseXML(scannedPluginsFile);

            if (pluginsXml.get() != nullptr) {
                _hasPreviousScan = true;
                _lastUpdateTime = juce::Time::getCurrentTime();
                _pluginList.recreateFromXml(*pluginsXml);
            } else {
                juce::Logger::writeToLog("Unable to parse scanned plugins XML, attempting to restore from backup");

                pluginsXml = juce::parseXML(Utils::DataDirectory.getChildFile(Utils::SCANNED_PLUGINS_BACKUP_FILE_NAME));

                if (pluginsXml.get() != nullptr) {
                    _hasPreviousScan = true;
                    _lastUpdateTime = juce::Time::getCurrentTime();
                    _pluginList.recreateFromXml(*pluginsXml);
                } else {
                    juce::Logger::writeToLog("Unable to parse backup scanned plugins XML");
                }
            }

            // Notfiy the listeners
            {
                std::scoped_lock lock(_listenersMutex);
                for (juce::MessageListener* listener : _listeners) {
                    _notifyListener(listener);
                }
            }

            juce::Logger::writeToLog("Restored " + juce::String(_pluginList.getNumTypes()) + " plugins from disk");
        }

    } else {
        juce::Logger::writeToLog("Nothing to restore plugins from");
    }
}

void PluginScanClient::startScan() {
    if (!isThreadRunning()) {
        startThread();
    }

    _callbacksToHandle.push([&]() {
        // Only start if we're not currently scanning
        if (_processClient == nullptr && !_scanStartedByAnotherInstance) {
            // Attempt to restore first if we haven't already
            if (!_hasAttemptedRestore) {
                restore();
            }

            _processClient.reset(new PluginScanProcessClient([&]() { _onConnectionLost(); }));

            // Start process
            _shouldRestart = true;
            juce::Logger::writeToLog("Starting plugin scan server from location: " + Utils::PluginScanServerBinary.getFullPathName());
            const bool started {
                _processClient->launchSlaveProcess(Utils::PluginScanServerBinary,
                                                   Utils::PLUGIN_SCAN_SERVER_UID,
                                                   8000)
            };

            if (started) {
                juce::Logger::writeToLog("Started plugin scan server");
            } else {
                juce::Logger::writeToLog("Failed to start plugin scan server");
            }
        } else {
            juce::Logger::writeToLog("Scan already started");
        }
    });

    _messageEvent.signal();
}

void PluginScanClient::stopScan() {
    // Only do this if we're currently scanning
    if (_processClient != nullptr) {
        _callbacksToHandle.push([&]() {
            // Stop process if we're currently scanning
            if (_processClient != nullptr) {
                juce::Logger::writeToLog("Stopping plugin scan server");
                _shouldRestart = false;
                _processClient->killSlaveProcess();

                // Since we requested the stop, we need to delete the keep alive
                _isAliveFile.deleteFile();
            }
        });

        _messageEvent.signal();
    } else {
        juce::Logger::writeToLog("Scan already stopped");
    }
}

void PluginScanClient::rescanAllPlugins() {
    juce::Logger::writeToLog("Attempting rescan of all plugins");

    if (_processClient == nullptr && !_scanStartedByAnotherInstance) {
        juce::Logger::writeToLog("Deleting all plugin data files");
        Utils::DataDirectory.getChildFile(Utils::SCANNED_PLUGINS_FILE_NAME).deleteFile();
        Utils::DataDirectory.getChildFile(Utils::SCANNED_PLUGINS_BACKUP_FILE_NAME).deleteFile();
        Utils::DataDirectory.getChildFile(Utils::CRASHED_PLUGINS_FILE_NAME).deleteFile();

        startScan();
    } else {
        juce::Logger::writeToLog("Couldn't delete plugin data files, scan is still running");
    }
}

void PluginScanClient::rescanCrashedPlugins() {
    juce::Logger::writeToLog("Attempting rescan of crashed plugins");

    if (_processClient == nullptr && !_scanStartedByAnotherInstance) {
        juce::Logger::writeToLog("Deleting crashed plugins file");
        Utils::DataDirectory.getChildFile(Utils::CRASHED_PLUGINS_FILE_NAME).deleteFile();

        startScan();
    } else {
        juce::Logger::writeToLog("Couldn't delete crashed plugins file, scan is still running");
    }
}

void PluginScanClient::addListener(juce::MessageListener* listener) {
    if (listener != nullptr) {
        std::scoped_lock lock(_listenersMutex);
        if (std::find(_listeners.begin(), _listeners.end(), listener) == _listeners.end()) {
            _listeners.push_back(listener);
        }

        // Start looking for updates from the scanner
        if (!isTimerRunning()) {
            startTimer(Utils::PLUGIN_SCANNER_IS_ALIVE_INTERVAL * 2);
        }

        // Notify the listener of the current state
        _notifyListener(listener);
    }
}

void PluginScanClient::removeListener(juce::MessageListener* listener) {
    if (listener != nullptr) {
        std::scoped_lock lock(_listenersMutex);
        _listeners.erase(std::remove(_listeners.begin(), _listeners.end(), listener), _listeners.end());

        if (_listeners.empty()) {
            stopTimer();
        }
    }
}

void PluginScanClient::run() {
    while (!threadShouldExit()) {
        if (_messageEvent.wait(1000)) {
            // Handle messages
            while (!_callbacksToHandle.empty()) {
                _callbacksToHandle.front()();
                _callbacksToHandle.pop();

                if (_processClient == nullptr) {
                    // The previous callback was probably a disconnect - clear everything so
                    // we're ready to start again
                    while (!_callbacksToHandle.empty()) {
                        _callbacksToHandle.pop();
                    }

                    // This is where we try to restart if appropriate (eg. the server crashed)
                    if (_shouldRestart) {
                        juce::Logger::writeToLog("Attempting to restart");
                        startScan();
                    } else {
                        juce::Logger::writeToLog("Not attempting to restart");
                        signalThreadShouldExit();
                    }
                }
            }
        }
    }
}

void PluginScanClient::timerCallback() {
    _readScannerFilesForUpdates();
}

void PluginScanClient::_notifyListener(juce::MessageListener* listener) {
    listener->postMessage(new PluginScanStatusMessage(
        _pluginList.getNumTypes(), _processClient != nullptr, _scanStartedByAnotherInstance, _hasPreviousScan));
}

void PluginScanClient::_onConnectionLost() {
    // Kill the process and restart it
    juce::Logger::writeToLog("Lost connection to plugin scan server");

    _callbacksToHandle.push([&]() {
        _processClient.reset();

        _shouldRestart = _isAliveFile.existsAsFile();

        std::scoped_lock lock(_listenersMutex);
        for (juce::MessageListener* listener : _listeners) {
            _notifyListener(listener);
        }
    });

    _messageEvent.signal();
}

void PluginScanClient::_readScannerFilesForUpdates() {
    // Check if the plugin scan files have been updated since we last checked them
    restore();

    if (_isAliveFile.existsAsFile()) {
        // File exists so the scan must be running (or was running and crashed)
        // Check if we own it
        const juce::RelativeTime timeout = juce::RelativeTime::milliseconds(Utils::PLUGIN_SCANNER_IS_ALIVE_INTERVAL * 2);
        const bool isStillRunning = _isAliveFile.getLastModificationTime() > juce::Time::getCurrentTime() - timeout;

        _scanStartedByAnotherInstance = _processClient == nullptr && isStillRunning;
    } else {
        if (_processClient != nullptr) {
            // We owned this scan and it has now finished cleanly - do nothing
        } else if (_scanStartedByAnotherInstance) {
            // Another plugin instance started the scan and it has now finished, so we just need to
            // send an update
            _scanStartedByAnotherInstance = false;
        }
    }

    for (juce::MessageListener* listener : _listeners) {
        _notifyListener(listener);
    }
}

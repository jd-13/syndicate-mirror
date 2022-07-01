#include "PluginScanManager.h"
#include "AllUtils.h"

PluginScanManager::PluginScanManager() : Thread("PluginScanManager"),
                                         _isScanning(false),
                                         _hasPreviousScan(false),
                                         _hasAttemptedRestore(false),
                                         _pluginsScannedSinceBackup(0) {

    _deadMansPedalFile = Utils::DataDirectory.getChildFile(Utils::CRASHED_PLUGINS_FILE_NAME);
    _deadMansPedalFile.create();

    _scannedPluginsFile = Utils::DataDirectory.getChildFile(Utils::SCANNED_PLUGINS_FILE_NAME);
    _scannedPluginsFile.create();

    _scannedPluginsBackupFile = Utils::DataDirectory.getChildFile(Utils::SCANNED_PLUGINS_BACKUP_FILE_NAME);
    _scannedPluginsBackupFile.create();

    _isAliveFile = Utils::DataDirectory.getChildFile(Utils::SCAN_IS_ALIVE_FILE_NAME);
    _isAliveFile.create();

    _stallingPluginsFile = Utils::DataDirectory.getChildFile(Utils::STALLING_PLUGINS_FILE_NAME);
    _stallingPluginsFile.create();

    _stallingPluginsBackupFile = Utils::DataDirectory.getChildFile(Utils::STALLING_PLUGINS_BACKUP_FILE_NAME);
    _stallingPluginsBackupFile.create();
}

void PluginScanManager::startScan() {
    if (!_isScanning) {
        juce::Logger::writeToLog("Starting plugin scan");

        _isScanning = true;

        startTimer(Utils::PLUGIN_SCANNER_IS_ALIVE_INTERVAL);

        startThread();
    }
}

void PluginScanManager::stopScan() {
    if (_isScanning) {
        juce::Logger::writeToLog("Stopping plugin scan");

        signalThreadShouldExit();

        _isScanning = false;

        stopTimer();

        juce::Logger::writeToLog("Stopped plugin scan");
    }
}

void PluginScanManager::run() {

    // Check if we have any plugins to restore
    if (!_hasAttemptedRestore) {
        _restore();
    }

    auto scanForFormat = [&](juce::AudioPluginFormat& format) {
        juce::PluginDirectoryScanner scanner(_pluginList,
                                             format,
                                             format.getDefaultLocationsToSearch(),
                                             true,
                                             _deadMansPedalFile);
        bool isFinished {false};

        while (!threadShouldExit() && !isFinished) {
            // Prevent the plugin scanning itself or a plugin that previously stalled a scan
            while (scanner.getNextPluginFileThatWillBeScanned() == "Syndicate" ||
                   _stallingPluginsNames.contains(scanner.getNextPluginFileThatWillBeScanned())) {
                if (!scanner.skipNextFile()) {
                    return;
                }
            }

            // Scan the plugin
            const int previousNumTypes {_pluginList.getNumTypes()};
            juce::Logger::writeToLog("[" + format.getName() + "] plugin #" + juce::String(_pluginList.getNumTypes()) + ": " + scanner.getNextPluginFileThatWillBeScanned());
            juce::String currentPluginName;
            isFinished = !scanner.scanNextFile(true, currentPluginName);

            // Some plugins cause the connection to drop while being scanned which means stopScan()
            // will have been called setting _isScanning to false. If it happens save the plugin to
            // a list so that we don't try to scan it again when automatically restarting
            // (This seems to happen for VSTs on arm64 macOS if they don't have an arm64 slice)
            // TODO check if currentPluginName is accurate enough or might cause duplicates
            if (!_isScanning) {
                _stallingPluginsNames.add(currentPluginName);
                _stallingPluginsFile.appendText(currentPluginName + "\n");
            }

            // Backup the scanned plugins file if it's time
            _pluginsScannedSinceBackup++;

            if (_shouldUpdateBackup(_pluginsScannedSinceBackup)) {
                _pluginsScannedSinceBackup = 0;
                juce::Logger::writeToLog("Updating plugin scan backup");
                _scannedPluginsFile.copyFileTo(_scannedPluginsBackupFile);
                _stallingPluginsFile.copyFileTo(_stallingPluginsBackupFile);
            }

            // Update the plugins file if anything has changed (it might not if we've just restored
            // from a previous scan)
            if (previousNumTypes < _pluginList.getNumTypes()) {
                std::unique_ptr<juce::XmlElement> pluginsXml = std::unique_ptr<juce::XmlElement>(_pluginList.createXml());

                if (pluginsXml.get() != nullptr) {
                    // Delete and recreate the file so that it's empty
                    _scannedPluginsFile.deleteFile();
                    _scannedPluginsFile.create();

                    // Write the Xml to the file
                    juce::FileOutputStream output(_scannedPluginsFile);

                    if (output.openedOk()) {
                        pluginsXml->writeTo(output);
                    }
                }
            }
        }
    };

#ifdef __APPLE__
    scanForFormat(_auFormat);
#endif

    scanForFormat(_vstFormat);
    scanForFormat(_vst3Format);

    juce::Logger::writeToLog("All plugin scan jobs finished");
    stopScan();

    // Delete the file so that plugins can see the scan finished
    _isAliveFile.deleteFile();

    juce::JUCEApplicationBase::quit();
}

void PluginScanManager::timerCallback() {
    if (_isScanning) {
        _isAliveFile.setLastModificationTime(juce::Time::getCurrentTime());
    }
}

bool PluginScanManager::_shouldUpdateBackup(int pluginsScannedSinceBackup) const {
    // It's possible for a badly timed crash to cause the scanned plugins file to become
    // corrupt so save a backup at random intervals (within a limit). This way
    // it's unlikely for a crash to (reliably) affect the backup as well as the
    // original, while also meaning that huge plugin libraries won't need to be scanned
    // entirely again.

    if (pluginsScannedSinceBackup < 10) {
        // No point updating yet
        return false;
    }

    if (pluginsScannedSinceBackup >= 80) {
        // Make sure we update the backup at least once in while
        return true;
    }

    if (_randomGenerator.nextInt(30) == 0) {
        return true;
    }

    return false;
}

void PluginScanManager::_restore() {
    juce::Logger::writeToLog("Attempting to restore scanned plugins");
    _hasAttemptedRestore = true;

    // Read the main plugins file first
    std::unique_ptr<juce::XmlElement> pluginsXml = juce::parseXML(_scannedPluginsFile);

    juce::KnownPluginList pluginList;
    if (pluginsXml.get() != nullptr) {
        _hasPreviousScan = true;
        pluginList.recreateFromXml(*pluginsXml);
    }

    // Read the backup plugins file
    std::unique_ptr<juce::XmlElement> pluginsBackupXml = juce::parseXML(_scannedPluginsBackupFile);

    juce::KnownPluginList backupPluginList;
    if (pluginsBackupXml.get() != nullptr) {
        _hasPreviousScan = true;
        backupPluginList.recreateFromXml(*pluginsBackupXml);
    }

    // Decide whether to use the main or the backup file
    if (_hasPreviousScan) {
        if (pluginList.getNumTypes() >= backupPluginList.getNumTypes()) {
            juce::Logger::writeToLog("Restoring from main plugins file");
            _pluginList.recreateFromXml(*pluginsXml);
        } else {
            juce::Logger::writeToLog("Restoring from backup plugins file");
            _pluginList.recreateFromXml(*pluginsBackupXml);
        }
    }

    // Restore stalling plugins list
    juce::StringArray stallingPluginsNames;
    _stallingPluginsFile.readLines(stallingPluginsNames);
    stallingPluginsNames.removeEmptyStrings();

    juce::StringArray stallingPluginsNamesBackup;
    _stallingPluginsBackupFile.readLines(stallingPluginsNamesBackup);
    stallingPluginsNamesBackup.removeEmptyStrings();

    _stallingPluginsNames = stallingPluginsNames.size() >= stallingPluginsNamesBackup.size() ?
                            stallingPluginsNames : stallingPluginsNamesBackup;

    juce::Logger::writeToLog("Restored " + juce::String(_pluginList.getNumTypes()) + " plugins from disk");
}

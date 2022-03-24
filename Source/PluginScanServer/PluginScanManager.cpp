#include "PluginScanManager.h"
#include "AllUtils.h"

namespace {
    #ifdef __APPLE__
        // Extra thread for audio units
        constexpr int NUM_SCAN_THREADS {3};
    #else
        constexpr int NUM_SCAN_THREADS {2};
    #endif
}

PluginScanManager::PluginScanManager() : Thread("PluginScanManager"),
                                         _threadPool(NUM_SCAN_THREADS),
                                         _isScanning(false),
                                         _hasPreviousScan(false),
                                         _hasAttemptedRestore(false),
                                         _numJobsToFinish(0),
                                         _pluginsScannedSinceBackup(0),
                                         _pluginScannedEvent(false) {

    _deadMansPedalFile = Utils::DataDirectory.getChildFile(Utils::CRASHED_PLUGINS_FILE_NAME);
    _deadMansPedalFile.create();

    _scannedPluginsFile = Utils::DataDirectory.getChildFile(Utils::SCANNED_PLUGINS_FILE_NAME);
    _scannedPluginsFile.create();

    _scannedPluginsBackupFile = Utils::DataDirectory.getChildFile(Utils::SCANNED_PLUGINS_BACKUP_FILE_NAME);
    _scannedPluginsBackupFile.create();

    _isAliveFile = Utils::DataDirectory.getChildFile(Utils::SCAN_IS_ALIVE_FILE_NAME);
    _isAliveFile.create();
}

void PluginScanManager::startScan() {
    if (!_isScanning) {
        juce::Logger::writeToLog("Starting plugin scan");

        _isScanning = true;

        startTimer(Utils::PLUGIN_SCANNER_IS_ALIVE_INTERVAL);

        // Start checking for scans that complete
        startThread();

        // Check if we have any to restore
        if (!_hasAttemptedRestore) {
            _restore();
        }

        auto onPluginScannedCallback = [&](bool isFinished) {
            if (isFinished) {
                _numJobsToFinish--;
            }

            _pluginScannedEvent.signal();
        };

        // Set the expected number of jobs before starting any of them, otherwise they can
        // decrement the counter prematurely and stop scanning
        _numJobsToFinish = NUM_SCAN_THREADS;

        // Scan AudioUnits
        #ifdef __APPLE__
            PluginScanJob* auScanJob = new PluginScanJob("AudioUnit Scan", _pluginList, _auFormat, _deadMansPedalFile, onPluginScannedCallback);
            _threadPool.addJob(auScanJob, true);
        #endif

        // Scan VSTs
        PluginScanJob* vstScanJob = new PluginScanJob("VST Scan", _pluginList, _vstFormat, _deadMansPedalFile, onPluginScannedCallback);
        _threadPool.addJob(vstScanJob, true);

        // Scan VST3s
        PluginScanJob* vst3ScanJob = new PluginScanJob("VST3 Scan", _pluginList, _vst3Format, _deadMansPedalFile, onPluginScannedCallback);
        _threadPool.addJob(vst3ScanJob, true);

    }
}

void PluginScanManager::stopScan() {
    if (_isScanning) {
        juce::Logger::writeToLog("Stopping plugin scan");

        // Wait for all the scanning jobs to finish before exiting this thread
        constexpr int TIMEOUT {1000}; // 1 second
        _threadPool.removeAllJobs(true, TIMEOUT);

        signalThreadShouldExit();

        _isScanning = false;

        stopTimer();

        juce::Logger::writeToLog("Stopped plugin scan");
    }
}

void PluginScanManager::run() {
    while (!threadShouldExit()) {
        // Wait for a limited time so we can check if the thread should exit
        if (_pluginScannedEvent.wait(1000)) {
            // Update the plugins file
            std::unique_ptr<juce::XmlElement> pluginsXml = std::unique_ptr<juce::XmlElement>(_pluginList.createXml());

            if (pluginsXml.get() != nullptr) {
                // Backup the scanned plugins file if it's time
                _pluginsScannedSinceBackup++;

                if (_shouldUpdateBackup(_pluginsScannedSinceBackup)) {
                    _pluginsScannedSinceBackup = 0;
                    juce::Logger::writeToLog("Updating plugin scan backup");
                    _scannedPluginsFile.copyFileTo(_scannedPluginsBackupFile);
                }

                // Delete and recreate the file so that it's empty
                _scannedPluginsFile.deleteFile();
                _scannedPluginsFile.create();

                // Write the Xml to the file
                juce::FileOutputStream output(_scannedPluginsFile);

                if (output.openedOk()) {
                    pluginsXml->writeTo(output);
                }
            }

            const bool isFinished {_numJobsToFinish == 0};

            // Tidy up if all jobs are finished
            if (isFinished) {
                juce::Logger::writeToLog("All plugin scan jobs finished");
                stopScan();

                // Delete the file so that plugins can see the scan finished
                _isAliveFile.deleteFile();

                juce::JUCEApplicationBase::quit();
            }
        }
    }
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

    std::unique_ptr<juce::XmlElement> pluginsXml = juce::parseXML(_scannedPluginsFile);

    if (pluginsXml.get() != nullptr) {
        _hasPreviousScan = true;
        _pluginList.recreateFromXml(*pluginsXml);
    } else {
        juce::Logger::writeToLog("Unable to parse scanned plugins XML, attempting to restore from backup");

        pluginsXml = juce::parseXML(_scannedPluginsBackupFile);

        if (pluginsXml.get() != nullptr) {
            _hasPreviousScan = true;
            _pluginList.recreateFromXml(*pluginsXml);
        } else {
            juce::Logger::writeToLog("Unable to parse backup scanned plugins XML");
        }
    }

    juce::Logger::writeToLog("Restored " + juce::String(_pluginList.getNumTypes()) + " plugins from disk");
}

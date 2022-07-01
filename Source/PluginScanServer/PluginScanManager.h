#pragma once

#include <JuceHeader.h>

/**
 * Manages multithreaded scanning of plugins and notifies listeners of progress.
 */
class PluginScanManager : public juce::Thread,
                          public juce::Timer {
public:
    PluginScanManager();

    void startScan();

    void stopScan();

    void run() override;

    bool isScanning() const { return _isScanning; }

    void timerCallback() override;

private:
    juce::KnownPluginList _pluginList;
    juce::File _deadMansPedalFile;
    juce::File _scannedPluginsFile;
    juce::File _scannedPluginsBackupFile;
    juce::File _isAliveFile;
    juce::File _stallingPluginsFile;
    juce::File _stallingPluginsBackupFile;

    // True if currently scanning
    std::atomic<bool> _isScanning;

    // True if was able to restore from previous scan
    bool _hasPreviousScan;

    // True if an attempt has been made to restore from previous scan (whether successful or not)
    bool _hasAttemptedRestore;

    int _pluginsScannedSinceBackup;

#ifdef __APPLE__
    juce::AudioUnitPluginFormat _auFormat;
#endif
    juce::VSTPluginFormat _vstFormat;
    juce::VST3PluginFormat _vst3Format;

    mutable juce::Random _randomGenerator;

    juce::StringArray _stallingPluginsNames;

    bool _shouldUpdateBackup(int pluginsScannedSinceBackup) const;

    void _restore();
};

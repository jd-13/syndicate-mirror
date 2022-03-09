#include "PluginScanJob.h"

PluginScanJob::PluginScanJob(const juce::String& name,
                             juce::KnownPluginList& pluginList,
                             juce::AudioPluginFormat& format,
                             juce::File& deadMansPedalFile,
                             std::function<void(bool)> onPluginScannedCallback) :
        ThreadPoolJob(name),
        _pluginList(pluginList),
        _format(format),
        _deadMansPedalFile(deadMansPedalFile),
        _onPluginScannedCallback(onPluginScannedCallback) {

}

PluginScanJob::JobStatus PluginScanJob::runJob() {
    juce::PluginDirectoryScanner scanner(_pluginList,
                                            _format,
                                            _format.getDefaultLocationsToSearch(),
                                            true,
                                            _deadMansPedalFile);

    juce::Logger::writeToLog("Starting thread " + getJobName());

    bool shouldContinue {true};

    while (shouldContinue) {
        // Prevent the plugin scanning itself
        if (scanner.getNextPluginFileThatWillBeScanned() == "Syndicate") {
            scanner.skipNextFile();
        }

        // Scan the plugin
        juce::Logger::writeToLog("[" + getJobName() + "] plugin #" + juce::String(_pluginList.getNumTypes()) + ": " + scanner.getNextPluginFileThatWillBeScanned());
        juce::String currentPluginName;
        shouldContinue = scanner.scanNextFile(true, currentPluginName) && !shouldExit();

        // Notify that a plugin has been scanned
        _onPluginScannedCallback(!shouldContinue);
    }

    juce::Logger::writeToLog("Finished thread " + getJobName());

    return jobHasFinished;
}

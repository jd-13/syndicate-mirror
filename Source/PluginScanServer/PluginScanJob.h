#pragma once

#include <JuceHeader.h>

/**
 * Scans for plugins of the given format.
 */
class PluginScanJob : public juce::ThreadPoolJob {
public:
    PluginScanJob(const juce::String& name,
                  juce::KnownPluginList& pluginList,
                  juce::AudioPluginFormat& format,
                  juce::File& deadMansPedalFile,
                  std::function<void(bool)> onPluginScannedCallback);

    ~PluginScanJob() = default;

    JobStatus runJob();

private:
    juce::KnownPluginList& _pluginList;
    juce::AudioPluginFormat& _format;
    juce::File& _deadMansPedalFile;
    std::function<void(bool)> _onPluginScannedCallback;
};

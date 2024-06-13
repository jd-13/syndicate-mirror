#pragma once

#include <JuceHeader.h>

namespace Utils {
    inline const char* PLUGIN_SCAN_SERVER_UID = "pluginScanServer";

    inline const char* SCANNED_PLUGINS_FILE_NAME = "ScannedPlugins.txt";
    inline const char* CRASHED_PLUGINS_FILE_NAME = "CrashedPlugins.txt";
    inline const char* SCAN_CONFIGURATION_FILE_NAME = "ScanConfiguration.txt";
    inline const char* CONFIG_FILE_NAME = "Config.json";

#ifdef __APPLE__
    const juce::File DataDirectory(juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("WhiteElephantAudio/Syndicate"));
    const juce::File PluginLogDirectory(juce::File::getSpecialLocation(juce::File::userHomeDirectory).getChildFile("Library/Logs/WhiteElephantAudio/Syndicate/Syndicate"));
    const juce::File PluginScanServerLogDirectory(juce::File::getSpecialLocation(juce::File::userHomeDirectory).getChildFile("Library/Logs/WhiteElephantAudio/Syndicate/PluginScanServer"));
    const juce::File PluginScanServerBinary(juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory().getSiblingFile("Resources").getChildFile("PluginScanServer"));
#elif _WIN32
    const juce::File ApplicationDirectory(juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("WhiteElephantAudio/Syndicate"));
    const juce::File DataDirectory(ApplicationDirectory.getChildFile("Data"));
    const juce::File PluginLogDirectory(ApplicationDirectory.getChildFile("Logs/Syndicate"));
    const juce::File PluginScanServerLogDirectory(ApplicationDirectory.getChildFile("Logs/PluginScanServer"));
    const juce::File PluginScanServerBinary(juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory().getSiblingFile("Resources").getChildFile("PluginScanServer.exe"));
#elif __linux__
    const juce::File ApplicationDirectory(juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("WhiteElephantAudio/Syndicate"));
    const juce::File DataDirectory(ApplicationDirectory.getChildFile("Data"));
    const juce::File PluginLogDirectory(ApplicationDirectory.getChildFile("Logs/Syndicate"));
    const juce::File PluginScanServerLogDirectory(ApplicationDirectory.getChildFile("Logs/PluginScanServer"));
    const juce::File PluginScanServerBinary(juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory().getSiblingFile("Resources").getChildFile("PluginScanServer"));
#else
    #error Unsupported OS
#endif

    struct Config {
        bool enableLogFile;

        Config() : enableLogFile(false) { }
    };

    inline Config LoadConfig() {
        Config config;

        juce::File configFile(DataDirectory.getChildFile(CONFIG_FILE_NAME));
        if (!configFile.exists()) {
            return config;
        }

        juce::FileInputStream input(configFile);
        if (!input.openedOk()) {
            return config;
        }

        const juce::var json = juce::JSON::parse(input.readEntireStreamAsString());
        if (!json.isObject()) {
            return config;
        }

        if (json.hasProperty("enableLogFile")) {
            const juce::var& enableLogFile = json["enableLogFile"];
            if (enableLogFile.isBool()) {
                config.enableLogFile = enableLogFile;
            }
        }

        return config;
    }
}

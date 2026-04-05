#pragma once

#include <JuceHeader.h>

#include "AllUtils.h"
#include "MainLogger.h"
#include "NullLogger.hpp"
#include "ServerProcess.h"

// TODO use macros for name and version

class ServerApplication : public juce::JUCEApplicationBase {
public:
    ServerApplication() {
        const Utils::Config config = Utils::LoadConfig();
        if (config.enableLogFile) {
            _fileLogger = std::make_unique<MainLogger>("Syndicate Plugin Scan Server", ProjectInfo::versionString, Utils::PluginScanServerLogDirectory);
            juce::Logger::setCurrentLogger(_fileLogger.get());
        } else {
            juce::Logger::setCurrentLogger(&_nullLogger);
        }
    }

    ~ServerApplication() {
        // Logger must be removed before being deleted
        // (this must be the last thing we do before exiting)
        juce::Logger::setCurrentLogger(nullptr);
    }

    const juce::String getApplicationName() override { return "Syndicate Plugin Scan Server"; }

    const juce::String getApplicationVersion() override { return "0.0.1"; }

    // Must be true: on macOS the scan server is launched via 'open -n' (LaunchServices),
    // which starts a new instance each time. Returning false would cause macOS to reuse
    // an existing instance, breaking the per-launch IPC pipe assignment.
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String& commandLineParameters) override {
        juce::Logger::writeToLog("Initialising");
        juce::Logger::writeToLog("Command line: " + commandLineParameters);

        const juce::String cleanedParams = resolveCommandLineParams(commandLineParameters);

        _process.reset(new ServerProcess());

        if (_process->initialiseFromCommandLine(cleanedParams, Utils::PLUGIN_SCAN_SERVER_UID)) {
            juce::Logger::writeToLog("Process started");
        }
    }

    void shutdown() override {
        juce::Logger::writeToLog("Shutting down");
    }

    void anotherInstanceStarted(const juce::String& commandLine) override {
        // TODO
    }

    void systemRequestedQuit() override {
        juce::Logger::writeToLog("System Requested Quit");
        quit();
    }

    void suspended() override {
        // TODO
    }

    void resumed() override {
        // TODO
    }

    void unhandledException(const std::exception*,
                            const juce::String& sourceFilename,
                            int lineNumber) override {
        juce::Logger::writeToLog("Unhandled exception");
    }

private:
    // When launched via 'open' (LaunchServices), 'open' does not reliably
    // forward --args or --env to JUCE app bundles. The pipe name is instead
    // written to a temp file by launch_scan_server.sh; read it here if the
    // pipe name is not present in the command line.
    // LaunchServices can also prepend extra arguments (e.g. -psn_...) before
    // our prefix, so strip everything before it.
    juce::String resolveCommandLineParams(const juce::String& commandLineParameters) {
        const juce::String prefix = juce::String("--") + Utils::PLUGIN_SCAN_SERVER_UID + ":";
        juce::String cleanedParams = commandLineParameters;

        if (!cleanedParams.contains(prefix)) {
            juce::File argsDir("/tmp/syndicate_scan_args");
            if (argsDir.isDirectory()) {
                auto files = argsDir.findChildFiles(juce::File::findFiles, false, "*.args");
                // Sort by modification time so the oldest unclaimed file is first
                std::sort(files.begin(), files.end(), [](const juce::File& a, const juce::File& b) {
                    return a.getLastModificationTime() < b.getLastModificationTime();
                });
                for (auto& f : files) {
                    // Atomically claim by renaming - avoids races with concurrent server instances
                    juce::File claimed(f.getFullPathName() + ".claimed");
                    if (f.moveFileTo(claimed)) {
                        cleanedParams = claimed.loadFileAsString().trim();
                        claimed.deleteFile();
                        juce::Logger::writeToLog("Loaded pipe args from temp file: " + cleanedParams);
                        break;
                    }
                }
            }
        }

        if (!cleanedParams.trimStart().startsWith(prefix) && cleanedParams.contains(prefix)) {
            cleanedParams = cleanedParams.fromFirstOccurrenceOf(prefix, true, false);
        }

        if (cleanedParams != commandLineParameters) {
            juce::Logger::writeToLog("Cleaned command line: " + cleanedParams);
        }

        return cleanedParams;
    }

    std::unique_ptr<MainLogger> _fileLogger;
    NullLogger _nullLogger;
    std::unique_ptr<ServerProcess> _process;
};

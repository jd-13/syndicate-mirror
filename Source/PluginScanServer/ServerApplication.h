#pragma once

#include <JuceHeader.h>

#include "AllUtils.h"
#include "MainLogger.h"
#include "ServerProcess.h"

// TODO use macros for name and version

class ServerApplication : public juce::JUCEApplicationBase {
public:
    ServerApplication() : _logger("Syndicate Plugin Scan Server", "0.0.1", Utils::PluginScanServerLogDirectory) {
        juce::Logger::setCurrentLogger(&_logger);
    }

    ~ServerApplication() {
        // Logger must be removed before being deleted
        // (this must be the last thing we do before exiting)
        juce::Logger::setCurrentLogger(nullptr);
    }

    const juce::String getApplicationName() override { return "Syndicate Plugin Scan Server"; }

    const juce::String getApplicationVersion() override { return "0.0.1"; }

    bool moreThanOneInstanceAllowed() override { return false; }

    void initialise(const juce::String& commandLineParameters) override {
        juce::Logger::writeToLog("Initialising");
        _process.reset(new ServerProcess());

        if (_process->initialiseFromCommandLine(commandLineParameters, Utils::PLUGIN_SCAN_SERVER_UID)) {
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
        _process->stop();
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
    MainLogger _logger;
    std::unique_ptr<ServerProcess> _process;
};

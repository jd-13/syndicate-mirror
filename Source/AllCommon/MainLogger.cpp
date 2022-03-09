#include "MainLogger.h"

#include "AllUtils.h"

namespace {
    juce::String getTimestamp() {
        const juce::Time currentTime(juce::Time::getCurrentTime());

        juce::String milliseconds(currentTime.getMilliseconds());
        while (milliseconds.length() < 3) {
            milliseconds = "0" + milliseconds;
        }

        return currentTime.formatted("%Y-%m-%d_%H-%M-%S.") + milliseconds;
    }
}

MainLogger::MainLogger(const char* appName, const char* appVersion, const juce::File& logDirectory) {
    // Open log file
    _logFile = logDirectory.getNonexistentChildFile(getTimestamp(), ".txt", true);
    _logFile.create();

    _logEnvironment(appName, appVersion);

    // Delete old log files at a random point within the next 5 minutes
    // This is so that we don't have multiple plugin instances trying to do this at the same time
    juce::Random _randomGenerator;
    juce::Timer::callAfterDelay(
        _randomGenerator.nextInt(5 * 60 * 1000), [&]() { _deleteOldLogs(); });
}

void MainLogger::logMessage(const juce::String& message) {

    const juce::String outputMessage = getTimestamp() + " :    " + message + "\n";

    juce::FileOutputStream output(_logFile);
    if (output.openedOk()) {
        output.writeText(outputMessage, false, false, "\n");
    }
}

void MainLogger::_logEnvironment(const char* appName, const char* appVersion) {

    juce::FileOutputStream output(_logFile);

    if (output.openedOk()) {
        const juce::String outputMessage(
            "******************************************************\n" +
            juce::String(appName) + ": " + juce::String(appVersion) + "\n"
            "OS:   " + juce::SystemStats::getOperatingSystemName() + "\n"
            "CPUs: " + juce::String(juce::SystemStats::getNumPhysicalCpus()) + " (" + juce::String(juce::SystemStats::getNumCpus()) + ")\n"
            "RAM:  " + juce::String(juce::SystemStats::getMemorySizeInMegabytes()) + "MB\n"
            "******************************************************\n\n");

        output.writeText(outputMessage, false, false, "\n");
    }
}

void MainLogger::_deleteOldLogs() {
    juce::Logger::writeToLog("MainLogger::_deleteOldLogs: Checking for old log files");

    // Delete old log files
    juce::Array<juce::File> logFiles = _logFile.getParentDirectory().findChildFiles(
        juce::File::TypesOfFileToFind::findFiles, false);

    // Get the time 1 week ago
    const juce::Time oneWeekAgo = juce::Time::getCurrentTime() - juce::RelativeTime::weeks(1);

    for (juce::File& thisLogFile : logFiles) {
        if (thisLogFile.getLastModificationTime() < oneWeekAgo) {
            // This file is more than a week old
            juce::Logger::writeToLog("MainLogger::_deleteOldLogs: Deleting old log file " + thisLogFile.getFileName());
            thisLogFile.deleteFile();
        }
    }
}

#include "ServerProcess.h"


void ServerProcess::handleMessageFromMaster(const juce::MemoryBlock& /*block*/) {
    // Log but do nothing
    juce::Logger::writeToLog("Received message");
}

void ServerProcess::handleConnectionMade() {
    // If the process has been started, it means we need to scan
    juce::Logger::writeToLog("Connection made, starting scan");
    _scanner.startScan();
}

void ServerProcess::handleConnectionLost() {
    juce::Logger::writeToLog("Lost connection to client");
    stop();
    juce::JUCEApplicationBase::quit();
}

void ServerProcess::stop() {
    _scanner.stopScan();
}

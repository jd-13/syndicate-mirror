#pragma once

#include <JuceHeader.h>
#include "PluginScanManager.h"

class ServerProcess : public juce::ChildProcessSlave {
public:
    ServerProcess() = default;

    void handleMessageFromMaster(const juce::MemoryBlock& block) override;

    void handleConnectionMade() override;

    void handleConnectionLost() override;

    void stop();

private:
    PluginScanManager _scanner;
};

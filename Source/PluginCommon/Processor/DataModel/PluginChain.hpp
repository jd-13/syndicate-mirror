#pragma once

#include <JuceHeader.h>
#include "ChainSlots.hpp"
#include "LatencyListener.hpp"
#include "General/AudioSpinMutex.h"

class PluginChain {
public:
    std::vector<std::shared_ptr<ChainSlotBase>> chain;

    bool isChainBypassed;
    bool isChainMuted;

    std::function<float(int, MODULATION_TYPE)> getModulationValueCallback;

    std::unique_ptr<juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::None>> latencyCompLine;
    WECore::AudioSpinMutex latencyCompLineMutex;

    PluginChainLatencyListener latencyListener;

    PluginChain(std::function<float(int, MODULATION_TYPE)> getModulationValueCallback) :
            isChainBypassed(false),
            isChainMuted(false),
            getModulationValueCallback(getModulationValueCallback),
            latencyListener(this) {
        latencyCompLine.reset(new juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::None>(0));
        latencyCompLine->setDelay(0);
    }

    ~PluginChain() = default;
};

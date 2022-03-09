#pragma once

#include <JuceHeader.h>

#include "PluginSplitter.h"

/**
 * Contains a single plugin graph for plugins arranged in a mid side split.
 */
class PluginSplitterMidSide : public PluginSplitter {
public:
    PluginSplitterMidSide(std::function<float(int, MODULATION_TYPE)> getModulationValueCallback);
    PluginSplitterMidSide(std::vector<PluginChainWrapper>& chains, std::function<float(int, MODULATION_TYPE)> getModulationValueCallback);
    ~PluginSplitterMidSide() = default;

    SPLIT_TYPE getSplitType() override { return SPLIT_TYPE::MIDSIDE; }

    // AudioProcessor methods
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

private:
    static constexpr int DEFAULT_NUM_CHAINS {2};

    std::unique_ptr<juce::AudioBuffer<float>> _midBuffer;
    std::unique_ptr<juce::AudioBuffer<float>> _sideBuffer;
};

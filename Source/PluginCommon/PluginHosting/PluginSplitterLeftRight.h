#pragma once

#include <JuceHeader.h>

#include "PluginSplitter.h"

/**
 * Contains a single plugin graph for plugins arranged in a left right split.
 */
class PluginSplitterLeftRight : public PluginSplitter {
public:
    PluginSplitterLeftRight(std::function<float(int, MODULATION_TYPE)> getModulationValueCallback);
    PluginSplitterLeftRight(std::vector<PluginChainWrapper>& chains, std::function<float(int, MODULATION_TYPE)> getModulationValueCallback);
    ~PluginSplitterLeftRight() = default;

    SPLIT_TYPE getSplitType() override { return SPLIT_TYPE::LEFTRIGHT; }

    // AudioProcessor methods
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

private:
    static constexpr int DEFAULT_NUM_CHAINS {2};

    std::unique_ptr<juce::AudioBuffer<float>> _leftBuffer;
    std::unique_ptr<juce::AudioBuffer<float>> _rightBuffer;
};

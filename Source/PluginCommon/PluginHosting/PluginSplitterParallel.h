#pragma once

#include <JuceHeader.h>

#include "PluginSplitter.h"

/**
 * Contains a single plugin graph for plugins arranged in parallel.
 */
class PluginSplitterParallel : public PluginSplitter {
public:
    PluginSplitterParallel(std::function<float(int, MODULATION_TYPE)> getModulationValueCallback);
    PluginSplitterParallel(std::vector<PluginChainWrapper>& chains, std::function<float(int, MODULATION_TYPE)> getModulationValueCallback);
    ~PluginSplitterParallel() = default;

    SPLIT_TYPE getSplitType() override { return SPLIT_TYPE::PARALLEL; }

    bool addChain();
    bool removeChain(int chainNumber);

    // AudioProcessor methods
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

private:
    static constexpr int DEFAULT_NUM_CHAINS {1};

    std::unique_ptr<juce::AudioBuffer<float>> _inputBuffer;
    std::unique_ptr<juce::AudioBuffer<float>> _outputBuffer;
};

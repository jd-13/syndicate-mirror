/*
  ==============================================================================

    PluginSplitterSeries.h
    Created: 29 May 2021 9:02:02pm
    Author:  Jack Devlin

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "PluginSplitter.h"

/**
 * Contains a single plugin graph for plugins arranged in series.
 */
class PluginSplitterSeries : public PluginSplitter {
public:
    PluginSplitterSeries(std::function<float(int, MODULATION_TYPE)> getModulationValueCallback);
    PluginSplitterSeries(std::vector<PluginChainWrapper>& chains, std::function<float(int, MODULATION_TYPE)> getModulationValueCallback);
    ~PluginSplitterSeries() = default;

    SPLIT_TYPE getSplitType() override { return SPLIT_TYPE::SERIES; }

    // AudioProcessor methods
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

private:
    static constexpr int DEFAULT_NUM_CHAINS {1};
};

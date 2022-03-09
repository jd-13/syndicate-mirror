/*
  ==============================================================================

    PluginSplitterSeries.cpp
    Created: 29 May 2021 9:02:02pm
    Author:  Jack Devlin

  ==============================================================================
*/

#include "PluginSplitterSeries.h"

PluginSplitterSeries::PluginSplitterSeries(std::function<float(int, MODULATION_TYPE)> getModulationValueCallback)
        : PluginSplitter(DEFAULT_NUM_CHAINS, getModulationValueCallback) {
    juce::Logger::writeToLog("Constructed PluginSplitterSeries");
}

PluginSplitterSeries::PluginSplitterSeries(std::vector<PluginChainWrapper>& chains, std::function<float(int, MODULATION_TYPE)> getModulationValueCallback)
        : PluginSplitter(chains, DEFAULT_NUM_CHAINS, getModulationValueCallback) {

    // We only have one active chain in the series splitter, so it can't be muted or soloed
    _chains[0].chain->setChainMute(false);
}

void PluginSplitterSeries::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    _chains[0].chain->processBlock(buffer, midiMessages);
}

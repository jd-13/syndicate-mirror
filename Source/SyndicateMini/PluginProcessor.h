/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "MainLogger.h"
#include "PluginScanner.h"
#include "PluginSelectorState.h"
#include "PluginSplitter.h"

//==============================================================================
/**
*/
class SyndicateAudioProcessor  : public juce::AudioProcessor
{
public:
    PluginScanner pluginScanner;
    PluginSelectorState pluginSelectorState;
    std::unique_ptr<PluginSplitter> pluginSplitter;
    std::shared_ptr<juce::AudioPluginInstance> guestPlugin;

    //==============================================================================
    SyndicateAudioProcessor();
    ~SyndicateAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layout) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    void onPluginSelectedByUser(std::unique_ptr<juce::AudioPluginInstance> plugin);
    void setPluginSplitType(SPLIT_TYPE splitType);

private:
    MainLogger _logger;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SyndicateAudioProcessor)
};

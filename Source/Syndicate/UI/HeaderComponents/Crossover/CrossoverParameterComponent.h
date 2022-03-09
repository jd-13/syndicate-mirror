#pragma once

#include <array>
#include <memory>
#include <optional>

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

/**
 * Draws the clickable parts of the crossover component.
 */
class CrossoverParameterComponent : public juce::Component {
public:

    CrossoverParameterComponent(SyndicateAudioProcessor& processor);
    virtual ~CrossoverParameterComponent() = default;

    void paint(juce::Graphics& g) override;

private:
    SyndicateAudioProcessor& _processor;

    void _drawSliderThumbs(juce::Graphics& g);

    void _drawFrequencyText(juce::Graphics& g);

    void _drawBandButtons(juce::Graphics& g);
};
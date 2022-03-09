#pragma once

#include <array>
#include <optional>

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "ParameterData.h"
#include "MONSTRFilters/MONSTRParameters.h"

class CrossoverMouseListener : public juce::MouseListener {
public:

    CrossoverMouseListener(SyndicateAudioProcessor& processor);
    virtual ~CrossoverMouseListener();

    void mouseDown(const juce::MouseEvent& event) override;

    void mouseDrag(const juce::MouseEvent& event) override;

    void mouseUp(const juce::MouseEvent& event) override;

private:

    // TODO remove this struct
    struct FloatParameterInteraction {
        std::function<void(const juce::MouseEvent&)> dragCallback;
    };

    std::array<FloatParameterInteraction, WECore::MONSTR::Parameters::_MAX_NUM_BANDS - 1> _crossoverFrequencies;

    SyndicateAudioProcessor& _processor;

    FloatParameterInteraction* _dragParameter;

    /**
     * If the mouse event occured inside a button the function will handle it and return null,
     * if the event occured inside a slider it will return the corresponding
     * FloatParameterInteraction for it to be handled by the appropriate event handlers.
     */
    FloatParameterInteraction* _resolveParameterInteraction(const juce::MouseEvent& event);
};

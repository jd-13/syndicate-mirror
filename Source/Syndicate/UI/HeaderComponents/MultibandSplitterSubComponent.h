#pragma once

#include <JuceHeader.h>
#include "SplitterHeaderComponent.h"
#include "CrossoverWrapperComponent.h"

class MultibandSplitterSubComponent : public SplitterHeaderComponent,
                                      public juce::Button::Listener {
public:
    MultibandSplitterSubComponent(SyndicateAudioProcessor& processor);
    ~MultibandSplitterSubComponent() override;

    void onParameterUpdate() override;

    void resized() override;
    void buttonClicked(juce::Button* buttonThatWasClicked) override;

private:
    SyndicateAudioProcessor& _processor;

    std::unique_ptr<juce::TextButton> addBandBtn;
    std::unique_ptr<juce::TextButton> removeBandBtn;
    std::unique_ptr<CrossoverWrapperComponent> crossoverComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MultibandSplitterSubComponent)
};

//[EndFile] You can add extra defines here...
//[/EndFile]


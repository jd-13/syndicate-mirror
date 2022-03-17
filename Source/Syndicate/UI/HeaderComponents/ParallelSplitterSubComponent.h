#pragma once

#include <JuceHeader.h>
#include "ChainButtonsComponent.h"
#include "SplitterHeaderComponent.h"

class ParallelSplitterSubComponent : public SplitterHeaderComponent,
                                     public juce::Button::Listener {
public:
    ParallelSplitterSubComponent(SyndicateAudioProcessor& processor, juce::Component* extensionComponent);
    ~ParallelSplitterSubComponent() override;

    void onParameterUpdate() override;

    void resized() override;
    void buttonClicked(juce::Button* buttonThatWasClicked) override;

private:
    std::vector<std::unique_ptr<ChainButtonsComponent>> _chainButtons;
    SyndicateAudioProcessor& _processor;
    UIUtils::StaticButtonLookAndFeel _buttonLookAndFeel;

    void _rebuildHeader();

    std::unique_ptr<juce::TextButton> addChainBtn;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParallelSplitterSubComponent)
};

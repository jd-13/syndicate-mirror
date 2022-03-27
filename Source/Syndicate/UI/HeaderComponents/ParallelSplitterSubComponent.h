#pragma once

#include <JuceHeader.h>
#include "ChainButtonsComponent.h"
#include "SplitterHeaderComponent.h"
#include "UIUtils.h"

class ParallelSplitterSubComponent : public SplitterHeaderComponent,
                                     public juce::Button::Listener {
public:
    ParallelSplitterSubComponent(SyndicateAudioProcessor& processor,
                                 juce::Component* extensionComponent,
                                 UIUtils::LinkedScrollView* graphView);
    ~ParallelSplitterSubComponent() override;

    void onParameterUpdate() override;

    void resized() override;
    void buttonClicked(juce::Button* buttonThatWasClicked) override;

private:
    std::vector<std::unique_ptr<ChainButtonsComponent>> _chainButtons;
    SyndicateAudioProcessor& _processor;
    UIUtils::StaticButtonLookAndFeel _buttonLookAndFeel;
    std::unique_ptr<juce::TextButton> addChainBtn;
    std::unique_ptr<UIUtils::LinkedScrollView> _viewPort;
    UIUtils::LinkedScrollView* _graphView;

    void _rebuildHeader();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParallelSplitterSubComponent)
};

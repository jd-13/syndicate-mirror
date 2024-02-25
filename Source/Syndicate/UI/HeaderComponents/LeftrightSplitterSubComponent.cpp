#include "UIUtils.h"

#include "LeftrightSplitterSubComponent.h"

LeftrightSplitterSubComponent::LeftrightSplitterSubComponent(SyndicateAudioProcessor& processor,
                                                             UIUtils::LinkedScrollView* graphView)
        : SplitterHeaderComponent(processor, graphView) {
    auto leftChainbuttons = std::make_unique<ChainButtonsComponent>(_processor, 0);
    _viewPort->getViewedComponent()->addAndMakeVisible(leftChainbuttons.get());
    leftChainbuttons->chainLabel->setText("Left", juce::dontSendNotification);
    _chainButtons.push_back(std::move(leftChainbuttons));

    auto rightChainbuttons = std::make_unique<ChainButtonsComponent>(_processor, 1);
    _viewPort->getViewedComponent()->addAndMakeVisible(rightChainbuttons.get());
    rightChainbuttons->chainLabel->setText("Right", juce::dontSendNotification);
    _chainButtons.push_back(std::move(rightChainbuttons));
}

#include "UIUtils.h"

#include "LeftrightSplitterSubComponent.h"

LeftrightSplitterSubComponent::LeftrightSplitterSubComponent(SyndicateAudioProcessor& processor,
                                                             ChainParameters& leftChainParams,
                                                             ChainParameters& rightChainParams,
                                                             UIUtils::LinkedScrollView* graphView)
        : SplitterHeaderComponent(processor, graphView) {
    auto leftChainbuttons = std::make_unique<ChainButtonsComponent>(0, leftChainParams);
    _viewPort->getViewedComponent()->addAndMakeVisible(leftChainbuttons.get());
    leftChainbuttons->chainLabel->setText("Left", juce::dontSendNotification);
    _chainButtons.push_back(std::move(leftChainbuttons));

    auto rightChainbuttons = std::make_unique<ChainButtonsComponent>(1, rightChainParams);
    _viewPort->getViewedComponent()->addAndMakeVisible(rightChainbuttons.get());
    rightChainbuttons->chainLabel->setText("Right", juce::dontSendNotification);
    _chainButtons.push_back(std::move(rightChainbuttons));
}

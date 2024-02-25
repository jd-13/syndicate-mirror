#include "UIUtils.h"

#include "MidsideSplitterSubComponent.h"

MidsideSplitterSubComponent::MidsideSplitterSubComponent(SyndicateAudioProcessor& processor,
                                                         UIUtils::LinkedScrollView* graphView)
        : SplitterHeaderComponent(processor, graphView) {
    auto midChainbuttons = std::make_unique<ChainButtonsComponent>(_processor, 0);
    _viewPort->getViewedComponent()->addAndMakeVisible(midChainbuttons.get());
    midChainbuttons->chainLabel->setText("Mid", juce::dontSendNotification);
    _chainButtons.push_back(std::move(midChainbuttons));

    auto sideChainbuttons = std::make_unique<ChainButtonsComponent>(_processor, 1);
    _viewPort->getViewedComponent()->addAndMakeVisible(sideChainbuttons.get());
    sideChainbuttons->chainLabel->setText("Side", juce::dontSendNotification);
    _chainButtons.push_back(std::move(sideChainbuttons));
}

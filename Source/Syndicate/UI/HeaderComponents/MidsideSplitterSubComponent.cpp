#include "UIUtils.h"

#include "MidsideSplitterSubComponent.h"

MidsideSplitterSubComponent::MidsideSplitterSubComponent(SyndicateAudioProcessor& processor,
                                                         ChainParameters& midChainParams,
                                                         ChainParameters& sideChainParams,
                                                         UIUtils::LinkedScrollView* graphView)
        : SplitterHeaderComponent(processor, graphView) {
    auto midChainbuttons = std::make_unique<ChainButtonsComponent>(0, midChainParams);
    _viewPort->getViewedComponent()->addAndMakeVisible(midChainbuttons.get());
    midChainbuttons->chainLabel->setText("Mid", juce::dontSendNotification);
    _chainButtons.push_back(std::move(midChainbuttons));

    auto sideChainbuttons = std::make_unique<ChainButtonsComponent>(1, sideChainParams);
    _viewPort->getViewedComponent()->addAndMakeVisible(sideChainbuttons.get());
    sideChainbuttons->chainLabel->setText("Side", juce::dontSendNotification);
    _chainButtons.push_back(std::move(sideChainbuttons));
}

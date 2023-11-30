#include "SeriesSplitterSubComponent.h"

#include "UIUtils.h"

SeriesSplitterSubComponent::SeriesSplitterSubComponent(SyndicateAudioProcessor& processor,
                                                       ChainParameters& params,
                                                       UIUtils::LinkedScrollView* graphView)
        : SplitterHeaderComponent(processor, graphView), _params(params) {
    auto chainButtons = std::make_unique<ChainButtonsComponent>(0, params);
    _viewPort->getViewedComponent()->addAndMakeVisible(chainButtons.get());
    chainButtons->chainLabel->setText("", juce::dontSendNotification);
    _chainButtons.push_back(std::move(chainButtons));
}

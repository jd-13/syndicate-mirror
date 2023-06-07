#include "MultibandSplitterSubComponent.h"

MultibandSplitterSubComponent::MultibandSplitterSubComponent(SyndicateAudioProcessor& processor,
                                                             juce::Component* extensionComponent,
                                                             UIUtils::LinkedScrollView* graphView)
        : _processor(processor), _graphView(graphView) {

    addBandBtn.reset(new juce::TextButton("Add Band Button"));
    extensionComponent->addAndMakeVisible(addBandBtn.get());
    addBandBtn->setButtonText(TRANS("+ Band"));
    addBandBtn->setTooltip("Adds another band");
    addBandBtn->setLookAndFeel(&_buttonLookAndFeel);
    addBandBtn->setColour(juce::TextButton::buttonOnColourId, UIUtils::neutralControlColour);
    addBandBtn->setColour(juce::TextButton::textColourOnId, UIUtils::neutralControlColour);
    addBandBtn->setColour(juce::TextButton::textColourOffId, UIUtils::neutralControlColour);
    addBandBtn->addListener(this);

    crossoverComponent.reset(new CrossoverWrapperComponent(processor));
    addAndMakeVisible(crossoverComponent.get());
    crossoverComponent->setName("Crossover Component");

    _viewPort.reset(new UIUtils::LinkedScrollView());
    _viewPort->setViewedComponent(new juce::Component());
    _viewPort->setScrollBarsShown(false, false, false, true);
    _viewPort->getHorizontalScrollBar().setColour(juce::ScrollBar::ColourIds::backgroundColourId, juce::Colour(0x00000000));
    _viewPort->getHorizontalScrollBar().setColour(juce::ScrollBar::ColourIds::thumbColourId, UIUtils::neutralHighlightColour.withAlpha(0.5f));
    _viewPort->getHorizontalScrollBar().setColour(juce::ScrollBar::ColourIds::trackColourId, juce::Colour(0x00000000));
    addAndMakeVisible(_viewPort.get(), 0);
    _viewPort->setBounds(getLocalBounds());
    _viewPort->setOtherView(_graphView);
    _graphView->setOtherView(_viewPort.get());
}

MultibandSplitterSubComponent::~MultibandSplitterSubComponent() {
    addBandBtn->setLookAndFeel(nullptr);
    _graphView->removeOtherView(_viewPort.get());

    addBandBtn = nullptr;
    crossoverComponent = nullptr;
}

void MultibandSplitterSubComponent::resized() {
    juce::Rectangle<int> headerArea = getLocalBounds();

    juce::Rectangle<int> extensionArea = addBandBtn->getParentComponent()->getLocalBounds();
    constexpr int BUTTON_HEIGHT {24};
    extensionArea.reduce(4, (extensionArea.getHeight() - BUTTON_HEIGHT) / 2);
    addBandBtn->setBounds(extensionArea);

    // Resize within the header component
    crossoverComponent->setBounds(headerArea.removeFromTop(getHeight() / 2));
    _viewPort->setBounds(getLocalBounds());
    _rebuildHeader();
}

void MultibandSplitterSubComponent::buttonClicked(juce::Button* buttonThatWasClicked){
    if (buttonThatWasClicked == addBandBtn.get()) {
        _processor.addCrossoverBand();
    }
}

void MultibandSplitterSubComponent::onParameterUpdate() {
    crossoverComponent->onParameterUpdate();

    if (_chainButtons.size() != SplitterInterface::getNumChains(_processor.splitter)) {
        _rebuildHeader();
    }

    for (size_t index {0}; index < _chainButtons.size(); index++) {
        _chainButtons[index]->onParameterUpdate();
    }
}

void MultibandSplitterSubComponent::_rebuildHeader() {
    // Set up the scrollable view
    const size_t numChains {SplitterInterface::getNumChains(_processor.splitter)};
    const int scrollPosition {_viewPort->getViewPositionX()};
    const int scrollableWidth {std::max(getWidth(), static_cast<int>(UIUtils::CHAIN_WIDTH * numChains))};
    const int scrollableHeight {getHeight()};
    _viewPort->getViewedComponent()->setBounds(juce::Rectangle<int>(scrollableWidth, scrollableHeight));

    _chainButtons.clear();

    for (size_t index {0}; index < numChains; index++) {
        _chainButtons.emplace_back(std::make_unique<ChainButtonsComponent>(_processor.chainParameters[index]));
        _chainButtons[index]->chainLabel->setText("", juce::dontSendNotification);
        _viewPort->getViewedComponent()->addAndMakeVisible(_chainButtons[index].get());
    }

    _viewPort->setViewPosition(scrollPosition, 0);

    juce::Rectangle<int> scrollableArea = _viewPort->getViewedComponent()->getLocalBounds();
    if (scrollableWidth == getWidth()) {
        // If the scrollable area is the same as the width we need to remove from the left to
        // make the chains centred properly (otherwise we just left align them since the scrolling
        // will make it appear correct)
        scrollableArea.removeFromLeft(UIUtils::getChainXPos(0, numChains, getWidth()));
    }

    for (std::unique_ptr<ChainButtonsComponent>& chainButton : _chainButtons) {
        chainButton->setBounds(scrollableArea.removeFromLeft(UIUtils::CHAIN_WIDTH));
    }
}

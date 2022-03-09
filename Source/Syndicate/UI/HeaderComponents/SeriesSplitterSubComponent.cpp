#include "SeriesSplitterSubComponent.h"

#include "UIUtils.h"

SeriesSplitterSubComponent::SeriesSplitterSubComponent(ChainParameters& params) : _params(params) {
    _chainButtons.reset(new ChainButtonsComponent(params));
    addAndMakeVisible(_chainButtons.get());
    _chainButtons->chainLabel->setText("", juce::dontSendNotification);
}

SeriesSplitterSubComponent::~SeriesSplitterSubComponent() {
    _chainButtons = nullptr;
}

void SeriesSplitterSubComponent::resized() {
    _chainButtons->setBounds(UIUtils::getChainXPos(0, 1, getWidth()), 0, UIUtils::CHAIN_WIDTH, getHeight());
}

void SeriesSplitterSubComponent::onParameterUpdate() {
    _chainButtons->onParameterUpdate();
}

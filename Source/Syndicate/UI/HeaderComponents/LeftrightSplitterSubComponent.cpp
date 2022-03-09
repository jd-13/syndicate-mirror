#include "UIUtils.h"

#include "LeftrightSplitterSubComponent.h"

LeftrightSplitterSubComponent::LeftrightSplitterSubComponent(ChainParameters& leftChainParams,
                                                             ChainParameters& rightChainParams) {
    _leftChainbuttons.reset(new ChainButtonsComponent(leftChainParams));
    addAndMakeVisible(_leftChainbuttons.get());
    _leftChainbuttons->chainLabel->setText("Left", juce::dontSendNotification);

    _rightChainbuttons.reset(new ChainButtonsComponent(rightChainParams));
    addAndMakeVisible(_rightChainbuttons.get());
    _rightChainbuttons->chainLabel->setText("Right", juce::dontSendNotification);
}

LeftrightSplitterSubComponent::~LeftrightSplitterSubComponent() {
    _leftChainbuttons = nullptr;
    _rightChainbuttons = nullptr;
}

void LeftrightSplitterSubComponent::resized() {
    _leftChainbuttons->setBounds(UIUtils::getChainXPos(0, 2, getWidth()), 0, UIUtils::CHAIN_WIDTH, getHeight());
    _rightChainbuttons->setBounds(UIUtils::getChainXPos(1, 2, getWidth()), 0, UIUtils::CHAIN_WIDTH, getHeight());
}

void LeftrightSplitterSubComponent::onParameterUpdate() {
    _leftChainbuttons->onParameterUpdate();
    _rightChainbuttons->onParameterUpdate();
}

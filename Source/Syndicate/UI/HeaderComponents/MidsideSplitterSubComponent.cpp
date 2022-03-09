#include "UIUtils.h"

#include "MidsideSplitterSubComponent.h"

MidsideSplitterSubComponent::MidsideSplitterSubComponent(ChainParameters& midChainParams,
                                                         ChainParameters& sideChainParams) {

    _midChainbuttons.reset(new ChainButtonsComponent(midChainParams));
    addAndMakeVisible(_midChainbuttons.get());
    _midChainbuttons->chainLabel->setText("Mid", juce::dontSendNotification);

    _sideChainbuttons.reset(new ChainButtonsComponent(sideChainParams));
    addAndMakeVisible(_sideChainbuttons.get());
    _sideChainbuttons->chainLabel->setText("Side", juce::dontSendNotification);
}

MidsideSplitterSubComponent::~MidsideSplitterSubComponent() {
    _midChainbuttons = nullptr;
    _sideChainbuttons = nullptr;
}

void MidsideSplitterSubComponent::resized() {
    _midChainbuttons->setBounds(UIUtils::getChainXPos(0, 2, getWidth()), 0, UIUtils::CHAIN_WIDTH, getHeight());
    _sideChainbuttons->setBounds(UIUtils::getChainXPos(1, 2, getWidth()), 0, UIUtils::CHAIN_WIDTH, getHeight());
}

void MidsideSplitterSubComponent::onParameterUpdate() {
    _midChainbuttons->onParameterUpdate();
    _sideChainbuttons->onParameterUpdate();
}

#include "MultibandSplitterSubComponent.h"

MultibandSplitterSubComponent::MultibandSplitterSubComponent(SyndicateAudioProcessor& processor)
        : _processor(processor) {

    addBandBtn.reset(new juce::TextButton("Add Band Button"));
    addAndMakeVisible(addBandBtn.get());
    addBandBtn->setButtonText(TRANS("Add Band"));
    addBandBtn->addListener(this);

    removeBandBtn.reset(new juce::TextButton("Remove Band Button"));
    addAndMakeVisible(removeBandBtn.get());
    removeBandBtn->setButtonText(TRANS("Remove Band"));
    removeBandBtn->addListener(this);

    crossoverComponent.reset(new CrossoverWrapperComponent (processor));
    addAndMakeVisible(crossoverComponent.get());
    crossoverComponent->setName("Crossover Component");

}

MultibandSplitterSubComponent::~MultibandSplitterSubComponent() {
    addBandBtn = nullptr;
    removeBandBtn = nullptr;
    crossoverComponent = nullptr;
}

void MultibandSplitterSubComponent::resized() {
    addBandBtn->setBounds(456, 8, 103, 24);
    removeBandBtn->setBounds(456, 48, 103, 24);
    crossoverComponent->setBounds(8, 8, 432, 64);
}

void MultibandSplitterSubComponent::buttonClicked(juce::Button* buttonThatWasClicked){
    if (buttonThatWasClicked == addBandBtn.get()) {
        _processor.addCrossoverBand();
    } else if (buttonThatWasClicked == removeBandBtn.get()) {
        _processor.removeCrossoverBand();
    }
}

void MultibandSplitterSubComponent::onParameterUpdate() {
    crossoverComponent->onParameterUpdate();
}

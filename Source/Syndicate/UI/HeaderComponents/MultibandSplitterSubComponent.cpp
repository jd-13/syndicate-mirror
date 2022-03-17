#include "MultibandSplitterSubComponent.h"

MultibandSplitterSubComponent::MultibandSplitterSubComponent(SyndicateAudioProcessor& processor, juce::Component* extensionComponent)
        : _processor(processor) {

    addBandBtn.reset(new juce::TextButton("Add Band Button"));
    extensionComponent->addAndMakeVisible(addBandBtn.get());
    addBandBtn->setButtonText(TRANS("+ Band"));
    addBandBtn->setTooltip("Adds another band");
    addBandBtn->setLookAndFeel(&_buttonLookAndFeel);
    addBandBtn->setColour(juce::TextButton::buttonOnColourId, UIUtils::neutralControlColour);
    addBandBtn->setColour(juce::TextButton::textColourOnId, UIUtils::neutralControlColour);
    addBandBtn->setColour(juce::TextButton::textColourOffId, UIUtils::neutralControlColour);
    addBandBtn->addListener(this);

    removeBandBtn.reset(new juce::TextButton("Remove Band Button"));
    extensionComponent->addAndMakeVisible(removeBandBtn.get());
    removeBandBtn->setButtonText(TRANS("- Band"));
    removeBandBtn->setTooltip("Removes the last band");
    removeBandBtn->setLookAndFeel(&_buttonLookAndFeel);
    removeBandBtn->setColour(juce::TextButton::buttonOnColourId, UIUtils::neutralControlColour);
    removeBandBtn->setColour(juce::TextButton::textColourOnId, UIUtils::neutralControlColour);
    removeBandBtn->setColour(juce::TextButton::textColourOffId, UIUtils::neutralControlColour);
    removeBandBtn->addListener(this);

    crossoverComponent.reset(new CrossoverWrapperComponent (processor));
    addAndMakeVisible(crossoverComponent.get());
    crossoverComponent->setName("Crossover Component");
}

MultibandSplitterSubComponent::~MultibandSplitterSubComponent() {
    addBandBtn->setLookAndFeel(nullptr);
    removeBandBtn->setLookAndFeel(nullptr);

    addBandBtn = nullptr;
    removeBandBtn = nullptr;
    crossoverComponent = nullptr;
}

void MultibandSplitterSubComponent::resized() {
    juce::Rectangle<int> headerArea = getLocalBounds();
    headerArea.reduce(8, 8);
    crossoverComponent->setBounds(headerArea);

    juce::Rectangle<int> extensionArea = addBandBtn->getParentComponent()->getLocalBounds();
    constexpr int BUTTON_HEIGHT {24};
    const int margin {(extensionArea.getHeight() - BUTTON_HEIGHT * 2) / 3};
    extensionArea.reduce(4, 0);

    extensionArea.removeFromTop(margin);
    addBandBtn->setBounds(extensionArea.removeFromTop(BUTTON_HEIGHT));
    extensionArea.removeFromTop(margin);
    removeBandBtn->setBounds(extensionArea.removeFromTop(BUTTON_HEIGHT));
    extensionArea.removeFromBottom(margin);}

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

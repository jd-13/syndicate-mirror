#include "ParameterData.h"

#include "SplitterButtonsComponent.h"

SplitterButtonsComponent::SplitterButtonsComponent(SyndicateAudioProcessor& processor)
        : _processor(processor) {
    _buttonLookAndFeel.reset(new UIUtils::ToggleButtonLookAndFeel());

    seriesBtn.reset(new juce::TextButton("Series Button"));
    addAndMakeVisible(seriesBtn.get());
    seriesBtn->setButtonText(TRANS("Series"));
    seriesBtn->addListener(this);
    seriesBtn->setLookAndFeel(_buttonLookAndFeel.get());
    seriesBtn->setColour(juce::TextButton::buttonOnColourId, UIUtils::neutralControlColour);
    seriesBtn->setColour(juce::TextButton::textColourOnId, UIUtils::neutralHighlightColour);
    seriesBtn->setColour(juce::TextButton::textColourOffId, UIUtils::neutralControlColour);
    seriesBtn->setTooltip("A single chain of plugins routed in series");

    parallelBtn.reset(new juce::TextButton("Parallel Button"));
    addAndMakeVisible(parallelBtn.get());
    parallelBtn->setButtonText(TRANS("Parallel"));
    parallelBtn->addListener(this);
    parallelBtn->setLookAndFeel(_buttonLookAndFeel.get());
    parallelBtn->setColour(juce::TextButton::buttonOnColourId, UIUtils::neutralControlColour);
    parallelBtn->setColour(juce::TextButton::textColourOnId, UIUtils::neutralHighlightColour);
    parallelBtn->setColour(juce::TextButton::textColourOffId, UIUtils::neutralControlColour);
    parallelBtn->setTooltip("Multiple chains of plugins routed in parallel");

    multibandBtn.reset(new juce::TextButton("Multiband Button"));
    addAndMakeVisible(multibandBtn.get());
    multibandBtn->setButtonText(TRANS("Multiband"));
    multibandBtn->addListener(this);
    multibandBtn->setLookAndFeel(_buttonLookAndFeel.get());
    multibandBtn->setColour(juce::TextButton::buttonOnColourId, UIUtils::neutralControlColour);
    multibandBtn->setColour(juce::TextButton::textColourOnId, UIUtils::neutralHighlightColour);
    multibandBtn->setColour(juce::TextButton::textColourOffId, UIUtils::neutralControlColour);
    multibandBtn->setTooltip("Multiple chains of plugins processing different frequency bands");

    leftrightBtn.reset(new juce::TextButton("Left/Right Button"));
    addAndMakeVisible(leftrightBtn.get());
    leftrightBtn->setButtonText(TRANS("Left/Right"));
    leftrightBtn->addListener(this);
    leftrightBtn->setLookAndFeel(_buttonLookAndFeel.get());
    leftrightBtn->setColour(juce::TextButton::buttonOnColourId, UIUtils::neutralControlColour);
    leftrightBtn->setColour(juce::TextButton::textColourOnId, UIUtils::neutralHighlightColour);
    leftrightBtn->setColour(juce::TextButton::textColourOffId, UIUtils::neutralControlColour);
    leftrightBtn->setTooltip("Two chains of plugins to process the left and right channels separately");

    midsideBtn.reset(new juce::TextButton("Mid/Side Button"));
    addAndMakeVisible(midsideBtn.get());
    midsideBtn->setButtonText(TRANS("Mid/Side"));
    midsideBtn->addListener(this);
    midsideBtn->setLookAndFeel(_buttonLookAndFeel.get());
    midsideBtn->setColour(juce::TextButton::buttonOnColourId, UIUtils::neutralControlColour);
    midsideBtn->setColour(juce::TextButton::textColourOnId, UIUtils::neutralHighlightColour);
    midsideBtn->setColour(juce::TextButton::textColourOffId, UIUtils::neutralControlColour);
    midsideBtn->setTooltip("Two chains of plugins to process the mid and side channels separately");

    leftrightBtn->setEnabled(_processor.canDoStereoSplitTypes());
    midsideBtn->setEnabled(_processor.canDoStereoSplitTypes());
}

SplitterButtonsComponent::~SplitterButtonsComponent() {
    seriesBtn->setLookAndFeel(nullptr);
    parallelBtn->setLookAndFeel(nullptr);
    multibandBtn->setLookAndFeel(nullptr);
    leftrightBtn->setLookAndFeel(nullptr);
    midsideBtn->setLookAndFeel(nullptr);

    seriesBtn = nullptr;
    parallelBtn = nullptr;
    multibandBtn = nullptr;
    leftrightBtn = nullptr;
    midsideBtn = nullptr;
}

void SplitterButtonsComponent::resized() {
    juce::Rectangle<int> availableArea = getLocalBounds();
    const int margin {availableArea.getHeight() / 5};
    availableArea.removeFromTop(margin);
    availableArea.removeFromBottom(margin);
    availableArea.removeFromLeft(margin);
    availableArea.removeFromRight(margin);

    juce::FlexBox flexBox;
    flexBox.flexWrap = juce::FlexBox::Wrap::wrap;
    flexBox.justifyContent = juce::FlexBox::JustifyContent::spaceAround;
    flexBox.alignContent = juce::FlexBox::AlignContent::center;
    flexBox.items.add(juce::FlexItem(*seriesBtn.get()).withMinWidth(88).withMinHeight(availableArea.getHeight()));
    flexBox.items.add(juce::FlexItem(*parallelBtn.get()).withMinWidth(88).withMinHeight(availableArea.getHeight()));
    flexBox.items.add(juce::FlexItem(*multibandBtn.get()).withMinWidth(88).withMinHeight(availableArea.getHeight()));
    flexBox.items.add(juce::FlexItem(*leftrightBtn.get()).withMinWidth(88).withMinHeight(availableArea.getHeight()));
    flexBox.items.add(juce::FlexItem(*midsideBtn.get()).withMinWidth(88).withMinHeight(availableArea.getHeight()));
    flexBox.performLayout(availableArea.toFloat());
}

void SplitterButtonsComponent::buttonClicked(juce::Button* buttonThatWasClicked) {
    if (buttonThatWasClicked == seriesBtn.get()) {
        _processor.setSplitType(SPLIT_TYPE::SERIES);
    } else if (buttonThatWasClicked == parallelBtn.get()) {
        _processor.setSplitType(SPLIT_TYPE::PARALLEL);
    } else if (buttonThatWasClicked == multibandBtn.get()) {
        _processor.setSplitType(SPLIT_TYPE::MULTIBAND);
    } else if (buttonThatWasClicked == leftrightBtn.get()) {
        _processor.setSplitType(SPLIT_TYPE::LEFTRIGHT);
    } else if (buttonThatWasClicked == midsideBtn.get()) {
        _processor.setSplitType(SPLIT_TYPE::MIDSIDE);
    }
}

void SplitterButtonsComponent::onParameterUpdate() {
    if (_processor.getSplitType() == SPLIT_TYPE::SERIES) {
        seriesBtn->setToggleState(true, juce::NotificationType::dontSendNotification);
        parallelBtn->setToggleState(false, juce::NotificationType::dontSendNotification);
        multibandBtn->setToggleState(false, juce::NotificationType::dontSendNotification);
        leftrightBtn->setToggleState(false, juce::NotificationType::dontSendNotification);
        midsideBtn->setToggleState(false, juce::NotificationType::dontSendNotification);
    } else if (_processor.getSplitType() == SPLIT_TYPE::PARALLEL) {
        seriesBtn->setToggleState(false, juce::NotificationType::dontSendNotification);
        parallelBtn->setToggleState(true, juce::NotificationType::dontSendNotification);
        multibandBtn->setToggleState(false, juce::NotificationType::dontSendNotification);
        leftrightBtn->setToggleState(false, juce::NotificationType::dontSendNotification);
        midsideBtn->setToggleState(false, juce::NotificationType::dontSendNotification);
    } else if (_processor.getSplitType() == SPLIT_TYPE::MULTIBAND) {
        seriesBtn->setToggleState(false, juce::NotificationType::dontSendNotification);
        parallelBtn->setToggleState(false, juce::NotificationType::dontSendNotification);
        multibandBtn->setToggleState(true, juce::NotificationType::dontSendNotification);
        leftrightBtn->setToggleState(false, juce::NotificationType::dontSendNotification);
        midsideBtn->setToggleState(false, juce::NotificationType::dontSendNotification);
    } else if (_processor.getSplitType() == SPLIT_TYPE::LEFTRIGHT) {
        seriesBtn->setToggleState(false, juce::NotificationType::dontSendNotification);
        parallelBtn->setToggleState(false, juce::NotificationType::dontSendNotification);
        multibandBtn->setToggleState(false, juce::NotificationType::dontSendNotification);
        leftrightBtn->setToggleState(true, juce::NotificationType::dontSendNotification);
        midsideBtn->setToggleState(false, juce::NotificationType::dontSendNotification);
    } else if (_processor.getSplitType() == SPLIT_TYPE::MIDSIDE) {
        seriesBtn->setToggleState(false, juce::NotificationType::dontSendNotification);
        parallelBtn->setToggleState(false, juce::NotificationType::dontSendNotification);
        multibandBtn->setToggleState(false, juce::NotificationType::dontSendNotification);
        leftrightBtn->setToggleState(false, juce::NotificationType::dontSendNotification);
        midsideBtn->setToggleState(true, juce::NotificationType::dontSendNotification);
    }
}

#include "ChainButtonsComponent.h"

ChainButtonsComponent::ChainButtonsComponent(ChainParameters& headerParams) :
        _headerParams(headerParams),
        _canRemove(false) {
    chainLabel.reset(new juce::Label("Chain Label", TRANS("")));
    addAndMakeVisible(chainLabel.get());
    UIUtils::setDefaultLabelStyle(chainLabel);
    chainLabel->addMouseListener(this, false);

    removeButton.reset(new UIUtils::CrossButton("Remove Button"));
    addAndMakeVisible(removeButton.get());
    removeButton->addListener(this);
    removeButton->setTooltip("Remove this chain");
    removeButton->setVisible(false);
    removeButton->setColour(juce::TextButton::buttonOnColourId, UIUtils::neutralHighlightColour);
    removeButton->addMouseListener(this, false);

    bypassBtn.reset(new ChainButton(CHAIN_BUTTON_TYPE::BYPASS));
    addAndMakeVisible(bypassBtn.get());
    bypassBtn->addListener(this);
    bypassBtn->setTooltip("Bypass this chain");
    bypassBtn->addMouseListener(this, false);

    muteBtn.reset(new ChainButton(CHAIN_BUTTON_TYPE::MUTE));
    addAndMakeVisible(muteBtn.get());
    muteBtn->addListener(this);
    muteBtn->setTooltip("Mute this chain");
    muteBtn->addMouseListener(this, false);

    soloBtn.reset(new ChainButton(CHAIN_BUTTON_TYPE::SOLO));
    addAndMakeVisible(soloBtn.get());
    soloBtn->addListener(this);
    soloBtn->setTooltip("Solo this chain");
    soloBtn->addMouseListener(this, false);
}

ChainButtonsComponent::ChainButtonsComponent(ChainParameters& headerParams,
                                             std::function<void()> removeChainCallback) :
        ChainButtonsComponent(headerParams) {
    _removeChainCallback = removeChainCallback;
    _canRemove = true;
}

ChainButtonsComponent::~ChainButtonsComponent() {
    chainLabel = nullptr;
    removeButton = nullptr;
    bypassBtn = nullptr;
    muteBtn = nullptr;
    soloBtn = nullptr;
}

void ChainButtonsComponent::resized() {
    juce::Rectangle<int> availableArea = getLocalBounds();
    const int margin {4};
    availableArea = availableArea.reduced(margin);

    const juce::Rectangle<int> topArea = availableArea.removeFromTop(availableArea.getHeight() / 2);
    chainLabel->setBounds(topArea);
    removeButton->setBounds(topArea);

    juce::FlexBox flexBox;
    flexBox.flexWrap = juce::FlexBox::Wrap::wrap;
    flexBox.justifyContent = juce::FlexBox::JustifyContent::spaceAround;
    flexBox.alignContent = juce::FlexBox::AlignContent::center;
    flexBox.items.add(juce::FlexItem(*bypassBtn.get()).withMinWidth(UIUtils::BAND_BUTTON_WIDTH).withMinHeight(UIUtils::BAND_BUTTON_WIDTH));
    flexBox.items.add(juce::FlexItem(*muteBtn.get()).withMinWidth(UIUtils::BAND_BUTTON_WIDTH).withMinHeight(UIUtils::BAND_BUTTON_WIDTH));
    flexBox.items.add(juce::FlexItem(*soloBtn.get()).withMinWidth(UIUtils::BAND_BUTTON_WIDTH).withMinHeight(UIUtils::BAND_BUTTON_WIDTH));
    flexBox.performLayout(availableArea.toFloat());
}

void ChainButtonsComponent::buttonClicked(juce::Button* buttonThatWasClicked) {
    if (buttonThatWasClicked == removeButton.get()) {
        _removeChainCallback();
    } else if (buttonThatWasClicked == bypassBtn.get()) {
        _headerParams.setBypass(!bypassBtn->getToggleState());
    } else if (buttonThatWasClicked == muteBtn.get()) {
        _headerParams.setMute(!muteBtn->getToggleState());
    } else if (buttonThatWasClicked == soloBtn.get()) {
        _headerParams.setSolo(!soloBtn->getToggleState());
    }
}

void ChainButtonsComponent::mouseEnter(const juce::MouseEvent& event) {
    if (_canRemove) {
        chainLabel->setVisible(false);
        removeButton->setVisible(true);
    }
}

void ChainButtonsComponent::mouseExit(const juce::MouseEvent& event) {
    if (_canRemove) {
        chainLabel->setVisible(true);
        removeButton->setVisible(false);
    }
}

void ChainButtonsComponent::onParameterUpdate() {
    bypassBtn->setToggleState(_headerParams.getBypass(), juce::dontSendNotification);
    muteBtn->setToggleState(_headerParams.getMute(), juce::dontSendNotification);
    soloBtn->setToggleState(_headerParams.getSolo(), juce::dontSendNotification);
}

int ChainButtonsComponent::_getButtonXPos(int buttonIndex) {
    const float margin {getWidth() / 10.0f};
    const float availableWidth {getWidth() - margin * 2};

    return (availableWidth / 6.0) * (buttonIndex * 2.0 + 1) - (UIUtils::BAND_BUTTON_WIDTH / 2.0) + margin;
}

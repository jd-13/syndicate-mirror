#include "ChainButtonsComponent.h"

ChainButtonsComponent::ChainButtonsComponent(int chainNumber, ChainParameters& headerParams) :
        _chainNumber(chainNumber), _headerParams(headerParams) {
    chainLabel.reset(new juce::Label("Chain Label", TRANS("")));
    addAndMakeVisible(chainLabel.get());
    UIUtils::setDefaultLabelStyle(chainLabel);
    chainLabel->setColour(juce::Label::textColourId, UIUtils::highlightColour);
    chainLabel->addMouseListener(this, false);

    dragHandle.reset(new UIUtils::DragHandle());
    addAndMakeVisible(dragHandle.get());
    dragHandle->setColour(UIUtils::DragHandle::handleColourId, UIUtils::highlightColour);
    dragHandle->setTooltip(TRANS("Drag to move this chain to another position"));
    dragHandle->addMouseListener(this, false);

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

    removeButton.reset(new UIUtils::CrossButton("Remove Button"));
    addAndMakeVisible(removeButton.get());
    removeButton->addListener(this);
    removeButton->setTooltip("Remove this chain");
    removeButton->setColour(UIUtils::CrossButton::enabledColour, UIUtils::highlightColour);
    removeButton->setColour(UIUtils::CrossButton::disabledColour, UIUtils::deactivatedColour);
    removeButton->addMouseListener(this, false);
    removeButton->setEnabled(false);
}

ChainButtonsComponent::ChainButtonsComponent(int chainNumber,
                                             ChainParameters& headerParams,
                                             std::function<void()> removeChainCallback) :
        ChainButtonsComponent(chainNumber, headerParams) {
    _removeChainCallback = removeChainCallback;
    removeButton->setEnabled(true);
}

ChainButtonsComponent::~ChainButtonsComponent() {
    chainLabel = nullptr;
    dragHandle = nullptr;
    bypassBtn = nullptr;
    muteBtn = nullptr;
    soloBtn = nullptr;
    removeButton = nullptr;
}

void ChainButtonsComponent::resized() {
    juce::Rectangle<int> availableArea = getLocalBounds();

    const juce::Rectangle<int> topArea = availableArea.removeFromTop(availableArea.getHeight() / 2);
    chainLabel->setBounds(topArea);

    constexpr int BUTTON_ROW_MARGIN {5};
    availableArea = availableArea.reduced(BUTTON_ROW_MARGIN, 0);

    constexpr int DRAG_HANDLE_WIDTH {22};
    constexpr int BAND_BUTTON_WIDTH {21};

    juce::FlexBox flexBox;
    flexBox.flexWrap = juce::FlexBox::Wrap::wrap;
    flexBox.justifyContent = juce::FlexBox::JustifyContent::spaceAround;
    flexBox.alignContent = juce::FlexBox::AlignContent::center;
    flexBox.items.add(juce::FlexItem(*dragHandle.get()).withMinWidth(DRAG_HANDLE_WIDTH).withMinHeight(DRAG_HANDLE_WIDTH));
    flexBox.items.add(juce::FlexItem(*bypassBtn.get()).withMinWidth(BAND_BUTTON_WIDTH).withMinHeight(BAND_BUTTON_WIDTH));
    flexBox.items.add(juce::FlexItem(*muteBtn.get()).withMinWidth(BAND_BUTTON_WIDTH).withMinHeight(BAND_BUTTON_WIDTH));
    flexBox.items.add(juce::FlexItem(*soloBtn.get()).withMinWidth(BAND_BUTTON_WIDTH).withMinHeight(BAND_BUTTON_WIDTH));
    flexBox.items.add(juce::FlexItem(*removeButton.get()).withMinWidth(BAND_BUTTON_WIDTH).withMinHeight(BAND_BUTTON_WIDTH));
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

void ChainButtonsComponent::onParameterUpdate() {
    bypassBtn->setToggleState(_headerParams.getBypass(), juce::dontSendNotification);
    muteBtn->setToggleState(_headerParams.getMute(), juce::dontSendNotification);
    soloBtn->setToggleState(_headerParams.getSolo(), juce::dontSendNotification);
}

void ChainButtonsComponent::mouseDrag(const juce::MouseEvent& e) {
    if (dragHandle != nullptr && e.originalComponent == dragHandle.get()) {
        juce::DragAndDropContainer* container = juce::DragAndDropContainer::findParentDragContainerFor(this);

        if (container != nullptr) {
            juce::var details;
            details.append(_chainNumber);

            // This is a copy if alt is down, otherwise move
            details.append(juce::ModifierKeys::currentModifiers.isAltDown());

            container->startDragging(details, this);
        }
    }
}

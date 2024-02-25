#include "ChainButtonsComponent.h"
#include "ModelInterface.hpp"

ChainButtonsComponent::ChainButtonsComponent(SyndicateAudioProcessor& processor, int chainNumber) :
        _processor(processor), _chainNumber(chainNumber) {
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
    bypassBtn->setTooltip("Bypass this chain");
    bypassBtn->addMouseListener(this, false);
    bypassBtn->onClick = [&processor, chainNumber] () {
        processor.setChainBypass(chainNumber, !ModelInterface::getChainBypass(processor.manager, chainNumber));
    };

    muteBtn.reset(new ChainButton(CHAIN_BUTTON_TYPE::MUTE));
    addAndMakeVisible(muteBtn.get());
    muteBtn->setTooltip("Mute this chain");
    muteBtn->addMouseListener(this, false);
    muteBtn->onClick = [&processor, chainNumber] () {
        processor.setChainMute(chainNumber, !ModelInterface::getChainMute(processor.manager, chainNumber));
    };

    soloBtn.reset(new ChainButton(CHAIN_BUTTON_TYPE::SOLO));
    addAndMakeVisible(soloBtn.get());
    soloBtn->setTooltip("Solo this chain");
    soloBtn->addMouseListener(this, false);
    soloBtn->onClick = [&processor, chainNumber] () {
        processor.setChainSolo(chainNumber, !ModelInterface::getChainSolo(processor.manager, chainNumber));
    };

    removeButton.reset(new UIUtils::CrossButton("Remove Button"));
    addAndMakeVisible(removeButton.get());
    removeButton->setTooltip("Remove this chain");
    removeButton->setColour(UIUtils::CrossButton::enabledColour, UIUtils::highlightColour);
    removeButton->setColour(UIUtils::CrossButton::disabledColour, UIUtils::deactivatedColour);
    removeButton->addMouseListener(this, false);
    removeButton->setEnabled(false);
    removeButton->onClick = [this] () {
        _removeChainCallback();
    };
}

ChainButtonsComponent::ChainButtonsComponent(SyndicateAudioProcessor& processor,
                                             int chainNumber,
                                             std::function<void()> removeChainCallback) :
        ChainButtonsComponent(processor, chainNumber) {
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

void ChainButtonsComponent::refresh() {
    bypassBtn->setToggleState(ModelInterface::getChainBypass(_processor.manager, _chainNumber), juce::dontSendNotification);
    muteBtn->setToggleState(ModelInterface::getChainMute(_processor.manager, _chainNumber), juce::dontSendNotification);
    soloBtn->setToggleState(ModelInterface::getChainSolo(_processor.manager, _chainNumber), juce::dontSendNotification);
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

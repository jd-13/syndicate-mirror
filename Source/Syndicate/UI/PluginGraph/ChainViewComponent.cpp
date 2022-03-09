#include "ChainViewComponent.h"
#include "UIUtils.h"
#include "EmptyPluginSlotComponent.h"
#include "GainStageSlotComponent.h"
#include "PluginSlotComponent.h"

namespace {
    std::tuple<bool, int, int> slotPositionFromVariant(juce::var variant) {
        bool isValid {false};
        int chainNumber {0};
        int slotNumber {0};

        if (variant.isArray() && variant.size() == 2 && variant[0].isInt() && variant[1].isInt()) {
            isValid = true;
            chainNumber = variant[0];
            slotNumber = variant[1];
        }

        return std::make_tuple(isValid, chainNumber, slotNumber);
    }
}

ChainViewComponent::ChainViewComponent(int chainNumber,
                                       PluginSelectionInterface& pluginSelectionInterface,
                                       PluginModulationInterface& pluginModulationInterface) :
        _chainNumber(chainNumber),
        _pluginSelectionInterface(pluginSelectionInterface),
        _pluginModulationInterface(pluginModulationInterface),
        _shouldDrawDragHint(false),
        _dragHintSlotNumber(0) {
}

void ChainViewComponent::setPlugins(PluginChain* newChain) {

    // Clear all slots and rebuild the chain
    _pluginSlots.clear();

    juce::Rectangle<int> availableArea = getLocalBounds();
    availableArea.removeFromLeft(1);
    availableArea.removeFromRight(1);

    for (size_t index {0}; index < newChain->getNumSlots(); index++) {
        // Add the slot
        std::unique_ptr<BaseSlotComponent> newSlot;
        if (_pluginSelectionInterface.isPluginSlot(_chainNumber, index)) {
            newSlot.reset(new PluginSlotComponent(_pluginSelectionInterface, _pluginModulationInterface, _chainNumber, index));
        } else {
            newSlot.reset(new GainStageSlotComponent(_pluginSelectionInterface, _chainNumber, index));
        }

        // Figure out the correct height for this slot
        const bool hasModulationTray {_pluginModulationInterface.getPluginModulationConfig(_chainNumber, index).isActive};
        const int slotHeight {
            UIUtils::PLUGIN_SLOT_HEIGHT + (hasModulationTray ? UIUtils::PLUGIN_SLOT_MOD_TRAY_HEIGHT : 0)
        };

        // Add the component for this slot
        addAndMakeVisible(newSlot.get());
        newSlot->setBounds(availableArea.removeFromTop(slotHeight));
        availableArea.removeFromTop(2);
        _pluginSlots.push_back(std::move(newSlot));
    }

    // Add the empty slot at the end
    std::unique_ptr<BaseSlotComponent> newSlot(
        new EmptyPluginSlotComponent(_pluginSelectionInterface, _chainNumber, _pluginSlots.size()));

    addAndMakeVisible(newSlot.get());
    newSlot->setBounds(availableArea.removeFromTop(UIUtils::PLUGIN_SLOT_HEIGHT));
    _pluginSlots.push_back(std::move(newSlot));
}

void ChainViewComponent::paint(juce::Graphics& g) {
    if (_shouldDrawDragHint) {
        g.setColour(UIUtils::neutralHighlightColour);
        const int hintYPos {_dragHintSlotNumber < _pluginSlots.size() ? _pluginSlots[_dragHintSlotNumber]->getY() : 0};
        g.drawLine(0, hintYPos, getWidth(), hintYPos, 2.0f);
    }
}

bool ChainViewComponent::isInterestedInDragSource(const SourceDetails& dragSourceDetails) {
    auto [isValid, fromChainNumber, fromSlotNumber] = slotPositionFromVariant(dragSourceDetails.description);

    // TODO check if the slot has actually moved

    return isValid;
}

void ChainViewComponent::itemDragEnter(const SourceDetails& dragSourceDetails) {
    auto [isValid, fromChainNumber, fromSlotNumber] = slotPositionFromVariant(dragSourceDetails.description);

    if (isValid) {
        _shouldDrawDragHint = true;
        _dragHintSlotNumber = _dragCursorPositionToSlotNumber(dragSourceDetails.localPosition);
        repaint();
    }
}

void ChainViewComponent::itemDragMove(const SourceDetails& dragSourceDetails) {
    auto [isValid, fromChainNumber, fromSlotNumber] = slotPositionFromVariant(dragSourceDetails.description);

    if (isValid) {
        _dragHintSlotNumber = _dragCursorPositionToSlotNumber(dragSourceDetails.localPosition);
        repaint();
    }
}

void ChainViewComponent::itemDragExit(const SourceDetails& /*dragSourceDetails*/) {
    _shouldDrawDragHint = false;
    repaint();
}

void ChainViewComponent::itemDropped(const SourceDetails& dragSourceDetails) {
    _shouldDrawDragHint = false;
    repaint();

    // Actually move the slot
    auto [isValid, fromChainNumber, fromSlotNumber] = slotPositionFromVariant(dragSourceDetails.description);

    if (isValid) {
        _pluginSelectionInterface.moveSlot(fromChainNumber, fromSlotNumber, _chainNumber, _dragHintSlotNumber);
    }
}

int ChainViewComponent::_dragCursorPositionToSlotNumber(juce::Point<int> cursorPosition) {
    int retVal {0};

    // Check which component the drag is over
    for (size_t slotNumber {0}; slotNumber < _pluginSlots.size(); slotNumber++) {

        juce::Rectangle<int> slotArea(_pluginSlots[slotNumber]->getBounds());

        if (slotArea.contains(cursorPosition)) {
            retVal = slotNumber;
            break;
        }
    }

    return retVal;
}

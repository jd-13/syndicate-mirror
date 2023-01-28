#include "BaseSlotComponent.h"

BaseSlotComponent::BaseSlotComponent() : _chainNumber(0), _slotNumber(0) {
}


BaseSlotComponent::BaseSlotComponent(int chainNumber, int slotNumber) : _chainNumber(chainNumber),
                                                                        _slotNumber(slotNumber) {
    // Set up the drag label
    dragHandle.reset(new UIUtils::DragHandle());
    addAndMakeVisible(dragHandle.get());
    dragHandle->setColour(UIUtils::DragHandle::handleColourId, UIUtils::neutralControlColour);

    juce::String copyKey;
#if _WIN32
        copyKey = "Alt";
#elif __APPLE__
        copyKey = "Option";
#elif __linux__
        copyKey = "Alt";
#else
    #error "Unknown OS"
#endif

    dragHandle->setTooltip(TRANS("Drag to move this slot to another position - hold " + copyKey + " key to copy"));
    dragHandle->addMouseListener(this, false);
}

void BaseSlotComponent::resized() {
    if (dragHandle != nullptr) {
        dragHandle->setBounds(juce::Rectangle<int>(getWidth(), UIUtils::PLUGIN_SLOT_HEIGHT).removeFromLeft(UIUtils::SLOT_DRAG_HANDLE_WIDTH));
    }
}

void BaseSlotComponent::paint(juce::Graphics& g) {
    const juce::Rectangle<float> fillArea = juce::Rectangle<int>(getWidth(), UIUtils::PLUGIN_SLOT_HEIGHT).reduced(MARGIN, MARGIN).toFloat();
    const float cornerRadius {fillArea.getHeight() / 2};

    g.setColour(UIUtils::backgroundColour);
    g.fillRoundedRectangle(fillArea, cornerRadius);

    g.setColour(UIUtils::neutralControlColour);
    g.drawRoundedRectangle(fillArea, cornerRadius, 1);
}

juce::Rectangle<int> BaseSlotComponent::getAvailableSlotArea() {
    return dragHandle == nullptr ? getLocalBounds() :
        juce::Rectangle<int>(getWidth(), UIUtils::PLUGIN_SLOT_HEIGHT).withTrimmedLeft(dragHandle->getWidth());
}

void BaseSlotComponent::mouseDrag(const juce::MouseEvent& e) {
    if (dragHandle != nullptr && e.originalComponent == dragHandle.get()) {
        juce::DragAndDropContainer* container = juce::DragAndDropContainer::findParentDragContainerFor(this);

        if (container != nullptr) {
            juce::var details;
            details.append(_chainNumber);
            details.append(_slotNumber);

            // This is a copy if alt is down, otherwise move
            details.append(juce::ModifierKeys::currentModifiers.isAltDown());

            container->startDragging(details, this);
        }
    }
}

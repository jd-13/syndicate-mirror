#include "MacrosComponent.h"
#include "UIUtils.h"

MacrosComponent::MacrosComponent(juce::DragAndDropContainer* dragContainer,
                                 std::array<juce::AudioParameterFloat*, NUM_MACROS>& macroParams,
                                 std::array<juce::String, NUM_MACROS>& macroNames) {
    for (int index {0}; index < UIUtils::NUM_MACROS; index++) {
        _macroComponents.push_back(std::make_unique<MacroComponent>(index + 1, dragContainer, macroParams[index], macroNames[index]));
        addAndMakeVisible(_macroComponents[index].get());
    }

#ifdef RESIZE_DISABLE_EDIT
    _hasAlreadyBeenResized = false;
#endif
}

MacrosComponent::~MacrosComponent() {
    for (std::unique_ptr<MacroComponent>& macro : _macroComponents) {
        macro = nullptr;
    }
}

void MacrosComponent::resized() {
    juce::Rectangle<int> availableArea = getLocalBounds();
    availableArea.removeFromTop(12);
    availableArea.removeFromBottom(12);
    availableArea.removeFromLeft(8);
    availableArea.removeFromRight(8);

    juce::FlexBox flexBox;
    flexBox.flexDirection = juce::FlexBox::Direction::column;
    flexBox.flexWrap = juce::FlexBox::Wrap::wrap;
    flexBox.justifyContent = juce::FlexBox::JustifyContent::spaceAround;
    flexBox.alignContent = juce::FlexBox::AlignContent::center;

    for (std::unique_ptr<MacroComponent>& macro : _macroComponents) {
        flexBox.items.add(juce::FlexItem(*macro.get()).withMinWidth(availableArea.getWidth()).withMinHeight(UIUtils::MACRO_HEIGHT));
    }

    flexBox.performLayout(availableArea.toFloat());

#ifdef RESIZE_DISABLE_EDIT
    if (_hasAlreadyBeenResized) {
        for (std::unique_ptr<MacroComponent>& component : _macroComponents) {
            component->disableEdit();
        }
    } else {
        _hasAlreadyBeenResized = true;
    }
#endif
}

void MacrosComponent::onParameterUpdate() {
    for (std::unique_ptr<MacroComponent>& component : _macroComponents) {
        component->onParameterUpdate();
    }
}

void MacrosComponent::updateNames(std::array<juce::String, NUM_MACROS>& macroNames) {
    for (int index {0}; index < NUM_MACROS; index++) {
        _macroComponents[index]->updateName(macroNames[index]);
    }
}


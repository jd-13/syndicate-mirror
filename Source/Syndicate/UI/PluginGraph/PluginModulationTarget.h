#pragma once

#include <JuceHeader.h>
#include "PluginModulationInterface.h"
#include "ModulationSourceDefinition.hpp"
#include "UIUtils.h"
#include "General/CoreMath.h"

class PluginModulationTargetSlider : public juce::Slider,
                                     public juce::Timer {
public:
    PluginModulationTargetSlider(juce::AudioProcessorParameter* pluginParameter);
    ~PluginModulationTargetSlider() = default;

    void paint(juce::Graphics& g) override;

    void timerCallback() override;

private:
    juce::AudioProcessorParameter* _pluginParameter;
    float _pluginParameterValue;
};

class PluginModulationTargetButton : public juce::TextButton {
public:
    PluginModulationTargetButton(std::function<void()> onRemoveCallback) : _onRemoveCallback(onRemoveCallback), _isRightClick(false) {}

    void mouseDown(const juce::MouseEvent& event) override;

    void mouseUp(const juce::MouseEvent& event) override;

private:
    std::function<void()> _onRemoveCallback;
    bool _isRightClick;
};

/**
 * Displays the small modulation source slider below the modulation target. A single target may have
 * multiple slots each representing a different modulation source.
 */
class PluginModulationTargetSourceSlider : public juce::Slider {
public:

    PluginModulationTargetSourceSlider(ModulationSourceDefinition definition,
                                       std::function<void(ModulationSourceDefinition)> onRemoveCallback);
    ~PluginModulationTargetSourceSlider() = default;

    void resized() override;
    void paint(juce::Graphics& g) override;

    void mouseDown(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

private:
    const ModulationSourceDefinition _definition;
    std::function<void(ModulationSourceDefinition)> _onRemoveCallback;

    std::unique_ptr<juce::Label> _idLabel;
    std::unique_ptr<juce::GlowEffect> _glowEffect;
    bool _isRightClick;
};

/**
 * Contains all the UI components needed for a particular modulation target, eg. the target slider,
 * target select button, and modulation slots.
 */
class PluginModulationTarget : public juce::Component,
                               public juce::Slider::Listener,
                               public juce::Button::Listener,
                               public juce::DragAndDropTarget {
public:
    PluginModulationTarget(PluginModulationInterface& pluginModulationInterface,
                           int chainNumber,
                           int pluginNumber,
                           int targetNumber);

    ~PluginModulationTarget();

    void resized() override;

    void sliderValueChanged(juce::Slider* sliderThatWasMoved) override;
    void buttonClicked(juce::Button* buttonThatWasClicked) override;

    bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override;
    void itemDragEnter(const SourceDetails& dragSourceDetails) override;
    void itemDragExit(const SourceDetails& dragSourceDetails) override;
    void itemDropped(const SourceDetails& dragSourceDetails) override;

private:
    PluginModulationInterface& _pluginModulationInterface;
    int _chainNumber;
    int _pluginNumber;
    int _targetNumber;
    std::unique_ptr<PluginModulationTargetSlider> _targetSlider;
    std::unique_ptr<PluginModulationTargetButton> _targetSelectButton;
    std::vector<std::unique_ptr<PluginModulationTargetSourceSlider>> _modulationSlots;
    UIUtils::StandardSliderLookAndFeel _sliderLookAndFeel;
    UIUtils::StaticButtonLookAndFeel _buttonLookAndFeel;

    void _addTargetSlot(ModulationSourceDefinition definition);
    void _removeTargetSlot(ModulationSourceDefinition definition);
    void _refreshSlotPositions();
    void _reloadState();
};

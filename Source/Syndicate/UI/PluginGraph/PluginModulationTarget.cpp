#include "PluginModulationTarget.h"
#include "UIUtils.h"

namespace {
    int getModulationSlotXPos(int slotIndex, int numSlots, int targetWidth) {
        return targetWidth / 2 + UIUtils::PLUGIN_MOD_TARGET_SLIDER_WIDTH * ((numSlots * -0.5) + slotIndex);
    }
}
PluginModulationTargetSlider::PluginModulationTargetSlider(juce::AudioProcessorParameter* pluginParameter) :
            _pluginParameter(pluginParameter), _pluginParameterValue(0) {
    startTimerHz(15);
}

void PluginModulationTargetSlider::paint(juce::Graphics& g) {
    // Draw the normal slider using LookAndFeel
    juce::Slider::paint(g);

    // Draw the inner modulation ring
    g.setColour(juce::Colours::grey);

    constexpr double arcGap {WECore::CoreMath::DOUBLE_TAU / 4};
    constexpr double rangeOfMotion {WECore::CoreMath::DOUBLE_TAU - arcGap};

    const double sliderNormalisedValue {(getValue() - getMinimum()) /
                                        (getMaximum() - getMinimum())};
    const double arcStartPoint {sliderNormalisedValue * rangeOfMotion + arcGap / 2};
    const double arcEndPoint {_pluginParameterValue * rangeOfMotion + arcGap / 2};

    constexpr double margin {1.5};
    const double diameter {static_cast<float>(getHeight() - margin * 2)};

    constexpr int arcSpacing {6};
    juce::Path p;
    p.addCentredArc(getWidth() / 2,
                    getHeight() / 2,
                    diameter / 2 - arcSpacing,
                    diameter / 2 - arcSpacing,
                    WECore::CoreMath::DOUBLE_PI,
                    arcStartPoint,
                    arcEndPoint,
                    true);

    g.strokePath(p, juce::PathStrokeType(2.0f));
}

void PluginModulationTargetSlider::timerCallback() {
    if (_pluginParameter != nullptr) {
        _pluginParameterValue = _pluginParameter->getValue();
    }

    repaint();
}

void PluginModulationTargetButton::mouseDown(const juce::MouseEvent& event) {
    // If this is a right click, we need to remove the target on mouseUp
    if (event.mods.isRightButtonDown()) {
        _isRightClick = true;
    } else {
        _isRightClick = false;
    }

    juce::Button::mouseDown(event);
}

void PluginModulationTargetButton::mouseUp(const juce::MouseEvent& event) {
    // If it's a right click make sure the mouse is still over the button
    if (_isRightClick && isDown() && isOver()) {
        // Don't send an event, just call the callback
        _onRemoveCallback();
    } else {
        juce::Button::mouseUp(event);
    }
}

PluginModulationTargetSourceSlider::PluginModulationTargetSourceSlider(
        ModulationSourceDefinition definition,
        std::function<void(ModulationSourceDefinition)> onRemoveCallback)
            : _definition(definition), _onRemoveCallback(onRemoveCallback), _isRightClick(false) {
    setRange(-1, 1, 0);
    setDoubleClickReturnValue(true, 0);

    _idLabel.reset(new juce::Label("ID Label", juce::String(_definition.id)));
    addAndMakeVisible(_idLabel.get());
    _idLabel->setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle("Regular"));
    _idLabel->setJustificationType(juce::Justification::centred);
    _idLabel->setEditable(true, true, false);
    _idLabel->setColour(juce::Label::textColourId, UIUtils::neutralColour);
    _idLabel->setColour(juce::Label::backgroundColourId, juce::Colour(0x00000000));
    _idLabel->setInterceptsMouseClicks(false, false);

    // TODO use custom effect that can be thicker without feathering like the glow effect does
    _glowEffect.reset(new juce::GlowEffect());
    _glowEffect->setGlowProperties(2, UIUtils::backgroundColour);
    _idLabel->setComponentEffect(_glowEffect.get());
}

void PluginModulationTargetSourceSlider::resized() {
    _idLabel->setBounds(getLocalBounds());
}

void PluginModulationTargetSourceSlider::paint(juce::Graphics& g) {
    // Figure out the dimensions of the circle we need to draw
    const int centreX {getWidth() / 2};
    const int centreY {getHeight() / 2};
    const int radius {getHeight() / 2};
    const double circleHeight {radius * getValue()};
    const double centreAngle {std::asin(circleHeight / radius)};

    // Start with the left arc
    juce::Path p;
    p.addCentredArc(centreX,
                    centreY,
                    radius,
                    radius,
                    -0.5 * WECore::CoreMath::DOUBLE_PI,
                    centreAngle,
                    0,
                    true);

    // Then the horizontal centre line
    p.lineTo(getWidth(), centreY);

    // Finally the right arc
    p.addCentredArc(centreX,
                    centreY,
                    radius,
                    radius,
                    0.5 * WECore::CoreMath::DOUBLE_PI,
                    0,
                    -centreAngle);

    // Fill the path
    p.closeSubPath();
    g.setColour(UIUtils::getColourForModulationType(_definition.type));
    g.fillPath(p);
}

void PluginModulationTargetSourceSlider::mouseDown(const juce::MouseEvent& event) {
    // If this is a right click, we need to remove the source on mouseUp
    if (event.mods.isRightButtonDown()) {
        _isRightClick = true;
    } else {
        _isRightClick = false;

        // If it's not a right click, pass the event through so the slider can be moved
        juce::Slider::mouseDown(event);
    }
}

void PluginModulationTargetSourceSlider::mouseUp(const juce::MouseEvent& event) {
    if (_isRightClick) {
        // If it's a right click make sure the mouse is still over the slider
        if (contains(event.getPosition())) {
            // Don't send an event, just call the callback
            _onRemoveCallback(_definition);
        }
    } else {
        // Not a right click, pass the event through as normal
        juce::Slider::mouseUp(event);
    }
}

PluginModulationTarget::PluginModulationTarget(PluginModulationInterface& pluginModulationInterface,
                                               int chainNumber,
                                               int pluginNumber,
                                               int targetNumber) :
        _pluginModulationInterface(pluginModulationInterface),
        _chainNumber(chainNumber),
        _pluginNumber(pluginNumber),
        _targetNumber(targetNumber) {

    juce::AudioProcessorParameter* param = _pluginModulationInterface.getPluginParameterForTarget(chainNumber, pluginNumber, targetNumber);
    _targetSlider.reset(new PluginModulationTargetSlider(param));
    addAndMakeVisible(_targetSlider.get());
    _targetSlider->setRange(0, 1, 0);
    _targetSlider->setSliderStyle(juce::Slider::RotaryVerticalDrag);
    _targetSlider->setTextBoxStyle(juce::Slider::NoTextBox, false, 80, 20);
    _targetSlider->addListener(this);
    _targetSlider->setLookAndFeel(&_sliderLookAndFeel);
    _targetSlider->setColour(juce::Slider::rotarySliderFillColourId, UIUtils::highlightColour);
    _targetSlider->setTooltip("Control for the selected plugin parameter, drag a source here to modulate");

    _targetSelectButton.reset(new PluginModulationTargetButton([&]() { _pluginModulationInterface.removeModulationTarget(_chainNumber, _pluginNumber, _targetNumber); }));
    addAndMakeVisible(_targetSelectButton.get());
    _targetSelectButton->addListener(this);
    _targetSelectButton->setLookAndFeel(&_buttonLookAndFeel);
    _targetSelectButton->setColour(UIUtils::StaticButtonLookAndFeel::backgroundColour, UIUtils::slotBackgroundColour);
    _targetSelectButton->setColour(UIUtils::StaticButtonLookAndFeel::highlightColour, UIUtils::highlightColour);
    _targetSelectButton->setColour(UIUtils::StaticButtonLookAndFeel::disabledColour, UIUtils::deactivatedColour);

    _targetAddButton.reset(new juce::TextButton("Param"));
    addAndMakeVisible(_targetAddButton.get());
    _targetAddButton->addListener(this);
    _targetAddButton->setLookAndFeel(&_addButtonLookAndFeel);
    _targetAddButton->setColour(UIUtils::StaticButtonLookAndFeel::backgroundColour, UIUtils::slotBackgroundColour);
    _targetAddButton->setColour(UIUtils::StaticButtonLookAndFeel::highlightColour, UIUtils::highlightColour);
    _targetAddButton->setColour(UIUtils::StaticButtonLookAndFeel::disabledColour, UIUtils::deactivatedColour);
    _targetAddButton->setTooltip("Add a plugin parameter to modulate");

    _reloadState();
}

PluginModulationTarget::~PluginModulationTarget() {
    _targetSlider->setLookAndFeel(nullptr);
    _targetSelectButton->setLookAndFeel(nullptr);
    _targetAddButton->setLookAndFeel(nullptr);

    _targetSlider = nullptr;
    _targetSelectButton = nullptr;
    _targetAddButton = nullptr;
}

void PluginModulationTarget::resized() {
    juce::Rectangle<int> availableArea = getLocalBounds();

    _targetSlider->setBounds(availableArea.removeFromTop(availableArea.getHeight() / 2));
    _targetSelectButton->setBounds(availableArea.removeFromTop(availableArea.getHeight() / 2));
    _targetAddButton->setBounds(getLocalBounds().withTrimmedBottom(availableArea.getHeight()));

    _refreshSlotPositions();
}

void PluginModulationTarget::sliderValueChanged(juce::Slider* sliderThatWasMoved) {
    if (sliderThatWasMoved == _targetSlider.get()) {
        _pluginModulationInterface.setModulationTargetValue(_chainNumber, _pluginNumber, _targetNumber, _targetSlider->getValue());
    } else {
        // Check if one of the source sliders were updated
        for (int slotIndex {0}; slotIndex < _modulationSlots.size(); slotIndex++) {
            if (sliderThatWasMoved == _modulationSlots[slotIndex].get()) {
                // Update the modulation amount for this source
                _pluginModulationInterface.setModulationSourceValue(_chainNumber, _pluginNumber, _targetNumber, slotIndex, sliderThatWasMoved->getValue());
            }
        }
    }
}

void PluginModulationTarget::buttonClicked(juce::Button* buttonThatWasClicked) {
    if (buttonThatWasClicked == _targetSelectButton.get()) {
        if (juce::ModifierKeys::currentModifiers.isRightButtonDown()) {
            // Remove this target
            _pluginModulationInterface.removeModulationTarget(_chainNumber, _pluginNumber, _targetNumber);
        } else {
            // Select a target
            _pluginModulationInterface.selectModulationTarget(_chainNumber, _pluginNumber, _targetNumber);
        }
    } else if (buttonThatWasClicked == _targetAddButton.get()) {
        // Select a target
        _pluginModulationInterface.selectModulationTarget(_chainNumber, _pluginNumber, _targetNumber);
    }
}

bool PluginModulationTarget::isInterestedInDragSource(const SourceDetails& dragSourceDetails) {
    const std::optional<ModulationSourceDefinition> draggedDefinition =
        ModulationSourceDefinition::fromVariant(dragSourceDetails.description);

    // Check dragged item contains a valid definition
    if (!draggedDefinition.has_value()) {
        return false;
    }

    // Check if we already have this source assigned to this target
    PluginModulationConfig modulationConfig =
                    _pluginModulationInterface.getPluginModulationConfig(_chainNumber, _pluginNumber);

    if (modulationConfig.parameterConfigs.size() > _targetNumber) {
        std::vector<std::shared_ptr<PluginParameterModulationSource>> sources = modulationConfig.parameterConfigs[_targetNumber]->sources;

        for (const auto source : sources) {
            if (source->definition == draggedDefinition.value()) {
                return false;
            }
        }
    } else {
        // This is an empty target
        return false;
    }

    return true;
}

void PluginModulationTarget::itemDragEnter(const SourceDetails& dragSourceDetails) {
    // Make the modulation slot visible
    const std::optional<ModulationSourceDefinition> draggedDefinition =
        ModulationSourceDefinition::fromVariant(dragSourceDetails.description);

    if (draggedDefinition.has_value()) {
        _addTargetSlot(draggedDefinition.value());
    }
}

void PluginModulationTarget::itemDragExit(const SourceDetails& /*dragSourceDetails*/) {
    // Remove the modulation slot from the UI
    _modulationSlots.pop_back();
    _refreshSlotPositions();
}

void PluginModulationTarget::itemDropped(const SourceDetails& dragSourceDetails) {
    const std::optional<ModulationSourceDefinition> draggedDefinition =
        ModulationSourceDefinition::fromVariant(dragSourceDetails.description);

    // Add another source to the target
    if (draggedDefinition.has_value()) {
        _pluginModulationInterface.addSourceToTarget(_chainNumber, _pluginNumber, _targetNumber, draggedDefinition.value());
    }
}

void PluginModulationTarget::_addTargetSlot(ModulationSourceDefinition definition) {
    // Add a slot to the UI (but not on the backend)
    std::unique_ptr<PluginModulationTargetSourceSlider> newModulationSlot =
        std::make_unique<PluginModulationTargetSourceSlider>(definition, [&](ModulationSourceDefinition definition) { _removeTargetSlot(definition); });

    addAndMakeVisible(newModulationSlot.get());
    newModulationSlot->setSliderStyle(juce::Slider::RotaryVerticalDrag);
    newModulationSlot->setTextBoxStyle(juce::Slider::NoTextBox, false, 80, 20);
    newModulationSlot->addListener(this);
    newModulationSlot->setTooltip("Drag up or down to add positive or negative modulation, right click to remove");
    _modulationSlots.push_back(std::move(newModulationSlot));
    _refreshSlotPositions();
}

void PluginModulationTarget::_removeTargetSlot(ModulationSourceDefinition definition) {
    _pluginModulationInterface.removeSourceFromTarget(_chainNumber, _pluginNumber, _targetNumber, definition);
}

void PluginModulationTarget::_refreshSlotPositions() {
    for (int index {0}; index < _modulationSlots.size(); index++) {
        // TODO figure out layout for more than four slots
        _modulationSlots[index]->setBounds(getModulationSlotXPos(index, _modulationSlots.size(), getWidth()),
                                           _targetSelectButton->getBounds().getBottom(),
                                           getHeight() / 4,
                                           getHeight() / 4);
    }
}

void PluginModulationTarget::_reloadState() {
    _modulationSlots.clear();

    // Try to restore UI state from the modulation config stored in the processor
    const PluginModulationConfig& modulationConfig =
        _pluginModulationInterface.getPluginModulationConfig(_chainNumber, _pluginNumber);

    if (modulationConfig.parameterConfigs.size() > _targetNumber) {
        _targetAddButton->setEnabled(false);
        _targetAddButton->setVisible(false);

        const std::shared_ptr<PluginParameterModulationConfig> thisParameterConfig = modulationConfig.parameterConfigs[_targetNumber];

        // Restoring slider position
        _targetSlider->setEnabled(true);
        _targetSlider->setVisible(true);
        _targetSlider->setValue(thisParameterConfig->restValue,
                                juce::NotificationType::dontSendNotification);

        // Restoring button text
        _targetSelectButton->setButtonText(thisParameterConfig->targetParameterName);
        _targetSelectButton->setTooltip(thisParameterConfig->targetParameterName + " - Click to select a different plugin parameter, right click to remove");

        // Restoring target slots
        for (const std::shared_ptr<PluginParameterModulationSource> thisSource : thisParameterConfig->sources) {
            _addTargetSlot(ModulationSourceDefinition(thisSource->definition.id, thisSource->definition.type));
            _modulationSlots[_modulationSlots.size() - 1]->setValue(thisSource->modulationAmount, juce::dontSendNotification);
        }

    } else {
        _targetSlider->setEnabled(false);
        _targetSlider->setVisible(false);
        _targetSelectButton->setEnabled(false);
        _targetSelectButton->setVisible(false);
    }
}

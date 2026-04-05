#include "ModulationTargetSourceSlider.hpp"
#include "UIUtils.h"

void ModulationTargetSourceSlider::RoundedPopupLabel::setText(const juce::String& text) {
    _text = text;
    repaint();
}

void ModulationTargetSourceSlider::RoundedPopupLabel::paint(juce::Graphics& g) {
    g.setColour(UIUtils::backgroundColour);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 5.0f);
    g.setColour(UIUtils::neutralColour);
    g.setFont(juce::Font(15.0f));
    g.drawText(_text, getLocalBounds(), juce::Justification::centred);
}

ModulationTargetSourceSlider::ModulationTargetSourceSlider(
        ModulationSourceDefinition definition,
        std::function<void(ModulationSourceDefinition)> onRemoveCallback)
            : _definition(definition), _onRemoveCallback(onRemoveCallback), _isRightClick(false) {
#if JUCE_IOS
    _longPressHandler = std::make_unique<UIUtils::LongPressHandler>([this]() {
        juce::PopupMenu menu;
        menu.addItem(1, TRANS("Remove source"));
        menu.showMenuAsync(juce::PopupMenu::Options(), [this](int result) {
            if (result == 1) {
                _onRemoveCallback(_definition);
            }
        });
    });
#endif
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

void ModulationTargetSourceSlider::resized() {
    _idLabel->setBounds(getLocalBounds());
}

void ModulationTargetSourceSlider::paint(juce::Graphics& g) {
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

void ModulationTargetSourceSlider::mouseDown(const juce::MouseEvent& event) {
#if JUCE_IOS
    _longPressHandler->mouseDown();
    juce::Slider::mouseDown(event);
#else
    // If this is a right click, we need to remove the source on mouseUp
    if (event.mods.isRightButtonDown()) {
        _isRightClick = true;
    } else {
        _isRightClick = false;

        // If it's not a right click, pass the event through so the slider can be moved
        juce::Slider::mouseDown(event);
    }
#endif
}

void ModulationTargetSourceSlider::mouseUp(const juce::MouseEvent& event) {
#if JUCE_IOS
    if (!_longPressHandler->mouseUp()) {
        juce::Slider::mouseUp(event);
    }
#else
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
#endif
}

void ModulationTargetSourceSlider::mouseEnter(const juce::MouseEvent& event) {
    _showValuePopup();
    juce::Slider::mouseEnter(event);
}

void ModulationTargetSourceSlider::mouseExit(const juce::MouseEvent& event) {
    _hideValuePopup();
    juce::Slider::mouseExit(event);
}

void ModulationTargetSourceSlider::mouseDrag(const juce::MouseEvent& event) {
#if JUCE_IOS
    _longPressHandler->mouseDrag();
#endif
    if (_valueBubble != nullptr) {
        _valueBubble->setText(juce::String(getValue(), 2));
    }

    juce::Slider::mouseDrag(event);
}

void ModulationTargetSourceSlider::_showValuePopup() {
    if (_valueBubble == nullptr) {
        auto bubble = std::make_unique<RoundedPopupLabel>();
        bubble->setInterceptsMouseClicks(false, false);
        getTopLevelComponent()->addAndMakeVisible(bubble.get());
        _valueBubble = std::move(bubble);
    }

    constexpr int popupWidth {50};
    constexpr int popupHeight {20};
    const juce::Point<int> bottomCentre {getTopLevelComponent()->getLocalPoint(
        this, juce::Point<int>(getWidth() / 2, getHeight()))};
    _valueBubble->setBounds(bottomCentre.x - popupWidth / 2,
                            bottomCentre.y + 2,
                            popupWidth,
                            popupHeight);
    _valueBubble->setText(juce::String(getValue(), 2));
}

void ModulationTargetSourceSlider::_hideValuePopup() {
    _valueBubble.reset();
}

ModulationTargetSourceSliders::ModulationTargetSourceSliders(
    std::vector<std::shared_ptr<PluginParameterModulationSource>> sources,
    std::function<void(int, float)> onSliderMovedCallback,
    std::function<void(ModulationSourceDefinition)> onRemoveCallback) :
        _onSliderMovedCallback(onSliderMovedCallback),
        _onRemoveCallback(onRemoveCallback) {

    // Restoring target slots
    for (const std::shared_ptr<PluginParameterModulationSource> thisSource : sources) {
        addSource(ModulationSourceDefinition(thisSource->definition.id, thisSource->definition.type));
        _sliders[_sliders.size() - 1]->setValue(thisSource->modulationAmount, juce::dontSendNotification);
    }
}

void ModulationTargetSourceSliders::resized() {
    _refreshSlotPositions();
}

void ModulationTargetSourceSliders::paint(juce::Graphics& g) {
    // Draw an empty source slot if there are no sources
    if (_sliders.size() == 0) {
        constexpr int MARGIN {1};
        const int diameter {getHeight() - 2 * MARGIN};
        juce::Path p;
        p.addEllipse(
            getWidth() / 2 - diameter / 2,
            MARGIN,
            diameter,
            diameter);

        // Draw the outline
        g.setColour(UIUtils::neutralColour.withAlpha(0.5f));
        g.strokePath(p, juce::PathStrokeType(1.0f));
    }
}

void ModulationTargetSourceSliders::sliderValueChanged(juce::Slider* sliderThatWasMoved) {
    for (int slotIndex {0}; slotIndex < _sliders.size(); slotIndex++) {
        if (sliderThatWasMoved == _sliders[slotIndex].get()) {
            _onSliderMovedCallback(slotIndex, sliderThatWasMoved->getValue());
        }
    }
}

void ModulationTargetSourceSliders::addSource(ModulationSourceDefinition definition) {
    auto newModulationSlot = std::make_unique<ModulationTargetSourceSlider>(
        definition, [&](ModulationSourceDefinition definition) { _onRemoveCallback(definition); });

    addAndMakeVisible(newModulationSlot.get());
    newModulationSlot->setSliderStyle(juce::Slider::RotaryVerticalDrag);
    newModulationSlot->setTextBoxStyle(juce::Slider::NoTextBox, false, 80, 20);
    newModulationSlot->addListener(this);
#if JUCE_IOS
    newModulationSlot->setTooltip("Drag up or down to add positive or negative modulation, long press to remove");
#else
    newModulationSlot->setTooltip("Drag up or down to add positive or negative modulation, right click to remove");
#endif
    _sliders.push_back(std::move(newModulationSlot));
    _refreshSlotPositions();
}

void ModulationTargetSourceSliders::removeLastSource() {
    _sliders.pop_back();
    _refreshSlotPositions();
}

void ModulationTargetSourceSliders::_refreshSlotPositions() {
    juce::FlexBox flexBox;
    flexBox.flexDirection = juce::FlexBox::Direction::row;
    flexBox.flexWrap = juce::FlexBox::Wrap::noWrap;
    flexBox.justifyContent = juce::FlexBox::JustifyContent::center;
    flexBox.alignContent = juce::FlexBox::AlignContent::center;

    for (std::unique_ptr<ModulationTargetSourceSlider>& slider : _sliders) {
        flexBox.items.add(juce::FlexItem(*slider).withMinHeight(getHeight()).withMinWidth(UIUtils::PLUGIN_MOD_TARGET_SLIDER_WIDTH));
    }

    juce::Rectangle<int> availableArea = getLocalBounds();
    flexBox.performLayout(availableArea.toFloat());
}

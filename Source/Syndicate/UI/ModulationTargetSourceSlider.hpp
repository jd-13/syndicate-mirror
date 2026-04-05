#pragma once

#include <JuceHeader.h>
#include "ModulationSourceDefinition.hpp"
#include "ChainSlots.hpp"
#include "UIUtils.h"

/**
 * Displays the small modulation source slider below the modulation target. A single target may have
 * multiple slots each representing a different modulation source.
 */
class ModulationTargetSourceSlider : public juce::Slider {
public:

    ModulationTargetSourceSlider(ModulationSourceDefinition definition,
                                 std::function<void(ModulationSourceDefinition)> onRemoveCallback);
    ~ModulationTargetSourceSlider() = default;

    void resized() override;
    void paint(juce::Graphics& g) override;

    void mouseDown(const juce::MouseEvent& event) override;
    void mouseEnter(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

private:
    class RoundedPopupLabel : public juce::Component {
    public:
        void setText(const juce::String& text);
        void paint(juce::Graphics& g) override;

    private:
        juce::String _text;
    };

    const ModulationSourceDefinition _definition;
    std::function<void(ModulationSourceDefinition)> _onRemoveCallback;

    std::unique_ptr<juce::Label> _idLabel;
    std::unique_ptr<juce::GlowEffect> _glowEffect;
    std::unique_ptr<RoundedPopupLabel> _valueBubble;
    bool _isRightClick;

    void _showValuePopup();
    void _hideValuePopup();
#if JUCE_IOS
    std::unique_ptr<UIUtils::LongPressHandler> _longPressHandler;
#endif
};

class ModulationTargetSourceSliders : public juce::Component,
                                      public juce::Slider::Listener {
public:
    ModulationTargetSourceSliders(
        std::vector<std::shared_ptr<PluginParameterModulationSource>> sources,
        std::function<void(int, float)> onSliderMovedCallback,
        std::function<void(ModulationSourceDefinition)> onRemoveCallback);
    ~ModulationTargetSourceSliders() = default;

    void resized() override;
    void paint(juce::Graphics& g) override;
    void sliderValueChanged(juce::Slider* sliderThatWasMoved) override;

    void addSource(ModulationSourceDefinition definition);
    void removeLastSource();

private:
    std::function<void(int, float)> _onSliderMovedCallback;
    std::function<void(ModulationSourceDefinition)> _onRemoveCallback;
    std::vector<std::unique_ptr<ModulationTargetSourceSlider>> _sliders;

    void _refreshSlotPositions();
};

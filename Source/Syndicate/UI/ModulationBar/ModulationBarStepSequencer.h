#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UIUtils.h"
#include "ModulatableParameter.hpp"

class ModulationBarStepSequencer : public juce::Component,
                                   public juce::Button::Listener,
                                   public juce::Slider::Listener,
                                   public juce::Timer {
public:
    ModulationBarStepSequencer(SyndicateAudioProcessor& processor, int seqIndex);
    ~ModulationBarStepSequencer() override;

    void resized() override;
    void buttonClicked(juce::Button* buttonThatWasClicked) override;
    void sliderValueChanged(juce::Slider* sliderThatWasMoved) override;
    void timerCallback() override;

private:
    // Draws the wave shape for a step and supports drag-to-adjust value
    class WaveShapeDisplay : public juce::Component,
                             public juce::SettableTooltipClient {
    public:
        WaveShapeDisplay(int stepNumber,
                         std::function<void(double)> onValueChanged,
                         std::function<int(bool)> getSelectedShape,
                         std::function<void(int)> onShapeChanged,
                         std::function<void(double)> onLengthChanged);

        void setStepState(double value, int shape, bool reverse, int repeat, double lengthMultiplier);
        void setPlaying(bool playing);
        void paint(juce::Graphics& g) override;
        void mouseDown(const juce::MouseEvent& e) override;
        void mouseDrag(const juce::MouseEvent& e) override;
        void mouseUp(const juce::MouseEvent& e) override;
        void mouseDoubleClick(const juce::MouseEvent& e) override;
        void mouseEnter(const juce::MouseEvent& e) override;
        void mouseExit(const juce::MouseEvent& e) override;

    private:
        enum class DragAxis { None, Value, Length };

        int _stepNumber {1};
        double _value {0.0};
        int _shape {1};
        bool _reverse {false};
        int _repeat {1};
        double _lengthMultiplier {1.0};
        bool _isPlaying {false};
        bool _isMouseOver {false};
        std::function<void(double)> _onValueChanged;
        std::function<int(bool)> _getSelectedShape;
        std::function<void(int)> _onShapeChanged;
        std::function<void(double)> _onLengthChanged;
        double _dragStartValue {0.0};
        int _dragStartLengthIndex {4};
        int _dragStartX {0};
        int _dragStartY {0};
        DragAxis _dragAxis {DragAxis::None};

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveShapeDisplay)
    };

    // A single cell in the shape palette grid
    class ShapePaletteCell : public juce::Component,
                             public juce::SettableTooltipClient {
    public:
        ShapePaletteCell(int shape, std::function<void(int)> onSelected);

        void paint(juce::Graphics& g) override;
        void mouseDown(const juce::MouseEvent& e) override;
        void setSelected(bool selected);

    private:
        int _shape;
        bool _isSelected {false};
        std::function<void(int)> _onSelected;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShapePaletteCell)
    };

    // A single step cell in the step grid
    class StepCell : public juce::Component {
    public:
        StepCell(SyndicateAudioProcessor& processor,
                 int seqIndex,
                 int stepIndex,
                 std::function<void()> onChanged,
                 std::function<int(bool)> getSelectedShape);
        ~StepCell() override;

        void resized() override;

        void refresh();
        void setPlaying(bool playing);
        double getLengthMultiplier() const;

    private:
        SyndicateAudioProcessor& _processor;
        int _seqIndex;
        int _stepIndex;
        std::function<void()> _onChanged;

        UIUtils::ToggleButtonLookAndFeel _buttonLookAndFeel;
        UIUtils::TempoSliderLookAndFeel _tempoSliderLookAndFeel;

        std::unique_ptr<WaveShapeDisplay> waveDisplay;
        std::unique_ptr<juce::TextButton> reverseButton;
        std::unique_ptr<juce::Slider> repeatSlider;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StepCell)
    };

    SyndicateAudioProcessor& _processor;
    int _seqIndex;

    UIUtils::StandardSliderLookAndFeel _sliderLookAndFeel;
    UIUtils::ToggleButtonLookAndFeel _buttonLookAndFeel;
    UIUtils::GridIconButtonLookAndFeel _gridIconButtonLookAndFeel;
    UIUtils::SettingsIconButtonLookAndFeel _settingsIconButtonLookAndFeel;
    UIUtils::TempoSliderLookAndFeel _tempoSliderLookAndFeel;

    bool _showingSequencerView {true};

    std::unique_ptr<juce::TextButton> settingsViewButton;
    std::unique_ptr<juce::TextButton> sequencerViewButton;

    std::unique_ptr<ModulatableParameter> freqSlider;
    std::unique_ptr<ModulatableParameter> depthSlider;
    std::unique_ptr<juce::TextButton> tempoSyncButton;
    std::unique_ptr<juce::Slider> tempoNumerSlider;
    std::unique_ptr<juce::Slider> tempoDenomSlider;
    std::unique_ptr<juce::Slider> stepCountSlider;

    int _selectedShape {0};
    bool _shapeAppliedOnce {false};
    int _lastPlayingStep {-1};
    std::vector<std::unique_ptr<ShapePaletteCell>> shapePaletteCells;
    std::unique_ptr<juce::Label> shapesLabel;

    std::unique_ptr<juce::Viewport> stepGridView;
    std::unique_ptr<juce::Component> stepGridContainer;
    std::vector<std::unique_ptr<StepCell>> stepCells;
    std::unique_ptr<juce::Label> stepsLabel;

    void _rebuildStepGrid();
    void _updateTempoToggles();
    void _updateStepCount();
    void _updateViewVisibility();
    void _updateShapePaletteSelection();
    int _getShapeForClick(bool cmdHeld);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulationBarStepSequencer)
};

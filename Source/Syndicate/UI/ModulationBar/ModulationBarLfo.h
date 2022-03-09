#pragma once

#include <JuceHeader.h>
#include "CoreJUCEPlugin/LabelReadoutSlider.h"
#include "RichterLFO/UI/RichterWaveViewer.h"
#include "RichterLFO/RichterLFO.h"
#include "UIUtils.h"

class ModulationBarLfo : public juce::Component,
                         public juce::Slider::Listener,
                         public juce::ComboBox::Listener,
                         public juce::Button::Listener {
public:
    ModulationBarLfo(std::shared_ptr<WECore::Richter::RichterLFO> lfo);
    ~ModulationBarLfo() override;

    void resized() override;
    void sliderValueChanged(juce::Slider* sliderThatWasMoved) override;
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;
    void buttonClicked(juce::Button* buttonThatWasClicked) override;

private:
    class TempoSliderLookAndFeel : public UIUtils::StandardSliderLookAndFeel {
    public:
        void drawButtonBackground(juce::Graphics& g,
                                  juce::Button& button,
                                  const juce::Colour& backgroundColour,
                                  bool isMouseOverButton,
                                  bool isButtonDown) override;

        void drawButtonText(juce::Graphics& g,
                            juce::TextButton& textButton,
                            bool isMouseOverButton,
                            bool isButtonDown) override;

        juce::Slider::SliderLayout getSliderLayout(juce::Slider& slider) override;

        juce::Label* createSliderTextBox(juce::Slider& slider) override;
    };

    std::shared_ptr<WECore::Richter::RichterLFO> _lfo;
    UIUtils::StandardSliderLookAndFeel _sliderLookAndFeel;
    UIUtils::ToggleButtonLookAndFeel _buttonLookAndFeel;
    UIUtils::StandardComboBoxLookAndFeel _comboBoxLookAndFeel;
    TempoSliderLookAndFeel _tempoSliderLookAndFeel;

    void _updateWaveView();
    void _updateTempoToggles();

    std::unique_ptr<WECore::JUCEPlugin::LabelReadoutSlider<double>> depthSlider;
    std::unique_ptr<WECore::JUCEPlugin::LabelReadoutSlider<double>> freqSlider;
    std::unique_ptr<juce::ComboBox> waveComboBox;
    std::unique_ptr<juce::Label> freqLabel;
    std::unique_ptr<juce::Label> depthLabel;
    std::unique_ptr<juce::TextButton> tempoSyncButton;
    std::unique_ptr<juce::Slider> tempoNumerSlider;
    std::unique_ptr<juce::Slider> tempoDenomSlider;
    std::unique_ptr<WECore::JUCEPlugin::LabelReadoutSlider<double>> phaseSlider;
    std::unique_ptr<juce::Label> phaseLabel;
    std::unique_ptr<WECore::Richter::WaveViewer> waveView;
    std::unique_ptr<juce::TextButton> invertButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulationBarLfo)
};

#include "UIUtils.h"

#include "ModulationBarLfo.h"

ModulationBarLfo::ModulationBarLfo(std::shared_ptr<WECore::Richter::RichterLFO> lfo) : _lfo(lfo) {

    namespace RP = WECore::Richter::Parameters;
    constexpr double INTERVAL {0.01};
    const juce::Colour& baseColour = UIUtils::getColourForModulationType(MODULATION_TYPE::LFO);

    depthSlider.reset(new WECore::JUCEPlugin::LabelReadoutSlider<double>("LFO Depth Slider"));
    addAndMakeVisible(depthSlider.get());
    depthSlider->setTooltip(TRANS("Depth of the LFO"));
    depthSlider->setRange(RP::DEPTH.minValue, RP::DEPTH.maxValue, INTERVAL);
    depthSlider->setDoubleClickReturnValue(true, WECore::Richter::Parameters::DEPTH.defaultValue);
    depthSlider->setSliderStyle(juce::Slider::RotaryVerticalDrag);
    depthSlider->setTextBoxStyle(juce::Slider::NoTextBox, false, 80, 20);
    depthSlider->addListener(this);
    depthSlider->setLookAndFeel(&_sliderLookAndFeel);
    depthSlider->setColour(juce::Slider::rotarySliderFillColourId, baseColour);

    freqSlider.reset(new WECore::JUCEPlugin::LabelReadoutSlider<double>("LFO Freq Slider"));
    addAndMakeVisible(freqSlider.get());
    freqSlider->setTooltip(TRANS("Frequency of the LFO in Hz"));
    freqSlider->setRange(RP::FREQ.minValue, RP::FREQ.maxValue, INTERVAL);
    freqSlider->setDoubleClickReturnValue(true, WECore::Richter::Parameters::FREQ.defaultValue);
    freqSlider->setSliderStyle(juce::Slider::RotaryVerticalDrag);
    freqSlider->setTextBoxStyle(juce::Slider::NoTextBox, false, 80, 20);
    freqSlider->addListener(this);
    freqSlider->setLookAndFeel(&_sliderLookAndFeel);
    freqSlider->setColour(juce::Slider::rotarySliderFillColourId, baseColour);

    waveComboBox.reset(new juce::ComboBox("LFO Wave"));
    addAndMakeVisible(waveComboBox.get());
    waveComboBox->setTooltip(TRANS("Wave shape for this LFO"));
    waveComboBox->setEditableText(false);
    waveComboBox->setJustificationType(juce::Justification::centredLeft);
    waveComboBox->setTextWhenNothingSelected(juce::String());
    waveComboBox->setTextWhenNoChoicesAvailable(TRANS("(no choices)"));
    waveComboBox->addItem(TRANS("Sine"), 1);
    waveComboBox->addItem(TRANS("Square"), 2);
    waveComboBox->addItem(TRANS("Saw"), 3);
    waveComboBox->addItem(TRANS("SC Comp"), 4);
    waveComboBox->setLookAndFeel(&_comboBoxLookAndFeel);
    waveComboBox->setColour(juce::ComboBox::textColourId, baseColour);
    waveComboBox->setColour(juce::ComboBox::arrowColourId, baseColour);
    waveComboBox->addListener(this);

    _comboBoxLookAndFeel.setColour(juce::PopupMenu::backgroundColourId, UIUtils::slotBackgroundColour);
    _comboBoxLookAndFeel.setColour(juce::PopupMenu::textColourId, baseColour);
    _comboBoxLookAndFeel.setColour(juce::PopupMenu::highlightedBackgroundColourId, baseColour);
    _comboBoxLookAndFeel.setColour(juce::PopupMenu::highlightedTextColourId, UIUtils::slotBackgroundColour);

    freqLabel.reset(new juce::Label("LFO Freq Label", TRANS("Rate")));
    addAndMakeVisible(freqLabel.get());
    UIUtils::setDefaultLabelStyle(freqLabel);

    depthLabel.reset(new juce::Label("LFO Depth Label", TRANS("Depth")));
    addAndMakeVisible(depthLabel.get());
    UIUtils::setDefaultLabelStyle(depthLabel);

    tempoSyncButton.reset(new juce::TextButton("LFO Tempo Sync Button"));
    addAndMakeVisible(tempoSyncButton.get());
    tempoSyncButton->setTooltip(TRANS("Sync LFO frequency to host tempo"));
    tempoSyncButton->setButtonText(TRANS("Tempo"));
    tempoSyncButton->setLookAndFeel(&_buttonLookAndFeel);
    tempoSyncButton->setColour(UIUtils::ToggleButtonLookAndFeel::backgroundColour, UIUtils::slotBackgroundColour);
    tempoSyncButton->setColour(UIUtils::ToggleButtonLookAndFeel::highlightColour, baseColour);
    tempoSyncButton->setColour(UIUtils::ToggleButtonLookAndFeel::disabledColour, UIUtils::deactivatedColour);
    tempoSyncButton->addListener(this);

    tempoNumerSlider.reset(new juce::Slider("LFO Tempo Numer Slider"));
    addAndMakeVisible(tempoNumerSlider.get());
    tempoNumerSlider->setTooltip(TRANS("Numerator of the tempo sync value"));
    tempoNumerSlider->setRange(1, 16, 1);
    tempoNumerSlider->setSliderStyle(juce::Slider::IncDecButtons);
    tempoNumerSlider->setTextBoxStyle(juce::Slider::TextBoxLeft, false, 40, 20);
    tempoNumerSlider->setColour(juce::Slider::textBoxTextColourId, baseColour);
    tempoNumerSlider->setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0x00000000));
    tempoNumerSlider->setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0x00000000));
    tempoNumerSlider->setColour(juce::TextButton::textColourOnId, baseColour);
    tempoNumerSlider->addListener(this);
    tempoNumerSlider->setIncDecButtonsMode(juce::Slider::incDecButtonsDraggable_Vertical);
    tempoNumerSlider->setLookAndFeel(&_tempoSliderLookAndFeel);

    tempoDenomSlider.reset(new juce::Slider("LFO Tempo Denom Slider"));
    addAndMakeVisible(tempoDenomSlider.get());
    tempoDenomSlider->setTooltip(TRANS("Denominator of the tempo sync value"));
    tempoDenomSlider->setRange(1, 32, 1);
    tempoDenomSlider->setSliderStyle(juce::Slider::IncDecButtons);
    tempoDenomSlider->setTextBoxStyle(juce::Slider::TextBoxLeft, false, 40, 20);
    tempoDenomSlider->setColour(juce::Slider::textBoxTextColourId, baseColour);
    tempoDenomSlider->setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0x00000000));
    tempoDenomSlider->setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0x00000000));
    tempoDenomSlider->setColour(juce::TextButton::textColourOnId, baseColour);
    tempoDenomSlider->addListener(this);
    tempoDenomSlider->setIncDecButtonsMode(juce::Slider::incDecButtonsDraggable_Vertical);
    tempoDenomSlider->setLookAndFeel(&_tempoSliderLookAndFeel);

    phaseSlider.reset(new WECore::JUCEPlugin::LabelReadoutSlider<double>("LFO Phase Slider"));
    addAndMakeVisible(phaseSlider.get());
    phaseSlider->setTooltip(TRANS("Phase shift the LFO by up to 360 degrees"));
    phaseSlider->setRange(RP::PHASE.minValue, RP::PHASE.maxValue, INTERVAL);
    phaseSlider->setDoubleClickReturnValue(true, WECore::Richter::Parameters::PHASE.defaultValue);
    phaseSlider->setSliderStyle(juce::Slider::RotaryVerticalDrag);
    phaseSlider->setTextBoxStyle(juce::Slider::NoTextBox, false, 80, 20);
    phaseSlider->setColour(juce::Slider::thumbColourId, juce::Colour(0xffbdffbb));
    phaseSlider->addListener(this);
    phaseSlider->setLookAndFeel(&_sliderLookAndFeel);
    phaseSlider->setColour(juce::Slider::rotarySliderFillColourId, baseColour);

    phaseLabel.reset(new juce::Label("LFO Phase Label", TRANS("Phase")));
    addAndMakeVisible(phaseLabel.get());
    UIUtils::setDefaultLabelStyle(phaseLabel);

    waveView.reset(new WECore::Richter::WaveViewer());
    addAndMakeVisible(waveView.get());
    waveView->setName("LFO Wave View");
    waveView->setTooltip(TRANS("Wave shape that will be output by this LFO"));
    waveView->setColour(WECore::Richter::WaveViewer::highlightColourId, baseColour);

    invertButton.reset(new juce::TextButton("LFO Invert Button"));
    addAndMakeVisible(invertButton.get());
    invertButton->setTooltip(TRANS("Invert the LFO output"));
    invertButton->setButtonText(TRANS("Invert"));
    invertButton->setLookAndFeel(&_buttonLookAndFeel);
    invertButton->setColour(juce::TextButton::buttonOnColourId, baseColour);
    invertButton->setColour(juce::TextButton::textColourOnId, UIUtils::backgroundColour);
    invertButton->setColour(juce::TextButton::textColourOffId, baseColour);
    invertButton->setColour(UIUtils::ToggleButtonLookAndFeel::backgroundColour, UIUtils::slotBackgroundColour);
    invertButton->setColour(UIUtils::ToggleButtonLookAndFeel::highlightColour, baseColour);
    invertButton->setColour(UIUtils::ToggleButtonLookAndFeel::disabledColour, UIUtils::deactivatedColour);
    invertButton->addListener(this);

    // Load UI state
    tempoSyncButton->setToggleState(_lfo->getTempoSyncSwitch(), juce::dontSendNotification);
    invertButton->setToggleState(_lfo->getInvertSwitch(), juce::dontSendNotification);
    waveComboBox->setSelectedId(_lfo->getWave(), juce::dontSendNotification);
    depthSlider->setValue(_lfo->getDepth(), juce::dontSendNotification);
    freqSlider->setValue(_lfo->getFreq(), juce::dontSendNotification);
    phaseSlider->setValue(_lfo->getManualPhase(), juce::dontSendNotification);
    tempoNumerSlider->setValue(_lfo->getTempoNumer(), juce::dontSendNotification);
    tempoDenomSlider->setValue(_lfo->getTempoDenom(), juce::dontSendNotification);

    // Start slider label readouts
    freqSlider->start(freqLabel.get(), freqLabel->getText());
    depthSlider->start(depthLabel.get(), depthLabel->getText());
    phaseSlider->start(phaseLabel.get(), phaseLabel->getText());

    _tempoSliderLookAndFeel.setColour(juce::TextButton::textColourOnId, baseColour);

    _updateTempoToggles();
    _updateWaveView();
}

ModulationBarLfo::~ModulationBarLfo() {
    // Stop slider label readouts
    freqSlider->stop();
    depthSlider->stop();
    phaseSlider->stop();

    freqSlider->setLookAndFeel(nullptr);
    depthSlider->setLookAndFeel(nullptr);
    phaseSlider->setLookAndFeel(nullptr);
    invertButton->setLookAndFeel(nullptr);
    waveComboBox->setLookAndFeel(nullptr);
    tempoNumerSlider->setLookAndFeel(nullptr);
    tempoDenomSlider->setLookAndFeel(nullptr);

    depthSlider = nullptr;
    freqSlider = nullptr;
    waveComboBox = nullptr;
    freqLabel = nullptr;
    depthLabel = nullptr;
    tempoSyncButton = nullptr;
    tempoNumerSlider = nullptr;
    tempoDenomSlider = nullptr;
    phaseSlider = nullptr;
    phaseLabel = nullptr;
    waveView = nullptr;
    invertButton = nullptr;
}

void ModulationBarLfo::resized() {
    juce::Rectangle<int> availableArea = getLocalBounds();
    availableArea.removeFromLeft(4);
    availableArea.removeFromRight(4);
    availableArea.removeFromTop(4);
    availableArea.removeFromBottom(4);

    constexpr int BUTTON_HEIGHT {24};

    // Freq/tempo area
    juce::Rectangle<int> freqArea = availableArea.removeFromLeft(availableArea.getWidth() / 5);

    // Enforce the max width for this section (otherwise buttons look silly)
    constexpr int MAX_FREQ_AREA_WIDTH {100};
    const int excessWidth {std::max(freqArea.getWidth() - MAX_FREQ_AREA_WIDTH, 0)};
    freqArea = freqArea.reduced(excessWidth / 2.0, 0);

    const int freqSliderSize {freqArea.getHeight() / 2 - BUTTON_HEIGHT};
    tempoSyncButton->setBounds(freqArea.removeFromTop(BUTTON_HEIGHT));

    // Copy this now, we'll need it later
    juce::Rectangle<int> tempoSlidersArea = freqArea;

    const int emptyHeight {freqArea.getHeight() - freqSliderSize - BUTTON_HEIGHT};
    freqArea.removeFromTop(emptyHeight / 2);
    freqArea.removeFromBottom(emptyHeight / 2);
    freqLabel->setBounds(freqArea.removeFromBottom(BUTTON_HEIGHT));
    freqSlider->setBounds(freqArea.removeFromBottom(freqSliderSize));

    // Now the tempo sliders
    juce::FlexBox tempoSlidersFlexBox;
    tempoSlidersFlexBox.flexDirection = juce::FlexBox::Direction::column;
    tempoSlidersFlexBox.flexWrap = juce::FlexBox::Wrap::wrap;
    tempoSlidersFlexBox.justifyContent = juce::FlexBox::JustifyContent::spaceAround;
    tempoSlidersFlexBox.alignContent = juce::FlexBox::AlignContent::center;

    tempoSlidersFlexBox.items.add(juce::FlexItem(*tempoNumerSlider.get()).withMinWidth(tempoSlidersArea.getWidth() * 0.75).withMinHeight(40));
    tempoSlidersFlexBox.items.add(juce::FlexItem(*tempoDenomSlider.get()).withMinWidth(tempoSlidersArea.getWidth() * 0.75).withMinHeight(40));

    tempoSlidersFlexBox.performLayout(tempoSlidersArea);

    // Depth/phase area
    juce::Rectangle<int> depthPhaseArea = availableArea.removeFromLeft(availableArea.getWidth() / 5);

    juce::Rectangle<int> depthArea = depthPhaseArea.removeFromTop(depthPhaseArea.getHeight() / 2);
    depthLabel->setBounds(depthArea.removeFromBottom(BUTTON_HEIGHT));
    depthSlider->setBounds(depthArea);

    phaseLabel->setBounds(depthPhaseArea.removeFromBottom(BUTTON_HEIGHT));
    phaseSlider->setBounds(depthPhaseArea);

    // Wave area
    constexpr int MAX_WAVE_AREA_WIDTH {450};
    const int excessWaveWidth {std::max(availableArea.getWidth() - MAX_WAVE_AREA_WIDTH, 0)};
    availableArea.removeFromLeft(excessWaveWidth / 2.0);
    availableArea.removeFromRight(excessWaveWidth / 2.0);

    juce::FlexBox waveFlexBox;
    waveFlexBox.flexDirection = juce::FlexBox::Direction::row;
    waveFlexBox.flexWrap = juce::FlexBox::Wrap::wrap;
    waveFlexBox.justifyContent = juce::FlexBox::JustifyContent::spaceAround;
    waveFlexBox.alignContent = juce::FlexBox::AlignContent::center;

    waveFlexBox.items.add(juce::FlexItem(*invertButton.get()).withMinWidth(availableArea.getWidth() / 3).withMinHeight(BUTTON_HEIGHT));
    waveFlexBox.items.add(juce::FlexItem(*waveComboBox.get()).withMinWidth(availableArea.getWidth() / 3).withMinHeight(BUTTON_HEIGHT));

    waveFlexBox.performLayout(availableArea.removeFromBottom(BUTTON_HEIGHT).toFloat());

    waveView->setBounds(availableArea);
}

void ModulationBarLfo::sliderValueChanged(juce::Slider* sliderThatWasMoved) {

    if (sliderThatWasMoved == depthSlider.get()) {
        _lfo->setDepth(depthSlider->getValue());
    } else if (sliderThatWasMoved == freqSlider.get()) {
        _lfo->setFreq(freqSlider->getValue());
    } else if (sliderThatWasMoved == tempoNumerSlider.get()) {
        _lfo->setTempoNumer(tempoNumerSlider->getValue());
    } else if (sliderThatWasMoved == tempoDenomSlider.get()) {
        _lfo->setTempoDenom(tempoDenomSlider->getValue());
    } else if (sliderThatWasMoved == phaseSlider.get()) {
        _lfo->setManualPhase(phaseSlider->getValue());
    }

    _updateWaveView();
}

void ModulationBarLfo::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) {
    if (comboBoxThatHasChanged == waveComboBox.get()) {
        _lfo->setWave(waveComboBox->getSelectedId());
    }

    _updateWaveView();
}

void ModulationBarLfo::buttonClicked(juce::Button* buttonThatWasClicked) {

    if (buttonThatWasClicked == tempoSyncButton.get()) {
        tempoSyncButton->setToggleState(!tempoSyncButton->getToggleState(), juce::dontSendNotification);
        _lfo->setTempoSyncSwitch(tempoSyncButton->getToggleState());
    } else if (buttonThatWasClicked == invertButton.get()) {
        invertButton->setToggleState(!invertButton->getToggleState(), juce::dontSendNotification);
        _lfo->setInvertSwitch(invertButton->getToggleState());
    }

    _updateTempoToggles();
    _updateWaveView();
}


void ModulationBarLfo::TempoSliderLookAndFeel::drawButtonBackground(
        juce::Graphics& /*g*/,
        juce::Button& /*button*/,
        const juce::Colour& /*backgroundColour*/,
        bool /*isMouseOverButton*/,
        bool /*isButtonDown*/) {
    // do nothing
}

void ModulationBarLfo::TempoSliderLookAndFeel::drawButtonText(juce::Graphics& g,
                                                              juce::TextButton& textButton,
                                                              bool /*isMouseOverButton*/,
                                                              bool /*isButtonDown*/) {
    g.setColour(findColour(juce::TextButton::textColourOnId));

    juce::Rectangle<int> area = textButton.getLocalBounds().withPosition(0, 0).reduced(8);

    constexpr int MAX_CARAT_WIDTH {10};
    const int excessWidth {std::max(area.getWidth() - MAX_CARAT_WIDTH, 0)};
    area = area.reduced(excessWidth / 2.0, 0);

    const int horizontalMid {area.getWidth() / 2 + area.getX()};
    const int centreY {area.getY() + (textButton.getButtonText() == "+" ? 0 : area.getHeight())};
    const int endY {area.getY() + (textButton.getButtonText() == "+" ? area.getHeight() : 0)};
    juce::Path p;

    p.startNewSubPath(area.getX(), endY);
    p.lineTo(horizontalMid, centreY);
    p.lineTo(area.getX() + area.getWidth(), endY);

    g.strokePath(p, juce::PathStrokeType(1));
}

juce::Slider::SliderLayout ModulationBarLfo::TempoSliderLookAndFeel::getSliderLayout(
            juce::Slider& slider) {
    juce::Rectangle<int> area = slider.getLocalBounds();

    juce::Slider::SliderLayout retVal;
    retVal.sliderBounds = area.removeFromRight(area.getWidth() / 2);
    retVal.textBoxBounds = area;

    return retVal;
}

juce::Label* ModulationBarLfo::TempoSliderLookAndFeel::createSliderTextBox(juce::Slider& slider) {
    juce::Label* label = UIUtils::StandardSliderLookAndFeel::createSliderTextBox(slider);

    // Use a bigger font
    label->setFont(juce::Font(20.00f, juce::Font::plain).withTypefaceStyle("Regular"));

    return label;
}

void ModulationBarLfo::_updateWaveView() {
    const double* wave {nullptr};

    if (_lfo->getWave() < 1.5) {
        wave = WECore::Richter::Wavetables::getInstance()->getSine();
    } else if (_lfo->getWave() < 2.5) {
        wave = WECore::Richter::Wavetables::getInstance()->getSquare();
    } else if (_lfo->getWave() < 3.5) {
        wave = WECore::Richter::Wavetables::getInstance()->getSaw();
    } else {
        wave = WECore::Richter::Wavetables::getInstance()->getSidechain();
    }

    waveView->setWave(wave, _lfo->getDepth(), _lfo->getManualPhase(), _lfo->getInvertSwitch());
    waveView->repaint();
}

void ModulationBarLfo::_updateTempoToggles() {
    if (tempoSyncButton->getToggleState()) {
        freqSlider->setVisible(false);
        freqLabel->setVisible(false);
        tempoNumerSlider->setVisible(true);
        tempoDenomSlider->setVisible(true);
    } else {
        freqSlider->setVisible(true);
        freqLabel->setVisible(true);
        tempoNumerSlider->setVisible(false);
        tempoDenomSlider->setVisible(false);
    }
}

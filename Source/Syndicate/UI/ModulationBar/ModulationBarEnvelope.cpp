#include "ParameterData.h"
#include "UIUtils.h"

#include "ModulationBarEnvelope.h"

EnvelopeViewer::EnvelopeViewer(std::shared_ptr<ModulationInterface::EnvelopeWrapper> envelope) : _envelope(envelope) {
    std::fill(_envelopeValues.begin(), _envelopeValues.end(), 0);

    start();
}

void EnvelopeViewer::paint(juce::Graphics& g) {
    const int XIncrement {static_cast<int>(getWidth() / _envelopeValues.size())};

    juce::Path p;

    const int height {getHeight()};
    auto envelopeValueToYPos = [&height](float value) {
        return height / 2 - value * height / 2;
    };

    p.startNewSubPath(0, envelopeValueToYPos(_envelopeValues[0]));

    for (int index {1}; index < _envelopeValues.size(); index++) {
        p.lineTo(index * XIncrement, envelopeValueToYPos(_envelopeValues[index]));
    }

    g.fillAll(UIUtils::backgroundColour);
    g.setColour(UIUtils::getColourForModulationType(MODULATION_TYPE::ENVELOPE));
    const juce::PathStrokeType pStroke(1);
    g.strokePath(p, pStroke);
}

void EnvelopeViewer::_onTimerCallback() {
    _stopEvent.reset();

    std::rotate(_envelopeValues.rbegin(), _envelopeValues.rbegin() + 1, _envelopeValues.rend());
    _envelopeValues[0] = _envelope->envelope.getLastOutput() * _envelope->amount;

    _stopEvent.signal();
}

ModulationBarEnvelope::ModulationBarEnvelope(std::shared_ptr<ModulationInterface::EnvelopeWrapper> envelope) : _envelope(envelope) {

    namespace AP = WECore::AREnv::Parameters;
    constexpr double INTERVAL {0.01};
    const juce::Colour& baseColour = UIUtils::getColourForModulationType(MODULATION_TYPE::ENVELOPE);

    attackSlider.reset(new WECore::JUCEPlugin::LabelReadoutSlider<double>("ENV Attack Slider"));
    addAndMakeVisible(attackSlider.get());
    attackSlider->setTooltip(TRANS("Attack of the envelope follower"));
    attackSlider->setRange(AP::ATTACK_MS.minValue, AP::ATTACK_MS.maxValue, INTERVAL);
    attackSlider->setDoubleClickReturnValue(true, AP::ATTACK_MS.defaultValue);
    attackSlider->setSliderStyle(juce::Slider::RotaryVerticalDrag);
    attackSlider->setTextBoxStyle(juce::Slider::NoTextBox, false, 80, 20);
    attackSlider->addListener(this);
    attackSlider->setSkewFactor(0.4);
    attackSlider->setLookAndFeel(&_sliderLookAndFeel);
    attackSlider->setColour(juce::Slider::rotarySliderFillColourId, baseColour);

    releaseSlider.reset(new WECore::JUCEPlugin::LabelReadoutSlider<double>("ENV RElease Slider"));
    addAndMakeVisible(releaseSlider.get());
    releaseSlider->setTooltip(TRANS("Release of the envelope follower"));
    releaseSlider->setRange(AP::RELEASE_MS.minValue, AP::RELEASE_MS.maxValue, INTERVAL);
    releaseSlider->setDoubleClickReturnValue(true, AP::RELEASE_MS.defaultValue);
    releaseSlider->setSliderStyle(juce::Slider::RotaryVerticalDrag);
    releaseSlider->setTextBoxStyle(juce::Slider::NoTextBox, false, 80, 20);
    releaseSlider->addListener(this);
    releaseSlider->setSkewFactor(0.4);
    releaseSlider->setLookAndFeel(&_sliderLookAndFeel);
    releaseSlider->setColour(juce::Slider::rotarySliderFillColourId, baseColour);

    amountSlider.reset(new WECore::JUCEPlugin::LabelReadoutSlider<double>("ENV Amount Slider"));
    addAndMakeVisible(amountSlider.get());
    amountSlider->setTooltip(TRANS("Amount of envelope follower modulation"));
    amountSlider->setRange(ENVELOPE_AMOUNT.minValue, ENVELOPE_AMOUNT.maxValue, INTERVAL);
    amountSlider->setDoubleClickReturnValue(true, ENVELOPE_AMOUNT.defaultValue);
    amountSlider->setSliderStyle(juce::Slider::RotaryVerticalDrag);
    amountSlider->setTextBoxStyle(juce::Slider::NoTextBox, false, 80, 20);
    amountSlider->addListener(this);
    amountSlider->setLookAndFeel(&_midAnchorSliderLookAndFeel);
    amountSlider->setColour(juce::Slider::rotarySliderFillColourId, baseColour);

    attackLabel.reset(new juce::Label("ENV Attack Label", TRANS("Attack")));
    addAndMakeVisible(attackLabel.get());
    UIUtils::setDefaultLabelStyle(attackLabel);

    releaseLabel.reset(new juce::Label("ENV Release Label", TRANS("Release")));
    addAndMakeVisible(releaseLabel.get());
    UIUtils::setDefaultLabelStyle(releaseLabel);

    amountLabel.reset(new juce::Label("ENV Amount Label", TRANS("Amount")));
    addAndMakeVisible(amountLabel.get());
    UIUtils::setDefaultLabelStyle(amountLabel);

    filterButton.reset(new juce::TextButton("ENV Filter Button"));
    addAndMakeVisible(filterButton.get());
    filterButton->setButtonText(TRANS("Filter"));
    filterButton->setTooltip(TRANS("Enable low/high pass filters on the envelope follower input"));
    filterButton->setLookAndFeel(&_buttonLookAndFeel);
    filterButton->setColour(juce::TextButton::buttonOnColourId, baseColour);
    filterButton->setColour(juce::TextButton::textColourOnId, UIUtils::backgroundColour);
    filterButton->setColour(juce::TextButton::textColourOffId, baseColour);
    filterButton->addListener(this);

    filterSlider.reset(new FilterSlider());
    addAndMakeVisible(filterSlider.get());
    filterSlider->setRange(AP::LOW_CUT.minValue, AP::LOW_CUT.maxValue, INTERVAL);
    filterSlider->setTooltip(TRANS("Low/high pass filter cutoff on the envelope follower input"));
    filterSlider->setSliderStyle(juce::Slider::TwoValueVertical);
    filterSlider->setTextBoxStyle(juce::Slider::NoTextBox, false, 80, 20);
    filterSlider->setSkewFactor(0.4);
    filterSlider->addListener(this);
    filterSlider->setLookAndFeel(&_filterSliderLookAndFeel);
    filterSlider->setColour(juce::Slider::thumbColourId, baseColour);
    filterSlider->setColour(juce::Slider::backgroundColourId, UIUtils::neutralDeactivatedColour);

    scInButton.reset(new juce::TextButton("ENV SC In Button"));
    addAndMakeVisible(scInButton.get());
    scInButton->setTooltip(TRANS("Use the sidechain input to trigger this envelope follower"));
    scInButton->setButtonText(TRANS("SC In"));
    scInButton->setLookAndFeel(&_buttonLookAndFeel);
    scInButton->setColour(juce::TextButton::buttonOnColourId, baseColour);
    scInButton->setColour(juce::TextButton::textColourOnId, UIUtils::backgroundColour);
    scInButton->setColour(juce::TextButton::textColourOffId, baseColour);
    scInButton->addListener(this);

    _envView.reset(new EnvelopeViewer(_envelope));
    addAndMakeVisible(_envView.get());
    _envView->setTooltip(TRANS("Output of this envelope follower"));

    // Start slider label readouts
    attackSlider->start(attackLabel.get(), attackLabel->getText());
    releaseSlider->start(releaseLabel.get(), releaseLabel->getText());
    amountSlider->start(amountLabel.get(), amountLabel->getText());
    filterSlider->start(filterButton.get(), filterButton->getButtonText());

    // Load UI state
    attackSlider->setValue(_envelope->envelope.getAttackTimeMs(), juce::dontSendNotification);
    releaseSlider->setValue(_envelope->envelope.getReleaseTimeMs(), juce::dontSendNotification);
    amountSlider->setValue(_envelope->amount, juce::dontSendNotification);
    filterSlider->setMinValue(_envelope->envelope.getLowCutHz(), juce::dontSendNotification);
    filterSlider->setMaxValue(_envelope->envelope.getHighCutHz(), juce::dontSendNotification);
    filterButton->setToggleState(_envelope->envelope.getFilterEnabled(), juce::dontSendNotification);
    filterSlider->setEnabled(_envelope->envelope.getFilterEnabled());
    scInButton->setToggleState(_envelope->useSidechainInput, juce::dontSendNotification);
}

ModulationBarEnvelope::~ModulationBarEnvelope() {
    // Stop slider label readouts
    attackSlider->stop();
    releaseSlider->stop();
    amountSlider->stop();

    attackSlider->setLookAndFeel(nullptr);
    releaseSlider->setLookAndFeel(nullptr);
    amountSlider->setLookAndFeel(nullptr);
    filterButton->setLookAndFeel(nullptr);
    scInButton->setLookAndFeel(nullptr);

    attackSlider = nullptr;
    releaseSlider = nullptr;
    amountSlider = nullptr;
    attackLabel = nullptr;
    releaseLabel = nullptr;
    amountLabel = nullptr;
    filterButton = nullptr;
    filterSlider = nullptr;
    scInButton = nullptr;
    _envView = nullptr;
}

void ModulationBarEnvelope::resized() {
    juce::Rectangle<int> availableArea = getLocalBounds();
    availableArea.removeFromLeft(4);
    availableArea.removeFromRight(4);
    availableArea.removeFromTop(4);
    availableArea.removeFromBottom(4);

    juce::Rectangle<int> inputArea = availableArea.removeFromLeft(availableArea.getWidth() / 5);

    // Enforce the max width for this section (otherwise buttons look silly)
    constexpr int MAX_INPUT_AREA_WIDTH {100};
    const int excessWidth {std::max(inputArea.getWidth() - MAX_INPUT_AREA_WIDTH, 0)};
    inputArea.removeFromLeft(excessWidth / 2.0);
    inputArea.removeFromRight(excessWidth / 2.0 - 4);

    filterButton->setBounds(inputArea.removeFromTop(24));
    inputArea.removeFromTop(4);
    scInButton->setBounds(inputArea.removeFromBottom(24));
    filterSlider->setBounds(inputArea);

    juce::Rectangle<int> controlArea = availableArea.removeFromLeft((availableArea.getWidth() * 2) / 5);
    juce::Rectangle<int> upperControlArea = controlArea.removeFromTop(controlArea.getHeight() / 2);

    juce::Rectangle<int> attackArea = upperControlArea.removeFromLeft(upperControlArea.getWidth() / 2);
    attackLabel->setBounds(attackArea.removeFromBottom(24));
    attackSlider->setBounds(attackArea);

    releaseLabel->setBounds(upperControlArea.removeFromBottom(24));
    releaseSlider->setBounds(upperControlArea);

    amountLabel->setBounds(controlArea.removeFromBottom(24));
    amountSlider->setBounds(controlArea);

    _envView->setBounds(availableArea);
}

void ModulationBarEnvelope::sliderValueChanged(juce::Slider* sliderThatWasMoved) {
    if (sliderThatWasMoved == attackSlider.get()) {
        _envelope->envelope.setAttackTimeMs(attackSlider->getValue());
    } else if (sliderThatWasMoved == releaseSlider.get()) {
        _envelope->envelope.setReleaseTimeMs(releaseSlider->getValue());
    } else if (sliderThatWasMoved == amountSlider.get()) {
        _envelope->amount = amountSlider->getValue();
    } else if (sliderThatWasMoved == filterSlider.get()) {
        _envelope->envelope.setLowCutHz(filterSlider->getMinValue());
        _envelope->envelope.setHighCutHz(filterSlider->getMaxValue());
    }
}

void ModulationBarEnvelope::buttonClicked(juce::Button* buttonThatWasClicked) {
    if (buttonThatWasClicked == filterButton.get()) {
        filterButton->setToggleState(!filterButton->getToggleState(), juce::dontSendNotification);
        _envelope->envelope.setFilterEnabled(filterButton->getToggleState());
        filterSlider->setEnabled(filterButton->getToggleState());
    } else if (buttonThatWasClicked == scInButton.get()) {
        scInButton->setToggleState(!scInButton->getToggleState(), juce::dontSendNotification);
        _envelope->useSidechainInput = scInButton->getToggleState();
    }
}

void ModulationBarEnvelope::FilterSliderLookAndFeel::drawLinearSliderThumb(
        juce::Graphics& g,
        int /*x*/,
        int /*y*/,
        int /*width*/,
        int /*height*/,
        float /*sliderPos*/,
        float minSliderPos,
        float maxSliderPos,
        const juce::Slider::SliderStyle,
        juce::Slider& slider) {
    if (slider.isEnabled()) {
        g.setColour(slider.findColour(juce::Slider::thumbColourId));
    } else {
        g.setColour(slider.findColour(juce::Slider::backgroundColourId));
    }

    juce::Path p;

    const int thumbLeftXPos {static_cast<int>(slider.getWidth() / 2 - 1.5 * getSliderThumbRadius(slider))};
    const int thumbRightXPos {slider.getWidth() - thumbLeftXPos};

    const int columnLeftXPos {static_cast<int>(slider.getWidth() / 2 - getSliderThumbRadius(slider) / 2)};
    const int columnRightXPos {slider.getWidth() - columnLeftXPos};
    constexpr int halfThumbThickness {1};
    const int cornerRadius {2};

    // Start in the top right and go clockwise
    const int topEdgeYPos {static_cast<int>(maxSliderPos - halfThumbThickness)};

    p.startNewSubPath(thumbRightXPos, topEdgeYPos);
    p.lineTo(thumbRightXPos, maxSliderPos + halfThumbThickness);
    p.lineTo(columnRightXPos, maxSliderPos + halfThumbThickness);

    // Bottom right rounded corner
    const int bottomEdgeYPos {static_cast<int>(minSliderPos + halfThumbThickness)};

    p.lineTo(columnRightXPos, bottomEdgeYPos - cornerRadius);
    p.addCentredArc(columnRightXPos - cornerRadius,
                    bottomEdgeYPos - cornerRadius,
                    cornerRadius,
                    cornerRadius,
                    0,
                    WECore::CoreMath::DOUBLE_PI / 2,
                    WECore::CoreMath::DOUBLE_PI);

    // Bottom left thumb
    p.lineTo(thumbLeftXPos, bottomEdgeYPos);
    p.lineTo(thumbLeftXPos, minSliderPos - halfThumbThickness);
    p.lineTo(columnLeftXPos, minSliderPos - halfThumbThickness);

    // Top left rounded corner
    p.lineTo(columnLeftXPos, topEdgeYPos + cornerRadius);
    p.addCentredArc(columnLeftXPos + cornerRadius,
                    topEdgeYPos + cornerRadius,
                    cornerRadius,
                    cornerRadius,
                    0,
                    1.5 * WECore::CoreMath::DOUBLE_PI,
                    2 * WECore::CoreMath::DOUBLE_PI);
    p.closeSubPath();
    g.strokePath(p, juce::PathStrokeType(1.0f));
    g.fillPath(p);
}

int ModulationBarEnvelope::FilterSliderLookAndFeel::getSliderThumbRadius(juce::Slider& slider) {
    return slider.getHeight() / 8;
}

ModulationBarEnvelope::FilterSlider::FilterSlider() :
        WECore::JUCEPlugin::LabelReadoutSlider<double>("Filter Slider") {
}

void ModulationBarEnvelope::FilterSlider::_updateLabel(const juce::MouseEvent& event) {
    if (_isRunning && isEnabled()) {
        const float minPosition {getPositionOfValue(getMinValue())};
        const float maxPosition {getPositionOfValue(getMaxValue())};

        const bool closerToMin {
            std::abs(event.y - minPosition) < std::abs(event.y - maxPosition)
        };

        const juce::String valueString(
            static_cast<int>(closerToMin ? getMinValue() : getMaxValue()));
        _targetCallback(valueString + " Hz");
    }
}

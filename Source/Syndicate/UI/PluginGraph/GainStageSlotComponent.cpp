#include "GainStageSlotComponent.h"
#include "ParameterData.h"
#include "UIUtils.h"

namespace {
    int dBToXPos(float dBValue, int meterWidth) {
        constexpr float MIN_DB {-60};
        constexpr float MAX_DB {20};
        constexpr float meterRange = MAX_DB - MIN_DB;

        return std::max(((std::min(dBValue, MAX_DB) - MIN_DB) / meterRange) * meterWidth, 0.0f);
    }
}

GainStageMeter::GainStageMeter(const GainStageLevelsInterface levelsProvider) :
            _levelsProvider(levelsProvider) {
    start();
}

void GainStageMeter::paint(juce::Graphics& g) {
    _stopEvent.reset();

    const int zeroLineXPos = dBToXPos(0, getWidth());

    // Draw the meter for each channel
    const int numChannels {_levelsProvider.getNumChannels()};
    constexpr int MARGIN {4};
    juce::Rectangle<int> availableArea = getLocalBounds();
    const int availableHeight {getHeight() - ((1 + numChannels) * MARGIN)};
    const int channelHeight {availableHeight / numChannels};


    for (int channel {0}; channel < numChannels; channel++) {
        const float gaindB = WECore::CoreMath::linearTodB(_levelsProvider.getOutputAmplitude(channel));
        const int meterWidth = dBToXPos(gaindB, getWidth());

        availableArea.removeFromTop(MARGIN);
        juce::Rectangle<int> meterArea = availableArea.removeFromTop(channelHeight).withWidth(meterWidth);

        // Draw the zero line
        g.setColour(juce::Colours::grey);
        g.drawLine(zeroLineXPos, meterArea.getY(), zeroLineXPos, meterArea.getY() + meterArea.getHeight());

        // Draw the lower part of the meter
        g.setColour(UIUtils::neutralControlColour.withBrightness(0.5));
        g.fillRect(meterArea.removeFromLeft(std::min(meterArea.getWidth(), zeroLineXPos)));

        // Draw the area above 0dB
        if (meterWidth > zeroLineXPos) {
            g.setColour(UIUtils::neutralControlColour.withLightness(0.5).withBrightness(0.5));
            g.fillRect(meterArea);
        }
    }

    _stopEvent.signal();
}

GainStageSlotComponent::GainStageSlotComponent(
        PluginSelectionInterface& pluginSelectionInterface,
        int chainNumber,
        int slotNumber) : BaseSlotComponent(chainNumber, slotNumber),
                          _pluginSelectionInterface(pluginSelectionInterface) {

    _buttonLookAndFeel.reset(new UIUtils::TextOnlyButtonLookAndFeel());

    std::optional<GainStageLevelsInterface> levelsInterface =
        pluginSelectionInterface.getGainStageLevelsInterface(_chainNumber, _slotNumber);

    const juce::String meterTooltip("Output of this gain stage");

    if (levelsInterface.has_value()) {
        levelMeter.reset(new GainStageMeter(levelsInterface.value()));
        addAndMakeVisible(levelMeter.get());
        levelMeter->setTooltip(meterTooltip);
    } else {
        juce::Logger::writeToLog("GainStageSlotComponent::GainStageSlotComponent: Missing levels provider");
    }

    valueLabel.reset(new juce::Label("Value Label", ""));
    addAndMakeVisible(valueLabel.get());
    valueLabel->setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    valueLabel->setJustificationType(juce::Justification::centred);
    valueLabel->setEditable(true, true, false);
    valueLabel->setColour(juce::Label::textColourId, UIUtils::neutralControlColour);

    // This is set to match the tooltip of the meter since it's overlaid on the meter
    valueLabel->setTooltip(meterTooltip);

    gainSld.reset(new WECore::JUCEPlugin::LabelReadoutSlider<double>("Gain Slider"));
    addAndMakeVisible(gainSld.get());
    gainSld->setRange(OUTPUTGAIN.minValue, OUTPUTGAIN.maxValue, 0);
    gainSld->setDoubleClickReturnValue(true, OUTPUTGAIN.defaultValue);
    gainSld->setSliderStyle(juce::Slider::RotaryVerticalDrag);
    gainSld->setTextBoxStyle(juce::Slider::NoTextBox, false, 80, 20);
    gainSld->addListener(this);
    gainSld->setColour(juce::Slider::rotarySliderFillColourId, UIUtils::neutralControlColour);
    gainSld->setLookAndFeel(&_gainSliderLookAndFeel);
    gainSld->setTooltip(TRANS("Gain applied by this gain stage"));

    panSld.reset(new WECore::JUCEPlugin::LabelReadoutSlider<double>("Pan Slider"));
    addAndMakeVisible(panSld.get());
    panSld->setRange(-1, 1, 0);
    panSld->setDoubleClickReturnValue(true, 0);
    panSld->setSliderStyle(juce::Slider::RotaryVerticalDrag);
    panSld->setTextBoxStyle(juce::Slider::NoTextBox, false, 80, 20);
    panSld->addListener(this);
    panSld->setColour(juce::Slider::rotarySliderFillColourId, UIUtils::neutralControlColour);
    panSld->setLookAndFeel(&_panSliderLookAndFeel);
    panSld->setTooltip(TRANS("Balance applied by this gain stage (if in stereo)"));

    removeBtn.reset(new UIUtils::CrossButton("Remove Button"));
    addAndMakeVisible(removeBtn.get());
    removeBtn->setTooltip(TRANS("Removes this gain stage"));
    removeBtn->setColour(juce::TextButton::buttonOnColourId, UIUtils::neutralControlColour);
    removeBtn->addListener(this);

    // Initialise the slider values
    // (we don't need to set this again in an onParameterUpdate() or anything like that as nothing
    // else can change the internal value but this slider)
    auto [linearGain, pan] = _pluginSelectionInterface.getGainStageGainAndPan(_chainNumber, _slotNumber);
    const double logGain {WECore::CoreMath::linearTodB(linearGain)};
    gainSld->setValue(logGain, juce::dontSendNotification);

    panSld->setValue(pan, juce::dontSendNotification);

    if (levelsInterface.has_value()) {
        panSld->setEnabled(levelsInterface.value().getNumChannels() == 2);
    }

    gainSld->setValueToString([](double value) { return juce::String(value, 1) + " dB";});
    gainSld->start(valueLabel.get(), valueLabel->getText());
    panSld->start(valueLabel.get(), valueLabel->getText());
}

GainStageSlotComponent::~GainStageSlotComponent() {
    gainSld->setLookAndFeel(nullptr);
    panSld->setLookAndFeel(nullptr);
    removeBtn->setLookAndFeel(nullptr);

    gainSld->stop();
    panSld->stop();

    levelMeter = nullptr;
    valueLabel = nullptr;
    gainSld = nullptr;
    panSld = nullptr;
    removeBtn = nullptr;
}

void GainStageSlotComponent::resized() {
    BaseSlotComponent::resized();

    juce::Rectangle<int> availableArea = getAvailableSlotArea().reduced(1, 1);

    const int meterWidth {availableArea.getWidth() / 2};
    valueLabel->setBounds(availableArea.withWidth(dBToXPos(0, meterWidth)));

    if (levelMeter != nullptr) {
        levelMeter->setBounds(availableArea.removeFromLeft(meterWidth));
    }

    const int elementWidth {availableArea.getWidth() / 3};
    gainSld->setBounds(availableArea.removeFromLeft(elementWidth));
    panSld->setBounds(availableArea.removeFromLeft(elementWidth));
    removeBtn->setBounds(availableArea.removeFromLeft(elementWidth));
}

void GainStageSlotComponent::sliderValueChanged(juce::Slider* sliderThatWasMoved) {
    if (sliderThatWasMoved == gainSld.get()) {
        // Convert log gain to linear
        const double linearGain {WECore::CoreMath::dBToLinear(gainSld->getValue())};
        _pluginSelectionInterface.setGainStageGain(_chainNumber, _slotNumber, linearGain);
    } else if (sliderThatWasMoved == panSld.get()) {
        _pluginSelectionInterface.setGainStagePan(_chainNumber, _slotNumber, panSld->getValue());
    }
}

void GainStageSlotComponent::buttonClicked(juce::Button* buttonThatWasClicked) {
    if (buttonThatWasClicked == removeBtn.get()) {
        // Stop the meter before deleting anything that it relies on
        levelMeter->stop();

        _pluginSelectionInterface.removePlugin(_chainNumber, _slotNumber);
    }
}

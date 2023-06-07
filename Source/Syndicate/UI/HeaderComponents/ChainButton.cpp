#include "ChainButton.h"
#include "UIUtils.h"

void ChainButtonLookAndFeel::drawButtonBackground(juce::Graphics& g,
                                                  juce::Button& button,
                                                  const juce::Colour& /*backgroundColour*/,
                                                  bool /*shouldDrawButtonAsHighlighted*/,
                                                  bool /*shouldDrawButtonAsDown*/) {
    const juce::Rectangle<float> area = button.getLocalBounds().reduced(1, 1).toFloat();

    // Draw the outline
    g.setColour(button.findColour(ChainButton::buttonOnColourId));
    g.drawEllipse(area, 1.0f);

    // Fill if needed
    if (button.getToggleState()) {
        g.fillEllipse(area);
    }
}

void ChainButtonLookAndFeel::drawButtonText(juce::Graphics& g,
                                            juce::TextButton& button,
                                            bool /*shouldDrawButtonAsHighlighted*/,
                                            bool /*shouldDrawButtonAsDown*/) {
    g.setColour(button.findColour(button.getToggleState() ? ChainButton::textColourOnId : ChainButton::textColourOffId));
    g.drawText(button.getButtonText(), button.getLocalBounds().reduced(2), juce::Justification::centred, false);
}

ChainButton::ChainButton(CHAIN_BUTTON_TYPE type) {
    setLookAndFeel(&_lookAndFeel);

    setColour(buttonOnColourId, UIUtils::neutralControlColour);
    setColour(textColourOnId, UIUtils::PLUGIN_SLOT_MOD_TRAY_BG_COLOUR);
    setColour(textColourOffId, UIUtils::neutralControlColour);

    switch (type) {
        case CHAIN_BUTTON_TYPE::BYPASS:
            setButtonText("B");
            break;
        case CHAIN_BUTTON_TYPE::MUTE:
            setButtonText("M");
            break;
        case CHAIN_BUTTON_TYPE::SOLO:
            setButtonText("S");
            break;
    }
}

ChainButton::~ChainButton() {
    setLookAndFeel(nullptr);
}

#include "ChainButton.h"

void ChainButtonLookAndFeel::drawButtonBackground(juce::Graphics& g,
                                                  juce::Button& button,
                                                  const juce::Colour& /*backgroundColour*/,
                                                  bool /*shouldDrawButtonAsHighlighted*/,
                                                  bool /*shouldDrawButtonAsDown*/) {
    constexpr int CORNER_RADIUS {2};
    constexpr int LINE_THICKNESS {1};

    // Draw translucent background
    const juce::Colour buttonBackground(static_cast<uint8_t>(0), 0, 0, 0.5f);
    g.setColour(buttonBackground);
    g.fillRoundedRectangle(0, 0, button.getWidth(), button.getHeight(), CORNER_RADIUS);

    // Draw button outline
    g.setColour(button.findColour(button.getToggleState() ? ChainButton::buttonOnColourId : ChainButton::buttonColourId));
    g.drawRoundedRectangle(0, 0, button.getWidth(), button.getHeight(), CORNER_RADIUS, LINE_THICKNESS);
}

void ChainButtonLookAndFeel::drawButtonText(juce::Graphics& g,
                                            juce::TextButton& button,
                                            bool /*shouldDrawButtonAsHighlighted*/,
                                            bool /*shouldDrawButtonAsDown*/) {
    g.setColour(button.findColour(button.getToggleState() ? ChainButton::buttonOnColourId : ChainButton::buttonColourId));
    g.drawText(button.getButtonText(), 0, 0, button.getWidth(), button.getHeight(), juce::Justification::centred);
}

ChainButton::ChainButton(CHAIN_BUTTON_TYPE type) {
    setLookAndFeel(&_lookAndFeel);

    setColour(buttonColourId, juce::Colour(200, 200, 200));

    switch (type) {
        case CHAIN_BUTTON_TYPE::BYPASS:
            setColour(buttonOnColourId, juce::Colour(252, 252, 22));
            setButtonText("B");
            break;
        case CHAIN_BUTTON_TYPE::MUTE:
            setColour(buttonOnColourId, juce::Colour(252, 0, 0));
            setButtonText("M");
            break;
        case CHAIN_BUTTON_TYPE::SOLO:
            setColour(buttonOnColourId, juce::Colour(252, 137, 22));
            setButtonText("S");
            break;
    }
}

ChainButton::~ChainButton() {
    setLookAndFeel(nullptr);
}

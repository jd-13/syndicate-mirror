#pragma once

#include <JuceHeader.h>

struct SelectorComponentStyle {
    const juce::Colour backgroundColour;
    const juce::Colour neutralColour;
    const juce::Colour controlColour;
    const std::unique_ptr<juce::LookAndFeel> searchBarLookAndFeel;
    const std::unique_ptr<juce::LookAndFeel> headerButtonLookAndFeel;
    const std::unique_ptr<juce::LookAndFeel> scanButtonLookAndFeel;
    const std::unique_ptr<juce::LookAndFeel> tableHeaderLookAndFeel;

    SelectorComponentStyle(juce::Colour newBackgroundColour,
                           juce::Colour newNeutralColour,
                           juce::Colour newControlColour,
                           std::unique_ptr<juce::LookAndFeel> newSearchBarLookAndFeel,
                           std::unique_ptr<juce::LookAndFeel> newHeaderButtonLookAndFeel,
                           std::unique_ptr<juce::LookAndFeel> newScanButtonLookAndFeel,
                           std::unique_ptr<juce::LookAndFeel> newTableHeaderLookAndFeel) :
            backgroundColour(newBackgroundColour),
            neutralColour(newNeutralColour),
            controlColour(newControlColour),
            searchBarLookAndFeel(std::move(newSearchBarLookAndFeel)),
            headerButtonLookAndFeel(std::move(newHeaderButtonLookAndFeel)),
            scanButtonLookAndFeel(std::move(newScanButtonLookAndFeel)),
            tableHeaderLookAndFeel(std::move(newTableHeaderLookAndFeel)) { }
};

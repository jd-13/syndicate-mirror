#pragma once

#include "MONSTRFilters/MONSTRParameters.h"

namespace UIUtils::Crossover {
    constexpr int SLIDER_THUMB_RADIUS {6};
    constexpr int SLIDER_THUMB_TARGET_WIDTH {SLIDER_THUMB_RADIUS * 2};

    constexpr int BAND_BUTTON_WIDTH {20};
    constexpr int BAND_BUTTON_PADDING {4};

    const juce::Colour lightGrey(200, 200, 200);
    const juce::Colour darkGrey(107, 107, 107);
    const juce::Colour lightGreyTrans(static_cast<uint8_t>(200), 200, 200, 0.5f);
    const juce::Colour mainHighlight(135, 252, 2);
    const juce::Colour transHighlight(static_cast<uint8_t>(135), 252, 2, 0.5f);
    const juce::Colour imagerColour(static_cast<uint8_t>(255), 255, 255, 0.2f);

    // These are tuned experimentally to get the desired log curve and crossings close to 0,0 and 1,1.
    constexpr double LOG_SCALE {3.00043};
    constexpr double LOG_OFFSET_1 {0.001};
    constexpr double LOG_OFFSET_2 {3};

    /**
     * Maps a linear scaled value in the range 0:1 to a log scaled value in the same range.
     */
    inline double sliderValueToInternalLog(double sliderValue) {
        return std::pow(10, LOG_SCALE * sliderValue - LOG_OFFSET_2) - LOG_OFFSET_1;
    }

    /**
     * Maps a log scaled value in the range 0:1 to a linear scaled value in the same range.
     */
    inline double internalLogToSliderValue(double internalValue) {
        return (std::log10(internalValue + LOG_OFFSET_1) + LOG_OFFSET_2) / LOG_SCALE;
    }

    /**
     * Converts a crossover frequency value between in the internal parameter range to an x coordinate.
     */
    inline double sliderValueToXPos(double sliderValue, int componentWidth) {
        const double MARGIN_PX {componentWidth * 0.05};
        const double realRange {componentWidth - 2 * MARGIN_PX};

        const double normalisedSliderValue {WECore::MONSTR::Parameters::CROSSOVER_FREQUENCY.InternalToNormalised(sliderValue)};

        return (internalLogToSliderValue(normalisedSliderValue) * realRange) + MARGIN_PX;
    }

    /**
     * Converts an x coordinate to a crossover frequency parameter value in the internal parameter range.
     */
    inline double XPosToSliderValue(int XPos, int componentWidth) {
        const double MARGIN_PX {componentWidth * 0.05};
        const double realRange {componentWidth - 2 * MARGIN_PX};

        return WECore::MONSTR::Parameters::CROSSOVER_FREQUENCY.NormalisedToInternal(sliderValueToInternalLog(std::max(XPos - MARGIN_PX, 0.0) / realRange));
    }

    /**
     * Returns a rectangle representing the position of a button for the given crossover position
     * and index.
     */
    inline juce::Rectangle<float> getButtonBounds(double crossoverXPos, int index) {
        return juce::Rectangle<float>(crossoverXPos - BAND_BUTTON_WIDTH - SLIDER_THUMB_RADIUS,
                                BAND_BUTTON_PADDING + index * (BAND_BUTTON_PADDING + BAND_BUTTON_WIDTH),
                                BAND_BUTTON_WIDTH,
                                BAND_BUTTON_WIDTH);
    }
}

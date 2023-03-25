#include "CrossoverMouseListener.h"

#include <algorithm>

#include "UIUtilsCrossover.h"
#include "WEFilters/StereoWidthProcessorParameters.h"

namespace {
    std::function<void(const juce::MouseEvent&)> createDragCallback(SyndicateAudioProcessor& processor, int crossoverNumber) {
        return [&processor, crossoverNumber](const juce::MouseEvent& event) {
            constexpr double MIN_SPACING {
                2 * UIUtils::Crossover::SLIDER_THUMB_RADIUS + UIUtils::BAND_BUTTON_WIDTH
            };

            const int currentXPos {event.getPosition().getX()};
            const int componentWidth {event.eventComponent->getWidth()};

            processor.setCrossoverFrequency(crossoverNumber, UIUtils::Crossover::XPosToSliderValue(currentXPos, componentWidth));

            // Check if this crossover handle is getting too close to another and move it if
            // needed
            const size_t numCrossovers {SplitterInterface::getNumChains(processor.splitter) - 1};
            for (int otherCrossoverNumber {0}; otherCrossoverNumber < numCrossovers; otherCrossoverNumber++) {
                const double otherXPos {
                    UIUtils::Crossover::sliderValueToXPos(SplitterInterface::getCrossoverFrequency(processor.splitter, otherCrossoverNumber), componentWidth)
                };

                const double requiredGap {MIN_SPACING * std::abs(crossoverNumber - otherCrossoverNumber)};
                const double actualGap {std::abs(currentXPos - otherXPos)};

                if (otherCrossoverNumber < crossoverNumber && actualGap < requiredGap) {
                    processor.setCrossoverFrequency(otherCrossoverNumber, UIUtils::Crossover::XPosToSliderValue(currentXPos - requiredGap, componentWidth));
                } else if (otherCrossoverNumber > crossoverNumber && actualGap < requiredGap) {
                    processor.setCrossoverFrequency(otherCrossoverNumber, UIUtils::Crossover::XPosToSliderValue(currentXPos + requiredGap, componentWidth));
                }
            }
        };
    }
}

CrossoverMouseListener::CrossoverMouseListener(SyndicateAudioProcessor& processor)
        : _processor(processor) {
}

CrossoverMouseListener::~CrossoverMouseListener() {
}

void CrossoverMouseListener::mouseDown(const juce::MouseEvent& event) {
    _resolveParameterInteraction(event);
}

void CrossoverMouseListener::mouseDrag(const juce::MouseEvent& event) {
    if (_dragCallback.has_value()) {
        _dragCallback.value()(event);
    }
}

void CrossoverMouseListener::mouseUp(const juce::MouseEvent& /*event*/) {
    if (_dragCallback.has_value()) {
        _dragCallback.reset();
    }
}

void CrossoverMouseListener::_resolveParameterInteraction(const juce::MouseEvent& event) {
    const int mouseDownX {event.getMouseDownX()};
    const int mouseDownY {event.getMouseDownY()};

    // For each available band, check if the cursor landed on a crossover frequency handle or on
    // the gaps in between
    const size_t numBands {SplitterInterface::getNumChains(_processor.splitter)};

    for (size_t bandIndex {0}; bandIndex < numBands; bandIndex++) {
        const double crossoverXPos {bandIndex < numBands - 1 ?
            UIUtils::Crossover::sliderValueToXPos(SplitterInterface::getCrossoverFrequency(_processor.splitter, bandIndex), event.eventComponent->getWidth()) :
            event.eventComponent->getWidth()
        };

        ChainParameters& thisChainParameter = _processor.chainParameters[bandIndex];
        if (UIUtils::Crossover::getButtonBounds(crossoverXPos, 0).contains(mouseDownX, mouseDownY)) {
            // Landed on the bypass button
            thisChainParameter.setBypass(!thisChainParameter.getBypass());
            break;

        } else if (UIUtils::Crossover::getButtonBounds(crossoverXPos, 1).contains(mouseDownX, mouseDownY)) {
            // Landed on the mute button
            thisChainParameter.setMute(!thisChainParameter.getMute());
            break;

        } else if (UIUtils::Crossover::getButtonBounds(crossoverXPos, 2).contains(mouseDownX, mouseDownY)) {
            // Landed on the solo button
            thisChainParameter.setSolo(!thisChainParameter.getSolo());
            break;

        } else if (mouseDownX < crossoverXPos - UIUtils::Crossover::SLIDER_THUMB_TARGET_WIDTH) {
            // Click landed below a crossover handle but outside a button - do nothing
            break;

        } else if (mouseDownX < crossoverXPos + UIUtils::Crossover::SLIDER_THUMB_TARGET_WIDTH) {
            // Click landed on a crossover handle
            _dragCallback = createDragCallback(_processor, bandIndex);
            break;
        }
    }
}

#include "CrossoverMouseListener.h"

#include <algorithm>

#include "UIUtilsCrossover.h"
#include "WEFilters/StereoWidthProcessorParameters.h"

CrossoverMouseListener::CrossoverMouseListener(SyndicateAudioProcessor& processor)
        : _processor(processor),
          _dragParameter(nullptr) {

    // Initialise the parameter interactions
    for (int bandIndex {0}; bandIndex < WECore::MONSTR::Parameters::NUM_BANDS.maxValue - 1; bandIndex++) {

        _crossoverFrequencies[bandIndex] = {
            [bandIndex, this](const juce::MouseEvent& event) {

                constexpr double MIN_SPACING {
                    2 * UIUtils::Crossover::SLIDER_THUMB_RADIUS + UIUtils::BAND_BUTTON_WIDTH
                };

                const int currentXPos {event.getPosition().getX()};
                const int componentWidth {event.eventComponent->getWidth()};

                _processor.setCrossoverFrequency(bandIndex, UIUtils::Crossover::XPosToSliderValue(currentXPos, componentWidth));

                // Check if this crossover handle is getting too close to another and move it if
                // needed
                const size_t numCrossovers {SplitterInterface::getNumChains(_processor.splitter) - 1};
                for (int crossoverIndex {0}; crossoverIndex < numCrossovers; crossoverIndex++) {
                    const double otherXPos {
                        UIUtils::Crossover::sliderValueToXPos(SplitterInterface::getCrossoverFrequency(_processor.splitter, crossoverIndex), componentWidth)
                    };

                    const double requiredGap {MIN_SPACING * std::abs(bandIndex - crossoverIndex)};
                    const double actualGap {std::abs(currentXPos - otherXPos)};

                    if (crossoverIndex < bandIndex && actualGap < requiredGap) {
                        _processor.setCrossoverFrequency(crossoverIndex, UIUtils::Crossover::XPosToSliderValue(currentXPos - requiredGap, componentWidth));
                    } else if (crossoverIndex > bandIndex && actualGap < requiredGap) {
                        _processor.setCrossoverFrequency(crossoverIndex, UIUtils::Crossover::XPosToSliderValue(currentXPos + requiredGap, componentWidth));
                    }
                }
            }
        };
    }
}

CrossoverMouseListener::~CrossoverMouseListener() {
}

void CrossoverMouseListener::mouseDown(const juce::MouseEvent& event) {
    _dragParameter = _resolveParameterInteraction(event);
}

void CrossoverMouseListener::mouseDrag(const juce::MouseEvent& event) {
    if (_dragParameter != nullptr) {
        _dragParameter->dragCallback(event);
    }
}

void CrossoverMouseListener::mouseUp(const juce::MouseEvent& /*event*/) {
    if (_dragParameter != nullptr) {
        _dragParameter = nullptr;
    }
}

CrossoverMouseListener::FloatParameterInteraction* CrossoverMouseListener::_resolveParameterInteraction(const juce::MouseEvent& event) {

    FloatParameterInteraction* retVal {nullptr};

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
            retVal = &(_crossoverFrequencies[bandIndex]);
            break;
        }
    }

    return retVal;
}

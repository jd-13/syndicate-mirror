#include "CrossoverParameterComponent.h"

#include <algorithm>
#include "WEFilters/StereoWidthProcessorParameters.h"
#include "UIUtilsCrossover.h"
#include "General/CoreMath.h"

namespace {
    constexpr int FREQUENCY_TEXT_HEIGHT {16};
    constexpr int FREQUENCY_TEXT_MARGIN {2};
}

CrossoverParameterComponent::CrossoverParameterComponent(SyndicateAudioProcessor& processor)
        : _processor(processor) {
}

void CrossoverParameterComponent::paint(juce::Graphics &g) {
    _drawSliderThumbs(g);
    _drawFrequencyText(g);
    _drawBandText(g);
}

void CrossoverParameterComponent::_drawSliderThumbs(juce::Graphics& g) {
    ModelInterface::forEachCrossover(_processor.manager, [&](float crossoverFrequency) {
        const double crossoverXPos {
            UIUtils::Crossover::sliderValueToXPos(crossoverFrequency, getWidth())
        };

        const int lineLength {getHeight() / 2 - FREQUENCY_TEXT_HEIGHT / 2 - FREQUENCY_TEXT_MARGIN};

        juce::Path p;
        p.startNewSubPath(crossoverXPos, 0);
        p.lineTo(crossoverXPos, lineLength);

        p.startNewSubPath(crossoverXPos, getHeight() - lineLength);
        p.lineTo(crossoverXPos, getHeight());

        g.setColour(UIUtils::highlightColour);
        g.strokePath(p, juce::PathStrokeType(0.5f));
    });
}

void CrossoverParameterComponent::_drawFrequencyText(juce::Graphics &g) {
    constexpr int WIDTH {100};
    const int yPos {getHeight() / 2 - FREQUENCY_TEXT_HEIGHT / 2};

    g.setColour(UIUtils::highlightColour);

    ModelInterface::forEachCrossover(_processor.manager, [&](float crossoverFrequency) {

        const double crossoverXPos {
            UIUtils::Crossover::sliderValueToXPos(crossoverFrequency, getWidth())
        };

        g.drawText(
            juce::String(static_cast<int>(crossoverFrequency)) + " Hz",
            crossoverXPos - WIDTH / 2,
            yPos,
            WIDTH,
            FREQUENCY_TEXT_HEIGHT,
            juce::Justification::centred,
            false);
    });
}

void CrossoverParameterComponent::_drawBandText(juce::Graphics& g) {
    double xPosLeft {0};

    g.setColour(UIUtils::highlightColour.withBrightness(0.7));

    ModelInterface::forEachChain(_processor.manager, [&](int bandNumber, std::shared_ptr<PluginChain>) {
        const double xPosRight {
            bandNumber < ModelInterface::getNumChains(_processor.manager) - 1 ?
                UIUtils::Crossover::sliderValueToXPos(ModelInterface::getCrossoverFrequency(_processor.manager, bandNumber), getWidth()) :
                getWidth()
        };

        g.drawText(
            juce::String(bandNumber + 1),
            xPosLeft,
            0,
            xPosRight - xPosLeft,
            getHeight(),
            juce::Justification::centred,
            false);

        xPosLeft = xPosRight;
    });
}

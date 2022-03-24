#include "CrossoverParameterComponent.h"

#include <algorithm>
#include "WEFilters/StereoWidthProcessorParameters.h"
#include "UIUtilsCrossover.h"
#include "General/CoreMath.h"

namespace {
    void drawBandButton(juce::String text, const juce::Colour& colour, juce::Graphics& g, double crossoverXPos, double index) {

        constexpr int CORNER_RADIUS {2};
        constexpr int LINE_THICKNESS {1};

        const juce::Rectangle<float> buttonRectange = UIUtils::Crossover::getButtonBounds(crossoverXPos, index);

        const juce::Colour buttonBackground(static_cast<uint8_t>(0), 0, 0, 0.5f);
        g.setColour(buttonBackground);
        g.fillRoundedRectangle(buttonRectange, CORNER_RADIUS);

        g.setColour(colour);
        g.drawRoundedRectangle(buttonRectange, CORNER_RADIUS, LINE_THICKNESS);

        g.drawText(text, buttonRectange, juce::Justification::centred);
    }
}

CrossoverParameterComponent::CrossoverParameterComponent(SyndicateAudioProcessor& processor)
        : _processor(processor) {
}

void CrossoverParameterComponent::paint(juce::Graphics &g) {
    _drawSliderThumbs(g);
    _drawFrequencyText(g);
    _drawBandButtons(g);
}

void CrossoverParameterComponent::_drawSliderThumbs(juce::Graphics& g) {

    const size_t numCrossovers {_processor.pluginSplitter->getNumChains() - 1};
    for (size_t bandIndex {0}; bandIndex < numCrossovers; bandIndex++) {
        const double crossoverXPos {
            UIUtils::Crossover::sliderValueToXPos(_processor.getCrossoverFrequency(bandIndex), getWidth())
        };


        juce::Path p;
        p.startNewSubPath(crossoverXPos, 0);
        p.lineTo(crossoverXPos, getHeight());

        g.setColour(UIUtils::neutralControlColour);
        g.strokePath(p, juce::PathStrokeType(0.5f));
    }
}

void CrossoverParameterComponent::_drawFrequencyText(juce::Graphics &g) {
    constexpr double fractionOfHeight {0.85};
    constexpr int spacing {10};

    const size_t numCrossovers {_processor.pluginSplitter->getNumChains() - 1};
    for (int bandIndex {0}; bandIndex < numCrossovers; bandIndex++) {
        g.setColour(UIUtils::neutralControlColour);

        const float crossoverValue {_processor.getCrossoverFrequency(bandIndex)};

        const double crossoverXPos {
            UIUtils::Crossover::sliderValueToXPos(crossoverValue, getWidth())
        };

        juce::GlyphArrangement ga;
        ga.addLineOfText(juce::Font(16.0f), juce::String(static_cast<int>(crossoverValue)) + " Hz", 0, 0);

        juce::Path p;
        ga.createPath(p);

        juce::Rectangle<float> pathBounds = p.getBounds();

        p.applyTransform(
            juce::AffineTransform().rotated(WECore::CoreMath::DOUBLE_PI / 2,
                                            pathBounds.getCentreX(),
                                            pathBounds.getCentreY()).translated(crossoverXPos - pathBounds.getWidth() / 2.0 + spacing,
                                                                                getHeight() * fractionOfHeight)


        );

        g.fillPath(p);
    }
}

void CrossoverParameterComponent::_drawBandButtons(juce::Graphics &g) {

    // TODO integrate this with ChainButton
    const juce::Colour bypassColour(252, 252, 22);
    const juce::Colour muteColour(252, 0, 0);
    const juce::Colour soloColour(252, 137, 22);

    for (int bandIndex {0}; bandIndex < _processor.pluginSplitter->getNumChains(); bandIndex++) {
        const double crossoverXPos {bandIndex < _processor.pluginSplitter->getNumChains() - 1 ?
            UIUtils::Crossover::sliderValueToXPos(_processor.getCrossoverFrequency(bandIndex), getWidth()) :
            getWidth()
        };

        drawBandButton("B",
                       _processor.chainParameters[bandIndex].getBypass() ? bypassColour : UIUtils::neutralHighlightColour,
                       g,
                       crossoverXPos,
                       0);

        drawBandButton("M",
                       _processor.chainParameters[bandIndex].getMute() ? muteColour : UIUtils::neutralHighlightColour,
                       g,
                       crossoverXPos,
                       1);

        drawBandButton("S",
                       _processor.chainParameters[bandIndex].getSolo() ? soloColour : UIUtils::neutralHighlightColour,
                       g,
                       crossoverXPos,
                       2);
    }
}

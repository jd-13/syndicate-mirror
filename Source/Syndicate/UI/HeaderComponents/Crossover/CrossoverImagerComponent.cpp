#include "CrossoverImagerComponent.h"
#include "UIUtils.h"
#include "SplitterInterface.hpp"

namespace {
    int dBToYPos(float dBValue, int crossoverHeight) {
        constexpr float MIN_DB {-60};
        constexpr float MAX_DB {20};
        constexpr float range = MAX_DB - MIN_DB;

        return crossoverHeight - std::max(((std::min(dBValue, MAX_DB) - MIN_DB) / range) * crossoverHeight, 0.0f);
    }

    int indexToXPos(int pointIndex, int numPoints, int crossoverWidth) {
        const float thisIndexLog {static_cast<float>(std::log10(pointIndex))};
        const float maxLog {static_cast<float>(std::log10(numPoints - 1))};

        return (thisIndexLog / maxLog)  * crossoverWidth;
    }
}

CrossoverImagerComponent::CrossoverImagerComponent(SyndicateAudioProcessor& processor)
        : _processor(processor) {
    start();
}

void CrossoverImagerComponent::paint(juce::Graphics& g) {
    _stopEvent.reset();

    g.fillAll(UIUtils::backgroundColour);

    auto fftBuffer = SplitterInterface::getFFTOutputs(_processor.splitter);

    // Draw a line to each point in the FFT
    juce::Path p;

    for (int index {1}; index < fftBuffer.size(); index++) {
        // TODO scale X and Y positions correctly
        const int XPos {indexToXPos(index, fftBuffer.size(), getWidth())};

        const float leveldB {static_cast<float>(WECore::CoreMath::linearTodB(fftBuffer[index]))};
        const int YPos {dBToYPos(leveldB, getHeight())};

        if (index == 1) {
            p.startNewSubPath(XPos, YPos);
        } else {
            p.lineTo(XPos, YPos);
        }
    }

    g.setColour(UIUtils::neutralControlColour);
    g.strokePath(p, juce::PathStrokeType(0.5f));

    _stopEvent.signal();
}

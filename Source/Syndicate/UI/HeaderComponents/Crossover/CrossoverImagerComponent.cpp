#include "CrossoverImagerComponent.h"
#include "PluginSplitterMultiband.h"
#include "UIUtils.h"

CrossoverImagerComponent::CrossoverImagerComponent(SyndicateAudioProcessor& processor)
        : _processor(processor) {
    start();
}

void CrossoverImagerComponent::paint(juce::Graphics& g) {
    _stopEvent.reset();

    PluginSplitterMultiband* multibandSplitter = dynamic_cast<PluginSplitterMultiband*>(_processor.pluginSplitter.get());

    g.fillAll(UIUtils::backgroundColour);

    if (multibandSplitter != nullptr) {
        const int numPoints {multibandSplitter->getFFTOutputsSize()};
        const float* fftBuffer {multibandSplitter->getFFTOutputs()};

        const float widthIncrement {getWidth() / static_cast<float>(numPoints)};

        // Draw a line to each point in the FFT
        juce::Path p;

        for (int index {0}; index < numPoints; index++) {
            // TODO scale X and Y positions correctly
            const int XPos {static_cast<int>(index * widthIncrement)};
            const int YPos {static_cast<int>(getHeight() - fftBuffer[index] * getHeight())};

            p.lineTo(XPos, YPos);
        }

        g.setColour(UIUtils::neutralControlColour);
        g.strokePath(p, juce::PathStrokeType(0.5f));

        _stopEvent.signal();
    }
}

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class SplitterHeaderComponent : public juce::Component {
public:
    SplitterHeaderComponent() = default;

    virtual void onParameterUpdate() = 0;
};

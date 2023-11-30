#pragma once

#include <JuceHeader.h>
#include "SplitterHeaderComponent.h"
#include "ChainButtonsComponent.h"

class LeftrightSplitterSubComponent : public SplitterHeaderComponent {
public:
    LeftrightSplitterSubComponent(SyndicateAudioProcessor& processor,
                                  ChainParameters& leftChainParams,
                                  ChainParameters& rightChainParams,
                                  UIUtils::LinkedScrollView* graphView);
    ~LeftrightSplitterSubComponent() = default;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LeftrightSplitterSubComponent)
};

#pragma once

#include <JuceHeader.h>
#include "ChainButtonsComponent.h"
#include "SplitterHeaderComponent.h"

class SeriesSplitterSubComponent : public SplitterHeaderComponent {
public:
    SeriesSplitterSubComponent(SyndicateAudioProcessor& processor,
                               ChainParameters& params,
                               UIUtils::LinkedScrollView* graphView);
    ~SeriesSplitterSubComponent() = default;

private:
    ChainParameters& _params;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SeriesSplitterSubComponent)
};

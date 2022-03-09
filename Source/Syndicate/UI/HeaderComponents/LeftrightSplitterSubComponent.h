#pragma once

#include <JuceHeader.h>
#include "SplitterHeaderComponent.h"
#include "ChainButtonsComponent.h"

class LeftrightSplitterSubComponent : public SplitterHeaderComponent {
public:
    LeftrightSplitterSubComponent(ChainParameters& leftChainParams,
                                  ChainParameters& rightChainParams);
    ~LeftrightSplitterSubComponent() override;

    void resized() override;

    void onParameterUpdate() override;

private:
    std::unique_ptr<ChainButtonsComponent> _leftChainbuttons;
    std::unique_ptr<ChainButtonsComponent> _rightChainbuttons;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LeftrightSplitterSubComponent)
};

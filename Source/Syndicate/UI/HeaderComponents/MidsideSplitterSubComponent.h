#pragma once

#include <JuceHeader.h>
#include "SplitterHeaderComponent.h"
#include "ChainButtonsComponent.h"

class MidsideSplitterSubComponent : public SplitterHeaderComponent {
public:
    MidsideSplitterSubComponent(ChainParameters& midChainParams, ChainParameters& sideChainParams);
    ~MidsideSplitterSubComponent() override;

    void onParameterUpdate() override;

    void resized() override;

private:
    std::unique_ptr<ChainButtonsComponent> _midChainbuttons;
    std::unique_ptr<ChainButtonsComponent> _sideChainbuttons;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidsideSplitterSubComponent)
};

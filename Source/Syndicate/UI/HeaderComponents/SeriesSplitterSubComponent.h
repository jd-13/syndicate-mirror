#pragma once

#include <JuceHeader.h>
#include "SplitterHeaderComponent.h"
#include "ChainButtonsComponent.h"

class SeriesSplitterSubComponent : public SplitterHeaderComponent {
public:
    SeriesSplitterSubComponent(ChainParameters& params);
    ~SeriesSplitterSubComponent() override;

    void onParameterUpdate() override;

    void resized() override;

private:
    ChainParameters& _params;
    std::unique_ptr<ChainButtonsComponent> _chainButtons;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SeriesSplitterSubComponent)
};

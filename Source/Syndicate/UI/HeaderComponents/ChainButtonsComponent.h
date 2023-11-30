#pragma once

#include "ChainButton.h"
#include "ChainParameters.h"
#include "UIUtils.h"

class ChainButtonsComponent : public juce::Component, public juce::Button::Listener {
public:
    ChainButtonsComponent(int chainNumber, ChainParameters& headerParams);
    ChainButtonsComponent(int chainNumber, ChainParameters& headerParams, std::function<void()> removeChainCallback);
    virtual ~ChainButtonsComponent();

    void resized() override;
    void buttonClicked(juce::Button* buttonThatWasClicked) override;

    void mouseDrag(const juce::MouseEvent& e) override;

    void onParameterUpdate();

    std::unique_ptr<juce::Label> chainLabel;
    std::unique_ptr<UIUtils::DragHandle> dragHandle;
    std::unique_ptr<ChainButton> bypassBtn;
    std::unique_ptr<ChainButton> muteBtn;
    std::unique_ptr<ChainButton> soloBtn;
    std::unique_ptr<UIUtils::CrossButton> removeButton;

private:
    int _chainNumber;
    ChainParameters& _headerParams;
    std::function<void()> _removeChainCallback;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChainButtonsComponent)
};

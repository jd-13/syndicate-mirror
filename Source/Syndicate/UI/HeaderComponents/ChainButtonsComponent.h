#pragma once

#include "ChainButton.h"
#include "ChainParameters.h"
#include "UIUtils.h"

class ChainButtonsComponent : public juce::Component, public juce::Button::Listener {
public:
    explicit ChainButtonsComponent(ChainParameters& headerParams);
    ChainButtonsComponent(ChainParameters& headerParams, std::function<void()> removeChainCallback);
    virtual ~ChainButtonsComponent();

    void resized() override;
    void buttonClicked(juce::Button* buttonThatWasClicked) override;

    void mouseEnter(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;

    void onParameterUpdate();

    std::unique_ptr<juce::Label> chainLabel;
    std::unique_ptr<UIUtils::CrossButton> removeButton;
    std::unique_ptr<ChainButton> bypassBtn;
    std::unique_ptr<ChainButton> muteBtn;
    std::unique_ptr<ChainButton> soloBtn;

private:
    ChainParameters& _headerParams;
    std::function<void()> _removeChainCallback;
    bool _canRemove;

    int _getButtonXPos(int buttonWidth);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChainButtonsComponent)
};

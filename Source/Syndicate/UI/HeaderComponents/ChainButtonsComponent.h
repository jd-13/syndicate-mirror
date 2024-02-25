#pragma once

#include "ChainButton.h"
#include "UIUtils.h"
#include "PluginProcessor.h"

class ChainButtonsComponent : public juce::Component {
public:
    ChainButtonsComponent(SyndicateAudioProcessor& processor, int chainNumber);
    ChainButtonsComponent(SyndicateAudioProcessor& processor, int chainNumber, std::function<void()> removeChainCallback);
    virtual ~ChainButtonsComponent();

    void resized() override;
    void mouseDrag(const juce::MouseEvent& e) override;

    void refresh();

    std::unique_ptr<juce::Label> chainLabel;
    std::unique_ptr<UIUtils::DragHandle> dragHandle;
    std::unique_ptr<ChainButton> bypassBtn;
    std::unique_ptr<ChainButton> muteBtn;
    std::unique_ptr<ChainButton> soloBtn;
    std::unique_ptr<UIUtils::CrossButton> removeButton;

private:
    SyndicateAudioProcessor& _processor;
    int _chainNumber;
    std::function<void()> _removeChainCallback;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChainButtonsComponent)
};

#pragma once

#include <JuceHeader.h>
#include "PluginScanClient.h"
#include "SelectorComponentStyle.h"
#include "UIUtils.h"

class PluginScanStatusBar : public juce::Component,
                            public juce::MessageListener,
                            public juce::Button::Listener {
public:
    PluginScanStatusBar(PluginScanClient& pluginScanClient,
                        const SelectorComponentStyle& style);
    ~PluginScanStatusBar() override;

    void resized() override;

    void handleMessage(const juce::Message& message) override;

    void buttonClicked(juce::Button* buttonThatWasClicked) override;

private:
    std::unique_ptr<juce::Label> statusLbl;
    std::unique_ptr<juce::TextButton> startScanBtn;
    std::unique_ptr<juce::TextButton> stopScanBtn;
    std::unique_ptr<juce::TextButton> rescanAllBtn;
    std::unique_ptr<juce::TextButton> rescanCrashedBtn;
    std::unique_ptr<juce::TextButton> viewCrashedBtn;
    std::unique_ptr<UIUtils::PopoverComponent> crashedPluginsPopover;
    PluginScanClient& _pluginScanClient;

    void _updateButtonState(bool isScanRunning);

    void _createCrashedPluginsDialogue();
};

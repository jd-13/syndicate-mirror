#pragma once

#include <JuceHeader.h>

#if JUCE_IOS

#include "PluginParameterSelectorListParameters.h"
#include "UIUtils.h"

class PluginParameterSelectorComponent;

class PluginParameterSelectorOverlay : public juce::Component {
public:
    PluginParameterSelectorOverlay(std::function<void()> onCloseCallback,
                                   PluginParameterSelectorListParameters selectorListParameters,
                                   juce::String title);
    ~PluginParameterSelectorOverlay() override;

    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    std::function<void()> _onCloseCallback;
    std::unique_ptr<UIUtils::StaticButtonLookAndFeel> _closeButtonLookAndFeel;
    std::unique_ptr<juce::Label> _titleLabel;
    std::unique_ptr<juce::TextButton> _closeButton;
    std::unique_ptr<PluginParameterSelectorComponent> _selectorComponent;
};

#endif // JUCE_IOS

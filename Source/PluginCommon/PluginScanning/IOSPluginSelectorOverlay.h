#pragma once

#include <JuceHeader.h>

#if JUCE_IOS
#include "PluginSelectorListParameters.h"
#include "SelectorComponentStyle.h"

class PluginSelectorComponent;

class IOSPluginSelectorOverlay : public juce::Component {
public:
    IOSPluginSelectorOverlay(std::function<void()> onCloseCallback,
                              PluginSelectorListParameters selectorListParameters,
                              std::unique_ptr<SelectorComponentStyle> style,
                              juce::String title);
    ~IOSPluginSelectorOverlay() override;

    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    std::function<void()> _onCloseCallback;
    std::unique_ptr<SelectorComponentStyle> _style;
    std::unique_ptr<juce::Label> _titleLabel;
    std::unique_ptr<juce::TextButton> _closeButton;
    std::unique_ptr<PluginSelectorComponent> _selectorComponent;
};

#endif // JUCE_IOS

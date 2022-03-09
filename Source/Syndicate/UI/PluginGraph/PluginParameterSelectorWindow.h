#pragma once

#include <JuceHeader.h>
#include "PluginParameterSelectorComponent.h"
#include "PluginParameterSelectorListParameters.h"

class PluginParameterSelectorWindow : public juce::DocumentWindow {
public:
    PluginParameterSelectorWindow(std::function<void()> onCloseCallback,
                                  PluginParameterSelectorListParameters selectorListParameters,
                                  juce::String pluginName);
    virtual ~PluginParameterSelectorWindow();

    virtual void closeButtonPressed() override;

    void takeFocus();

private:
    std::function<void()> _onCloseCallback;
    PluginParameterSelectorComponent* _content;
};

/*
  ==============================================================================

    PluginSelectorWindow.h
    Created: 17 May 2021 10:23:02pm
    Author:  Jack Devlin

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginSelectorComponent.h"
#include "PluginSelectorListParameters.h"

class PluginSelectorWindow : public juce::DocumentWindow {
public:
    PluginSelectorWindow(std::function<void()> onCloseCallback,
                         PluginSelectorListParameters selectorListParameters,
                         std::unique_ptr<SelectorComponentStyle> style);
    virtual ~PluginSelectorWindow();

    virtual void closeButtonPressed() override;

    void takeFocus();

private:
    std::function<void()> _onCloseCallback;
    PluginSelectorComponent* _content;
    std::unique_ptr<SelectorComponentStyle> _style;
};

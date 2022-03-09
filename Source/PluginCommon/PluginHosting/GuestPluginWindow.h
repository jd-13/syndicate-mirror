/*
  ==============================================================================

    GuestPluginWindow.h
    Created: 31 May 2021 6:04:24pm
    Author:  Jack Devlin

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class GuestPluginWindow  : public juce::DocumentWindow
{
public:
    const std::shared_ptr<juce::AudioPluginInstance> plugin;

    GuestPluginWindow(std::function<void()> onCloseCallback, std::shared_ptr<juce::AudioPluginInstance> newPlugin);
    ~GuestPluginWindow();

    void closeButtonPressed() override;

private:
    std::function<void()> _onCloseCallback;
};

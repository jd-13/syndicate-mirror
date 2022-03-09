/*
  ==============================================================================

    GuestPluginWindow.cpp
    Created: 31 May 2021 6:04:24pm
    Author:  Jack Devlin

  ==============================================================================
*/

#include "GuestPluginWindow.h"

namespace {
    const juce::Colour BACKGROUND_COLOUR(0, 0, 0);
    constexpr int TITLE_BAR_BUTTONS {
        juce::DocumentWindow::TitleBarButtons::minimiseButton |
        juce::DocumentWindow::TitleBarButtons::closeButton
    };
}

GuestPluginWindow::GuestPluginWindow(std::function<void()> onCloseCallback,
                                     std::shared_ptr<juce::AudioPluginInstance> newPlugin)
       : DocumentWindow(newPlugin->getPluginDescription().name, BACKGROUND_COLOUR, TITLE_BAR_BUTTONS),
         plugin(newPlugin),
         _onCloseCallback(onCloseCallback) {

    juce::AudioProcessorEditor* editor = plugin->createEditorIfNeeded();

    if (editor != nullptr) {
        setContentOwned(editor, true);
    }

    setUsingNativeTitleBar(true);
    setVisible(true);
    setResizable(false, false);
    setAlwaysOnTop(true);

    juce::Logger::writeToLog("Created GuestPluginWindow");
}

GuestPluginWindow::~GuestPluginWindow() {
    juce::Logger::writeToLog("Closing GuestPluginWindow");
    clearContentComponent();
}

void GuestPluginWindow::closeButtonPressed() {
    _onCloseCallback();
}

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
                                     std::shared_ptr<juce::AudioPluginInstance> newPlugin,
                                     std::shared_ptr<PluginEditorBounds> editorBounds)
       : DocumentWindow(newPlugin->getPluginDescription().name, BACKGROUND_COLOUR, TITLE_BAR_BUTTONS),
         plugin(newPlugin),
         _onCloseCallback(onCloseCallback),
         _editorBounds(editorBounds) {

    juce::AudioProcessorEditor* editor = plugin->createEditorIfNeeded();

    if (editor != nullptr) {
        setContentOwned(editor, true);
        setResizable(editor->isResizable(), true);
    }

    if (_editorBounds != nullptr && _editorBounds->has_value()) {
        setBounds(_editorBounds->value());
    }

    // Can't use setUsingNativeTitleBar(true) as it prevents some plugin (ie. NI) UIs from loading
    // for some reason

    setVisible(true);
    setAlwaysOnTop(true);

    juce::Logger::writeToLog("Created GuestPluginWindow");
}

GuestPluginWindow::~GuestPluginWindow() {
    juce::Logger::writeToLog("Closing GuestPluginWindow");
    clearContentComponent();
}

void GuestPluginWindow::closeButtonPressed() {
    *_editorBounds.get() = getBounds();
    _onCloseCallback();
}

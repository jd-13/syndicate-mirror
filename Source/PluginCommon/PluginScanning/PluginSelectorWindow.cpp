/*
  ==============================================================================

    PluginSelectorWindow.cpp
    Created: 17 May 2021 10:23:02pm
    Author:  Jack Devlin

  ==============================================================================
*/

#include "PluginSelectorWindow.h"

namespace {
    const juce::Colour BACKGROUND_COLOUR(0, 0, 0);
    constexpr int TITLE_BAR_BUTTONS {
        juce::DocumentWindow::TitleBarButtons::minimiseButton |
        juce::DocumentWindow::TitleBarButtons::closeButton
    };
}

PluginSelectorWindow::PluginSelectorWindow(std::function<void()> onCloseCallback,
                                           PluginSelectorListParameters selectorListParameters,
                                           std::unique_ptr<SelectorComponentStyle> style) :
        juce::DocumentWindow("Plugin Selector",
                             BACKGROUND_COLOUR,
                             TITLE_BAR_BUTTONS,
                             true),
        _onCloseCallback(onCloseCallback),
        _content(nullptr),
        _style(std::move(style)) {
    setUsingNativeTitleBar(true);
    centreWithSize(PLUGIN_SELECTOR_WINDOW_WIDTH, PLUGIN_SELECTOR_WINDOW_HEIGHT);
    setVisible(true);
    setResizable(false, false);
    setAlwaysOnTop(true);
    _content = new PluginSelectorComponent(selectorListParameters, onCloseCallback, *(_style.get()));
    setContentOwned(_content, false);
    _content->setBounds(0, 0, PLUGIN_SELECTOR_WINDOW_WIDTH, PLUGIN_SELECTOR_WINDOW_HEIGHT);

    juce::Logger::writeToLog("Created PluginSelectorWindow");
}

PluginSelectorWindow::~PluginSelectorWindow() {
    juce::Logger::writeToLog("Closing PluginSelectorWindow");
    clearContentComponent();
}


void PluginSelectorWindow::closeButtonPressed() {
    _onCloseCallback();
}

void PluginSelectorWindow::takeFocus() {
    if (_content != nullptr) {
        _content->grabKeyboardFocus();
    }
}

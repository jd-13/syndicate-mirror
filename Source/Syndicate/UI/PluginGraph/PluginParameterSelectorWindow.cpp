#include "PluginParameterSelectorWindow.h"
#include "UIUtils.h"

namespace {
    const juce::Colour BACKGROUND_COLOUR(0, 0, 0);
    constexpr int TITLE_BAR_BUTTONS {
        juce::DocumentWindow::TitleBarButtons::minimiseButton |
        juce::DocumentWindow::TitleBarButtons::closeButton
    };
}

PluginParameterSelectorWindow::PluginParameterSelectorWindow(
        std::function<void()> onCloseCallback,
        PluginParameterSelectorListParameters selectorListParameters,
        juce::String pluginName) :
            juce::DocumentWindow(pluginName + " Parameter Selection",
                             BACKGROUND_COLOUR,
                             TITLE_BAR_BUTTONS,
                             true),
            _onCloseCallback(onCloseCallback),
            _content(nullptr) {
    centreWithSize(UIUtils::PLUGIN_MOD_TARGET_SELECTOR_WIDTH, UIUtils::PLUGIN_MOD_TARGET_SELECTOR_HEIGHT);
    setVisible(true);
    setResizable(false, false);
    setAlwaysOnTop(true);
    _content = new PluginParameterSelectorComponent(selectorListParameters, onCloseCallback);
    setContentOwned(_content, false);

    juce::Logger::writeToLog("Created PluginParameterSelectorWindow");
}

PluginParameterSelectorWindow::~PluginParameterSelectorWindow() {
    juce::Logger::writeToLog("Closing PluginParameterSelectorWindow");
    clearContentComponent();
}


void PluginParameterSelectorWindow::closeButtonPressed() {
    _onCloseCallback();
}

void PluginParameterSelectorWindow::takeFocus() {
    if (_content != nullptr) {
        _content->grabKeyboardFocus();
    }
}

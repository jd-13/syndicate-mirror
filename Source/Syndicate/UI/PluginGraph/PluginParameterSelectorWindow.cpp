#include "PluginParameterSelectorOverlay.h"
#include "PluginParameterSelectorComponent.h"
#include "PluginParameterSelectorState.h"

#if JUCE_IOS

namespace {
    constexpr int TITLE_BAR_HEIGHT {44};
    constexpr int CLOSE_BUTTON_WIDTH {80};
    constexpr int OVERLAY_MARGIN {10};
}

PluginParameterSelectorOverlay::PluginParameterSelectorOverlay(
        std::function<void()> onCloseCallback,
        PluginParameterSelectorListParameters selectorListParameters,
        juce::String title)
        : _onCloseCallback(onCloseCallback) {
    _titleLabel = std::make_unique<juce::Label>("Title", title);
    _titleLabel->setColour(juce::Label::textColourId, UIUtils::highlightColour);
    _titleLabel->setFont(juce::Font(18.0f, juce::Font::bold));
    _titleLabel->setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(_titleLabel.get());

    _closeButtonLookAndFeel = std::make_unique<UIUtils::StaticButtonLookAndFeel>();
    _closeButton = std::make_unique<juce::TextButton>("Close Button");
    _closeButton->setButtonText("Close");
    _closeButton->setLookAndFeel(_closeButtonLookAndFeel.get());
    _closeButton->setColour(UIUtils::StaticButtonLookAndFeel::backgroundColour, UIUtils::slotBackgroundColour);
    _closeButton->setColour(UIUtils::StaticButtonLookAndFeel::highlightColour, UIUtils::highlightColour);
    _closeButton->onClick = _onCloseCallback;
    addAndMakeVisible(_closeButton.get());

    _selectorComponent = std::make_unique<PluginParameterSelectorComponent>(
        selectorListParameters, onCloseCallback);
    addAndMakeVisible(_selectorComponent.get());
    _selectorComponent->restoreScrollPosition();

    juce::Logger::writeToLog("Created PluginParameterSelectorOverlay");
}

PluginParameterSelectorOverlay::~PluginParameterSelectorOverlay() {
    juce::Logger::writeToLog("Closing PluginParameterSelectorOverlay");
    _titleLabel = nullptr;
    _closeButton->setLookAndFeel(nullptr);
    _closeButton = nullptr;
    _selectorComponent = nullptr;
}

void PluginParameterSelectorOverlay::resized() {
    auto area = getLocalBounds().reduced(OVERLAY_MARGIN);
    auto titleBar = area.removeFromTop(TITLE_BAR_HEIGHT);
    _closeButton->setBounds(titleBar.removeFromRight(CLOSE_BUTTON_WIDTH));
    _titleLabel->setBounds(titleBar);
    _selectorComponent->setBounds(area);
    _selectorComponent->restoreScrollPosition();
}

void PluginParameterSelectorOverlay::paint(juce::Graphics& g) {
    g.fillAll(UIUtils::backgroundColour);
}

#else // !JUCE_IOS

#include "PluginParameterSelectorWindow.h"

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
        juce::String title) :
            juce::DocumentWindow(title,
                             BACKGROUND_COLOUR,
                             TITLE_BAR_BUTTONS,
                             true),
            _onCloseCallback(onCloseCallback),
            _content(nullptr),
            _state(selectorListParameters.state) {

    if (_state.bounds.has_value()) {
        // Use the previous bounds if we have them
        setBounds(_state.bounds.value());
    } else {
        // Default to the centre
        constexpr int DEFAULT_WIDTH {300};
        constexpr int DEFAULT_HEIGHT {500};
        centreWithSize(DEFAULT_WIDTH, DEFAULT_HEIGHT);
    }

    setVisible(true);
    setResizable(true, true);
    setAlwaysOnTop(true);
    _content = new PluginParameterSelectorComponent(selectorListParameters, onCloseCallback);
    setContentOwned(_content, false);
    _content->restoreScrollPosition();

    juce::Logger::writeToLog("Created PluginParameterSelectorWindow");
}

PluginParameterSelectorWindow::~PluginParameterSelectorWindow() {
    juce::Logger::writeToLog("Closing PluginParameterSelectorWindow");
    _state.bounds = getBounds();
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

#endif // !JUCE_IOS

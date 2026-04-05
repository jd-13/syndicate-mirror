#include "IOSPluginSelectorOverlay.h"
#include "PluginSelectorComponent.h"
#include "PluginSelectorListParameters.h"

#if JUCE_IOS

#include "UIUtils.h"

namespace {
    constexpr int TITLE_BAR_HEIGHT {44};
    constexpr int CLOSE_BUTTON_WIDTH {80};
    constexpr int OVERLAY_MARGIN {10};
}

IOSPluginSelectorOverlay::IOSPluginSelectorOverlay(
        std::function<void()> onCloseCallback,
        PluginSelectorListParameters selectorListParameters,
        std::unique_ptr<SelectorComponentStyle> style,
        juce::String title)
        : _onCloseCallback(onCloseCallback),
          _style(std::move(style)) {

    _titleLabel = std::make_unique<juce::Label>("Title", title);
    _titleLabel->setColour(juce::Label::textColourId, _style->highlightColour);
    _titleLabel->setFont(juce::Font(18.0f, juce::Font::bold));
    _titleLabel->setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(_titleLabel.get());

    _closeButton = std::make_unique<juce::TextButton>("Close Button");
    _closeButton->setButtonText("Close");
    _closeButton->setLookAndFeel(_style->scanButtonLookAndFeel.get());
    _closeButton->setColour(UIUtils::StaticButtonLookAndFeel::backgroundColour, _style->buttonBackgroundColour);
    _closeButton->setColour(UIUtils::StaticButtonLookAndFeel::highlightColour, _style->highlightColour);
    _closeButton->onClick = _onCloseCallback;
    addAndMakeVisible(_closeButton.get());

    _selectorComponent = std::make_unique<PluginSelectorComponent>(
        selectorListParameters, onCloseCallback, *(_style.get()));
    addAndMakeVisible(_selectorComponent.get());

    selectorListParameters.scanner.clearMissingPlugins();
}

IOSPluginSelectorOverlay::~IOSPluginSelectorOverlay() {
    _closeButton->setLookAndFeel(nullptr);
    _titleLabel = nullptr;
    _closeButton = nullptr;
    _selectorComponent = nullptr;
}

void IOSPluginSelectorOverlay::resized() {
    auto area = getLocalBounds().reduced(OVERLAY_MARGIN);
    auto titleBar = area.removeFromTop(TITLE_BAR_HEIGHT);
    _closeButton->setBounds(titleBar.removeFromRight(CLOSE_BUTTON_WIDTH));
    _titleLabel->setBounds(titleBar);
    _selectorComponent->setBounds(area);
    _selectorComponent->restoreScrollPosition();
}

void IOSPluginSelectorOverlay::paint(juce::Graphics& g) {
    g.fillAll(_style->backgroundColour);
}

#else // !JUCE_IOS

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
                                           std::unique_ptr<SelectorComponentStyle> style,
                                           juce::String title) :
        juce::DocumentWindow(title, BACKGROUND_COLOUR, TITLE_BAR_BUTTONS),
        _onCloseCallback(onCloseCallback),
        _content(nullptr),
        _style(std::move(style)),
        _state(selectorListParameters.state) {

    if (_state.bounds.has_value()) {
        // Use the previous bounds if we have them
        setBounds(_state.bounds.value());
    } else {
        // Default to the centre
        constexpr int DEFAULT_WIDTH {1000};
        constexpr int DEFAULT_HEIGHT {550};
        centreWithSize(DEFAULT_WIDTH, DEFAULT_HEIGHT);
    }

    setVisible(true);
    setResizable(true, true);
    setAlwaysOnTop(true);
    _content = new PluginSelectorComponent(selectorListParameters, onCloseCallback, *(_style.get()));
    setContentOwned(_content, false);
    _content->setBounds(0, getTitleBarHeight(), getWidth(), getHeight() - getTitleBarHeight());

    _content->restoreScrollPosition();

    selectorListParameters.scanner.clearMissingPlugins();

    juce::Logger::writeToLog("Created PluginSelectorWindow");
}

PluginSelectorWindow::~PluginSelectorWindow() {
    juce::Logger::writeToLog("Closing PluginSelectorWindow");
    _state.bounds = getBounds();
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

#endif // !JUCE_IOS

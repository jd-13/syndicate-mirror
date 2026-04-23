#include "GuestPluginOverlay.h"
#include "GuestPluginWindow.h"

#if JUCE_IOS

namespace {
    constexpr int TITLE_BAR_HEIGHT {44};
    constexpr int CLOSE_BUTTON_WIDTH {80};
}

GuestPluginOverlay::GuestPluginOverlay(std::function<void()> onCloseCallback,
                                       std::shared_ptr<juce::AudioPluginInstance> newPlugin)
        : plugin(newPlugin),
          _onCloseCallback(onCloseCallback) {
    _titleLabel = std::make_unique<juce::Label>("Title", plugin->getPluginDescription().name);
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

    _editor.reset(plugin->createEditorIfNeeded());
    if (_editor != nullptr) {
        addAndMakeVisible(_editor.get());
    } else {
        juce::Logger::writeToLog("GuestPluginOverlay: failed to create editor for " + plugin->getPluginDescription().name);
    }

    juce::Logger::writeToLog("Created GuestPluginOverlay for " + plugin->getPluginDescription().name);
}

GuestPluginOverlay::~GuestPluginOverlay() {
    juce::Logger::writeToLog("Closing GuestPluginOverlay for " + plugin->getPluginDescription().name);
    _closeButton->setLookAndFeel(nullptr);
}

void GuestPluginOverlay::resized() {
    auto area = getLocalBounds();
    auto titleBar = area.removeFromTop(TITLE_BAR_HEIGHT);
    _closeButton->setBounds(titleBar.removeFromRight(CLOSE_BUTTON_WIDTH));
    _titleLabel->setBounds(titleBar);

    if (_editor != nullptr) {
        _editor->setBounds(area);
    }
}

void GuestPluginOverlay::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::black);
}

void GuestPluginOverlay::parentSizeChanged() {
    if (auto* parent = getParentComponent()) {
        setBounds(parent->getLocalBounds());
    }
}

#else // !JUCE_IOS

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
    } else {
        juce::Logger::writeToLog("GuestPluginWindow failed to create editor");
    }

    // Attempt to restore the previous editor bounds
    if (_editorBounds != nullptr && _editorBounds->has_value()) {
        // The user may have unplugged the display they were using or made some other change to
        // their displays since last opening the plugin - we need to make sure the editor bounds
        // are still within one of the displays so that the UI doesn't appear off screen
        // (which is really annoying)

        // Find the display the editor should be on
        const juce::Rectangle<int> nearestDisplayArea =
            juce::Desktop::getInstance().getDisplays().getDisplayForRect(_editorBounds->value().editorBounds)->userArea;

        // If this is different to the one used last time just set the top left corner to 0, 0
        if (nearestDisplayArea != _editorBounds->value().displayArea) {
            _editorBounds->value().editorBounds.setPosition(0, 0);
        }

        setBounds(_editorBounds->value().editorBounds);
    }

    // Can't use setUsingNativeTitleBar(true) as it prevents some plugin (ie. NI) UIs from loading
    // for some reason

    setVisible(true);

#if defined(__APPLE__) || defined(_WIN32)
    setAlwaysOnTop(true);
#elif __linux__
    // TODO find a way to make this work on Linux
    setAlwaysOnTop(false);
#else
    #error Unsupported OS
#endif

    juce::Logger::writeToLog("Created GuestPluginWindow");
}

GuestPluginWindow::~GuestPluginWindow() {
    juce::Logger::writeToLog("Closing GuestPluginWindow");
    clearContentComponent();
}

void GuestPluginWindow::closeButtonPressed() {
    PluginEditorBoundsContainer newBounds(
        getBounds(),
        juce::Desktop::getInstance().getDisplays().getDisplayForRect(getBounds())->userArea
    );

    *_editorBounds.get() = newBounds;
    _onCloseCallback();
}

#endif // !JUCE_IOS

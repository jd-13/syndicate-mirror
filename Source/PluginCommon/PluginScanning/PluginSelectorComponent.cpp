#include "PluginSelectorComponent.h"
#include "PluginUtils.h"
namespace {
    constexpr int MARGIN_SIZE {10};
    constexpr int ROW_HEIGHT {24};
}

PluginSelectorComponent::PluginSelectorComponent(PluginSelectorListParameters selectorListParameters,
                                                 std::function<void()> onCloseCallback,
                                                 const SelectorComponentStyle& style)
        : _state(selectorListParameters.state),
          _onCloseCallback(onCloseCallback),
          _backgroundColour(style.backgroundColour) {

    setComponentID(Utils::pluginSelectorComponentID);

    // Position the header row
    _setupHeaderRow(style);

    // Position the status bar
    statusBar.reset(new PluginScanStatusBar(selectorListParameters.scanner, style));
    addAndMakeVisible(statusBar.get());
    statusBar->setName("Plugin Scan Status Bar");

    // Position the table to use the remaining space
    pluginTableListBox.reset(new PluginSelectorTableListBox(selectorListParameters, style));
    addAndMakeVisible(pluginTableListBox.get());
    pluginTableListBox->setName("Plugin Table List Box");
    pluginTableListBox->getHeader().setLookAndFeel(style.tableHeaderLookAndFeel.get());
    pluginTableListBox->getHeader().setColour(juce::TableHeaderComponent::textColourId, style.controlColour);
    pluginTableListBox->getHeader().setColour(juce::TableHeaderComponent::outlineColourId, style.controlColour);
    pluginTableListBox->getHeader().setColour(juce::TableHeaderComponent::backgroundColourId, style.backgroundColour);
    pluginTableListBox->getHeader().setColour(juce::TableHeaderComponent::highlightColourId, style.neutralColour);
    pluginTableListBox->getVerticalScrollBar().setColour(juce::ScrollBar::ColourIds::backgroundColourId, juce::Colour(0x00000000));
    pluginTableListBox->getVerticalScrollBar().setColour(juce::ScrollBar::ColourIds::thumbColourId, style.neutralColour.withAlpha(0.5f));
    pluginTableListBox->getVerticalScrollBar().setColour(juce::ScrollBar::ColourIds::trackColourId, juce::Colour(0x00000000));

    // Recall UI from state
    searchEdt->setText(_state.filterString, false);
    vstBtn->setToggleState(_state.includeVST, juce::dontSendNotification);
    vst3Btn->setToggleState(_state.includeVST3, juce::dontSendNotification);
    auBtn->setToggleState(_state.includeAU, juce::dontSendNotification);
}

PluginSelectorComponent::~PluginSelectorComponent() {
    searchEdt->setLookAndFeel(nullptr);
    vstBtn->setLookAndFeel(nullptr);
    vst3Btn->setLookAndFeel(nullptr);
    auBtn->setLookAndFeel(nullptr);
    pluginTableListBox->getHeader().setLookAndFeel(nullptr);

    searchEdt = nullptr;
    vstBtn = nullptr;
    vst3Btn = nullptr;
    auBtn = nullptr;
    pluginTableListBox = nullptr;
    statusBar = nullptr;
}

void PluginSelectorComponent::resized() {
    juce::Rectangle<int> availableArea = getLocalBounds().reduced(MARGIN_SIZE);

    // Header
    juce::Rectangle<int> headerArea = availableArea.removeFromTop(ROW_HEIGHT);
    constexpr int buttonWidth {56};
    searchEdt->setBounds(headerArea.removeFromLeft(560));
    headerArea.removeFromLeft(MARGIN_SIZE);
    vstBtn->setBounds(headerArea.removeFromLeft(buttonWidth));
    headerArea.removeFromLeft(MARGIN_SIZE);
    vst3Btn->setBounds(headerArea.removeFromLeft(buttonWidth));
    headerArea.removeFromLeft(MARGIN_SIZE);
    auBtn->setBounds(headerArea.removeFromLeft(buttonWidth));
    availableArea.removeFromTop(MARGIN_SIZE);

    // Status bar
    statusBar->setBounds(availableArea.removeFromBottom(ROW_HEIGHT));
    availableArea.removeFromBottom(MARGIN_SIZE);

    // Table
    pluginTableListBox->setBounds(availableArea);
}

void PluginSelectorComponent::paint(juce::Graphics& g) {
    g.fillAll(_backgroundColour);
}

void PluginSelectorComponent::buttonClicked(juce::Button* buttonThatWasClicked) {
    if (buttonThatWasClicked == vstBtn.get()) {
        vstBtn->setToggleState(!vstBtn->getToggleState(), juce::dontSendNotification);
        _state.includeVST = vstBtn->getToggleState();
        pluginTableListBox->onFiltersOrSortUpdate();
    } else if (buttonThatWasClicked == vst3Btn.get()) {
        vst3Btn->setToggleState(!vst3Btn->getToggleState(), juce::dontSendNotification);
        _state.includeVST3 = vst3Btn->getToggleState();
        pluginTableListBox->onFiltersOrSortUpdate();
    } else if (buttonThatWasClicked == auBtn.get()) {
        auBtn->setToggleState(!auBtn->getToggleState(), juce::dontSendNotification);
        _state.includeAU = auBtn->getToggleState();
        pluginTableListBox->onFiltersOrSortUpdate();
    }
}

bool PluginSelectorComponent::keyPressed(const juce::KeyPress& key) {
    if (key.isKeyCode(juce::KeyPress::escapeKey)) {
        // Close the window
        _onCloseCallback();
        return true;
    }

    return false;  // Return true if your handler uses this key event, or false to allow it to be passed-on.
}

void PluginSelectorComponent::textEditorTextChanged(juce::TextEditor& textEditor) {
    _state.filterString = searchEdt->getText();
    pluginTableListBox->onFiltersOrSortUpdate();
}

void PluginSelectorComponent::_setupHeaderRow(const SelectorComponentStyle& style) {
    searchEdt.reset(new juce::TextEditor("Search Text Editor"));
    addAndMakeVisible(searchEdt.get());
    searchEdt->setMultiLine(false);
    searchEdt->setReturnKeyStartsNewLine(false);
    searchEdt->setReadOnly(false);
    searchEdt->setScrollbarsShown(true);
    searchEdt->setCaretVisible(true);
    searchEdt->setPopupMenuEnabled(true);
    searchEdt->setText(juce::String());
    searchEdt->addListener(this);
    searchEdt->setEscapeAndReturnKeysConsumed(false);
    searchEdt->setSelectAllWhenFocused(true);
    searchEdt->setWantsKeyboardFocus(true);
    searchEdt->setLookAndFeel(style.searchBarLookAndFeel.get());
    searchEdt->setColour(juce::TextEditor::outlineColourId, style.controlColour);
    searchEdt->setColour(juce::TextEditor::backgroundColourId, style.backgroundColour);
    searchEdt->setColour(juce::TextEditor::textColourId, style.controlColour);
    searchEdt->setColour(juce::TextEditor::highlightColourId, style.controlColour);
    searchEdt->setColour(juce::TextEditor::highlightedTextColourId, style.neutralColour);
    searchEdt->setColour(juce::CaretComponent::caretColourId, style.controlColour);

    vstBtn.reset(new juce::TextButton("VST Button"));
    addAndMakeVisible(vstBtn.get());
    vstBtn->setButtonText(TRANS("VST"));
    vstBtn->addListener(this);
    vstBtn->setLookAndFeel(style.headerButtonLookAndFeel.get());
    vstBtn->setColour(juce::TextButton::buttonOnColourId, style.controlColour);
    vstBtn->setColour(juce::TextButton::textColourOnId, style.neutralColour);
    vstBtn->setColour(juce::TextButton::textColourOffId, style.controlColour);

    vst3Btn.reset(new juce::TextButton ("VST3 Button"));
    addAndMakeVisible(vst3Btn.get());
    vst3Btn->setButtonText(TRANS("VST3"));
    vst3Btn->addListener(this);
    vst3Btn->setLookAndFeel(style.headerButtonLookAndFeel.get());
    vst3Btn->setColour(juce::TextButton::buttonOnColourId, style.controlColour);
    vst3Btn->setColour(juce::TextButton::textColourOnId, style.neutralColour);
    vst3Btn->setColour(juce::TextButton::textColourOffId, style.controlColour);

    auBtn.reset(new juce::TextButton("AU Button"));
    addAndMakeVisible(auBtn.get());
    auBtn->setButtonText(TRANS("AU"));
    auBtn->addListener(this);
    auBtn->setLookAndFeel(style.headerButtonLookAndFeel.get());
    auBtn->setColour(juce::TextButton::buttonOnColourId, style.controlColour);
    auBtn->setColour(juce::TextButton::textColourOnId, style.neutralColour);
    auBtn->setColour(juce::TextButton::textColourOffId, style.controlColour);
}

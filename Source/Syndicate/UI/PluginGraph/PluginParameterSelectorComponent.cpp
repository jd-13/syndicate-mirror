#include "PluginParameterSelectorComponent.h"

PluginParameterSelectorComponent::PluginParameterSelectorComponent(
        PluginParameterSelectorListParameters selectorListParameters,
        std::function<void()> onCloseCallback)
            : _state(selectorListParameters.state),
              _onCloseCallback(onCloseCallback) {
    _searchTextEditor.reset(new juce::TextEditor("Search Text Editor"));
    addAndMakeVisible(_searchTextEditor.get());
    _searchTextEditor->setMultiLine(false);
    _searchTextEditor->setReturnKeyStartsNewLine(false);
    _searchTextEditor->setReadOnly(false);
    _searchTextEditor->setScrollbarsShown(true);
    _searchTextEditor->setCaretVisible(true);
    _searchTextEditor->setPopupMenuEnabled(true);
    _searchTextEditor->setText(juce::String());
    _searchTextEditor->setEscapeAndReturnKeysConsumed(false);
    _searchTextEditor->setSelectAllWhenFocused(true);
    _searchTextEditor->setWantsKeyboardFocus(true);
    _searchTextEditor->addListener(this);
    _searchTextEditor->setLookAndFeel(&_searchBarLookAndFeel);
    _searchTextEditor->setColour(juce::TextEditor::outlineColourId, UIUtils::neutralControlColour);
    _searchTextEditor->setColour(juce::TextEditor::backgroundColourId, UIUtils::backgroundColour);
    _searchTextEditor->setColour(juce::TextEditor::textColourId, UIUtils::neutralControlColour);
    _searchTextEditor->setColour(juce::TextEditor::highlightColourId, UIUtils::neutralControlColour);
    _searchTextEditor->setColour(juce::TextEditor::highlightedTextColourId, UIUtils::neutralHighlightColour);
    _searchTextEditor->setColour(juce::CaretComponent::caretColourId, UIUtils::neutralControlColour);

    _parameterTableListBox.reset(new PluginParameterSelectorTableListBox(selectorListParameters));
    addAndMakeVisible(_parameterTableListBox.get());
    _parameterTableListBox->setName("Plugin Parameter Table List Box");
}

void PluginParameterSelectorComponent::resized() {
    constexpr int MARGIN_SIZE {10};
    constexpr int ROW_HEIGHT {24};

    juce::Rectangle<int> area = getLocalBounds().reduced(MARGIN_SIZE);
    _searchTextEditor->setBounds(area.removeFromTop(ROW_HEIGHT));
    area.removeFromTop(MARGIN_SIZE);
    _parameterTableListBox->setBounds(area);
}

void PluginParameterSelectorComponent::textEditorTextChanged(juce::TextEditor& textEditor) {
    _state.filterString = _searchTextEditor->getText();
    _parameterTableListBox->onFilterUpdate();
}

void PluginParameterSelectorComponent::paint(juce::Graphics& g) {
    g.fillAll(UIUtils::backgroundColour);
}

bool PluginParameterSelectorComponent::keyPressed(const juce::KeyPress& key)
{
    if (key.isKeyCode(juce::KeyPress::escapeKey)) {
        // Close the window
        _onCloseCallback();
        return true;
    }

    return false;
}

#include "ImportExportComponent.h"

#include "PluginEditor.h"

namespace {
    const juce::String undoTooltipPrefix = "Undo the previous change - ";
    const juce::String redoTooltipPrefix = "Redo the last change - ";
    const juce::String defaultUndoTooltip = undoTooltipPrefix + "no changes to undo";
    const juce::String defaultRedoTooltip = redoTooltipPrefix + "no changes to redo";
}

ImportExportComponent::ImportExportComponent(SyndicateAudioProcessor& processor, SyndicateAudioProcessorEditor& editor)
        : _processor(processor), _editor(editor) {
    _exportButton.reset(new juce::TextButton("Save Button"));
    addAndMakeVisible(_exportButton.get());
    _exportButton->setButtonText(TRANS("Save"));
    _exportButton->setTooltip("Save the current settings to a file");
    _exportButton->setLookAndFeel(&_buttonLookAndFeel);
    _exportButton->setColour(UIUtils::StaticButtonLookAndFeel::backgroundColour, UIUtils::slotBackgroundColour);
    _exportButton->setColour(UIUtils::StaticButtonLookAndFeel::highlightColour, UIUtils::highlightColour);
    _exportButton->setColour(UIUtils::StaticButtonLookAndFeel::disabledColour, UIUtils::deactivatedColour);
    _exportButton->onClick = [&]() {
        const int flags {juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting};
        _fileChooser.reset(new juce::FileChooser("Export Syndicate Preset", juce::File(), "*.syn"));
        _fileChooser->launchAsync(flags, [&](const juce::FileChooser& chooser) {
            _onExportToFile(chooser.getResult());
        });
    };

    _importButton.reset(new juce::TextButton("Load Button"));
    addAndMakeVisible(_importButton.get());
    _importButton->setButtonText(TRANS("Load"));
    _importButton->setTooltip("Load settings from a previously saved file");
    _importButton->setLookAndFeel(&_buttonLookAndFeel);
    _importButton->setColour(UIUtils::StaticButtonLookAndFeel::backgroundColour, UIUtils::slotBackgroundColour);
    _importButton->setColour(UIUtils::StaticButtonLookAndFeel::highlightColour, UIUtils::highlightColour);
    _importButton->setColour(UIUtils::StaticButtonLookAndFeel::disabledColour, UIUtils::deactivatedColour);
    _importButton->onClick = [&]() {
        const int flags {juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::openMode};
        _fileChooser.reset(new juce::FileChooser("Import Syndicate Preset", juce::File(), "*.syn"));
        _fileChooser->launchAsync(flags, [&](const juce::FileChooser& chooser) {
            _onImportFromFile(chooser.getResult());
        });
    };

    _undoButton.reset(new juce::TextButton("Undo Button"));
    addAndMakeVisible(_undoButton.get());
    _undoButton->setButtonText(TRANS("Undo"));
    _undoButton->setTooltip(defaultUndoTooltip);
    _undoButton->setLookAndFeel(&_buttonLookAndFeel);
    _undoButton->setColour(UIUtils::StaticButtonLookAndFeel::backgroundColour, UIUtils::slotBackgroundColour);
    _undoButton->setColour(UIUtils::StaticButtonLookAndFeel::highlightColour, UIUtils::highlightColour);
    _undoButton->setColour(UIUtils::StaticButtonLookAndFeel::disabledColour, UIUtils::deactivatedColour);
    _undoButton->onClick = [&]() {
        _processor.undo();
    };

    _redoButton.reset(new juce::TextButton("Redo Button"));
    addAndMakeVisible(_redoButton.get());
    _redoButton->setButtonText(TRANS("Redo"));
    _redoButton->setTooltip(defaultRedoTooltip);
    _redoButton->setLookAndFeel(&_buttonLookAndFeel);
    _redoButton->setColour(UIUtils::StaticButtonLookAndFeel::backgroundColour, UIUtils::slotBackgroundColour);
    _redoButton->setColour(UIUtils::StaticButtonLookAndFeel::highlightColour, UIUtils::highlightColour);
    _redoButton->setColour(UIUtils::StaticButtonLookAndFeel::disabledColour, UIUtils::deactivatedColour);
    _redoButton->onClick = [&]() {
        processor.redo();
    };

    refresh();
}

ImportExportComponent::~ImportExportComponent() {
}

void ImportExportComponent::resized() {
    constexpr int BUTTON_HEIGHT {24};
    juce::Rectangle<int> availableArea = getLocalBounds();
    availableArea.reduce(4, 0);
    availableArea.removeFromTop(8);

    // Save/Load
    _exportButton->setBounds(availableArea.removeFromTop(BUTTON_HEIGHT));
    availableArea.removeFromTop(4);
    _importButton->setBounds(availableArea.removeFromTop(BUTTON_HEIGHT));

    availableArea.removeFromTop(16);

    // Undo/Redo
    _undoButton->setBounds(availableArea.removeFromTop(BUTTON_HEIGHT));
    availableArea.removeFromTop(4);
    _redoButton->setBounds(availableArea.removeFromTop(BUTTON_HEIGHT));
}

void ImportExportComponent::refresh() {
    const std::optional<juce::String> undoOperation = ModelInterface::getUndoOperation(_processor.manager);
    if (undoOperation.has_value()) {
        _undoButton->setTooltip(undoTooltipPrefix + undoOperation.value());
        _undoButton->setEnabled(true);
    } else {
        _undoButton->setTooltip(defaultUndoTooltip);
        _undoButton->setEnabled(false);
    }

    const std::optional<juce::String> redoOperation = ModelInterface::getRedoOperation(_processor.manager);
    if (redoOperation.has_value()) {
        _redoButton->setTooltip(redoTooltipPrefix + redoOperation.value());
        _redoButton->setEnabled(true);
    } else {
        _redoButton->setTooltip(defaultRedoTooltip);
        _redoButton->setEnabled(false);
    }
}

void ImportExportComponent::_onExportToFile(juce::File file) {
    std::unique_ptr<juce::XmlElement> element = _processor.writeToXml();
    element->writeTo(file);
}

void ImportExportComponent::_onImportFromFile(juce::File file) {
    if (file.existsAsFile()) {
        std::unique_ptr<juce::XmlElement> element = juce::XmlDocument::parse(file);
        if (element != nullptr) {
            _processor.restoreFromXml(std::move(element));
            _editor.needsToRefreshAll();
        }
    }
}

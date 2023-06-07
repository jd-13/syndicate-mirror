#include "ImportExportComponent.h"

#include "PluginEditor.h"

ImportExportComponent::ImportExportComponent(SyndicateAudioProcessor& processor, SyndicateAudioProcessorEditor& editor)
        : _processor(processor), _editor(editor) {
    _exportButton.reset(new juce::TextButton("Save Button"));
    addAndMakeVisible(_exportButton.get());
    _exportButton->setButtonText(TRANS("Save"));
    _exportButton->setTooltip("Saves the current settings to a file");
    _exportButton->setLookAndFeel(&_buttonLookAndFeel);
    _exportButton->setColour(juce::TextButton::buttonOnColourId, UIUtils::neutralControlColour);
    _exportButton->setColour(juce::TextButton::textColourOnId, UIUtils::neutralControlColour);
    _exportButton->setColour(juce::TextButton::textColourOffId, UIUtils::neutralControlColour);
    _exportButton->onClick = [&]() {
        const int flags {juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::saveMode};
        _fileChooser.reset(new juce::FileChooser("Export Syndicate Preset", juce::File(), "*.syn"));
        _fileChooser->launchAsync(flags, [&](const juce::FileChooser& chooser) {
            _onExportToFile(chooser.getResult());
        });
    };

    _importButton.reset(new juce::TextButton("Load Button"));
    addAndMakeVisible(_importButton.get());
    _importButton->setButtonText(TRANS("Load"));
    _importButton->setTooltip("Loads settings from a previously saved file");
    _importButton->setLookAndFeel(&_buttonLookAndFeel);
    _importButton->setColour(juce::TextButton::buttonOnColourId, UIUtils::neutralControlColour);
    _importButton->setColour(juce::TextButton::textColourOnId, UIUtils::neutralControlColour);
    _importButton->setColour(juce::TextButton::textColourOffId, UIUtils::neutralControlColour);
    _importButton->onClick = [&]() {
        const int flags {juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::openMode};
        _fileChooser.reset(new juce::FileChooser("Import Syndicate Preset", juce::File(), "*.syn"));
        _fileChooser->launchAsync(flags, [&](const juce::FileChooser& chooser) {
            _onImportFromFile(chooser.getResult());
        });
    };
}

ImportExportComponent::~ImportExportComponent() {
}

void ImportExportComponent::resized() {
    constexpr int BUTTON_HEIGHT {24};
    juce::Rectangle<int> availableArea = getLocalBounds();
    availableArea.reduce(4, 0);
    availableArea.removeFromTop(8);

    _exportButton->setBounds(availableArea.removeFromTop(BUTTON_HEIGHT));

    availableArea.removeFromTop(4);

    _importButton->setBounds(availableArea.removeFromTop(BUTTON_HEIGHT));
}

void ImportExportComponent::_onExportToFile(juce::File file) {
    if (!file.exists()) {
        std::unique_ptr<juce::XmlElement> element = _processor.writeToXml();
        element->writeTo(file);
    }
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

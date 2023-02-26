#pragma once

#include <JuceHeader.h>
#include "UIUtils.h"
#include "PluginProcessor.h"

class SyndicateAudioProcessorEditor;

class ImportExportComponent : public juce::Component {
public:
    ImportExportComponent(SyndicateAudioProcessor& processor, SyndicateAudioProcessorEditor& editor);
    ~ImportExportComponent() override;

    void resized() override;

private:
    UIUtils::StaticButtonLookAndFeel _buttonLookAndFeel;

    std::unique_ptr<juce::TextButton> _exportButton;
    std::unique_ptr<juce::TextButton> _importButton;
    std::unique_ptr<juce::FileChooser> _fileChooser;

    SyndicateAudioProcessor& _processor;
    SyndicateAudioProcessorEditor& _editor;

    void _onExportToFile(juce::File file);
    void _onImportFromFile(juce::File file);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ImportExportComponent)
};

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ModulationBarEnvelope.h"
#include "ModulationBarLfo.h"
#include "ModulationBarRandom.hpp"
#include "ModulationBarStepSequencer.h"
#include "CoreJUCEPlugin/CoreLookAndFeel.h"
#include "ModulationButton.h"
#include "UIUtils.h"

class ModulationBar : public juce::Component,
                             juce::Button::Listener,
                             juce::ComboBox::Listener {
public:
    ModulationBar(SyndicateAudioProcessor& processor, juce::DragAndDropContainer* dragContainer);
    ~ModulationBar() override;

    void resized() override;
    void buttonClicked(juce::Button* buttonThatWasClicked) override;
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

    void needsRebuild();
    void needsSelectedSourceRebuild();

private:
    class AddButtonLookAndFeel : public juce::LookAndFeel_V2 {
    public:
        AddButtonLookAndFeel() = default;
        ~AddButtonLookAndFeel() = default;

        void drawButtonBackground(juce::Graphics& g,
                                  juce::Button& button,
                                  const juce::Colour& backgroundColour,
                                  bool shouldDrawButtonAsHighlighted,
                                  bool shouldDrawButtonAsDown) override;
    };

    SyndicateAudioProcessor& _processor;

    std::vector<std::unique_ptr<ModulationButton>> _lfoButtons;
    std::unique_ptr<juce::TextButton> _addLfoButton;
    std::unique_ptr<juce::Viewport> _lfoButtonsView;

    std::vector<std::unique_ptr<ModulationButton>> _envelopeButtons;
    std::unique_ptr<juce::TextButton> _addEnvelopeButton;
    std::unique_ptr<juce::Viewport> _envelopeButtonsView;

    std::vector<std::unique_ptr<ModulationButton>> _rndButtons;
    std::unique_ptr<juce::TextButton> _addRndButton;
    std::unique_ptr<juce::Viewport> _rndButtonsView;

    std::vector<std::unique_ptr<ModulationButton>> _seqButtons;
    std::unique_ptr<juce::TextButton> _addSeqButton;
    std::unique_ptr<juce::Viewport> _seqButtonsView;

    // Compact layout: single column with dropdown add button + combined source list
    std::unique_ptr<juce::ComboBox> _addSourceDropdownButton;
    std::unique_ptr<juce::Viewport> _sourcesView;
    bool _useCompactLayout {false};
    bool _suppressReselection {false};

    juce::Rectangle<int> _selectedSourceComponentArea;
    std::unique_ptr<juce::Component> _selectedSourceComponent;
    juce::DragAndDropContainer* _dragContainer;
    std::unique_ptr<AddButtonLookAndFeel> _addButtonLookAndFeel;
    UIUtils::StandardComboBoxLookAndFeel _addSourceDropdownLookAndFeel;

    bool _hasRestoredScroll;

    void _resetButtons();
    void _createModulationSourceButton(ModulationSourceDefinition definition);
    void _selectModulationSource(ModulationButton* selectedButton);
    void _removeModulationSource(ModulationSourceDefinition definition);
    std::optional<ModulationSourceDefinition> _getSelectedDefinition();
    void _attemptToSelectByDefinition(ModulationSourceDefinition definition);
};

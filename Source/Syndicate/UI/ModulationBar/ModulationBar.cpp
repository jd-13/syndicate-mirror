#include "ModulationBar.h"
#include "EnvelopeFollowerWrapper.h"
#include "UIUtils.h"

ModulationBar::ModulationBar(SyndicateAudioProcessor& processor,
                             juce::DragAndDropContainer* dragContainer) :
        _processor(processor),
        _dragContainer(dragContainer),
        _hasRestoredScroll(false) {
    _addButtonLookAndFeel.reset(new AddButtonLookAndFeel());

    auto setUpButtonsView = [&] (std::unique_ptr<juce::Viewport>& view) {
        view.reset(new juce::Viewport());
        view->setViewedComponent(new juce::Component());
        view->setScrollBarsShown(true, false);
        view->getVerticalScrollBar().setColour(juce::ScrollBar::ColourIds::backgroundColourId, juce::Colour(0x00000000));
        view->getVerticalScrollBar().setColour(juce::ScrollBar::ColourIds::thumbColourId, UIUtils::neutralHighlightColour.withAlpha(0.5f));
        view->getVerticalScrollBar().setColour(juce::ScrollBar::ColourIds::trackColourId, juce::Colour(0x00000000));
        addAndMakeVisible(view.get());
    };

    setUpButtonsView(_lfoButtonsView);
    setUpButtonsView(_envelopeButtonsView);

    needsRebuild();

    // Restore the selection
    if (_processor.mainWindowState.selectedModulationSource.has_value()) {
        const ModulationSourceDefinition definition = _processor.mainWindowState.selectedModulationSource.value();

        if (definition.type == MODULATION_TYPE::LFO) {
            if (definition.id <= _lfoButtons.size()) {
                _selectModulationSource(_lfoButtons[definition.id - 1].get());
            }
        } else if (definition.type == MODULATION_TYPE::ENVELOPE) {
            if (definition.id <= _envelopeButtons.size()) {
                _selectModulationSource(_envelopeButtons[definition.id - 1].get());
            }
        }
    }
}

ModulationBar::~ModulationBar() {
    _processor.mainWindowState.lfoButtonsScrollPosition = _lfoButtonsView->getViewPositionY();
    _processor.mainWindowState.envButtonsScrollPosition = _envelopeButtonsView->getViewPositionY();

    // Reset stored state since it might now be invalid
    _processor.mainWindowState.selectedModulationSource.reset();

    // Store the selection
    for (int buttonIndex {0}; buttonIndex < _lfoButtons.size() && !_processor.mainWindowState.selectedModulationSource.has_value(); buttonIndex++) {
        if (_lfoButtons[buttonIndex]->getIsSelected()) {
            _processor.mainWindowState.selectedModulationSource = _lfoButtons[buttonIndex]->definition;
        }
    }

    for (int buttonIndex {0}; buttonIndex < _envelopeButtons.size() && !_processor.mainWindowState.selectedModulationSource.has_value(); buttonIndex++) {
        if (_envelopeButtons[buttonIndex]->getIsSelected()) {
            _processor.mainWindowState.selectedModulationSource = _envelopeButtons[buttonIndex]->definition;
        }
    }

    // Usual clean up
    _addLfoButton->setLookAndFeel(nullptr);
    _addEnvelopeButton->setLookAndFeel(nullptr);

    _addButtonLookAndFeel = nullptr;

    _lfoButtonsView = nullptr;
    _envelopeButtonsView = nullptr;
}

void ModulationBar::resized() {
    juce::Rectangle<int> availableArea = getLocalBounds();

    auto arrangeColumn = [&availableArea](std::vector<std::unique_ptr<ModulationButton>>& buttons,
                                          std::unique_ptr<juce::TextButton>& addButton,
                                          std::unique_ptr<juce::Viewport>& buttonsView) {

        const int columnHeight {static_cast<int>(UIUtils::MODULATION_LIST_BUTTON_HEIGHT * (buttons.size() + 1))};

        const juce::Rectangle<int> visibleButtonsArea =
            availableArea.removeFromLeft(UIUtils::MODULATION_LIST_COLUMN_WIDTH);
        buttonsView->setBounds(visibleButtonsArea);

        const int scrollPosition {buttonsView->getViewPositionY()};
        juce::Rectangle<int> scrollablebuttonsArea = visibleButtonsArea.withHeight(columnHeight);
        buttonsView->getViewedComponent()->setBounds(scrollablebuttonsArea);
        buttonsView->setViewPosition(0, scrollPosition);

        // Set the origin to 0 since we're now using it to position buttons relative to the inner
        // component
        scrollablebuttonsArea.setPosition(0, 0);

        const int scrollbarWidth {buttonsView->getScrollBarThickness()};

        for (std::unique_ptr<ModulationButton>& button : buttons) {
            button->setBounds(scrollablebuttonsArea.removeFromTop(UIUtils::MODULATION_LIST_BUTTON_HEIGHT).withTrimmedRight(scrollbarWidth));
        }

        if (addButton != nullptr) {
            addButton->setBounds(scrollablebuttonsArea.removeFromTop(UIUtils::MODULATION_LIST_BUTTON_HEIGHT).withTrimmedRight(scrollbarWidth));
        }
    };

    // LFO buttons
    arrangeColumn(_lfoButtons, _addLfoButton, _lfoButtonsView);

    // Envelope buttons
    arrangeColumn(_envelopeButtons, _addEnvelopeButton, _envelopeButtonsView);

    _selectedSourceComponentArea = availableArea;

    if (_selectedSourceComponent != nullptr) {
        _selectedSourceComponent->setBounds(_selectedSourceComponentArea);
    }
}

void ModulationBar::buttonClicked(juce::Button* buttonThatWasClicked) {
    if (buttonThatWasClicked == _addLfoButton.get()) {
        _processor.addLfo();
        _resetButtons();
        _selectModulationSource(_lfoButtons[_lfoButtons.size() - 1].get());
    } else if (buttonThatWasClicked == _addEnvelopeButton.get()) {
        _processor.addEnvelope();
        _resetButtons();
        _selectModulationSource(_envelopeButtons[_envelopeButtons.size() - 1].get());
    }
}

void ModulationBar::needsRebuild() {
    _resetButtons();

    if (_lfoButtons.size() > 0) {
        _selectModulationSource(_lfoButtons[0].get());
    } else if (_envelopeButtons.size() > 0) {
        _selectModulationSource(_envelopeButtons[0].get());
    }
}

void ModulationBar::AddButtonLookAndFeel::drawButtonBackground(juce::Graphics& g,
                                                               juce::Button& button,
                                                               const juce::Colour& backgroundColour,
                                                               bool shouldDrawButtonAsHighlighted,
                                                               bool shouldDrawButtonAsDown) {

    constexpr int MARGIN {1};
    juce::Rectangle<int> area = juce::Rectangle<int>(button.getWidth(), button.getHeight()).reduced(MARGIN, MARGIN);

    juce::Path p;
    const int centreY {area.getHeight() / 2 + area.getY()};
    const int centreX {area.getHeight() / 2 + area.getX()};
    const int radius {area.getHeight() / 2};

    p.startNewSubPath(centreX, area.getY());
    p.addCentredArc(centreX,
                    centreY,
                    radius,
                    radius,
                    0,
                    0,
                    -WECore::CoreMath::DOUBLE_PI);
    p.lineTo(area.getWidth() - centreX, area.getHeight() + area.getY());
    p.addCentredArc(area.getWidth() - centreX,
                    centreY,
                    radius,
                    radius,
                    0,
                    WECore::CoreMath::DOUBLE_PI,
                    0);
    p.closeSubPath();

    g.setColour(button.findColour(juce::TextButton::buttonColourId));
    g.strokePath(p, juce::PathStrokeType(1.0f));
}

void ModulationBar::_resetButtons() {
    // Add the LFO buttons
    _lfoButtons.clear();
    for (int index {0}; index < _processor.lfos.size(); index++) {
        _createModulationSourceButton(ModulationSourceDefinition(index + 1, MODULATION_TYPE::LFO));
    }

    // And the add button
    _addLfoButton.reset(new juce::TextButton("Add LFO Button"));
    _lfoButtonsView->getViewedComponent()->addAndMakeVisible(_addLfoButton.get());
    _addLfoButton->setButtonText("+ LFO");
    _addLfoButton->addListener(this);
    _addLfoButton->setTooltip("Creates a new LFO");
    _addLfoButton->setColour(juce::TextButton::buttonColourId, UIUtils::getColourForModulationType(MODULATION_TYPE::LFO));
    _addLfoButton->setColour(juce::TextButton::textColourOffId, UIUtils::getColourForModulationType(MODULATION_TYPE::LFO));
    _addLfoButton->setLookAndFeel(_addButtonLookAndFeel.get());

    // Add the envelope buttons
    _envelopeButtons.clear();
    for (int index {0}; index < _processor.envelopes.size(); index++) {
        _createModulationSourceButton(ModulationSourceDefinition(index + 1, MODULATION_TYPE::ENVELOPE));
    }

    // And the add button
    _addEnvelopeButton.reset(new juce::TextButton("Add Envelope Button"));
    _envelopeButtonsView->getViewedComponent()->addAndMakeVisible(_addEnvelopeButton.get());
    _addEnvelopeButton->setButtonText("+ ENV");
    _addEnvelopeButton->addListener(this);
    _addEnvelopeButton->setTooltip("Creates a new envelope");
    _addEnvelopeButton->setColour(juce::TextButton::buttonColourId, UIUtils::getColourForModulationType(MODULATION_TYPE::ENVELOPE));
    _addEnvelopeButton->setColour(juce::TextButton::textColourOffId, UIUtils::getColourForModulationType(MODULATION_TYPE::ENVELOPE));
    _addEnvelopeButton->setLookAndFeel(_addButtonLookAndFeel.get());

    resized();

    // We need to run this only once after the graph view has been constructed to restore the scroll
    // position to the same as before the UI was last closed
    if (!_hasRestoredScroll) {
        _hasRestoredScroll = true;

        _lfoButtonsView->setViewPosition(0, _processor.mainWindowState.lfoButtonsScrollPosition);
        _envelopeButtonsView->setViewPosition(0, _processor.mainWindowState.envButtonsScrollPosition);
    }
}

void ModulationBar::_createModulationSourceButton(ModulationSourceDefinition definition) {
    std::unique_ptr<ModulationButton> newButton;
    newButton.reset(new ModulationButton(
        definition,
        [&](ModulationButton* button) { _selectModulationSource(button); },
        [&, definition]() { _removeModulationSource(definition); },
        _dragContainer));

    if (definition.type == MODULATION_TYPE::LFO) {
        _lfoButtonsView->getViewedComponent()->addAndMakeVisible(newButton.get());
        _lfoButtons.push_back(std::move(newButton));
    } else if (definition.type == MODULATION_TYPE::ENVELOPE) {
        _envelopeButtonsView->getViewedComponent()->addAndMakeVisible(newButton.get());
        _envelopeButtons.push_back(std::move(newButton));
    }
}

void ModulationBar::_selectModulationSource(ModulationButton* selectedButton) {
    // Deactivate all buttons
    for (std::unique_ptr<ModulationButton>& button : _lfoButtons) {
        button->setIsSelected(false);
    }

    for (std::unique_ptr<ModulationButton>& button : _envelopeButtons) {
        button->setIsSelected(false);
    }

    // Activate just the one that's selected
    selectedButton->setIsSelected(true);

    // Draw the modulaton source UI
    if (selectedButton->definition.type == MODULATION_TYPE::LFO) {
        std::shared_ptr<WECore::Richter::RichterLFO> thisLfo(_processor.lfos[selectedButton->definition.id - 1]);
        _selectedSourceComponent.reset(new ModulationBarLfo(thisLfo));
    } else if (selectedButton->definition.type == MODULATION_TYPE::ENVELOPE) {
        EnvelopeFollowerWrapper& thisEnvelope(_processor.envelopes[selectedButton->definition.id - 1]);
        _selectedSourceComponent.reset(new ModulationBarEnvelope(thisEnvelope));
    }

    addAndMakeVisible(_selectedSourceComponent.get());
    _selectedSourceComponent->setBounds(_selectedSourceComponentArea);
}

void ModulationBar::_removeModulationSource(ModulationSourceDefinition definition) {
    // Get the currently selected definition
    std::optional<ModulationSourceDefinition> selectedDefinition;
    for (int buttonIndex {0}; buttonIndex < _lfoButtons.size() && !selectedDefinition.has_value(); buttonIndex++) {
        if (_lfoButtons[buttonIndex]->getIsSelected()) {
            selectedDefinition = _lfoButtons[buttonIndex]->definition;
        }
    }

    for (int buttonIndex {0}; buttonIndex < _envelopeButtons.size() && !selectedDefinition.has_value(); buttonIndex++) {
        if (_envelopeButtons[buttonIndex]->getIsSelected()) {
            selectedDefinition = _envelopeButtons[buttonIndex]->definition;
        }
    }

    // Delete selected component before changing anything in the processor to make sure nothing in
    // it is referring to anything we might change
    _selectedSourceComponent.reset();

    // Remove the modulation source and rebuild the UI
    _processor.removeModulationSource(definition);
    _resetButtons();

    // Update the selected ID if we're deleting something ahead of it
    if (selectedDefinition.value().id > definition.id) {
        selectedDefinition.value().id--;
    }

    // Decide which definition should now be selected
    if (selectedDefinition.value().type == MODULATION_TYPE::LFO) {
        if (_lfoButtons.size() > selectedDefinition.value().id - 1) {
            // Select the previously selected definition
            _selectModulationSource(_lfoButtons[selectedDefinition.value().id - 1].get());
        } else if (_lfoButtons.size() > 0) {
            // We deleted the last definition, so select the current last one
            _selectModulationSource(_lfoButtons[_lfoButtons.size() - 1].get());
        } else if (_envelopeButtons.size() > 0) {
            // We deleted all the LFOs, so select an envelope follower
            _selectModulationSource(_envelopeButtons[0].get());
        } else {
            // We deleted everything, select nothing
            _selectedSourceComponent.reset();
        }
    } else if (selectedDefinition.value().type == MODULATION_TYPE::ENVELOPE) {
        if (_envelopeButtons.size() > selectedDefinition.value().id - 1) {
            // Restore the previously selected definition
            _selectModulationSource(_envelopeButtons[selectedDefinition.value().id - 1].get());
        } else if (_envelopeButtons.size() > 0) {
            // We deleted the last definition, so select the current last one
            _selectModulationSource(_envelopeButtons[0].get());
        } else if (_lfoButtons.size() > 0) {
            // We deleted all the envelope followers, so select an LFO
            _selectModulationSource(_lfoButtons[0].get());
        } else {
            // We deleted everything, select nothing
            _selectedSourceComponent.reset();
        }
    }
}

/*
  ==============================================================================

  This is an automatically generated GUI class created by the Projucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Projucer version: 6.1.4

  ------------------------------------------------------------------------------

  The Projucer is part of the JUCE library.
  Copyright (c) 2020 - Raw Material Software Limited.

  ==============================================================================
*/

#pragma once

//[Headers]     -- You can add your own extra header files here --
#include <JuceHeader.h>
#include "CoreJUCEPlugin/CoreProcessorEditor.h"
#include "CoreJUCEPlugin/TooltipLabelUpdater.h"
#include "PluginProcessor.h"
#include "MacrosComponent.h"
#include "OutputComponent.h"
#include "SplitterButtonsComponent.h"
#include "SplitterHeaderComponent.h"
#include "PluginSlotComponent.h"
#include "GraphViewComponent.h"
#include "ModulationBar.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Projucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class SyndicateAudioProcessorEditor  : public WECore::JUCEPlugin::CoreProcessorEditor,
                                       public juce::DragAndDropContainer
{
public:
    //==============================================================================
    SyndicateAudioProcessorEditor (SyndicateAudioProcessor& ownerProcessor);
    ~SyndicateAudioProcessorEditor() override;

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    void needsGraphRebuild();
    //[/UserMethods]

    void paint (juce::Graphics& g) override;
    void resized() override;



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    SyndicateAudioProcessor& _processor;
    WECore::JUCEPlugin::TooltipLabelUpdater _tooltipLabelUpdater;
    std::unique_ptr<SplitterHeaderComponent> splitterHeader;
    bool _isHeaderInitialised;
    std::unique_ptr<UIUtils::PopoverComponent> _errorPopover;

    void _enableDoubleClickToDefault();
    void _startSliderReadouts();
    void _stopSliderReadouts();
    void _setSliderRanges();
    void _onParameterUpdate() override;
    void _updateSplitterHeader();
    void _displayErrorsIfNeeded();
    //[/UserVariables]

    //==============================================================================
    std::unique_ptr<MacrosComponent> macrosSidebar;
    std::unique_ptr<SplitterButtonsComponent> splitterButtonsBar;
    std::unique_ptr<OutputComponent> outputSidebar;
    std::unique_ptr<ModulationBar> modulationBar;
    std::unique_ptr<GraphViewComponent> graphView;
    std::unique_ptr<juce::Label> tooltipLbl;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SyndicateAudioProcessorEditor)
};

//[EndFile] You can add extra defines here...
//[/EndFile]


/*
  ==============================================================================

  This is an automatically generated GUI class created by the Projucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Projucer version: 6.1.6

  ------------------------------------------------------------------------------

  The Projucer is part of the JUCE library.
  Copyright (c) 2020 - Raw Material Software Limited.

  ==============================================================================
*/

//[Headers] You can add your own extra header files here...
#include "CoreJUCEPlugin/CoreProcessorEditor.h"
#include "LeftrightSplitterSubComponent.h"
#include "MidsideSplitterSubComponent.h"
#include "MultibandSplitterSubComponent.h"
#include "ParallelSplitterSubComponent.h"
#include "ParameterData.h"
#include "SeriesSplitterSubComponent.h"
//[/Headers]

#include "PluginEditor.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
SyndicateAudioProcessorEditor::SyndicateAudioProcessorEditor (SyndicateAudioProcessor& ownerProcessor)
    : CoreProcessorEditor(ownerProcessor), _processor(ownerProcessor), _isHeaderInitialised(false)
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]

    macrosSidebar.reset (new MacrosComponent (this, _processor.macros, _processor.macroNames));
    addAndMakeVisible (macrosSidebar.get());
    macrosSidebar->setName ("Macros");

    macrosSidebar->setBounds (0, 0, 64, 526);

    splitterButtonsBar.reset (new SplitterButtonsComponent (_processor));
    addAndMakeVisible (splitterButtonsBar.get());
    splitterButtonsBar->setName ("Splitter Buttons");

    splitterButtonsBar->setBounds (64, 0, 572, 40);

    outputSidebar.reset (new OutputComponent (_processor));
    addAndMakeVisible (outputSidebar.get());
    outputSidebar->setName ("Output");

    outputSidebar->setBounds (636, 120, 64, 406);

    modulationBar.reset (new ModulationBar (_processor, this));
    addAndMakeVisible (modulationBar.get());
    modulationBar->setName ("Modulation");

    modulationBar->setBounds (64, 396, 572, 130);

    graphView.reset (new GraphViewComponent (_processor));
    addAndMakeVisible (graphView.get());
    graphView->setName ("Graph View");

    graphView->setBounds (64, 120, 572, 276);

    tooltipLbl.reset (new juce::Label ("Tooltip Label",
                                       juce::String()));
    addAndMakeVisible (tooltipLbl.get());
    tooltipLbl->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    tooltipLbl->setJustificationType (juce::Justification::centred);
    tooltipLbl->setEditable (false, false, false);
    tooltipLbl->setColour (juce::Label::textColourId, juce::Colour (0xff929292));
    tooltipLbl->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    tooltipLbl->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    tooltipLbl->setBounds (8, 525, 684, 24);

    headerExtensionComponent.reset (new juce::Component());
    addAndMakeVisible (headerExtensionComponent.get());
    headerExtensionComponent->setName ("Header Extension Component");

    headerExtensionComponent->setBounds (636, 40, 64, 80);


    //[UserPreSize]
    //[/UserPreSize]

    setSize (700, 550);


    //[Constructor] You can add your own custom stuff here..
    _processor.setEditor(this);

    _setSliderRanges();

    // Start tooltip label
    addMouseListener(&_tooltipLabelUpdater, true);
    _tooltipLabelUpdater.start(tooltipLbl.get(), getAudioProcessor()->wrapperType);

    // Call this once to force an update
    needsGraphRebuild();
    _onParameterUpdate();

    _displayErrorsIfNeeded();

    //[/Constructor]
}

SyndicateAudioProcessorEditor::~SyndicateAudioProcessorEditor()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    _processor.setEditor(nullptr);
    _tooltipLabelUpdater.stop();
    //[/Destructor_pre]

    macrosSidebar = nullptr;
    splitterButtonsBar = nullptr;
    outputSidebar = nullptr;
    modulationBar = nullptr;
    graphView = nullptr;
    tooltipLbl = nullptr;
    headerExtensionComponent = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void SyndicateAudioProcessorEditor::paint (juce::Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (juce::Colour (0xff272727));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void SyndicateAudioProcessorEditor::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    if (_errorPopover != nullptr) {
        _errorPopover->setBounds(getLocalBounds());
    }
    //[/UserPreResize]

    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void SyndicateAudioProcessorEditor::needsGraphRebuild() {
    splitterButtonsBar->onParameterUpdate();
    _updateSplitterHeader();
    graphView->onParameterUpdate();
}

void SyndicateAudioProcessorEditor::_enableDoubleClickToDefault() {
    // TODO
}

void SyndicateAudioProcessorEditor::_startSliderReadouts() {
    // TODO
}

void SyndicateAudioProcessorEditor::_stopSliderReadouts() {
    // TODO
}

void SyndicateAudioProcessorEditor::_setSliderRanges() {
    // TODO
}

void SyndicateAudioProcessorEditor::_onParameterUpdate() {
    splitterHeader->onParameterUpdate();
    outputSidebar->onParameterUpdate();
    macrosSidebar->onParameterUpdate();
}

void SyndicateAudioProcessorEditor::_updateSplitterHeader() {
    // Cache the previous value so we don't have recreate the component when nothing has changed
    static SPLIT_TYPE previousSplitType {0};

    if (_processor.getSplitType() != previousSplitType || !_isHeaderInitialised) {
        previousSplitType = _processor.getSplitType();
        _isHeaderInitialised = true;

        switch (_processor.getSplitType()) {
            case SPLIT_TYPE::SERIES:
                splitterHeader.reset(new SeriesSplitterSubComponent(_processor.chainParameters[0]));
                break;
            case SPLIT_TYPE::PARALLEL:
                splitterHeader.reset(new ParallelSplitterSubComponent(_processor, headerExtensionComponent.get()));
                break;
            case SPLIT_TYPE::MULTIBAND:
                splitterHeader.reset(new MultibandSplitterSubComponent(_processor, headerExtensionComponent.get()));
                break;
            case SPLIT_TYPE::LEFTRIGHT:
                splitterHeader.reset(new LeftrightSplitterSubComponent(_processor.chainParameters[0], _processor.chainParameters[1]));
                break;
            case SPLIT_TYPE::MIDSIDE:
                splitterHeader.reset(new MidsideSplitterSubComponent(_processor.chainParameters[0], _processor.chainParameters[1]));
                break;
        }

        addAndMakeVisible(splitterHeader.get());
        splitterHeader->setName("Splitter Header");
        splitterHeader->setBounds(64, 40, 573, 80);
    }

    splitterHeader->onParameterUpdate();
}

void SyndicateAudioProcessorEditor::_displayErrorsIfNeeded() {

    if (_processor.restoreErrors.size() > 0) {
        juce::String bodyText;

        for (juce::String& error : _processor.restoreErrors) {
            bodyText += error;
            bodyText += "\n";
        }

        _errorPopover.reset(new UIUtils::PopoverComponent("Encountered the following errors while restoring plugin state:", bodyText, [&]() {_errorPopover.reset(); }));
        addAndMakeVisible(_errorPopover.get());
        _errorPopover->setBounds(getLocalBounds());

        _processor.restoreErrors.clear();
    }
}

//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

    This is where the Projucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="SyndicateAudioProcessorEditor"
                 componentName="" parentClasses="public WECore::JUCEPlugin::CoreProcessorEditor, public juce::DragAndDropContainer"
                 constructorParams="SyndicateAudioProcessor&amp; ownerProcessor"
                 variableInitialisers="CoreProcessorEditor(ownerProcessor), _processor(ownerProcessor), _isHeaderInitialised(false)"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="1" initialWidth="700" initialHeight="550">
  <BACKGROUND backgroundColour="ff272727"/>
  <GENERICCOMPONENT name="Macros" id="457c6719877e7b6b" memberName="macrosSidebar"
                    virtualName="MacrosComponent" explicitFocusOrder="0" pos="0 0 64 526"
                    class="juce::Component" params="this, _processor.macros, _processor.macroNames"/>
  <GENERICCOMPONENT name="Splitter Buttons" id="f0b13db8ae7b5461" memberName="splitterButtonsBar"
                    virtualName="SplitterButtonsComponent" explicitFocusOrder="0"
                    pos="64 0 572 40" class="juce::Component" params="_processor"/>
  <GENERICCOMPONENT name="Output" id="5e939e07c46125f3" memberName="outputSidebar"
                    virtualName="OutputComponent" explicitFocusOrder="0" pos="636 120 64 406"
                    class="juce::Component" params="_processor"/>
  <GENERICCOMPONENT name="Modulation" id="de122523938b745c" memberName="modulationBar"
                    virtualName="ModulationBar" explicitFocusOrder="0" pos="64 396 572 130"
                    class="juce::Component" params="_processor, this"/>
  <GENERICCOMPONENT name="Graph View" id="beaec81e6712632e" memberName="graphView"
                    virtualName="GraphViewComponent" explicitFocusOrder="0" pos="64 120 572 276"
                    class="juce::Component" params="_processor"/>
  <LABEL name="Tooltip Label" id="37c38fbe0fd8f213" memberName="tooltipLbl"
         virtualName="" explicitFocusOrder="0" pos="8 525 684 24" textCol="ff929292"
         edTextCol="ff000000" edBkgCol="0" labelText="" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.0" kerning="0.0" bold="0" italic="0" justification="36"/>
  <GENERICCOMPONENT name="Header Extension Component" id="d0575d325a1b0f48" memberName="headerExtensionComponent"
                    virtualName="" explicitFocusOrder="0" pos="636 40 64 80" class="juce::Component"
                    params=""/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]


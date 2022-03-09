/*
  ==============================================================================

  This is an automatically generated GUI class created by the Projucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Projucer version: 6.0.8

  ------------------------------------------------------------------------------

  The Projucer is part of the JUCE library.
  Copyright (c) 2020 - Raw Material Software Limited.

  ==============================================================================
*/

//[Headers] You can add your own extra header files here...
#include "PluginSelectorComponent.h"
#include "PluginSelectorListParameters.h"
//[/Headers]

#include "PluginEditor.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
SyndicateAudioProcessorEditor::SyndicateAudioProcessorEditor (SyndicateAudioProcessor& ownerProcessor)
    : AudioProcessorEditor (&ownerProcessor), _processor (ownerProcessor)
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]

    statusBar.reset (new PluginScanStatusBar (_processor.pluginScanner));
    addAndMakeVisible (statusBar.get());
    statusBar->setName ("Plugin Scan Status Bar");

    statusBar->setBounds (0, 376, 440, 24);

    juce__tabbedComponent.reset (new juce::TabbedComponent (juce::TabbedButtonBar::TabsAtTop));
    addAndMakeVisible (juce__tabbedComponent.get());
    juce__tabbedComponent->setTabBarDepth (30);
    juce__tabbedComponent->addTab (TRANS("Series"), juce::Colours::lightgrey, 0, false);
    juce__tabbedComponent->addTab (TRANS("Parallel"), juce::Colours::lightgrey, 0, false);
    juce__tabbedComponent->addTab (TRANS("Multiband"), juce::Colours::lightgrey, 0, false);
    juce__tabbedComponent->addTab (TRANS("Left/Right"), juce::Colours::lightgrey, 0, false);
    juce__tabbedComponent->addTab (TRANS("Mid/Side"), juce::Colours::lightgrey, 0, false);
    juce__tabbedComponent->setCurrentTabIndex (0);

    juce__tabbedComponent->setBounds (16, 16, 488, 64);

    juce__component.reset (new juce::Component());
    addAndMakeVisible (juce__component.get());
    juce__component->setName ("new component");

    juce__component->setBounds (16, 152, 232, 136);

    juce__component2.reset (new juce::Component());
    addAndMakeVisible (juce__component2.get());
    juce__component2->setName ("new component");

    juce__component2->setBounds (16, 304, 456, 56);

    juce__component3.reset (new juce::Component());
    addAndMakeVisible (juce__component3.get());
    juce__component3->setName ("new component");

    juce__component3->setBounds (344, 96, 88, 184);

    juce__slider.reset (new juce::Slider ("new slider"));
    addAndMakeVisible (juce__slider.get());
    juce__slider->setRange (0, 10, 0);
    juce__slider->setSliderStyle (juce::Slider::LinearVertical);
    juce__slider->setTextBoxStyle (juce::Slider::NoTextBox, false, 80, 20);
    juce__slider->addListener (this);

    juce__slider->setBounds (272, 104, 46, 184);

    juce__slider2.reset (new juce::Slider ("new slider"));
    addAndMakeVisible (juce__slider2.get());
    juce__slider2->setRange (0, 10, 0);
    juce__slider2->setSliderStyle (juce::Slider::LinearVertical);
    juce__slider2->setTextBoxStyle (juce::Slider::NoTextBox, false, 80, 20);
    juce__slider2->addListener (this);

    juce__slider2->setBounds (448, 104, 46, 184);

    pluginSelectorBtn.reset (new juce::TextButton ("Plugin Selector Button"));
    addAndMakeVisible (pluginSelectorBtn.get());
    pluginSelectorBtn->setButtonText (TRANS("Select a plugin"));
    pluginSelectorBtn->addListener (this);

    pluginSelectorBtn->setBounds (24, 104, 104, 24);

    pluginBtn.reset (new juce::TextButton ("Plugin Button"));
    addAndMakeVisible (pluginBtn.get());
    pluginBtn->setButtonText (TRANS("No plugin"));
    pluginBtn->addListener (this);

    pluginBtn->setBounds (136, 104, 112, 24);


    //[UserPreSize]
    //[/UserPreSize]

    setSize (600, 400);


    //[Constructor] You can add your own custom stuff here..

    // Initialise plugin button
    if (_processor.guestPlugin != nullptr) {
        pluginBtn->setButtonText(_processor.guestPlugin->getPluginDescription().name);
    } else {
        pluginBtn->setEnabled(false);
    }

    //[/Constructor]
}

SyndicateAudioProcessorEditor::~SyndicateAudioProcessorEditor()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    statusBar = nullptr;
    juce__tabbedComponent = nullptr;
    juce__component = nullptr;
    juce__component2 = nullptr;
    juce__component3 = nullptr;
    juce__slider = nullptr;
    juce__slider2 = nullptr;
    pluginSelectorBtn = nullptr;
    pluginBtn = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void SyndicateAudioProcessorEditor::paint (juce::Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (juce::Colour (0xff323e44));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void SyndicateAudioProcessorEditor::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void SyndicateAudioProcessorEditor::sliderValueChanged (juce::Slider* sliderThatWasMoved)
{
    //[UsersliderValueChanged_Pre]
    //[/UsersliderValueChanged_Pre]

    if (sliderThatWasMoved == juce__slider.get())
    {
        //[UserSliderCode_juce__slider] -- add your slider handling code here..
        //[/UserSliderCode_juce__slider]
    }
    else if (sliderThatWasMoved == juce__slider2.get())
    {
        //[UserSliderCode_juce__slider2] -- add your slider handling code here..
        //[/UserSliderCode_juce__slider2]
    }

    //[UsersliderValueChanged_Post]
    //[/UsersliderValueChanged_Post]
}

void SyndicateAudioProcessorEditor::buttonClicked (juce::Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == pluginSelectorBtn.get())
    {
        //[UserButtonCode_pluginSelectorBtn] -- add your button handler code here..
        PluginSelectorListParameters parameters {
            _processor.pluginScanner,
            _processor.pluginSelectorState,
            [&](std::unique_ptr<juce::AudioPluginInstance> plugin, const juce::String& error) { _onPluginSelected(std::move(plugin), error); },
            [&]() { return _processor.getSampleRate(); },
            [&]() { return _processor.getBlockSize(); },
        };

        _pluginSelectorWindow = std::make_unique<PluginSelectorWindow>(
            [&]() { _pluginSelectorWindow.reset(); }, parameters
        );
        //[/UserButtonCode_pluginSelectorBtn]
    }
    else if (buttonThatWasClicked == pluginBtn.get())
    {
        //[UserButtonCode_pluginBtn] -- add your button handler code here..
        // Only open the plugin window if a plugin exists and the window isn't open already
        if (_processor.guestPlugin != nullptr && _guestPluginWindow == nullptr) {
            _guestPluginWindow = std::make_unique<GuestPluginWindow>(_guestPluginWindow, _processor.guestPlugin);
        }
        //[/UserButtonCode_pluginBtn]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void SyndicateAudioProcessorEditor::_onPluginSelected(std::unique_ptr<juce::AudioPluginInstance> plugin, const juce::String& error) {

    if (plugin != nullptr) {
        juce::Logger::writeToLog("SyndicateAudioProcessorEditor::_onPluginSelected: Loaded plugin");

        // Pass the plugin to the processor
        _processor.onPluginSelectedByUser(std::move(plugin));

        // Create the plugin window
        _guestPluginWindow.reset(new GuestPluginWindow(_guestPluginWindow, _processor.guestPlugin));

        // Close the selector window
        _pluginSelectorWindow.reset();

        // Update the button
        pluginBtn->setButtonText(_processor.guestPlugin->getPluginDescription().name);
        pluginBtn->setEnabled(true);

    } else {
        // Plugin failed to load
        juce::Logger::writeToLog("SyndicateAudioProcessorEditor::_onPluginSelected: Failed to load plugin: " + error);
        // TODO: display the error
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
                 componentName="" parentClasses="public juce::AudioProcessorEditor"
                 constructorParams="SyndicateAudioProcessor&amp; ownerProcessor"
                 variableInitialisers="AudioProcessorEditor (&amp;ownerProcessor), _processor (ownerProcessor)"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="1" initialWidth="600" initialHeight="400">
  <BACKGROUND backgroundColour="ff323e44"/>
  <GENERICCOMPONENT name="Plugin Scan Status Bar" id="1b592eb960e0d7af" memberName="statusBar"
                    virtualName="PluginScanStatusBar" explicitFocusOrder="0" pos="0 376 440 24"
                    class="juce::Component" params="_processor.pluginScanner"/>
  <TABBEDCOMPONENT name="new tabbed component" id="c9c23529705d6617" memberName="juce__tabbedComponent"
                   virtualName="" explicitFocusOrder="0" pos="16 16 488 64" orientation="top"
                   tabBarDepth="30" initialTab="0">
    <TAB name="Series" colour="ffd3d3d3" useJucerComp="0" contentClassName=""
         constructorParams="" jucerComponentFile=""/>
    <TAB name="Parallel" colour="ffd3d3d3" useJucerComp="0" contentClassName=""
         constructorParams="" jucerComponentFile=""/>
    <TAB name="Multiband" colour="ffd3d3d3" useJucerComp="0" contentClassName=""
         constructorParams="" jucerComponentFile=""/>
    <TAB name="Left/Right" colour="ffd3d3d3" useJucerComp="0" contentClassName=""
         constructorParams="" jucerComponentFile=""/>
    <TAB name="Mid/Side" colour="ffd3d3d3" useJucerComp="0" contentClassName=""
         constructorParams="" jucerComponentFile=""/>
  </TABBEDCOMPONENT>
  <GENERICCOMPONENT name="new component" id="c39c33131a8ad7c3" memberName="juce__component"
                    virtualName="" explicitFocusOrder="0" pos="16 152 232 136" class="juce::Component"
                    params=""/>
  <GENERICCOMPONENT name="new component" id="af7008a0150e6f8e" memberName="juce__component2"
                    virtualName="" explicitFocusOrder="0" pos="16 304 456 56" class="juce::Component"
                    params=""/>
  <GENERICCOMPONENT name="new component" id="44f62096bab10294" memberName="juce__component3"
                    virtualName="" explicitFocusOrder="0" pos="344 96 88 184" class="juce::Component"
                    params=""/>
  <SLIDER name="new slider" id="1b9dd8e6eb193c91" memberName="juce__slider"
          virtualName="" explicitFocusOrder="0" pos="272 104 46 184" min="0.0"
          max="10.0" int="0.0" style="LinearVertical" textBoxPos="NoTextBox"
          textBoxEditable="1" textBoxWidth="80" textBoxHeight="20" skewFactor="1.0"
          needsCallback="1"/>
  <SLIDER name="new slider" id="21d69bfa2ba63b5e" memberName="juce__slider2"
          virtualName="" explicitFocusOrder="0" pos="448 104 46 184" min="0.0"
          max="10.0" int="0.0" style="LinearVertical" textBoxPos="NoTextBox"
          textBoxEditable="1" textBoxWidth="80" textBoxHeight="20" skewFactor="1.0"
          needsCallback="1"/>
  <TEXTBUTTON name="Plugin Selector Button" id="bbbcd217371ce72" memberName="pluginSelectorBtn"
              virtualName="" explicitFocusOrder="0" pos="24 104 104 24" buttonText="Select a plugin"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="Plugin Button" id="f63459aafbd03997" memberName="pluginBtn"
              virtualName="" explicitFocusOrder="0" pos="136 104 112 24" buttonText="No plugin"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]


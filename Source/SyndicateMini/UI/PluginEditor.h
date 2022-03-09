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

#pragma once

//[Headers]     -- You can add your own extra header files here --
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PluginScanStatusBar.h"
#include "PluginSelectorWindow.h"
#include "GuestPluginWindow.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Projucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class SyndicateAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                       public juce::Slider::Listener,
                                       public juce::Button::Listener
{
public:
    //==============================================================================
    SyndicateAudioProcessorEditor (SyndicateAudioProcessor& ownerProcessor);
    ~SyndicateAudioProcessorEditor() override;

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    //[/UserMethods]

    void paint (juce::Graphics& g) override;
    void resized() override;
    void sliderValueChanged (juce::Slider* sliderThatWasMoved) override;
    void buttonClicked (juce::Button* buttonThatWasClicked) override;



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    SyndicateAudioProcessor& _processor;
    std::unique_ptr<PluginSelectorWindow> _pluginSelectorWindow;
    std::unique_ptr<GuestPluginWindow> _guestPluginWindow;
    void _onPluginSelected(std::unique_ptr<juce::AudioPluginInstance> plugin, const juce::String& error);
    //[/UserVariables]

    //==============================================================================
    std::unique_ptr<PluginScanStatusBar> statusBar;
    std::unique_ptr<juce::TabbedComponent> juce__tabbedComponent;
    std::unique_ptr<juce::Component> juce__component;
    std::unique_ptr<juce::Component> juce__component2;
    std::unique_ptr<juce::Component> juce__component3;
    std::unique_ptr<juce::Slider> juce__slider;
    std::unique_ptr<juce::Slider> juce__slider2;
    std::unique_ptr<juce::TextButton> pluginSelectorBtn;
    std::unique_ptr<juce::TextButton> pluginBtn;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SyndicateAudioProcessorEditor)
};

//[EndFile] You can add extra defines here...
//[/EndFile]


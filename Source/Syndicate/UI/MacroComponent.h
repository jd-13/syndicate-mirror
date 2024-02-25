#pragma once

#include <JuceHeader.h>
#include "ModulationSourceDefinition.hpp"
#include "UIUtils.h"

#if defined(__APPLE__) && defined(__aarch64__)
// Workaround for an Apple M1 bug after resize
// https://forum.juce.com/t/bug-m1-apple-silicon-keyboard-focus-lost-permanently-in-logic-when-plugin-is-resized/51292
// TODO remove this when the bug is resolved
#define RESIZE_DISABLE_EDIT
#endif

class MacroComponent  : public juce::Component,
                        public juce::Slider::Listener,
                        public juce::Label::Listener,
                        public juce::SettableTooltipClient {
public:
    MacroComponent(int macroNumber,
                   juce::DragAndDropContainer* dragContainer,
                   juce::AudioParameterFloat* macroParam,
                   const juce::String& macroName);
    ~MacroComponent() override;

    void onParameterUpdate();
    void updateName(juce::String name);

    void resized() override;
    void sliderValueChanged(juce::Slider* sliderThatWasMoved) override;
    void sliderDragStarted(juce::Slider* slider) override;
    void sliderDragEnded(juce::Slider* slider) override;
    void labelTextChanged(juce::Label* labelThatHasChanged) override { }
    void mouseDrag(const juce::MouseEvent& e) override;

#ifdef RESIZE_DISABLE_EDIT
    void disableEdit() {
        nameLbl->setEditable(false, false, false);
    }
#endif

private:
    juce::DragAndDropContainer* _dragContainer;
    ModulationSourceDefinition _modulationSourceDefinition;
    juce::AudioParameterFloat* _macroParam;
    UIUtils::StandardSliderLookAndFeel _sliderLookAndFeel;

    std::unique_ptr<juce::Slider> macroSld;
    std::unique_ptr<juce::Label> nameLbl;
    std::unique_ptr<UIUtils::DragHandle> dragHandle;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MacroComponent)
};

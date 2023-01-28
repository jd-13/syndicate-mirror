#pragma once

#include <JuceHeader.h>
#include "MacroComponent.h"
#include "ParameterData.h"

#if defined(__APPLE__) && defined(__aarch64__)
// Workaround for an Apple M1 bug after resize
// https://forum.juce.com/t/bug-m1-apple-silicon-keyboard-focus-lost-permanently-in-logic-when-plugin-is-resized/51292
// TODO remove this when the bug is resolved
#define RESIZE_DISABLE_EDIT
#endif

class MacrosComponent  : public juce::Component
{
public:
    MacrosComponent(juce::DragAndDropContainer* dragContainer,
                    std::array<juce::AudioParameterFloat*, NUM_MACROS>& macroParams,
                    std::array<juce::String, NUM_MACROS>& macroNames);
    ~MacrosComponent() override;

    void resized() override;

    void onParameterUpdate();

private:
    std::vector<std::unique_ptr<MacroComponent>> _macroComponents;

#ifdef RESIZE_DISABLE_EDIT
    bool _hasAlreadyBeenResized;
#endif
};

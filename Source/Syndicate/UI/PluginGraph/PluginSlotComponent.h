#pragma once

#include <JuceHeader.h>
#include "BaseSlotComponent.h"
#include "PluginSelectionInterface.h"
#include "PluginSlotModulationTray.h"

class PluginSlotComponent : public BaseSlotComponent,
                            juce::Button::Listener {
public:
    PluginSlotComponent(PluginSelectionInterface& pluginSelectionInterface,
                        PluginModulationInterface& pluginModulationInterface,
                        int chainNumber,
                        int pluginNumber);
    ~PluginSlotComponent() override;

    void resized() override;
    void mouseMove(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;
    void buttonClicked(juce::Button* buttonThatWasClicked) override;
    void paint(juce::Graphics& g) override;
#if JUCE_IOS
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
#endif

private:
    PluginSelectionInterface& _pluginSelectionInterface;
    PluginModulationInterface& _pluginModulationInterface;
    bool _isHover;
    std::unique_ptr<PluginSlotModulationTray> _modulationTray;
#if JUCE_IOS
    std::unique_ptr<UIUtils::LongPressHandler> _longPressHandler;
#endif

    std::unique_ptr<UIUtils::BypassButton> _bypassButton;
    std::unique_ptr<UIUtils::PluginOpenButton> _openButton;
    std::unique_ptr<UIUtils::PluginReplaceButton> _replaceButton;
    std::unique_ptr<UIUtils::CrossButton> _deleteButton;
    std::unique_ptr<UIUtils::ModulationButton> _modulationButton;
    std::unique_ptr<juce::Label> _descriptionLabel;
};

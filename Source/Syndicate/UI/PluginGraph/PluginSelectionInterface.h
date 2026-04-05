#pragma once

#include <JuceHeader.h>
#if JUCE_IOS
#include "IOSPluginSelectorOverlay.h"
#include "GuestPluginOverlay.h"
#else
#include "PluginSelectorWindow.h"
#include "GuestPluginWindow.h"
#endif
#include "UIUtils.h"

class SyndicateAudioProcessor;

/**
 * The interface between the processor and parts of the UI that control plugin selection.
 */
class PluginSelectionInterface {
public:
    PluginSelectionInterface(SyndicateAudioProcessor& processor);
    ~PluginSelectionInterface() = default;

    void selectNewPlugin(int chainNumber, int pluginNumber);
    juce::String getPluginName(int chainNumber, int pluginNumber);
    void openPluginEditor(int chainNumber, int pluginNumber);
    void removePlugin(int chainNumber, int pluginNumber);
    void togglePluginBypass(int chainNumber, int pluginNumber);
    bool getPluginBypass(int chainNumber, int pluginNumber);
    void insertGainStage(int chainNumber, int pluginNumber);
    void copySlot(int fromChainNumber, int fromSlotNumber, int toChainNumber, int toSlotNumber);
    void moveSlot(int fromChainNumber, int fromSlotNumber, int toChainNumber, int toSlotNumber);

    /**
     * Returns true if it's a plugin in this slot, otherwise false.
     */
    bool isPluginSlot(int chainNumber, int slotNumber);

    void setGainStageGain(int chainNumber, int slotNumber, float gain);
    std::tuple<float, float> getGainStageGainAndPan(int chainNumber, int slotNumber);
    void setGainStagePan(int chainNumber, int slotNumber, float pan);
    int getNumMainChannels() const;
    float getGainStageOutputAmplitude(int chainNumber, int slotNumber, int channelNumber) const;

    void closeGuestPluginWindows();

private:
    SyndicateAudioProcessor& _processor;
#if JUCE_IOS
    std::unique_ptr<IOSPluginSelectorOverlay> _pluginSelectorOverlay;
    std::unique_ptr<GuestPluginOverlay> _guestPluginOverlay;
#else
    std::unique_ptr<PluginSelectorWindow> _pluginSelectorWindow;
    std::vector<std::unique_ptr<GuestPluginWindow>> _guestPluginWindows;
#endif
    std::unique_ptr<UIUtils::PopoverComponent> _errorPopover;

    int _chainNumber;
    int _pluginNumber;

    void _onPluginSelected(std::unique_ptr<juce::AudioPluginInstance> plugin, const juce::String& error, bool shouldClose);
    void _onPluginWindowClose(std::shared_ptr<juce::AudioPluginInstance> plugin);
};

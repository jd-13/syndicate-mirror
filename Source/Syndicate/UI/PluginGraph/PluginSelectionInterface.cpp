#include "PluginSelectionInterface.h"
#include "UIUtils.h"
#include "PluginUtils.h"

PluginSelectionInterface::PluginSelectionInterface(SyndicateAudioProcessor& processor)
    : _processor(processor),
      _chainNumber(0),
      _pluginNumber(0) {
}

void PluginSelectionInterface::selectNewPlugin(int chainNumber, int pluginNumber) {
    _chainNumber = chainNumber;
    _pluginNumber = pluginNumber;

    PluginSelectorListParameters parameters {
        _processor.pluginScanClient,
        _processor.pluginSelectorState,
        [&](std::unique_ptr<juce::AudioPluginInstance> plugin, const juce::String& error) { _onPluginSelected(std::move(plugin), error); },
        [&]() { return _processor.getSampleRate(); },
        [&]() { return _processor.getBlockSize(); },
    };

    std::unique_ptr<SelectorComponentStyle> style = std::make_unique<SelectorComponentStyle>(
        UIUtils::backgroundColour,
        UIUtils::neutralHighlightColour,
        UIUtils::neutralControlColour,
        std::make_unique<UIUtils::SearchBarLookAndFeel>(),
        std::make_unique<UIUtils::ToggleButtonLookAndFeel>(),
        std::make_unique<UIUtils::StaticButtonLookAndFeel>(),
        std::make_unique<UIUtils::TableHeaderLookAndFeel>()
    );

    _pluginSelectorWindow = std::make_unique<PluginSelectorWindow>(
        [&]() { _errorPopover.reset(); _pluginSelectorWindow.reset(); }, parameters, std::move(style)
    );

    _pluginSelectorWindow->takeFocus();
}

juce::String PluginSelectionInterface::getPluginName(int chainNumber, int pluginNumber) {
    juce::String retVal;

    std::shared_ptr<juce::AudioPluginInstance> plugin =
        _processor.pluginSplitter->getPlugin(chainNumber, pluginNumber);

    if (plugin != nullptr) {
        retVal = plugin->getPluginDescription().name;
    }

    return retVal;
}

void PluginSelectionInterface::openPluginEditor(int chainNumber, int pluginNumber) {
    std::shared_ptr<juce::AudioPluginInstance> plugin =
        _processor.pluginSplitter->getPlugin(chainNumber, pluginNumber);

    if (plugin != nullptr) {
        // Check if a window is already open for this plugin
        for (const std::unique_ptr<GuestPluginWindow>& window : _guestPluginWindows) {
            if (window->plugin == plugin) {
                // Don't open another window for this plugin
                return;
            }
        }

        std::shared_ptr<PluginEditorBounds> editorBounds =
            _processor.pluginSplitter->getPluginEditorBounds(chainNumber, pluginNumber);
        _guestPluginWindows.emplace_back(new GuestPluginWindow([&, plugin]() { _onPluginWindowClose(plugin); }, plugin, editorBounds));
    }
}

void PluginSelectionInterface::removePlugin(int chainNumber, int pluginNumber) {
    std::shared_ptr<juce::AudioPluginInstance> plugin =
        _processor.pluginSplitter->getPlugin(chainNumber, pluginNumber);

    if (plugin != nullptr) {
        // Check if the plugin owns an open editor window and close it
        for (int index {0}; index < _guestPluginWindows.size(); index++) {
            if (_guestPluginWindows[index]->plugin == plugin) {
                _guestPluginWindows.erase(_guestPluginWindows.begin() + index);
                break;
            }
        }
    }

    // Actually remove the plugin
    _processor.removePlugin(chainNumber, pluginNumber);
}

void PluginSelectionInterface::togglePluginBypass(int chainNumber, int pluginNumber) {
    if (_processor.pluginSplitter != nullptr) {
        _processor.pluginSplitter->setSlotBypass(
            chainNumber, pluginNumber, !_processor.pluginSplitter->getSlotBypass(chainNumber, pluginNumber));
    }
}

bool PluginSelectionInterface::getPluginBypass(int chainNumber, int pluginNumber) {
    if (_processor.pluginSplitter != nullptr) {
        return _processor.pluginSplitter->getSlotBypass(chainNumber, pluginNumber);
    }
    return false;
}

void PluginSelectionInterface::insertGainStage(int chainNumber, int pluginNumber) {
    _processor.insertGainStage(chainNumber, pluginNumber);
}

void PluginSelectionInterface::moveSlot(int fromChainNumber, int fromSlotNumber, int toChainNumber, int toSlotNumber) {
    _processor.moveSlot(fromChainNumber, fromSlotNumber, toChainNumber, toSlotNumber);
}

void PluginSelectionInterface::_onPluginSelected(std::unique_ptr<juce::AudioPluginInstance> plugin, const juce::String& error) {
    auto createErrorPopover = [&](juce::String errorText) {
        if (_pluginSelectorWindow != nullptr) {
            _errorPopover.reset(new UIUtils::PopoverComponent(
                "Failed to load plugin", errorText, [&]() {_errorPopover.reset(); }));

            juce::Component* targetComponent = _pluginSelectorWindow->findChildWithID(Utils::pluginSelectorComponentID);
            if (targetComponent != nullptr) {
                targetComponent->addAndMakeVisible(_errorPopover.get());
                _errorPopover->setBounds(targetComponent->getLocalBounds());
            }
        }
    };

    if (plugin != nullptr) {
        juce::Logger::writeToLog("PluginSelectionInterface::_onPluginSelected: Loading plugin");

        // Create the shared pointer here as we need it for the window
        std::shared_ptr<juce::AudioPluginInstance> sharedPlugin = std::move(plugin);

        // If we are replacing a previous plugin, we need to check if it had a window open and close
        // it
        const std::shared_ptr<juce::AudioPluginInstance> previousPlugin =
            _processor.pluginSplitter->getPlugin(_chainNumber, _pluginNumber);

        if (previousPlugin != nullptr) {
            for (int index {0}; index < _guestPluginWindows.size(); index++) {
                if (_guestPluginWindows[index]->plugin == previousPlugin) {
                    _guestPluginWindows.erase(_guestPluginWindows.begin() + index);
                    break;
                }
            }
        }

        // Pass the plugin to the processor
        if (_processor.onPluginSelectedByUser(sharedPlugin, _chainNumber, _pluginNumber)) {
            juce::Logger::writeToLog("PluginSelectionInterface::_onPluginSelected: Loaded successfully");

            // Create the new plugin window
            std::shared_ptr<PluginEditorBounds> editorBounds =
                _processor.pluginSplitter->getPluginEditorBounds(_chainNumber, _pluginNumber);
            _guestPluginWindows.emplace_back(new GuestPluginWindow([&, sharedPlugin]() { _onPluginWindowClose(sharedPlugin); },
                                                                   sharedPlugin,
                                                                   editorBounds));

            // Close the selector window
            _pluginSelectorWindow.reset();
        } else {
            juce::Logger::writeToLog("PluginSelectionInterface::_onPluginSelected: Failed to load " + sharedPlugin->getPluginDescription().name);

            // Failed to load plugin, show error in selector window
            createErrorPopover(sharedPlugin->getPluginDescription().name + " doesn't support the required inputs/outputs\nThis may be synth, or a mono only plugin being loaded into a stereo instance of Syndicate or vice versa");
        }
    } else {
        // Plugin failed to load
        juce::Logger::writeToLog("PluginSelectionInterface::_onPluginSelected: Failed to load plugin: " + error);
        createErrorPopover(error);
    }
}

void PluginSelectionInterface::_onPluginWindowClose(std::shared_ptr<juce::AudioPluginInstance> plugin) {
    // Close/delete the window
    for (int index {0}; index < _guestPluginWindows.size(); index++) {
        if (_guestPluginWindows[index]->plugin == plugin) {
            _guestPluginWindows.erase(_guestPluginWindows.begin() + index);
            break;
        }
    }
}

bool PluginSelectionInterface::isPluginSlot(int chainNumber, int slotNumber) {
    // TODO implement a more reliable way of checking this
    std::shared_ptr<juce::AudioPluginInstance> plugin =
        _processor.pluginSplitter->getPlugin(chainNumber, slotNumber);

    return plugin != nullptr;
}

void PluginSelectionInterface::setGainStageGain(int chainNumber, int slotNumber, float gain) {
    if (_processor.pluginSplitter != nullptr) {
        _processor.pluginSplitter->setGainLinear(chainNumber, slotNumber, gain);
    }
}

float PluginSelectionInterface::getGainStageGain(int chainNumber, int slotNumber) {
    if (_processor.pluginSplitter != nullptr) {
        return _processor.pluginSplitter->getGainLinear(chainNumber, slotNumber);
    }

    return 0.0f;
}

std::optional<GainStageLevelsInterface> PluginSelectionInterface::getGainStageLevelsInterface(int chainNumber, int slotNumber) {
    if (_processor.pluginSplitter != nullptr) {
        return _processor.pluginSplitter->getGainStageLevelsInterface(chainNumber, slotNumber);
    }

    return std::optional<GainStageLevelsInterface>();
}

void PluginSelectionInterface::setGainStagePan(int chainNumber, int slotNumber, float pan) {
    if (_processor.pluginSplitter != nullptr) {
        _processor.pluginSplitter->setPan(chainNumber, slotNumber, pan);
    }
}

float PluginSelectionInterface::getGainStagePan(int chainNumber, int slotNumber) {
    if (_processor.pluginSplitter != nullptr) {
        return _processor.pluginSplitter->getPan(chainNumber, slotNumber);
    }

    return 0.0f;
}

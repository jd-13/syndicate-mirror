#include "Xml.hpp"
#include "XmlConsts.hpp"

namespace XmlReader {
    bool XmlElementIsPlugin(juce::XmlElement* element) {
        if (element->hasAttribute(XML_SLOT_TYPE_STR)) {
            if (element->getStringAttribute(XML_SLOT_TYPE_STR) == XML_SLOT_TYPE_PLUGIN_STR) {
                return true;
            }
        }

        return false;
    }

    bool XmlElementIsGainStage(juce::XmlElement* element) {
        if (element->hasAttribute(XML_SLOT_TYPE_STR)) {
            if (element->getStringAttribute(XML_SLOT_TYPE_STR) == XML_SLOT_TYPE_GAIN_STAGE_STR) {
                return true;
            }
        }

        return false;
    }

    std::unique_ptr<ChainSlotGainStage> restoreChainSlotGainStage(
            juce::XmlElement* element, const juce::AudioProcessor::BusesLayout& busesLayout) {

        bool isSlotBypassed {false};
        if (element->hasAttribute(XML_SLOT_IS_BYPASSED_STR)) {
            isSlotBypassed = element->getBoolAttribute(XML_SLOT_IS_BYPASSED_STR);
        } else {
            juce::Logger::writeToLog("Missing attribute " + juce::String(XML_SLOT_IS_BYPASSED_STR));
        }

        float gain {1.0f};
        if (element->hasAttribute(XML_GAIN_STAGE_GAIN_STR)) {
            gain = element->getDoubleAttribute(XML_GAIN_STAGE_GAIN_STR);
        } else {
            juce::Logger::writeToLog("Missing attribute " + juce::String(XML_GAIN_STAGE_GAIN_STR));
        }

        float pan {0.0f};
        if (element->hasAttribute(XML_GAIN_STAGE_PAN_STR)) {
            pan = element->getDoubleAttribute(XML_GAIN_STAGE_PAN_STR);
        } else {
            juce::Logger::writeToLog("Missing attribute " + juce::String(XML_GAIN_STAGE_PAN_STR));
        }

        return std::make_unique<ChainSlotGainStage>(gain, pan, isSlotBypassed, busesLayout);
    }

    std::unique_ptr<ChainSlotPlugin> restoreChainSlotPlugin(
            juce::XmlElement* element,
            std::function<float(int, MODULATION_TYPE)> getModulationValueCallback,
            HostConfiguration configuration,
            const PluginConfigurator& pluginConfigurator,
            LoadPluginFunction loadPlugin,
            std::function<void(juce::String)> onErrorCallback) {
        std::unique_ptr<ChainSlotPlugin> retVal;

        // Restore the plugin level bypass
        bool isPluginBypassed {false};
        if (element->hasAttribute(XML_SLOT_IS_BYPASSED_STR)) {
            isPluginBypassed = element->getBoolAttribute(XML_SLOT_IS_BYPASSED_STR);
        } else {
            juce::Logger::writeToLog("Missing attribute " + juce::String(XML_SLOT_IS_BYPASSED_STR));
        }

        // Load the actual plugin
        if (element->getNumChildElements() > 0) {
            juce::XmlElement* pluginDescriptionXml = element->getChildElement(0);
            juce::PluginDescription pluginDescription;

            if (pluginDescription.loadFromXml(*pluginDescriptionXml)) {
                auto [thisPlugin, errorMessage] = loadPlugin(pluginDescription, configuration);

                if (thisPlugin != nullptr) {
                    std::shared_ptr<juce::AudioPluginInstance> sharedPlugin = std::move(thisPlugin);

                    if (pluginConfigurator.configure(sharedPlugin, configuration)) {
                        retVal.reset(new ChainSlotPlugin(sharedPlugin, isPluginBypassed, getModulationValueCallback));

                        // Restore the editor bounds
                        if (element->hasAttribute(XML_PLUGIN_EDITOR_BOUNDS_STR)) {
                            juce::String boundsString = element->getStringAttribute(XML_PLUGIN_EDITOR_BOUNDS_STR);
                            retVal->editorBounds.reset(new PluginEditorBounds(juce::Rectangle<int>::fromString(boundsString)));
                        } else {
                            juce::Logger::writeToLog("Missing attribute " + juce::String(XML_PLUGIN_EDITOR_BOUNDS_STR));
                        }

                        // Restore the plugin's internal state
                        if (element->hasAttribute(XML_PLUGIN_DATA_STR)) {
                            const juce::String pluginDataString = element->getStringAttribute(XML_PLUGIN_DATA_STR);
                            juce::MemoryBlock pluginData;
                            pluginData.fromBase64Encoding(pluginDataString);

                            sharedPlugin->setStateInformation(pluginData.getData(), pluginData.getSize());

                            // Now that the plugin is restored, we can restore the modulation config
                            juce::XmlElement* modulationConfigElement = element->getChildByName(XML_MODULATION_CONFIG_STR);
                            if (modulationConfigElement != nullptr) {
                                retVal->modulationConfig = restorePluginModulationConfig(modulationConfigElement);
                            } else {
                                juce::Logger::writeToLog("Missing element " + juce::String(XML_MODULATION_CONFIG_STR));
                            }
                        } else {
                            juce::Logger::writeToLog("Missing attribute " + juce::String(XML_PLUGIN_DATA_STR));
                        }
                    } else {
                        juce::Logger::writeToLog("Failed to configure plugin: " + sharedPlugin->getPluginDescription().name);
                        onErrorCallback("Failed to restore " + sharedPlugin->getPluginDescription().name + " as it may be a mono only plugin being restored into a stereo instance of Syndicate or vice versa");
                    }
                } else {
                    juce::Logger::writeToLog("Failed to load plugin: " + errorMessage);
                    onErrorCallback("Failed to restore plugin: " + errorMessage);
                }
            } else {
                juce::Logger::writeToLog("Failed to parse plugin description");
            }
        } else {
            juce::Logger::writeToLog("Plugin element missing description");
        }

        return retVal;
    }

    std::unique_ptr<PluginModulationConfig> restorePluginModulationConfig(juce::XmlElement* element) {
        auto retVal = std::make_unique<PluginModulationConfig>();

        if (element->hasAttribute(XML_MODULATION_IS_ACTIVE_STR)) {
            retVal->isActive = element->getBoolAttribute(XML_MODULATION_IS_ACTIVE_STR);
        } else {
            juce::Logger::writeToLog("Missing attribute " + juce::String(XML_MODULATION_IS_ACTIVE_STR));
        }

        const int numParameterConfigs {element->getNumChildElements()};
        for (int index {0}; index < numParameterConfigs; index++) {
            juce::Logger::writeToLog("Restoring parameter modulation config " + juce::String(index));

            const juce::String parameterConfigElementName = getParameterModulationConfigXmlName(index);
            juce::XmlElement* thisParameterConfigElement = element->getChildByName(parameterConfigElementName);

            retVal->parameterConfigs.push_back(
                restorePluginParameterModulationConfig(thisParameterConfigElement));
        }

        return retVal;
    }

    std::unique_ptr<PluginParameterModulationConfig> restorePluginParameterModulationConfig(juce::XmlElement* element) {
        auto retVal = std::make_unique<PluginParameterModulationConfig>();

        if (element->hasAttribute(XML_MODULATION_TARGET_PARAMETER_NAME_STR)) {
            retVal->targetParameterName = element->getStringAttribute(XML_MODULATION_TARGET_PARAMETER_NAME_STR);
        } else {
            juce::Logger::writeToLog("Missing attribute " + juce::String(XML_MODULATION_TARGET_PARAMETER_NAME_STR));
        }

        if (element->hasAttribute(XML_MODULATION_REST_VALUE_STR)) {
            retVal->restValue = element->getDoubleAttribute(XML_MODULATION_REST_VALUE_STR);
        } else {
            juce::Logger::writeToLog("Missing attribute " + juce::String(XML_MODULATION_REST_VALUE_STR));
        }

        const int numSources {element->getNumChildElements()};
        for (int index {0}; index < numSources; index++) {
            juce::Logger::writeToLog("Restoring parameter modulation sources " + juce::String(index));

            const juce::String sourceElementName = getParameterModulationSourceXmlName(index);
            juce::XmlElement* thisSourceElement = element->getChildByName(sourceElementName);

            std::shared_ptr<PluginParameterModulationSource> thisSource = restorePluginParameterModulationSource(thisSourceElement);
            retVal->sources.push_back(thisSource);
        }

        return retVal;
    }

    std::unique_ptr<PluginParameterModulationSource> restorePluginParameterModulationSource(juce::XmlElement* element) {
        // TODO tidy definition construction
        ModulationSourceDefinition definition(0, MODULATION_TYPE::MACRO);
        definition.restoreFromXml(element);

        float modulationAmount {0};
        if (element->hasAttribute(XML_MODULATION_SOURCE_AMOUNT)) {
            modulationAmount = element->getDoubleAttribute(XML_MODULATION_SOURCE_AMOUNT);
        } else {
            juce::Logger::writeToLog("Missing attribute " + juce::String(XML_MODULATION_SOURCE_AMOUNT));
        }

        return std::make_unique<PluginParameterModulationSource>(definition, modulationAmount);
    }
}

namespace XmlWriter {
    void write(std::shared_ptr<ChainSlotGainStage> gainStage, juce::XmlElement* element) {
        element->setAttribute(XML_SLOT_TYPE_STR, XML_SLOT_TYPE_GAIN_STAGE_STR);

        element->setAttribute(XML_SLOT_IS_BYPASSED_STR, gainStage->isBypassed);
        element->setAttribute(XML_GAIN_STAGE_GAIN_STR, gainStage->gain);
        element->setAttribute(XML_GAIN_STAGE_PAN_STR, gainStage->pan);
    }

    void write(std::shared_ptr<ChainSlotPlugin> chainSlot, juce::XmlElement* element) {
        element->setAttribute(XML_SLOT_TYPE_STR, XML_SLOT_TYPE_PLUGIN_STR);

        // Store the plugin level bypass
        element->setAttribute(XML_SLOT_IS_BYPASSED_STR, chainSlot->isBypassed);

        // Store the plugin description
        std::unique_ptr<juce::XmlElement> pluginDescriptionXml =
            chainSlot->plugin->getPluginDescription().createXml();
        element->addChildElement(pluginDescriptionXml.release());

        // Store the plugin's internal state
        juce::MemoryBlock pluginMemoryBlock;
        chainSlot->plugin->getStateInformation(pluginMemoryBlock);
        element->setAttribute(XML_PLUGIN_DATA_STR, pluginMemoryBlock.toBase64Encoding());

        // Store the modulation config
        juce::XmlElement* modulationConfigElement = element->createNewChildElement(XML_MODULATION_CONFIG_STR);
        write(chainSlot->modulationConfig, modulationConfigElement);

        // Store the editor bounds
        if (chainSlot->editorBounds->has_value()) {
            element->setAttribute(XML_PLUGIN_EDITOR_BOUNDS_STR, chainSlot->editorBounds->value().toString());
        }
    }

    void write(std::shared_ptr<PluginModulationConfig> config, juce::XmlElement* element) {
        element->setAttribute(XML_MODULATION_IS_ACTIVE_STR, config->isActive);

        for (int index {0}; index < config->parameterConfigs.size(); index++) {
            juce::XmlElement* thisParameterConfigElement =
                element->createNewChildElement(getParameterModulationConfigXmlName(index));
            write(config->parameterConfigs[index], thisParameterConfigElement);
        }
    }

    void write(std::shared_ptr<PluginParameterModulationConfig> config, juce::XmlElement* element) {
        element->setAttribute(XML_MODULATION_TARGET_PARAMETER_NAME_STR, config->targetParameterName);
        element->setAttribute(XML_MODULATION_REST_VALUE_STR, config->restValue);

        for (int index {0}; index < config->sources.size(); index++ ) {
            juce::XmlElement* thisSourceElement =
                element->createNewChildElement(getParameterModulationSourceXmlName(index));
            write(config->sources[index], thisSourceElement);
        }
    }

    void write(std::shared_ptr<PluginParameterModulationSource> source, juce::XmlElement* element) {
        source->definition.writeToXml(element);
        element->setAttribute(XML_MODULATION_SOURCE_AMOUNT, source->modulationAmount);
    }
}

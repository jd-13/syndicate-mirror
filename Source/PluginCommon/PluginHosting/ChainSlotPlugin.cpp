#include "ChainSlotPlugin.h"

namespace {
    const char* XML_SLOT_IS_BYPASSED_STR {"isSlotBypassed"};
    const char* XML_PLUGIN_DATA_STR {"PluginData"};
    const char* XML_MODULATION_CONFIG_STR {"ModulationConfig"};
    const char* XML_MODULATION_IS_ACTIVE_STR {"ModulationIsActive"};
    const char* XML_MODULATION_TARGET_PARAMETER_NAME_STR {"TargetParameterName"};
    const char* XML_MODULATION_REST_VALUE_STR {"RestValue"};
    const char* XML_MODULATION_SOURCE {"Source"};
    const char* XML_MODULATION_SOURCE_AMOUNT {"SourceAmount"};

    std::string getParameterModulationConfigXmlName(int configNumber) {
        std::string retVal("ParamConfig_");
        retVal += std::to_string(configNumber);
        return retVal;
    }

    std::string getParameterModulationSourceXmlName(int sourceNumber) {
        std::string retVal("Source_");
        retVal += std::to_string(sourceNumber);
        return retVal;
    }
}

void PluginParameterModulationSource::restoreFromXml(juce::XmlElement* element) {
    definition.restoreFromXml(element);

    if (element->hasAttribute(XML_MODULATION_SOURCE_AMOUNT)) {
        modulationAmount = element->getDoubleAttribute(XML_MODULATION_SOURCE_AMOUNT);
    } else {
        juce::Logger::writeToLog("Missing attribute " + juce::String(XML_MODULATION_SOURCE_AMOUNT));
    }
}

void PluginParameterModulationSource::writeToXml(juce::XmlElement* element) {
    definition.writeToXml(element);
    element->setAttribute(XML_MODULATION_SOURCE_AMOUNT, modulationAmount);
}

void PluginParameterModulationConfig::restoreFromXml(juce::XmlElement* element) {
    if (element->hasAttribute(XML_MODULATION_TARGET_PARAMETER_NAME_STR)) {
        targetParameterName = element->getStringAttribute(XML_MODULATION_TARGET_PARAMETER_NAME_STR);
    } else {
        juce::Logger::writeToLog("Missing attribute " + juce::String(XML_MODULATION_TARGET_PARAMETER_NAME_STR));
    }

    if (element->hasAttribute(XML_MODULATION_REST_VALUE_STR)) {
        restValue = element->getDoubleAttribute(XML_MODULATION_REST_VALUE_STR);
    } else {
        juce::Logger::writeToLog("Missing attribute " + juce::String(XML_MODULATION_REST_VALUE_STR));
    }

    const int numSources {element->getNumChildElements()};
    for (int index {0}; index < numSources; index++) {
        juce::Logger::writeToLog("Restoring parameter modulation sources " + juce::String(index));

        const juce::String sourceElementName = getParameterModulationSourceXmlName(index);
        juce::XmlElement* thisSourceElement = element->getChildByName(sourceElementName);

        PluginParameterModulationSource thisSource;
        thisSource.restoreFromXml(thisSourceElement);
        sources.push_back(thisSource);
    }
}

void PluginParameterModulationConfig::writeToXml(juce::XmlElement* element) {
    element->setAttribute(XML_MODULATION_TARGET_PARAMETER_NAME_STR, targetParameterName);
    element->setAttribute(XML_MODULATION_REST_VALUE_STR, restValue);

    for (int index {0}; index < sources.size(); index++ ) {
        juce::XmlElement* thisSourceElement =
            element->createNewChildElement(getParameterModulationSourceXmlName(index));
        sources[index].writeToXml(thisSourceElement);
    }
}

void PluginModulationConfig::restoreFromXml(juce::XmlElement* element) {
    if (element->hasAttribute(XML_MODULATION_IS_ACTIVE_STR)) {
        isActive = element->getBoolAttribute(XML_MODULATION_IS_ACTIVE_STR);
    } else {
        juce::Logger::writeToLog("Missing attribute " + juce::String(XML_MODULATION_IS_ACTIVE_STR));
    }

    const int numParameterConfigs {element->getNumChildElements()};
    for (int index {0}; index < numParameterConfigs; index++) {
        juce::Logger::writeToLog("Restoring parameter modulation config " + juce::String(index));

        const juce::String parameterConfigElementName = getParameterModulationConfigXmlName(index);
        juce::XmlElement* thisParameterConfigElement = element->getChildByName(parameterConfigElementName);

        PluginParameterModulationConfig thisParameterConfig;
        thisParameterConfig.restoreFromXml(thisParameterConfigElement);
        parameterConfigs.push_back(thisParameterConfig);
    }
}

void PluginModulationConfig::writeToXml(juce::XmlElement* element) {
    element->setAttribute(XML_MODULATION_IS_ACTIVE_STR, isActive);

    for (int index {0}; index < parameterConfigs.size(); index++ ) {
        juce::XmlElement* thisParameterConfigElement =
            element->createNewChildElement(getParameterModulationConfigXmlName(index));
        parameterConfigs[index].writeToXml(thisParameterConfigElement);
    }
}

void ChainSlotPlugin::prepareToPlay(double sampleRate, int samplesPerBlock) {
    plugin->setRateAndBufferSizeDetails(sampleRate, samplesPerBlock);
    plugin->prepareToPlay(sampleRate, samplesPerBlock);
}

void ChainSlotPlugin::releaseResources() {
    plugin->releaseResources();
}

void ChainSlotPlugin::reset() {
    plugin->reset();
}

void ChainSlotPlugin::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    if (!isBypassed) {
        // Apply parameter modulation
        if (modulationConfig.isActive) {
            // TODO we need a faster way of retrieving parameters
            // Try again to get PluginParameterModulationConfig to hold a pointer to the parameter directly
            const juce::Array<juce::AudioProcessorParameter*>& parameters = plugin->getParameters();
            for (const PluginParameterModulationConfig& parameterConfig : modulationConfig.parameterConfigs) {
                for (juce::AudioProcessorParameter* targetParameter : parameters) {
                    if (targetParameter->getName(PluginParameterModulationConfig::PLUGIN_PARAMETER_NAME_LENGTH_LIMIT) == parameterConfig.targetParameterName) {
                        _applyModulationForParamter(targetParameter, parameterConfig);
                    }
                }
            }
        }

        // Do processing
        plugin->processBlock(buffer, midiMessages);
    }
}

std::unique_ptr<ChainSlotPlugin> ChainSlotPlugin::restoreFromXml(
        juce::XmlElement* element,
        std::function<float(int, MODULATION_TYPE)> getModulationValueCallback,
        HostConfiguration configuration,
        const PluginConfigurator& pluginConfigurator,
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
            juce::AudioPluginFormatManager formatManager;
            formatManager.addDefaultFormats();

            juce::String errorMessage;
            std::unique_ptr<juce::AudioPluginInstance> thisPlugin =
                formatManager.createPluginInstance(
                    pluginDescription, configuration.sampleRate, configuration.blockSize, errorMessage);

            if (thisPlugin != nullptr) {
                std::shared_ptr<juce::AudioPluginInstance> sharedPlugin = std::move(thisPlugin);

                if (pluginConfigurator.configure(sharedPlugin, configuration)) {
                    retVal.reset(new ChainSlotPlugin(sharedPlugin, isPluginBypassed, getModulationValueCallback));

                    // Restore the plugin's internal state
                    if (element->hasAttribute(XML_PLUGIN_DATA_STR)) {
                        const juce::String pluginDataString = element->getStringAttribute(XML_PLUGIN_DATA_STR);
                        juce::MemoryBlock pluginData;
                        pluginData.fromBase64Encoding(pluginDataString);

                        sharedPlugin->setStateInformation(pluginData.getData(), pluginData.getSize());

                        // Now that the plugin is restored, we can restore the modulation config
                        juce::XmlElement* modulationConfigElement = element->getChildByName(XML_MODULATION_CONFIG_STR);
                        retVal->modulationConfig.restoreFromXml(modulationConfigElement);
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

    return std::move(retVal);
}

void ChainSlotPlugin::writeToXml(juce::XmlElement* element) {
    element->setAttribute(XML_SLOT_TYPE_STR, XML_SLOT_TYPE_PLUGIN_STR);

    // Store the plugin level bypass
    element->setAttribute(XML_SLOT_IS_BYPASSED_STR, isBypassed);

    // Store the plugin description
    std::unique_ptr<juce::XmlElement> pluginDescriptionXml = plugin->getPluginDescription().createXml();
    element->addChildElement(pluginDescriptionXml.release());

    // Store the plugin's internal state
    juce::MemoryBlock pluginMemoryBlock;
    plugin->getStateInformation(pluginMemoryBlock);
    element->setAttribute(XML_PLUGIN_DATA_STR, pluginMemoryBlock.toBase64Encoding());

    // Store the modulation config
    juce::XmlElement* modulationConfigElement = element->createNewChildElement(XML_MODULATION_CONFIG_STR);
    modulationConfig.writeToXml(modulationConfigElement);
}

void ChainSlotPlugin::_applyModulationForParamter(juce::AudioProcessorParameter* targetParameter,
                                                  const PluginParameterModulationConfig& parameterConfig) {

    // Start from the rest value
    float paramValue {parameterConfig.restValue};

    for (const PluginParameterModulationSource& source : parameterConfig.sources) {
        // Add the modulation from each source
        const float valueForThisSource {
            _getModulationValueCallback(source.definition.id, source.definition.type) * source.modulationAmount
        };

        paramValue += valueForThisSource;
    }

    // Assign the value to the parameter
    targetParameter->setValue(paramValue);
}

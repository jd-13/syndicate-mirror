#include "XmlReader.hpp"
#include "XmlConsts.hpp"
#include "ChainSlotProcessors.hpp"
#include "ChainProcessors.hpp"
#include "SplitterMutators.hpp"
#include "SplitTypes.hpp"

namespace XmlReader {
    std::shared_ptr<PluginSplitter> restoreSplitterFromXml(
            juce::XmlElement* element,
            std::function<float(int, MODULATION_TYPE)> getModulationValueCallback,
            std::function<void(int)> latencyChangeCallback,
            HostConfiguration configuration,
            const PluginConfigurator& pluginConfigurator,
            std::function<void(juce::String)> onErrorCallback) {

        // Default to series
        SPLIT_TYPE splitType = SPLIT_TYPE::SERIES;

        if (element->hasAttribute(XML_SPLIT_TYPE_STR)) {
            const juce::String splitTypeString = element->getStringAttribute(XML_SPLIT_TYPE_STR);
            juce::Logger::writeToLog("Restoring split type: " + splitTypeString);

            // We need to check if the split type we're attempting to restore is supported.
            // For example in Logic when switching from a stereo to mono plugin we may have saved to XML
            // using a left/right split in a 2in2out configuration but will be restoring into a 1in1out
            // configuration.
            // In that case we move to a parallel split type.
            splitType = stringToSplitType(splitTypeString);
            const bool isExpecting2in2out {splitType == SPLIT_TYPE::LEFTRIGHT || splitType == SPLIT_TYPE::MIDSIDE};

            if (isExpecting2in2out && !canDoStereoSplitTypes(configuration.layout)) {
                // Migrate to parallel
                splitType = SPLIT_TYPE::PARALLEL;
            }
        } else {
            juce::Logger::writeToLog("Missing attribute " + juce::String(XML_SPLIT_TYPE_STR));
        }

        std::shared_ptr<PluginSplitter> splitter;
        if (splitType == SPLIT_TYPE::SERIES) {
            splitter = std::make_shared<PluginSplitterSeries>(configuration, getModulationValueCallback, latencyChangeCallback);
        } else if (splitType == SPLIT_TYPE::PARALLEL) {
            splitter = std::make_shared<PluginSplitterParallel>(configuration, getModulationValueCallback, latencyChangeCallback);
        } else if (splitType == SPLIT_TYPE::MULTIBAND) {
            splitter = std::make_shared<PluginSplitterMultiband>(configuration, getModulationValueCallback, latencyChangeCallback);
        } else if (splitType == SPLIT_TYPE::LEFTRIGHT) {
            splitter = std::make_shared<PluginSplitterLeftRight>(configuration, getModulationValueCallback, latencyChangeCallback);
        } else if (splitType == SPLIT_TYPE::MIDSIDE) {
            splitter = std::make_shared<PluginSplitterMidSide>(configuration, getModulationValueCallback, latencyChangeCallback);
        }

        // Reset state
        while (!splitter->chains.empty()) {
            splitter->chains.erase(splitter->chains.begin());
        }

        // Restore each chain
        juce::XmlElement* chainsElement = element->getChildByName(XML_CHAINS_STR);
        const int numChains {
            chainsElement == nullptr ? 0 : chainsElement->getNumChildElements()
        };

        for (int chainNumber {0}; chainNumber < numChains; chainNumber++) {
            juce::Logger::writeToLog("Restoring chain " + juce::String(chainNumber));

            const juce::String chainElementName = getChainXMLName(chainNumber);
            juce::XmlElement* thisChainElement = chainsElement->getChildByName(chainElementName);

            if (thisChainElement == nullptr) {
                juce::Logger::writeToLog("Failed to get element " + chainElementName);
            } else {
                bool isSoloed {false};
                if (thisChainElement->hasAttribute(XML_IS_CHAIN_SOLOED_STR)) {
                    isSoloed = thisChainElement->getBoolAttribute(XML_IS_CHAIN_SOLOED_STR);
                } else {
                    juce::Logger::writeToLog("Missing attribute " + juce::String(XML_IS_CHAIN_SOLOED_STR));
                }

                // Add the chain to the vector
                splitter->chains.emplace_back(std::make_shared<PluginChain>(getModulationValueCallback), false);
                PluginChainWrapper& thisChain = splitter->chains[splitter->chains.size() - 1];
                thisChain.chain = XmlReader::restoreChainFromXml(thisChainElement, configuration, pluginConfigurator, getModulationValueCallback, onErrorCallback);

                if (auto multibandSplitter = std::dynamic_pointer_cast<PluginSplitterMultiband>(splitter)) {
                    // Since we deleted all chains at the start to make sure we have a
                    // clean starting point, that can mean the first few crossover bands could still exist
                    // and be pointing at chains that have been deleted. We handle this here.
                    if (multibandSplitter->chains.size() > CrossoverMutators::getNumBands(multibandSplitter->crossover)) {
                        // Need to add a new band and chain
                        CrossoverMutators::addBand(multibandSplitter->crossover);
                    } else {
                        // We already have the bands in the crossover
                    }

                    // Now assign the chain to the band
                    CrossoverMutators::setPluginChain(multibandSplitter->crossover, multibandSplitter->chains.size() - 1, thisChain.chain);
                }

                ChainProcessors::prepareToPlay(*thisChain.chain.get(), configuration);

                thisChain.chain->latencyListener.setSplitter(splitter.get());
                SplitterMutators::setChainSolo(splitter, chainNumber, isSoloed);
            }
        }

        if (auto multibandSplitter = std::dynamic_pointer_cast<PluginSplitterMultiband>(splitter)) {
            // Restore the crossover frequencies
            juce::XmlElement* crossoversElement = element->getChildByName(XML_CROSSOVERS_STR);
            if (crossoversElement != nullptr) {
                for (int crossoverNumber {0}; crossoverNumber < multibandSplitter->chains.size() - 1; crossoverNumber++) {
                    const juce::String frequencyAttribute(getCrossoverXMLName(crossoverNumber));
                    if (crossoversElement->hasAttribute(frequencyAttribute)) {
                        SplitterMutators::setCrossoverFrequency(multibandSplitter, crossoverNumber, crossoversElement->getDoubleAttribute(frequencyAttribute));
                    } else {
                        juce::Logger::writeToLog("Missing attribute " + juce::String(frequencyAttribute));
                    }
                }
            } else {
                juce::Logger::writeToLog("Missing element " + juce::String(XML_CROSSOVERS_STR));
            }
        }

        splitter->onLatencyChange();

        return splitter;
    }

    std::unique_ptr<PluginChain> restoreChainFromXml(
            juce::XmlElement* element,
            HostConfiguration configuration,
            const PluginConfigurator& pluginConfigurator,
            std::function<float(int, MODULATION_TYPE)> getModulationValueCallback,
            std::function<void(juce::String)> onErrorCallback) {

        auto retVal = std::make_unique<PluginChain>(getModulationValueCallback);

        // Restore chain level bypass and mute
        if (element->hasAttribute(XML_IS_CHAIN_BYPASSED_STR)) {
            retVal->isChainBypassed = element->getBoolAttribute(XML_IS_CHAIN_BYPASSED_STR);
        } else {
            juce::Logger::writeToLog("Missing attribute " + juce::String(XML_IS_CHAIN_BYPASSED_STR));
        }

        if (element->hasAttribute(XML_IS_CHAIN_MUTED_STR)) {
            retVal->isChainMuted = element->getBoolAttribute(XML_IS_CHAIN_MUTED_STR);
        } else {
            juce::Logger::writeToLog("Missing attribute " + juce::String(XML_IS_CHAIN_MUTED_STR));
        }

        // Load each plugin
        juce::XmlElement* pluginsElement = element->getChildByName(XML_PLUGINS_STR);
        if (pluginsElement == nullptr) {
            juce::Logger::writeToLog("Missing child element " + juce::String(XML_PLUGINS_STR));
        }

        const int numPlugins {
            pluginsElement == nullptr ? 0 : pluginsElement->getNumChildElements()
        };

        for (int pluginNumber {0}; pluginNumber < numPlugins; pluginNumber++) {
            juce::Logger::writeToLog("Restoring slot " + juce::String(pluginNumber));

            const juce::String pluginElementName = getSlotXMLName(pluginNumber);
            juce::XmlElement* thisPluginElement = pluginsElement->getChildByName(pluginElementName);

            if (thisPluginElement == nullptr) {
                juce::Logger::writeToLog("Failed to get element " + pluginElementName);
                continue;
            }

            if (XmlReader::XmlElementIsPlugin(thisPluginElement)) {
                auto loadPlugin = [](const juce::PluginDescription& description, const HostConfiguration& config) {
                    juce::AudioPluginFormatManager formatManager;
                    formatManager.addDefaultFormats();

                    juce::String errorMessage;
                    std::unique_ptr<juce::AudioPluginInstance> thisPlugin =
                        formatManager.createPluginInstance(
                            description, config.sampleRate, config.blockSize, errorMessage);

                    return std::make_tuple<std::unique_ptr<juce::AudioPluginInstance>, juce::String>(
                        std::move(thisPlugin), juce::String(errorMessage));
                };

                auto newPlugin = XmlReader::restoreChainSlotPlugin(
                    thisPluginElement, getModulationValueCallback, configuration, pluginConfigurator, loadPlugin, onErrorCallback);

                if (newPlugin != nullptr) {
                    newPlugin->plugin->addListener(&retVal->latencyListener);
                    retVal->chain.push_back(std::move(newPlugin));
                }
            } else if (XmlReader::XmlElementIsGainStage(thisPluginElement)) {
                auto newGainStage = XmlReader::restoreChainSlotGainStage(thisPluginElement, configuration.layout);

                if (newGainStage != nullptr) {
                    // Call prepareToPlay since some hosts won't call it after restoring
                    ChainProcessors::prepareToPlay(*newGainStage.get(), configuration);
                    retVal->chain.push_back(std::move(newGainStage));
                }
            } else {
                juce::Logger::writeToLog("Can't determine slot type");
            }
        }

        retVal->latencyListener.onPluginChainUpdate();

        return retVal;
    }

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
                            const juce::String boundsString = element->getStringAttribute(XML_PLUGIN_EDITOR_BOUNDS_STR);

                            if (element->hasAttribute(XML_DISPLAY_AREA_STR)) {
                                const juce::String displayString = element->getStringAttribute(XML_DISPLAY_AREA_STR);

                                retVal->editorBounds.reset(new PluginEditorBounds());
                                *(retVal->editorBounds.get()) =  PluginEditorBoundsContainer(
                                    juce::Rectangle<int>::fromString(boundsString),
                                    juce::Rectangle<int>::fromString(displayString)
                                );
                            } else {
                                juce::Logger::writeToLog("Missing attribute " + juce::String(XML_DISPLAY_AREA_STR));
                            }

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

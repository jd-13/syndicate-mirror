#include "XmlWriter.hpp"
#include "XmlConsts.hpp"

#include "SplitterMutators.hpp"

namespace XmlWriter {
    void write(std::shared_ptr<PluginSplitter> splitter, juce::XmlElement* element) {
        juce::Logger::writeToLog("Storing splitter state");

        const char* splitTypeString = XML_SPLIT_TYPE_SERIES_STR;
        if (auto seriesSplitter = std::dynamic_pointer_cast<PluginSplitterSeries>(splitter)) {
            splitTypeString = XML_SPLIT_TYPE_SERIES_STR;
        } else if (auto parallelSplitter = std::dynamic_pointer_cast<PluginSplitterParallel>(splitter)) {
            splitTypeString = XML_SPLIT_TYPE_PARALLEL_STR;
        } else if (auto multibandSplitter = std::dynamic_pointer_cast<PluginSplitterMultiband>(splitter)) {
            splitTypeString = XML_SPLIT_TYPE_MULTIBAND_STR;
        } else if (auto leftRightSplitter = std::dynamic_pointer_cast<PluginSplitterLeftRight>(splitter)) {
            splitTypeString = XML_SPLIT_TYPE_LEFTRIGHT_STR;
        } else if (auto midSideSplitter = std::dynamic_pointer_cast<PluginSplitterMidSide>(splitter)) {
            splitTypeString = XML_SPLIT_TYPE_MIDSIDE_STR;
        }

        element->setAttribute(XML_SPLIT_TYPE_STR, splitTypeString);

        juce::XmlElement* chainsElement = element->createNewChildElement(XML_CHAINS_STR);

        for (int chainNumber {0}; chainNumber < splitter->chains.size(); chainNumber++) {
            juce::Logger::writeToLog("Storing chain " + juce::String(chainNumber));

            juce::XmlElement* thisChainElement = chainsElement->createNewChildElement(getChainXMLName(chainNumber));
            PluginChainWrapper& thisChain = splitter->chains[chainNumber];

            thisChainElement->setAttribute(XML_IS_CHAIN_SOLOED_STR, thisChain.isSoloed);
            XmlWriter::write(thisChain.chain, thisChainElement);
        }

        if (auto multibandSplitter = std::dynamic_pointer_cast<PluginSplitterMultiband>(splitter)) {
            // Store the crossover frequencies
            juce::XmlElement* crossoversElement = element->createNewChildElement(XML_CROSSOVERS_STR);
            for (int crossoverNumber {0}; crossoverNumber < multibandSplitter->chains.size() - 1; crossoverNumber++) {
                crossoversElement->setAttribute(getCrossoverXMLName(crossoverNumber), SplitterMutators::getCrossoverFrequency(multibandSplitter, crossoverNumber));
            }
        }
    }

    void write(std::shared_ptr<PluginChain> chain, juce::XmlElement* element) {
        // Store chain level bypass and mute
        element->setAttribute(XML_IS_CHAIN_BYPASSED_STR, chain->isChainBypassed);
        element->setAttribute(XML_IS_CHAIN_MUTED_STR, chain->isChainMuted);

        // Store each plugin
        juce::XmlElement* pluginsElement = element->createNewChildElement(XML_PLUGINS_STR);
        for (int pluginNumber {0}; pluginNumber < chain->chain.size(); pluginNumber++) {
            juce::Logger::writeToLog("Storing plugin " + juce::String(pluginNumber));

            juce::XmlElement* thisPluginElement = pluginsElement->createNewChildElement(getSlotXMLName(pluginNumber));

            if (auto gainStage = std::dynamic_pointer_cast<ChainSlotGainStage>(chain->chain[pluginNumber])) {
                XmlWriter::write(gainStage, thisPluginElement);
            } else if (auto pluginSlot = std::dynamic_pointer_cast<ChainSlotPlugin>(chain->chain[pluginNumber])) {
                XmlWriter::write(pluginSlot, thisPluginElement);
            }
        }
    }

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

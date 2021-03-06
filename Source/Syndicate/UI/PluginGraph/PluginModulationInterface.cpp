#include "PluginModulationInterface.h"
#include "GraphViewComponent.h"

namespace {
    juce::Array<juce::AudioProcessorParameter*> getParamsExcludingSelected(
            const juce::Array<juce::AudioProcessorParameter*>& pluginParameters,
            PluginModulationConfig config) {
        // Get the list of all parameters, and create a subset that includes only ones that haven't
        // been selected yet
        // TODO this might break if the plugin changes its list of parameters after this list has
        // been created
        juce::Array<juce::AudioProcessorParameter*> availableParameters;

        for (juce::AudioProcessorParameter* thisParam : pluginParameters) {
            bool shouldCopy {true};

            // If this parameter name is already in the config, don't add it to the list
            for (const PluginParameterModulationConfig& paramConfig : config.parameterConfigs) {
                const juce::String thisParamName = thisParam->getName(PluginParameterModulationConfig::PLUGIN_PARAMETER_NAME_LENGTH_LIMIT);
                if (thisParamName == paramConfig.targetParameterName) {
                    shouldCopy = false;
                    break;
                }
            }

            if (shouldCopy) {
                availableParameters.add(thisParam);
            }
        }

        return availableParameters;
    }
}

PluginModulationInterface::PluginModulationInterface(SyndicateAudioProcessor& processor, GraphViewComponent* graphView)
    : _processor(processor),
      _graphView(graphView) {
}

PluginModulationConfig PluginModulationInterface::getPluginModulationConfig(int chainNumber, int pluginNumber) {
    PluginModulationConfig retVal;

    if (_processor.pluginSplitter != nullptr) {
        retVal = _processor.pluginSplitter->getPluginModulationConfig(chainNumber, pluginNumber);
    }

    return retVal;
}

bool PluginModulationInterface::setPluginModulationConfig(PluginModulationConfig config, int chainNumber, int pluginNumber) {
    bool retVal {false};

    if (_processor.pluginSplitter != nullptr) {
        retVal = _processor.pluginSplitter->setPluginModulationConfig(config, chainNumber, pluginNumber);
    }

    return retVal;
}

bool PluginModulationInterface::togglePluginModulationActive(int chainNumber, int pluginNumber) {
    bool retVal {false};

    if (_processor.pluginSplitter != nullptr) {
        PluginModulationConfig config = _processor.pluginSplitter->getPluginModulationConfig(chainNumber, pluginNumber);
        config.isActive = !config.isActive;
        _processor.pluginSplitter->setPluginModulationConfig(config, chainNumber, pluginNumber);

        // We need the graph to redraw so that it puts all the plugins at the right height after
        // the modulation tray is expanded/collapsed
        if (_graphView != nullptr) {
            _graphView->onParameterUpdate();
        }
    }

    return retVal;
}

void PluginModulationInterface::selectModulationTarget(int chainNumber, int pluginNumber, int targetNumber) {
    if (_processor.pluginSplitter != nullptr) {
        // Collect the parameter list for this plugin
        std::shared_ptr<juce::AudioPluginInstance> plugin =
            _processor.pluginSplitter->getPlugin(chainNumber, pluginNumber);

        // Create the selector
        PluginParameterSelectorListParameters parameters {
            _processor.pluginParameterSelectorState,
            getParamsExcludingSelected(plugin->getParameters(), _processor.pluginSplitter->getPluginModulationConfig(chainNumber, pluginNumber)),
            [&, chainNumber, pluginNumber, targetNumber](juce::AudioProcessorParameter* parameter) { _onPluginParameterSelected(parameter, chainNumber, pluginNumber, targetNumber); }
        };

        _parameterSelectorWindow.reset(new PluginParameterSelectorWindow(
            [&]() { _parameterSelectorWindow.reset(); }, parameters, plugin->getPluginDescription().name));

        _parameterSelectorWindow->takeFocus();
    }
}

void PluginModulationInterface::removeModulationTarget(int chainNumber, int pluginNumber, int targetNumber) {
    if (_processor.pluginSplitter != nullptr) {
        PluginModulationConfig config = _processor.pluginSplitter->getPluginModulationConfig(chainNumber, pluginNumber);

        if (config.parameterConfigs.size() > targetNumber) {
            config.parameterConfigs.erase(config.parameterConfigs.begin() + targetNumber);
            _processor.pluginSplitter->setPluginModulationConfig(config, chainNumber, pluginNumber);

            // TODO maybe find a more efficient way to refresh this part of the display than redrawing
            // the whole graph?
            if (_graphView != nullptr) {
                _graphView->onParameterUpdate();
            }
        }
    }
}

juce::AudioProcessorParameter* PluginModulationInterface::getPluginParameterForTarget(int chainNumber, int pluginNumber, int targetNumber) {
    juce::AudioProcessorParameter* retVal {nullptr};

    if (_processor.pluginSplitter != nullptr) {
        PluginModulationConfig config = _processor.pluginSplitter->getPluginModulationConfig(chainNumber, pluginNumber);

        if (config.parameterConfigs.size() > targetNumber) {

            std::shared_ptr<juce::AudioPluginInstance> plugin = _processor.pluginSplitter->getPlugin(chainNumber, pluginNumber);

            if (plugin != nullptr) {
                const juce::String paramName(config.parameterConfigs[targetNumber].targetParameterName);

                const juce::Array<juce::AudioProcessorParameter*>& parameters = plugin->getParameters();
                for (juce::AudioProcessorParameter* thisParam : parameters) {
                    if (thisParam->getName(PluginParameterModulationConfig::PLUGIN_PARAMETER_NAME_LENGTH_LIMIT) == paramName) {
                        retVal = thisParam;
                        break;
                    }
                }
            }
        }
    }

    return retVal;
}

void PluginModulationInterface::_onPluginParameterSelected(juce::AudioProcessorParameter* parameter, int chainNumber, int pluginNumber, int targetNumber) {
    if (_processor.pluginSplitter != nullptr) {
        PluginModulationConfig config = _processor.pluginSplitter->getPluginModulationConfig(chainNumber, pluginNumber);

        // Increase the number of configs if needed
        while (config.parameterConfigs.size() <= targetNumber) {
            config.parameterConfigs.emplace_back();
        }

        config.parameterConfigs[targetNumber].targetParameterName = parameter->getName(PluginParameterModulationConfig::PLUGIN_PARAMETER_NAME_LENGTH_LIMIT);

        _processor.pluginSplitter->setPluginModulationConfig(config, chainNumber, pluginNumber);

        // TODO maybe find a more efficient way to refresh this part of the display than redrawing
        // the whole graph?
        if (_graphView != nullptr) {
            _graphView->onParameterUpdate();
        }
    }

    _parameterSelectorWindow.reset();
}

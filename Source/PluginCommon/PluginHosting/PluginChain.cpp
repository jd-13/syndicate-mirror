/*
  ==============================================================================

    PluginChain.cpp
    Created: 28 May 2021 9:17:35pm
    Author:  Jack Devlin

  ==============================================================================
*/

#include "PluginChain.h"

namespace {
    const char* XML_IS_CHAIN_BYPASSED_STR {"isChainBypassed"};
    const char* XML_IS_CHAIN_MUTED_STR {"isChainMuted"};
    const char* XML_PLUGINS_STR {"Plugins"};

    std::string getSlotXMLName(int pluginNumber) {
        std::string retVal("Slot_");
        retVal += std::to_string(pluginNumber);
        return retVal;
    }
}

PluginChain::PluginChain(std::function<float(int, MODULATION_TYPE)> getModulationValueCallback) :
        AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
                                        .withInput("Sidechain", juce::AudioChannelSet::stereo(), true)),
        _isChainBypassed(false),
        _isChainMuted(false),
        _getModulationValueCallback(getModulationValueCallback) {
    _latencyCompLine.reset(new juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::None>(0));
    _latencyCompLine->setDelay(0);
}

void PluginChain::insertPlugin(std::shared_ptr<juce::AudioPluginInstance> plugin, int position) {
    if (_chain.size() > position) {
        _chain.insert(_chain.begin() + position, std::make_unique<ChainSlotPlugin>(plugin, false, _getModulationValueCallback));
    } else {
        // If the position is bigger than the chain just add it to the end
        _chain.push_back(std::make_unique<ChainSlotPlugin>(plugin, false, _getModulationValueCallback));
    }

    plugin->addListener(this);
    _onLatencyChange();
}

void PluginChain::replacePlugin(std::shared_ptr<juce::AudioPluginInstance> plugin, int position) {
    if (_chain.size() > position) {
        plugin->setBusesLayout(getBusesLayout());

        // If it's a plugin remove the listener so we don't continue getting updates if it's kept
        // alive somewhere else
        const ChainSlotPlugin* oldPluginSlot = dynamic_cast<const ChainSlotPlugin*>(_chain[position].get());
        if (oldPluginSlot != nullptr) {
            oldPluginSlot->plugin->removeListener(this);
        }

        _chain[position] = std::make_unique<ChainSlotPlugin>(plugin, false, _getModulationValueCallback);
    } else {
        // If the position is bigger than the chain just add it to the end
        _chain.push_back(std::make_unique<ChainSlotPlugin>(plugin, false, _getModulationValueCallback));
    }

    plugin->addListener(this);
    _onLatencyChange();
}

bool PluginChain::removeSlot(int position) {
    bool success {false};

    if (_chain.size() > position) {

        // If it's a plugin remove the listener so we don't continue getting updates if it's kept
        // alive somewhere else
        const ChainSlotPlugin* oldPluginSlot = dynamic_cast<const ChainSlotPlugin*>(_chain[position].get());
        if (oldPluginSlot != nullptr) {
            oldPluginSlot->plugin->removeListener(this);
        }

        _chain.erase(_chain.begin() + position);
        _onLatencyChange();
        success = true;
    }

    return success;
}

void PluginChain::insertGainStage(int position, const juce::AudioProcessor::BusesLayout& busesLayout) {
    std::unique_ptr<ChainSlotGainStage> gainStage = std::make_unique<ChainSlotGainStage>(1, 0, false, busesLayout);
    gainStage->prepareToPlay(getSampleRate(), getBlockSize());

    if (_chain.size() > position) {
        _chain.insert(_chain.begin() + position, std::move(gainStage));
    } else {
        // If the position is bigger than the chain just add it to the end
        _chain.push_back(std::move(gainStage));
    }
}

std::shared_ptr<juce::AudioPluginInstance> PluginChain::getPlugin(int position) const {
    std::shared_ptr<juce::AudioPluginInstance> retVal;

    if (_chain.size() > position) {
        const ChainSlotPlugin* pluginSlot = dynamic_cast<const ChainSlotPlugin*>(_chain[position].get());

        if (pluginSlot != nullptr) {
            retVal = pluginSlot->plugin;
        }
    }

    return retVal;
}

bool PluginChain::setPluginModulationConfig(PluginModulationConfig config, int position) {
    bool retVal {false};

    if (_chain.size() > position) {
        ChainSlotPlugin* pluginSlot = dynamic_cast<ChainSlotPlugin*>(_chain[position].get());

        if (pluginSlot != nullptr) {
            pluginSlot->modulationConfig = config;
            retVal = true;
        }
    }

    return retVal;
}

PluginModulationConfig PluginChain::getPluginModulationConfig(int position) const {
    PluginModulationConfig retVal;

    if (_chain.size() > position) {
        const ChainSlotPlugin* pluginSlot = dynamic_cast<const ChainSlotPlugin*>(_chain[position].get());

        if (pluginSlot != nullptr) {
            retVal = pluginSlot->modulationConfig;
        }
    }

    return retVal;
}

void PluginChain::setSlotBypass(int position, bool isBypassed) {
    if (_chain.size() > position) {
        if (_chain[position]->isBypassed != isBypassed) {
            _chain[position]->isBypassed = isBypassed;

            // Trigger an update to the latency compensation
            _onLatencyChange();
        }
    }
}

bool PluginChain::getSlotBypass(int position) {
    if (_chain.size() > position) {
        return _chain[position]->isBypassed;
    }

    return false;
}

void PluginChain::setChainBypass(bool val) {
    _isChainBypassed = val;

    // Trigger an update to the latency compensation
    _onLatencyChange();
}

void PluginChain::setChainMute(bool val) {
    _isChainMuted = val;
}

bool PluginChain::setGainLinear(int position, float gain) {
    bool retVal {false};

    if (_chain.size() > position) {
        ChainSlotGainStage* gainStage = dynamic_cast<ChainSlotGainStage*>(_chain[position].get());

        if (gainStage != nullptr) {
            // TODO bounds check
            gainStage->gain = gain;
            retVal = true;
        }
    }

    return retVal;
}

float PluginChain::getGainLinear(int position) {
    float retVal {0.0f};

    if (_chain.size() > position) {
        ChainSlotGainStage* gainStage = dynamic_cast<ChainSlotGainStage*>(_chain[position].get());

        if (gainStage != nullptr) {
            retVal = gainStage->gain;
        }
    }

    return retVal;
}

std::optional<GainStageLevelsProvider> PluginChain::getGainStageLevelsProvider(int position) {
    std::optional<GainStageLevelsProvider> retVal;

    if (_chain.size() > position) {
        const ChainSlotGainStage* gainStage = dynamic_cast<ChainSlotGainStage*>(_chain[position].get());

        if (gainStage != nullptr) {
            retVal.emplace(*gainStage);
        }
    }

    return retVal;
}

bool PluginChain::setPan(int position, float pan) {
    bool retVal {false};

    if (_chain.size() > position) {
        ChainSlotGainStage* gainStage = dynamic_cast<ChainSlotGainStage*>(_chain[position].get());

        if (gainStage != nullptr) {
            // TODO bounds check
            gainStage->pan = pan;
            retVal = true;
        }
    }

    return retVal;
}

float PluginChain::getPan(int position) {
    float retVal {0.0f};

    if (_chain.size() > position) {
        ChainSlotGainStage* gainStage = dynamic_cast<ChainSlotGainStage*>(_chain[position].get());

        if (gainStage != nullptr) {
            retVal = gainStage->pan;
        }
    }

    return retVal;
}

void PluginChain::setRequiredLatency(int numSamples) {
    // The compensation is the amount of latency we need to add artificially to the latency of the
    // plugins in this chain in order to meet the required amount
    // If this is the slowest chain owned by the splitter this should be 0
    const int compensation {std::max(numSamples - getLatencySamples(), 0)};

    WECore::AudioSpinLock lock(_latencyCompLineMutex);
    _latencyCompLine.reset(new juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::None>(compensation));
    _latencyCompLine->prepare({getSampleRate(), static_cast<juce::uint32>(getBlockSize()), 4});
    _latencyCompLine->setDelay(compensation);
}

std::shared_ptr<PluginEditorBounds> PluginChain::getPluginEditorBounds(int position) const {
    std::shared_ptr<PluginEditorBounds> retVal(new PluginEditorBounds());

    if (_chain.size() > position) {
        const ChainSlotPlugin* pluginSlot = dynamic_cast<const ChainSlotPlugin*>(_chain[position].get());

        if (pluginSlot != nullptr) {
            retVal = pluginSlot->editorBounds;
        }
    }

    return retVal;
}

void PluginChain::restoreFromXml(juce::XmlElement* element,
                                 HostConfiguration configuration,
                                 const PluginConfigurator& pluginConfigurator,
                                 std::function<void(juce::String)> onErrorCallback) {
    // Restore chain level bypass and mute
    if (element->hasAttribute(XML_IS_CHAIN_BYPASSED_STR)) {
        _isChainBypassed = element->getBoolAttribute(XML_IS_CHAIN_BYPASSED_STR);
    } else {
        juce::Logger::writeToLog("Missing attribute " + juce::String(XML_IS_CHAIN_BYPASSED_STR));
    }

    if (element->hasAttribute(XML_IS_CHAIN_MUTED_STR)) {
        _isChainMuted = element->getBoolAttribute(XML_IS_CHAIN_MUTED_STR);
    } else {
        juce::Logger::writeToLog("Missing attribute " + juce::String(XML_IS_CHAIN_MUTED_STR));
    }

    // Load each plugin
    juce::XmlElement* pluginsElement = element->getChildByName(XML_PLUGINS_STR);
    const int numPlugins {pluginsElement->getNumChildElements()};

    for (int pluginNumber {0}; pluginNumber < numPlugins; pluginNumber++) {
        juce::Logger::writeToLog("Restoring slot " + juce::String(pluginNumber));

        const juce::String pluginElementName = getSlotXMLName(pluginNumber);
        juce::XmlElement* thisPluginElement = pluginsElement->getChildByName(pluginElementName);

        if (thisPluginElement == nullptr) {
            juce::Logger::writeToLog("Failed to get element " + pluginElementName);
            continue;
        }

        if (ChainSlotBase::XmlElementIsPlugin(thisPluginElement)) {
            std::unique_ptr<ChainSlotPlugin> newPlugin = ChainSlotPlugin::restoreFromXml(thisPluginElement, _getModulationValueCallback, configuration, pluginConfigurator, onErrorCallback);

            if (newPlugin != nullptr) {
                newPlugin->plugin->addListener(this);
                _chain.push_back(std::move(newPlugin));
            }
        } else if (ChainSlotBase::XmlElementIsGainStage(thisPluginElement)) {
            std::unique_ptr<ChainSlotGainStage> newGainStage = ChainSlotGainStage::restoreFromXml(thisPluginElement, configuration.layout);

            if (newGainStage != nullptr) {
                // Call prepareToPlay since some hosts won't call it after restoring
                newGainStage->prepareToPlay(configuration.sampleRate, configuration.blockSize);
                _chain.push_back(std::move(newGainStage));
            }
        } else {
            juce::Logger::writeToLog("Can't determine slot type");
        }
    }

    _onLatencyChange();
}

void PluginChain::writeToXml(juce::XmlElement* element) {
    // Store chain level bypass and mute
    element->setAttribute(XML_IS_CHAIN_BYPASSED_STR, _isChainBypassed);
    element->setAttribute(XML_IS_CHAIN_MUTED_STR, _isChainMuted);

    // Store each plugin
    juce::XmlElement* pluginsElement = element->createNewChildElement(XML_PLUGINS_STR);
    for (int pluginNumber {0}; pluginNumber < _chain.size(); pluginNumber++) {
        juce::Logger::writeToLog("Storing plugin " + juce::String(pluginNumber));

        juce::XmlElement* thisPluginElement = pluginsElement->createNewChildElement(getSlotXMLName(pluginNumber));
        _chain[pluginNumber]->writeToXml(thisPluginElement);
    }
}

// AudioProcessor methods

const juce::String PluginChain::getName() const {
    return "Chain";
}
void PluginChain::prepareToPlay(double sampleRate, int samplesPerBlock) {
    setRateAndBufferSizeDetails(sampleRate, samplesPerBlock);

    _latencyCompLine->prepare({sampleRate, static_cast<juce::uint32>(samplesPerBlock), 4});

    for (std::unique_ptr<ChainSlotBase>& slot : _chain) {
        slot->prepareToPlay(sampleRate, samplesPerBlock);
    }
}

void PluginChain::releaseResources() {
    for (std::unique_ptr<ChainSlotBase>& slot : _chain) {
        slot->releaseResources();
    }
}

void PluginChain::reset() {
    for (std::unique_ptr<ChainSlotBase>& slot : _chain) {
        slot->reset();
    }
}

void PluginChain::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    // Add the latency compensation
    juce::dsp::AudioBlock<float> bufferBlock(buffer);
    juce::dsp::ProcessContextReplacing<float> context(bufferBlock);

    {
        WECore::AudioSpinTryLock lock(_latencyCompLineMutex);
        if (lock.isLocked()) {
            _latencyCompLine->process(context);
        }
    }

    if (_isChainBypassed) {
        // Bypassed - do nothing
    } else if (_isChainMuted) {
        // Muted - return empty buffers
        juce::FloatVectorOperations::fill(buffer.getWritePointer(0), 0, buffer.getNumSamples());
        juce::FloatVectorOperations::fill(buffer.getWritePointer(1), 0, buffer.getNumSamples());
    } else {
        // Chain is active - process as normal
        for (std::unique_ptr<ChainSlotBase>& slot : _chain) {
            slot->processBlock(buffer, midiMessages);
        }
    }
}

double PluginChain::getTailLengthSeconds() const {
    // TODO
    return 0;
}

bool PluginChain::acceptsMidi() const {
    return false;
}

bool PluginChain::producesMidi() const {
    return false;
}

juce::AudioProcessorEditor* PluginChain::createEditor() {
    // TODO
    return nullptr;
}

bool PluginChain::hasEditor() const {
    // TODO
    return false;
}

int PluginChain::getNumPrograms() {
    // TODO
    return 0;
}

int PluginChain::getCurrentProgram() {
    // TODO
    return 0;
}

void PluginChain::setCurrentProgram (int index) {
    // TODO
}

const juce::String PluginChain::getProgramName (int index) {
    // TODO
    return "";
}

void PluginChain::changeProgramName(int index, const juce::String& newName) {
    // TODO
}

void PluginChain::getStateInformation (juce::MemoryBlock& destData) {
    // TODO
}

void PluginChain::setStateInformation (const void* data, int sizeInBytes) {
    // TODO
}

void PluginChain::_onLatencyChange() {
    // Iterate through each plugin and total the reported latency
    int totalLatency {0};

    // If the chain is bypassed the reported latency should be 0
    if (!_isChainBypassed) {
        for (int index {0}; index < _chain.size(); index++) {
            const ChainSlotPlugin* pluginSlot = dynamic_cast<const ChainSlotPlugin*>(_chain[index].get());

            // If this slot is a plugin and it's not bypassed, add it to the total
            if (pluginSlot != nullptr && !pluginSlot->isBypassed) {
                totalLatency += pluginSlot->plugin->getLatencySamples();
            }
        }
    }

    // Calling this will notify the splitter, which then calls setRequiredLatency for the chain to
    // apply the new latency compensation
    setLatencySamples(totalLatency);
}

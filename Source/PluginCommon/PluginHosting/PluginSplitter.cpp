/*
  ==============================================================================

    PluginSplitter.cpp
    Created: 29 May 2021 2:35:45pm
    Author:  Jack Devlin

  ==============================================================================
*/

#include "PluginSplitter.h"

PluginSplitter::PluginSplitter(int defaultNumChains,
                               std::function<float(int, MODULATION_TYPE)> getModulationValueCallback) :
        AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
                                        .withInput("Sidechain", juce::AudioChannelSet::stereo(), true)),
        _numChainsSoloed(0),
        _getModulationValueCallback(getModulationValueCallback) {
    // Set up the default number of chains
    for (int idx {0}; idx < defaultNumChains; idx++) {
        _chains.emplace_back(std::make_unique<PluginChain>(_getModulationValueCallback), false);
        _chains[_chains.size() - 1].chain->addListener(this);
    }
    _onLatencyChange();
}

PluginSplitter::PluginSplitter(std::vector<PluginChainWrapper>& chains,
                               int defaultNumChains,
                               std::function<float(int, MODULATION_TYPE)> getModulationValueCallback) :
        AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
                                        .withInput("Sidechain", juce::AudioChannelSet::stereo(), true)),
        _numChainsSoloed(0),
        _getModulationValueCallback(getModulationValueCallback) {

    // Carry all the chains over from the previous splitter
    for (size_t index {0}; index < chains.size(); index++) {
        // Update the number of chains soloed
        if (chains[index].isSoloed) {
            _numChainsSoloed++;
        }

        // Take ownership of this chain
        chains[index].chain->addListener(this);
        _chains.push_back(std::move(chains[index]));
    }

    // Add chains if we still need to reach the default
    while (defaultNumChains > _chains.size()) {
        _chains.emplace_back(std::make_unique<PluginChain>(_getModulationValueCallback), false);
        _chains[_chains.size() - 1].chain->addListener(this);
    }
    _onLatencyChange();
}

bool PluginSplitter::insertPlugin(std::shared_ptr<juce::AudioPluginInstance> plugin,
                                  int chainNumber,
                                  int positionInChain) {

    bool success {false};

    if (_chains.size() > chainNumber) {
        _chains[chainNumber].chain->insertPlugin(plugin, positionInChain);
        success = true;
    }

    return success;
}

bool PluginSplitter::replacePlugin(std::shared_ptr<juce::AudioPluginInstance> plugin,
                                   int chainNumber,
                                   int positionInChain) {

    bool success {false};

    if (_chains.size() > chainNumber) {
        _chains[chainNumber].chain->replacePlugin(plugin, positionInChain);
        success = true;
    }

    return success;
}

bool PluginSplitter::removeSlot(int chainNumber, int positionInChain) {
    bool success {false};

    if (_chains.size() > chainNumber) {
        success = _chains[chainNumber].chain->removeSlot(positionInChain);
    }

    return success;
}

bool PluginSplitter::insertGainStage(int chainNumber, int positionInChain, const juce::AudioProcessor::BusesLayout& busesLayout) {
    bool success {false};

    if (_chains.size() > chainNumber) {
        _chains[chainNumber].chain->insertGainStage(positionInChain, busesLayout);
        success = true;
    }

    return success;
}

std::shared_ptr<juce::AudioPluginInstance> PluginSplitter::getPlugin(int chainNumber, int positionInChain) {

    std::shared_ptr<juce::AudioPluginInstance> retVal;

    if (_chains.size() > chainNumber) {
        retVal = _chains[chainNumber].chain->getPlugin(positionInChain);
    }

    return retVal;
}

bool PluginSplitter::setPluginModulationConfig(PluginModulationConfig config, int chainNumber, int positionInChain) {
    bool retVal {false};

    if (chainNumber < _chains.size()) {
        retVal = _chains[chainNumber].chain->setPluginModulationConfig(config, positionInChain);
    }

    return retVal;
}

PluginModulationConfig PluginSplitter::getPluginModulationConfig(int chainNumber, int positionInChain) const {
    PluginModulationConfig retVal;

    if (_chains.size() > chainNumber) {
        retVal = _chains[chainNumber].chain->getPluginModulationConfig(positionInChain);
    }

    return retVal;
}

std::vector<PluginChainWrapper>& PluginSplitter::releaseChains() {
    for (PluginChainWrapper& chain : _chains) {
        chain.chain->removeListener(this);
    }

    return _chains;
}

void PluginSplitter::setSlotBypass(int chainNumber, int positionInChain, bool isBypassed) {
    if (_chains.size() > chainNumber) {
        _chains[chainNumber].chain->setSlotBypass(positionInChain, isBypassed);
    }
}

bool PluginSplitter::getSlotBypass(int chainNumber, int positionInChain) {
    if (_chains.size() > chainNumber) {
        return _chains[chainNumber].chain->getSlotBypass(positionInChain);
    }

    return false;
}

void PluginSplitter::setChainSolo(int chainNumber, bool val) {
    if (chainNumber < _chains.size()) {

        // If the new value is different to the existing one, update it and the counter
        if (val != _chains[chainNumber].isSoloed) {

            _chains[chainNumber].isSoloed = val;

            if (val) {
                _numChainsSoloed++;
            } else {
                _numChainsSoloed--;
            }
        }
    }
}

bool PluginSplitter::getChainSolo(int chainNumber) {
    bool retVal {false};

    if (chainNumber < _chains.size()) {
        retVal = _chains[chainNumber].isSoloed;
    }

    return retVal;
}

bool PluginSplitter::setGainLinear(int chainNumber, int positionInChain, float gain) {
    bool retVal {false};

    if (chainNumber < _chains.size()) {
        retVal = _chains[chainNumber].chain->setGainLinear(positionInChain, gain);
    }

    return retVal;
}

float PluginSplitter::getGainLinear(int chainNumber, int positionInChain) {
    float retVal {0.0f};

    if (chainNumber < _chains.size()) {
        retVal = _chains[chainNumber].chain->getGainLinear(positionInChain);
    }

    return retVal;
}

std::optional<GainStageLevelsProvider> PluginSplitter::getGainStageLevelsProvider(int chainNumber, int positionInChain) {
    if (chainNumber < _chains.size()) {
        return _chains[chainNumber].chain->getGainStageLevelsProvider(positionInChain);
    }

    return std::optional<GainStageLevelsProvider>();
}

bool PluginSplitter::setPan(int chainNumber, int positionInChain, float pan) {
    bool retVal {false};

    if (chainNumber < _chains.size()) {
        retVal = _chains[chainNumber].chain->setPan(positionInChain, pan);
    }

    return retVal;
}

float PluginSplitter::getPan(int chainNumber, int positionInChain) {
    float retVal {0.0f};

    if (chainNumber < _chains.size()) {
        retVal = _chains[chainNumber].chain->getPan(positionInChain);
    }

    return retVal;
}

std::shared_ptr<PluginEditorBounds> PluginSplitter::getPluginEditorBounds(int chainNumber, int positionInChain) const {
    std::shared_ptr<PluginEditorBounds> retVal(new PluginEditorBounds());

    if (chainNumber < _chains.size()) {
        retVal = _chains[chainNumber].chain->getPluginEditorBounds(positionInChain);
    }

    return retVal;
}

void PluginSplitter::restoreFromXml(juce::XmlElement* element,
                                    HostConfiguration configuration,
                                    const PluginConfigurator& pluginConfigurator,
                                    std::function<void(juce::String)> onErrorCallback) {
    // Reset state
    _numChainsSoloed = 0;
    while (!_chains.empty()) {
        _chains.erase(_chains.begin());
    }

    // Restore each chain
    juce::XmlElement* chainsElement = element->getChildByName(XML_CHAINS_STR);
    const int numChains {chainsElement->getNumChildElements()};

    for (int chainNumber {0}; chainNumber < numChains; chainNumber++) {
        juce::Logger::writeToLog("Restoring chain " + juce::String(chainNumber));

        const juce::String chainElementName = _getChainXMLName(chainNumber);
        juce::XmlElement* thisChainElement = chainsElement->getChildByName(chainElementName);

        if (thisChainElement == nullptr) {
            juce::Logger::writeToLog("Failed to get element " + chainElementName);
        } else {
            bool isSoloed {false};
            if (thisChainElement->hasAttribute(XML_ISSOLOED_STR)) {
                isSoloed = thisChainElement->getBoolAttribute(XML_ISSOLOED_STR);
            } else {
                juce::Logger::writeToLog("Missing attribute " + juce::String(XML_ISSOLOED_STR));
            }

            // Add the chain to the vector
            _chains.emplace_back(std::make_unique<PluginChain>(_getModulationValueCallback), false);
            PluginChainWrapper& thisChain = _chains[_chains.size() - 1];
            thisChain.chain->prepareToPlay(configuration.sampleRate, configuration.blockSize);
            thisChain.chain->addListener(this);
            thisChain.isSoloed = isSoloed;
            thisChain.chain->restoreFromXml(thisChainElement, configuration, pluginConfigurator, onErrorCallback);
        }
    }

    _onLatencyChange();
}

void PluginSplitter::writeToXml(juce::XmlElement* element) {
    juce::Logger::writeToLog("Storing splitter state");

    juce::XmlElement* chainsElement = element->createNewChildElement(XML_CHAINS_STR);

    for (int chainNumber {0}; chainNumber < _chains.size(); chainNumber++) {
        juce::Logger::writeToLog("Storing chain " + juce::String(chainNumber));

        juce::XmlElement* thisChainElement = chainsElement->createNewChildElement(_getChainXMLName(chainNumber));
        PluginChainWrapper& thisChain = _chains[chainNumber];

        thisChainElement->setAttribute(XML_ISSOLOED_STR, thisChain.isSoloed);
        thisChain.chain->writeToXml(thisChainElement);
    }
}

// AudioProcessor methods

const juce::String PluginSplitter::getName() const {
    return "Splitter";
}
void PluginSplitter::prepareToPlay(double sampleRate, int samplesPerBlock) {
    setRateAndBufferSizeDetails(sampleRate, samplesPerBlock);

    for (PluginChainWrapper& chainWrapper : _chains) {
        chainWrapper.chain->prepareToPlay(sampleRate, samplesPerBlock);
    }
}

void PluginSplitter::releaseResources() {
    for (PluginChainWrapper& chainWrapper : _chains) {
        chainWrapper.chain->releaseResources();
    }
}

void PluginSplitter::reset() {
    for (PluginChainWrapper& chainWrapper : _chains) {
        chainWrapper.chain->reset();
    }
}

double PluginSplitter::getTailLengthSeconds() const {
    // TODO
    return 0;
}

bool PluginSplitter::acceptsMidi() const {
    return false;
}

bool PluginSplitter::producesMidi() const {
    return false;
}

juce::AudioProcessorEditor* PluginSplitter::createEditor() {
    // TODO
    return nullptr;
}

bool PluginSplitter::hasEditor() const {
    // TODO
    return false;
}

int PluginSplitter::getNumPrograms() {
    // TODO
    return 0;
}

int PluginSplitter::getCurrentProgram() {
    // TODO
    return 0;
}

void PluginSplitter::setCurrentProgram(int index) {
    // TODO
}

const juce::String PluginSplitter::getProgramName(int index) {
    // TODO
    return "";
}

void PluginSplitter::changeProgramName(int index, const juce::String& newName) {
    // TODO
}

void PluginSplitter::getStateInformation(juce::MemoryBlock& destData) {
    // TODO
}

void PluginSplitter::setStateInformation(const void* data, int sizeInBytes) {
    // TODO
}

void PluginSplitter::_copyBuffer(juce::AudioBuffer<float>& source, juce::AudioBuffer<float>& destination) {
    if (source.getNumSamples() == destination.getNumSamples()) {

        int channelsToCopy {std::min(source.getNumChannels(), destination.getNumChannels())};

        // For each channel, copy each sample
        for (int channelIndex {0}; channelIndex < channelsToCopy; channelIndex++) {
            const float* readPointer = source.getReadPointer(channelIndex);
            float* writePointer = destination.getWritePointer(channelIndex);

            juce::FloatVectorOperations::copy(writePointer, readPointer, source.getNumSamples());
        }
    }
}

void PluginSplitter::_addBuffers(juce::AudioBuffer<float>& source, juce::AudioBuffer<float>& destination) {
    if (source.getNumSamples() == destination.getNumSamples()) {

        int channelsToCopy {std::min(source.getNumChannels(), destination.getNumChannels())};

        // For each channel, add each sample
        for (int channelIndex {0}; channelIndex < channelsToCopy; channelIndex++) {
            const float* readPointer = source.getReadPointer(channelIndex);
            float* writePointer = destination.getWritePointer(channelIndex);

            juce::FloatVectorOperations::add(writePointer, readPointer, source.getNumSamples());
        }
    }
}

void PluginSplitter::_onLatencyChange() {
    // The latency of the splitter is the latency of the slowest chain, so iterate through each
    // chain and report the highest latency
    int highestLatency {0};

    for (const PluginChainWrapper& chain : _chains) {
        const int thisLatency {chain.chain->getLatencySamples()};
        if (highestLatency < thisLatency) {
            highestLatency = thisLatency;
        }
    }

    setLatencySamples(highestLatency);

    // Tell each chain the latency of the slowest chain, so they can all add compensation to match
    // it
    for (PluginChainWrapper& chain : _chains) {
        chain.chain->setRequiredLatency(highestLatency);
    }
}

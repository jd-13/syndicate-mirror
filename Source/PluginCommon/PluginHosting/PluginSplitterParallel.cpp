#include "PluginSplitterParallel.h"
#include "MONSTRFilters/MONSTRParameters.h"

PluginSplitterParallel::PluginSplitterParallel(std::function<float(int, MODULATION_TYPE)> getModulationValueCallback)
        : PluginSplitter(DEFAULT_NUM_CHAINS, getModulationValueCallback) {
    juce::Logger::writeToLog("Constructed PluginSplitterParallel");
}

PluginSplitterParallel::PluginSplitterParallel(std::vector<PluginChainWrapper>& chains, std::function<float(int, MODULATION_TYPE)> getModulationValueCallback)
        : PluginSplitter(chains, DEFAULT_NUM_CHAINS, getModulationValueCallback) {
}

bool PluginSplitterParallel::addChain() {
    bool success {false};

    // Number of parallel chains is limited to the same as the crossover to avoid weird edge cases
    // when switching bands
    if (_chains.size() < WECore::MONSTR::Parameters::NUM_BANDS.maxValue) {
        _chains.emplace_back(std::make_unique<PluginChain>(_getModulationValueCallback), false);
        _chains[_chains.size() - 1].chain->prepareToPlay(getSampleRate(), getBlockSize());
        _chains[_chains.size() - 1].chain->addListener(this);

        _onLatencyChange();
        success = true;
    }

    return success;
}

bool PluginSplitterParallel::removeChain(int chainNumber) {
    bool success {false};

    if (_chains.size() > 1 && chainNumber < _chains.size()) {
        _chains[chainNumber].chain->removeListener(this);
        _chains.erase(_chains.begin() + chainNumber);

        _onLatencyChange();
        success = true;
    }

    return success;
}

void PluginSplitterParallel::prepareToPlay(double sampleRate, int samplesPerBlock) {
    _inputBuffer.reset(new juce::AudioBuffer<float>(4, samplesPerBlock)); // stereo main + stereo sidechain
    _outputBuffer.reset(new juce::AudioBuffer<float>(2, samplesPerBlock)); // stereo main

    PluginSplitter::prepareToPlay(sampleRate, samplesPerBlock);
}

void PluginSplitterParallel::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    _outputBuffer->clear();

    for (PluginChainWrapper& chain : _chains) {

        // Only process if no bands are soloed or this one is soloed
        if (_numChainsSoloed == 0 || chain.isSoloed) {
            // Make a copy of the input buffer for us to process, preserving the original for the other
            // chains
            // TODO: do the same for midi
            _copyBuffer(buffer, *(_inputBuffer.get()));

            // Process the newly copied buffer
            chain.chain->processBlock(*(_inputBuffer.get()), midiMessages);

            // Add the output of this chain to the output buffer
            _addBuffers(*(_inputBuffer.get()), *(_outputBuffer.get()));
        }
    }

    // Overwrite the original buffer with our own output
    _copyBuffer(*(_outputBuffer.get()), buffer);
}

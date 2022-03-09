#include "PluginSplitterLeftRight.h"

PluginSplitterLeftRight::PluginSplitterLeftRight(std::function<float(int, MODULATION_TYPE)> getModulationValueCallback)
        : PluginSplitter(DEFAULT_NUM_CHAINS, getModulationValueCallback) {
    juce::Logger::writeToLog("Constructed PluginSplitterLeftRight");
}

PluginSplitterLeftRight::PluginSplitterLeftRight(std::vector<PluginChainWrapper>& chains, std::function<float(int, MODULATION_TYPE)> getModulationValueCallback)
        : PluginSplitter(chains, DEFAULT_NUM_CHAINS, getModulationValueCallback) {
}

void PluginSplitterLeftRight::prepareToPlay(double sampleRate, int samplesPerBlock) {
    _leftBuffer.reset(new juce::AudioBuffer<float>(4, samplesPerBlock)); // stereo main + stereo sidechain
    _rightBuffer.reset(new juce::AudioBuffer<float>(4, samplesPerBlock)); // stereo main + stereo sidechain

    PluginSplitter::prepareToPlay(sampleRate, samplesPerBlock);
}

void PluginSplitterLeftRight::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    // TODO: maybe this should be done using mono buffers? (depends if plugins can handle it reliably)

    // Make sure to clear the buffers each time, as on a previous call the plugins may have left
    // data in the unused channel of each buffer, and since it's not used it won't get implicitly
    // overwritten (but it'll still be copied to the output)
    _leftBuffer->clear();
    _rightBuffer->clear();

    // Copy the left and right channels to separate buffers
    if (_numChainsSoloed == 0 || _chains[0].isSoloed) {
        const float* leftRead {buffer.getReadPointer(0)};
        float* leftWrite {_leftBuffer->getWritePointer(0)};
        juce::FloatVectorOperations::copy(leftWrite, leftRead, buffer.getNumSamples());
    }

    if (_numChainsSoloed == 0 || _chains[1].isSoloed) {
        const float* rightRead {buffer.getReadPointer(1)};
        float* rightWrite {_rightBuffer->getWritePointer(1)};
        juce::FloatVectorOperations::copy(rightWrite, rightRead, buffer.getNumSamples());
    }

    // Now the input has been copied we can clear the original
    buffer.clear();

    // Process the left chain
    if (_numChainsSoloed == 0 || _chains[0].isSoloed) {
        _chains[0].chain->processBlock(*(_leftBuffer.get()), midiMessages);
        _addBuffers(*(_leftBuffer.get()), buffer);
    }

    // Process the right chain
    if (_numChainsSoloed == 0 || _chains[1].isSoloed) {
        _chains[1].chain->processBlock(*(_rightBuffer.get()), midiMessages);
        _addBuffers(*(_rightBuffer.get()), buffer);
    }
}

#include "PluginSplitterMidSide.h"

PluginSplitterMidSide::PluginSplitterMidSide(std::function<float(int, MODULATION_TYPE)> getModulationValueCallback)
        : PluginSplitter(DEFAULT_NUM_CHAINS, getModulationValueCallback) {
    juce::Logger::writeToLog("Constructed PluginSplitterMidSide");
}

PluginSplitterMidSide::PluginSplitterMidSide(std::vector<PluginChainWrapper>& chains, std::function<float(int, MODULATION_TYPE)> getModulationValueCallback)
        : PluginSplitter(chains, DEFAULT_NUM_CHAINS, getModulationValueCallback) {
}

void PluginSplitterMidSide::prepareToPlay(double sampleRate, int samplesPerBlock) {
    _midBuffer.reset(new juce::AudioBuffer<float>(4, samplesPerBlock)); // stereo main + stereo sidechain
    _sideBuffer.reset(new juce::AudioBuffer<float>(4, samplesPerBlock)); // stereo main + stereo sidechain

    PluginSplitter::prepareToPlay(sampleRate, samplesPerBlock);
}

void PluginSplitterMidSide::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    // TODO: check if this can be done using mono buffers that refer to the original buffer rather
    // than copying to new ones (guest plugins don't seem to like mono buffers)

    // Make sure to clear the buffers each time, as on a previous call the plugins may have left
    // data in the unused channel of each buffer, and since it's not used it won't get implicitly
    // overwritten (but it'll still be copied to the output)
    _midBuffer->clear();
    _sideBuffer->clear();

    // Convert the left/right buffer to mid/side
    const float* leftRead {buffer.getReadPointer(0)};
    const float* rightRead {buffer.getReadPointer(1)};
    float* midWrite {_midBuffer->getWritePointer(0)};
    float* sideWrite {_sideBuffer->getWritePointer(0)};

    const int numSamples {buffer.getNumSamples()};

    // Add the right channel to get the mid, subtract it to get the side
    juce::FloatVectorOperations::add(midWrite, leftRead, rightRead, numSamples);
    juce::FloatVectorOperations::subtract(sideWrite, leftRead, rightRead, numSamples);

    // Multiply both by 0.5
    juce::FloatVectorOperations::multiply(midWrite, 0.5, numSamples);
    juce::FloatVectorOperations::multiply(sideWrite, 0.5, numSamples);

    // Process the buffers
    if (_numChainsSoloed == 0 || _chains[0].isSoloed) {
        _chains[0].chain->processBlock(*(_midBuffer.get()), midiMessages);
    } else {
        // Mute the mid channel if only the other one is soloed
        juce::FloatVectorOperations::fill(midWrite, 0, numSamples);
    }

    if (_numChainsSoloed == 0 || _chains[1].isSoloed) {
        _chains[1].chain->processBlock(*(_sideBuffer.get()), midiMessages);
    } else {
        // Mute the side channel if only the other one is soloed
        juce::FloatVectorOperations::fill(sideWrite, 0, numSamples);
    }

    // Convert from mid/side back to left/right, overwrite the original buffer with our own output
    float* leftWrite {buffer.getWritePointer(0)};
    float* rightWrite {buffer.getWritePointer(1)};
    const float* midRead {_midBuffer->getReadPointer(0)};
    const float* sideRead {_sideBuffer->getReadPointer(0)};

    // Subtract side from mid to get the left buffer, add them to get the right buffer
    juce::FloatVectorOperations::subtract(leftWrite, midRead, sideRead, numSamples);
    juce::FloatVectorOperations::add(rightWrite, midRead, sideRead, numSamples);
}

#include "ProcessingInterface.hpp"

#include "SplitterProcessors.hpp"

namespace SplitterInterface {
    void prepareToPlay(Splitter& splitter, double sampleRate, int samplesPerBlock, juce::AudioProcessor::BusesLayout layout) {
        WECore::AudioSpinLock lock(splitter.sharedMutex);
        if (splitter.splitter != nullptr) {
            SplitterProcessor::prepareToPlay(*splitter.splitter.get(), sampleRate, samplesPerBlock, layout);
        }
    }

    void releaseResources(Splitter& splitter) {
        WECore::AudioSpinLock lock(splitter.sharedMutex);
        if (splitter.splitter != nullptr) {
            SplitterProcessor::releaseResources(*splitter.splitter.get());
        }
    }

    void reset(Splitter& splitter) {
        WECore::AudioSpinLock lock(splitter.sharedMutex);
        if (splitter.splitter != nullptr) {
            SplitterProcessor::reset(*splitter.splitter.get());
        }
    }

    void processBlock(Splitter& splitter, juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
        // Use the try lock on the audio thread
        WECore::AudioSpinTryLock lock(splitter.sharedMutex);
        if (lock.isLocked() && splitter.splitter != nullptr) {
            SplitterProcessor::processBlock(*splitter.splitter.get(), buffer, midiMessages);
        }
    }
}
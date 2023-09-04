#include "ProcessingInterface.hpp"

#include "SplitterProcessors.hpp"
#include "ModulationProcessors.hpp"

namespace SplitterInterface {
    void prepareToPlay(Splitter& splitter, double sampleRate, int samplesPerBlock, juce::AudioProcessor::BusesLayout layout) {
        WECore::AudioSpinLock lock(splitter.sharedMutex);
        if (splitter.splitter != nullptr) {
            SplitterProcessors::prepareToPlay(*splitter.splitter.get(), sampleRate, samplesPerBlock, layout);
        }
    }

    void releaseResources(Splitter& splitter) {
        WECore::AudioSpinLock lock(splitter.sharedMutex);
        if (splitter.splitter != nullptr) {
            SplitterProcessors::releaseResources(*splitter.splitter.get());
        }
    }

    void reset(Splitter& splitter) {
        WECore::AudioSpinLock lock(splitter.sharedMutex);
        if (splitter.splitter != nullptr) {
            SplitterProcessors::reset(*splitter.splitter.get());
        }
    }

    void processBlock(Splitter& splitter, juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages, juce::AudioPlayHead* newPlayHead) {
        // Use the try lock on the audio thread
        WECore::AudioSpinTryLock lock(splitter.sharedMutex);
        if (lock.isLocked() && splitter.splitter != nullptr) {
            SplitterProcessors::processBlock(*splitter.splitter.get(), buffer, midiMessages, newPlayHead);
        }
    }
}

namespace ModulationInterface {
    void prepareToPlay(ModulationSourcesState& modulationSources, double sampleRate, int samplesPerBlock, juce::AudioProcessor::BusesLayout layout) {
        WECore::AudioSpinLock lock(modulationSources.sharedMutex);
        ModulationProcessors::prepareToPlay(modulationSources, sampleRate, samplesPerBlock, layout);
    }

    void reset(ModulationSourcesState& modulationSources) {
        WECore::AudioSpinLock lock(modulationSources.sharedMutex);
        ModulationProcessors::reset(modulationSources);
    }

    void processBlock(ModulationSourcesState& modulationSources, juce::AudioBuffer<float>& buffer, juce::AudioPlayHead::CurrentPositionInfo tempoInfo) {
        // Use the try lock on the audio thread
        WECore::AudioSpinTryLock lock(modulationSources.sharedMutex);
        if (lock.isLocked()) {
            ModulationProcessors::processBlock(modulationSources, buffer, tempoInfo);
        }

        // TODO LFOs should still be advanced even if the lock is not acquired to stop them
        // drifting out of sync
    }

    double getLfoModulationValue(ModulationSourcesState& modulationSources, int lfoNumber) {
        // Use the try lock on the audio thread
        WECore::AudioSpinTryLock lock(modulationSources.sharedMutex);
        if (lock.isLocked()) {
            return ModulationProcessors::getLfoModulationValue(modulationSources, lfoNumber);
        }

        return 0.0f;
    }

    double getEnvelopeModulationValue(ModulationSourcesState& modulationSources, int envelopeNumber) {
        // Use the try lock on the audio thread
        WECore::AudioSpinTryLock lock(modulationSources.sharedMutex);
        if (lock.isLocked()) {
            return ModulationProcessors::getEnvelopeModulationValue(modulationSources, envelopeNumber);
        }

        return 0.0f;
    }
}

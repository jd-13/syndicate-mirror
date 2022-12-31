#include "FFTProvider.hpp"

FFTProvider::FFTProvider() : _buffer(nullptr),
                             _outputs(nullptr),
                             _fft(FFT_ORDER),
                             _isStereo(false) {
    _buffer = new float[FFT_SIZE];
    _outputs = new float[NUM_OUTPUTS];

    juce::FloatVectorOperations::fill(_buffer, 0, NUM_OUTPUTS);
    juce::FloatVectorOperations::fill(_outputs, 0, NUM_OUTPUTS);

    for (auto& env : _envs) {
        env.setAttackTimeMs(0.1);
        env.setReleaseTimeMs(2);
        env.setFilterEnabled(false);
    }
}

FFTProvider::~FFTProvider() {
    WECore::AudioSpinLock lock(_fftMutex);
    delete[] _buffer;
    delete[] _outputs;
}

void FFTProvider::setSampleRate(double sampleRate) {
    for (auto& env : _envs) {
        env.setSampleRate(sampleRate);
    }
}

void FFTProvider::reset() {
    for (auto& env : _envs) {
        env.reset();
    }
}

void FFTProvider::processBlock(juce::AudioBuffer<float>& buffer) {
    WECore::AudioSpinTryLock lock(_fftMutex);
    if (lock.isLocked()) {
        const size_t numBuffersRequired {static_cast<size_t>(
            std::ceil(static_cast<double>(buffer.getNumSamples()) / FFT_SIZE)
        )};

        for (size_t bufferNumber {0}; bufferNumber < numBuffersRequired; bufferNumber++) {
            // Calculate how many samples need to be processed in this chunk
            const size_t numSamplesRemaining {buffer.getNumSamples() - (bufferNumber * FFT_SIZE)};
            const size_t numSamplesToCopy {
                std::min(numSamplesRemaining, static_cast<size_t>(FFT_SIZE))
            };

            // The input buffer size might be smaller than the FFT buffer size, so then we need to
            // append the the existing FFT buffer by shifting it back and adding the new samples to
            // the end
            float* const fillStart {_buffer + FFT_SIZE - numSamplesToCopy};
            juce::FloatVectorOperations::copy(_buffer, fillStart, numSamplesToCopy);

            // Add the left and right buffers
            if (_isStereo) {
                const float* const leftBufferInputStart {buffer.getReadPointer(0) + bufferNumber * FFT_SIZE};
                const float* const rightBufferInputStart {buffer.getReadPointer(1) + bufferNumber * FFT_SIZE};
                juce::FloatVectorOperations::add(fillStart, leftBufferInputStart, rightBufferInputStart, numSamplesToCopy);
                juce::FloatVectorOperations::multiply(fillStart, 0.5, numSamplesToCopy);
            } else {
                const float* const bufferInputStart {buffer.getReadPointer(0) + bufferNumber * FFT_SIZE};
                juce::FloatVectorOperations::copy(fillStart, bufferInputStart, numSamplesToCopy);
            }

            // Perform the FFT
            _fft.performFrequencyOnlyForwardTransform(_buffer);

            // Run each FFT output bin through an envelope follower so that it is smoothed when
            // displayed on the UI
            for (int index {0}; index < NUM_OUTPUTS; index++) {
                _outputs[index] = _envs[index].getNextOutput(_buffer[index]);
            }
        }
    }
}
#include "SplitterBand.h"

void SplitterBand::setLowCutoff(double val) {
    _lowCutoffHz = WECore::MONSTR::Parameters::CROSSOVER_FREQUENCY.BoundsCheck(val);
    _monoFilters.setupLow(_sampleRate, _lowCutoffHz);
    _stereoFilters.setupLow(_sampleRate, _lowCutoffHz);

    // Move the high cutoff up if necessary as they shouldn't swap places
    if (_lowCutoffHz > _highCutoffHz) {
        _monoFilters.setupHigh(_sampleRate, _lowCutoffHz);
        _stereoFilters.setupHigh(_sampleRate, _lowCutoffHz);

    }
}

void SplitterBand::setHighCutoff(double val) {
    _highCutoffHz = WECore::MONSTR::Parameters::CROSSOVER_FREQUENCY.BoundsCheck(val);
    _monoFilters.setupHigh(_sampleRate, _highCutoffHz);
    _stereoFilters.setupHigh(_sampleRate, _highCutoffHz);

    // Move the low cutoff down if necessary as they shouldn't swap places
    if (_lowCutoffHz > _highCutoffHz) {
        _monoFilters.setupLow(_sampleRate, _highCutoffHz);
        _stereoFilters.setupLow(_sampleRate, _highCutoffHz);
    }
}

void SplitterBand::setBandType(BandType bandType) {
    _bandType = bandType;
}

void SplitterBand::setSampleRate(double newSampleRate) {
    // if the new sample rate is different, recalculate the filter coefficients
    if (!WECore::CoreMath::compareFloatsEqual(newSampleRate, _sampleRate)) {
        _sampleRate = newSampleRate;
        setLowCutoff(_lowCutoffHz);
        setHighCutoff(_highCutoffHz);
    }
}

void SplitterBand::setIsStereo(bool val) {
    _isStereo = val;
}

void SplitterBand::processBlock(juce::AudioBuffer<float>& buffer) {
    if (_isMuted) {
        // TODO don't clear side chain - check mute on other splitters for this
        // Muted - set the output to 0 for this band
        buffer.clear();
    } else {
        // Apply the filtering before processing
        if (_isStereo) {
            _stereoFilters.processBlock(buffer, _bandType);
        } else {
            _monoFilters.processBlock(buffer, _bandType);
        }

        if (_isActive) {
            if (_chain != nullptr) {
                // TODO support midi buffers
                juce::MidiBuffer midiBuffer;
                _chain->processBlock(buffer, midiBuffer);
            }
        }
    }
}

void SplitterBand::reset() {
    _monoFilters.reset();
    _stereoFilters.reset();
}

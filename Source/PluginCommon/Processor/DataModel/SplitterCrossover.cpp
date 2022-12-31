#include "SplitterCrossover.hpp"

SplitterCrossover::SplitterCrossover() : _numBands(WECore::MONSTR::Parameters::_DEFAULT_NUM_BANDS), _numBandsSoloed(0) {

    // The bands are defaulted to lower, set them correctly
    static_assert(WECore::MONSTR::Parameters::_DEFAULT_NUM_BANDS == 3,
                    "This constructor code needs to be updated if the default changes");
    _bands[1].band.setBandType(BandType::MIDDLE);
    _bands[2].band.setBandType(BandType::UPPER);

    setCrossoverFrequency(0, WECore::MONSTR::Parameters::CROSSOVER_LOWER_DEFAULT);
    setCrossoverFrequency(1, WECore::MONSTR::Parameters::CROSSOVER_UPPER_DEFAULT);
}

void SplitterCrossover::setIsActive(size_t index, bool isActive) {
    if (index < _bands.size()) {
        _bands[index].band.setIsActive(isActive);
    }
}

void SplitterCrossover::setIsMuted(size_t index, bool isMuted) {
    if (index < _bands.size()) {
        _bands[index].band.setIsMuted(isMuted);
    }
}

void SplitterCrossover::setIsSoloed(size_t index, bool isSoloed) {
    if (index < _bands.size()) {

        // If the new value is different to the existing one, update it and the counter
        if (isSoloed != _bands[index].isSoloed) {

            _bands[index].isSoloed = isSoloed;

            if (isSoloed) {
                _numBandsSoloed++;
            } else {
                _numBandsSoloed--;
            }
        }
    }
}

void SplitterCrossover::setCrossoverFrequency(size_t index, double val) {

    if (index < _bands.size() - 1) {

        // Set the crossover frequency
        val = WECore::MONSTR::Parameters::CROSSOVER_FREQUENCY.BoundsCheck(val);
        _bands[index].band.setHighCutoff(val);
        _bands[index + 1].band.setLowCutoff(val);

        // Make sure the crossover frequencies are still in the correct order
        for (size_t otherCrossoverIndex {0}; otherCrossoverIndex < _bands.size() - 1; otherCrossoverIndex++) {

            const double otherCrossoverFrequency {getCrossoverFrequency(otherCrossoverIndex)};

            const bool needsCrossoverUpdate {
                // We've moved the crossover frequency of index below another one that should be
                // below it - move the other one to the new value
                (val < otherCrossoverFrequency && otherCrossoverIndex < index) ||

                // We've moved the crossover frequency of index above another one that should be
                // above it - move the other one to the new value
                (otherCrossoverFrequency > val && index > otherCrossoverIndex)
            };

            if (needsCrossoverUpdate) {
                // We've moved the crossover frequency of index below another one that should be
                // below it - move the other one to the new value
                setCrossoverFrequency(otherCrossoverIndex, val);

            }
        }
    }
}

void SplitterCrossover::setPluginChain(size_t index, PluginChain* chain) {
    if (index < _bands.size()) {
        _bands[index].band.setPluginChain(chain);
    }
}

void SplitterCrossover::setSampleRate(double newSampleRate) {
    for (BandWrapper& band : _bands) {
        band.band.setSampleRate(newSampleRate);
    }
}

void SplitterCrossover::setNumBands(int val) {
    _numBands = WECore::MONSTR::Parameters::NUM_BANDS.BoundsCheck(val);

    // Make sure the bands are set to the correct types
    for (size_t bandIdx {0}; bandIdx < _numBands; bandIdx++) {
        if (bandIdx == 0) {
            _bands[bandIdx].band.setBandType(BandType::LOWER);
        } else if (bandIdx == _numBands - 1) {
            _bands[bandIdx].band.setBandType(BandType::UPPER);
        } else {
            _bands[bandIdx].band.setBandType(BandType::MIDDLE);
        }
    }
}

void SplitterCrossover::setIsStereo(bool val) {
    for (BandWrapper& band : _bands) {
        band.band.setIsStereo(val);
    }
}

bool SplitterCrossover::getIsActive(size_t index) const {
    bool retVal {false};

    if (index < _bands.size()) {
        retVal = _bands[index].band.getIsActive();
    }

    return retVal;
}

bool SplitterCrossover::getIsMuted(size_t index) const {
    bool retVal {false};

    if (index < _bands.size()) {
        retVal = _bands[index].band.getIsMuted();
    }

    return retVal;
}

bool SplitterCrossover::getIsSoloed(size_t index) const {
    bool retVal {false};

    if (index < _bands.size()) {
        retVal = _bands[index].isSoloed;
    }

    return retVal;
}

double SplitterCrossover::getCrossoverFrequency(size_t index) const {
    double retVal {0};

    if (index < _bands.size()) {
        retVal = _bands[index].band.getHighCutoff();
    }

    return retVal;
}

void SplitterCrossover::addBand() {

    if (static_cast<int>(_numBands) < WECore::MONSTR::Parameters::NUM_BANDS.maxValue) {

        // Convert the current highest band to a middle band
        _bands[_numBands - 1].band.setBandType(BandType::MIDDLE);

        // Set the next band up to be an upper band
        _bands[_numBands].band.setBandType(BandType::UPPER);

        const double oldHighestCrossover {getCrossoverFrequency(_numBands - 2)};

        if (oldHighestCrossover < WECore::MONSTR::Parameters::CROSSOVER_FREQUENCY.maxValue) {
            // The old highest crossover frequency is below the maximum, insert the new one halfway
            // between it and the maximum
            const double topBandWidth {WECore::MONSTR::Parameters::CROSSOVER_FREQUENCY.maxValue - oldHighestCrossover};
            setCrossoverFrequency(_numBands - 1, oldHighestCrossover + (topBandWidth / 2));
        } else {
            // The old highest crossover is at the maximum, move it halfway down to the one below
            // and place the new one at the maximum
            const double secondHighestCrossover {getCrossoverFrequency(_numBands - 3)};
            const double bandWidth {WECore::MONSTR::Parameters::CROSSOVER_FREQUENCY.maxValue - secondHighestCrossover};

            setCrossoverFrequency(_numBands - 2, secondHighestCrossover + (bandWidth / 2));
            setCrossoverFrequency(_numBands - 1, WECore::MONSTR::Parameters::CROSSOVER_FREQUENCY.maxValue);
        }

        _numBands++;
    }

    reset();
}

void SplitterCrossover::removeBand() {

    if (static_cast<int>(_numBands) > WECore::MONSTR::Parameters::NUM_BANDS.minValue) {
        // Make sure the band's parameters are reset to their default values
        setIsActive(_numBands - 1, true);
        setIsMuted(_numBands - 1, false);
        setPluginChain(_numBands - 1, nullptr);

        // Decrement the counter and make the new highest band the upper band
        _numBands--;
        _bands[_numBands - 1].band.setBandType(BandType::UPPER);
    }

    reset();
}

void SplitterCrossover::processBlock(juce::AudioBuffer<float>& buffer) {

    // If the buffer we've been passed is bigger than our static internal buffer, then we need
    // to break it into chunks
    const size_t numBuffersRequired {static_cast<size_t>(
        std::ceil(static_cast<double>(buffer.getNumSamples()) / INTERNAL_BUFFER_SIZE)
        )};

    const int numChannels {std::min(buffer.getNumChannels(), INTERNAL_BUFFER_CHANNELS)};

    for (size_t bufferNumber {0}; bufferNumber < numBuffersRequired; bufferNumber++) {

        // Calculate how many samples need to be processed in this chunk
        const size_t numSamplesRemaining {buffer.getNumSamples() - (bufferNumber * INTERNAL_BUFFER_SIZE)};
        const size_t numSamplesToCopy {
            std::min(numSamplesRemaining, static_cast<size_t>(INTERNAL_BUFFER_SIZE))
        };

        // Populate the band specific buffers, and do the processing
        for (size_t bandIndex {0}; bandIndex < _numBands; bandIndex++) {
            BandWrapper& thisBand = _bands[bandIndex];

            // Only do processing no bands are soloed or if this band is soloed
            if (_numBandsSoloed == 0 || thisBand.isSoloed) {

                // Copy the input buffer into the buffer for this band
                for (size_t channelIndex {0}; channelIndex < numChannels; channelIndex++) {
                    float* const inputBufferStart {buffer.getWritePointer(channelIndex) + bufferNumber * INTERNAL_BUFFER_SIZE};

                    juce::FloatVectorOperations::copy(thisBand.buffer.getWritePointer(channelIndex),
                                                      inputBufferStart,
                                                      numSamplesToCopy);
                }

                // Do processing
                thisBand.band.processBlock(thisBand.buffer);
            }
        }

        // Combine the output from each band, and write to output
        for (size_t channelIndex {0}; channelIndex < numChannels; channelIndex++) {
            float* outputBufferStart {buffer.getWritePointer(channelIndex) + bufferNumber * INTERNAL_BUFFER_SIZE};
            juce::FloatVectorOperations::clear(outputBufferStart, INTERNAL_BUFFER_SIZE);

            for (size_t bandIndex {0}; bandIndex < _numBands; bandIndex++) {
                BandWrapper& thisBand = _bands[bandIndex];

                if (_numBandsSoloed == 0 || thisBand.isSoloed) {
                    juce::FloatVectorOperations::add(outputBufferStart, thisBand.buffer.getReadPointer(channelIndex), INTERNAL_BUFFER_SIZE);
                }
            }
        }
    }
}

void SplitterCrossover::reset() {
    for (BandWrapper& band : _bands) {
        band.band.reset();
    }
}

#pragma once

#include <algorithm>
#include <array>
#include "SplitterBand.h"

class SplitterCrossover {
public:
    SplitterCrossover();
    virtual ~SplitterCrossover() = default;

    void setIsActive(size_t index, bool isActive);
    void setIsMuted(size_t index, bool isMuted);
    void setIsSoloed(size_t index, bool isSoloed);
    void setCrossoverFrequency(size_t index, double val);
    void setPluginChain(size_t index, PluginChain* chain);
    void setSampleRate(double newSampleRate);
    void setNumBands(int val);
    void setIsStereo(bool val);

    bool getIsActive(size_t index) const;
    bool getIsMuted(size_t index) const;
    bool getIsSoloed(size_t index) const;
    double getCrossoverFrequency(size_t index) const;
    size_t getNumBands() { return _numBands; }

    void addBand();
    void removeBand();
    void processBlock(juce::AudioBuffer<float> buffer);

    void reset();

private:
    static constexpr int INTERNAL_BUFFER_SIZE = 512;
    static constexpr int INTERNAL_BUFFER_CHANNELS = 4;

    class BandWrapper {
    public:
        BandWrapper() : band(BandType::LOWER),
                        isSoloed(WECore::MONSTR::Parameters::BANDSOLO_DEFAULT),
                        buffer(INTERNAL_BUFFER_CHANNELS, INTERNAL_BUFFER_SIZE) {
        }

        SplitterBand band;
        bool isSoloed;
        juce::AudioBuffer<float> buffer;
    };

    size_t _numBands;
    size_t _numBandsSoloed;
    std::array<BandWrapper, WECore::MONSTR::Parameters::_MAX_NUM_BANDS> _bands;
};

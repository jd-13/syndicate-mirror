#pragma once

#include <memory>
#include <JuceHeader.h>
#include "MONSTRFilters/MONSTRParameters.h"
#include "General/CoreMath.h"
#include "WEFilters/EffectsProcessor.h"
#include "PluginChain.hpp"

// DSPFilters sets off a lot of clang warnings - disable them for Butterworth.h only
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wignored-qualifiers"
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wextra-semi-stmt"
#elif __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#endif

#include "DspFilters/Butterworth.h"

#ifdef __clang__
#pragma clang diagnostic pop
#elif __GNUC__
#pragma GCC diagnostic pop
#endif

class SplitterCrossover;

enum class BandType {
    LOWER,
    MIDDLE,
    UPPER
};

class MonoFilterBank {
public:
    MonoFilterBank() = default;

    void setupLow(double sampleRate, double lowCutoffHz) {
        _lowCut1.setup(FILTER_ORDER, sampleRate, lowCutoffHz);
        _lowCut2.setup(FILTER_ORDER, sampleRate, lowCutoffHz);
    }

    void setupHigh(double sampleRate, double highCutoffHz) {
        _highCut1.setup(FILTER_ORDER, sampleRate, highCutoffHz);
        _highCut2.setup(FILTER_ORDER, sampleRate, highCutoffHz);
    }

    void reset() {
        _lowCut1.reset();
        _lowCut2.reset();
        _highCut1.reset();
        _highCut2.reset();
    }

    void processBlock(juce::AudioBuffer<float>& buffer, BandType bandType) {
        float* channelsArray[1];

        channelsArray[0] = buffer.getWritePointer(0);

        if (bandType == BandType::LOWER) {
            _highCut1.process(buffer.getNumSamples(), channelsArray);
            _highCut2.process(buffer.getNumSamples(), channelsArray);
        } else if (bandType == BandType::UPPER) {
            _lowCut1.process(buffer.getNumSamples(), channelsArray);
            _lowCut2.process(buffer.getNumSamples(), channelsArray);
        } else {
            _lowCut1.process(buffer.getNumSamples(), channelsArray);
            _lowCut2.process(buffer.getNumSamples(), channelsArray);
            _highCut1.process(buffer.getNumSamples(), channelsArray);
            _highCut2.process(buffer.getNumSamples(), channelsArray);
        }
    }

private:
    static constexpr int FILTER_ORDER {2};

    Dsp::SimpleFilter<Dsp::Butterworth::HighPass<FILTER_ORDER>, 1> _lowCut1;
    Dsp::SimpleFilter<Dsp::Butterworth::HighPass<FILTER_ORDER>, 1> _lowCut2;
    Dsp::SimpleFilter<Dsp::Butterworth::LowPass<FILTER_ORDER>, 1> _highCut1;
    Dsp::SimpleFilter<Dsp::Butterworth::LowPass<FILTER_ORDER>, 1> _highCut2;
};

class StereoFilterBank {
public:
    StereoFilterBank() = default;

    void setupLow(double sampleRate, double lowCutoffHz) {
        _lowCut1.setup(FILTER_ORDER, sampleRate, lowCutoffHz);
        _lowCut2.setup(FILTER_ORDER, sampleRate, lowCutoffHz);
    }

    void setupHigh(double sampleRate, double highCutoffHz) {
        _highCut1.setup(FILTER_ORDER, sampleRate, highCutoffHz);
        _highCut2.setup(FILTER_ORDER, sampleRate, highCutoffHz);
    }

    void reset() {
        _lowCut1.reset();
        _lowCut2.reset();
        _highCut1.reset();
        _highCut2.reset();
    }

    void processBlock(juce::AudioBuffer<float>& buffer, BandType bandType) {
        float* channelsArray[2];

        channelsArray[0] = buffer.getWritePointer(0);
        channelsArray[1] = buffer.getWritePointer(1);

        if (bandType == BandType::LOWER) {
            _highCut1.process(buffer.getNumSamples(), channelsArray);
            _highCut2.process(buffer.getNumSamples(), channelsArray);
        } else if (bandType == BandType::UPPER) {
            _lowCut1.process(buffer.getNumSamples(), channelsArray);
            _lowCut2.process(buffer.getNumSamples(), channelsArray);
        } else {
            _lowCut1.process(buffer.getNumSamples(), channelsArray);
            _lowCut2.process(buffer.getNumSamples(), channelsArray);
            _highCut1.process(buffer.getNumSamples(), channelsArray);
            _highCut2.process(buffer.getNumSamples(), channelsArray);
        }
    }

private:
    static constexpr int FILTER_ORDER {2};

    Dsp::SimpleFilter<Dsp::Butterworth::HighPass<FILTER_ORDER>, 2> _lowCut1;
    Dsp::SimpleFilter<Dsp::Butterworth::HighPass<FILTER_ORDER>, 2> _lowCut2;
    Dsp::SimpleFilter<Dsp::Butterworth::LowPass<FILTER_ORDER>, 2> _highCut1;
    Dsp::SimpleFilter<Dsp::Butterworth::LowPass<FILTER_ORDER>, 2> _highCut2;
};

class SplitterBand {
public:
    SplitterBand(BandType bandType) : _bandType(bandType),
                                      _isActive(WECore::MONSTR::Parameters::BANDSWITCH_DEFAULT),
                                      _isMuted(WECore::MONSTR::Parameters::BANDMUTED_DEFAULT),
                                      _lowCutoffHz(100),
                                      _highCutoffHz(5000),
                                      _sampleRate(44100),
                                      _chain(nullptr),
                                      _isStereo(false) {
        _monoFilters.setupLow(_sampleRate, _lowCutoffHz);
        _stereoFilters.setupLow(_sampleRate, _lowCutoffHz);
        _monoFilters.setupHigh(_sampleRate, _highCutoffHz);
        _stereoFilters.setupHigh(_sampleRate, _highCutoffHz);
    }

    ~SplitterBand() = default;

    void setIsActive(bool val) { _isActive = val; }
    void setIsMuted(bool val) { _isMuted = val; }
    void setLowCutoff(double val);
    void setHighCutoff(double val);
    void setBandType(BandType bandType);
    void setSampleRate(double newSampleRate);
    void setPluginChain(PluginChain* chain) { _chain = chain; }
    void setIsStereo(bool val);

    bool getIsActive() const { return _isActive; }
    bool getIsMuted() const { return _isMuted; }
    double getLowCutoff() const { return _lowCutoffHz; }
    double getHighCutoff() const { return _highCutoffHz; }

    void processBlock(juce::AudioBuffer<float>& buffer);

    void reset();

private:
    BandType _bandType;

    bool _isActive;
    bool _isMuted;

    double _lowCutoffHz;
    double _highCutoffHz;

    double _sampleRate;

    MonoFilterBank _monoFilters;
    StereoFilterBank _stereoFilters;

    PluginChain* _chain;

    bool _isStereo;
};

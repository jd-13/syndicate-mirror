#pragma once

#include <JuceHeader.h>

#include "PluginSplitter.h"
#include "SplitterCrossover.h"
#include "WEFilters/EffectsProcessor.h"
#include "WEFilters/AREnvelopeFollowerSquareLaw.h"

/**
 * Performs an FFT on the signal which can be provided to the UI to drive the visualiser.
 */
class FFTProvider {
public:
    static constexpr int FFT_ORDER {7};
    static constexpr int FFT_SIZE {(1 << FFT_ORDER) * 2};
    static constexpr int NUM_OUTPUTS { FFT_SIZE / 4 };

    FFTProvider();
    ~FFTProvider();

    void setSampleRate(double sampleRate);

    void reset();

    void processBlock(juce::AudioBuffer<float>& buffer);

    const float* getOutputs() { return _outputs; }

private:
    float* _buffer;
    float* _outputs;
    juce::dsp::FFT _fft;
    std::array<WECore::AREnv::AREnvelopeFollowerSquareLaw, NUM_OUTPUTS> _envs;
    WECore::AudioSpinMutex _fftMutex;
};

/**
 * Contains a single plugin graph for plugins arranged in a multiband split.
 */
class PluginSplitterMultiband : public PluginSplitter {
public:
    PluginSplitterMultiband(std::function<float(int, MODULATION_TYPE)> getModulationValueCallback, bool isStereo);
    PluginSplitterMultiband(std::vector<PluginChainWrapper>& chains, std::function<float(int, MODULATION_TYPE)> getModulationValueCallback, bool isStereo);
    ~PluginSplitterMultiband() = default;

    SPLIT_TYPE getSplitType() override { return SPLIT_TYPE::MULTIBAND; }

    bool addBand();
    bool removeBand();
    size_t getNumBands();
    void setCrossoverFrequency(size_t index, double val);
    double getCrossoverFrequency(size_t index);

    void setChainSolo(int chainNumber, bool val) override;
    bool getChainSolo(int chainNumber) override;

    int getFFTOutputsSize() { return FFTProvider::NUM_OUTPUTS; }
    const float* getFFTOutputs() { return _fftProvider.getOutputs(); }

    void restoreFromXml(juce::XmlElement* element,
                        HostConfiguration configuration,
                        const PluginConfigurator& pluginConfigurator,
                        std::function<void(juce::String)> onErrorCallback) override;
    void writeToXml(juce::XmlElement* element) override;

    // AudioProcessor methods
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

private:
    static constexpr int DEFAULT_NUM_CHAINS {2};
    SplitterCrossover _crossover;
    FFTProvider _fftProvider;
};

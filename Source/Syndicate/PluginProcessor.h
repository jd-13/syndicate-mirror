/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "CoreJUCEPlugin/CoreAudioProcessor.h"
#include "CoreJUCEPlugin/CustomParameter.h"
#include "MainLogger.h"
#include "PluginScanClient.h"
#include "PluginSplitter.h"
#include "PluginSelectorState.h"
#include "PluginParameterSelectorState.h"
#include "ChainParameters.h"
#include "General/AudioSpinMutex.h"
#include "ParameterData.h"
#include "RichterLFO/RichterLFO.h"
#include "EnvelopeFollowerWrapper.h"
#include "PluginConfigurator.h"

class SyndicateAudioProcessorEditor;

//==============================================================================
/**
*/
class SyndicateAudioProcessor : public WECore::JUCEPlugin::CoreAudioProcessor,
                                public LatencyListener
{
public:
    PluginScanClient pluginScanClient;
    PluginSelectorState pluginSelectorState; // TODO convert this to a custom parameter
    PluginParameterSelectorState pluginParameterSelectorState;
    std::unique_ptr<PluginSplitter> pluginSplitter;
    WECore::AudioSpinMutex pluginSplitterMutex;
    std::vector<ChainParameters> chainParameters;
    std::vector<std::shared_ptr<WECore::Richter::RichterLFO>> lfos;
    std::vector<EnvelopeFollowerWrapper> envelopes;
    PluginConfigurator pluginConfigurator;
    std::array<juce::String, NUM_MACROS> macroNames;
    std::array<WECore::AREnv::AREnvelopeFollowerSquareLaw, 2> meterEnvelopes;
    std::vector<juce::String> restoreErrors; // Populated during restore, displayed and cleared when the UI is opened

    //==============================================================================
    SyndicateAudioProcessor();
    ~SyndicateAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void reset() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    // Public parameters
    juce::AudioParameterFloat* outputGainLog;
    juce::AudioParameterFloat* outputPan;
    std::array<juce::AudioParameterFloat*, NUM_MACROS> macros;

    void setEditor(SyndicateAudioProcessorEditor* editor) { _editor = editor; }
    void removeEditor() { _editor = nullptr; }

    float getModulationValueForSource(int id, MODULATION_TYPE type);
    void addLfo();
    void addEnvelope();
    void removeModulationSource(ModulationSourceDefinition definition);

    void setSplitType(SPLIT_TYPE splitType);
    SPLIT_TYPE getSplitType() { return _splitType; }

    // Parallel Split
    void addParallelChain();
    void removeParallelChain(int chainNumber);

    // Multiband Split
    void addCrossoverBand();
    void removeCrossoverBand();
    void setCrossoverFrequency(size_t index, float val);
    float getCrossoverFrequency(size_t index);

    // Plugin events
    bool onPluginSelectedByUser(std::shared_ptr<juce::AudioPluginInstance> plugin,
                                int chainNumber,
                                int pluginNumber);

    void removePlugin(int chainNumber, int pluginNumber);

    void insertGainStage(int chainNumber, int pluginNumber);

    void moveSlot(int fromChainNumber, int fromSlotNumber, int toChainNumber, int toSlotNumber);

    /**
     * Returns true if split types that require a stereo in/out configuration can be used.
     */
    bool canDoStereoSplitTypes() const;

private:
    /**
     * Provides a way for the processor to trigger UI updates, and also manages saving and restoring
     * parameter state for the splitter.
     */
    class SplitterParameters : public WECore::JUCEPlugin::CustomParameter {
    public:
        SplitterParameters() : _processor(nullptr) { }
        ~SplitterParameters() = default;

        void triggerUpdate() { _updateListener(); }

        void setProcessor(SyndicateAudioProcessor* processor) { _processor = processor; }

        void restoreFromXml(juce::XmlElement* element) override;
        void writeToXml(juce::XmlElement* element) override;

    private:
        SyndicateAudioProcessor* _processor;

        void _restoreSplitterFromXml(juce::XmlElement* element);
        void _restoreChainParameters();
        void _restoreModulationSourcesFromXml(juce::XmlElement* element);
        void _restoreMacroNamesFromXml(juce::XmlElement* element);

        void _writeSplitterToXml(juce::XmlElement* element);
        void _writeModulationSourcesToXml(juce::XmlElement* element);
        void _writeMacroNamesToXml(juce::XmlElement* element);
    };

    MainLogger _logger;
    SyndicateAudioProcessorEditor* _editor;
    SPLIT_TYPE _splitType;
    double _outputGainLinear;

    SplitterParameters* _splitterParameters;

    bool _isSplitterInitialised;

    std::vector<juce::String> _provideParamNamesForMigration() override;
    void _migrateParamValues(std::vector<float>& paramValues) override;

    void _onParameterUpdate() override;

    void _resetModulationSources();

    void _onLatencyChange() override;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SyndicateAudioProcessor)
};

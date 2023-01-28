/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"

#include "PluginEditor.h"
#include "ParameterData.h"
#include "AllUtils.h"
#include "PluginUtils.h"
#include "ChainMutators.hpp"
#include "SplitterMutators.hpp"
#include "SplitterProcessors.hpp"
#include "XmlReader.hpp"
#include "XmlWriter.hpp"

namespace {
    // Modulation sources
    const char* XML_MODULATION_SOURCES_STR {"ModulationSources"};

    const char* XML_LFOS_STR {"LFOs"};
    const char* XML_LFO_BYPASS_STR {"lfoBypass"};
    const char* XML_LFO_PHASE_SYNC_STR {"lfoPhaseSync"};
    const char* XML_LFO_TEMPO_SYNC_STR {"lfoTempoSync"};
    const char* XML_LFO_INVERT_STR {"lfoInvert"};
    const char* XML_LFO_WAVE_STR {"lfoWave"};
    const char* XML_LFO_TEMPO_NUMER_STR {"lfoTempoNumer"};
    const char* XML_LFO_TEMPO_DENOM_STR {"lfoTempoDenom"};
    const char* XML_LFO_FREQ_STR {"lfoFreq"};
    const char* XML_LFO_FREQ_MOD_STR {"lfoFreqMod"};
    const char* XML_LFO_DEPTH_STR {"lfoDepth"};
    const char* XML_LFO_DEPTH_MOD_STR {"lfoDepthMod"};
    const char* XML_LFO_MANUAL_PHASE_STR {"lfoManualPhase"};

    const char* XML_ENVELOPES_STR {"Envelopes"};
    const char* XML_ENV_ATTACK_TIME_STR {"envelopeAttack"};
    const char* XML_ENV_RELEASE_TIME_STR {"envelopeRelease"};
    const char* XML_ENV_FILTER_ENABLED_STR {"envelopeFilterEnabled"};
    const char* XML_ENV_LOW_CUT_STR {"envelopeLowCut"};
    const char* XML_ENV_HIGH_CUT_STR {"envelopeHighCut"};
    const char* XML_ENV_AMOUNT_STR {"envelopeAmount"};

    const char* XML_MACRO_NAMES_STR {"MacroNames"};

    const char* XML_CACHED_CROSSOVER_FREQUENCIES_STR {"CrossoverFrequencies"};

    const char* XML_MAIN_WINDOW_STATE_STR {"MainWindowState"};
    const char* XML_MAIN_WINDOW_BOUNDS_STR {"MainEditorBounds"};
    const char* XML_GRAPH_VIEW_POSITION_STR {"GraphViewScrollPosition"};
    const char* XML_CHAIN_VIEW_POSITIONS_STR {"ChainViewScrollPositions"};
    const char* XML_LFO_BUTTONS_POSITION_STR {"LfoButtonsScrollPosition"};
    const char* XML_ENV_BUTTONS_POSITION_STR {"EnvButtonsScrollPosition"};

    std::string getLfoXMLName(int lfoNumber) {
        std::string retVal("LFO_");
        retVal += std::to_string(lfoNumber);
        return retVal;
    }

    std::string getEnvelopeXMLName(int envelopeNumber) {
        std::string retVal("Envelope_");
        retVal += std::to_string(envelopeNumber);
        return retVal;
    }

    juce::String getMacroNameXMLName(int macroNumber) {
        juce::String retVal("MacroName_");
        retVal += juce::String(macroNumber);
        return retVal;
    }

    juce::String getCachedCrossoverFreqXMLName(int crossoverNumber) {
        juce::String retVal("Crossover_");
        retVal += juce::String(crossoverNumber);
        return retVal;
    }

    juce::String getChainPositionXMLName(int chainNumber) {
        juce::String retVal("Chain_");
        retVal += juce::String(chainNumber);
        return retVal;
    }

    // Window states
    const char* XML_PLUGIN_SELECTOR_STATE_STR {"pluginSelectorState"};
    const char* XML_PLUGIN_PARAMETER_SELECTOR_STATE_STR {"pluginParameterSelectorState"};
}

//==============================================================================
SyndicateAudioProcessor::SyndicateAudioProcessor() :
        WECore::JUCEPlugin::CoreAudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                                                .withOutput("Output", juce::AudioChannelSet::stereo(), true)
                                                                .withInput("Sidechain", juce::AudioChannelSet::stereo(), true)),
        _logger(JucePlugin_Name, JucePlugin_VersionString, Utils::PluginLogDirectory),
        _editor(nullptr),
        _splitType(SPLIT_TYPE::SERIES),
        _outputGainLinear(1),
        _isSplitterInitialised(false)
{
    juce::Logger::setCurrentLogger(&_logger);

    constexpr float PRECISION {0.01f};
    registerPrivateParameter(_splitterParameters, "SplitterParameters");

    // Register the macro parameters
    for (int index {0}; index < macros.size(); index++) {
        registerParameter(macros[index], MACRO_STRS[index], &MACRO, MACRO.defaultValue, PRECISION);
    }

    registerParameter(outputGainLog, OUTPUTGAIN_STR, &OUTPUTGAIN, OUTPUTGAIN.defaultValue, PRECISION);
    registerParameter(outputPan, OUTPUTPAN_STR, &OUTPUTPAN, OUTPUTPAN.defaultValue, PRECISION);

    // Add a default LFO and envelope
    addLfo();
    addEnvelope();

    // Make sure everything is initialised
    _splitterParameters->setProcessor(this);
    setSplitType(SPLIT_TYPE::SERIES);
    chainParameters.emplace_back([&]() { _splitterParameters->triggerUpdate(); });
    _onParameterUpdate();
    pluginScanClient.restore();

    for (int index {0}; index < macroNames.size(); index++) {
        macroNames[index] = "Macro " + juce::String(index + 1);
    }

    for (auto& env : meterEnvelopes) {
        env.setAttackTimeMs(1);
        env.setReleaseTimeMs(50);
        env.setFilterEnabled(false);
    }

    formatManager.addDefaultFormats();
}

SyndicateAudioProcessor::~SyndicateAudioProcessor()
{
    pluginScanClient.stopScan();

    // Logger must be removed before being deleted
    // (this must be the last thing we do before exiting)
    juce::Logger::setCurrentLogger(nullptr);
}

//==============================================================================
const juce::String SyndicateAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SyndicateAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SyndicateAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SyndicateAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SyndicateAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SyndicateAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SyndicateAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SyndicateAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SyndicateAudioProcessor::getProgramName (int index)
{
    return {};
}

void SyndicateAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SyndicateAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    // Update the modulation sources
    for (std::shared_ptr<WECore::Richter::RichterLFO> lfo : lfos) {
        lfo->setSampleRate(sampleRate);
    }

    for (EnvelopeFollowerWrapper& env : envelopes) {
        env.envelope->setSampleRate(sampleRate);
    }

    for (auto& env : meterEnvelopes) {
        env.setSampleRate(sampleRate);
    }

    // Set the bus layout before calling prepare to play, the splitter will need the buses to be
    // correct before then
    WECore::AudioSpinLock lock(pluginSplitterMutex);
    if (pluginSplitter != nullptr) {
        juce::Logger::writeToLog("Setting bus layout:\n" + Utils::busesLayoutToString(getBusesLayout()));
        SplitterProcessor::prepareToPlay(*pluginSplitter.get(), sampleRate, samplesPerBlock, getBusesLayout());
    }
}

void SyndicateAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    WECore::AudioSpinLock lock(pluginSplitterMutex);
    if (pluginSplitter != nullptr) {
        SplitterProcessor::releaseResources(*pluginSplitter.get());
    }

    _resetModulationSources();
}

void SyndicateAudioProcessor::reset() {
    _resetModulationSources();

    for (auto& env : meterEnvelopes) {
        env.reset();
    }

    WECore::AudioSpinLock lock(pluginSplitterMutex);
    if (pluginSplitter != nullptr) {
        SplitterProcessor::reset(*pluginSplitter.get());
    }
}

bool SyndicateAudioProcessor::isBusesLayoutSupported(const BusesLayout& layout) const {
    const bool inputEqualsOutput {
        layout.getMainInputChannelSet() == layout.getMainOutputChannelSet()
    };

    const bool isMonoOrStereo {
        layout.getMainInputChannelSet().size() == 1 || layout.getMainInputChannelSet().size() == 2
    };

    return inputEqualsOutput && isMonoOrStereo && !layout.getMainInputChannelSet().isDisabled();
}

void SyndicateAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Send tempo and playhead information to the LFOs
    juce::AudioPlayHead::CurrentPositionInfo mTempoInfo;
    getPlayHead()->getCurrentPosition(mTempoInfo);
    for (std::shared_ptr<WECore::Richter::RichterLFO>& lfo : lfos) {
        lfo->prepareForNextBuffer(mTempoInfo.bpm, mTempoInfo.timeInSeconds);
    }

    // Advance the modulation sources
    // (the envelopes need to be done now before we overwrite the buffer)
    for (std::shared_ptr<WECore::Richter::RichterLFO>& lfo : lfos) {
        for (int sampleIndex {0}; sampleIndex < buffer.getNumSamples(); sampleIndex++) {
            lfo->getNextOutput(0);
        }
    }

    // TODO this could be faster
    for (EnvelopeFollowerWrapper& env : envelopes) {
        // Figure out which channels we need to be looking at
        int startChannel {0};
        int endChannel {0};

        if (env.useSidechainInput) {
            startChannel = getMainBusNumInputChannels();
            endChannel = totalNumInputChannels;
        } else {
            startChannel = 0;
            endChannel = getMainBusNumInputChannels();
        }

        for (int sampleIndex {0}; sampleIndex < buffer.getNumSamples(); sampleIndex++) {
            // Average the samples across all channels
            float averageSample {0};
            for (int channelIndex {startChannel}; channelIndex < endChannel; channelIndex++) {
                averageSample += buffer.getReadPointer(channelIndex)[sampleIndex];
            }
            averageSample /= totalNumInputChannels;

            env.envelope->getNextOutput(averageSample);
        }
    }


    // Pass the audio through the splitter
    {
        WECore::AudioSpinTryLock lock(pluginSplitterMutex);
        if (lock.isLocked() && pluginSplitter != nullptr) {
            SplitterProcessor::processBlock(*pluginSplitter.get(), buffer, midiMessages);
        }
    }

    // Apply the output gain
    for (int channel {0}; channel < getMainBusNumInputChannels(); channel++)
    {
        juce::FloatVectorOperations::multiply(buffer.getWritePointer(channel),
                                              _outputGainLinear,
                                              buffer.getNumSamples());
    }

    // Apply the output pan
    if (canDoStereoSplitTypes(getBusesLayout())) {
        // Stereo input - apply balance
        Utils::processBalance(outputPan->get(), buffer);
    }

    // After processing everything, update the meter envelopes
    for (int sampleIndex {0}; sampleIndex < buffer.getNumSamples(); sampleIndex++) {
        for (int channel {0}; channel < std::min(getMainBusNumInputChannels(), static_cast<int>(meterEnvelopes.size())); channel++) {
            meterEnvelopes[channel].getNextOutput(buffer.getReadPointer(channel)[sampleIndex]);
        }
    }
}

//==============================================================================
bool SyndicateAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SyndicateAudioProcessor::createEditor()
{
    return new SyndicateAudioProcessorEditor (*this);
}

float SyndicateAudioProcessor::getModulationValueForSource(int id, MODULATION_TYPE type) {
    // TODO This method may be called multiple times for each buffer, could be optimised
    const int index {id - 1};

    switch (type) {
        case MODULATION_TYPE::MACRO:
            if (index < macros.size()) {
                return macros[index]->get();
            }
            break;
        case MODULATION_TYPE::LFO:
            if (index < lfos.size()) {
                return lfos[index]->getLastOutput();
            }
            break;
        case MODULATION_TYPE::ENVELOPE:
            if (index < envelopes.size()) {
                return envelopes[index].envelope->getLastOutput() * envelopes[index].amount;
            }
            break;
    };

    return 0.0f;
}

void SyndicateAudioProcessor::addLfo() {
    std::shared_ptr<WECore::Richter::RichterLFO> newLfo {new WECore::Richter::RichterLFO()};
    newLfo->setBypassSwitch(true);
    newLfo->setSampleRate(getSampleRate());
    lfos.push_back(newLfo);
}

void SyndicateAudioProcessor::addEnvelope() {
    std::shared_ptr<WECore::AREnv::AREnvelopeFollowerSquareLaw> newEnv {new WECore::AREnv::AREnvelopeFollowerSquareLaw()};
    newEnv->setSampleRate(getSampleRate());
    envelopes.push_back({newEnv, 0, false});
}

void SyndicateAudioProcessor::removeModulationSource(ModulationSourceDefinition definition) {
    // Remove the source from the processor
    if (definition.type == MODULATION_TYPE::LFO) {
        const int index {definition.id - 1};
        if (lfos.size() > index) {
            lfos.erase(lfos.begin() + index);
        }
    } else if (definition.type == MODULATION_TYPE::ENVELOPE) {
        const int index {definition.id - 1};
        if (envelopes.size() > index) {
            envelopes.erase(envelopes.begin() + index);
        }
    }

    // Iterate through each plugin, remove the source if it has been assigned and renumber ones that
    // are numbered higher
    for (PluginChainWrapper& chain : pluginSplitter->chains) {
        for (int slotIndex {0}; slotIndex < ChainMutators::getNumSlots(chain.chain); slotIndex++) {
            PluginModulationConfig thisPluginConfig = ChainMutators::getPluginModulationConfig(chain.chain, slotIndex);

            // Iterate through each configured parameter
            for (std::shared_ptr<PluginParameterModulationConfig> parameterConfig : thisPluginConfig.parameterConfigs) {
                bool needsToDelete {false};
                int indexToDelete {0};

                // Iterate through each configured source
                for (int sourceIndex {0}; sourceIndex < parameterConfig->sources.size(); sourceIndex++) {
                    std::shared_ptr<PluginParameterModulationSource> thisSource = parameterConfig->sources[sourceIndex];

                    if (thisSource->definition == definition) {
                        // We need to come back and delete this one
                        needsToDelete = true;
                        indexToDelete = sourceIndex;
                    } else if (thisSource->definition.type == definition.type &&
                               thisSource->definition.id > definition.id) {
                        // We need to renumber this one
                        thisSource->definition.id--;
                    }
                }

                if (needsToDelete) {
                    parameterConfig->sources.erase(parameterConfig->sources.begin() + indexToDelete);
                }
            }

            ChainMutators::setPluginModulationConfig(chain.chain, thisPluginConfig, slotIndex);
        }
    }

    // Make sure any changes to assigned sources are reflected in the UI
    if (_editor != nullptr) {
        _editor->needsGraphRebuild();
    }
}

void SyndicateAudioProcessor::setSplitType(SPLIT_TYPE splitType) {
    WECore::AudioSpinLock lock(pluginSplitterMutex);

    if (!_isSplitterInitialised) {
        pluginSplitter.reset(
            new PluginSplitterSeries({getBusesLayout(), getSampleRate(), getBlockSize()},
                                     [&](int id, MODULATION_TYPE type) { return getModulationValueForSource(id, type); },
                                     [&](int newLatencySamples) { onLatencyChange(newLatencySamples); })
        );
    }

    if (splitType != _splitType || !_isSplitterInitialised) {
        if (auto multibandSplitter = std::dynamic_pointer_cast<PluginSplitterMultiband>(pluginSplitter)) {
            cachedcrossoverFrequencies = std::vector<float>();

            for (int index {0}; index < multibandSplitter->crossover.getNumBands(); index++) {
                cachedcrossoverFrequencies.value().push_back(multibandSplitter->crossover.getCrossoverFrequency(index));
            }
        }

        _splitType = splitType;
        _isSplitterInitialised = true;

        switch (splitType) {
            case SPLIT_TYPE::SERIES:
                pluginSplitter.reset(new PluginSplitterSeries(pluginSplitter));
                break;
            case SPLIT_TYPE::PARALLEL:
                pluginSplitter.reset(new PluginSplitterParallel(pluginSplitter));
                break;
            case SPLIT_TYPE::MULTIBAND:
                pluginSplitter.reset(new PluginSplitterMultiband(pluginSplitter, cachedcrossoverFrequencies));
                break;
            case SPLIT_TYPE::LEFTRIGHT:
                if (canDoStereoSplitTypes(getBusesLayout())) {
                    pluginSplitter.reset(new PluginSplitterLeftRight(pluginSplitter));
                } else {
                    juce::Logger::writeToLog("SyndicateAudioProcessor::setSplitType: Attempted to use left/right split while not in 2in2out configuration");
                    assert(false);
                }
                break;
            case SPLIT_TYPE::MIDSIDE:
                if (canDoStereoSplitTypes(getBusesLayout())) {
                    pluginSplitter.reset(new PluginSplitterMidSide(pluginSplitter));
                } else {
                    juce::Logger::writeToLog("SyndicateAudioProcessor::setSplitType: Attempted to use mid/side split while not in 2in2out configuration");
                    assert(false);
                }
                break;
        }

        // Add chain parameters if needed
        while (chainParameters.size() < SplitterMutators::getNumChains(pluginSplitter)) {
            chainParameters.emplace_back([&]() { _splitterParameters->triggerUpdate(); });
        }

        // Make sure prepareToPlay has been called on the splitter as we don't actually know if the host
        // will call it via the PluginProcessor
        if (pluginSplitter != nullptr) {
            SplitterProcessor::prepareToPlay(*pluginSplitter.get(), getSampleRate(), getBlockSize(), getBusesLayout());
        }

        // For graph state changes we need to make sure the processor has updated its state first,
        // then the UI can rebuild based on the processor state
        if (_editor != nullptr) {
            // Need to unlock the mutex first since methods called from needsGraphRebuild() will
            // need to lock it
            lock.unlock();
            _editor->needsGraphRebuild();
        }
    }
}

void SyndicateAudioProcessor::addParallelChain() {
    WECore::AudioSpinLock lock(pluginSplitterMutex);
    auto parallelSplitter = std::dynamic_pointer_cast<PluginSplitterParallel>(pluginSplitter);

    if (parallelSplitter != nullptr) {
        SplitterMutators::addChain(parallelSplitter);
        lock.unlock();
        chainParameters.emplace_back([&]() { _splitterParameters->triggerUpdate(); });

        if (_editor != nullptr) {
            _editor->needsGraphRebuild();
        }
    }
}

void SyndicateAudioProcessor::removeParallelChain(int chainNumber) {
    WECore::AudioSpinLock lock(pluginSplitterMutex);
    auto parallelSplitter = std::dynamic_pointer_cast<PluginSplitterParallel>(pluginSplitter);

    if (parallelSplitter != nullptr) {
        if (SplitterMutators::removeChain(parallelSplitter, chainNumber)) {
            lock.unlock();
            chainParameters.erase(chainParameters.begin() + chainNumber);

            if (_editor != nullptr) {
                _editor->needsGraphRebuild();
            }
        }
    }
}

void SyndicateAudioProcessor::addCrossoverBand() {
    WECore::AudioSpinLock lock(pluginSplitterMutex);
    auto multibandSplitter = std::dynamic_pointer_cast<PluginSplitterMultiband>(pluginSplitter);

    if (multibandSplitter != nullptr) {
        if (SplitterMutators::addBand(multibandSplitter)) {
            lock.unlock();
            chainParameters.emplace_back([&]() { _splitterParameters->triggerUpdate(); });

            if (_editor != nullptr) {
                _editor->needsGraphRebuild();
            }
        }
    }
}

void SyndicateAudioProcessor::removeCrossoverBand() {
    WECore::AudioSpinLock lock(pluginSplitterMutex);
    auto multibandSplitter = std::dynamic_pointer_cast<PluginSplitterMultiband>(pluginSplitter);

    if (multibandSplitter != nullptr) {
        if (SplitterMutators::removeBand(multibandSplitter)) {
            lock.unlock();
            chainParameters.erase(chainParameters.begin() + chainParameters.size() - 1);

            if (_editor != nullptr) {
                _editor->needsGraphRebuild();
            }
        }
    }
}

void SyndicateAudioProcessor::setCrossoverFrequency(size_t index, float val) {
    WECore::AudioSpinLock lock(pluginSplitterMutex);
    auto multibandSplitter = std::dynamic_pointer_cast<PluginSplitterMultiband>(pluginSplitter);

    if (multibandSplitter != nullptr) {
        if (index < SplitterMutators::getNumBands(multibandSplitter) - 1) {

            // Changing the frequency of one crossover may affect others if they also need to be
            // moved - so we set the splitter first, it will update all the frequencies internally,
            // then update the parameter
            SplitterMutators::setCrossoverFrequency(multibandSplitter, index, val);
            lock.unlock();
            _splitterParameters->triggerUpdate();
        }
    }
}

float SyndicateAudioProcessor::getCrossoverFrequency(size_t index) {
    WECore::AudioSpinLock lock(pluginSplitterMutex);
    auto multibandSplitter = std::dynamic_pointer_cast<PluginSplitterMultiband>(pluginSplitter);

    float retVal {0};

    if (multibandSplitter != nullptr) {
        if (index < SplitterMutators::getNumBands(multibandSplitter) - 1) {
            retVal = SplitterMutators::getCrossoverFrequency(multibandSplitter, index);
        }
    }

    return retVal;
}

bool SyndicateAudioProcessor::onPluginSelectedByUser(std::shared_ptr<juce::AudioPluginInstance> plugin,
                                                     int chainNumber,
                                                     int pluginNumber) {

    juce::Logger::writeToLog("SyndicateAudioProcessor::onPluginSelectedByUser: Loading plugin");

    if (pluginConfigurator.configure(plugin,
                                     {getBusesLayout(), getSampleRate(), getBlockSize()})) {
        juce::Logger::writeToLog("SyndicateAudioProcessor::onPluginSelectedByUser: Plugin configured");

        // Hand the plugin over to the splitter
        {
            WECore::AudioSpinLock lock(pluginSplitterMutex);
            SplitterMutators::replacePlugin(pluginSplitter, std::move(plugin), chainNumber, pluginNumber);
        }

        // Ideally we'd like to handle plugin selection like any other parameter - just update the
        // parameter and just action the update in the callback
        // We can't do that though as the splitters are stateful, but we still update the parameter
        // so the UI also gets the update - need to do this last though as the UI pulls its state
        // from the splitter
        if (_editor != nullptr) {
            _editor->needsGraphRebuild();
        }

        return true;
    } else {
        juce::Logger::writeToLog("SyndicateAudioProcessor::onPluginSelectedByUser: Failed to configure plugin");
        return false;
    }
}

void SyndicateAudioProcessor::removePlugin(int chainNumber, int pluginNumber) {
    juce::Logger::writeToLog("Removing slot from graph: " + juce::String(chainNumber) + " " + juce::String(pluginNumber));

    {
        WECore::AudioSpinLock lock(pluginSplitterMutex);
        if (pluginSplitter != nullptr) {
            SplitterMutators::removeSlot(pluginSplitter, chainNumber, pluginNumber);
        }
    }

    if (_editor != nullptr) {
        _editor->needsGraphRebuild();
    }
}

void SyndicateAudioProcessor::insertGainStage(int chainNumber, int pluginNumber) {
    juce::Logger::writeToLog("Inserting gain stage: " + juce::String(chainNumber) + " " + juce::String(pluginNumber));

    {
        WECore::AudioSpinLock lock(pluginSplitterMutex);
        if (pluginSplitter != nullptr) {
            SplitterMutators::insertGainStage(pluginSplitter, chainNumber, pluginNumber);
        }
    }

    if (_editor != nullptr) {
        _editor->needsGraphRebuild();
    }
}

void SyndicateAudioProcessor::copySlot(int fromChainNumber, int fromSlotNumber, int toChainNumber, int toSlotNumber) {
    std::shared_ptr<juce::AudioPluginInstance> sourcePlugin =
        SplitterMutators::getPlugin(pluginSplitter, fromChainNumber, fromSlotNumber);

    if (sourcePlugin != nullptr) {
        // This is a plugin

        // Get the state and config before making changes that might change the plugin's position
        juce::MemoryBlock sourceState;
        sourcePlugin->getStateInformation(sourceState);

        PluginModulationConfig sourceConfig =
            SplitterMutators::getPluginModulationConfig(pluginSplitter, fromChainNumber, fromSlotNumber);

        // Create the callback
        // Be careful about what is used in this callback - anything in local scope needs to be captured by value
        auto onPluginCreated = [&, sourceState, sourceConfig, toChainNumber, toSlotNumber](std::unique_ptr<juce::AudioPluginInstance> plugin, const juce::String& error) {
            if (plugin != nullptr) {
                // Create the shared pointer here as we need it for the window
                std::shared_ptr<juce::AudioPluginInstance> sharedPlugin = std::move(plugin);

                if (pluginConfigurator.configure(
                        sharedPlugin, {getBusesLayout(), getSampleRate(), getBlockSize()})) {
                    juce::Logger::writeToLog("SyndicateAudioProcessor::copySlot: Plugin configured");

                    // Hand the plugin over to the splitter
                    {
                        WECore::AudioSpinLock lock(pluginSplitterMutex);
                        SplitterMutators::insertPlugin(pluginSplitter, sharedPlugin, toChainNumber, toSlotNumber);
                    }

                    // Apply plugin state
                    sharedPlugin->setStateInformation(sourceState.getData(), sourceState.getSize());

                    // Apply modulation
                    SplitterMutators::setPluginModulationConfig(pluginSplitter, sourceConfig, toChainNumber, toSlotNumber);

                    // Ideally we'd like to handle plugin selection like any other parameter - just update the
                    // parameter and just action the update in the callback
                    // We can't do that though as the splitters are stateful, but we still update the parameter
                    // so the UI also gets the update - need to do this last though as the UI pulls its state
                    // from the splitter
                    if (_editor != nullptr) {
                        _editor->needsGraphRebuild();
                    }
                } else {
                    juce::Logger::writeToLog("SyndicateAudioProcessor::copySlot: Failed to configure plugin");
                }
            } else {
                juce::Logger::writeToLog("SyndicateAudioProcessor::copySlot: Failed to load plugin: " + error);
            }
        };

        // Try to load the plugin
        formatManager.createPluginInstanceAsync(
            sourcePlugin->getPluginDescription(),
            getSampleRate(),
            getBlockSize(),
            onPluginCreated);
    } else {
        // This is a gain stage
        const float gain {SplitterMutators::getGainLinear(pluginSplitter, fromChainNumber, fromSlotNumber)};
        const float pan {SplitterMutators::getPan(pluginSplitter, fromChainNumber, fromSlotNumber)};

        // Add it in the new position
        SplitterMutators::insertGainStage(pluginSplitter, toChainNumber, toSlotNumber);
        SplitterMutators::setGainLinear(pluginSplitter, toChainNumber, toSlotNumber, gain);
        SplitterMutators::setPan(pluginSplitter, toChainNumber, toSlotNumber, pan);

        if (_editor != nullptr) {
            _editor->needsGraphRebuild();
        }
    }
}

void SyndicateAudioProcessor::moveSlot(int fromChainNumber, int fromSlotNumber, int toChainNumber, int toSlotNumber) {
    // Copy everything we need
    std::shared_ptr<juce::AudioPluginInstance> plugin =
        SplitterMutators::getPlugin(pluginSplitter, fromChainNumber, fromSlotNumber);

    if (plugin != nullptr) {
        // This is a plugin
        PluginModulationConfig config =
            SplitterMutators::getPluginModulationConfig(pluginSplitter, fromChainNumber, fromSlotNumber);

        // Remove it from the chain
        SplitterMutators::removeSlot(pluginSplitter, fromChainNumber, fromSlotNumber);

        // Add it in the new position
        SplitterMutators::insertPlugin(pluginSplitter, plugin, toChainNumber, toSlotNumber);
        SplitterMutators::setPluginModulationConfig(pluginSplitter, config, toChainNumber, toSlotNumber);

    } else {
        // This is a gain stage
        const float gain {SplitterMutators::getGainLinear(pluginSplitter, fromChainNumber, fromSlotNumber)};
        const float pan {SplitterMutators::getPan(pluginSplitter, fromChainNumber, fromSlotNumber)};

        // Remove it from the chain
        SplitterMutators::removeSlot(pluginSplitter, fromChainNumber, fromSlotNumber);

        // Add it in the new position
        SplitterMutators::insertGainStage(pluginSplitter, toChainNumber, toSlotNumber);
        SplitterMutators::setGainLinear(pluginSplitter, toChainNumber, toSlotNumber, gain);
        SplitterMutators::setPan(pluginSplitter, toChainNumber, toSlotNumber, pan);
    }

    if (_editor != nullptr) {
        _editor->needsGraphRebuild();
    }
}

void SyndicateAudioProcessor::onLatencyChange(int newLatencySamples) {
    setLatencySamples(newLatencySamples);
}

std::vector<juce::String> SyndicateAudioProcessor::_provideParamNamesForMigration() {
    // No parameters to migrate
    return std::vector<juce::String>();
}

void SyndicateAudioProcessor::_migrateParamValues(std::vector<float>& /*paramValues*/) {
    // Do nothing - no parameters to migrate
}

void SyndicateAudioProcessor::_onParameterUpdate() {
    _outputGainLinear = WECore::CoreMath::dBToLinear(outputGainLog->get());

    // Set the bypass/mute/solo for each chain
    // TODO - try to avoid locking here
    WECore::AudioSpinLock lock(pluginSplitterMutex);
    if (pluginSplitter != nullptr) {
        switch (_splitType) {
            case SPLIT_TYPE::SERIES:
                ChainMutators::setChainBypass(pluginSplitter->chains[0].chain, chainParameters[0].getBypass());
                break;
            case SPLIT_TYPE::PARALLEL:
            case SPLIT_TYPE::MULTIBAND:
                for (size_t chainIndex {0}; chainIndex < chainParameters.size(); chainIndex++) {
                    const ChainParameters& params = chainParameters[chainIndex];

                    ChainMutators::setChainBypass(pluginSplitter->chains[chainIndex].chain, params.getBypass());
                    ChainMutators::setChainMute(pluginSplitter->chains[chainIndex].chain, params.getMute());
                    SplitterMutators::setChainSolo(pluginSplitter, chainIndex, params.getSolo());
                }
                break;
            case SPLIT_TYPE::LEFTRIGHT:
            case SPLIT_TYPE::MIDSIDE:
                ChainMutators::setChainBypass(pluginSplitter->chains[0].chain, chainParameters[0].getBypass());
                ChainMutators::setChainMute(pluginSplitter->chains[0].chain, chainParameters[0].getMute());
                SplitterMutators::setChainSolo(pluginSplitter, 0, chainParameters[0].getSolo());

                ChainMutators::setChainBypass(pluginSplitter->chains[1].chain, chainParameters[1].getBypass());
                ChainMutators::setChainMute(pluginSplitter->chains[1].chain, chainParameters[1].getMute());
                SplitterMutators::setChainSolo(pluginSplitter, 1, chainParameters[1].getSolo());
                break;
        }
    }
}

void SyndicateAudioProcessor::_resetModulationSources() {
    for (std::shared_ptr<WECore::Richter::RichterLFO>& lfo : lfos) {
        lfo->reset();
    }

    for (EnvelopeFollowerWrapper& env : envelopes) {
        env.envelope->reset();
    }
}

void SyndicateAudioProcessor::SplitterParameters::restoreFromXml(juce::XmlElement* element) {
#ifdef DEMO_BUILD
    juce::Logger::writeToLog("Not restoring state - demo build");
#else
    juce::Logger::writeToLog("Restoring plugin state from XML");

    if (_processor != nullptr) {
        juce::XmlElement* splitterElement = element->getChildByName(XML_SPLITTER_STR);
        if (splitterElement != nullptr) {
            // Restore the splitter first as we need to know how many chains there are
            _restoreSplitterFromXml(splitterElement);

            // Now make sure the chain parameters are configured
            _restoreChainParameters();
        } else {
            juce::Logger::writeToLog("Missing element " + juce::String(XML_SPLITTER_STR));
        }

        juce::XmlElement* modulationElement = element->getChildByName(XML_MODULATION_SOURCES_STR);
        if (modulationElement != nullptr) {
            // Restore the modulation source parameters
            _restoreModulationSourcesFromXml(modulationElement);
        } else {
            juce::Logger::writeToLog("Missing element " + juce::String(XML_MODULATION_SOURCES_STR));
        }

        juce::XmlElement* macroNamesElement = element->getChildByName(XML_MACRO_NAMES_STR);
        if (macroNamesElement != nullptr) {
            // Restore the macro names
            _restoreMacroNamesFromXml(macroNamesElement);
        } else {
            juce::Logger::writeToLog("Missing element " + juce::String(XML_MACRO_NAMES_STR));
        }

        juce::XmlElement* pluginSelectorElement = element->getChildByName(XML_PLUGIN_SELECTOR_STATE_STR);
        if (pluginSelectorElement != nullptr) {
            // Restore the plugin selector window state
            _processor->pluginSelectorState.restoreFromXml(pluginSelectorElement);
        } else {
            juce::Logger::writeToLog("Missing element " + juce::String(XML_PLUGIN_SELECTOR_STATE_STR));
        }

        juce::XmlElement* pluginParameterSelectorElement = element->getChildByName(XML_PLUGIN_PARAMETER_SELECTOR_STATE_STR);
        if (pluginParameterSelectorElement != nullptr) {
            // Restore the plugin parameter selector window state
            _processor->pluginParameterSelectorState.restoreFromXml(pluginParameterSelectorElement);
        } else {
            juce::Logger::writeToLog("Missing element " + juce::String(XML_PLUGIN_PARAMETER_SELECTOR_STATE_STR));
        }

        juce::XmlElement* mainWindowElement = element->getChildByName(XML_MAIN_WINDOW_STATE_STR);
        if (mainWindowElement != nullptr) {
            // Restore the main window state
            _restoreMainWindowStateFromXml(mainWindowElement);
        } else {
            juce::Logger::writeToLog("Missing element " + juce::String(XML_MAIN_WINDOW_STATE_STR));
        }
    } else {
        juce::Logger::writeToLog("Restore failed - no processor");
    }
#endif
}

void SyndicateAudioProcessor::SplitterParameters::writeToXml(juce::XmlElement* element) {
#ifdef DEMO_BUILD
    juce::Logger::writeToLog("Not writing state - demo build");
#else
    juce::Logger::writeToLog("Writing plugin state to XML");

    if (_processor != nullptr) {
        // Store the splitter
        juce::XmlElement* splitterElement = element->createNewChildElement(XML_SPLITTER_STR);
        _writeSplitterToXml(splitterElement);

        // Store the LFOs/envelopes
        juce::XmlElement* modulationElement = element->createNewChildElement(XML_MODULATION_SOURCES_STR);
        _writeModulationSourcesToXml(modulationElement);

        // Store the macro names
        juce::XmlElement* macroNamesElement = element->createNewChildElement(XML_MACRO_NAMES_STR);
        _writeMacroNamesToXml(macroNamesElement);

        // Store window states
        juce::XmlElement* mainWindowElement = element->createNewChildElement(XML_MAIN_WINDOW_STATE_STR);
        _writeMainWindowStateToXml(mainWindowElement);

        juce::XmlElement* pluginSelectorElement = element->createNewChildElement(XML_PLUGIN_SELECTOR_STATE_STR);
        _processor->pluginSelectorState.writeToXml(pluginSelectorElement);

        juce::XmlElement* pluginParameterSelectorElement = element->createNewChildElement(XML_PLUGIN_PARAMETER_SELECTOR_STATE_STR);
        _processor->pluginParameterSelectorState.writeToXml(pluginParameterSelectorElement);

    } else {
        juce::Logger::writeToLog("Writing failed - no processor");
    }
#endif
}

void SyndicateAudioProcessor::SplitterParameters::_restoreSplitterFromXml(juce::XmlElement* element) {
    // Restore the cached crossover frequencies first, we need to allow for them to be overwritten
    // by the later call to setSplitType() in the case that we're restoring a multiband split
    juce::XmlElement* frequenciesElement = element->getChildByName(XML_CACHED_CROSSOVER_FREQUENCIES_STR);
    if (frequenciesElement != nullptr) {
        _processor->cachedcrossoverFrequencies = std::vector<float>();
        const int numFrequencies {frequenciesElement->getNumAttributes()};
        for (int index {0}; index < numFrequencies; index++) {
            if (frequenciesElement->hasAttribute(getCachedCrossoverFreqXMLName(index))) {
                _processor->cachedcrossoverFrequencies.value().push_back(
                    frequenciesElement->getDoubleAttribute(getCachedCrossoverFreqXMLName(index)));
            }
        }
    }

    {
        WECore::AudioSpinLock lock(_processor->pluginSplitterMutex);

        // We need a temporary copy so that the lambda can capture it by value
        // (for some reason lambdas can only capture members by reference, but that won't work here
        // since the member will be destroyed by the time the lambda wants to use it)
        SyndicateAudioProcessor* tmpProcessor = _processor;

        _processor->pluginSplitter = XmlReader::restoreSplitterFromXml(
            element,
            [tmpProcessor](int id, MODULATION_TYPE type) { return tmpProcessor->getModulationValueForSource(id, type); },
            [tmpProcessor](int newLatencySamples) { tmpProcessor->onLatencyChange(newLatencySamples); },
            {_processor->getBusesLayout(), _processor->getSampleRate(), _processor->getBlockSize()},
            _processor->pluginConfigurator,
            [&](juce::String errorText) { _processor->restoreErrors.push_back(errorText); });
    }

    // Call this just to make sure everything is set up
    if (auto seriesSplitter = std::dynamic_pointer_cast<PluginSplitterSeries>(_processor->pluginSplitter)) {
        _processor->setSplitType(SPLIT_TYPE::SERIES);
    } else if (auto parallelSplitter = std::dynamic_pointer_cast<PluginSplitterParallel>(_processor->pluginSplitter)) {
        _processor->setSplitType(SPLIT_TYPE::PARALLEL);
    } else if (auto multibandSplitter = std::dynamic_pointer_cast<PluginSplitterMultiband>(_processor->pluginSplitter)) {
        _processor->setSplitType(SPLIT_TYPE::MULTIBAND);
    } else if (auto leftRightSplitter = std::dynamic_pointer_cast<PluginSplitterLeftRight>(_processor->pluginSplitter)) {
        _processor->setSplitType(SPLIT_TYPE::LEFTRIGHT);
    } else if (auto midSideSplitter = std::dynamic_pointer_cast<PluginSplitterMidSide>(_processor->pluginSplitter)) {
        _processor->setSplitType(SPLIT_TYPE::MIDSIDE);
    }
}

void SyndicateAudioProcessor::SplitterParameters::_restoreChainParameters() {
    while (_processor->chainParameters.size() > _processor->pluginSplitter->chains.size()) {
        // More parameters than chains, delete them
        _processor->chainParameters.erase(_processor->chainParameters.begin());
    }

    for (int chainNumber {0}; chainNumber < _processor->pluginSplitter->chains.size(); chainNumber++) {
        // Add parameters if needed
        if (_processor->chainParameters.size() <= chainNumber) {
            _processor->chainParameters.emplace_back([&]() { triggerUpdate(); });
        }

        _processor->chainParameters[chainNumber].setBypass(_processor->pluginSplitter->chains[chainNumber].chain->isChainBypassed);
        _processor->chainParameters[chainNumber].setMute(_processor->pluginSplitter->chains[chainNumber].chain->isChainMuted);
        _processor->chainParameters[chainNumber].setSolo(SplitterMutators::getChainSolo(_processor->pluginSplitter, chainNumber));
    }
}

void SyndicateAudioProcessor::SplitterParameters::_restoreModulationSourcesFromXml(juce::XmlElement* element) {
    // LFOs
    juce::XmlElement* lfosElement = element->getChildByName(XML_LFOS_STR);
    if (lfosElement != nullptr) {
        const int numLfos {lfosElement->getNumChildElements()};

        for (int index {0}; index < numLfos; index++) {
            juce::Logger::writeToLog("Restoring LFO " + juce::String(index));

            const juce::String lfoElementName = getLfoXMLName(index);
            juce::XmlElement* thisLfoElement = lfosElement->getChildByName(lfoElementName);

            if (thisLfoElement == nullptr) {
                juce::Logger::writeToLog("Failed to get element " + lfoElementName);
                continue;
            }

            std::shared_ptr<WECore::Richter::RichterLFO> newLfo {new WECore::Richter::RichterLFO()};
            newLfo->setBypassSwitch(thisLfoElement->getBoolAttribute(XML_LFO_BYPASS_STR));
            newLfo->setPhaseSyncSwitch(thisLfoElement->getBoolAttribute(XML_LFO_PHASE_SYNC_STR));
            newLfo->setTempoSyncSwitch(thisLfoElement->getBoolAttribute(XML_LFO_TEMPO_SYNC_STR));
            newLfo->setInvertSwitch(thisLfoElement->getBoolAttribute(XML_LFO_INVERT_STR));
            newLfo->setWave(thisLfoElement->getIntAttribute(XML_LFO_WAVE_STR));
            newLfo->setTempoNumer(thisLfoElement->getIntAttribute(XML_LFO_TEMPO_NUMER_STR));
            newLfo->setTempoDenom (thisLfoElement->getIntAttribute(XML_LFO_TEMPO_DENOM_STR));
            newLfo->setFreq(thisLfoElement->getDoubleAttribute(XML_LFO_FREQ_STR));
            newLfo->setFreqMod(thisLfoElement->getDoubleAttribute(XML_LFO_FREQ_MOD_STR));
            newLfo->setDepth(thisLfoElement->getDoubleAttribute(XML_LFO_DEPTH_STR));
            newLfo->setDepthMod(thisLfoElement->getDoubleAttribute(XML_LFO_DEPTH_MOD_STR));
            newLfo->setManualPhase(thisLfoElement->getDoubleAttribute(XML_LFO_MANUAL_PHASE_STR));
            // newLfo->setSampleRate(thisLfoElement->getDoubleAttribute()); // TODO make sure sample rate is set some other way after restoring

            if (_processor->lfos.size() > index) {
                // Replace an existing (default) LFO
                _processor->lfos[index] = newLfo;
            } else {
                // Add a new LFO
                _processor->lfos.push_back(newLfo);
            }
        }
    } else {
        juce::Logger::writeToLog("Missing element " + juce::String(XML_LFOS_STR));
    }

    // Envelopes
    juce::XmlElement* envelopesElement = element->getChildByName(XML_ENVELOPES_STR);
    if (envelopesElement != nullptr) {
        const int numEnvelopes {envelopesElement->getNumChildElements()};

        for (int index {0}; index < numEnvelopes; index++) {
            juce::Logger::writeToLog("Restoring envelope " + juce::String(index));

            const juce::String envelopeElementName = getEnvelopeXMLName(index);
            juce::XmlElement* thisEnvelopeElement = envelopesElement->getChildByName(envelopeElementName);

            if (thisEnvelopeElement == nullptr) {
                juce::Logger::writeToLog("Failed to get element " + envelopeElementName);
                continue;
            }

            std::shared_ptr<WECore::AREnv::AREnvelopeFollowerSquareLaw> newEnv {new WECore::AREnv::AREnvelopeFollowerSquareLaw()};
            newEnv->setAttackTimeMs(thisEnvelopeElement->getDoubleAttribute(XML_ENV_ATTACK_TIME_STR));
            newEnv->setReleaseTimeMs(thisEnvelopeElement->getDoubleAttribute(XML_ENV_RELEASE_TIME_STR));
            newEnv->setFilterEnabled(thisEnvelopeElement->getBoolAttribute(XML_ENV_FILTER_ENABLED_STR));
            newEnv->setLowCutHz(thisEnvelopeElement->getDoubleAttribute(XML_ENV_LOW_CUT_STR));
            newEnv->setHighCutHz(thisEnvelopeElement->getDoubleAttribute(XML_ENV_HIGH_CUT_STR));

            if (_processor->envelopes.size() > index) {
                // Replace an existing (default) envelope
                _processor->envelopes[index] = {newEnv, static_cast<float>(thisEnvelopeElement->getDoubleAttribute(XML_ENV_AMOUNT_STR)), false};
            } else {
                // Add a new envelope
                _processor->envelopes.push_back({newEnv, static_cast<float>(thisEnvelopeElement->getDoubleAttribute(XML_ENV_AMOUNT_STR)), false});
            }
        }
    } else {
        juce::Logger::writeToLog("Missing element " + juce::String(XML_ENVELOPES_STR));
    }
}

void SyndicateAudioProcessor::SplitterParameters::_restoreMacroNamesFromXml(juce::XmlElement* element) {
    for (int index {0}; index < _processor->macroNames.size(); index++) {
        if (element->hasAttribute(getMacroNameXMLName(index))) {
            _processor->macroNames[index] = element->getStringAttribute(getMacroNameXMLName(index));
        } else {
            juce::Logger::writeToLog("Missing macro name attribute: " + getMacroNameXMLName(index));
        }
    }
}

void SyndicateAudioProcessor::SplitterParameters::_restoreMainWindowStateFromXml(juce::XmlElement* element) {
    if (element->hasAttribute(XML_MAIN_WINDOW_BOUNDS_STR)) {
        const juce::String boundsString = element->getStringAttribute(XML_MAIN_WINDOW_BOUNDS_STR);
        _processor->mainWindowState.bounds = juce::Rectangle<int>::fromString(boundsString);
    } else {
        juce::Logger::writeToLog("Missing attribute " + juce::String(XML_MAIN_WINDOW_BOUNDS_STR));
    }

    if (element->hasAttribute(XML_GRAPH_VIEW_POSITION_STR)) {
        _processor->mainWindowState.graphViewScrollPosition = element->getIntAttribute(XML_GRAPH_VIEW_POSITION_STR);
    } else {
        juce::Logger::writeToLog("Missing attribute " + juce::String(XML_GRAPH_VIEW_POSITION_STR));
    }

    juce::XmlElement* chainScrollPositionsElement = element->getChildByName(XML_CHAIN_VIEW_POSITIONS_STR);
    if (chainScrollPositionsElement != nullptr) {
        std::vector<int> chainViewScrollPositions;

        const int numChains {chainScrollPositionsElement->getNumAttributes()};
        for (int index {0}; index < numChains; index++) {
            if (chainScrollPositionsElement->hasAttribute(getChainPositionXMLName(index))) {
                chainViewScrollPositions.push_back(chainScrollPositionsElement->getIntAttribute(getChainPositionXMLName(index)));
            }
        }

        _processor->mainWindowState.chainViewScrollPositions = chainViewScrollPositions;
    }

    if (element->hasAttribute(XML_LFO_BUTTONS_POSITION_STR)) {
        _processor->mainWindowState.lfoButtonsScrollPosition = element->getIntAttribute(XML_LFO_BUTTONS_POSITION_STR);
    } else {
        juce::Logger::writeToLog("Missing attribute " + juce::String(XML_LFO_BUTTONS_POSITION_STR));
    }

    if (element->hasAttribute(XML_ENV_BUTTONS_POSITION_STR)) {
        _processor->mainWindowState.envButtonsScrollPosition = element->getIntAttribute(XML_ENV_BUTTONS_POSITION_STR);
    } else {
        juce::Logger::writeToLog("Missing attribute " + juce::String(XML_ENV_BUTTONS_POSITION_STR));
    }
}

void SyndicateAudioProcessor::SplitterParameters::_writeSplitterToXml(juce::XmlElement* element) {
    WECore::AudioSpinLock lock(_processor->pluginSplitterMutex);
    XmlWriter::write(_processor->pluginSplitter, element);

    // Store the cached crossover frequencies
    if (_processor->cachedcrossoverFrequencies.has_value()) {
        juce::XmlElement* frequenciesElement = element->createNewChildElement(XML_CACHED_CROSSOVER_FREQUENCIES_STR);
        for (int index {0}; index < _processor->cachedcrossoverFrequencies.value().size(); index++) {
            frequenciesElement->setAttribute(getCachedCrossoverFreqXMLName(index), _processor->cachedcrossoverFrequencies.value()[index]);
        }
    }
}

void SyndicateAudioProcessor::SplitterParameters::_writeModulationSourcesToXml(juce::XmlElement* element) {
    // LFOs
    juce::XmlElement* lfosElement = element->createNewChildElement(XML_LFOS_STR);
    for (int index {0}; index < _processor->lfos.size(); index++) {
        juce::XmlElement* thisLfoElement = lfosElement->createNewChildElement(getLfoXMLName(index));
        std::shared_ptr<WECore::Richter::RichterLFO> thisLfo = _processor->lfos[index];

        thisLfoElement->setAttribute(XML_LFO_BYPASS_STR, thisLfo->getBypassSwitch());
        thisLfoElement->setAttribute(XML_LFO_PHASE_SYNC_STR, thisLfo->getPhaseSyncSwitch());
        thisLfoElement->setAttribute(XML_LFO_TEMPO_SYNC_STR, thisLfo->getTempoSyncSwitch());
        thisLfoElement->setAttribute(XML_LFO_INVERT_STR, thisLfo->getInvertSwitch());
        thisLfoElement->setAttribute(XML_LFO_WAVE_STR, thisLfo->getWave());
        thisLfoElement->setAttribute(XML_LFO_TEMPO_NUMER_STR, thisLfo->getTempoNumer());
        thisLfoElement->setAttribute(XML_LFO_TEMPO_DENOM_STR, thisLfo->getTempoDenom());
        thisLfoElement->setAttribute(XML_LFO_FREQ_STR, thisLfo->getFreq());
        thisLfoElement->setAttribute(XML_LFO_FREQ_MOD_STR, thisLfo->getFreqMod());
        thisLfoElement->setAttribute(XML_LFO_DEPTH_STR, thisLfo->getDepth());
        thisLfoElement->setAttribute(XML_LFO_DEPTH_MOD_STR, thisLfo->getDepthMod());
        thisLfoElement->setAttribute(XML_LFO_MANUAL_PHASE_STR, thisLfo->getManualPhase());
    }

    // Envelopes
    juce::XmlElement* envelopesElement = element->createNewChildElement(XML_ENVELOPES_STR);
    for (int index {0}; index < _processor->envelopes.size(); index++) {
        juce::XmlElement* thisEnvelopeElement = envelopesElement->createNewChildElement(getEnvelopeXMLName(index));
        EnvelopeFollowerWrapper thisEnvelope = _processor->envelopes[index];

        thisEnvelopeElement->setAttribute(XML_ENV_ATTACK_TIME_STR, thisEnvelope.envelope->getAttackTimeMs());
        thisEnvelopeElement->setAttribute(XML_ENV_RELEASE_TIME_STR, thisEnvelope.envelope->getReleaseTimeMs());
        thisEnvelopeElement->setAttribute(XML_ENV_FILTER_ENABLED_STR, thisEnvelope.envelope->getFilterEnabled());
        thisEnvelopeElement->setAttribute(XML_ENV_LOW_CUT_STR, thisEnvelope.envelope->getLowCutHz());
        thisEnvelopeElement->setAttribute(XML_ENV_HIGH_CUT_STR, thisEnvelope.envelope->getHighCutHz());
        thisEnvelopeElement->setAttribute(XML_ENV_AMOUNT_STR, thisEnvelope.amount);
    }
}

void SyndicateAudioProcessor::SplitterParameters::_writeMacroNamesToXml(juce::XmlElement* element) {
    for (int index {0}; index < _processor->macroNames.size(); index++) {
        element->setAttribute(getMacroNameXMLName(index), _processor->macroNames[index]);
    }
}

void SyndicateAudioProcessor::SplitterParameters::_writeMainWindowStateToXml(juce::XmlElement* element) {
    // Store the main window bounds
    element->setAttribute(XML_MAIN_WINDOW_BOUNDS_STR, _processor->mainWindowState.bounds.toString());

    // Store scroll positions
    element->setAttribute(XML_GRAPH_VIEW_POSITION_STR, _processor->mainWindowState.graphViewScrollPosition);

    juce::XmlElement* chainScrollPositionsElement = element->createNewChildElement(XML_CHAIN_VIEW_POSITIONS_STR);
    for (int index {0}; index < _processor->mainWindowState.chainViewScrollPositions.size(); index++) {
        chainScrollPositionsElement->setAttribute(
            getChainPositionXMLName(index), _processor->mainWindowState.chainViewScrollPositions[index]
        );
    }

    element->setAttribute(XML_LFO_BUTTONS_POSITION_STR, _processor->mainWindowState.lfoButtonsScrollPosition);
    element->setAttribute(XML_ENV_BUTTONS_POSITION_STR, _processor->mainWindowState.envButtonsScrollPosition);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SyndicateAudioProcessor();
}

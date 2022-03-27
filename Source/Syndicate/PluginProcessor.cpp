/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"

#include "PluginEditor.h"
#include "ParameterData.h"
#include "PluginSplitterSeries.h"
#include "PluginSplitterParallel.h"
#include "PluginSplitterMultiband.h"
#include "PluginSplitterLeftRight.h"
#include "PluginSplitterMidSide.h"
#include "AllUtils.h"
#include "PluginUtils.h"

namespace {
    // Splitter
    const char* XML_SPLITTER_STR {"Splitter"};
    const char* XML_SPLIT_TYPE_STR {"SplitType"};
    const char* XML_SPLIT_TYPE_SERIES_STR {"series"};
    const char* XML_SPLIT_TYPE_PARALLEL_STR {"parallel"};
    const char* XML_SPLIT_TYPE_MULTIBAND_STR {"multiband"};
    const char* XML_SPLIT_TYPE_LEFTRIGHT_STR {"leftright"};
    const char* XML_SPLIT_TYPE_MIDSIDE_STR {"midside"};

    const char* splitTypeToString(SPLIT_TYPE splitType) {
        switch (splitType) {
            case SPLIT_TYPE::SERIES:
                return XML_SPLIT_TYPE_SERIES_STR;
            case SPLIT_TYPE::PARALLEL:
                return XML_SPLIT_TYPE_PARALLEL_STR;
            case SPLIT_TYPE::MULTIBAND:
                return XML_SPLIT_TYPE_MULTIBAND_STR;
            case SPLIT_TYPE::LEFTRIGHT:
                return XML_SPLIT_TYPE_LEFTRIGHT_STR;
            case SPLIT_TYPE::MIDSIDE:
                return XML_SPLIT_TYPE_MIDSIDE_STR;
        }
    }

    SPLIT_TYPE stringToSplitType(juce::String splitTypeString) {
        SPLIT_TYPE retVal {SPLIT_TYPE::SERIES};

        if (splitTypeString == XML_SPLIT_TYPE_PARALLEL_STR) {
            retVal = SPLIT_TYPE::PARALLEL;
        } else if (splitTypeString == XML_SPLIT_TYPE_MULTIBAND_STR) {
            retVal = SPLIT_TYPE::MULTIBAND;
        } else if (splitTypeString == XML_SPLIT_TYPE_LEFTRIGHT_STR) {
            retVal = SPLIT_TYPE::LEFTRIGHT;
        } else if (splitTypeString == XML_SPLIT_TYPE_MIDSIDE_STR) {
            retVal = SPLIT_TYPE::MIDSIDE;
        }

        return retVal;
    }

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
        pluginSplitter->setBusesLayout(getBusesLayout());
    }

    if (pluginSplitter != nullptr) {
        pluginSplitter->prepareToPlay(sampleRate, samplesPerBlock);
    }
}

void SyndicateAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    WECore::AudioSpinLock lock(pluginSplitterMutex);
    if (pluginSplitter != nullptr) {
        pluginSplitter->releaseResources();
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
        pluginSplitter->reset();
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
            pluginSplitter->processBlock(buffer, midiMessages);
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
    if (canDoStereoSplitTypes()) {
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
    envelopes.push_back({newEnv, 0});
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

    // Iterate through each plugin, remove the source if it has been assigned an renumber ones that
    // are numbered higher
    for (PluginChainWrapper& chain : pluginSplitter->getChains()) {
        for (int slotIndex {0}; slotIndex < chain.chain->getNumSlots(); slotIndex++) {
            PluginModulationConfig thisPluginConfig = chain.chain->getPluginModulationConfig(slotIndex);

            // Iterate through each configured parameter
            for (PluginParameterModulationConfig& parameterConfig : thisPluginConfig.parameterConfigs) {
                bool needsToDelete {false};
                int indexToDelete {0};

                // Iterate through each configured source
                for (int sourceIndex {0}; sourceIndex < parameterConfig.sources.size(); sourceIndex++) {
                    PluginParameterModulationSource& thisSource = parameterConfig.sources[sourceIndex];

                    if (thisSource.definition == definition) {
                        // We need to come back and delete this one
                        needsToDelete = true;
                        indexToDelete = sourceIndex;
                    } else if (thisSource.definition.type == definition.type &&
                                thisSource.definition.id > definition.id) {
                        // We need to renumber this one
                        thisSource.definition.id--;
                    }
                }

                if (needsToDelete) {
                    parameterConfig.sources.erase(parameterConfig.sources.begin() + indexToDelete);
                }
            }

            chain.chain->setPluginModulationConfig(thisPluginConfig, slotIndex);
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
        pluginSplitter.reset(new PluginSplitterSeries([&](int id, MODULATION_TYPE type) {return getModulationValueForSource(id, type);}));
        pluginSplitter->addListener(this);
    }

    if (splitType != _splitType || !_isSplitterInitialised) {
        _splitType = splitType;
        _isSplitterInitialised = true;

        switch (splitType) {
            case SPLIT_TYPE::SERIES:
                pluginSplitter.reset(new PluginSplitterSeries(pluginSplitter->releaseChains(),
                                     [&](int id, MODULATION_TYPE type) {return getModulationValueForSource(id, type);}));
                break;
            case SPLIT_TYPE::PARALLEL:
                pluginSplitter.reset(new PluginSplitterParallel(pluginSplitter->releaseChains(),
                                     [&](int id, MODULATION_TYPE type) {return getModulationValueForSource(id, type);}));
                break;
            case SPLIT_TYPE::MULTIBAND:
                pluginSplitter.reset(new PluginSplitterMultiband(pluginSplitter->releaseChains(),
                                     [&](int id, MODULATION_TYPE type) {return getModulationValueForSource(id, type);},
                                     canDoStereoSplitTypes()));
                break;
            case SPLIT_TYPE::LEFTRIGHT:
                if (canDoStereoSplitTypes()) {
                    pluginSplitter.reset(new PluginSplitterLeftRight(pluginSplitter->releaseChains(),
                                         [&](int id, MODULATION_TYPE type) {return getModulationValueForSource(id, type);}));
                } else {
                    juce::Logger::writeToLog("SyndicateAudioProcessor::setSplitType: Attempted to use left/right split while not in 2in2out configuration");
                    assert(false);
                }
                break;
            case SPLIT_TYPE::MIDSIDE:
                if (canDoStereoSplitTypes()) {
                    pluginSplitter.reset(new PluginSplitterMidSide(pluginSplitter->releaseChains(),
                                         [&](int id, MODULATION_TYPE type) {return getModulationValueForSource(id, type);}));
                } else {
                    juce::Logger::writeToLog("SyndicateAudioProcessor::setSplitType: Attempted to use mid/side split while not in 2in2out configuration");
                    assert(false);
                }
                break;
        }

        // Add chain parameters if needed
        while (chainParameters.size() < pluginSplitter->getNumChains()) {
            chainParameters.emplace_back([&]() { _splitterParameters->triggerUpdate(); });
        }

        // Make sure prepareToPlay has been called on the splitter as we don't actually know if the host
        // will call it via the PluginProcessor
        if (pluginSplitter != nullptr) {
            pluginSplitter->prepareToPlay(getSampleRate(), getBlockSize());
        }

        pluginSplitter->addListener(this);

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
    PluginSplitterParallel* parallelSplitter = dynamic_cast<PluginSplitterParallel*>(pluginSplitter.get());

    if (parallelSplitter != nullptr) {
        if (parallelSplitter->addChain()) {
            lock.unlock();
            chainParameters.emplace_back([&]() { _splitterParameters->triggerUpdate(); });

            if (_editor != nullptr) {
                _editor->needsGraphRebuild();
            }
        }
    }
}

void SyndicateAudioProcessor::removeParallelChain(int chainNumber) {
    WECore::AudioSpinLock lock(pluginSplitterMutex);
    PluginSplitterParallel* parallelSplitter = dynamic_cast<PluginSplitterParallel*>(pluginSplitter.get());

    if (parallelSplitter != nullptr) {
        if (parallelSplitter->removeChain(chainNumber)) {
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
    PluginSplitterMultiband* multibandSplitter = dynamic_cast<PluginSplitterMultiband*>(pluginSplitter.get());

    if (multibandSplitter != nullptr) {
        if (multibandSplitter->addBand()) {
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
    PluginSplitterMultiband* multibandSplitter = dynamic_cast<PluginSplitterMultiband*>(pluginSplitter.get());

    if (multibandSplitter != nullptr) {
        if (multibandSplitter->removeBand()) {
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
    PluginSplitterMultiband* multibandSplitter = dynamic_cast<PluginSplitterMultiband*>(pluginSplitter.get());

    if (multibandSplitter != nullptr) {
        if (index < multibandSplitter->getNumBands() - 1) {

            // Changing the frequency of one crossover may affect others if they also need to be
            // moved - so we set the splitter first, it will update all the frequencies internally,
            // then update the parameter
            multibandSplitter->setCrossoverFrequency(index, val);
            lock.unlock();
            _splitterParameters->triggerUpdate();
        }
    }
}

float SyndicateAudioProcessor::getCrossoverFrequency(size_t index) {
    WECore::AudioSpinLock lock(pluginSplitterMutex);
    PluginSplitterMultiband* multibandSplitter = dynamic_cast<PluginSplitterMultiband*>(pluginSplitter.get());

    float retVal {0};

    if (multibandSplitter != nullptr) {
        if (index < multibandSplitter->getNumBands() - 1) {
            retVal = multibandSplitter->getCrossoverFrequency(index);
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
            pluginSplitter->replacePlugin(std::move(plugin), chainNumber, pluginNumber);
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
            pluginSplitter->removeSlot(chainNumber, pluginNumber);
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
            pluginSplitter->insertGainStage(chainNumber, pluginNumber, getBusesLayout());
        }
    }

    if (_editor != nullptr) {
        _editor->needsGraphRebuild();
    }
}

void SyndicateAudioProcessor::moveSlot(int fromChainNumber, int fromSlotNumber, int toChainNumber, int toSlotNumber) {
    // Copy everything we need
    std::shared_ptr<juce::AudioPluginInstance> plugin =
        pluginSplitter->getPlugin(fromChainNumber, fromSlotNumber);

    if (plugin != nullptr) {
        // This is a plugin
        PluginModulationConfig config =
            pluginSplitter->getPluginModulationConfig(fromChainNumber, fromSlotNumber);

        // Remove it from the chain
        pluginSplitter->removeSlot(fromChainNumber, fromSlotNumber);

        // Add it in the new position
        pluginSplitter->insertPlugin(plugin, toChainNumber, toSlotNumber);
        pluginSplitter->setPluginModulationConfig(config, toChainNumber, toSlotNumber);

    } else {
        // This is a gain stage
        const float gain {pluginSplitter->getGainLinear(fromChainNumber, fromSlotNumber)};
        const float pan {pluginSplitter->getPan(fromChainNumber, fromSlotNumber)};

        // Remove it from the chain
        pluginSplitter->removeSlot(fromChainNumber, fromSlotNumber);

        // Add it in the new position
        pluginSplitter->insertGainStage(toChainNumber, toSlotNumber, getBusesLayout());
        pluginSplitter->setGainLinear(toChainNumber, toSlotNumber, gain);
        pluginSplitter->setPan(toChainNumber, toSlotNumber, pan);
    }

    if (_editor != nullptr) {
        _editor->needsGraphRebuild();
    }
}

bool SyndicateAudioProcessor::canDoStereoSplitTypes() const {
    return getMainBusNumInputChannels() == getMainBusNumOutputChannels() &&
           getMainBusNumOutputChannels() == 2;
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
                pluginSplitter->getChain(0)->setChainBypass(chainParameters[0].getBypass());
                break;
            case SPLIT_TYPE::PARALLEL:
            case SPLIT_TYPE::MULTIBAND:
                for (size_t chainIndex {0}; chainIndex < chainParameters.size(); chainIndex++) {
                    const ChainParameters& params = chainParameters[chainIndex];

                    pluginSplitter->getChain(chainIndex)->setChainBypass(params.getBypass());
                    pluginSplitter->getChain(chainIndex)->setChainMute(params.getMute());
                    pluginSplitter->setChainSolo(chainIndex, params.getSolo());
                }
                break;
            case SPLIT_TYPE::LEFTRIGHT:
            case SPLIT_TYPE::MIDSIDE:
                pluginSplitter->getChain(0)->setChainBypass(chainParameters[0].getBypass());
                pluginSplitter->getChain(0)->setChainMute(chainParameters[0].getMute());
                pluginSplitter->setChainSolo(0, chainParameters[0].getSolo());

                pluginSplitter->getChain(1)->setChainBypass(chainParameters[1].getBypass());
                pluginSplitter->getChain(1)->setChainMute(chainParameters[1].getMute());
                pluginSplitter->setChainSolo(1, chainParameters[1].getSolo());
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
    } else {
        juce::Logger::writeToLog("Writing failed - no processor");
    }
#endif
}

void SyndicateAudioProcessor::SplitterParameters::_restoreSplitterFromXml(juce::XmlElement* element) {
    // Restore the split type before doing anything else
    if (element->hasAttribute(XML_SPLIT_TYPE_STR)) {
        const juce::String splitTypeString = element->getStringAttribute(XML_SPLIT_TYPE_STR);
        juce::Logger::writeToLog("Restoring split type: " + splitTypeString);

        // We need to check if the split type we're attempting to restore is supported.
        // For example in Logic when switching from a stereo to mono plugin we may have saved to XML
        // using a left/right split in a 2in2out configuration but will be restoring into a 1in1out
        // configuration.
        // In that case we move to a parallel split type.
        const SPLIT_TYPE splitType {stringToSplitType(splitTypeString)};
        const bool isExpecting2in2out {splitType == SPLIT_TYPE::LEFTRIGHT || splitType == SPLIT_TYPE::MIDSIDE};

        if (isExpecting2in2out && !_processor->canDoStereoSplitTypes()) {
            // Migrate to parallel
            _processor->setSplitType(SPLIT_TYPE::PARALLEL);
        } else {
            // Carry on as normal
            _processor->setSplitType(splitType);
        }

    } else {
        juce::Logger::writeToLog("Missing attribute " + juce::String(XML_SPLIT_TYPE_STR));
    }

    // Now actually restore the splitter
    WECore::AudioSpinLock lock(_processor->pluginSplitterMutex);
    _processor->pluginSplitter->restoreFromXml(
        element,
        {_processor->getBusesLayout(), _processor->getSampleRate(), _processor->getBlockSize()},
        _processor->pluginConfigurator,
        [&](juce::String errorText) { _processor->restoreErrors.push_back(errorText); });
    _processor->pluginSplitter->prepareToPlay(_processor->getSampleRate(), _processor->getBlockSize());
}

void SyndicateAudioProcessor::SplitterParameters::_restoreChainParameters() {
    while (_processor->chainParameters.size() > _processor->pluginSplitter->getNumChains()) {
        // More parameters than chains, delete them
        _processor->chainParameters.erase(_processor->chainParameters.begin());
    }

    for (int chainNumber {0}; chainNumber < _processor->pluginSplitter->getNumChains(); chainNumber++) {
        // Add parameters if needed
        if (_processor->chainParameters.size() <= chainNumber) {
            _processor->chainParameters.emplace_back([&]() { triggerUpdate(); });
        }

        _processor->chainParameters[chainNumber].setBypass(_processor->pluginSplitter->getChain(chainNumber)->getChainBypass());
        _processor->chainParameters[chainNumber].setMute(_processor->pluginSplitter->getChain(chainNumber)->getChainMute());
        _processor->chainParameters[chainNumber].setSolo(_processor->pluginSplitter->getChainSolo(chainNumber));
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
                _processor->envelopes[index] = {newEnv, static_cast<float>(thisEnvelopeElement->getDoubleAttribute(XML_ENV_AMOUNT_STR))};
            } else {
                // Add a new envelope
                _processor->envelopes.push_back({newEnv, static_cast<float>(thisEnvelopeElement->getDoubleAttribute(XML_ENV_AMOUNT_STR))});
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

void SyndicateAudioProcessor::SplitterParameters::_writeSplitterToXml(juce::XmlElement* element) {
    WECore::AudioSpinLock lock(_processor->pluginSplitterMutex);
    _processor->pluginSplitter->writeToXml(element);

    // We take responsibility for storing the split type here as when restoring the split type
    // again later the processor can change the splitter's type but the splitter can't do it
    // itself
    element->setAttribute(
        XML_SPLIT_TYPE_STR, splitTypeToString(_processor->pluginSplitter->getSplitType()));
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

void SyndicateAudioProcessor::_onLatencyChange() {
    if (pluginSplitter != nullptr) {
        setLatencySamples(pluginSplitter->getLatencySamples());
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SyndicateAudioProcessor();
}

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
#include "XmlReader.hpp"
#include "XmlWriter.hpp"

namespace {
    // Modulation sources
    const char* XML_MODULATION_SOURCES_STR {"ModulationSources"};

    const char* XML_MACRO_NAMES_STR {"MacroNames"};

    const char* XML_MAIN_WINDOW_STATE_STR {"MainWindowState"};
    const char* XML_MAIN_WINDOW_BOUNDS_STR {"MainEditorBounds"};
    const char* XML_GRAPH_VIEW_POSITION_STR {"GraphViewScrollPosition"};
    const char* XML_CHAIN_VIEW_POSITIONS_STR {"ChainViewScrollPositions"};
    const char* XML_LFO_BUTTONS_POSITION_STR {"LfoButtonsScrollPosition"};
    const char* XML_ENV_BUTTONS_POSITION_STR {"EnvButtonsScrollPosition"};
    const char* XML_SELECTED_SOURCE_STR {"SelectedSource"};

    juce::String getMacroNameXMLName(int macroNumber) {
        juce::String retVal("MacroName_");
        retVal += juce::String(macroNumber);
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
        splitter({getBusesLayout(), getSampleRate(), getBlockSize()},
                 [&](int id, MODULATION_TYPE type) { return getModulationValueForSource(id, type); },
                 [&](int newLatencySamples) { onLatencyChange(newLatencySamples); }),
        _logger(JucePlugin_Name, JucePlugin_VersionString, Utils::PluginLogDirectory),
        _editor(nullptr),
        _outputGainLinear(1)
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
    ModulationInterface::addLfo(modulationSources);
    ModulationInterface::addEnvelope(modulationSources);

    // Make sure everything is initialised
    _splitterParameters->setProcessor(this);
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
    ModulationInterface::prepareToPlay(modulationSources, sampleRate, samplesPerBlock, getBusesLayout());

    for (auto& env : meterEnvelopes) {
        env.setSampleRate(sampleRate);
    }

    juce::Logger::writeToLog("Setting bus layout:\n" + Utils::busesLayoutToString(getBusesLayout()));
    SplitterInterface::prepareToPlay(splitter, sampleRate, samplesPerBlock, getBusesLayout());
}

void SyndicateAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    SplitterInterface::releaseResources(splitter);
    ModulationInterface::reset(modulationSources);
}

void SyndicateAudioProcessor::reset() {
    ModulationInterface::reset(modulationSources);

    for (auto& env : meterEnvelopes) {
        env.reset();
    }

    SplitterInterface::reset(splitter);
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

    // Advance the modulation sources
    // (the envelopes need to be done now before we overwrite the buffer)
    ModulationInterface::processBlock(modulationSources, buffer, mTempoInfo);

    // Pass the audio through the splitter (this is also the only safe place to pass the playhead through)
    SplitterInterface::processBlock(splitter, buffer, midiMessages, getPlayHead());

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
    if (type == MODULATION_TYPE::MACRO) {
        const int index {id - 1};
        if (index < macros.size()) {
            return macros[index]->get();
        }
    } else if (type == MODULATION_TYPE::LFO) {
        return ModulationInterface::getLfoModulationValue(modulationSources, id);
    } else if (type == MODULATION_TYPE::ENVELOPE) {
        return ModulationInterface::getEnvelopeModulationValue(modulationSources, id);
    }

    return 0.0f;
}

void SyndicateAudioProcessor::removeModulationSource(ModulationSourceDefinition definition) {
    ModulationInterface::removeModulationSource(modulationSources, definition);
    SplitterInterface::removeModulationSource(splitter, definition);

    // Make sure any changes to assigned sources are reflected in the UI
    if (_editor != nullptr) {
        _editor->needsGraphRebuild();
    }
}

void SyndicateAudioProcessor::setSplitType(SPLIT_TYPE splitType) {
    if (SplitterInterface::setSplitType(splitter, splitType, {getBusesLayout(), getSampleRate(), getBlockSize()})) {
        // Add chain parameters if needed
        while (chainParameters.size() < SplitterInterface::getNumChains(splitter)) {
            chainParameters.emplace_back([&]() { _splitterParameters->triggerUpdate(); });
        }

        // For graph state changes we need to make sure the processor has updated its state first,
        // then the UI can rebuild based on the processor state
        if (_editor != nullptr) {
            _editor->needsGraphRebuild();
        }
    }
}

void SyndicateAudioProcessor::addParallelChain() {
    if (SplitterInterface::addParallelChain(splitter)) {
        chainParameters.emplace_back([&]() { _splitterParameters->triggerUpdate(); });

        if (_editor != nullptr) {
            _editor->needsGraphRebuild();
        }
    }
}

void SyndicateAudioProcessor::removeParallelChain(int chainNumber) {
    if (SplitterInterface::removeParallelChain(splitter, chainNumber)) {
        chainParameters.erase(chainParameters.begin() + chainNumber);

        if (_editor != nullptr) {
            _editor->needsGraphRebuild();
        }
    }
}

void SyndicateAudioProcessor::addCrossoverBand() {
    SplitterInterface::addCrossoverBand(splitter);
    chainParameters.emplace_back([&]() { _splitterParameters->triggerUpdate(); });

    if (_editor != nullptr) {
        _editor->needsGraphRebuild();
    }
}

void SyndicateAudioProcessor::removeCrossoverBand(int bandNumber) {
    if (SplitterInterface::removeCrossoverBand(splitter, bandNumber)) {
        chainParameters.erase(chainParameters.begin() + bandNumber);

        if (_editor != nullptr) {
            _editor->needsGraphRebuild();
        }
    }
}

void SyndicateAudioProcessor::setCrossoverFrequency(size_t index, float val) {
    // Changing the frequency of one crossover may affect others if they also need to be
    // moved - so we set the splitter first, it will update all the frequencies internally,
    // then update the parameter
    if (SplitterInterface::setCrossoverFrequency(splitter, index, val)) {
        _splitterParameters->triggerUpdate();
    }
}

bool SyndicateAudioProcessor::onPluginSelectedByUser(std::shared_ptr<juce::AudioPluginInstance> plugin,
                                                     int chainNumber,
                                                     int pluginNumber) {

    juce::Logger::writeToLog("SyndicateAudioProcessor::onPluginSelectedByUser: Loading plugin");

    if (pluginConfigurator.configure(plugin,
                                     {getBusesLayout(), getSampleRate(), getBlockSize()})) {
        juce::Logger::writeToLog("SyndicateAudioProcessor::onPluginSelectedByUser: Plugin configured");

        // Hand the plugin over to the splitter
        if (SplitterInterface::replacePlugin(splitter, std::move(plugin), chainNumber, pluginNumber)) {
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
            juce::Logger::writeToLog("SyndicateAudioProcessor::onPluginSelectedByUser: Failed to insert new plugin");
        }
    } else {
        juce::Logger::writeToLog("SyndicateAudioProcessor::onPluginSelectedByUser: Failed to configure plugin");
    }

    return false;
}

void SyndicateAudioProcessor::removePlugin(int chainNumber, int pluginNumber) {
    juce::Logger::writeToLog("Removing slot from graph: " + juce::String(chainNumber) + " " + juce::String(pluginNumber));

    if (SplitterInterface::removeSlot(splitter, chainNumber, pluginNumber)) {
        if (_editor != nullptr) {
            _editor->needsGraphRebuild();
        }
    }
}

void SyndicateAudioProcessor::insertGainStage(int chainNumber, int pluginNumber) {
    juce::Logger::writeToLog("Inserting gain stage: " + juce::String(chainNumber) + " " + juce::String(pluginNumber));

    if (SplitterInterface::insertGainStage(splitter, chainNumber, pluginNumber)) {
        if (_editor != nullptr) {
            _editor->needsGraphRebuild();
        }
    }
}

void SyndicateAudioProcessor::copySlot(int fromChainNumber, int fromSlotNumber, int toChainNumber, int toSlotNumber) {
    std::shared_ptr<juce::AudioPluginInstance> sourcePlugin =
        SplitterInterface::getPlugin(splitter, fromChainNumber, fromSlotNumber);

    if (sourcePlugin != nullptr) {
        // This is a plugin

        // Get the state and config before making changes that might change the plugin's position
        juce::MemoryBlock sourceState;
        sourcePlugin->getStateInformation(sourceState);

        PluginModulationConfig sourceConfig =
            SplitterInterface::getPluginModulationConfig(splitter, fromChainNumber, fromSlotNumber);

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
                    if (SplitterInterface::insertPlugin(splitter, sharedPlugin, toChainNumber, toSlotNumber)) {
                        // Apply plugin state
                        sharedPlugin->setStateInformation(sourceState.getData(), sourceState.getSize());

                        // Apply modulation
                        SplitterInterface::setPluginModulationConfig(splitter, sourceConfig, toChainNumber, toSlotNumber);

                        // Ideally we'd like to handle plugin selection like any other parameter - just update the
                        // parameter and just action the update in the callback
                        // We can't do that though as the splitters are stateful, but we still update the parameter
                        // so the UI also gets the update - need to do this last though as the UI pulls its state
                        // from the splitter
                        if (_editor != nullptr) {
                            _editor->needsGraphRebuild();
                        }
                    } else {
                        juce::Logger::writeToLog("SyndicateAudioProcessor::copySlot: Failed to insert plugin");
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
        auto [gain, pan] = SplitterInterface::getGainLinearAndPan(splitter, fromChainNumber, fromSlotNumber);

        // Add it in the new position
        if (SplitterInterface::insertGainStage(splitter, toChainNumber, toSlotNumber)) {
            SplitterInterface::setGainLinear(splitter, toChainNumber, toSlotNumber, gain);
            SplitterInterface::setPan(splitter, toChainNumber, toSlotNumber, pan);

            if (_editor != nullptr) {
                _editor->needsGraphRebuild();
            }
        }
    }
}

void SyndicateAudioProcessor::moveSlot(int fromChainNumber, int fromSlotNumber, int toChainNumber, int toSlotNumber) {
    // Copy everything we need
    std::shared_ptr<juce::AudioPluginInstance> plugin =
        SplitterInterface::getPlugin(splitter, fromChainNumber, fromSlotNumber);

    if (plugin != nullptr) {
        // This is a plugin
        PluginModulationConfig config =
            SplitterInterface::getPluginModulationConfig(splitter, fromChainNumber, fromSlotNumber);

        // Remove it from the chain
        if (SplitterInterface::removeSlot(splitter, fromChainNumber, fromSlotNumber)) {
            // Add it in the new position
            SplitterInterface::insertPlugin(splitter, plugin, toChainNumber, toSlotNumber);
            SplitterInterface::setPluginModulationConfig(splitter, config, toChainNumber, toSlotNumber);
        }
    } else {
        // This is a gain stage
        auto [gain, pan] = SplitterInterface::getGainLinearAndPan(splitter, fromChainNumber, fromSlotNumber);

        // Remove it from the chain
        if (SplitterInterface::removeSlot(splitter, fromChainNumber, fromSlotNumber)) {
            // Add it in the new position
            SplitterInterface::insertGainStage(splitter, toChainNumber, toSlotNumber);
            SplitterInterface::setGainLinear(splitter, toChainNumber, toSlotNumber, gain);
            SplitterInterface::setPan(splitter, toChainNumber, toSlotNumber, pan);
        }
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
    switch (SplitterInterface::getSplitType(splitter)) {
        case SPLIT_TYPE::SERIES:
            SplitterInterface::setChainBypass(splitter, 0, chainParameters[0].getBypass());
            break;
        case SPLIT_TYPE::PARALLEL:
        case SPLIT_TYPE::MULTIBAND:
            for (size_t chainIndex {0}; chainIndex < chainParameters.size(); chainIndex++) {
                const ChainParameters& params = chainParameters[chainIndex];

                SplitterInterface::setChainBypass(splitter, chainIndex, params.getBypass());
                SplitterInterface::setChainMute(splitter, chainIndex, params.getMute());
                SplitterInterface::setChainSolo(splitter, chainIndex, params.getSolo());
            }
            break;
        case SPLIT_TYPE::LEFTRIGHT:
        case SPLIT_TYPE::MIDSIDE:
            SplitterInterface::setChainBypass(splitter, 0, chainParameters[0].getBypass());
            SplitterInterface::setChainMute(splitter, 0, chainParameters[0].getMute());
            SplitterInterface::setChainSolo(splitter, 0, chainParameters[0].getSolo());

            SplitterInterface::setChainBypass(splitter, 1, chainParameters[1].getBypass());
            SplitterInterface::setChainMute(splitter, 1, chainParameters[1].getMute());
            SplitterInterface::setChainSolo(splitter, 1, chainParameters[1].getSolo());
            break;
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
    SyndicateAudioProcessor* tmpProcessor = _processor;

    SplitterInterface::restoreFromXml(
        _processor->splitter,
        element,
        [tmpProcessor](int id, MODULATION_TYPE type) { return tmpProcessor->getModulationValueForSource(id, type); },
        [tmpProcessor](int newLatencySamples) { tmpProcessor->onLatencyChange(newLatencySamples); },
        {_processor->getBusesLayout(), _processor->getSampleRate(), _processor->getBlockSize()},
        _processor->pluginConfigurator,
        [&](juce::String errorText) { _processor->restoreErrors.push_back(errorText); }
    );
}

void SyndicateAudioProcessor::SplitterParameters::_restoreChainParameters() {
    const size_t numChains {SplitterInterface::getNumChains(_processor->splitter)};
    while (_processor->chainParameters.size() > numChains) {
        // More parameters than chains, delete them
        _processor->chainParameters.erase(_processor->chainParameters.begin());
    }

    for (int chainNumber {0}; chainNumber < numChains; chainNumber++) {
        // Add parameters if needed
        if (_processor->chainParameters.size() <= chainNumber) {
            _processor->chainParameters.emplace_back([&]() { triggerUpdate(); });
        }

        _processor->chainParameters[chainNumber].setBypass(SplitterInterface::getChainBypass(_processor->splitter, chainNumber));
        _processor->chainParameters[chainNumber].setMute(SplitterInterface::getChainMute(_processor->splitter, chainNumber));
        _processor->chainParameters[chainNumber].setSolo(SplitterInterface::getChainSolo(_processor->splitter, chainNumber));
    }
}

void SyndicateAudioProcessor::SplitterParameters::_restoreModulationSourcesFromXml(juce::XmlElement* element) {
    ModulationInterface::restoreFromXml(
        _processor->modulationSources,
        element,
        {_processor->getBusesLayout(), _processor->getSampleRate(), _processor->getBlockSize()}
    );
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

    juce::XmlElement* selectedSourceElement = element->getChildByName(XML_SELECTED_SOURCE_STR);
    if (selectedSourceElement != nullptr) {
        _processor->mainWindowState.selectedModulationSource = ModulationSourceDefinition(1, MODULATION_TYPE::LFO);
        _processor->mainWindowState.selectedModulationSource.value().restoreFromXml(selectedSourceElement);
    } else {
        juce::Logger::writeToLog("Missing element " + juce::String(XML_SELECTED_SOURCE_STR));
    }
}

void SyndicateAudioProcessor::SplitterParameters::_writeSplitterToXml(juce::XmlElement* element) {
    SplitterInterface::writeToXml(_processor->splitter, element);
}

void SyndicateAudioProcessor::SplitterParameters::_writeModulationSourcesToXml(juce::XmlElement* element) {
    ModulationInterface::writeToXml(_processor->modulationSources, element);
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

    if (_processor->mainWindowState.selectedModulationSource.has_value()) {
        juce::XmlElement* selectedSourceElement = element->createNewChildElement(XML_SELECTED_SOURCE_STR);
        _processor->mainWindowState.selectedModulationSource.value().writeToXml(selectedSourceElement);
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SyndicateAudioProcessor();
}

/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"

#include "PluginEditor.h"
#include "PluginSplitterSeries.h"
#include "Utils.h"

//==============================================================================
SyndicateAudioProcessor::SyndicateAudioProcessor() :
        AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
                                        .withInput("Sidechain", juce::AudioChannelSet::stereo(), true)),
        _logger(JucePlugin_Name, JucePlugin_VersionString)
{
    juce::Logger::setCurrentLogger(&_logger);
    setPluginSplitType(SPLIT_TYPE::SERIES);
}

SyndicateAudioProcessor::~SyndicateAudioProcessor()
{
    // Logger must be removed before being deleted
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

    // Set the bus layout before calling prepare to play, the splitter will need the buses to be
    // correct before then
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
    if (pluginSplitter != nullptr) {
        pluginSplitter->releaseResources();
    }
}

bool SyndicateAudioProcessor::isBusesLayoutSupported(const BusesLayout& layout) const {
    return layout.getMainInputChannelSet() == layout.getMainOutputChannelSet() &&
           !layout.getMainInputChannelSet().isDisabled();
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

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }

    // Pass the audio through the splitter
    if (pluginSplitter != nullptr) {
        pluginSplitter->processBlock(buffer, midiMessages);
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

//==============================================================================
void SyndicateAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (pluginSplitter != nullptr) {
        pluginSplitter->getStateInformation(destData);
    }
}

void SyndicateAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (pluginSplitter != nullptr) {
        pluginSplitter->setStateInformation(data, sizeInBytes);
    }
}

//==============================================================================
void SyndicateAudioProcessor::onPluginSelectedByUser(std::unique_ptr<juce::AudioPluginInstance> plugin) {

    juce::Logger::writeToLog("SyndicateAudioProcessor::_onPluginSelected: Loaded plugin");

    // We need to make sure the plugin is set up properly
    plugin->setBusesLayout(getBusesLayout());
    plugin->enableAllBuses();
    plugin->setRateAndBufferSizeDetails(getSampleRate(), getBlockSize());
    plugin->prepareToPlay(getSampleRate(), getBlockSize());

    switch (pluginSplitter->getSplitType()) {
        case SPLIT_TYPE::SERIES:
            if (guestPlugin == nullptr) {
                pluginSplitter->insertPlugin(std::move(plugin), 0, 0);
            } else {
                pluginSplitter->replacePlugin(std::move(plugin), 0, 0);
            }
            guestPlugin = pluginSplitter->getPlugin(0, 0);
            break;
        case SPLIT_TYPE::PARALLEL:
            if (guestPlugin == nullptr) {
                pluginSplitter->insertPlugin(std::move(plugin), 0, 0);
            } else {
                pluginSplitter->replacePlugin(std::move(plugin), 0, 0);
            }
            guestPlugin = pluginSplitter->getPlugin(0, 0);
            break;
        case SPLIT_TYPE::MULTIBAND:
            // TODO
            break;
        case SPLIT_TYPE::LEFTRIGHT:
            // TODO
            break;
        case SPLIT_TYPE::MIDSIDE:
            // TODO
            break;
    }
}

void SyndicateAudioProcessor::setPluginSplitType(SPLIT_TYPE splitType) {

    juce::Logger::writeToLog("SyndicateAudioProcessor::setPluginSplitType");

    switch (splitType) {
        case SPLIT_TYPE::SERIES:
            if (pluginSplitter == nullptr) {
                pluginSplitter.reset(new PluginSplitterSeries());
            } else {
                pluginSplitter.reset(new PluginSplitterSeries(pluginSplitter->getChains()));
            }
            break;
        case SPLIT_TYPE::PARALLEL:
        //    pluginSplitter.reset(new PluginSplitterParallel(getBusesLayout(), std::move(chains)));
            break;
        case SPLIT_TYPE::MULTIBAND:
        //    pluginSplitter.reset(new PluginSplitterMultiband(getBusesLayout(), std::move(chains)));
            break;
        case SPLIT_TYPE::LEFTRIGHT:
            if (getMainBusNumOutputChannels() == 2) {
            //    pluginSplitter.reset(new PluginSplitterLeftRight(getBusesLayout(), std::move(chains)));
            } else {
                assert(false);
            }
            break;
        case SPLIT_TYPE::MIDSIDE:
        //    if (getMainBusNumInputChannels() == 2 && getMainBusNumOutputChannels() == 2) {
        //        pluginSplitter.reset(new PluginSplitterMidSide(getBusesLayout(), std::move(chains)));
        //    } else {
        //        assert(false);
        //    }
            break;
    }

    // Make sure prepareToPlay has been called on the splitter as we don't actually know if the host
    // will call it via the PluginProcessor
    if (pluginSplitter != nullptr) {
        pluginSplitter->prepareToPlay(getSampleRate(), getBlockSize());
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SyndicateAudioProcessor();
}

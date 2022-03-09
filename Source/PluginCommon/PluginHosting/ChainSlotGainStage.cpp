#include "ChainSlotGainStage.h"

#include "PluginUtils.h"
#include <assert.h>

namespace {
    const char* XML_SLOT_IS_BYPASSED_STR {"isSlotBypassed"};
    const char* XML_GAIN_STAGE_GAIN_STR {"Gain"};
    const char* XML_GAIN_STAGE_PAN_STR {"Pan"};
}

ChainSlotGainStage::ChainSlotGainStage(float newGain, float newPan, bool newIsBypassed, const juce::AudioProcessor::BusesLayout& busesLayout)
        : ChainSlotBase(newIsBypassed), gain(newGain), pan(newPan), _numMainChannels(busesLayout.getMainInputChannels()) {

    for (auto& env : _meterEnvelopes) {
        env.setAttackTimeMs(1);
        env.setReleaseTimeMs(50);
        env.setFilterEnabled(false);
    }
}

void ChainSlotGainStage::prepareToPlay(double sampleRate, int samplesPerBlock) {
    assert(_numMainChannels <= 2);

    for (auto& env : _meterEnvelopes) {
        env.setSampleRate(sampleRate);
    }
}

void ChainSlotGainStage::releaseResources() {
    // Do nothing
}

void ChainSlotGainStage::reset() {
    for (auto& env : _meterEnvelopes) {
        env.reset();
    }
}

void ChainSlotGainStage::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    if (!isBypassed) {
        // Apply gain
        for (int channel {0}; channel < _numMainChannels; channel++) {
            juce::FloatVectorOperations::multiply(buffer.getWritePointer(channel),
                                                  gain,
                                                  buffer.getNumSamples());
        }

        if (_numMainChannels == 2) {
            // Stereo input - apply balance
            Utils::processBalance(pan, buffer);
        }
    }

    // Update the envelope follower
    for (int sampleIndex {0}; sampleIndex < buffer.getNumSamples(); sampleIndex++) {
        for (int channel {0}; channel < _numMainChannels; channel++) {
            _meterEnvelopes[channel].getNextOutput(buffer.getReadPointer(channel)[sampleIndex]);
        }
    }
}

std::unique_ptr<ChainSlotGainStage> ChainSlotGainStage::restoreFromXml(juce::XmlElement* element, const juce::AudioProcessor::BusesLayout& busesLayout) {
    bool isSlotBypassed {false};
    if (element->hasAttribute(XML_SLOT_IS_BYPASSED_STR)) {
        isSlotBypassed = element->getBoolAttribute(XML_SLOT_IS_BYPASSED_STR);
    } else {
        juce::Logger::writeToLog("Missing attribute " + juce::String(XML_SLOT_IS_BYPASSED_STR));
    }

    float gain {1.0f};
    if (element->hasAttribute(XML_GAIN_STAGE_GAIN_STR)) {
        gain = element->getDoubleAttribute(XML_GAIN_STAGE_GAIN_STR);
    } else {
        juce::Logger::writeToLog("Missing attribute " + juce::String(XML_GAIN_STAGE_GAIN_STR));
    }

    float pan {0.0f};
    if (element->hasAttribute(XML_GAIN_STAGE_PAN_STR)) {
        pan = element->getDoubleAttribute(XML_GAIN_STAGE_PAN_STR);
    } else {
        juce::Logger::writeToLog("Missing attribute " + juce::String(XML_GAIN_STAGE_PAN_STR));
    }

    return std::make_unique<ChainSlotGainStage>(gain, pan, isSlotBypassed, busesLayout);
}

void ChainSlotGainStage::writeToXml(juce::XmlElement* element) {
    element->setAttribute(XML_SLOT_TYPE_STR, XML_SLOT_TYPE_GAIN_STAGE_STR);

    element->setAttribute(XML_SLOT_IS_BYPASSED_STR, isBypassed);
    element->setAttribute(XML_GAIN_STAGE_GAIN_STR, gain);
    element->setAttribute(XML_GAIN_STAGE_PAN_STR, pan);
}

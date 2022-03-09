#pragma once

#include <JuceHeader.h>

inline const char* XML_SLOT_TYPE_STR {"SlotType"};
inline const char* XML_SLOT_TYPE_PLUGIN_STR {"Plugin"};
inline const char* XML_SLOT_TYPE_GAIN_STAGE_STR {"GainStage"};

class ChainSlotBase {
public:
    bool isBypassed;

    explicit ChainSlotBase(bool newIsBypassed) : isBypassed(newIsBypassed) {}
    virtual ~ChainSlotBase() = default;

    virtual void prepareToPlay(double sampleRate, int samplesPerBlock) = 0;
    virtual void releaseResources() = 0;
    virtual void reset() = 0;
    virtual void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) = 0;

    virtual void writeToXml(juce::XmlElement* element) = 0;

    static bool XmlElementIsPlugin(juce::XmlElement* element);
    static bool XmlElementIsGainStage(juce::XmlElement* element);
};

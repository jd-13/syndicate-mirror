#include "ChainSlotBase.h"

bool ChainSlotBase::XmlElementIsPlugin(juce::XmlElement* element) {
    bool retVal {false};

    if (element->hasAttribute(XML_SLOT_TYPE_STR)) {
        if (element->getStringAttribute(XML_SLOT_TYPE_STR) == XML_SLOT_TYPE_PLUGIN_STR) {
            retVal = true;
        }
    }

    return retVal;
}

bool ChainSlotBase::XmlElementIsGainStage(juce::XmlElement* element) {
    bool retVal {false};

    if (element->hasAttribute(XML_SLOT_TYPE_STR)) {
        if (element->getStringAttribute(XML_SLOT_TYPE_STR) == XML_SLOT_TYPE_GAIN_STAGE_STR) {
            retVal = true;
        }
    }

    return retVal;
}

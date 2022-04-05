#pragma once

#include <JuceHeader.h>

#include <optional>

enum class MODULATION_TYPE {
    MACRO,
    LFO,
    ENVELOPE
};

class ModulationSourceDefinition {
public:
    int id;
    MODULATION_TYPE type;

    ModulationSourceDefinition(int newID, MODULATION_TYPE newType) : id(newID), type(newType) {}

    bool operator==(const ModulationSourceDefinition& other) const {
        return id == other.id && type == other.type;
    }

    /**
     * Returns an optional containing a ModulationSourceDefinition if one can be constructed from
     * the provided juce::var, otherwise empty.
     */
    static std::optional<ModulationSourceDefinition> fromVariant(juce::var variant) {
        std::optional<ModulationSourceDefinition> retVal;

        if (variant.isArray() && variant.size() == 2 && variant[0].isInt() && variant[1].isString()) {
            const int newId = variant[0];

            const juce::String typeString = variant[1].toString();
            const MODULATION_TYPE newType = _stringToModulationType(typeString);

            retVal.emplace(newId, newType);
        }

        return retVal;
    }

    juce::var toVariant() const {
        // Encode the members as items in the variant's array
        juce::var retVal;

        retVal.append(id);
        retVal.append(_modulationTypeToString(type));

        return retVal;
    }

    void restoreFromXml(juce::XmlElement* element) {
        if (element->hasAttribute(XML_MODULATION_SOURCE_ID)) {
            id = element->getIntAttribute(XML_MODULATION_SOURCE_ID);
        } else {
            juce::Logger::writeToLog("Missing attribute " + juce::String(XML_MODULATION_SOURCE_ID));
        }

        if (element->hasAttribute(XML_MODULATION_SOURCE_TYPE)) {
            type = _stringToModulationType(element->getStringAttribute(XML_MODULATION_SOURCE_TYPE));
        } else {
            juce::Logger::writeToLog("Missing attribute " + juce::String(XML_MODULATION_SOURCE_TYPE));
        }
    }

    void writeToXml(juce::XmlElement* element) {
        element->setAttribute(XML_MODULATION_SOURCE_ID, id);
        element->setAttribute(XML_MODULATION_SOURCE_TYPE, _modulationTypeToString(type));
    }

private:
    const char* XML_MODULATION_SOURCE_ID {"SourceId"};
    const char* XML_MODULATION_SOURCE_TYPE {"SourceType"};

    static juce::String _modulationTypeToString(MODULATION_TYPE type) {
        switch (type) {
            case MODULATION_TYPE::MACRO:
                return "macro";
            case MODULATION_TYPE::LFO:
                return "lfo";
            case MODULATION_TYPE::ENVELOPE:
                return "envelope";
            default:
                return "";
        }
    }

    static MODULATION_TYPE _stringToModulationType(juce::String typeString) {
        if (typeString == "macro") {
            return MODULATION_TYPE::MACRO;
        } else if (typeString == "lfo") {
            return MODULATION_TYPE::LFO;
        } else /*(typeString == "envelope")*/ {
            return MODULATION_TYPE::ENVELOPE;
        }
    }
};

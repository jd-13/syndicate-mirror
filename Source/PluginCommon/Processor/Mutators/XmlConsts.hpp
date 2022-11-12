#pragma once

inline const char* XML_SLOT_TYPE_STR {"SlotType"};
inline const char* XML_SLOT_TYPE_PLUGIN_STR {"Plugin"};
inline const char* XML_SLOT_TYPE_GAIN_STAGE_STR {"GainStage"};
inline const char* XML_SLOT_IS_BYPASSED_STR {"isSlotBypassed"};

inline const char* XML_GAIN_STAGE_GAIN_STR {"Gain"};
inline const char* XML_GAIN_STAGE_PAN_STR {"Pan"};

inline const char* XML_PLUGIN_DATA_STR {"PluginData"};
inline const char* XML_MODULATION_CONFIG_STR {"ModulationConfig"};
inline const char* XML_MODULATION_IS_ACTIVE_STR {"ModulationIsActive"};
inline const char* XML_MODULATION_TARGET_PARAMETER_NAME_STR {"TargetParameterName"};
inline const char* XML_MODULATION_REST_VALUE_STR {"RestValue"};
inline const char* XML_MODULATION_SOURCE_AMOUNT {"SourceAmount"};
inline const char* XML_PLUGIN_EDITOR_BOUNDS_STR {"PluginEditorBounds"};

inline const char* XML_MODULATION_SOURCE_ID {"SourceId"};
inline const char* XML_MODULATION_SOURCE_TYPE {"SourceType"};

inline std::string getParameterModulationConfigXmlName(int configNumber) {
    std::string retVal("ParamConfig_");
    retVal += std::to_string(configNumber);
    return retVal;
}

inline std::string getParameterModulationSourceXmlName(int sourceNumber) {
    std::string retVal("Source_");
    retVal += std::to_string(sourceNumber);
    return retVal;
}

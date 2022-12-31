#include "catch.hpp"

#include "XmlWriter.hpp"
#include "XmlConsts.hpp"
#include "TestUtils.hpp"
#include "SplitterMutators.hpp"

namespace {
    // Use values that would never be sensible defaults so we know if they were actually set
    constexpr int NUM_SAMPLES {10};
    constexpr int SAMPLE_RATE {2000};
    class XMLTestPluginInstance : public TestUtils::TestPluginInstance {
    public:
        XMLTestPluginInstance() = default;

        void getStateInformation(juce::MemoryBlock& destData) override {
            const std::string testString("testPluginData");
            destData.append(testString.c_str(), testString.size());
         }
    };
}

SCENARIO("XmlWriter: Can write PluginSplitter") {
    GIVEN("A PluginSplitterParallel with 3 chains") {
        HostConfiguration config;
        config.sampleRate = SAMPLE_RATE;
        config.blockSize = NUM_SAMPLES;

        auto modulationCallback = [](int, MODULATION_TYPE) {
            return 0.0f;
        };

        auto latencyCallback = [](int) {
            // Do nothing
        };

        auto splitterParallel = std::make_shared<PluginSplitterParallel>(config, modulationCallback, latencyCallback);

        SplitterMutators::addChain(splitterParallel);
        SplitterMutators::addChain(splitterParallel);

        auto splitter = std::dynamic_pointer_cast<PluginSplitter>(splitterParallel);

        // Chain 0
        SplitterMutators::insertPlugin(splitter, std::make_shared<XMLTestPluginInstance>(), 0, 0);

        // Chain 1
        SplitterMutators::insertGainStage(splitter, 1, 0);
        SplitterMutators::insertPlugin(splitter, std::make_shared<XMLTestPluginInstance>(), 1, 1);
        SplitterMutators::insertGainStage(splitter, 1, 2);

        // Chain 2
        SplitterMutators::insertGainStage(splitter, 2, 0);
        SplitterMutators::setChainSolo(splitter, 2, true);

        WHEN("Asked to write it to XML") {
            // WHEN("The splitter is unmodified")
            {
                juce::XmlElement e("test");
                XmlWriter::write(splitter, &e);

                // THEN("An XmlElement with the correct attributes is created")
                CHECK(e.getStringAttribute(XML_SPLIT_TYPE_STR) == XML_SPLIT_TYPE_PARALLEL_STR);

                juce::XmlElement* chainsElement = e.getChildByName(XML_CHAINS_STR);
                REQUIRE(chainsElement != nullptr);
                CHECK(chainsElement->getNumChildElements() == 3);

                juce::XmlElement* chain0Element = chainsElement->getChildByName(getChainXMLName(0));
                CHECK(chain0Element->getChildByName(XML_PLUGINS_STR)->getNumChildElements() == 1);
                CHECK(chain0Element->getBoolAttribute(XML_IS_CHAIN_SOLOED_STR) == false);

                juce::XmlElement* chain1Element = chainsElement->getChildByName(getChainXMLName(1));
                CHECK(chain1Element->getChildByName(XML_PLUGINS_STR)->getNumChildElements() == 3);
                CHECK(chain1Element->getBoolAttribute(XML_IS_CHAIN_SOLOED_STR) == false);

                juce::XmlElement* chain2Element = chainsElement->getChildByName(getChainXMLName(2));
                CHECK(chain2Element->getChildByName(XML_PLUGINS_STR)->getNumChildElements() == 1);
                CHECK(chain2Element->getBoolAttribute(XML_IS_CHAIN_SOLOED_STR) == true);
            }

            splitterParallel.reset(); // Still in scope so reset it

            // WHEN("The splitter is changed to mid/side")
            {
                auto splitterMidSide = std::make_shared<PluginSplitterMidSide>(splitter);
                splitter = std::dynamic_pointer_cast<PluginSplitter>(splitterMidSide);

                juce::XmlElement e("test");
                XmlWriter::write(splitter, &e);

                // THEN("An XmlElement with the same attributes is created apart from the type")
                CHECK(e.getStringAttribute(XML_SPLIT_TYPE_STR) == XML_SPLIT_TYPE_MIDSIDE_STR);

                juce::XmlElement* chainsElement = e.getChildByName(XML_CHAINS_STR);
                REQUIRE(chainsElement != nullptr);
                CHECK(chainsElement->getNumChildElements() == 3);

                juce::XmlElement* chain0Element = chainsElement->getChildByName(getChainXMLName(0));
                CHECK(chain0Element->getChildByName(XML_PLUGINS_STR)->getNumChildElements() == 1);
                CHECK(chain0Element->getBoolAttribute(XML_IS_CHAIN_SOLOED_STR) == false);

                juce::XmlElement* chain1Element = chainsElement->getChildByName(getChainXMLName(1));
                CHECK(chain1Element->getChildByName(XML_PLUGINS_STR)->getNumChildElements() == 3);
                CHECK(chain1Element->getBoolAttribute(XML_IS_CHAIN_SOLOED_STR) == false);

                juce::XmlElement* chain2Element = chainsElement->getChildByName(getChainXMLName(2));
                CHECK(chain2Element->getChildByName(XML_PLUGINS_STR)->getNumChildElements() == 1);
                CHECK(chain2Element->getBoolAttribute(XML_IS_CHAIN_SOLOED_STR) == true);
            }

            // WHEN("The splitter is changed to left/right")
            {
                auto splitterLeftRight = std::make_shared<PluginSplitterLeftRight>(splitter);
                splitter = std::dynamic_pointer_cast<PluginSplitter>(splitterLeftRight);

                juce::XmlElement e("test");
                XmlWriter::write(splitter, &e);

                // THEN("An XmlElement with the same attributes is created apart from the type")
                CHECK(e.getStringAttribute(XML_SPLIT_TYPE_STR) == XML_SPLIT_TYPE_LEFTRIGHT_STR);

                juce::XmlElement* chainsElement = e.getChildByName(XML_CHAINS_STR);
                REQUIRE(chainsElement != nullptr);
                CHECK(chainsElement->getNumChildElements() == 3);

                juce::XmlElement* chain0Element = chainsElement->getChildByName(getChainXMLName(0));
                CHECK(chain0Element->getChildByName(XML_PLUGINS_STR)->getNumChildElements() == 1);
                CHECK(chain0Element->getBoolAttribute(XML_IS_CHAIN_SOLOED_STR) == false);

                juce::XmlElement* chain1Element = chainsElement->getChildByName(getChainXMLName(1));
                CHECK(chain1Element->getChildByName(XML_PLUGINS_STR)->getNumChildElements() == 3);
                CHECK(chain1Element->getBoolAttribute(XML_IS_CHAIN_SOLOED_STR) == false);

                juce::XmlElement* chain2Element = chainsElement->getChildByName(getChainXMLName(2));
                CHECK(chain2Element->getChildByName(XML_PLUGINS_STR)->getNumChildElements() == 1);
                CHECK(chain2Element->getBoolAttribute(XML_IS_CHAIN_SOLOED_STR) == true);
            }

            // WHEN("The splitter is changed to multiband")
            {
                auto splitterMultiband = std::make_shared<PluginSplitterMultiband>(splitter);
                splitter = std::dynamic_pointer_cast<PluginSplitter>(splitterMultiband);

                juce::XmlElement e("test");
                XmlWriter::write(splitter, &e);

                // THEN("An XmlElement with the same attributes is created apart from the type")
                CHECK(e.getStringAttribute(XML_SPLIT_TYPE_STR) == XML_SPLIT_TYPE_MULTIBAND_STR);

                juce::XmlElement* chainsElement = e.getChildByName(XML_CHAINS_STR);
                REQUIRE(chainsElement != nullptr);
                CHECK(chainsElement->getNumChildElements() == 3);

                juce::XmlElement* chain0Element = chainsElement->getChildByName(getChainXMLName(0));
                CHECK(chain0Element->getChildByName(XML_PLUGINS_STR)->getNumChildElements() == 1);
                CHECK(chain0Element->getBoolAttribute(XML_IS_CHAIN_SOLOED_STR) == false);

                juce::XmlElement* chain1Element = chainsElement->getChildByName(getChainXMLName(1));
                CHECK(chain1Element->getChildByName(XML_PLUGINS_STR)->getNumChildElements() == 3);
                CHECK(chain1Element->getBoolAttribute(XML_IS_CHAIN_SOLOED_STR) == false);

                juce::XmlElement* chain2Element = chainsElement->getChildByName(getChainXMLName(2));
                CHECK(chain2Element->getChildByName(XML_PLUGINS_STR)->getNumChildElements() == 1);
                CHECK(chain2Element->getBoolAttribute(XML_IS_CHAIN_SOLOED_STR) == true);
            }

            // WHEN("The splitter is changed to series")
            {
                auto splitterSeries = std::make_shared<PluginSplitterSeries>(splitter);
                splitter = std::dynamic_pointer_cast<PluginSplitter>(splitterSeries);

                juce::XmlElement e("test");
                XmlWriter::write(splitter, &e);

                // THEN("An XmlElement with the same attributes is created apart from the type")
                CHECK(e.getStringAttribute(XML_SPLIT_TYPE_STR) == XML_SPLIT_TYPE_SERIES_STR);

                juce::XmlElement* chainsElement = e.getChildByName(XML_CHAINS_STR);
                REQUIRE(chainsElement != nullptr);
                CHECK(chainsElement->getNumChildElements() == 3);

                juce::XmlElement* chain0Element = chainsElement->getChildByName(getChainXMLName(0));
                CHECK(chain0Element->getChildByName(XML_PLUGINS_STR)->getNumChildElements() == 1);
                CHECK(chain0Element->getBoolAttribute(XML_IS_CHAIN_SOLOED_STR) == false);

                juce::XmlElement* chain1Element = chainsElement->getChildByName(getChainXMLName(1));
                CHECK(chain1Element->getChildByName(XML_PLUGINS_STR)->getNumChildElements() == 3);
                CHECK(chain1Element->getBoolAttribute(XML_IS_CHAIN_SOLOED_STR) == false);

                juce::XmlElement* chain2Element = chainsElement->getChildByName(getChainXMLName(2));
                CHECK(chain2Element->getChildByName(XML_PLUGINS_STR)->getNumChildElements() == 1);
                CHECK(chain2Element->getBoolAttribute(XML_IS_CHAIN_SOLOED_STR) == true);
            }
        }
    }
}

SCENARIO("XmlWriter: Can write PluginChain") {
    GIVEN("A PluginChain") {
        auto modulationCallback = [](int, MODULATION_TYPE) {
            // Return something unique we can test for later
            return 1.234f;
        };

        const auto isChainBypassed = GENERATE(false, true);
        const auto isChainMuted = GENERATE(false, true);
        const auto chainLayout = GENERATE(
            std::vector<std::string>{},
            std::vector<std::string>{"gain", "plugin"},
            std::vector<std::string>{"plugin", "gain"}
        );

        auto chain = std::make_shared<PluginChain>(modulationCallback);
        chain->isChainBypassed = isChainBypassed;
        chain->isChainMuted = isChainMuted;

        for (int slotIndex {0}; slotIndex < chainLayout.size(); slotIndex++) {
            if (chainLayout[slotIndex] == "gain") {
                juce::AudioProcessor::BusesLayout layout;
                ChainMutators::insertGainStage(chain, slotIndex, {layout, 44100, 64});

                // Add something unique so we know it actually wrote this slot
                ChainMutators::setSlotBypass(chain, slotIndex, true);
                ChainMutators::setGainLinear(chain, slotIndex, 0.1);

            } else if (chainLayout[slotIndex] == "plugin") {
                auto plugin = std::make_shared<XMLTestPluginInstance>();
                ChainMutators::insertPlugin(chain, plugin, slotIndex);
                ChainMutators::setSlotBypass(chain, slotIndex, true);
            }
        }

        WHEN("Asked to write it to XML") {
            juce::XmlElement e("test");
            XmlWriter::write(chain, &e);

            THEN("An XmlElement with the correct attributes is created") {
                CHECK(e.getBoolAttribute(XML_IS_CHAIN_BYPASSED_STR) == isChainBypassed);
                CHECK(e.getBoolAttribute(XML_IS_CHAIN_MUTED_STR) == isChainMuted);

                auto* pluginsElement = e.getChildByName(XML_PLUGINS_STR);
                REQUIRE(pluginsElement != nullptr);
                CHECK(pluginsElement->getNumChildElements() == chainLayout.size());

                for (int slotIndex {0}; slotIndex < chainLayout.size(); slotIndex++) {
                    auto* thisSlotElement = pluginsElement->getChildByName(getSlotXMLName(slotIndex));

                    if (chainLayout[slotIndex] == "gain") {
                        REQUIRE(thisSlotElement != nullptr);
                        CHECK(thisSlotElement->getStringAttribute(XML_SLOT_TYPE_STR) == XML_SLOT_TYPE_GAIN_STAGE_STR);
                        CHECK(thisSlotElement->getBoolAttribute(XML_SLOT_IS_BYPASSED_STR) == true);
                        CHECK(thisSlotElement->getDoubleAttribute(XML_GAIN_STAGE_GAIN_STR) == Approx(0.1));

                    } else if (chainLayout[slotIndex] == "plugin") {
                        REQUIRE(thisSlotElement != nullptr);
                        CHECK(thisSlotElement->getStringAttribute(XML_SLOT_TYPE_STR) == XML_SLOT_TYPE_PLUGIN_STR);
                        CHECK(thisSlotElement->getBoolAttribute(XML_SLOT_IS_BYPASSED_STR) == true);

                        auto descriptionElement = thisSlotElement->getChildByName("PLUGIN");
                        CHECK(descriptionElement->getStringAttribute("name") == "TestPlugin");

                        juce::MemoryBlock pluginState;
                        pluginState.fromBase64Encoding(thisSlotElement->getStringAttribute(XML_PLUGIN_DATA_STR));
                        std::string pluginStateStr(pluginState.begin(), pluginState.getSize());
                        CHECK(pluginStateStr == "testPluginData");
                    }
                }
            }
        }
    }
}

SCENARIO("XmlWriter: Can write ChainSlotGainStage") {
    GIVEN("A ChainSlotGainStage") {
        juce::AudioProcessor::BusesLayout layout;
        layout.inputBuses.add(juce::AudioChannelSet::mono());

        const bool isBypassed = GENERATE(true, false);

        auto gainStage = std::make_shared<ChainSlotGainStage>(0.5, 0.6, isBypassed, layout);

        WHEN("Asked to write it to XML") {
            juce::XmlElement e("test");
            XmlWriter::write(gainStage, &e);

            THEN("An XmlElement with the correct attributes is created") {
                CHECK(e.getStringAttribute(XML_SLOT_TYPE_STR) == XML_SLOT_TYPE_GAIN_STAGE_STR);
                CHECK(e.getDoubleAttribute(XML_GAIN_STAGE_GAIN_STR) == 0.5f);
                CHECK(e.getDoubleAttribute(XML_GAIN_STAGE_PAN_STR) == 0.6f);
                CHECK(e.getBoolAttribute(XML_SLOT_IS_BYPASSED_STR) == isBypassed);
            }
        }
    }
}

SCENARIO("XmlWriter: Can write ChainSlotPlugin") {
    GIVEN("A ChainSlotPlugin") {
        const bool isBypassed = GENERATE(false, true);

        std::shared_ptr<juce::AudioPluginInstance> plugin =
            std::make_shared<XMLTestPluginInstance>();

        auto config = std::make_shared<PluginModulationConfig>();
        config->parameterConfigs.push_back(std::make_shared<PluginParameterModulationConfig>());
        config->parameterConfigs[0]->targetParameterName = "testConfig1";

        auto modulationCallback = [](int, MODULATION_TYPE) { return 0.0f; };

        auto slot = std::make_shared<ChainSlotPlugin>(plugin, isBypassed, modulationCallback);
        slot->modulationConfig = config;

        const juce::Rectangle<int> bounds(150, 200);
        slot->editorBounds = std::make_shared<PluginEditorBounds>(std::optional<juce::Rectangle<int>>(bounds));

        WHEN("Asked to write it to XML") {
            juce::XmlElement e("test");
            XmlWriter::write(slot, &e);

            THEN("An XmlElement with the correct attributes is created") {
                CHECK(e.getStringAttribute(XML_SLOT_TYPE_STR) == XML_SLOT_TYPE_PLUGIN_STR);
                CHECK(e.getBoolAttribute(XML_SLOT_IS_BYPASSED_STR) == isBypassed);

                auto descriptionElement = e.getChildByName("PLUGIN");
                CHECK(descriptionElement->getStringAttribute("name") == "TestPlugin");

                juce::MemoryBlock pluginState;
                pluginState.fromBase64Encoding(e.getStringAttribute(XML_PLUGIN_DATA_STR));
                std::string pluginStateStr(pluginState.begin(), pluginState.getSize());
                CHECK(pluginStateStr == "testPluginData");

                auto configElement = e.getChildByName(XML_MODULATION_CONFIG_STR);
                auto paramConfigElement = configElement->getChildByName("ParamConfig_0");
                CHECK(paramConfigElement->getStringAttribute(XML_MODULATION_TARGET_PARAMETER_NAME_STR) == "testConfig1");

                CHECK(juce::Rectangle<int>::fromString(e.getStringAttribute(XML_PLUGIN_EDITOR_BOUNDS_STR)) == bounds);
            }
        }
    }
}

SCENARIO("XmlWriter: Can write PluginModulationConfig") {
    GIVEN("A PluginModulationConfig") {
        const bool isActive = GENERATE(false, true);

        auto config = std::make_shared<PluginModulationConfig>();
        config->isActive = isActive;
        config->parameterConfigs.push_back(std::make_shared<PluginParameterModulationConfig>());
        config->parameterConfigs[0]->targetParameterName = "testConfig1";
        config->parameterConfigs.push_back(std::make_shared<PluginParameterModulationConfig>());
        config->parameterConfigs[1]->targetParameterName = "testConfig2";

        WHEN("Asked to write it to XML") {
            juce::XmlElement e("test");
            XmlWriter::write(config, &e);

            THEN("An XmlElement with the correct attributes is created") {
                CHECK(e.getBoolAttribute(XML_MODULATION_IS_ACTIVE_STR) == isActive);

                juce::XmlElement* firstConfig = e.getChildByName("ParamConfig_0");
                CHECK(firstConfig->getStringAttribute(XML_MODULATION_TARGET_PARAMETER_NAME_STR) == "testConfig1");

                juce::XmlElement* secondSource = e.getChildByName("ParamConfig_1");
                CHECK(secondSource->getStringAttribute(XML_MODULATION_TARGET_PARAMETER_NAME_STR) == "testConfig2");
            }
        }
    }
}

SCENARIO("XmlWriter: Can write PluginParameterModulationConfig") {
    GIVEN("A PluginParameterModulationConfig") {

        typedef std::tuple<std::string, float> testData;
        auto [parameterName, restValue] = GENERATE(
            testData("parameterName", 0),
            testData(" ", 0.5),
            testData("name with spaces", 1));

        auto config = std::make_shared<PluginParameterModulationConfig>();
        config->targetParameterName = parameterName;
        config->restValue = restValue;
        config->sources.push_back(std::make_shared<PluginParameterModulationSource>(
            ModulationSourceDefinition(1, MODULATION_TYPE::LFO), -0.5));
        config->sources.push_back(std::make_shared<PluginParameterModulationSource>(
            ModulationSourceDefinition(2, MODULATION_TYPE::ENVELOPE), 0.5));

        WHEN("Asked to write it to XML") {
            juce::XmlElement e("test");
            XmlWriter::write(config, &e);

            THEN("An XmlElement with the correct attributes is created") {
                CHECK(e.getStringAttribute(XML_MODULATION_TARGET_PARAMETER_NAME_STR) == juce::String(parameterName));
                CHECK(e.getDoubleAttribute(XML_MODULATION_REST_VALUE_STR) == restValue);

                juce::XmlElement* firstSource = e.getChildByName("Source_0");
                CHECK(firstSource->getIntAttribute(XML_MODULATION_SOURCE_ID) == 1);
                CHECK(firstSource->getStringAttribute(XML_MODULATION_SOURCE_TYPE) == "lfo");
                CHECK(firstSource->getDoubleAttribute(XML_MODULATION_SOURCE_AMOUNT) == -0.5);

                juce::XmlElement* secondSource = e.getChildByName("Source_1");
                CHECK(secondSource->getIntAttribute(XML_MODULATION_SOURCE_ID) == 2);
                CHECK(secondSource->getStringAttribute(XML_MODULATION_SOURCE_TYPE) == "envelope");
                CHECK(secondSource->getDoubleAttribute(XML_MODULATION_SOURCE_AMOUNT) == 0.5);
            }
        }
    }
}

SCENARIO("XmlWriter: Can write PluginParameterModulationSource") {
    GIVEN("A PluginParameterModulationSource") {
        const double modulationAmount = GENERATE(-1, -0.5, 0, 0.5, 1);

        auto source = std::make_shared<PluginParameterModulationSource>(
            ModulationSourceDefinition(1, MODULATION_TYPE::LFO), modulationAmount);

        WHEN("Asked to write it to XML") {
            juce::XmlElement e("test");
            XmlWriter::write(source, &e);

            THEN("An XmlElement with the correct attributes is created") {
                CHECK(e.getIntAttribute(XML_MODULATION_SOURCE_ID) == 1);
                CHECK(e.getStringAttribute(XML_MODULATION_SOURCE_TYPE) == "lfo");
                CHECK(e.getDoubleAttribute(XML_MODULATION_SOURCE_AMOUNT) == modulationAmount);
            }
        }
    }
}

SCENARIO("XmlWriter: Can write ModulationSourceDefinition") {
    GIVEN("A ModulationSourceDefinition") {

        const int modulationId = GENERATE(1, 2, 3);

        typedef std::pair<MODULATION_TYPE, juce::String> testData;

        auto [modulationType, modulationTypeString] = GENERATE(
            testData(MODULATION_TYPE::MACRO, "macro"),
            testData(MODULATION_TYPE::LFO, "lfo"),
            testData(MODULATION_TYPE::ENVELOPE, "envelope")
        );

        ModulationSourceDefinition definition(modulationId, modulationType);

        WHEN("Asked to write it to XML") {
            juce::XmlElement e("test");
            definition.writeToXml(&e);

            THEN("An XmlElement with the correct attributes is created") {
                CHECK(e.getIntAttribute(XML_MODULATION_SOURCE_ID) == modulationId);
                CHECK(e.getStringAttribute(XML_MODULATION_SOURCE_TYPE) == modulationTypeString);
            }
        }
    }
}

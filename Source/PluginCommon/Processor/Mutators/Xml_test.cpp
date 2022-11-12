#include "catch.hpp"

#include "Xml.hpp"
#include "XmlConsts.hpp"
#include "TestUtils.hpp"

namespace {
    class XMLTestPluginInstance : public TestUtils::TestPluginInstance {
    public:
        juce::PluginDescription description;
        HostConfiguration config;
        std::string retrievedData;

        bool shouldSetLayoutSuccessfully;

        XMLTestPluginInstance(const juce::PluginDescription& newDescription,
                              const HostConfiguration& newConfig) : description(newDescription), config(newConfig) {
            shouldSetLayoutSuccessfully = description.name == "failLayout" ? false : true;
        }

        void getStateInformation(juce::MemoryBlock& destData) override {
            const std::string testString("testPluginData");
            destData.append(testString.c_str(), testString.size());
         }
        void setStateInformation(const void* data, int sizeInBytes) override { retrievedData = std::string(static_cast<const char*>(data), sizeInBytes); }

    protected:
        bool isBusesLayoutSupported(const BusesLayout& arr) const override { return shouldSetLayoutSuccessfully; }
    };
}

SCENARIO("XmlReader: Can determine if an XmlElement is a gain stage or a plugin") {
    GIVEN("An XmlElement that has no attributes") {
        juce::XmlElement e("test");

        WHEN("Checked as a plugin") {
            THEN("It's not a plugin") {
                CHECK_FALSE(XmlReader::XmlElementIsPlugin(&e));
            }
        }

        WHEN("Checked as a gain stage") {
            THEN("It's not a gain stage") {
                CHECK_FALSE(XmlReader::XmlElementIsGainStage(&e));
            }
        }
    }

    GIVEN("An XmlElement that has an invalid value in the slot type") {
        juce::XmlElement e("test");
        e.setAttribute(XML_SLOT_TYPE_STR, "some invalid value");

        WHEN("Checked as a plugin") {
            THEN("It's not a plugin") {
                CHECK_FALSE(XmlReader::XmlElementIsPlugin(&e));
            }
        }

        WHEN("Checked as a gain stage") {
            THEN("It's not a gain stage") {
                CHECK_FALSE(XmlReader::XmlElementIsGainStage(&e));
            }
        }
    }

    GIVEN("An XmlElement that is a gain stage") {
        juce::XmlElement e("test");
        e.setAttribute(XML_SLOT_TYPE_STR, XML_SLOT_TYPE_GAIN_STAGE_STR);

        WHEN("Checked as a plugin") {
            THEN("It's not a plugin") {
                CHECK_FALSE(XmlReader::XmlElementIsPlugin(&e));
            }
        }

        WHEN("Checked as a gain stage") {
            THEN("It's a gain stage") {
                CHECK(XmlReader::XmlElementIsGainStage(&e));
            }
        }
    }

    GIVEN("An XmlElement that is a plugin") {
        juce::XmlElement e("test");
        e.setAttribute(XML_SLOT_TYPE_STR, XML_SLOT_TYPE_PLUGIN_STR);

        WHEN("Checked as a plugin") {
            THEN("It's a plugin") {
                CHECK(XmlReader::XmlElementIsPlugin(&e));
            }
        }

        WHEN("Checked as a gain stage") {
            THEN("It's not a gain stage") {
                CHECK_FALSE(XmlReader::XmlElementIsGainStage(&e));
            }
        }
    }
}

SCENARIO("XmlReader: Can restore ChainSlotGainStage") {
    GIVEN("An XmlElement that has no attributes") {
        juce::AudioProcessor::BusesLayout layout;
        juce::XmlElement e("test");

        WHEN("Asked to restore a ChainSlotGainStage from it") {
            auto gainStage = XmlReader::restoreChainSlotGainStage(&e, layout);

            THEN("A ChainSlotGainStage with default values is created") {
                CHECK(gainStage->gain == 1.0f);
                CHECK(gainStage->pan == 0.0f);
                CHECK(gainStage->isBypassed == false);
                CHECK(gainStage->numMainChannels == 0);
            }
        }
    }

    GIVEN("An XmlElement that has attributes set correctly") {
        juce::AudioProcessor::BusesLayout layout = GENERATE(
            TestUtils::createLayoutWithInputChannels(juce::AudioChannelSet::mono()),
            TestUtils::createLayoutWithInputChannels(juce::AudioChannelSet::stereo()));

        juce::XmlElement e("test");
        e.setAttribute(XML_SLOT_TYPE_STR, XML_SLOT_TYPE_GAIN_STAGE_STR);
        e.setAttribute(XML_GAIN_STAGE_GAIN_STR, 0.5f);
        e.setAttribute(XML_GAIN_STAGE_PAN_STR, 0.6f);
        e.setAttribute(XML_SLOT_IS_BYPASSED_STR, true);

        WHEN("Asked to restore a ChainSlotGainStage from it") {
            auto gainStage = XmlReader::restoreChainSlotGainStage(&e, layout);

            THEN("A ChainSlotGainStage with the correct values is created") {
                CHECK(gainStage->gain == 0.5f);
                CHECK(gainStage->pan == 0.6f);
                CHECK(gainStage->isBypassed == true);
                CHECK(gainStage->numMainChannels == layout.getMainInputChannels());
            }
        }
    }
}

SCENARIO("XmlReader: Can restore ChainSlotPlugin") {
    auto createDefaultTestData = []() {
        return std::make_tuple<
            std::function<float(int, MODULATION_TYPE)>,
            HostConfiguration,
            PluginConfigurator,
            XmlReader::LoadPluginFunction,
            std::function<void(juce::String)>
        >(
            [](int, MODULATION_TYPE) { return 0.0f; },
            HostConfiguration{TestUtils::createLayoutWithInputChannels(juce::AudioChannelSet::mono()), 60000, 100}, // Deliberately use values the code would never default to so we can check they are actually set correctly
            PluginConfigurator(),
            [](const juce::PluginDescription& description, const HostConfiguration& config) {
                return std::make_tuple<std::unique_ptr<juce::AudioPluginInstance>, juce::String>(
                    std::make_unique<XMLTestPluginInstance>(description, config), ""
                );
            },
            [](juce::String errorMsg) { }
        );
    };

    GIVEN("An XmlElement that has no attributes") {
        juce::XmlElement e("test");

        WHEN("Asked to restore a ChainSlotPlugin from it") {
            auto [modulationCallback, config, configurator, loadPlugin, onError] =
                createDefaultTestData();

            auto slot = XmlReader::restoreChainSlotPlugin(
                &e, modulationCallback, config, configurator, loadPlugin, onError);

            THEN("A ChainSlotPlugin isn't created") {
                CHECK(slot == nullptr);
            }
        }
    }

    GIVEN("An XmlElement that has an empty description element") {
        juce::XmlElement e("test");
        auto empty = e.createNewChildElement("empty");

        WHEN("Asked to restore a ChainSlotPlugin from it") {
            auto [modulationCallback, config, configurator, loadPlugin, onError] =
                createDefaultTestData();

            auto slot = XmlReader::restoreChainSlotPlugin(
                &e, modulationCallback, config, configurator, loadPlugin, onError);

            THEN("A ChainSlotPlugin isn't created") {
                CHECK(slot == nullptr);
            }
        }
    }

    GIVEN("An XmlElement that has a valid plugin description element") {
        juce::XmlElement e("test");
        juce::PluginDescription description;
        description.name = "testPlugin";
        auto descriptionElement = description.createXml();
        e.addChildElement(descriptionElement.release());

        WHEN("Asked to restore a ChainSlotPlugin from it but loadPlugin fails") {
            auto [modulationCallback, config, configurator, loadPlugin, onError] =
                createDefaultTestData();

            loadPlugin = [](const juce::PluginDescription& description, const HostConfiguration& config) {
                return std::make_tuple<std::unique_ptr<juce::AudioPluginInstance>, juce::String>(
                    nullptr, "Test won't load plugin"
                );
            };

            onError = [](juce::String errorMsg) {
                CHECK(errorMsg == "Failed to restore plugin: Test won't load plugin");
            };

            auto slot = XmlReader::restoreChainSlotPlugin(
                &e, modulationCallback, config, configurator, loadPlugin, onError);

            THEN("A ChainSlotPlugin isn't created") {
                CHECK(slot == nullptr);
            }
        }
    }

    GIVEN("An XmlElement that has a valid plugin description element (that secretly signals to the test plugin to fail attempts to set layout)") {
        juce::XmlElement e("test");
        juce::PluginDescription description;
        description.name = "failLayout";
        auto descriptionElement = description.createXml();
        e.addChildElement(descriptionElement.release());

        WHEN("Asked to restore a ChainSlotPlugin from it but it can't find a valid layout") {
            auto [modulationCallback, config, configurator, loadPlugin, onError] =
                createDefaultTestData();

            onError = [](juce::String errorMsg) {
                CHECK(errorMsg == "Failed to restore TestPlugin as it may be a mono only plugin being restored into a stereo instance of Syndicate or vice versa");
            };

            auto slot = XmlReader::restoreChainSlotPlugin(
                &e, modulationCallback, config, configurator, loadPlugin, onError);

            THEN("A ChainSlotPlugin isn't created") {
                CHECK(slot == nullptr);
            }
        }
    }

    GIVEN("An XmlElement that has all the correct data") {
        const bool isBypassed = GENERATE(false, true);

        juce::XmlElement e("test");

        e.setAttribute(XML_SLOT_IS_BYPASSED_STR, isBypassed);

        juce::PluginDescription description;
        description.name = "TestPlugin";
        auto descriptionElement = description.createXml();
        e.addChildElement(descriptionElement.release());

        const juce::Rectangle<int> bounds(150, 200);
        e.setAttribute(XML_PLUGIN_EDITOR_BOUNDS_STR, bounds.toString());

        const std::string testString("testPluginData");
        juce::MemoryBlock block(testString.size(), true);
        block.copyFrom(testString.c_str(), 0, testString.size());
        e.setAttribute(XML_PLUGIN_DATA_STR, block.toBase64Encoding());

        auto configElement = e.createNewChildElement(XML_MODULATION_CONFIG_STR);
        configElement->setAttribute(XML_MODULATION_IS_ACTIVE_STR, true);
        auto parameterConfigElement1 = configElement->createNewChildElement("ParamConfig_0");
        parameterConfigElement1->setAttribute(XML_MODULATION_TARGET_PARAMETER_NAME_STR, "testParam1");
        auto parameterConfigElement2 = configElement->createNewChildElement("ParamConfig_1");
        parameterConfigElement2->setAttribute(XML_MODULATION_TARGET_PARAMETER_NAME_STR, "testParam2");

        WHEN("Asked to restore a ChainSlotPlugin from it") {
            auto [modulationCallback, config, configurator, loadPlugin, onError] =
                createDefaultTestData();

            auto slot = XmlReader::restoreChainSlotPlugin(
                &e, modulationCallback, config, configurator, loadPlugin, onError);

            THEN("A ChainSlotPlugin is created with the correct fields") {
                REQUIRE(slot != nullptr);
                CHECK(slot->isBypassed == isBypassed);
                CHECK(*slot->editorBounds.get() == bounds);

                auto plugin = dynamic_cast<XMLTestPluginInstance*>(slot->plugin.get());
                REQUIRE(plugin != nullptr);
                CHECK(plugin->retrievedData == testString);

                CHECK(slot->plugin->getSampleRate() == 60000);
                CHECK(slot->plugin->getBlockSize() == 100);

                CHECK(slot->modulationConfig->isActive);
                CHECK(slot->modulationConfig->parameterConfigs.size() == 2);
                CHECK(slot->modulationConfig->parameterConfigs[0]->targetParameterName == "testParam1");
                CHECK(slot->modulationConfig->parameterConfigs[1]->targetParameterName == "testParam2");
            }
        }
    }
}

SCENARIO("XmlReader: Can restore PluginModulationConfig") {
    GIVEN("An XmlElement that has no attributes") {
        juce::XmlElement e("test");

        WHEN("Asked to restore a PluginModulationConfig from it") {
            auto config = XmlReader::restorePluginModulationConfig(&e);

            THEN("A PluginModulationConfig with default values is created") {
                CHECK_FALSE(config->isActive);
                CHECK(config->parameterConfigs.size() == 0);
            }
        }
    }

    GIVEN("An XmlElement that has attributes set correctly") {
        const bool isActive = GENERATE(false, true);

        juce::XmlElement e("test");
        e.setAttribute(XML_MODULATION_IS_ACTIVE_STR, isActive);

        auto config1 = e.createNewChildElement("ParamConfig_0");
        config1->setAttribute(XML_MODULATION_TARGET_PARAMETER_NAME_STR, "testParam1");

        auto config2 = e.createNewChildElement("ParamConfig_1");
        config2->setAttribute(XML_MODULATION_TARGET_PARAMETER_NAME_STR, "testParam2");

        WHEN("Asked to restore a PluginModulationConfig from it") {
            auto config = XmlReader::restorePluginModulationConfig(&e);

            THEN("A PluginModulationConfig with the correct values is created") {
                CHECK(config->isActive == isActive);
                CHECK(config->parameterConfigs.size() == 2);
                CHECK(config->parameterConfigs[0]->targetParameterName == "testParam1");
                CHECK(config->parameterConfigs[1]->targetParameterName == "testParam2");
            }
        }
    }
}

SCENARIO("XmlReader: Can restore PluginParameterModulationConfig") {
    GIVEN("An XmlElement that has no attributes") {
        juce::XmlElement e("test");

        WHEN("Asked to restore a PluginParameterModulationConfig from it") {
            auto config = XmlReader::restorePluginParameterModulationConfig(&e);

            THEN("A PluginParameterModulationConfig with default values is created") {
                CHECK(config->targetParameterName == "");
                CHECK(config->restValue == 0);
                CHECK(config->sources.size() == 0);
            }
        }
    }

    GIVEN("An XmlElement that has attributes set correctly") {
        juce::XmlElement e("test");
        e.setAttribute(XML_MODULATION_TARGET_PARAMETER_NAME_STR, "testName");
        e.setAttribute(XML_MODULATION_REST_VALUE_STR, 0.5);

        auto source1 = e.createNewChildElement("Source_0");
        source1->setAttribute(XML_MODULATION_SOURCE_AMOUNT, -0.5);

        auto source2 = e.createNewChildElement("Source_1");
        source2->setAttribute(XML_MODULATION_SOURCE_AMOUNT, 0.5);

        WHEN("Asked to restore a PluginParameterModulationConfig from it") {
            auto config = XmlReader::restorePluginParameterModulationConfig(&e);

            THEN("A PluginParameterModulationConfig with the correct values is created") {
                CHECK(config->targetParameterName == "testName");
                CHECK(config->restValue == 0.5);
                CHECK(config->sources.size() == 2);
                CHECK(config->sources[0]->modulationAmount == -0.5f);
                CHECK(config->sources[1]->modulationAmount == 0.5f);
            }
        }
    }
}

SCENARIO("XmlReader: Can restore PluginParameterModulationSource") {
    GIVEN("An XmlElement that has no attributes") {
        juce::XmlElement e("test");

        WHEN("Asked to restore a PluginParameterModulationSource from it") {
            auto source = XmlReader::restorePluginParameterModulationSource(&e);

            THEN("A PluginParameterModulationSource with default values is created") {
                CHECK(source->definition == ModulationSourceDefinition(0, MODULATION_TYPE::MACRO));
                CHECK(source->modulationAmount == 0.0f);
            }
        }
    }

    GIVEN("An XmlElement that has attributes set correctly") {
        const float amount = GENERATE(-1, 0.5, 1);

        juce::XmlElement e("test");
        e.setAttribute(XML_MODULATION_SOURCE_ID, 1);
        e.setAttribute(XML_MODULATION_SOURCE_TYPE, "lfo");
        e.setAttribute(XML_MODULATION_SOURCE_AMOUNT, amount);

        WHEN("Asked to restore a PluginParameterModulationSource from it") {
            auto source = XmlReader::restorePluginParameterModulationSource(&e);

            THEN("A PluginParameterModulationSource with the correct values is created") {
                CHECK(source->definition == ModulationSourceDefinition(1, MODULATION_TYPE::LFO));
                CHECK(source->modulationAmount == amount);
            }
        }
    }
}

SCENARIO("XmlReader: Can restore ModulationSourceDefinition") {
    GIVEN("An XmlElement that has no attributes") {
        juce::XmlElement e("test");

        WHEN("Asked to restore a ModulationSourceDefinition from it") {
            ModulationSourceDefinition definition(0, MODULATION_TYPE::MACRO);
            definition.restoreFromXml(&e);

            THEN("Nothing is changed") {
                CHECK(definition.id == 0);
                CHECK(definition.type == MODULATION_TYPE::MACRO);
            }
        }
    }

    GIVEN("An XmlElement that has attributes set correctly") {
        juce::XmlElement e("test");

        const int modulationId = GENERATE(1, 2, 3);

        auto [modulationType, modulationTypeString] = GENERATE(
            std::pair<MODULATION_TYPE, juce::String>(MODULATION_TYPE::MACRO, "macro"),
            std::pair<MODULATION_TYPE, juce::String>(MODULATION_TYPE::LFO, "lfo"),
            std::pair<MODULATION_TYPE, juce::String>(MODULATION_TYPE::ENVELOPE, "envelope")
        );

        e.setAttribute(XML_MODULATION_SOURCE_ID, modulationId);
        e.setAttribute(XML_MODULATION_SOURCE_TYPE, modulationTypeString);

        WHEN("Asked to restore a PluginParameterModulationSource from it") {
            ModulationSourceDefinition definition(0, MODULATION_TYPE::MACRO);
            definition.restoreFromXml(&e);

            THEN("A PluginParameterModulationSource with the correct values is created") {
                CHECK(definition.id == modulationId);
                CHECK(definition.type == modulationType);
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

SCENARIO("XmlReader: Can write ChainSlotPlugin") {
    GIVEN("A ChainSlotPlugin") {
        const bool isBypassed = GENERATE(false, true);
        juce::PluginDescription description;
        description.name = "random name - will be discarded by test plugin";

        HostConfiguration hostConfig;
        std::shared_ptr<juce::AudioPluginInstance> plugin =
            std::make_shared<XMLTestPluginInstance>(description, hostConfig);

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

SCENARIO("XmlReader: Can write PluginModulationConfig") {
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

SCENARIO("XmlReader: Can write PluginParameterModulationConfig") {
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

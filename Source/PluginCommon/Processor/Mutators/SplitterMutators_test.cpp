#include "catch.hpp"
#include "TestUtils.hpp"
#include "SplitterMutators.hpp"

namespace {
    constexpr int SAMPLE_RATE {44100};
    constexpr int NUM_SAMPLES {64};

    class MutatorTestPluginInstance : public TestUtils::TestPluginInstance {
    public:
        MutatorTestPluginInstance() = default;
    };
}

SCENARIO("SplitterMutators: Chains and slots can be added, replaced, and removed") {
    auto messageManager = juce::MessageManager::getInstance();

    GIVEN("An empty splitter") {
        HostConfiguration config;
        config.sampleRate = SAMPLE_RATE;
        config.blockSize = NUM_SAMPLES;

        auto modulationCallback = [](int, MODULATION_TYPE) {
            return 0.0f;
        };

        int latencyCalled {false};
        int receivedLatency {0};
        auto latencyCallback = [&latencyCalled, &receivedLatency](int latency) {
            latencyCalled = true;
            receivedLatency = latency;
        };

        auto splitterSeries = std::make_shared<PluginSplitterSeries>(config, modulationCallback, latencyCallback);

        REQUIRE(splitterSeries->chains[0].chain->latencyListener.calculatedTotalPluginLatency == 0);

        // Catch2 resets the state if we use multiple WHEN clauses. We could use AND_WHEN, but they
        // need to be nested which makes then hard to read, so instead we use a single WHEN.
        WHEN("The splitter is modified") {
            // WHEN("A plugin is added")
            {
                auto plugin = std::make_shared<MutatorTestPluginInstance>();
                plugin->setLatencySamples(10);

                auto splitter = std::dynamic_pointer_cast<PluginSplitter>(splitterSeries);
                bool isSuccess = SplitterMutators::insertPlugin(splitter, plugin, 0, 0);

                // THEN("The splitter contains a single chain with a single plugin")
                CHECK(isSuccess);
                CHECK(splitterSeries->chains.size() == 1);
                CHECK(splitterSeries->chains[0].chain->chain.size() == 1);
                CHECK(SplitterMutators::getPlugin(splitter, 0, 0) == plugin);
                CHECK(latencyCalled);
                CHECK(receivedLatency == 10);

                // Reset
                latencyCalled = false;
                receivedLatency = 0;
            }

            // WHEN("A plugin is added at chainNumber > chains.size()")
            {
                auto plugin = std::make_shared<MutatorTestPluginInstance>();
                plugin->setLatencySamples(10);

                auto splitter = std::dynamic_pointer_cast<PluginSplitter>(splitterSeries);
                bool isSuccess = SplitterMutators::insertPlugin(splitter, plugin, 10, 0);

                // THEN("Nothing changes")
                CHECK(!isSuccess);
                CHECK(splitterSeries->chains.size() == 1);
                CHECK(splitterSeries->chains[0].chain->chain.size() == 1);
                CHECK(!latencyCalled);
                CHECK(receivedLatency == 0);

                // Reset
                latencyCalled = false;
                receivedLatency = 0;
            }

            // WHEN("A gain stage is added")
            {
                auto splitter = std::dynamic_pointer_cast<PluginSplitter>(splitterSeries);
                bool isSuccess = SplitterMutators::insertGainStage(splitter, 0, 1);

                // THEN("The splitter contains a single chain with a plugin and gain stage")
                CHECK(isSuccess);
                CHECK(splitterSeries->chains.size() == 1);
                CHECK(splitterSeries->chains[0].chain->chain.size() == 2);
                CHECK(SplitterMutators::getPlugin(splitter, 0, 1) == nullptr);
                CHECK(!latencyCalled);
                CHECK(receivedLatency == 0);

                // Reset
                latencyCalled = false;
                receivedLatency = 0;
            }

            // WHEN("A gain stage is added at chainNumber > chains.size()")
            {
                auto splitter = std::dynamic_pointer_cast<PluginSplitter>(splitterSeries);
                bool isSuccess = SplitterMutators::insertGainStage(splitter, 10, 0);

                // THEN("Nothing changes")
                CHECK(!isSuccess);
                CHECK(splitterSeries->chains.size() == 1);
                CHECK(splitterSeries->chains[0].chain->chain.size() == 2);
                CHECK(!latencyCalled);
                CHECK(receivedLatency == 0);

                // Reset
                latencyCalled = false;
                receivedLatency = 0;
            }

            // WHEN("A two chains are added")
            auto splitterParallel = std::make_shared<PluginSplitterParallel>(std::dynamic_pointer_cast<PluginSplitter>(splitterSeries));
            splitterSeries.reset();
            {
                SplitterMutators::addChain(splitterParallel);
                SplitterMutators::addChain(splitterParallel);

                // THEN("The splitter contains a chain with two slots, and two empty chains")
                CHECK(splitterParallel->chains.size() == 3);
                CHECK(splitterParallel->chains[0].chain->chain.size() == 2);
                CHECK(splitterParallel->chains[1].chain->chain.size() == 0);
                CHECK(splitterParallel->chains[2].chain->chain.size() == 0);
                CHECK(latencyCalled);
                CHECK(receivedLatency == 10);

                // Reset
                latencyCalled = false;
                receivedLatency = 0;
            }

            // WHEN("A plugin is replaced in the middle chain")
            {
                auto plugin = std::make_shared<MutatorTestPluginInstance>();
                plugin->setLatencySamples(15);

                auto splitter = std::dynamic_pointer_cast<PluginSplitter>(splitterParallel);
                bool isSuccess = SplitterMutators::replacePlugin(splitter, plugin, 1, 0);

                // THEN("The plugin is added to the middle chain")
                CHECK(isSuccess);
                CHECK(splitterParallel->chains.size() == 3);
                CHECK(splitterParallel->chains[0].chain->chain.size() == 2);
                CHECK(splitterParallel->chains[1].chain->chain.size() == 1);
                CHECK(splitterParallel->chains[2].chain->chain.size() == 0);
                CHECK(latencyCalled);
                CHECK(receivedLatency == 15);

                // Reset
                latencyCalled = false;
                receivedLatency = 0;
            }

            // WHEN("A plugin changes latency")
            {
                auto splitter = std::dynamic_pointer_cast<PluginSplitter>(splitterParallel);
                auto plugin = SplitterMutators::getPlugin(splitter, 1, 0);
                plugin->setLatencySamples(30);

                // Allow the latency message to be sent
                messageManager->runDispatchLoopUntil(10);

                // THEN("The latency is updated")
                CHECK(latencyCalled);
                CHECK(receivedLatency == 30);

                // Reset
                latencyCalled = false;
                receivedLatency = 0;
            }

            // WHEN("A gain stage is removed in the first chain")
            {
                auto splitter = std::dynamic_pointer_cast<PluginSplitter>(splitterParallel);
                bool isSuccess = SplitterMutators::removeSlot(splitter, 0, 1);

                // THEN("The first chain has only one plugin left")
                CHECK(isSuccess);
                CHECK(splitterParallel->chains.size() == 3);
                CHECK(splitterParallel->chains[0].chain->chain.size() == 1);
                CHECK(splitterParallel->chains[1].chain->chain.size() == 1);
                CHECK(splitterParallel->chains[2].chain->chain.size() == 0);
                CHECK(latencyCalled);
                CHECK(receivedLatency == 30);

                // Reset
                latencyCalled = false;
                receivedLatency = 0;
            }

            // WHEN("A plugin is removed in the middle chain")
            {
                auto splitter = std::dynamic_pointer_cast<PluginSplitter>(splitterParallel);
                bool isSuccess = SplitterMutators::removeSlot(splitter, 1, 0);

                // THEN("The middle chain is empty")
                CHECK(isSuccess);
                CHECK(splitterParallel->chains.size() == 3);
                CHECK(splitterParallel->chains[0].chain->chain.size() == 1);
                CHECK(splitterParallel->chains[1].chain->chain.size() == 0);
                CHECK(splitterParallel->chains[2].chain->chain.size() == 0);
                CHECK(latencyCalled);
                CHECK(receivedLatency == 10);

                // Reset
                latencyCalled = false;
                receivedLatency = 0;
            }

            // WHEN("A chain with a gain stage is removed")
            {
                auto splitter = std::dynamic_pointer_cast<PluginSplitter>(splitterParallel);
                bool isSuccess = SplitterMutators::insertGainStage(splitter, 1, 0);

                REQUIRE(isSuccess);
                REQUIRE(splitterParallel->chains.size() == 3);
                REQUIRE(splitterParallel->chains[1].chain->chain.size() == 1);

                // Reset
                latencyCalled = false;
                receivedLatency = 0;

                isSuccess = SplitterMutators::removeChain(splitterParallel, 1);

                // THEN("The correct chain is removed")
                CHECK(isSuccess);
                CHECK(splitterParallel->chains.size() == 2);
                CHECK(splitterParallel->chains[0].chain->chain.size() == 1);
                CHECK(splitterParallel->chains[1].chain->chain.size() == 0);
                CHECK(latencyCalled);
                CHECK(receivedLatency == 10);

                // Reset
                latencyCalled = false;
                receivedLatency = 0;
            }

            // WHEN("A chain is removed at chainNumber > chains.size()")
            {
                bool isSuccess = SplitterMutators::removeChain(splitterParallel, 10);

                // THEN("The nothing changes")
                CHECK(!isSuccess);
                CHECK(splitterParallel->chains.size() == 2);
                CHECK(!latencyCalled);
                CHECK(receivedLatency == 0);

                // Reset
                latencyCalled = false;
                receivedLatency = 0;
            }

            WHEN("A too many chains are removed")
            {
                bool isSuccess = SplitterMutators::removeChain(splitterParallel, 1);

                REQUIRE(isSuccess);
                REQUIRE(splitterParallel->chains.size() == 1);

                // Reset
                latencyCalled = false;
                receivedLatency = 0;

                isSuccess = SplitterMutators::removeChain(splitterParallel, 0);

                // THEN("The nothing changes")
                CHECK(!isSuccess);
                CHECK(splitterParallel->chains.size() == 1);
                CHECK(splitterParallel->chains[0].chain->chain.size() == 1);
                CHECK(!latencyCalled);
                CHECK(receivedLatency == 0);

                // Reset
                latencyCalled = false;
                receivedLatency = 0;
            }
        }
    }

    juce::MessageManager::deleteInstance();
}

SCENARIO("SplitterMutators: Modulation config can be set and retrieved") {
    GIVEN("A parallel splitter with two chains") {
        HostConfiguration config;
        config.sampleRate = SAMPLE_RATE;
        config.blockSize = NUM_SAMPLES;

        auto modulationCallback = [](int, MODULATION_TYPE) {
            // Return something unique we can test for later
            return 1.234f;
        };

        bool latencyCalled {false};
        int receivedLatency {0};
        auto latencyCallback = [&latencyCalled, &receivedLatency](int latency) {
            latencyCalled = true;
            receivedLatency = latency;
        };

        auto splitterParallel = std::make_shared<PluginSplitterParallel>(config, modulationCallback, latencyCallback);
        SplitterMutators::addChain(splitterParallel);

        auto splitter = std::dynamic_pointer_cast<PluginSplitter>(splitterParallel);
        SplitterMutators::insertGainStage(splitter, 0, 0);
        SplitterMutators::insertPlugin(splitter, std::make_shared<MutatorTestPluginInstance>(), 0, 1);
        SplitterMutators::insertPlugin(splitter, std::make_shared<MutatorTestPluginInstance>(), 1, 0);

        WHEN("The config is set for a plugin") {
            PluginModulationConfig config;
            config.isActive = true;

            auto paramConfig = std::make_shared<PluginParameterModulationConfig>();
            paramConfig->targetParameterName = "testParam";
            config.parameterConfigs.push_back(paramConfig);

            const bool success {SplitterMutators::setPluginModulationConfig(splitter, config, 1, 0)};

            THEN("The new config can be retrieved") {
                CHECK(success);

                const PluginModulationConfig retrievedConfig {SplitterMutators::getPluginModulationConfig(splitter, 1, 0)};
                CHECK(retrievedConfig.isActive);
                CHECK(retrievedConfig.parameterConfigs[0]->targetParameterName == config.parameterConfigs[0]->targetParameterName);
            }
         }

        WHEN("The config is set for a gain stage") {
            PluginModulationConfig config;
            config.isActive = true;

            auto paramConfig = std::make_shared<PluginParameterModulationConfig>();
            paramConfig->targetParameterName = "testParam";
            config.parameterConfigs.push_back(paramConfig);

            const bool success {SplitterMutators::setPluginModulationConfig(splitter, config, 0, 0)};

            THEN("The configs haven't changed") {
                CHECK(!success);

                for (int chainIndex {0}; chainIndex < splitter->chains.size(); chainIndex++) {
                    for (int slotIndex {0}; slotIndex < splitter->chains[chainIndex].chain->chain.size(); slotIndex++) {
                        const PluginModulationConfig retrievedConfig {SplitterMutators::getPluginModulationConfig(splitter, chainIndex, slotIndex)};
                        CHECK(!retrievedConfig.isActive);
                        CHECK(retrievedConfig.parameterConfigs.size() == 0);
                    }
                }
            }
        }
    }
}

SCENARIO("SplitterMutators: Slot parameters can be modified and retrieved") {
    auto messageManager = juce::MessageManager::getInstance();

    GIVEN("A splitter with two chains") {
        HostConfiguration config;
        config.sampleRate = SAMPLE_RATE;
        config.blockSize = NUM_SAMPLES;

        auto modulationCallback = [](int, MODULATION_TYPE) {
            // Return something unique we can test for later
            return 1.234f;
        };

        bool latencyCalled {false};
        int receivedLatency {0};
        auto latencyCallback = [&latencyCalled, &receivedLatency](int latency) {
            latencyCalled = true;
            receivedLatency = latency;
        };

        const juce::String splitTypeString = GENERATE(
            // We don't need series
            juce::String(XML_SPLIT_TYPE_PARALLEL_STR),
            juce::String(XML_SPLIT_TYPE_MULTIBAND_STR),
            juce::String(XML_SPLIT_TYPE_LEFTRIGHT_STR),
            juce::String(XML_SPLIT_TYPE_MIDSIDE_STR)
        );

        std::shared_ptr<PluginSplitter> splitter;

        if (splitTypeString == XML_SPLIT_TYPE_PARALLEL_STR) {
            auto splitterParallel = std::make_shared<PluginSplitterParallel>(config, modulationCallback, latencyCallback);
            SplitterMutators::addChain(splitterParallel);
            splitter = std::dynamic_pointer_cast<PluginSplitter>(splitterParallel);
        } else if (splitTypeString == XML_SPLIT_TYPE_MULTIBAND_STR) {
            auto splitterMultiband = std::make_shared<PluginSplitterMultiband>(config, modulationCallback, latencyCallback);
            splitter = std::dynamic_pointer_cast<PluginSplitter>(splitterMultiband);
        } else if (splitTypeString == XML_SPLIT_TYPE_LEFTRIGHT_STR) {
            auto splitterLeftRight = std::make_shared<PluginSplitterLeftRight>(config, modulationCallback, latencyCallback);
            splitter = std::dynamic_pointer_cast<PluginSplitter>(splitterLeftRight);
        } else if (splitTypeString == XML_SPLIT_TYPE_MIDSIDE_STR) {
            auto splitterMidSide = std::make_shared<PluginSplitterMidSide>(config, modulationCallback, latencyCallback);
            splitter = std::dynamic_pointer_cast<PluginSplitter>(splitterMidSide);
        }

        REQUIRE(splitter->chains.size() == 2);
        REQUIRE(splitter->chains[0].chain->latencyListener.calculatedTotalPluginLatency == 0);
        REQUIRE(splitter->chains[1].chain->latencyListener.calculatedTotalPluginLatency == 0);


        SplitterMutators::insertGainStage(splitter, 0, 0);

        {
            auto plugin = std::make_shared<MutatorTestPluginInstance>();
            plugin->setLatencySamples(20);
            SplitterMutators::insertPlugin(splitter, plugin, 0, 1);
        }

        {
            auto plugin = std::make_shared<MutatorTestPluginInstance>();
            plugin->setLatencySamples(15);
            SplitterMutators::insertPlugin(splitter, plugin, 1, 0);
        }

        REQUIRE(receivedLatency == 20);

        // Reset
        latencyCalled = false;
        receivedLatency = 0;

        WHEN("A plugin is bypassed") {
            SplitterMutators::setSlotBypass(splitter, 0, 1, true);

            // Allow the latency message to be sent
            messageManager->runDispatchLoopUntil(10);

            THEN("The plugin is bypassed correctly") {
                CHECK(!SplitterMutators::getSlotBypass(splitter, 0, 0));
                CHECK(SplitterMutators::getSlotBypass(splitter, 0, 1));
                CHECK(!SplitterMutators::getSlotBypass(splitter, 1, 0));
                CHECK(latencyCalled);
                CHECK(receivedLatency == 15);
            }
        }

        WHEN("A gain stage is bypassed") {
            SplitterMutators::setSlotBypass(splitter, 0, 0, true);

            // Allow the latency message to be sent (though we don't expect one for a gain stage change)
            messageManager->runDispatchLoopUntil(10);

            THEN("The gain stage is bypassed correctly") {
                CHECK(SplitterMutators::getSlotBypass(splitter, 0, 0));
                CHECK(!SplitterMutators::getSlotBypass(splitter, 0, 1));
                CHECK(!SplitterMutators::getSlotBypass(splitter, 1, 0));
                CHECK(latencyCalled);
                CHECK(receivedLatency == 20);
            }
        }

        WHEN("An out of bounds slot is bypassed") {
            SplitterMutators::setSlotBypass(splitter, 10, 0, true);

            // Allow the latency message to be sent (though we don't expect one here)
            messageManager->runDispatchLoopUntil(10);

            THEN("Nothing is bypassed") {
                CHECK(!SplitterMutators::getSlotBypass(splitter, 0, 0));
                CHECK(!SplitterMutators::getSlotBypass(splitter, 0, 1));
                CHECK(!SplitterMutators::getSlotBypass(splitter, 1, 0));
                CHECK(!latencyCalled);
                CHECK(receivedLatency == 0);
            }
        }

        WHEN("A chain is soloed") {
            SplitterMutators::setChainSolo(splitter, 0, true);

            // Allow the latency message to be sent
            messageManager->runDispatchLoopUntil(10);

            THEN("The chain is soloed correctly") {
                CHECK(SplitterMutators::getChainSolo(splitter, 0));
                CHECK(!SplitterMutators::getChainSolo(splitter, 1));
                CHECK(!latencyCalled);
                CHECK(receivedLatency == 0);

                if (splitTypeString == XML_SPLIT_TYPE_MULTIBAND_STR) {
                    // Make sure the crossover state matches
                    auto splitterMultiband = std::dynamic_pointer_cast<PluginSplitterMultiband>(splitter);
                    REQUIRE(splitterMultiband != nullptr);
                    CHECK(splitterMultiband->crossover.getIsSoloed(0));
                    CHECK(!splitterMultiband->crossover.getIsSoloed(1));
                }
            }
        }

        WHEN("An out of bounds chain is soloed") {
            SplitterMutators::setChainSolo(splitter, 10, true);

            // Allow the latency message to be sent
            messageManager->runDispatchLoopUntil(10);

            THEN("Nothing is soloed") {
                CHECK(!SplitterMutators::getChainSolo(splitter, 0));
                CHECK(!SplitterMutators::getChainSolo(splitter, 1));
                CHECK(!latencyCalled);
                CHECK(receivedLatency == 0);
            }
        }

        WHEN("Gain and pan is set for a plugin slot") {
            const bool gainSuccess {SplitterMutators::setGainLinear(splitter, 0, 1, 0.5)};
            const bool panSuccess {SplitterMutators::setPan(splitter, 0, 1, -0.5)};

            THEN("Nothing is changed") {
                CHECK(!gainSuccess);
                CHECK(!panSuccess);
                CHECK(SplitterMutators::getGainLinear(splitter, 0, 1) == 0.0f);
                CHECK(SplitterMutators::getPan(splitter, 0, 1) == 0.0f);
            }
        }

        WHEN("Gain and pan is set for a gain stage slot") {
            const bool gainSuccess {SplitterMutators::setGainLinear(splitter, 0, 0, 0.5)};
            const bool panSuccess {SplitterMutators::setPan(splitter, 0, 0, -0.5)};

            THEN("The gain stage is bypassed correctly") {
                CHECK(gainSuccess);
                CHECK(panSuccess);
                CHECK(SplitterMutators::getGainLinear(splitter, 0, 0) == 0.5f);
                CHECK(SplitterMutators::getPan(splitter, 0, 0) == -0.5f);
            }
        }

        WHEN("Gain and pan is set for an out of bounds chain") {
            const bool gainSuccess {SplitterMutators::setGainLinear(splitter, 10, 0, 0.5)};
            const bool panSuccess {SplitterMutators::setPan(splitter, 10, 0, -0.5)};

            THEN("Nothing is bypassed") {
                CHECK(!gainSuccess);
                CHECK(!panSuccess);
            }
        }

        if (splitTypeString == XML_SPLIT_TYPE_MULTIBAND_STR) {
            // Make sure the crossover state matches
            auto splitterMultiband = std::dynamic_pointer_cast<PluginSplitterMultiband>(splitter);
            REQUIRE(splitterMultiband != nullptr);

            WHEN("Crossover frequency is set") {
                SplitterMutators::setCrossoverFrequency(splitterMultiband, 0, 2500);

                THEN("the crossover frequency is set correctly") {
                    CHECK(SplitterMutators::getCrossoverFrequency(splitterMultiband, 0) == 2500);
                }
            }

            WHEN("Crossover frequency is set for an out of bounds crossover") {
                SplitterMutators::setCrossoverFrequency(splitterMultiband, 4, 4000);

                THEN("The default value is unchanged") {
                    CHECK(SplitterMutators::getCrossoverFrequency(splitterMultiband, 0) == 100);
                }
            }
        }
    }

    juce::MessageManager::deleteInstance();
}


SCENARIO("SplitterMutators: PluginEditorBounds can be retrieved") {
    GIVEN("A parallel splitter with two chains") {
        HostConfiguration config;
        config.sampleRate = SAMPLE_RATE;
        config.blockSize = NUM_SAMPLES;

        auto modulationCallback = [](int, MODULATION_TYPE) {
            // Return something unique we can test for later
            return 1.234f;
        };

        bool latencyCalled {false};
        int receivedLatency {0};
        auto latencyCallback = [&latencyCalled, &receivedLatency](int latency) {
            latencyCalled = true;
            receivedLatency = latency;
        };

        auto splitterParallel = std::make_shared<PluginSplitterParallel>(config, modulationCallback, latencyCallback);
        SplitterMutators::addChain(splitterParallel);
        REQUIRE(splitterParallel->chains[0].chain->latencyListener.calculatedTotalPluginLatency == 0);
        REQUIRE(splitterParallel->chains[1].chain->latencyListener.calculatedTotalPluginLatency == 0);

        auto splitter = std::dynamic_pointer_cast<PluginSplitter>(splitterParallel);
        SplitterMutators::insertGainStage(splitter, 0, 0);
        SplitterMutators::insertPlugin(splitter, std::make_shared<MutatorTestPluginInstance>(), 1, 0);

        auto& pluginBounds = std::dynamic_pointer_cast<ChainSlotPlugin>(splitter->chains[1].chain->chain[0])->editorBounds;
        pluginBounds.reset(new PluginEditorBounds());
        *(pluginBounds.get()) = PluginEditorBoundsContainer(
            juce::Rectangle<int>(150, 200),
            juce::Rectangle<int>(2000, 1000));

        WHEN("Editor bounds are retrieved for a plugin") {
            auto bounds = SplitterMutators::getPluginEditorBounds(splitter, 1, 0);

            THEN("Bounds are retrieved correctly") {
                CHECK(bounds->value().editorBounds == juce::Rectangle<int>(150, 200));
                CHECK(bounds->value().displayArea == juce::Rectangle<int>(2000, 1000));
            }
        }

        WHEN("Editor bounds are retrieved for a gain stage") {
            auto bounds = SplitterMutators::getPluginEditorBounds(splitter, 0, 0);

            THEN("Bounds aren't retrieved") {
                CHECK(!bounds->has_value());
            }
        }

        WHEN("Editor bounds are retrieved for an out of bounds slot") {
            auto bounds = SplitterMutators::getPluginEditorBounds(splitter, 10, 0);

            THEN("Bounds aren't retrieved") {
                CHECK(!bounds->has_value());
            }
        }
    }
}

#include "catch.hpp"

#include "TestUtils.hpp"

namespace {
    class ConfigTestPluginInstance : public TestUtils::TestPluginInstance {
    public:
        std::vector<BusesLayout> acceptedLayouts;
        bool isPrepared;

        ConfigTestPluginInstance() : isPrepared(false) {}

        void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override {
            TestUtils::TestPluginInstance::prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
            isPrepared = true;
        }

    protected:
        bool isBusesLayoutSupported(const BusesLayout& arr) const override {
            for (const BusesLayout& layout : acceptedLayouts) {
                if (layout.getMainInputChannels() == arr.getMainInputChannels()) {
                    return true;
                }
            }

            return false;
        }
    };

    std::vector<juce::AudioProcessor::BusesLayout> getAcceptedLayouts(std::string layoutName) {
        std::vector<juce::AudioProcessor::BusesLayout> retVal;

        if (layoutName == "mono") {
            retVal.push_back(TestUtils::createLayoutWithInputChannels(juce::AudioChannelSet::mono()));
        } else if (layoutName == "stereo") {
            retVal.push_back(TestUtils::createLayoutWithInputChannels(juce::AudioChannelSet::stereo()));
        } else if (layoutName == "monostereo") {
            retVal.push_back(TestUtils::createLayoutWithInputChannels(juce::AudioChannelSet::mono()));
            retVal.push_back(TestUtils::createLayoutWithInputChannels(juce::AudioChannelSet::stereo()));
        }

        return retVal;
    };

    HostConfiguration getHostConfig(std::string name) {
        HostConfiguration config;

        if (name == "mono") {
            config.layout.inputBuses.add(juce::AudioChannelSet::mono());
            config.layout.outputBuses.add(juce::AudioChannelSet::mono());
            config.sampleRate = 80000;
            config.blockSize = 100;
        } else if (name == "stereo") {
            config.layout.inputBuses.add(juce::AudioChannelSet::stereo());
            config.layout.outputBuses.add(juce::AudioChannelSet::stereo());
            config.sampleRate = 90000;
            config.blockSize = 200;
        }

        return config;
    }
}

// host: stereo mono
// plugin: stereosc stereo monosc mono none

SCENARIO("PluginConfigurator: Can configure the plugin correctly") {
    PluginConfigurator configurator;

    GIVEN("A host configuration and a plugin") {
        typedef std::tuple<HostConfiguration, std::vector<juce::AudioProcessor::BusesLayout>, bool> TestData;

        auto [hostConfig, acceptedLayouts, shouldSucceed] = GENERATE(
            TestData(getHostConfig("mono"), getAcceptedLayouts("mono"), true),
            TestData(getHostConfig("mono"), getAcceptedLayouts("stereo"), false),
            TestData(getHostConfig("mono"), getAcceptedLayouts("none"), false),
            TestData(getHostConfig("stereo"), getAcceptedLayouts("mono"), false),
            TestData(getHostConfig("stereo"), getAcceptedLayouts("stereo"), true),
            TestData(getHostConfig("stereo"), getAcceptedLayouts("none"), false)
        );

        auto plugin = std::make_shared<ConfigTestPluginInstance>();
        plugin->acceptedLayouts = acceptedLayouts;

        WHEN("Asked to configure it") {
            const bool success {configurator.configure(plugin, hostConfig)};

            THEN("The plugin is configured") {
                CHECK(success == shouldSucceed);
                CHECK(plugin->isPrepared == shouldSucceed);

                if (shouldSucceed) {
                    CHECK(plugin->getSampleRate() == hostConfig.sampleRate);
                    CHECK(plugin->getBlockSize() == hostConfig.blockSize);
                }
            }
        }
    }
}

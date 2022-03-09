#pragma once

#include <JuceHeader.h>

/**
 * Listens to updates to reported audio processor latency and calls the provided implementation.
 */
class LatencyListener : public juce::AsyncUpdater,
                        public juce::AudioProcessorListener {
public:
    LatencyListener() = default;

    /**
     * Called on the message thread after a latency change.
     */
    void handleAsyncUpdate() override { _onLatencyChange(); }

    void audioProcessorParameterChanged(juce::AudioProcessor* /*processor*/,
                                        int /*parameterIndex*/,
                                        float /*newValue*/) override {
        // Do nothing
    }

    /**
     * Called on the audio thread by a processor after a latency change.
     */
    void audioProcessorChanged(juce::AudioProcessor* /*processor*/, const ChangeDetails& details) override {
        if (details.latencyChanged) {
            // Trigger the update on another thread
            triggerAsyncUpdate();
        }
    }

protected:
    /**
     * Called on the message thread after a latency change in one of the processors being listened
     * to.
     */
    virtual void _onLatencyChange() = 0;
};

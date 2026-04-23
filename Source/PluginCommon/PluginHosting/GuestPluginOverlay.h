#pragma once

#include <JuceHeader.h>

#if JUCE_IOS

#include "UIUtils.h"

class GuestPluginOverlay : public juce::Component {
public:
    const std::shared_ptr<juce::AudioPluginInstance> plugin;

    GuestPluginOverlay(std::function<void()> onCloseCallback,
                       std::shared_ptr<juce::AudioPluginInstance> newPlugin);
    ~GuestPluginOverlay() override;

    void resized() override;
    void paint(juce::Graphics& g) override;
    void parentSizeChanged() override;

private:
    std::function<void()> _onCloseCallback;
    std::unique_ptr<UIUtils::StaticButtonLookAndFeel> _closeButtonLookAndFeel;
    std::unique_ptr<juce::Label> _titleLabel;
    std::unique_ptr<juce::TextButton> _closeButton;
    std::unique_ptr<juce::AudioProcessorEditor> _editor;
};

#endif // JUCE_IOS

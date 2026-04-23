#pragma once

#include <JuceHeader.h>

#include "CoreJUCEPlugin/CoreLookAndFeel.h"
#include "CoreJUCEPlugin/LookAndFeelMixins/ComboBoxV2.h"
#include "CoreJUCEPlugin/LookAndFeelMixins/LinearSliderV2.h"
#include "CoreJUCEPlugin/LookAndFeelMixins/MidAnchoredRotarySlider.h"
#include "CoreJUCEPlugin/LookAndFeelMixins/RotarySliderV2.h"
#include "UIUtilsConstants.h"

enum class MODULATION_TYPE;

namespace UIUtils {
    // LookAndFeel
    typedef WECore::LookAndFeelMixins::LinearSliderV2<WECore::LookAndFeelMixins::RotarySliderV2<WECore::JUCEPlugin::CoreLookAndFeel>> StandardSliderLookAndFeel;
    typedef WECore::LookAndFeelMixins::MidAnchoredRotarySlider<WECore::JUCEPlugin::CoreLookAndFeel> MidAnchoredSliderLookAndFeel;

    class ToggleButtonLookAndFeel : public WECore::JUCEPlugin::CoreLookAndFeel {
    public:
        enum ColourIds {
            backgroundColour,
            highlightColour,
            disabledColour
        };

        void drawButtonBackground(juce::Graphics& g,
                                  juce::Button& button,
                                  const juce::Colour& backgroundColour,
                                  bool isMouseOverButton,
                                  bool isButtonDown) override;

        void drawButtonText(juce::Graphics& g,
                            juce::TextButton& textButton,
                            bool isMouseOverButton,
                            bool isButtonDown) override;
    };

    class GridIconButtonLookAndFeel : public ToggleButtonLookAndFeel {
    public:
        void drawButtonText(juce::Graphics& g,
                            juce::TextButton& textButton,
                            bool isMouseOverButton,
                            bool isButtonDown) override;
    };

    class SettingsIconButtonLookAndFeel : public ToggleButtonLookAndFeel {
    public:
        void drawButtonText(juce::Graphics& g,
                            juce::TextButton& textButton,
                            bool isMouseOverButton,
                            bool isButtonDown) override;
    };

    class StaticButtonLookAndFeel : public WECore::JUCEPlugin::CoreLookAndFeel {
    public:
        enum ColourIds {
            backgroundColour,
            highlightColour,
            disabledColour
        };

        virtual void drawButtonBackground(juce::Graphics& g,
                                          juce::Button& button,
                                          const juce::Colour& backgroundColour,
                                          bool isMouseOverButton,
                                          bool isButtonDown) override;

        virtual void drawButtonText(juce::Graphics& g,
                                    juce::TextButton& textButton,
                                    bool isMouseOverButton,
                                    bool isButtonDown) override;

    protected:
        virtual int _getCornerSize(int width, int height) const;
    };

    class AddButtonLookAndFeel : public StaticButtonLookAndFeel {
    public:
        virtual void drawButtonText(juce::Graphics& g,
                                    juce::TextButton& textButton,
                                    bool isMouseOverButton,
                                    bool isButtonDown) override;

    protected:
        virtual int _getCornerSize(int width, int height) const override;
    };

    class TextOnlyButtonLookAndFeel : public StaticButtonLookAndFeel {
    public:
        void drawButtonBackground(juce::Graphics& /*g*/,
                                  juce::Button& /*button*/,
                                  const juce::Colour& /*backgroundColour*/,
                                  bool /*isMouseOverButton*/,
                                  bool /*isButtonDown*/) override { /* Do nothing */ }
    };

    class SearchBarLookAndFeel : public WECore::JUCEPlugin::CoreLookAndFeel {
    public:
        void drawTextEditorOutline(juce::Graphics& g,
                                   int width,
                                   int height,
                                   juce::TextEditor& textEditor) override;
    };

    class StandardComboBoxLookAndFeel : public WECore::LookAndFeelMixins::ComboBoxV2<WECore::JUCEPlugin::CoreLookAndFeel> {
    public:
        void drawComboBox(juce::Graphics& g,
                          int width,
                          int height,
                          bool isButtonDown,
                          int buttonX,
                          int buttonY,
                          int buttonW,
                          int buttonH,
                          juce::ComboBox& box) override;

        void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override;

        void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override;

        void drawPopupMenuItem(juce::Graphics& g,
                               const juce::Rectangle<int>& area,
                               bool isSeparator,
                               bool isActive,
                               bool isHighlighted,
                               bool isTicked,
                               bool hasSubMenu,
                               const juce::String& text,
                               const juce::String& shortcutKeyText,
                               const juce::Drawable* /*icon*/,
                               const juce::Colour* /*textColour*/) override;
    };

    class TempoSliderLookAndFeel : public StandardSliderLookAndFeel {
    public:
        void drawButtonBackground(juce::Graphics& g,
                                  juce::Button& button,
                                  const juce::Colour& backgroundColour,
                                  bool isMouseOverButton,
                                  bool isButtonDown) override;

        void drawButtonText(juce::Graphics& g,
                            juce::TextButton& textButton,
                            bool isMouseOverButton,
                            bool isButtonDown) override;

        juce::Slider::SliderLayout getSliderLayout(juce::Slider& slider) override;

        juce::Label* createSliderTextBox(juce::Slider& slider) override;

        juce::Button* createSliderButton(juce::Slider& slider, bool isIncrement) override;

        juce::MouseCursor getMouseCursorFor(juce::Component& component) override;
    };

    class TableHeaderLookAndFeel : public WECore::JUCEPlugin::CoreLookAndFeel {
    public:
        virtual void drawTableHeaderBackground(juce::Graphics& g,
                                               juce::TableHeaderComponent& header) override;

        virtual void drawTableHeaderColumn(juce::Graphics& g,
                                           juce::TableHeaderComponent& header,
                                           const juce::String& columnName,
                                           int columnId,
                                           int width,
                                           int height,
                                           bool isMouseOver,
                                           bool isMouseDown,
                                           int columnFlags) override;
    };

    class PopoverComponent : public juce::Component {
    public:
        PopoverComponent(juce::String title,
                         juce::String content,
                         std::function<void()> onCloseCallback);

        void resized() override;
        void paint(juce::Graphics& g) override;

    private:
        std::function<void()> _onCloseCallback;

        StaticButtonLookAndFeel _buttonLookAndFeel;

        std::unique_ptr<juce::Label> _titleLabel;
        std::unique_ptr<juce::Label> _contentLabel;
        std::unique_ptr<juce::Viewport> _contentView;
        std::unique_ptr<juce::TextButton> _button;

        juce::Rectangle<int> _contentSize;

        juce::Rectangle<int> _getBoundsForText(const juce::String& content, const juce::Font& font) const;
    };

    const juce::Colour& getColourForModulationType(MODULATION_TYPE type);

    const juce::Colour neutralColour = juce::Colour(226, 226, 226);
    const juce::Colour neutralColourWithAlpha = UIUtils::neutralColour.withAlpha(0.3f);
    const juce::Colour highlightColour = juce::Colour(0xfffc9d74);
    const juce::Colour deactivatedColour = neutralColour.withBrightness(0.5);

    const juce::Colour backgroundColour = juce::Colour(0xff272727);
    const juce::Colour slotBackgroundColour = juce::Colour(0xff272727).withMultipliedLightness(1.2f);
    const juce::Colour modulationTrayBackgroundColour = slotBackgroundColour.withMultipliedLightness(1.2f);

    const juce::Colour PLUGIN_SLOT_MODULATION_ON_COLOUR(161, 102, 221);

    const juce::Colour tooltipColour = juce::Colour(0xff929292);

    void setDefaultLabelStyle(std::unique_ptr<juce::Label>& label);

    class SafeAnimatedComponent : public juce::Component,
                                  protected juce::Timer,
                                  public juce::SettableTooltipClient {
    public:
        SafeAnimatedComponent();
        virtual ~SafeAnimatedComponent();

        void start();

        /**
         * We need to make sure the UI thread isn't inside paint() before deleting a gain stage, so
         * this will block until the timer has stopped and paint() is finished.
         */
        void stop();

    protected:
        /**
         * Inheriting classes must reset this at the start of paint() and signal it at the end of
         * paint()
         */
        juce::WaitableEvent _stopEvent;

        /**
         * Can be overidden by inheriting classes for any processing that should be done on the
         * message thread.
         */
        virtual void _onTimerCallback() {}

    private:
        void timerCallback() override;
    };

    /**
     * Returns a square that fits in the given rectangle.
     */
    template <typename T>
    juce::Rectangle<T> reduceToSquare(juce::Rectangle<T> area) {
        const T smallestDimension {std::min(area.getWidth(), area.getHeight())};
        return area.withSizeKeepingCentre(smallestDimension, smallestDimension);
    }

    class BypassButton : public juce::Button {
    public:
        BypassButton(const juce::String& buttonName);

        void paintButton(juce::Graphics& g,
                         bool shouldDrawButtonAsHighlighted,
                         bool shouldDrawButtonAsDown) override;
    };

    class ModulationButton : public juce::Button {
    public:
        ModulationButton(const juce::String& buttonName);

        void paintButton(juce::Graphics& g,
                         bool shouldDrawButtonAsHighlighted,
                         bool shouldDrawButtonAsDown) override;
    };

    class CrossButton : public juce::Button {
    public:
        enum ColourIds {
            enabledColour,
            disabledColour
        };

        CrossButton(const juce::String& buttonName);

        void paintButton(juce::Graphics& g,
                         bool shouldDrawButtonAsHighlighted,
                         bool shouldDrawButtonAsDown) override;
    };

    class PluginOpenButton : public juce::Button {
    public:
        PluginOpenButton(const juce::String& buttonName);

        void paintButton(juce::Graphics& g,
                         bool shouldDrawButtonAsHighlighted,
                         bool shouldDrawButtonAsDown) override;
    };

    class PluginReplaceButton : public juce::Button {
    public:
        PluginReplaceButton(const juce::String& buttonName);

        void paintButton(juce::Graphics& g,
                         bool shouldDrawButtonAsHighlighted,
                         bool shouldDrawButtonAsDown) override;
    };

    class DragHandle : public juce::Component, public juce::SettableTooltipClient {
    public:
        DragHandle() : juce::Component("Drag handle") { }

        void paint(juce::Graphics& g) override;

        enum ColourIds
        {
            handleColourId = 0x1201202
        };
    };

    class LinkedScrollView : public juce::Viewport {
    public:
        LinkedScrollView();
        virtual ~LinkedScrollView() = default;

        void setOtherView(juce::Viewport* otherView);
        void removeOtherView(juce::Viewport* otherView);

        void scrollBarMoved(juce::ScrollBar* scrollBar, double newRangeStart) override;

    private:
        juce::Viewport* _otherView;
    };

    /**
     * Displays the historic values of a signal similarly to a seismograph or ECG stylus.
     */
    class WaveStylusViewer : public UIUtils::SafeAnimatedComponent {
    public:
        enum ColourIds {
            lineColourId
        };

        WaveStylusViewer(std::function<float()> getNextValueCallback);

        void paint(juce::Graphics& g) override;

    private:
        std::function<float()> _getNextValueCallback;
        std::array<float, 20> _envelopeValues;

        void _onTimerCallback() override;
    };

    class UniBiModeButtons : public juce::Component {
    public:
        UniBiModeButtons(std::function<void()> onUniClick,
                         std::function<void()> onBiClick,
                         std::function<bool()> getUniState,
                         std::function<bool()> getBiState,
                         juce::Colour buttonColour);
        virtual ~UniBiModeButtons() override;

        void resized() override;

    private:
        std::function<void()> _onUniClick;
        std::function<void()> _onBiClick;

        UIUtils::ToggleButtonLookAndFeel _buttonLookAndFeel;

        std::unique_ptr<juce::TextButton> _unipolarButton;
        std::unique_ptr<juce::TextButton> _bipolarButton;
    };

    juce::String getCopyKeyName();
    juce::String getCmdKeyName();
    juce::String presetNameOrPlaceholder(const juce::String& value);

#if JUCE_IOS
    /**
     * Manages long-press detection for iOS touch events. Pass the action to fire on long press
     * to the constructor, then forward mouseDown/mouseUp/mouseDrag from the owning component.
     *
     * mouseUp() returns true if the long press already fired, so the caller can suppress the
     * normal click action.
     */
    class LongPressHandler {
    public:
        explicit LongPressHandler(std::function<void()> callback, int durationMs = 400);

        void mouseDown();
        bool mouseUp();
        void mouseDrag();

    private:
        bool _fired {false};
        int _durationMs;
        std::function<void()> _callback;

        struct Timer : juce::Timer {
            LongPressHandler& owner;
            explicit Timer(LongPressHandler& o) : owner(o) {}
            void timerCallback() override;
        } _timer;
    };
#endif
}

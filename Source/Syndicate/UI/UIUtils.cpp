#include "UIUtils.h"

namespace UIUtils {

    int getChainXPos(int chainIndex, int numChains, int graphViewWidth) {
        const int graphMidpoint {graphViewWidth / 2};
        const double offsetCoefficent {-0.5 * numChains + 1 * chainIndex};
        return static_cast<int>(graphMidpoint + CHAIN_WIDTH * offsetCoefficent);
    }

    void ToggleButtonLookAndFeel::drawButtonBackground(juce::Graphics& g,
                                                        juce::Button& button,
                                                        const juce::Colour& backgroundColour,
                                                        bool isMouseOverButton,
                                                        bool isButtonDown) {
        const int width {button.getWidth()};
        const int height {button.getHeight()};

        constexpr float indent {2.0f};
        const int cornerSize {juce::jmin(juce::roundToInt(width * 0.4f),
                                        juce::roundToInt(height * 0.4f))};

        juce::Path p;
        juce::PathStrokeType pStroke(1);

        p.addRoundedRectangle(indent,
                                indent,
                                width - 2 * indent,
                                height - 2 * indent,
                                static_cast<float>(cornerSize));

        if (button.isEnabled()) {
            g.setColour(button.findColour(juce::TextButton::buttonOnColourId));
        } else {
            g.setColour(button.findColour(juce::TextButton::textColourOnId));
        }

        g.strokePath(p, pStroke);
        if (button.getToggleState() && button.isEnabled()) {
            g.fillPath(p);
        }
    }

    void ToggleButtonLookAndFeel::drawButtonText(juce::Graphics& g,
                                                  juce::TextButton& textButton,
                                                  bool isMouseOverButton,
                                                  bool isButtonDown) {
        constexpr int MARGIN {0};

        juce::Font font;
        font.setTypefaceName(WECore::JUCEPlugin::CoreLookAndFeel::getTypefaceForFont(font)->getName());
        g.setFont(font);

        if (!textButton.isEnabled()) {
            g.setColour(textButton.findColour(juce::TextButton::textColourOnId));
        } else if (textButton.getToggleState()) {
            g.setColour(textButton.findColour(juce::TextButton::textColourOnId));
        } else {
            g.setColour(textButton.findColour(juce::TextButton::textColourOffId));
        }

        g.drawFittedText(textButton.getButtonText(),
                         MARGIN,
                         0,
                         textButton.getWidth() - 2 * MARGIN,
                         textButton.getHeight(),
                         juce::Justification::centred,
                         0);
    }


    void StaticButtonLookAndFeel::drawButtonBackground(juce::Graphics& g,
                                                       juce::Button& button,
                                                       const juce::Colour& backgroundColour,
                                                       bool isMouseOverButton,
                                                       bool isButtonDown) {
        const int width {button.getWidth()};
        const int height {button.getHeight()};

        constexpr float indent {2.0f};
        const int cornerSize {_getCornerSize(width, height)};

        juce::Path p;
        juce::PathStrokeType pStroke(1);

        p.addRoundedRectangle(indent,
                                indent,
                                width - 2 * indent,
                                height - 2 * indent,
                                static_cast<float>(cornerSize));

        if (button.isEnabled()) {
            g.setColour(button.findColour(juce::TextButton::buttonOnColourId));
        } else {
            g.setColour(button.findColour(juce::TextButton::buttonColourId));
        }

        g.strokePath(p, pStroke);
    }

    void StaticButtonLookAndFeel::drawButtonText(juce::Graphics& g,
                                                 juce::TextButton& textButton,
                                                 bool isMouseOverButton,
                                                 bool isButtonDown) {

        const int cornerSize {_getCornerSize(textButton.getWidth(), textButton.getHeight())};

        juce::Font font;
        font.setTypefaceName(WECore::JUCEPlugin::CoreLookAndFeel::getTypefaceForFont(font)->getName());
        g.setFont(font);

        if (textButton.isEnabled()) {
            g.setColour(textButton.findColour(juce::TextButton::textColourOnId));
        } else {
            g.setColour(textButton.findColour(juce::TextButton::textColourOffId));
        }

        g.drawFittedText(textButton.getButtonText(),
                         cornerSize,
                         0,
                         textButton.getWidth() - 2 * cornerSize,
                         textButton.getHeight(),
                         juce::Justification::centred,
                         0);
    }

    int StaticButtonLookAndFeel::_getCornerSize(int width, int height) const {
        return juce::jmin(juce::roundToInt(width * 0.4f), juce::roundToInt(height * 0.4f));
    }

    void SearchBarLookAndFeel::drawTextEditorOutline(juce::Graphics& g,
                                                     int width,
                                                     int height,
                                                     juce::TextEditor& textEditor) {
        const juce::Rectangle<float> area =
            juce::Rectangle<float>(textEditor.getWidth(), textEditor.getHeight()).reduced(1, 1);

        g.setColour(textEditor.findColour(juce::TextEditor::outlineColourId));
        g.drawRoundedRectangle(area, 2, 1);
    }


    const juce::Colour& getColourForModulationType(MODULATION_TYPE type) {
        static const juce::Colour macroColour(147, 252, 116);
        static const juce::Colour lfoColour(252, 116, 147);
        static const juce::Colour envelopeColour(116, 147, 252);

        switch (type) {
            case MODULATION_TYPE::MACRO:
                return macroColour;
            case MODULATION_TYPE::LFO:
                return lfoColour;
            case MODULATION_TYPE::ENVELOPE:
                return envelopeColour;
        }
    }

    void setDefaultLabelStyle(std::unique_ptr<juce::Label>& label) {
        label->setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle("Regular"));
        label->setJustificationType(juce::Justification::centred);
        label->setEditable(false, false, false);
        label->setColour(juce::Label::textColourId, neutralHighlightColour);
        label->setColour(juce::TextEditor::textColourId, juce::Colours::black);
        label->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x00000000));
    }

    void StandardComboBoxLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height) {
        g.fillAll(findColour(juce::PopupMenu::backgroundColourId));
    }

    void StandardComboBoxLookAndFeel::drawPopupMenuItem(juce::Graphics& g,
                                                        const juce::Rectangle<int>& area,
                                                        bool isSeparator,
                                                        bool isActive,
                                                        bool isHighlighted,
                                                        bool isTicked,
                                                        bool hasSubMenu,
                                                        const juce::String& text,
                                                        const juce::String& shortcutKeyText,
                                                        const juce::Drawable* /*icon*/,
                                                        const juce::Colour* /*textColour*/) {

        juce::Rectangle<int> availableArea = area.reduced(1);

        juce::Rectangle<int> leftMarginArea = availableArea.removeFromLeft(availableArea.getWidth() / 8);

        if (isHighlighted) {
            g.setColour(findColour(juce::PopupMenu::highlightedBackgroundColourId));
            g.fillRect(availableArea);
            g.fillRect(leftMarginArea);
            g.setColour(findColour(juce::PopupMenu::highlightedTextColourId));
        } else {
            if (isTicked) {
                g.setColour(findColour(juce::PopupMenu::highlightedBackgroundColourId));
                g.fillRect(leftMarginArea.removeFromLeft(leftMarginArea.getWidth() / 2));
            }

            g.setColour(findColour(juce::PopupMenu::textColourId));
        }

        juce::Font font(getPopupMenuFont());

        const float maxFontHeight = static_cast<float>(area.getHeight() / 1.3f);

        if (font.getHeight() > maxFontHeight) {
            font.setHeight(maxFontHeight);
        }

        g.setFont(font);
        g.drawFittedText(text, availableArea, juce::Justification::centredLeft, 1);
    }

    void TableHeaderLookAndFeel::drawTableHeaderBackground(juce::Graphics& g,
                                                           juce::TableHeaderComponent& header) {
        // Main fill
        g.fillAll(header.findColour(juce::TableHeaderComponent::backgroundColourId));

        // Bottom line
        g.setColour(header.findColour(juce::TableHeaderComponent::outlineColourId));
        g.fillRect(header.getLocalBounds().removeFromBottom(1));
    }

    void TableHeaderLookAndFeel::drawTableHeaderColumn(juce::Graphics& g,
                                                       juce::TableHeaderComponent& header,
                                                       const juce::String& columnName,
                                                       int columnId,
                                                       int width,
                                                       int height,
                                                       bool isMouseOver,
                                                       bool isMouseDown,
                                                       int columnFlags) {
        // Hover/click highlight
        const juce::Colour highlightColour = header.findColour(juce::TableHeaderComponent::highlightColourId);

        if (isMouseOver) {
            g.fillAll(highlightColour.withMultipliedAlpha(0.1f));
        }

        // Sort arrow
        g.setColour(header.findColour(juce::TableHeaderComponent::textColourId));
        juce::Rectangle<int> area(width, height);
        area.reduce(4, 0);

        if ((columnFlags & (juce::TableHeaderComponent::sortedForwards | juce::TableHeaderComponent::sortedBackwards)) != 0) {
            juce::Path sortArrow;
            sortArrow.addTriangle(0.0f,
                                  0.0f,
                                  0.5f,
                                  (columnFlags & juce::TableHeaderComponent::sortedForwards) != 0 ? -0.8f : 0.8f,
                                  1.0f,
                                  0.0f);

            g.fillPath(sortArrow, sortArrow.getTransformToScaleToFit(area.removeFromRight(height / 2).reduced(2).toFloat(), true));
        }

        // Header title
        g.setFont(juce::Font(height * 0.5f, juce::Font::bold));
        g.drawFittedText(columnName, area, juce::Justification::centredLeft, 1);
    }

    PopoverComponent::PopoverComponent(juce::String title,
                                       juce::String content,
                                       std::function<void()> onCloseCallback) :
                _onCloseCallback(onCloseCallback) {
        _titleLabel.reset(new juce::Label("Title Label", title));
        addAndMakeVisible(_titleLabel.get());
        _titleLabel->setFont(juce::Font(20.00f, juce::Font::plain).withTypefaceStyle("Bold"));
        _titleLabel->setJustificationType(juce::Justification::centred);
        _titleLabel->setEditable(false, false, false);
        _titleLabel->setColour(juce::Label::textColourId, UIUtils::neutralHighlightColour);

        const juce::Font contentFont = juce::Font(15.0f, juce::Font::plain).withTypefaceStyle("Regular");
        _contentLabel.reset(new juce::Label("Content Label", content));
        addAndMakeVisible(_contentLabel.get());
        _contentLabel->setFont(contentFont);
        _contentLabel->setJustificationType(juce::Justification::centred);
        _contentLabel->setEditable(false, false, false);
        _contentLabel->setColour(juce::Label::textColourId, UIUtils::neutralHighlightColour);

        _contentSize = _getBoundsForText(content, contentFont);

        _contentView.reset(new juce::Viewport());
        _contentView->setViewedComponent(_contentLabel.get(), false);
        _contentView->setScrollBarsShown(true, true);
        _contentView->getVerticalScrollBar().setColour(juce::ScrollBar::ColourIds::backgroundColourId, juce::Colour(0x00000000));
        _contentView->getVerticalScrollBar().setColour(juce::ScrollBar::ColourIds::thumbColourId, UIUtils::neutralHighlightColour.withAlpha(0.5f));
        _contentView->getVerticalScrollBar().setColour(juce::ScrollBar::ColourIds::trackColourId, juce::Colour(0x00000000));
        _contentView->getHorizontalScrollBar().setColour(juce::ScrollBar::ColourIds::backgroundColourId, juce::Colour(0x00000000));
        _contentView->getHorizontalScrollBar().setColour(juce::ScrollBar::ColourIds::thumbColourId, UIUtils::neutralHighlightColour.withAlpha(0.5f));
        _contentView->getHorizontalScrollBar().setColour(juce::ScrollBar::ColourIds::trackColourId, juce::Colour(0x00000000));
        addAndMakeVisible(_contentView.get());

        _button.reset(new juce::TextButton("OK button"));
        addAndMakeVisible(_button.get());
        _button->setButtonText(TRANS("OK"));
        _button->addListener(this);
        _button->setLookAndFeel(&_buttonLookAndFeel);
        _button->setColour(juce::TextButton::buttonOnColourId, UIUtils::neutralControlColour);
        _button->setColour(juce::TextButton::buttonColourId, UIUtils::neutralHighlightColour);
        _button->setColour(juce::TextButton::textColourOnId, UIUtils::neutralControlColour);
        _button->setColour(juce::TextButton::textColourOffId, UIUtils::neutralHighlightColour);
    }

    void PopoverComponent::resized() {
        juce::Rectangle<int> availableArea = getLocalBounds().reduced(20);

        _titleLabel->setBounds(availableArea.removeFromTop(availableArea.getHeight() / 5));

        juce::Rectangle<int> buttonArea = availableArea.removeFromBottom(availableArea.getHeight() / 4);
        _button->setBounds(buttonArea.withSizeKeepingCentre(60, 40));
        _contentView->setBounds(availableArea);

        const int scrollBarWidth {10};
        const int contentWidth {std::max(_contentSize.getWidth(), availableArea.getWidth() - scrollBarWidth)};
        _contentLabel->setBounds(_contentSize.withWidth(contentWidth));
    }

    void PopoverComponent::paint(juce::Graphics& g) {
        g.fillAll(juce::Colours::black.withAlpha(0.8f));
    }

    void PopoverComponent::buttonClicked(juce::Button* buttonThatWasClicked) {
        if (buttonThatWasClicked == _button.get()) {
            _onCloseCallback();
        }
    }

    juce::Rectangle<int> PopoverComponent::_getBoundsForText(const juce::String& content, const juce::Font& font) const {
        const juce::StringArray lines = juce::StringArray::fromLines(content);

        int maxWidth {0};
        for (const juce::String& line : lines) {
            const int thisWidth {font.getStringWidth(line)};
            if (thisWidth > maxWidth) {
                maxWidth = thisWidth;
            }
        }

        constexpr int MARGIN {5};
        return juce::Rectangle<int>(maxWidth + MARGIN, lines.size() * font.getHeight());
    }

    SafeAnimatedComponent::SafeAnimatedComponent() : _stopEvent(true) {
    }

    SafeAnimatedComponent::~SafeAnimatedComponent() {
        stop();
    }

    void SafeAnimatedComponent::start() {
        startTimerHz(20);
    }

    void SafeAnimatedComponent::stop() {
        stopTimer();
        _stopEvent.wait(1000);
    }

    void SafeAnimatedComponent::timerCallback() {
        _onTimerCallback();
        repaint();
    }

    BypassButton::BypassButton(const juce::String& buttonName) : juce::Button(buttonName) { }

    void BypassButton::paintButton(juce::Graphics& g,
                                   bool /*shouldDrawButtonAsHighlighted*/,
                                   bool /*shouldDrawButtonAsDown*/) {
        const juce::Rectangle<float> area = getLocalBounds().reduced(1, 1).toFloat();

        g.setColour(findColour(juce::TextButton::buttonOnColourId));
        g.drawEllipse(area, 1.0f);
        if (getToggleState()) {
            g.fillEllipse(area);
            g.setColour(findColour(juce::TextButton::textColourOnId));
        } else {
            g.setColour(findColour(juce::TextButton::textColourOffId));
        }

        g.drawLine(getWidth() * 0.5, getHeight() * 0.25, getWidth() * 0.5, getHeight() * 0.75, 1.0f);
    }

    ModulationButton::ModulationButton(const juce::String& buttonName) : juce::Button(buttonName) { }

    void ModulationButton::paintButton(juce::Graphics& g,
                                   bool /*shouldDrawButtonAsHighlighted*/,
                                   bool /*shouldDrawButtonAsDown*/) {
        const juce::Rectangle<float> area = getLocalBounds().reduced(1, 1).toFloat();

        g.setColour(findColour(juce::TextButton::buttonOnColourId));
        g.drawEllipse(area, 1.0f);
        if (getToggleState()) {
            g.fillEllipse(area);
            g.setColour(findColour(juce::TextButton::textColourOnId));
        } else {
            g.setColour(findColour(juce::TextButton::textColourOffId));
        }

        g.drawText("M", getLocalBounds().reduced(2), juce::Justification::centred, false);
    }

    CrossButton::CrossButton(const juce::String& buttonName) : juce::Button(buttonName) { }

    void CrossButton::paintButton(juce::Graphics& g,
                                  bool /*shouldDrawButtonAsHighlighted*/,
                                  bool /*shouldDrawButtonAsDown*/) {
        juce::Rectangle<float> area = reduceToSquare(getLocalBounds()).toFloat();
        area = area.reduced(area.getWidth() / 3);

        g.setColour(findColour(juce::TextButton::buttonOnColourId));
        g.drawLine(juce::Line(area.getTopLeft(), area.getBottomRight()));
        g.drawLine(juce::Line(area.getTopRight(), area.getBottomLeft()));
    }

    PluginOpenButton::PluginOpenButton(const juce::String& buttonName) : juce::Button(buttonName) { }

    void PluginOpenButton::paintButton(juce::Graphics& g,
                                       bool /*shouldDrawButtonAsHighlighted*/,
                                       bool /*shouldDrawButtonAsDown*/) {
        juce::Rectangle<float> area = reduceToSquare(getLocalBounds()).toFloat();
        area = area.reduced(area.getWidth() / 5, area.getWidth() / 4);

        const float cornerRadius {area.getWidth() / 5};
        const float lineThickness {1.0f};

        // Draw the outer part
        g.setColour(findColour(juce::TextButton::buttonOnColourId));
        g.drawRoundedRectangle(area, cornerRadius, lineThickness);

        // Draw the inner part
        const float border {area.getWidth() / 10};
        area.removeFromTop(border * 2);

        g.drawRoundedRectangle(area, cornerRadius, lineThickness);
    }

    PluginReplaceButton::PluginReplaceButton(const juce::String& buttonName) : juce::Button(buttonName) { }

    void PluginReplaceButton::paintButton(juce::Graphics& g,
                                          bool /*shouldDrawButtonAsHighlighted*/,
                                          bool /*shouldDrawButtonAsDown*/) {
        juce::Rectangle<float> area = reduceToSquare(getLocalBounds()).toFloat();
        area = area.reduced(area.getWidth() / 4);


        const float cornerRadius {area.getWidth() / 4};
        const float horizontalGap {area.getWidth() / 2};
        const float arrowHeadLength {area.getWidth() / 3};

        // Draw from top left going clockwise
        juce::Path p;
        p.startNewSubPath(area.getX() + horizontalGap, area.getY());
        p.lineTo(area.getRight() - cornerRadius, area.getY());
        p.addCentredArc(area.getRight() - cornerRadius,
                        area.getY() + cornerRadius,
                        cornerRadius,
                        cornerRadius,
                        0,
                        0,
                        WECore::CoreMath::DOUBLE_PI / 2);
        p.lineTo(area.getBottomRight());
        p.lineTo(area.getRight() - arrowHeadLength, area.getBottom() - arrowHeadLength);
        p.startNewSubPath(area.getBottomRight());
        p.lineTo(area.getRight() + arrowHeadLength, area.getBottom() - arrowHeadLength);

        // Second arrow
        p.startNewSubPath(area.getRight() - horizontalGap, area.getBottom());
        p.lineTo(area.getX() + cornerRadius, area.getBottom());
        p.addCentredArc(area.getX() + cornerRadius,
                        area.getBottom() - cornerRadius,
                        cornerRadius,
                        cornerRadius,
                        WECore::CoreMath::DOUBLE_PI,
                        0,
                        WECore::CoreMath::DOUBLE_PI / 2);
        p.lineTo(area.getTopLeft());
        p.lineTo(area.getX() + arrowHeadLength, area.getY() + arrowHeadLength);
        p.startNewSubPath(area.getTopLeft());
        p.lineTo(area.getX() - arrowHeadLength, area.getY() + arrowHeadLength);

        g.setColour(findColour(juce::TextButton::buttonOnColourId));
        g.strokePath(p, juce::PathStrokeType(1));
    }

    void DragHandle::paint(juce::Graphics& g) {
        juce::Rectangle<int> area = reduceToSquare(getLocalBounds());
        area = area.reduced(area.getWidth() / 5);

        const int arrowHeadLength {area.getWidth() / 5};

        juce::Path p;

        // Left arrow head
        p.startNewSubPath(area.getX(), area.getCentreY());
        p.lineTo(area.getX() + arrowHeadLength, area.getCentreY() - arrowHeadLength);
        p.startNewSubPath(area.getX(), area.getCentreY());
        p.lineTo(area.getX() + arrowHeadLength, area.getCentreY() + arrowHeadLength);

        // Arrow going right
        p.startNewSubPath(area.getX(), area.getCentreY());
        p.lineTo(area.getRight(), area.getCentreY());
        p.lineTo(area.getRight() - arrowHeadLength, area.getCentreY() - arrowHeadLength);
        p.startNewSubPath(area.getRight(), area.getCentreY());
        p.lineTo(area.getRight() - arrowHeadLength, area.getCentreY() + arrowHeadLength);

        // Bottom arrow head
        p.startNewSubPath(area.getCentreX(), area.getBottom());
        p.lineTo(area.getCentreX() - arrowHeadLength, area.getBottom() - arrowHeadLength);
        p.startNewSubPath(area.getCentreX(), area.getBottom());
        p.lineTo(area.getCentreX() + arrowHeadLength, area.getBottom() - arrowHeadLength);

        // Arrow going up
        p.startNewSubPath(area.getCentreX(), area.getBottom());
        p.lineTo(area.getCentreX(), area.getY());
        p.lineTo(area.getCentreX() - arrowHeadLength, area.getY() + arrowHeadLength);
        p.startNewSubPath(area.getCentreX(), area.getY());
        p.lineTo(area.getCentreX() + arrowHeadLength, area.getY() + arrowHeadLength);

        g.setColour(findColour(ColourIds::handleColourId));
        g.strokePath(p, juce::PathStrokeType(1));
    }

    LinkedScrollView::LinkedScrollView() : _otherView(nullptr) {
    }

    void LinkedScrollView::setOtherView(juce::Viewport* otherView) {
        _otherView = otherView;
    }

    void LinkedScrollView::scrollBarMoved(juce::ScrollBar* scrollBar, double newRangeStart) {
        juce::Viewport::scrollBarMoved(scrollBar, newRangeStart);

        const int newRangeStartInt {juce::roundToInt(newRangeStart)};
        if (_otherView != nullptr) {
            // Only linked horizontally
            _otherView->setViewPosition(newRangeStartInt, 0);
        }
    }
}

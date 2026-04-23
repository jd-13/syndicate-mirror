#include "UIUtils.h"
#include "ModulationBarStepSequencer.h"

namespace {
    const juce::Colour& baseColour = UIUtils::getColourForModulationType(MODULATION_TYPE::STEP_SEQUENCER);
    const juce::Colour faintColour = baseColour.withAlpha(0.5f);

    juce::Colour getBrightenedColour(const juce::Colour& colour, bool isPlaying) {
        return isPlaying ? colour.brighter(0.1f) : colour;
    }

    constexpr int STEP_CELL_WIDTH {40};
    constexpr int STEP_CELL_HEIGHT {80};

    const std::array<double, 9> ALLOWED_LENGTH_MULTIPLIERS {
        0.25, 1.0 / 3.0, 0.5, 2.0 / 3.0, 1.0, 1.5, 2.0, 3.0, 4.0
    };

    const std::array<const char*, 9> LENGTH_MULTIPLIER_LABELS {
        "1/4x", "1/3x", "1/2x", "2/3x", "1x", "3/2x", "2x", "3x", "4x"
    };

    int closestLengthIndex(double multiplier) {
        int best {0};
        double bestDist {std::abs(multiplier - ALLOWED_LENGTH_MULTIPLIERS[0])};

        for (int i = 1; i < static_cast<int>(ALLOWED_LENGTH_MULTIPLIERS.size()); ++i) {
            const double thisDist = std::abs(multiplier - ALLOWED_LENGTH_MULTIPLIERS[i]);

            if (thisDist < bestDist) {
                bestDist = thisDist;
                best = i;
            } else if (thisDist > bestDist) {
                // Since the allowed multipliers are sorted, we can stop once the distance starts increasing
                break;
            }
        }
        return best;
    }

    struct FractionText {
        juce::String numerator;
        juce::String denominator;
        juce::String suffix;
    };

    FractionText parseFractionText(const juce::String& text) {
        const int slashIndex {text.indexOfChar('/')};

        if (slashIndex <= 0) {
            return {text, "", ""};
        }

        const juce::String numerator = text.substring(0, slashIndex);
        juce::String rest = text.substring(slashIndex + 1);
        juce::String suffix;
        if (rest.endsWithChar('x')) {
            suffix = "x";
            rest = rest.dropLastCharacters(1);
        }

        return {numerator, rest, suffix};
    }

    struct FractionTextLayout {
        juce::Rectangle<int> numeratorArea;
        juce::Rectangle<int> denominatorArea;
        juce::Rectangle<int> suffixArea;
        juce::Rectangle<int> barArea;
    };

    FractionTextLayout layoutFractionText(const FractionText& text, const juce::Font& font, const juce::Rectangle<float>& bounds) {
        const int fractionWidth {
            std::max(font.getStringWidth(text.numerator), font.getStringWidth(text.denominator))
        };

        const int suffixWidth = text.suffix.isNotEmpty() ? font.getStringWidth(text.suffix) : 0;
        const int gap = text.suffix.isNotEmpty() ? 2 : 0;
        const int totalWidth = fractionWidth + gap + suffixWidth;

        const juce::Rectangle<int> iBounds = bounds.toNearestInt();
        const int cx = iBounds.getCentreX() - totalWidth / 2;
        const int cy = iBounds.getCentreY();

        return {
            juce::Rectangle<int>(cx, cy - font.getHeight(), fractionWidth, static_cast<int>(font.getHeight())),
            juce::Rectangle<int>(cx, cy, fractionWidth, static_cast<int>(font.getHeight())),
            juce::Rectangle<int>(cx + fractionWidth + gap, cy - static_cast<int>(font.getHeight()) / 2, suffixWidth, static_cast<int>(font.getHeight())),
            juce::Rectangle<int>(static_cast<float>(cx), static_cast<float>(cy) - 0.5f, static_cast<float>(fractionWidth), 1.0f)
        };
    }
}

// ===========================================================================
// WaveShapeDisplay

ModulationBarStepSequencer::WaveShapeDisplay::WaveShapeDisplay(
        int stepNumber,
        std::function<void(double)> onValueChanged,
        std::function<int(bool)> getSelectedShape,
        std::function<void(int)> onShapeChanged,
        std::function<void(double)> onLengthChanged)
    : _stepNumber(stepNumber),
      _onValueChanged(std::move(onValueChanged)),
      _getSelectedShape(std::move(getSelectedShape)),
      _onShapeChanged(std::move(onShapeChanged)),
      _onLengthChanged(std::move(onLengthChanged)) {
}

void ModulationBarStepSequencer::WaveShapeDisplay::setStepState(double value, int shape, bool reverse, int repeat, double lengthMultiplier) {
    _value = value;
    _shape = shape;
    _reverse = reverse;
    _repeat = repeat;
    _lengthMultiplier = lengthMultiplier;
    repaint();
}

void ModulationBarStepSequencer::WaveShapeDisplay::setPlaying(bool playing) {
    if (_isPlaying != playing) {
        _isPlaying = playing;
        repaint();
    }
}

void ModulationBarStepSequencer::WaveShapeDisplay::paint(juce::Graphics& g) {
    const auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    const float width = bounds.getWidth();

    const float bottom = bounds.getBottom();
    const float height = bounds.getHeight();

    // Background
    g.setColour(getBrightenedColour(UIUtils::slotBackgroundColour, _isPlaying));
    g.fillRect(bounds);

    // Build curve path — draw the shape _repeat times across the cell width
    juce::Path curvePath;
    constexpr int NUM_POINTS_PER_REP = 32;
    const float repWidth = width / _repeat;

    for (int rep = 0; rep < _repeat; ++rep) {
        for (int i = 0; i <= NUM_POINTS_PER_REP; ++i) {
            const double t = static_cast<double>(i) / NUM_POINTS_PER_REP;
            const double tShaped = _reverse ? 1.0 - t : t;

            // TODO: these shape formulas should come from the sequencer
            double shaped;
            switch (_shape) {
                case 2:  shaped = tShaped; break;               // RAMP
                case 3:  shaped = tShaped * tShaped; break;     // EXP
                case 4:  shaped = 1.0 - tShaped * tShaped; break; // REV_EXP
                default: shaped = 1.0; break;                   // FLAT
            }

            const float x = bounds.getX() + (rep + static_cast<float>(t)) * repWidth;
            const float y = bottom - static_cast<float>(_value * shaped) * height;

            if (rep == 0 && i == 0) {
                curvePath.startNewSubPath(x, y);
            } else {
                curvePath.lineTo(x, y);
            }
        }
    }

    // Fill from curve to bottom
    juce::Path fillPath(curvePath);
    fillPath.lineTo(bounds.getRight(), bottom);
    fillPath.lineTo(bounds.getX(), bottom);
    fillPath.closeSubPath();

    g.setColour(getBrightenedColour(faintColour, _isPlaying));
    g.fillPath(fillPath);

    // Stroke the curve
    g.setColour(getBrightenedColour(baseColour, _isPlaying));
    g.strokePath(curvePath, juce::PathStrokeType(1.5f));

    // Step number (or length ratio on hover), centred, faint
    const bool showRatio {_isMouseOver};
    const juce::String cellText = showRatio ?
        juce::String(LENGTH_MULTIPLIER_LABELS[closestLengthIndex(_lengthMultiplier)]) :
        juce::String(_stepNumber);

    g.setColour(getBrightenedColour(faintColour, _isPlaying));

    const int slashIdx {cellText.indexOfChar('/')};
    if (showRatio && slashIdx > 0) {
        // Stacked fraction: numerator / denominator, with trailing 'x' to the right
        const FractionText fractionText = parseFractionText(cellText);

        const juce::Font fracFont(14.0f);

        const FractionTextLayout layout = layoutFractionText(fractionText, fracFont, bounds);

        g.setFont(fracFont);
        g.drawText(fractionText.numerator,
                   layout.numeratorArea,
                   juce::Justification::centred, false);
        g.drawText(fractionText.denominator,
                   layout.denominatorArea,
                   juce::Justification::centred, false);

        // Fraction bar
        g.fillRect(layout.barArea);

        if (fractionText.suffix.isNotEmpty()) {
            g.setFont(fracFont);
            g.drawText(fractionText.suffix, layout.suffixArea, juce::Justification::centredLeft, false);
        }
    } else {
        g.setFont(juce::Font(14.0f));
        g.drawText(cellText, bounds.toNearestInt(), juce::Justification::centred, false);
    }
}

void ModulationBarStepSequencer::WaveShapeDisplay::mouseDown(const juce::MouseEvent& e) {
    if (_getSelectedShape) {
        const bool cmdHeld = e.mods.isCommandDown();
        const int selectedShape = _getSelectedShape(cmdHeld);
        if (selectedShape != 0 && selectedShape != _shape) {
            _shape = selectedShape;
            if (_onShapeChanged) _onShapeChanged(selectedShape);
            repaint();
        }
    }

    _dragStartValue = _value;
    _dragStartLengthIndex = closestLengthIndex(_lengthMultiplier);
    _dragStartX = e.x;
    _dragStartY = e.y;
    _dragAxis = DragAxis::None;
}

void ModulationBarStepSequencer::WaveShapeDisplay::mouseDrag(const juce::MouseEvent& e) {
    const int dx {e.x - _dragStartX};
    const int dy {e.y - _dragStartY};

    // Figure out what kind of drag this is
    if (_dragAxis == DragAxis::None) {
        constexpr int DRAG_AXIS_THRESHOLD {4};

        if (std::abs(dx) >= DRAG_AXIS_THRESHOLD && std::abs(dx) > std::abs(dy)) {
            _dragAxis = DragAxis::Length;
        } else if (std::abs(dy) >= DRAG_AXIS_THRESHOLD) {
            _dragAxis = DragAxis::Value;
        } else {
            // Nothing to do yet
            return;
        }
    }

    if (_dragAxis == DragAxis::Value) {
        const float sensitivity {1.0f / static_cast<float>(getHeight())};
        const float delta {static_cast<float>(-dy) * sensitivity};
        const double newValue {juce::jlimit(0.0, 1.0, _dragStartValue + static_cast<double>(delta))};
        if (newValue != _value) {
            _value = newValue;
            repaint();
            if (_onValueChanged) {
                _onValueChanged(_value);
            }
        }
    } else if (_dragAxis == DragAxis::Length) {
        // Snap length to an allowed increment
        constexpr int LENGTH_DRAG_PIXELS_PER_INCREMENT {18};
        const int increments {dx / LENGTH_DRAG_PIXELS_PER_INCREMENT};

        constexpr int maxIdx {static_cast<int>(ALLOWED_LENGTH_MULTIPLIERS.size()) - 1};
        const int newIndex = juce::jlimit(0, maxIdx, _dragStartLengthIndex + increments);

        const double newMultiplier {ALLOWED_LENGTH_MULTIPLIERS[newIndex]};
        if (newMultiplier != _lengthMultiplier) {
            _lengthMultiplier = newMultiplier;
            repaint();
            if (_onLengthChanged) {
                _onLengthChanged(_lengthMultiplier);
            }
        }
    }
}

void ModulationBarStepSequencer::WaveShapeDisplay::mouseUp(const juce::MouseEvent&) {
    _dragAxis = DragAxis::None;
}

void ModulationBarStepSequencer::WaveShapeDisplay::mouseDoubleClick(const juce::MouseEvent&) {
    _value = 0.0;
    repaint();
    if (_onValueChanged) {
        _onValueChanged(_value);
    }
}

void ModulationBarStepSequencer::WaveShapeDisplay::mouseEnter(const juce::MouseEvent&) {
    _isMouseOver = true;
    repaint();
}

void ModulationBarStepSequencer::WaveShapeDisplay::mouseExit(const juce::MouseEvent&) {
    _isMouseOver = false;
    repaint();
}

// ===========================================================================
// ShapePaletteCell

ModulationBarStepSequencer::ShapePaletteCell::ShapePaletteCell(int shape, std::function<void(int)> onSelected)
    : _shape(shape), _onSelected(std::move(onSelected)) {
}

void ModulationBarStepSequencer::ShapePaletteCell::paint(juce::Graphics& g) {
    const auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    const float bottom = bounds.getBottom();
    const float height = bounds.getHeight();
    const float width = bounds.getWidth();

    // Background
    g.setColour(UIUtils::slotBackgroundColour);
    g.fillRect(bounds);

    // Border when selected
    if (_isSelected) {
        g.setColour(baseColour);
        g.drawRect(bounds, 1.5f);
    }

    // Draw wave shape curve at full amplitude
    juce::Path curvePath;
    constexpr int NUM_POINTS = 16;

    for (int i = 0; i <= NUM_POINTS; ++i) {
        const double t = static_cast<double>(i) / NUM_POINTS;
        double shaped;
        switch (_shape) {
            case 2:  shaped = t; break;
            case 3:  shaped = t * t; break;
            case 4:  shaped = 1.0 - t * t; break;
            default: shaped = 1.0; break;
        }

        const float x = bounds.getX() + static_cast<float>(t) * width;
        const float y = bottom - static_cast<float>(shaped) * height * 0.8f;

        if (i == 0) {
            curvePath.startNewSubPath(x, y);
        } else {
            curvePath.lineTo(x, y);
        }
    }

    g.setColour(_isSelected ? baseColour : faintColour);
    g.strokePath(curvePath, juce::PathStrokeType(1.5f));
}

void ModulationBarStepSequencer::ShapePaletteCell::mouseDown(const juce::MouseEvent&) {
    if (_onSelected) {
        _onSelected(_shape);
    }
}

void ModulationBarStepSequencer::ShapePaletteCell::setSelected(bool selected) {
    if (_isSelected != selected) {
        _isSelected = selected;
        repaint();
    }
}

// ===========================================================================
// StepCell

ModulationBarStepSequencer::StepCell::StepCell(SyndicateAudioProcessor& processor,
                                               int seqIndex,
                                               int stepIndex,
                                               std::function<void()> onChanged,
                                               std::function<int(bool)> getSelectedShape) :
        _processor(processor),
        _seqIndex(seqIndex),
        _stepIndex(stepIndex),
        _onChanged(onChanged) {

    waveDisplay = std::make_unique<WaveShapeDisplay>(
        _stepIndex + 1,
        [this](double newValue) {
            _processor.setStepSeqStepValue(_seqIndex, 0, _stepIndex, newValue);
            if (_onChanged) {
                _onChanged();
            }
        },
        std::move(getSelectedShape),
        [this](int newShape) {
            _processor.setStepSeqStepShape(_seqIndex, 0, _stepIndex, newShape);
            if (_onChanged) {
                _onChanged();
            }
        },
        [this](double newLength) {
            _processor.setStepSeqStepLengthMultiplier(_seqIndex, 0, _stepIndex, newLength);
            if (_onChanged) {
                _onChanged();
            }
        });
    addAndMakeVisible(waveDisplay.get());
    waveDisplay->setTooltip(TRANS("Drag vertically to change value, horizontally to change length."));

    reverseButton.reset(new juce::TextButton("Step Reverse Button " + juce::String(stepIndex)));
    addAndMakeVisible(reverseButton.get());
    reverseButton->setButtonText(TRANS("Rev"));
    reverseButton->setTooltip(TRANS("Reverse step shape"));
    reverseButton->setClickingTogglesState(true);
    reverseButton->setLookAndFeel(&_buttonLookAndFeel);
    reverseButton->setColour(UIUtils::ToggleButtonLookAndFeel::backgroundColour, UIUtils::slotBackgroundColour);
    reverseButton->setColour(UIUtils::ToggleButtonLookAndFeel::highlightColour, baseColour);
    reverseButton->setColour(UIUtils::ToggleButtonLookAndFeel::disabledColour, UIUtils::deactivatedColour);
    reverseButton->onClick = [this]() {
        _processor.setStepSeqStepReverse(_seqIndex, 0, _stepIndex, reverseButton->getToggleState());
        refresh();
        if (_onChanged) {
            _onChanged();
        }
    };

    namespace SP = WECore::StepSeq::Parameters;
    repeatSlider.reset(new juce::Slider("Step Repeat Slider " + juce::String(stepIndex)));
    addAndMakeVisible(repeatSlider.get());
    repeatSlider->setTooltip(TRANS("Number of repeats for this step"));
    repeatSlider->setRange(SP::STEP_REPEAT.minValue, SP::STEP_REPEAT.maxValue, 1);
    repeatSlider->setSliderStyle(juce::Slider::IncDecButtons);
    repeatSlider->setTextBoxStyle(juce::Slider::TextBoxLeft, false, 24, 16);
    repeatSlider->setIncDecButtonsMode(juce::Slider::incDecButtonsDraggable_Vertical);
    repeatSlider->setColour(juce::Slider::textBoxTextColourId, baseColour);
    repeatSlider->setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0x00000000));
    repeatSlider->setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0x00000000));
    repeatSlider->setColour(juce::TextButton::textColourOnId, baseColour);
    repeatSlider->setLookAndFeel(&_tempoSliderLookAndFeel);
    repeatSlider->onValueChange = [this]() {
        _processor.setStepSeqStepRepeat(_seqIndex, 0, _stepIndex, static_cast<int>(repeatSlider->getValue()));
        refresh();
        if (_onChanged) {
            _onChanged();
        }
    };

    _tempoSliderLookAndFeel.setColour(juce::TextButton::textColourOnId, baseColour);

    refresh();
}

ModulationBarStepSequencer::StepCell::~StepCell() {
    reverseButton->setLookAndFeel(nullptr);
}

void ModulationBarStepSequencer::StepCell::resized() {
    juce::Rectangle<int> area = getLocalBounds();
    constexpr int REPEAT_HEIGHT {16};
    constexpr int REVERSE_HEIGHT {16};

    repeatSlider->setBounds(area.removeFromTop(REPEAT_HEIGHT));
    reverseButton->setBounds(area.removeFromTop(REVERSE_HEIGHT));
    waveDisplay->setBounds(area);
}

void ModulationBarStepSequencer::StepCell::refresh() {
    const double val = ModelInterface::getStepSeqStepValue(_processor.manager, _seqIndex, 0, _stepIndex);
    const int shape = ModelInterface::getStepSeqStepShape(_processor.manager, _seqIndex, 0, _stepIndex);
    const bool reverse = ModelInterface::getStepSeqStepReverse(_processor.manager, _seqIndex, 0, _stepIndex);
    const int repeat = ModelInterface::getStepSeqStepRepeat(_processor.manager, _seqIndex, 0, _stepIndex);
    const double lengthMultiplier = ModelInterface::getStepSeqStepLengthMultiplier(_processor.manager, _seqIndex, 0, _stepIndex);

    waveDisplay->setStepState(val, shape, reverse, repeat, lengthMultiplier);
    reverseButton->setToggleState(reverse, juce::dontSendNotification);
    repeatSlider->setValue(repeat, juce::dontSendNotification);
}

void ModulationBarStepSequencer::StepCell::setPlaying(bool playing) {
    waveDisplay->setPlaying(playing);
}

double ModulationBarStepSequencer::StepCell::getLengthMultiplier() const {
    return ModelInterface::getStepSeqStepLengthMultiplier(_processor.manager, _seqIndex, 0, _stepIndex);
}

// ===========================================================================
// ModulationBarStepSequencer

ModulationBarStepSequencer::ModulationBarStepSequencer(SyndicateAudioProcessor& processor, int seqIndex) :
        _processor(processor), _seqIndex(seqIndex) {

    namespace SP = WECore::StepSeq::Parameters;
    constexpr double INTERVAL {0.01};

    freqSlider.reset(new ModulatableParameter(
        ModelInterface::getStepSeqFreqModulationSources(_processor.manager, _seqIndex),
        [&](ModulationSourceDefinition definition) { _processor.addSourceToStepSeqFreq(_seqIndex, definition); },
        [&](ModulationSourceDefinition definition) { _processor.removeSourceFromStepSeqFreq(_seqIndex, definition); },
        [&](int sourceIndex, float value) { _processor.setStepSeqFreqModulationAmount(_seqIndex, sourceIndex, value); },
        [&]() { return ModelInterface::getStepSeqModulatedFreqValue(_processor.manager, _seqIndex); },
        "Seq Freq Slider",
        "Seq Freq Label",
        "Rate"));
    addAndMakeVisible(freqSlider.get());
    freqSlider->slider->setTooltip(TRANS("Number of steps per second"));
    freqSlider->slider->setRange(SP::FREQ.minValue, SP::FREQ.maxValue, INTERVAL);
    freqSlider->slider->setDoubleClickReturnValue(true, SP::FREQ.defaultValue);
    freqSlider->slider->setLookAndFeel(&_sliderLookAndFeel);
    freqSlider->slider->setColour(juce::Slider::rotarySliderFillColourId, baseColour);
    freqSlider->slider->onValueChange = [this]() {
        _processor.setStepSeqFreq(_seqIndex, freqSlider->slider->getValue());
    };

    depthSlider.reset(new ModulatableParameter(
        ModelInterface::getStepSeqDepthModulationSources(_processor.manager, _seqIndex),
        [&](ModulationSourceDefinition definition) { _processor.addSourceToStepSeqDepth(_seqIndex, definition); },
        [&](ModulationSourceDefinition definition) { _processor.removeSourceFromStepSeqDepth(_seqIndex, definition); },
        [&](int sourceIndex, float value) { _processor.setStepSeqDepthModulationAmount(_seqIndex, sourceIndex, value); },
        [&]() { return ModelInterface::getStepSeqModulatedDepthValue(_processor.manager, _seqIndex); },
        "Seq Depth Slider",
        "Seq Depth Label",
        "Depth"));
    addAndMakeVisible(depthSlider.get());
    depthSlider->slider->setTooltip(TRANS("Depth of the sequencer"));
    depthSlider->slider->setRange(SP::DEPTH.minValue, SP::DEPTH.maxValue, INTERVAL);
    depthSlider->slider->setDoubleClickReturnValue(true, SP::DEPTH.defaultValue);
    depthSlider->slider->setLookAndFeel(&_sliderLookAndFeel);
    depthSlider->slider->setColour(juce::Slider::rotarySliderFillColourId, baseColour);
    depthSlider->slider->onValueChange = [this]() {
        _processor.setStepSeqDepth(_seqIndex, depthSlider->slider->getValue());
    };

    tempoSyncButton.reset(new juce::TextButton("Seq Tempo Sync Button"));
    addAndMakeVisible(tempoSyncButton.get());
    tempoSyncButton->setTooltip(TRANS("Sync step rate to host tempo"));
    tempoSyncButton->setButtonText(TRANS("Tempo"));
    tempoSyncButton->setLookAndFeel(&_buttonLookAndFeel);
    tempoSyncButton->setColour(UIUtils::ToggleButtonLookAndFeel::backgroundColour, UIUtils::slotBackgroundColour);
    tempoSyncButton->setColour(UIUtils::ToggleButtonLookAndFeel::highlightColour, baseColour);
    tempoSyncButton->setColour(UIUtils::ToggleButtonLookAndFeel::disabledColour, UIUtils::deactivatedColour);
    tempoSyncButton->addListener(this);

    tempoNumerSlider.reset(new juce::Slider("Seq Tempo Numer Slider"));
    addAndMakeVisible(tempoNumerSlider.get());
    tempoNumerSlider->setTooltip(TRANS("Numerator of the note length of a step"));
    tempoNumerSlider->setRange(SP::TEMPONUMER.minValue, SP::TEMPONUMER.maxValue, 1);
    tempoNumerSlider->setSliderStyle(juce::Slider::IncDecButtons);
    tempoNumerSlider->setTextBoxStyle(juce::Slider::TextBoxLeft, false, 40, 20);
    tempoNumerSlider->setColour(juce::Slider::textBoxTextColourId, baseColour);
    tempoNumerSlider->setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0x00000000));
    tempoNumerSlider->setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0x00000000));
    tempoNumerSlider->setColour(juce::TextButton::textColourOnId, baseColour);
    tempoNumerSlider->setIncDecButtonsMode(juce::Slider::incDecButtonsDraggable_Vertical);
    tempoNumerSlider->setLookAndFeel(&_tempoSliderLookAndFeel);
    tempoNumerSlider->addListener(this);

    tempoDenomSlider.reset(new juce::Slider("Seq Tempo Denom Slider"));
    addAndMakeVisible(tempoDenomSlider.get());
    tempoDenomSlider->setTooltip(TRANS("Denominator of the note length of a step"));
    tempoDenomSlider->setRange(SP::TEMPODENOM.minValue, SP::TEMPODENOM.maxValue, 1);
    tempoDenomSlider->setSliderStyle(juce::Slider::IncDecButtons);
    tempoDenomSlider->setTextBoxStyle(juce::Slider::TextBoxLeft, false, 40, 20);
    tempoDenomSlider->setColour(juce::Slider::textBoxTextColourId, baseColour);
    tempoDenomSlider->setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0x00000000));
    tempoDenomSlider->setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0x00000000));
    tempoDenomSlider->setColour(juce::TextButton::textColourOnId, baseColour);
    tempoDenomSlider->setIncDecButtonsMode(juce::Slider::incDecButtonsDraggable_Vertical);
    tempoDenomSlider->setLookAndFeel(&_tempoSliderLookAndFeel);
    tempoDenomSlider->addListener(this);

    stepCountSlider.reset(new juce::Slider("Seq Step Count Slider"));
    addAndMakeVisible(stepCountSlider.get());
    stepCountSlider->setTooltip(TRANS("Number of steps"));
    stepCountSlider->setRange(1, 64, 1);
    stepCountSlider->setSliderStyle(juce::Slider::IncDecButtons);
    stepCountSlider->setTextBoxStyle(juce::Slider::TextBoxLeft, false, 40, 20);
    stepCountSlider->setColour(juce::Slider::textBoxTextColourId, baseColour);
    stepCountSlider->setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0x00000000));
    stepCountSlider->setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0x00000000));
    stepCountSlider->setColour(juce::TextButton::textColourOnId, baseColour);
    stepCountSlider->setIncDecButtonsMode(juce::Slider::incDecButtonsDraggable_Vertical);
    stepCountSlider->setLookAndFeel(&_tempoSliderLookAndFeel);
    stepCountSlider->addListener(this);

    settingsViewButton.reset(new juce::TextButton("Settings View Button"));
    addAndMakeVisible(settingsViewButton.get());
    settingsViewButton->setButtonText("");
    settingsViewButton->setTooltip(TRANS("Show settings"));
    settingsViewButton->setClickingTogglesState(false);
    settingsViewButton->setLookAndFeel(&_settingsIconButtonLookAndFeel);
    settingsViewButton->setColour(UIUtils::ToggleButtonLookAndFeel::backgroundColour, UIUtils::slotBackgroundColour);
    settingsViewButton->setColour(UIUtils::ToggleButtonLookAndFeel::highlightColour, baseColour);
    settingsViewButton->setColour(UIUtils::ToggleButtonLookAndFeel::disabledColour, UIUtils::deactivatedColour);
    settingsViewButton->addListener(this);

    sequencerViewButton.reset(new juce::TextButton("Sequencer View Button"));
    addAndMakeVisible(sequencerViewButton.get());
    sequencerViewButton->setButtonText("");
    sequencerViewButton->setTooltip(TRANS("Show step grid"));
    sequencerViewButton->setClickingTogglesState(false);
    sequencerViewButton->setLookAndFeel(&_gridIconButtonLookAndFeel);
    sequencerViewButton->setColour(UIUtils::ToggleButtonLookAndFeel::backgroundColour, UIUtils::slotBackgroundColour);
    sequencerViewButton->setColour(UIUtils::ToggleButtonLookAndFeel::highlightColour, baseColour);
    sequencerViewButton->setColour(UIUtils::ToggleButtonLookAndFeel::disabledColour, UIUtils::deactivatedColour);
    sequencerViewButton->addListener(this);

    shapesLabel.reset(new juce::Label({}, TRANS("Shapes")));
    shapesLabel->setJustificationType(juce::Justification::centred);
    shapesLabel->setFont(juce::Font(14.0f));
    shapesLabel->setColour(juce::Label::textColourId, baseColour);
    addAndMakeVisible(shapesLabel.get());

    stepsLabel.reset(new juce::Label({}, TRANS("Steps")));
    stepsLabel->setJustificationType(juce::Justification::centred);
    stepsLabel->setFont(juce::Font(14.0f));
    stepsLabel->setColour(juce::Label::textColourId, baseColour);
    addAndMakeVisible(stepsLabel.get());

    for (int shape = 1; shape <= 4; ++shape) {
        auto cell = std::make_unique<ShapePaletteCell>(shape, [this](int selectedShape) {
            _selectedShape = selectedShape;
            _shapeAppliedOnce = false;
            _updateShapePaletteSelection();
        });
        cell->setTooltip(
#if JUCE_IOS
            "Select a shape and then a step to apply it."
#else
            "Select a shape and then a step to apply it. Hold " + UIUtils::getCmdKeyName() + " to apply to multiple steps."
#endif
);
        addAndMakeVisible(cell.get());
        shapePaletteCells.push_back(std::move(cell));
    }
    _updateShapePaletteSelection();

    stepGridView.reset(new juce::Viewport());
    stepGridContainer.reset(new juce::Component());
    stepGridView->setViewedComponent(stepGridContainer.get(), false);
    stepGridView->setScrollBarsShown(false, true);
    stepGridView->setScrollOnDragMode(juce::Viewport::ScrollOnDragMode::never);
    stepGridView->getHorizontalScrollBar().setColour(juce::ScrollBar::thumbColourId, baseColour.withAlpha(0.5f));
    stepGridView->getHorizontalScrollBar().setColour(juce::ScrollBar::backgroundColourId, juce::Colour(0x00000000));
    stepGridView->getHorizontalScrollBar().setColour(juce::ScrollBar::trackColourId, juce::Colour(0x00000000));
    addAndMakeVisible(stepGridView.get());

    // Load UI state
    freqSlider->slider->setValue(ModelInterface::getStepSeqFreq(_processor.manager, _seqIndex), juce::dontSendNotification);
    depthSlider->slider->setValue(ModelInterface::getStepSeqDepth(_processor.manager, _seqIndex), juce::dontSendNotification);
    tempoSyncButton->setToggleState(ModelInterface::getStepSeqTempoSyncSwitch(_processor.manager, _seqIndex), juce::dontSendNotification);
    tempoNumerSlider->setValue(ModelInterface::getStepSeqTempoNumer(_processor.manager, _seqIndex), juce::dontSendNotification);
    tempoDenomSlider->setValue(ModelInterface::getStepSeqTempoDenom(_processor.manager, _seqIndex), juce::dontSendNotification);

    _tempoSliderLookAndFeel.setColour(juce::TextButton::textColourOnId, baseColour);

    // Restore view state
    const std::vector<bool>& seqViews = _processor.mainWindowState.seqShowingSequencerView;
    if (_seqIndex < static_cast<int>(seqViews.size())) {
        _showingSequencerView = seqViews[_seqIndex];
    }

    _rebuildStepGrid();
    _updateTempoToggles();
    _updateStepCount();
    _updateViewVisibility();

}

ModulationBarStepSequencer::~ModulationBarStepSequencer() {
    // Save view state
    std::vector<bool>& seqViews = _processor.mainWindowState.seqShowingSequencerView;
    if (_seqIndex >= static_cast<int>(seqViews.size())) {
        seqViews.resize(_seqIndex + 1, true);
    }
    seqViews[_seqIndex] = _showingSequencerView;

    stopTimer();
    freqSlider->slider->setLookAndFeel(nullptr);
    depthSlider->slider->setLookAndFeel(nullptr);
    tempoSyncButton->setLookAndFeel(nullptr);
    tempoNumerSlider->setLookAndFeel(nullptr);
    tempoDenomSlider->setLookAndFeel(nullptr);
    stepCountSlider->setLookAndFeel(nullptr);
    settingsViewButton->setLookAndFeel(nullptr);
    sequencerViewButton->setLookAndFeel(nullptr);

    freqSlider = nullptr;
    depthSlider = nullptr;
}

void ModulationBarStepSequencer::resized() {
    juce::Rectangle<int> availableArea = getLocalBounds();
    availableArea.reduce(4, 4);

    constexpr int BUTTON_HEIGHT {24};
    constexpr int VIEW_BUTTON_WIDTH {20};
    constexpr int SCROLL_BAR_HEIGHT {12};
    constexpr int STEP_CONTROL_WIDTH {64};

    // Far-left strip: view toggle buttons
    juce::Rectangle<int> viewButtonStrip = availableArea.removeFromLeft(VIEW_BUTTON_WIDTH);
    sequencerViewButton->setBounds(viewButtonStrip.removeFromTop(viewButtonStrip.getHeight() / 2));
    settingsViewButton->setBounds(viewButtonStrip);

    if (_showingSequencerView) {
        constexpr int SHAPE_CELL_HEIGHT {22};
        constexpr int SHAPE_CELL_COLS {2};
        constexpr int PALETTE_MARGIN {4};
        constexpr int STEP_COUNT_HEIGHT {40};

        // Left strip: 2x2 shape palette, then step count slider
        juce::Rectangle<int> stepControlStrip = availableArea.removeFromLeft(STEP_CONTROL_WIDTH);

        constexpr int LABEL_HEIGHT {14};
        const int numRows = static_cast<int>(shapePaletteCells.size()) / SHAPE_CELL_COLS;
        const int paletteHeight = numRows * SHAPE_CELL_HEIGHT + PALETTE_MARGIN * 2;
        const int totalContentHeight = LABEL_HEIGHT * 2 + paletteHeight + STEP_COUNT_HEIGHT;
        const int gap = (stepControlStrip.getHeight() - totalContentHeight) / 3;

        stepControlStrip.removeFromTop(gap);
        shapesLabel->setBounds(stepControlStrip.removeFromTop(LABEL_HEIGHT));
        juce::Rectangle<int> paletteArea = stepControlStrip.removeFromTop(paletteHeight).reduced(PALETTE_MARGIN);
        const int cellWidth = paletteArea.getWidth() / SHAPE_CELL_COLS;
        for (int i = 0; i < static_cast<int>(shapePaletteCells.size()); ++i) {
            shapePaletteCells[i]->setBounds(paletteArea.getX() + (i % SHAPE_CELL_COLS) * cellWidth,
                                            paletteArea.getY() + (i / SHAPE_CELL_COLS) * SHAPE_CELL_HEIGHT,
                                            cellWidth,
                                            SHAPE_CELL_HEIGHT);
        }
        stepControlStrip.removeFromTop(gap);
        stepsLabel->setBounds(stepControlStrip.removeFromTop(LABEL_HEIGHT));
        stepCountSlider->setBounds(stepControlStrip.removeFromTop(STEP_COUNT_HEIGHT));

        // Viewport fills the rest
        stepGridView->setBounds(availableArea);

        const int gridHeight = availableArea.getHeight() - SCROLL_BAR_HEIGHT;

        // Layout step cells horizontally with widths based on length multiplier
        int x = 0;
        const int numSteps {static_cast<int>(stepCells.size())};
        for (int i = 0; i < numSteps; ++i) {
            const double multipler {stepCells[i]->getLengthMultiplier()};

            // Use sqrt for smaller sizes so they're not too narrow
            const double scale = multipler < 1.0 ? std::sqrt(multipler) : multipler;

            const int width {juce::roundToInt(STEP_CELL_WIDTH * scale)};
            stepCells[i]->setBounds(x, 0, width, gridHeight);
            x += width;
        }
        stepGridContainer->setSize(x, gridHeight);
    } else {
        // Settings: freq/tempo on left, depth on right, space between grows with component width
        constexpr int MAX_PARAM_WIDTH {100};
        const int paramWidth = std::min(availableArea.getWidth() / 4, MAX_PARAM_WIDTH);

        const int excessWidth = availableArea.getWidth() - 2 * paramWidth;
        availableArea.removeFromLeft(excessWidth / 3);
        availableArea.removeFromRight(excessWidth / 3);

        juce::Rectangle<int> freqArea = availableArea.removeFromLeft(paramWidth);
        juce::Rectangle<int> depthArea = availableArea.removeFromRight(paramWidth);

        tempoSyncButton->setBounds(freqArea.removeFromTop(BUTTON_HEIGHT));

        juce::Rectangle<int> tempoSlidersArea = freqArea;
        freqSlider->setBounds(freqArea);

        juce::FlexBox tempoSlidersFlexBox;
        tempoSlidersFlexBox.flexDirection = juce::FlexBox::Direction::column;
        tempoSlidersFlexBox.flexWrap = juce::FlexBox::Wrap::wrap;
        tempoSlidersFlexBox.justifyContent = juce::FlexBox::JustifyContent::spaceAround;
        tempoSlidersFlexBox.alignContent = juce::FlexBox::AlignContent::center;
        tempoSlidersFlexBox.items.add(juce::FlexItem(*tempoNumerSlider.get()).withMinWidth(tempoSlidersArea.getWidth() * 0.75f).withMinHeight(40));
        tempoSlidersFlexBox.items.add(juce::FlexItem(*tempoDenomSlider.get()).withMinWidth(tempoSlidersArea.getWidth() * 0.75f).withMinHeight(40));
        tempoSlidersFlexBox.performLayout(tempoSlidersArea);

        depthSlider->setBounds(depthArea.withTrimmedTop(BUTTON_HEIGHT));
    }
}

void ModulationBarStepSequencer::buttonClicked(juce::Button* buttonThatWasClicked) {
    if (buttonThatWasClicked == sequencerViewButton.get()) {
        _showingSequencerView = true;
        _updateViewVisibility();
        resized();
    } else if (buttonThatWasClicked == settingsViewButton.get()) {
        _showingSequencerView = false;
        _updateViewVisibility();
        resized();
    } else if (buttonThatWasClicked == tempoSyncButton.get()) {
        tempoSyncButton->setToggleState(!tempoSyncButton->getToggleState(), juce::dontSendNotification);
        _processor.setStepSeqTempoSyncSwitch(_seqIndex, tempoSyncButton->getToggleState());
        _updateTempoToggles();
    }
}

void ModulationBarStepSequencer::sliderValueChanged(juce::Slider* sliderThatWasMoved) {
    if (sliderThatWasMoved == tempoNumerSlider.get()) {
        _processor.setStepSeqTempoNumer(_seqIndex, static_cast<int>(tempoNumerSlider->getValue()));
    } else if (sliderThatWasMoved == tempoDenomSlider.get()) {
        _processor.setStepSeqTempoDenom(_seqIndex, static_cast<int>(tempoDenomSlider->getValue()));
    } else if (sliderThatWasMoved == stepCountSlider.get()) {
        int currentCount = ModelInterface::getStepSeqNumSteps(_processor.manager, _seqIndex, 0);
        const int newCount = static_cast<int>(stepCountSlider->getValue());
        while (currentCount < newCount) {
            _processor.addStepSeqStep(_seqIndex, 0);
            ++currentCount;
        }
        while (currentCount > newCount) {
            _processor.removeStepSeqStep(_seqIndex, 0);
            --currentCount;
        }
        _rebuildStepGrid();
        resized();
    }
}

void ModulationBarStepSequencer::timerCallback() {
    // Poll playing step and update highlight
    const int playingStep = ModelInterface::getStepSeqCurrentStep(_processor.manager, _seqIndex);
    if (playingStep != _lastPlayingStep) {
        if (_lastPlayingStep >= 0 && _lastPlayingStep < static_cast<int>(stepCells.size())) {
            stepCells[_lastPlayingStep]->setPlaying(false);
        }
        if (playingStep >= 0 && playingStep < static_cast<int>(stepCells.size())) {
            stepCells[playingStep]->setPlaying(true);
        }
        _lastPlayingStep = playingStep;
    }

    // Poll for ctrl-key release when a shape is being multi-applied
    if (_selectedShape != 0 && _shapeAppliedOnce) {
        const auto mods = juce::ModifierKeys::getCurrentModifiers();
        if (!mods.isCommandDown()) {
            _selectedShape = 0;
            _shapeAppliedOnce = false;
            _updateShapePaletteSelection();
        }
    }
}

void ModulationBarStepSequencer::_rebuildStepGrid() {
    _lastPlayingStep = -1;
    stepCells.clear();

    const int numSteps = ModelInterface::getStepSeqNumSteps(_processor.manager, _seqIndex, 0);
    for (int i = 0; i < numSteps; ++i) {
        auto cell = std::make_unique<StepCell>(
            _processor, _seqIndex, i,
            [this]() { resized(); },
            [this](bool cmdHeld) { return _getShapeForClick(cmdHeld); });
        stepGridContainer->addAndMakeVisible(cell.get());
        stepCells.push_back(std::move(cell));
    }
}

void ModulationBarStepSequencer::_updateTempoToggles() {
    if (_showingSequencerView) {
        freqSlider->setVisible(false);
        tempoNumerSlider->setVisible(false);
        tempoDenomSlider->setVisible(false);
    } else if (tempoSyncButton->getToggleState()) {
        freqSlider->setVisible(false);
        tempoNumerSlider->setVisible(true);
        tempoDenomSlider->setVisible(true);
    } else {
        freqSlider->setVisible(true);
        tempoNumerSlider->setVisible(false);
        tempoDenomSlider->setVisible(false);
    }
}

void ModulationBarStepSequencer::_updateStepCount() {
    const int numSteps = ModelInterface::getStepSeqNumSteps(_processor.manager, _seqIndex, 0);
    stepCountSlider->setValue(numSteps, juce::dontSendNotification);
}

void ModulationBarStepSequencer::_updateViewVisibility() {
    // Settings view components
    depthSlider->setVisible(!_showingSequencerView);
    tempoSyncButton->setVisible(!_showingSequencerView);

    // Sequencer view components
    stepCountSlider->setVisible(_showingSequencerView);
    stepGridView->setVisible(_showingSequencerView);
    for (auto& cell : shapePaletteCells) {
        cell->setVisible(_showingSequencerView);
    }
    shapesLabel->setVisible(_showingSequencerView);
    stepsLabel->setVisible(_showingSequencerView);

    // View button highlight states
    sequencerViewButton->setToggleState(_showingSequencerView, juce::dontSendNotification);
    settingsViewButton->setToggleState(!_showingSequencerView, juce::dontSendNotification);

    // Run timer only while sequencer view is visible
    if (_showingSequencerView) {
        startTimerHz(20);
    } else {
        stopTimer();

        // Clear any playing highlight when leaving sequencer view
        if (_lastPlayingStep >= 0 && _lastPlayingStep < static_cast<int>(stepCells.size())) {
            stepCells[_lastPlayingStep]->setPlaying(false);
        }
        _lastPlayingStep = -1;
    }

    // Freq/tempo visibility depends on both view and tempo sync state
    _updateTempoToggles();
}

void ModulationBarStepSequencer::_updateShapePaletteSelection() {
    for (int i = 0; i < static_cast<int>(shapePaletteCells.size()); ++i) {
        shapePaletteCells[i]->setSelected((i + 1) == _selectedShape);
    }
}

// TODO: should return enum
// TODO: check if this can be simplified
int ModulationBarStepSequencer::_getShapeForClick(bool cmdHeld) {
    if (_selectedShape == 0) {
        return 0;
    }

    if (cmdHeld) {
        // Command held: apply and keep selection active
        _shapeAppliedOnce = true;
        return _selectedShape;
    }

    if (_shapeAppliedOnce) {
        // Already applied at least once (while Command was held), now Command released — deactivate
        _selectedShape = 0;
        _shapeAppliedOnce = false;
        _updateShapePaletteSelection();
        return 0;
    }

    // First application without Command — apply and deactivate
    const int shape = _selectedShape;
    _selectedShape = 0;
    _shapeAppliedOnce = false;
    _updateShapePaletteSelection();
    return shape;
}

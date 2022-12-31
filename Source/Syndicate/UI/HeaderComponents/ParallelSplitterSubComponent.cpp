#include "UIUtils.h"

#include "ParallelSplitterSubComponent.h"
#include "SplitterMutators.hpp"

ParallelSplitterSubComponent::ParallelSplitterSubComponent(SyndicateAudioProcessor& processor,
                                                           juce::Component* extensionComponent,
                                                           UIUtils::LinkedScrollView* graphView)
    : _processor(processor), _graphView(graphView) {
    addChainBtn.reset(new juce::TextButton("Add Chain Button"));
    extensionComponent->addAndMakeVisible(addChainBtn.get());
    addChainBtn->setButtonText(TRANS("+ Chain"));
    addChainBtn->setTooltip("Adds another parallel chain");
    addChainBtn->setLookAndFeel(&_buttonLookAndFeel);
    addChainBtn->setColour(juce::TextButton::buttonOnColourId, UIUtils::neutralControlColour);
    addChainBtn->setColour(juce::TextButton::textColourOnId, UIUtils::neutralControlColour);
    addChainBtn->setColour(juce::TextButton::textColourOffId, UIUtils::neutralControlColour);
    addChainBtn->addListener(this);

    _viewPort.reset(new UIUtils::LinkedScrollView());
    _viewPort->setViewedComponent(new juce::Component());
    _viewPort->setScrollBarsShown(false, false, false, true);
    _viewPort->getHorizontalScrollBar().setColour(juce::ScrollBar::ColourIds::backgroundColourId, juce::Colour(0x00000000));
    _viewPort->getHorizontalScrollBar().setColour(juce::ScrollBar::ColourIds::thumbColourId, UIUtils::neutralHighlightColour.withAlpha(0.5f));
    _viewPort->getHorizontalScrollBar().setColour(juce::ScrollBar::ColourIds::trackColourId, juce::Colour(0x00000000));
    addAndMakeVisible(_viewPort.get());
    _viewPort->setBounds(getLocalBounds());
    _viewPort->setOtherView(_graphView);
    _graphView->setOtherView(_viewPort.get());
}

ParallelSplitterSubComponent::~ParallelSplitterSubComponent() {
    addChainBtn->setLookAndFeel(nullptr);
    _graphView->setOtherView(nullptr);

    addChainBtn = nullptr;
}

void ParallelSplitterSubComponent::resized() {
    // Resize within the extension
    juce::Rectangle<int> extensionArea = addChainBtn->getParentComponent()->getLocalBounds();
    constexpr int BUTTON_HEIGHT {24};
    extensionArea.reduce(4, (extensionArea.getHeight() - BUTTON_HEIGHT) / 2);
    addChainBtn->setBounds(extensionArea);

    // Resize within the header component
    _viewPort->setBounds(getLocalBounds());
    _rebuildHeader();
}

void ParallelSplitterSubComponent::buttonClicked(juce::Button* buttonThatWasClicked) {
    if (buttonThatWasClicked == addChainBtn.get()) {
        _processor.addParallelChain();
    }
}

void ParallelSplitterSubComponent::onParameterUpdate() {
    // Lock here because we're in onParameterUpdate, so the UI thread could change something while
    // we're here
    WECore::AudioSpinLock lock(_processor.pluginSplitterMutex);

    if (_chainButtons.size() != SplitterMutators::getNumChains(_processor.pluginSplitter)) {
        _rebuildHeader();
    }

    for (size_t index {0}; index < _chainButtons.size(); index++) {
        _chainButtons[index]->onParameterUpdate();
    }
}

void ParallelSplitterSubComponent::_rebuildHeader() {
    // Set up the scrollable view
    const size_t numChains {SplitterMutators::getNumChains(_processor.pluginSplitter)};
    const int scrollPosition {_viewPort->getViewPositionX()};
    const int scrollableWidth {std::max(getWidth(), static_cast<int>(UIUtils::CHAIN_WIDTH * numChains))};
    const int scrollableHeight {getHeight()};
    _viewPort->getViewedComponent()->setBounds(juce::Rectangle<int>(scrollableWidth, scrollableHeight));

    _chainButtons.clear();

    for (size_t index {0}; index < numChains; index++) {
        if (numChains == 1) {
            // Don't provide a callback if there's only one chain - it can't be removed
            _chainButtons.emplace_back(std::make_unique<ChainButtonsComponent>(_processor.chainParameters[index]));
        } else {
            _chainButtons.emplace_back(std::make_unique<ChainButtonsComponent>(
                _processor.chainParameters[index],
                [&, index]() { _processor.removeParallelChain(index); }
            ));
        }

        _chainButtons[index]->chainLabel->setText("Chain " + juce::String(index + 1), juce::dontSendNotification);
        _viewPort->getViewedComponent()->addAndMakeVisible(_chainButtons[index].get());
    }

    _viewPort->setViewPosition(scrollPosition, 0);

    juce::Rectangle<int> scrollableArea = _viewPort->getViewedComponent()->getLocalBounds();
    if (scrollableWidth == getWidth()) {
        // If the scrollable area is the same as the width we need to remove from the left to
        // make the chains centred properly (otherwise we just left align them since the scrolling
        // will make it appear correct)
        scrollableArea.removeFromLeft(UIUtils::getChainXPos(0, numChains, getWidth()));
    }

    for (std::unique_ptr<ChainButtonsComponent>& chainButton : _chainButtons) {
        chainButton->setBounds(scrollableArea.removeFromLeft(UIUtils::CHAIN_WIDTH));
    }
}

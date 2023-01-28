#include "GraphViewComponent.h"
#include "UIUtils.h"
#include "SplitterMutators.hpp"

namespace {
    std::vector<int> getChainViewScrollPositions(const std::vector<std::unique_ptr<ChainViewComponent>>& chainViews) {
        std::vector<int> scrollPositions;
        for (const std::unique_ptr<ChainViewComponent>& chainView : chainViews) {
            scrollPositions.push_back(chainView->getScrollPosition());
        }

        return scrollPositions;
    }

    void setChainViewScrollPositions(std::vector<std::unique_ptr<ChainViewComponent>>& chainViews, const std::vector<int>& scrollPositions) {
        const size_t numChains {
            std::min(chainViews.size(), scrollPositions.size())
        };

        for (int chainIndex {0}; chainIndex < numChains; chainIndex++) {
            chainViews[chainIndex]->setScrollPosition(scrollPositions[chainIndex]);
        }
    }
}

GraphViewComponent::GraphViewComponent(SyndicateAudioProcessor& processor)
        : _processor(processor),
          _pluginSelectionInterface(processor),
          _pluginModulationInterface(processor, this),
          _hasRestoredScroll(false) {

    _viewPort.reset(new UIUtils::LinkedScrollView());
    _viewPort->setViewedComponent(new juce::Component());
    _viewPort->setScrollBarsShown(false, true);
    _viewPort->getHorizontalScrollBar().setColour(juce::ScrollBar::ColourIds::backgroundColourId, juce::Colour(0x00000000));
    _viewPort->getHorizontalScrollBar().setColour(juce::ScrollBar::ColourIds::thumbColourId, UIUtils::neutralHighlightColour.withAlpha(0.5f));
    _viewPort->getHorizontalScrollBar().setColour(juce::ScrollBar::ColourIds::trackColourId, juce::Colour(0x00000000));
    addAndMakeVisible(_viewPort.get());
}

GraphViewComponent::~GraphViewComponent() {
    // Store scroll positions
    _processor.mainWindowState.graphViewScrollPosition = _viewPort->getViewPositionX();
    _processor.mainWindowState.chainViewScrollPositions = getChainViewScrollPositions(_chainViews);

    _viewPort = nullptr;
}

void GraphViewComponent::resized() {
    const int scrollPosition {_viewPort->getViewPositionX()};

    _viewPort->setBounds(getLocalBounds());

    const int scrollableWidth {std::max(getWidth(), static_cast<int>(UIUtils::CHAIN_WIDTH * _chainViews.size()))};
    const int scrollableHeight {getHeight()};
    _viewPort->getViewedComponent()->setBounds(juce::Rectangle<int>(scrollableWidth, scrollableHeight));

    juce::Rectangle<int> scrollableArea = _viewPort->getViewedComponent()->getLocalBounds();
    if (scrollableWidth == getWidth()) {
        // If the scrollable area is the same as the width we need to remove from the left to
        // make the chains centred properly (otherwise we just left align them since the scrolling
        // will make it appear correct)
        scrollableArea.removeFromLeft(UIUtils::getChainXPos(0, _chainViews.size(), getWidth()));
    }

    for (std::unique_ptr<ChainViewComponent>& chainView : _chainViews) {
        chainView->setBounds(scrollableArea.removeFromLeft(UIUtils::CHAIN_WIDTH));
    }

    // Maintain the previous scroll position
    _viewPort->setViewPosition(scrollPosition, 0);
}

void GraphViewComponent::onParameterUpdate() {

    // Lock here because we're in onParameterUpdate, so the UI thread could change something while
    // we're here
    WECore::AudioSpinLock lock(_processor.pluginSplitterMutex);

    // Store the scroll positions of each chain
    std::vector<int> chainScrollPositions = getChainViewScrollPositions(_chainViews);

    _chainViews.clear();

    if (_processor.getSplitType() == SPLIT_TYPE::SERIES) {
        _chainViews.push_back(std::make_unique<ChainViewComponent>(0, _pluginSelectionInterface, _pluginModulationInterface));
        _viewPort->getViewedComponent()->addAndMakeVisible(_chainViews[0].get());

        if (_processor.pluginSplitter != nullptr) {
            _chainViews[0]->setPlugins(_processor.pluginSplitter->chains[0].chain);
        }
    } else if (_processor.getSplitType() == SPLIT_TYPE::PARALLEL) {
        for (size_t index {0}; index < SplitterMutators::getNumChains(_processor.pluginSplitter); index++) {
            _chainViews.push_back(std::make_unique<ChainViewComponent>(index, _pluginSelectionInterface, _pluginModulationInterface));
            _viewPort->getViewedComponent()->addAndMakeVisible(_chainViews[index].get());

            if (_processor.pluginSplitter != nullptr) {
                _chainViews[index]->setPlugins(_processor.pluginSplitter->chains[index].chain);
            }
        }
    } else if (_processor.getSplitType() == SPLIT_TYPE::MULTIBAND) {
        const int numChains {
            std::min(WECore::MONSTR::Parameters::NUM_BANDS.maxValue, static_cast<int>(SplitterMutators::getNumChains(_processor.pluginSplitter)))
        };
        for (size_t index {0}; index < numChains; index++) {
            _chainViews.push_back(std::make_unique<ChainViewComponent>(index, _pluginSelectionInterface, _pluginModulationInterface));
            _viewPort->getViewedComponent()->addAndMakeVisible(_chainViews[index].get());

            if (_processor.pluginSplitter != nullptr) {
                _chainViews[index]->setPlugins(_processor.pluginSplitter->chains[index].chain);
            }
        }
    } else if (_processor.getSplitType() == SPLIT_TYPE::LEFTRIGHT ||
               _processor.getSplitType() == SPLIT_TYPE::MIDSIDE) {
        _chainViews.push_back(std::make_unique<ChainViewComponent>(0, _pluginSelectionInterface, _pluginModulationInterface));
        _viewPort->getViewedComponent()->addAndMakeVisible(_chainViews[0].get());

        if (_processor.pluginSplitter != nullptr) {
            _chainViews[0]->setPlugins(_processor.pluginSplitter->chains[0].chain);
        }

        _chainViews.push_back(std::make_unique<ChainViewComponent>(1, _pluginSelectionInterface, _pluginModulationInterface));
        _viewPort->getViewedComponent()->addAndMakeVisible(_chainViews[1].get());

        if (_processor.pluginSplitter != nullptr) {
            _chainViews[1]->setPlugins(_processor.pluginSplitter->chains[1].chain);
        }
    }

    setChainViewScrollPositions(_chainViews, chainScrollPositions);

    resized();

    // We need to run this only once after the graph view has been constructed to restore the scroll
    // position to the same as before the UI was last closed
    if (!_hasRestoredScroll) {
        _hasRestoredScroll = true;

        // Graph horizontal scroll
        _viewPort->setViewPosition(_processor.mainWindowState.graphViewScrollPosition, 0);

        // Chains vertical scroll
        setChainViewScrollPositions(_chainViews, _processor.mainWindowState.chainViewScrollPositions);
    }
}

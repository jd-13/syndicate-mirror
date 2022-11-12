#include "GraphViewComponent.h"
#include "UIUtils.h"

GraphViewComponent::GraphViewComponent(SyndicateAudioProcessor& processor)
        : _processor(processor),
          _pluginSelectionInterface(processor),
          _pluginModulationInterface(processor, this) {
    setSize (572, 276);

    _viewPort.reset(new UIUtils::LinkedScrollView());
    _viewPort->setViewedComponent(new juce::Component());
    _viewPort->setScrollBarsShown(false, true);
    _viewPort->getHorizontalScrollBar().setColour(juce::ScrollBar::ColourIds::backgroundColourId, juce::Colour(0x00000000));
    _viewPort->getHorizontalScrollBar().setColour(juce::ScrollBar::ColourIds::thumbColourId, UIUtils::neutralHighlightColour.withAlpha(0.5f));
    _viewPort->getHorizontalScrollBar().setColour(juce::ScrollBar::ColourIds::trackColourId, juce::Colour(0x00000000));
    addAndMakeVisible(_viewPort.get());
    _viewPort->setBounds(getLocalBounds());
}

GraphViewComponent::~GraphViewComponent() {
    _viewPort = nullptr;
}

void GraphViewComponent::onParameterUpdate() {

    // Lock here because we're in onParameterUpdate, so the UI thread could change something while
    // we're here
    WECore::AudioSpinLock lock(_processor.pluginSplitterMutex);

    // Store the scroll positions of each chain
    std::vector<int> chainScrollPositions;
    for (std::unique_ptr<ChainViewComponent>& chainView : _chainViews) {
        chainScrollPositions.push_back(chainView->getScrollPosition());
    }

    _chainViews.clear();

    switch (_processor.getSplitType()) {
        case SPLIT_TYPE::SERIES:
            _chainViews.push_back(std::make_unique<ChainViewComponent>(0, _pluginSelectionInterface, _pluginModulationInterface));
            _viewPort->getViewedComponent()->addAndMakeVisible(_chainViews[0].get());

            if (_processor.pluginSplitter != nullptr) {
                _chainViews[0]->setPlugins(_processor.pluginSplitter->getChain(0).get());
            }
            break;
        case SPLIT_TYPE::PARALLEL:
        case SPLIT_TYPE::MULTIBAND:
            for (size_t index {0}; index < _processor.pluginSplitter->getNumChains(); index++) {
                _chainViews.push_back(std::make_unique<ChainViewComponent>(index, _pluginSelectionInterface, _pluginModulationInterface));
                _viewPort->getViewedComponent()->addAndMakeVisible(_chainViews[index].get());

                if (_processor.pluginSplitter != nullptr) {
                    _chainViews[index]->setPlugins(_processor.pluginSplitter->getChain(index).get());
                }
            }
            break;
        case SPLIT_TYPE::LEFTRIGHT:
        case SPLIT_TYPE::MIDSIDE:
            _chainViews.push_back(std::make_unique<ChainViewComponent>(0, _pluginSelectionInterface, _pluginModulationInterface));
            _viewPort->getViewedComponent()->addAndMakeVisible(_chainViews[0].get());

            if (_processor.pluginSplitter != nullptr) {
                _chainViews[0]->setPlugins(_processor.pluginSplitter->getChain(0).get());
            }

            _chainViews.push_back(std::make_unique<ChainViewComponent>(1, _pluginSelectionInterface, _pluginModulationInterface));
            _viewPort->getViewedComponent()->addAndMakeVisible(_chainViews[1].get());

            if (_processor.pluginSplitter != nullptr) {
                _chainViews[1]->setPlugins(_processor.pluginSplitter->getChain(1).get());
            }
            break;
    }

    const int scrollPosition {_viewPort->getViewPositionX()};
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

    for (int chainIndex {0}; chainIndex < _chainViews.size(); chainIndex++) {
        if (chainScrollPositions.size() > chainIndex) {
            _chainViews[chainIndex]->setScrollPosition(chainScrollPositions[chainIndex]);
        }
    }
}

#include "GraphViewComponent.h"
#include "UIUtils.h"

GraphViewComponent::GraphViewComponent(SyndicateAudioProcessor& processor)
        : _processor(processor),
          _pluginSelectionInterface(processor),
          _pluginModulationInterface(processor, this) {
    setSize (572, 276);
}

void GraphViewComponent::onParameterUpdate() {

    // Lock here because we're in onParameterUpdate, so the UI thread could change something while
    // we're here
    WECore::AudioSpinLock lock(_processor.pluginSplitterMutex);
    _chainViews.clear();

    switch (_processor.getSplitType()) {
        case SPLIT_TYPE::SERIES:
            _chainViews.push_back(std::make_unique<ChainViewComponent>(0, _pluginSelectionInterface, _pluginModulationInterface));
            addAndMakeVisible(_chainViews[0].get());
            _chainViews[0]->setBounds(UIUtils::getChainXPos(0, 1, getWidth()),
                                      0,
                                      UIUtils::CHAIN_WIDTH,
                                      getHeight());

            if (_processor.pluginSplitter != nullptr) {
                _chainViews[0]->setPlugins(_processor.pluginSplitter->getChain(0).get());
            }
            break;
        case SPLIT_TYPE::PARALLEL:
        case SPLIT_TYPE::MULTIBAND:
            for (size_t index {0}; index < _processor.pluginSplitter->getNumChains(); index++) {
                _chainViews.push_back(std::make_unique<ChainViewComponent>(index, _pluginSelectionInterface, _pluginModulationInterface));
                addAndMakeVisible(_chainViews[index].get());
                _chainViews[index]->setBounds(UIUtils::getChainXPos(index, _processor.pluginSplitter->getNumChains(), getWidth()),
                                              0,
                                              UIUtils::CHAIN_WIDTH,
                                              getHeight());

                if (_processor.pluginSplitter != nullptr) {
                    _chainViews[index]->setPlugins(_processor.pluginSplitter->getChain(index).get());
                }
            }
            break;
        case SPLIT_TYPE::LEFTRIGHT:
        case SPLIT_TYPE::MIDSIDE:
            _chainViews.push_back(std::make_unique<ChainViewComponent>(0, _pluginSelectionInterface, _pluginModulationInterface));
            addAndMakeVisible(_chainViews[0].get());
            _chainViews[0]->setBounds(UIUtils::getChainXPos(0, 2, getWidth()),
                                      0,
                                      UIUtils::CHAIN_WIDTH,
                                      getHeight());

            if (_processor.pluginSplitter != nullptr) {
                _chainViews[0]->setPlugins(_processor.pluginSplitter->getChain(0).get());
            }

            _chainViews.push_back(std::make_unique<ChainViewComponent>(1, _pluginSelectionInterface, _pluginModulationInterface));
            addAndMakeVisible(_chainViews[1].get());
            _chainViews[1]->setBounds(UIUtils::getChainXPos(1, 2, getWidth()),
                                      0,
                                      UIUtils::CHAIN_WIDTH,
                                      getHeight());

            if (_processor.pluginSplitter != nullptr) {
                _chainViews[1]->setPlugins(_processor.pluginSplitter->getChain(1).get());
            }
            break;
    }
}

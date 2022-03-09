#include "UIUtils.h"

#include "ParallelSplitterSubComponent.h"

ParallelSplitterSubComponent::ParallelSplitterSubComponent(SyndicateAudioProcessor& processor)
    : _processor(processor) {
    addChainBtn.reset(new juce::TextButton("Add Chain Button"));
    addAndMakeVisible(addChainBtn.get());
    addChainBtn->setButtonText(TRANS("Add Chain"));
    addChainBtn->addListener(this);
}

ParallelSplitterSubComponent::~ParallelSplitterSubComponent() {
    addChainBtn = nullptr;
}

void ParallelSplitterSubComponent::resized() {
    addChainBtn->setBounds(448, 8, 112, 24);
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

    if (_chainButtons.size() != _processor.pluginSplitter->getNumChains()) {
        _rebuildHeader();
    }

    for (size_t index {0}; index < _chainButtons.size(); index++) {
        _chainButtons[index]->onParameterUpdate();
    }
}

void ParallelSplitterSubComponent::_rebuildHeader() {
    _chainButtons.clear();

    for (size_t index {0}; index < _processor.pluginSplitter->getNumChains(); index++) {
        if (_processor.pluginSplitter->getNumChains() == 1) {
            // Don't provide a callback if there's only one chain - it can't be removed
            _chainButtons.emplace_back(std::make_unique<ChainButtonsComponent>(_processor.chainParameters[index]));
        } else {
            _chainButtons.emplace_back(std::make_unique<ChainButtonsComponent>(
                _processor.chainParameters[index],
                [&, index]() { _processor.removeParallelChain(index); }
            ));
        }

        _chainButtons[index]->chainLabel->setText("Chain " + juce::String(index + 1), juce::dontSendNotification);

        addAndMakeVisible(_chainButtons[index].get());
        _chainButtons[index]->setBounds(UIUtils::getChainXPos(index, _processor.pluginSplitter->getNumChains(), getWidth()),
                                        0,
                                        UIUtils::CHAIN_WIDTH,
                                        getHeight());
    }
}

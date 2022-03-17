#include "UIUtils.h"

#include "ParallelSplitterSubComponent.h"

ParallelSplitterSubComponent::ParallelSplitterSubComponent(SyndicateAudioProcessor& processor, juce::Component* extensionComponent)
    : _processor(processor) {
    addChainBtn.reset(new juce::TextButton("Add Chain Button"));
    extensionComponent->addAndMakeVisible(addChainBtn.get());
    addChainBtn->setButtonText(TRANS("+ Chain"));
    addChainBtn->setTooltip("Adds another parallel chain");
    addChainBtn->setLookAndFeel(&_buttonLookAndFeel);
    addChainBtn->setColour(juce::TextButton::buttonOnColourId, UIUtils::neutralControlColour);
    addChainBtn->setColour(juce::TextButton::textColourOnId, UIUtils::neutralControlColour);
    addChainBtn->setColour(juce::TextButton::textColourOffId, UIUtils::neutralControlColour);
    addChainBtn->addListener(this);
}

ParallelSplitterSubComponent::~ParallelSplitterSubComponent() {
    addChainBtn->setLookAndFeel(nullptr);

    addChainBtn = nullptr;
}

void ParallelSplitterSubComponent::resized() {
    juce::Rectangle<int> extensionArea = addChainBtn->getParentComponent()->getLocalBounds();
    constexpr int BUTTON_HEIGHT {24};
    extensionArea.reduce(4, (extensionArea.getHeight() - BUTTON_HEIGHT) / 2);
    addChainBtn->setBounds(extensionArea);
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

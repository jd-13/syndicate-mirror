#include <JuceHeader.h>
#include "ChainParameters.h"
#include "BaseSlotComponent.h"
#include "PluginChain.h"
#include "PluginSelectionInterface.h"
#include "PluginModulationInterface.h"

/**
 * Manages a single chain of plugins.
 */
class ChainViewComponent : public juce::Component,
                           public juce::DragAndDropTarget {
public:
    ChainViewComponent(int chainNumber,
                       PluginSelectionInterface& pluginSelectionInterface,
                       PluginModulationInterface& pluginModulationInterface);
    ~ChainViewComponent();

    void setPlugins(PluginChain* newChain);

    void resized() override;

    void paint(juce::Graphics& g) override;

    bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override;
    void itemDragEnter(const SourceDetails& dragSourceDetails) override;
    void itemDragMove(const SourceDetails& dragSourceDetails) override;
    void itemDragExit(const SourceDetails& dragSourceDetails) override;
    void itemDropped(const SourceDetails& dragSourceDetails) override;

private:
    int _chainNumber;
    PluginSelectionInterface& _pluginSelectionInterface;
    PluginModulationInterface& _pluginModulationInterface;
    std::vector<std::unique_ptr<BaseSlotComponent>> _pluginSlots;
    std::unique_ptr<juce::Viewport> _viewPort;
    bool _shouldDrawDragHint;
    int _dragHintSlotNumber;

    int _dragCursorPositionToSlotNumber(juce::Point<int> cursorPosition);

};

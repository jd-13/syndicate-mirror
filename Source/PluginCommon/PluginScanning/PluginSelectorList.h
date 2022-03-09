/*
  ==============================================================================

    PluginSelectorList.h
    Created: 23 May 2021 12:39:35am
    Author:  Jack Devlin

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginScanClient.h"
#include "PluginSelectorListParameters.h"
#include "PluginSelectorState.h"

class PluginListSorter {
public:
    PluginSelectorState& state;

    PluginListSorter(PluginSelectorState& state);

    ~PluginListSorter() = default;

    void setPluginList(juce::Array<juce::PluginDescription> pluginList);

    juce::Array<juce::PluginDescription> getFilteredPluginList() const;

    int compareElements(juce::PluginDescription first, juce::PluginDescription second) const;

private:
    juce::Array<juce::PluginDescription> _fullPluginList;

    bool _isFilterNeeded() const;

    bool _passesFilter(const juce::PluginDescription& plugin) const;
};

class PluginSelectorTableListBoxModel : public juce::TableListBoxModel,
                                        public juce::MessageListener {
public:
    PluginSelectorTableListBoxModel(PluginSelectorListParameters selectorListParameters);
    virtual ~PluginSelectorTableListBoxModel();

    void onFiltersOrSortUpdate();

    virtual int getNumRows() override;

    virtual void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;

    virtual void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;

    void cellDoubleClicked(int rowNumber, int columnId, const juce::MouseEvent& event) override;

    void sortOrderChanged(int newSortColumnId, bool isForwards) override;

    void handleMessage(const juce::Message& message) override;

private:
    PluginScanClient& _scanner;
    PluginListSorter _pluginListSorter;
    juce::Array<juce::PluginDescription> _pluginList;
    juce::AudioPluginFormat::PluginCreationCallback _pluginCreationCallback;
    std::function<double()> _getSampleRateCallback;
    std::function<int()> _getBlockSizeCallback;
    juce::AudioPluginFormatManager _formatManager;
};

class PluginSelectorTableListBox : public juce::TableListBox {
public:
    PluginSelectorTableListBox(PluginSelectorListParameters selectorListParameters);
    virtual ~PluginSelectorTableListBox() = default;

    void onFiltersOrSortUpdate();

private:
    PluginSelectorTableListBoxModel _pluginTableListBoxModel;
};


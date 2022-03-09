#pragma once

#include <JuceHeader.h>

#include "PluginParameterSelectorListParameters.h"
#include "PluginParameterSelectorState.h"

class PluginParameterListSorter {
public:
    PluginParameterSelectorState& state;

    PluginParameterListSorter(PluginParameterSelectorState& newState,
                              const juce::Array<juce::AudioProcessorParameter*>& fullParameterList);
    ~PluginParameterListSorter() = default;

    juce::Array<juce::AudioProcessorParameter*> getFilteredParameterList() const;

    int compareElements(juce::AudioProcessorParameter* first, juce::AudioProcessorParameter* second) const;

private:
    const juce::Array<juce::AudioProcessorParameter*>& _fullParameterList;

    bool _isFilterNeeded() const;
    bool _passesFilter(const juce::AudioProcessorParameter* parameter) const;
};

class PluginParameterSelectorTableListBoxModel : public juce::TableListBoxModel {
public:
    PluginParameterSelectorTableListBoxModel(PluginParameterSelectorListParameters selectorListParameters);
    ~PluginParameterSelectorTableListBoxModel() = default;

    void onFilterUpdate();

    int getNumRows() override;

    void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;

    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;

    void cellDoubleClicked(int rowNumber, int columnId, const juce::MouseEvent& event) override;

private:
    PluginParameterListSorter _parameterListSorter;
    juce::Array<juce::AudioProcessorParameter*> _parameterList;
    std::function<void(juce::AudioProcessorParameter*)> _parameterSelectedCallback;
};

class PluginParameterSelectorTableListBox : public juce::TableListBox {
public:
    PluginParameterSelectorTableListBox(PluginParameterSelectorListParameters selectorListParameters);
    ~PluginParameterSelectorTableListBox() = default;

    void onFilterUpdate();

private:
    PluginParameterSelectorTableListBoxModel _parameterTableListBoxModel;
};

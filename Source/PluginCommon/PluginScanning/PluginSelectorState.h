/*
  ==============================================================================

    PluginSelectorState.h
    Created: 25 May 2021 10:20:37pm
    Author:  Jack Devlin

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

struct PluginSelectorState {
    int sortColumnId;
    bool sortForwards;
    bool includeVST;
    bool includeVST3;
    bool includeAU;
    juce::String filterString;

    PluginSelectorState() : sortColumnId(0),
                            sortForwards(true),
                            includeVST(true),
                            includeVST3(true),
                            includeAU(true),
                            filterString("") { }
};

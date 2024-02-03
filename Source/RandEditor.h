#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "RandPanels.h"
#include <random>


//==============================================================================
class RandEditor : public juce::Component
{
public:
	RandEditor(APAudioProcessor& proc_);
	void resized() override;
	void randomize();
	void clearAll();
    void randomizeOSCs();
    void randomizeENVs();
    void randomizeLFOs();
    void randLFOtoOSC();
    void randENVtoOSC();
    void randLFOtoTimbre();
    void randENVtoTimbre();
    void resetFine();
    
private:
	APAudioProcessor& proc;
	RandMatrixBox matrix{ "matrix", proc };
	juce::TextButton randomizeButton{ "Randomize" }, clearAllButton{ "Clear All" },
    randOSCsButton{"Randomize OSCs"}, randENVsButton{"Randomize ENVs"},
    randLFOsButton{"Randomize LFOs"}, randLfoToOscButton{"LFO -> OSC"}, randEnvToOscButton{"ENV -> OSC"},
    randLfoToTimbreButton{"LFO -> Timbre"}, randEnvToTimbreButton{"ENV -> Timbre"},
    randFXButton{"Randomize FX"}, reharmonizeButton{"Reset inharmonic"};
	std::random_device rd;
    juce::Slider randNumber, randAmount;
    juce::Label test{"test", "test"};
    juce::ToggleButton inharmonic{"inharmonic"};
};

//  Buttons for
//      — randomize oscs
//      — randomize lfos
//      — randomize envs
//
//  Randomize LFO->OSC mods
//  Randomize ENV->OSC mods
//
//  Rand LFO->Timbre (no pitch)
//  Rand ENV->Timbre (no pitch)
//
//Rand -> FX params (not selectors)

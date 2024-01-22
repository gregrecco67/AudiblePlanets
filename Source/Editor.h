#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Panels.h"
#include "BinaryData.h"
#include "OrbitViz.h"

//==============================================================================
class Editor : public juce::Component, public juce::Timer
{
public:
    Editor (APAudioProcessor& proc_);

    void setupCallbacks();
    void resized() override;
    void timerCallback() override;

private:
    APAudioProcessor& proc;

	OscillatorBox osc1{ "osc1", proc, proc.osc1Params };
	OscillatorBox osc2{ "osc2", proc, proc.osc2Params };
	OscillatorBox osc3{ "osc3", proc, proc.osc3Params };
	OscillatorBox osc4{ "osc4", proc, proc.osc4Params };

	ENVBox env1{ "env1", proc, proc.env1Params };
	ENVBox env2{ "env2", proc, proc.env2Params };
	ENVBox env3{ "env3", proc, proc.env3Params };
	ENVBox env4{ "env4", proc, proc.env4Params };

    FilterBox filter                        { "flt", proc };

	LFOBox lfo1{ "lfo1", proc, proc.lfo1Params, proc.modSrcLFO1, proc.modSrcMonoLFO1, proc.lfo1 };
	LFOBox lfo2{ "lfo2", proc, proc.lfo2Params, proc.modSrcLFO2, proc.modSrcMonoLFO2, proc.lfo2 };
	LFOBox lfo3{ "lfo3", proc, proc.lfo3Params, proc.modSrcLFO3, proc.modSrcMonoLFO3, proc.lfo3 };
	LFOBox lfo4{ "lfo4", proc, proc.lfo4Params, proc.modSrcLFO4, proc.modSrcMonoLFO4, proc.lfo4 };
    ModBox mod                              { "mod", proc };
    MatrixBox mtx                           { "mtx", proc };
    GlobalBox global                        { "global", proc };
	TimbreBox timbre						{ "timbre", proc };

    ComboBox algoSelector;
    OrbitViz orbitViz;
    int frameRate{ 30 };
    float vizDefPhase{ 0.f }, vizEpi1Phase{ 0.f }, vizEpi2Phase{ 0.f }, vizEpi3Phase{ 0.f };
    float phaseIncrement{ juce::MathConstants<float>::pi / (2.0f * (float)frameRate) };

    //DelayBox delay { proc };
    
    gin::Layout layout { *this };
};

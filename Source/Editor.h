#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Panels.h"

//==============================================================================
class Editor : public juce::Component
{
public:
    Editor (APAudioProcessor& proc_);

    void setupCallbacks();
    void resized() override;

private:
    APAudioProcessor& proc;

	OscillatorBox osc1{ "osc1", proc, proc.osc1Params };
	OscillatorBox osc2{ "osc2", proc, proc.osc2Params };
	OscillatorBox osc3{ "osc3", proc, proc.osc3Params };
	OscillatorBox osc4{ "osc4", proc, proc.osc4Params };

	ADSRBox adsr1{ "env1", proc, proc.env1Params };
	ADSRBox adsr2{ "env2", proc, proc.env2Params };
	ADSRBox adsr3{ "env3", proc, proc.env3Params };
	ADSRBox adsr4{ "env4", proc, proc.env4Params };
    FilterBox filter                        { "filter", proc };

	//LFOBox lfo1{ "LFO1", proc, proc.lfo1Params, proc.modSrcLFO1, proc.modSrcMonoLFO1, proc.lfo1 };
	//LFOBox lfo2{ "LFO2", proc, proc.lfo2Params, proc.modSrcLFO2, proc.modSrcMonoLFO2, proc.lfo2 };
	//LFOBox lfo3{ "LFO3", proc, proc.lfo3Params, proc.modSrcLFO3, proc.modSrcMonoLFO3, proc.lfo3 };
	//LFOBox lfo4{ "LFO4", proc, proc.lfo4Params, proc.modSrcLFO4, proc.modSrcMonoLFO4, proc.lfo4 };
    ModBox mod                              { "mod", proc };
    MatrixBox mtx                           { "mtx", proc };
    GlobalBox global                        { "global", proc };

    //DelayBox delay { proc };

    gin::Layout layout { *this };
};

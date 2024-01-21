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

	ADSRBox env1{ "env1", proc, proc.env1Params };
    FilterBox filter                        { "filter", proc };

	LFOBox lfo1{ "LFO1", proc, proc.lfo1Params, proc.modSrcLFO1, proc.modSrcMonoLFO1, proc.lfo1 };
	//LFOBox lfo2{ "LFO2", proc, proc.lfo2Params, proc.modSrcLFO2, proc.modSrcMonoLFO2, proc.lfo2 };
	//LFOBox lfo3{ "LFO3", proc, proc.lfo3Params, proc.modSrcLFO3, proc.modSrcMonoLFO3, proc.lfo3 };
	//LFOBox lfo4{ "LFO4", proc, proc.lfo4Params, proc.modSrcLFO4, proc.modSrcMonoLFO4, proc.lfo4 };
    ModBox mod                              { "mod", proc };
    MatrixBox mtx                           { "mtx", proc };
    GlobalBox global                        { "global", proc };

    //DelayBox delay { proc };

    gin::Layout layout { *this };
};

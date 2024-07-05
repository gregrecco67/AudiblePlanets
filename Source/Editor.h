/*
 * Audible Planets - an expressive, quasi-Ptolemaic semi-modular synthesizer
 *
 * Copyright 2024, Greg Recco
 *
 * Audible Planets is released under the GNU General Public Licence v3
 * or later (GPL-3.0-or-later). The license is found in the "LICENSE"
 * file in the root of this repository, or at
 * https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * All source for Audible Planets is available at
 * https://github.com/gregrecco67/AudiblePlanets
 */

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Panels.h"
#include "BinaryData.h"
#include "OrbitViz.h"

//==============================================================================
class Editor : public juce::Component, public juce::Timer, public gin::Parameter::ParameterListener
{
public:
    Editor(APAudioProcessor& proc_);
	~Editor() override;

    void resized() override;
    void timerCallback() override;
	void valueUpdated(gin::Parameter* param) override;

private:
    APAudioProcessor& proc;

	OscillatorBox osc{ proc };
	
	ENVBox env{ proc };

    FilterBox filter                        { "flt", proc };

	LFOBox lfo1{ "lfo1", proc, proc.lfo1Params, proc.modSrcLFO1, proc.modSrcMonoLFO1};
	LFOBox lfo2{ "lfo2", proc, proc.lfo2Params, proc.modSrcLFO2, proc.modSrcMonoLFO2};
	LFOBox lfo3{ "lfo3", proc, proc.lfo3Params, proc.modSrcLFO3, proc.modSrcMonoLFO3};
	LFOBox lfo4{ "lfo4", proc, proc.lfo4Params, proc.modSrcLFO4, proc.modSrcMonoLFO4};
    ModBox mod                              { "mod", proc };
    GlobalBox global                        { "global", proc };
	TimbreBox timbre						{ "timbre", proc };
	OrbitBox orbit							{ "orbit", proc };

    OrbitViz orbitViz;
    int frameRate{ 30 };
    float vizDefPhase{ 0.f }, vizEpi1Phase{ 0.f }, vizEpi2Phase{ 0.f }, vizEpi3Phase{ 0.f };
    float phaseIncrement{ juce::MathConstants<float>::pi / (2.0f * (float)frameRate) };
    gin::Layout layout { *this };
	juce::ToggleButton liveViz{ "Live" };
	APLNF aplnf;
};

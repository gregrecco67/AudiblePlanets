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

#include <gin_plugin/gin_plugin.h>
#include "BinaryData.h"
#include "DSP/PluginProcessor.h"
#include "OrbitViz.h"
#include "Panels.h"

//==============================================================================
class Editor : public juce::Component,
               public gin::Parameter::ParameterListener,
               juce::Timer {
public:
	explicit Editor(APAudioProcessor &proc_);
	~Editor() override;

	void resized() override;
	void valueUpdated(gin::Parameter*) override {}
	static void setGrid(
	    gin::ParamBox *box, float x, float y, float heds, float w, float h);
	void timerCallback() override;
	void paint(juce::Graphics &g) override;

private:
	APAudioProcessor &proc;

	OSCBox osc1{proc, proc.osc1Params, 0};
	OSCBox osc2{proc, proc.osc2Params, 1};
	OSCBox osc3{proc, proc.osc3Params, 2};
	OSCBox osc4{proc, proc.osc4Params, 3};

	ENVBox env1{proc, proc.env1Params, 0};
	ENVBox env2{proc, proc.env2Params, 1};
	ENVBox env3{proc, proc.env3Params, 2};
	ENVBox env4{proc, proc.env4Params, 3};

	FilterBox filter{"  flt", proc};
	ModBox modsrc{"  mod", proc};
	TimbreBox timbre{"  timbre", proc};
	AuxBox aux{"      Aux Oscillator", proc};
	MainMatrixBox matrix{"  Mod Matrix", proc};
	OrbitViz orbitViz;
	VolumeBox volumeBox{proc};
	LevelBox levelBox{proc.levelTracker};

	GlobalBox global{"  global", proc};
	MacrosBox macros{proc};

	int frameRate{24};
	float vizDefPhase{0.f}, vizEpi1Phase{0.f}, vizEpi2Phase{0.f}, vizEpi3Phase{0.f};
	juce::ToggleButton liveViz{"Live"};
	juce::TextButton swap{"Swap"};
	APModMatrixBox::Row::APDepthSlider speedSlider;
	float phaseIncrement{juce::MathConstants<float>::pi / (2.0f * frameRate)};
	APLNF aplnf;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Editor)
};

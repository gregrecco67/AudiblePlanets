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
#include "DSP/PluginProcessor.h"
#include "ModPanels.h"
#include "Panels.h"

//==============================================================================
class ModEditor : public juce::Component,
                  public gin::Parameter::ParameterListener  // , juce::Timer
{
public:
	explicit ModEditor(APAudioProcessor &proc_);
	~ModEditor() override;

	void resized() override;
	void valueUpdated(gin::Parameter*) override {}
	void setGrid(
	    gin::ParamBox *box, float x, float y, float heds, float w, float h);
	// void timerCallback() override;

private:
	APAudioProcessor &proc;

	LFOBox lfo1{proc, proc.lfo1Params, 1};
	LFOBox lfo2{proc, proc.lfo2Params, 2};
	LFOBox lfo3{proc, proc.lfo3Params, 3};
	LFOBox lfo4{proc, proc.lfo4Params, 4};

	MainMatrixBox matrix{"  Mod Matrix", proc};
	ModBox modsrc{"  mod", proc};
	MsegBox msegA{proc, proc.mseg1Params, proc.mseg2Params, proc.mseg1Data,
	    proc.mseg2Data, 1};
	MsegBox msegB{proc, proc.mseg3Params, proc.mseg4Params, proc.mseg3Data,
	    proc.mseg4Data, 3};
	FilterBox filter{"  flt", proc};
	TimbreBox timbre{"  timbre", proc};
	GlobalBox global{"  global", proc};

	MacrosBox macros{proc};
	APLNF aplnf;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModEditor)
};

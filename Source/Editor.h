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
#include "PluginProcessor.h"
#include "Panels.h"
#include "BinaryData.h"


//==============================================================================
class Editor : public juce::Component, public gin::Parameter::ParameterListener
{
public:
    Editor(APAudioProcessor& proc_);
	~Editor() override;

    void resized() override;
	void valueUpdated(gin::Parameter* param) override;
    void setGrid(gin::ParamBox* box, float x, float y, float heds, float w, float h);

private:
    APAudioProcessor& proc;

	OscillatorBox osc{ proc };
	ENVBox env{ proc };
    FilterBox filter                        { "  flt", proc };
    LFOBox lfo{ proc };
    ModBox modsrc                              { "  mod", proc };
    GlobalBox global                        { "  global", proc };
	TimbreBox timbre						{ "  timbre", proc };
    AuxBox aux{ "      Aux Oscillator", proc };
    MainMatrixBox matrix{ "  Mod Matrix", proc };
    MsegBox mseg{ proc };
	MacrosBox macros{ proc };
	VolumeBox volume{ proc };
	
	APLNF aplnf;
};

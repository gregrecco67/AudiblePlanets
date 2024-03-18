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
#include "MsegPanels.h"

 //==============================================================================
class MsegEditor : public juce::Component, public gin::Parameter::ParameterListener
{
public:
	MsegEditor(APAudioProcessor& proc_);

	void resized() override;
	void valueUpdated(gin::Parameter* param) override;

private:
	APAudioProcessor& proc;
	gin::Layout layout{ *this };

	MsegMatrixBox msegMatrixBox{"Mod Matrix", proc};
	MsegBox mseg1Box{"MSEG1", proc, proc.mseg1Params, proc.mseg1Data};
	MsegBox mseg2Box{"MSEG2", proc, proc.mseg2Params, proc.mseg2Data};
	MsegBox mseg3Box{"MSEG3", proc, proc.mseg3Params, proc.mseg3Data};
	MsegBox mseg4Box{"MSEG4", proc, proc.mseg4Params, proc.mseg4Data};

};

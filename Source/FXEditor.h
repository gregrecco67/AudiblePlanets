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
#include "BinaryData.h"
#include "FXPanels.h"

//==============================================================================
class FXEditor : public juce::Component, public gin::Parameter::ParameterListener
{
public:
	FXEditor(APAudioProcessor& proc_);
	void resized() override;
	void valueUpdated(gin::Parameter* param) override;
	void removeDuplicates(int effect, int componentToSkip);

private:
	APAudioProcessor& proc;
	FXBox fxa1Box{ "FX A1", proc},
		fxa2Box{ "FX A2", proc},
		fxa3Box{ "FX A3", proc},
		fxa4Box{ "FX A4", proc},
		fxb1Box{ "FX B1", proc},
		fxb2Box{ "FX B2", proc},
		fxb3Box{ "FX B3", proc},
		fxb4Box{ "FX B4", proc};
	gin::Select chainSetting{ proc.fxOrderParams.chainAtoB };
	gin::Select fxa1Selector{proc.fxOrderParams.fxa1},
		fxa2Selector{ proc.fxOrderParams.fxa2 },
		fxa3Selector{ proc.fxOrderParams.fxa3 },
		fxa4Selector{ proc.fxOrderParams.fxa4 },
		fxb1Selector{ proc.fxOrderParams.fxb1 },
		fxb2Selector{ proc.fxOrderParams.fxb2 },
		fxb3Selector{ proc.fxOrderParams.fxb3 },
		fxb4Selector{ proc.fxOrderParams.fxb4 };
    FXModBox mod{"mod", proc};
	MatrixBox matrix{"matrix", proc};
	gin::Knob laneAGain{ proc.fxOrderParams.laneAGain }, laneBGain{ proc.fxOrderParams.laneBGain }, outLevel{ proc.globalParams.level };
};

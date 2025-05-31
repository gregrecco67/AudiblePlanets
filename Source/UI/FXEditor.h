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
#include "APModAdditions.h"
#include "BinaryData.h"
#include "DSP/PluginProcessor.h"
#include "FXPanels.h"
#include "Panels.h"

//==============================================================================
class FXEditor : public juce::Component,
                 public gin::Parameter::ParameterListener {
public:
	explicit FXEditor(APAudioProcessor &proc_);
	~FXEditor() override;
	void resized() override;
	void valueUpdated(gin::Parameter *param) override;
	void removeDuplicates(int effect, int componentToSkip);
	void setAllControls();

	void setGrid(
	    gin::ParamBox *box, float x, float y, float heds, float w, float h);

private:
	APAudioProcessor &proc;
	FXBox fxa1Box{"FX A1", proc}, fxa2Box{"FX A2", proc},
	    fxa3Box{"FX A3", proc}, fxa4Box{"FX A4", proc}, fxb1Box{"FX B1", proc},
	    fxb2Box{"FX B2", proc}, fxb3Box{"FX B3", proc}, fxb4Box{"FX B4", proc};
	gin::Select chainSetting{proc.fxOrderParams.chainAtoB};
	gin::Select fxa1Selector{proc.fxOrderParams.fxa1},
	    fxa2Selector{proc.fxOrderParams.fxa2},
	    fxa3Selector{proc.fxOrderParams.fxa3},
	    fxa4Selector{proc.fxOrderParams.fxa4},
	    fxb1Selector{proc.fxOrderParams.fxb1},
	    fxb2Selector{proc.fxOrderParams.fxb2},
	    fxb3Selector{proc.fxOrderParams.fxb3},
	    fxb4Selector{proc.fxOrderParams.fxb4};

	MainMatrixBox matrix{"  Mod Matrix", proc};
	ModBox mod{"  mod", proc};

	APKnob laneAGain{proc.fxOrderParams.laneAGain},
	    laneAFreq{proc.fxOrderParams.laneAFreq},
	    laneARes{proc.fxOrderParams.laneARes},
	    laneAPan{proc.fxOrderParams.laneAPan},
	    laneBGain{proc.fxOrderParams.laneBGain},
	    laneBFreq{proc.fxOrderParams.laneBFreq},
	    laneBRes{proc.fxOrderParams.laneBRes},
	    laneBPan{proc.fxOrderParams.laneBPan};
	gin::Select laneAType{proc.fxOrderParams.laneAType},
	    laneBType{proc.fxOrderParams.laneBType},
	    laneAPrePost{proc.fxOrderParams.laneAPrePost},
	    laneBPrePost{proc.fxOrderParams.laneBPrePost};
	juce::Label laneALabel{"Lane A", "Lane A"}, laneBLabel{"Lane B", "Lane B"};

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FXEditor)
};

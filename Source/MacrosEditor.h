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
#include "MacrosPanels.h"

 //==============================================================================
class MacrosEditor : public juce::Component
{
public:
	MacrosEditor(APAudioProcessor& proc_);

	void resized() override;
	//void timerCallback() override;



private:
	APAudioProcessor& proc;
	
	MacrosMatrixBox macro1{ "matrix", proc, proc.macroSrc1, proc.macroParams.macro1 };
	MacrosMatrixBox macro2{ "matrix", proc, proc.macroSrc2, proc.macroParams.macro2 };
	MacrosMatrixBox macro3{ "matrix", proc, proc.macroSrc3, proc.macroParams.macro3 };
	MacrosMatrixBox macro4{ "matrix", proc, proc.macroSrc4, proc.macroParams.macro4 };

	gin::Layout layout{ *this };

	
};

// TODO: change those mod depth sliders
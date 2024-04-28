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
class MacrosEditor : public juce::Component, public juce::FileDragAndDropTarget, public gin::Parameter::ParameterListener
{
public:
	MacrosEditor(APAudioProcessor& proc_);
	~MacrosEditor() override;
	void resized() override;
	bool isInterestedInFileDrag(const juce::StringArray& files) override;
	void filesDropped(const juce::StringArray& files, int x, int y) override;
    void valueUpdated(gin::Parameter* param) override;

private:
	APAudioProcessor& proc;
	
	MacrosMatrixBox macro1{ "Macro 1", proc, proc.macroSrc1, proc.macroParams.macro1, proc.macroParams.macro1cc, 1 };
	MacrosMatrixBox macro2{ "Macro 2", proc, proc.macroSrc2, proc.macroParams.macro2, proc.macroParams.macro2cc, 2 };
	MacrosMatrixBox macro3{ "Macro 3", proc, proc.macroSrc3, proc.macroParams.macro3, proc.macroParams.macro3cc, 3 };
	MacrosMatrixBox macro4{ "Macro 4", proc, proc.macroSrc4, proc.macroParams.macro4, proc.macroParams.macro4cc, 4 };
	AuxBox aux{ "Aux Oscillator", proc };
	MacrosModBox macrosModBox{ "Mod Sources", proc };
	SamplerBox samplerBox{ "Sampler", proc };
	APLNF lookAndFeel;
    TextEditor textEditor;
};


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
#include "APModAdditions.h"

class MacrosMatrixBox : public gin::ParamBox
{
public:
	MacrosMatrixBox(const juce::String& name, APAudioProcessor& proc_)
		: gin::ParamBox(name), proc(proc_)
	{
		setName("mtx");
		addControl(new APModMatrixBox(proc, proc.modMatrix), 0, 0, 8, 5);
	}

	APAudioProcessor& proc;
};

// TODO: create MacroModMatrixBox class.
// constructor will take the ModSrcId of the macro it represents.
// refresh() method will pull assignments from getModDepths(macro).
// Row should show a linear slider (class MacroParamKnob) for the affected parameter,
// a mod depth slider (like APModDepthSlider), and an overlay for the
// degree of modulation. resized() will have to arrange all these elements:
// enable button, mod depth slider, bipolar toggle, curve menu, mod src name,
// mod dst name, param slider, delete button.
// MacroModMatrixBox should also have a Create button in the header, which should
// use the ParameterSelector popup to assign the macro to a parameter.

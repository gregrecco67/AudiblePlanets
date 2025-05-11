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

#include "ModEditor.h"
#include "APModAdditions.h"

ModEditor::ModEditor(APAudioProcessor& proc_)
    : proc(proc_)
{
	addAndMakeVisible(modsrc);
    addAndMakeVisible(matrix);
    addAndMakeVisible(mseg);
	addAndMakeVisible(macros);
    addAndMakeVisible(lfo1);
    addAndMakeVisible(lfo2);
    addAndMakeVisible(lfo3);
    addAndMakeVisible(lfo4);
    
	//startTimerHz(x);
}

ModEditor::~ModEditor()
{
	proc.globalParams.pitchbendRange->removeListener(this);
	proc.globalParams.mpe->removeListener(this);
	setLookAndFeel(nullptr);
}


void ModEditor::valueUpdated(gin::Parameter* param)
{
	if (param == proc.globalParams.pitchbendRange || param == proc.globalParams.mpe) {
		proc.updatePitchbend();
		return;
	}
}

void ModEditor::setGrid(gin::ParamBox* box, float x, float y, float heds, float w, float h) {
	box->setBounds((int)(x * 56.f), (int)(y * 70.f + 23.f * heds), (int)(w * 56.f), (int)(h * 70.f + 23.f));
}

void ModEditor::resized()
{
    auto area = getLocalBounds();

	//if (area.getWidth() > 1186 || area.getHeight() > 725) {
	//	return;
	//}

	setGrid(&lfo1,      0,  0, 0, 5, 2);
	setGrid(&lfo2,      0,  2, 1, 5, 2);
	setGrid(&lfo3,      0,  4, 2, 5, 2);
	setGrid(&lfo4,      0,  6, 3, 5, 2);


    setGrid(&modsrc,  16,  0, 0, 5, 4.328571f);
    setGrid(&matrix,  16,  4.328571f, 1, 5, 4.328571f);
}


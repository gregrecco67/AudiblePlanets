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

#include "Editor.h"
#include "BinaryData.h"
#include "APModAdditions.h"

Editor::Editor(APAudioProcessor& proc_)
    : proc(proc_)
{
    addAndMakeVisible(osc);
	addAndMakeVisible(env);
    addAndMakeVisible(lfo);
	addAndMakeVisible(filter);
	addAndMakeVisible(modsrc);
	addAndMakeVisible(global);
	addAndMakeVisible(timbre);
    addAndMakeVisible(matrix);
    // addAndMakeVisible(orbitViz);
    // addAndMakeVisible(orbit);
	proc.globalParams.pitchbendRange->addListener(this);
	proc.globalParams.mpe->addListener(this);
    // startTimerHz(frameRate);
	// addAndMakeVisible(liveViz);
	// liveViz.setLookAndFeel(&aplnf);
    addAndMakeVisible(aux);
    addAndMakeVisible(matrix);
    addAndMakeVisible(mseg);
	addAndMakeVisible(macros);
	//addAndMakeVisible(volume);

	osc.setRight(true);
	env.setRight(true);
	filter.setRight(true);
	timbre.setRight(true);
	lfo.setRight(true);
	macros.setRight(true);
	aux.setRight(true);
	global.setRight(true);
}

Editor::~Editor()
{
	proc.globalParams.pitchbendRange->removeListener(this);
	proc.globalParams.mpe->removeListener(this);
	setLookAndFeel(nullptr);
}


void Editor::valueUpdated(gin::Parameter* param) // we'll use this to set any other box with the same effect selected to "None"
{
	if (param == proc.globalParams.pitchbendRange || param == proc.globalParams.mpe) {
		proc.updatePitchbend();
		return;
	}
}

void Editor::setGrid(gin::ParamBox* box, float x, float y, float heds, float w, float h) {
	box->setBounds(x * 56.f, y * 70.f + 23.f * heds, w * 56.f, h * 70.f + 23.f);
}

void Editor::resized()
{
    auto area = getLocalBounds();

	if (area.getWidth() > 1186 || area.getHeight() > 725) {
		return;
	}

    setGrid(&osc,      0,  0, 0, 4, 2);
    setGrid(&env,      4,  0, 0, 6, 2);
    setGrid(&filter,  10,  0, 0, 2, 2);
	setGrid(&timbre,  12,  0, 0, 4, 2);
	setGrid(&lfo,      0,  2, 1, 5, 2);
	setGrid(&aux,      5,  2, 1, 5, 2);
	setGrid(&macros,  10,  2, 1, 3, 2);
	setGrid(&global,   13, 2, 1, 3, 2);
    setGrid(&mseg,     0,  4, 2, 8, 5);

    setGrid(&modsrc,  16,  0, 0, 5, 4.328571f);
    setGrid(&matrix,  16,  4.328571, 1, 5, 4.328571f);
	

}

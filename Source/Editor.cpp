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
	addAndMakeVisible(volume);

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

void Editor::setGrid(gin::ParamBox* box, int x, int y, int boxheaders, int w, int h) {
	box->setBounds(x * 56, y * 70 + 23 * boxheaders, w * 56, h * 70 + 23);
}

void Editor::resized()
{
    auto area = getLocalBounds();

	if (area.getWidth() > 1186 || area.getHeight() > 725) {
		return;
	}

    setGrid(&osc,      0,  0, 0, 5, 2);
    setGrid(&env,      5,  0, 0, 6, 2);
	setGrid(&timbre,   0,  2, 1, 4, 2);
    setGrid(&filter,   4,  2, 1, 2, 2);
	setGrid(&lfo,      6,  2, 1, 5, 2);
	setGrid(&global,   0,  4, 2, 3, 2);
    setGrid(&mseg,     3,  4, 2, 8, 5);
	setGrid(&macros,   0,  6, 3, 3, 2);
	setGrid(&aux,     11,  0, 0, 5, 2);
	setGrid(&volume,  11,  7, 0, 5, 2);
    setGrid(&modsrc,  16,  0, 0, 5, 4);
    setGrid(&matrix,  16,  4, 1, 5, 4);
	//    osc.setBounds(0,0,280,163);
	//    env.setBounds(282,0,336,163);
	//	timbre.setBounds(0,163,222,162);
	//    filter.setBounds(224,163,112,163);
	//	lfo.setBounds(338,163,280,163);
	//	global.setBounds(0,326,168,163);
	//    mseg.setBounds(170, 326, 56*8, 323);
	//	macros.setBounds(0, 491, 168, 163);
	//	aux.setBounds   (620,   0, 280, 163);
	//	volume.setBounds(620, 491, 280, 163);
	//    modsrc.setBounds(902,   0, 280, 303);
	//    matrix.setBounds(902, 304, 280, 303);

}

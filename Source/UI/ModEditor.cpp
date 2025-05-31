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

#include "APModAdditions.h"
#include "ModEditor.h"

ModEditor::ModEditor(APAudioProcessor &proc_) : proc(proc_)
{
	addAndMakeVisible(modsrc);
	addAndMakeVisible(matrix);
	addAndMakeVisible(msegA);
	addAndMakeVisible(msegB);
	addAndMakeVisible(macros);
	addAndMakeVisible(lfo1);
	addAndMakeVisible(lfo2);
	addAndMakeVisible(lfo3);
	addAndMakeVisible(lfo4);
	addAndMakeVisible(filter);
	addAndMakeVisible(timbre);
	addAndMakeVisible(global);

	lfo1.setRight(true);
	lfo2.setRight(true);
	lfo3.setRight(true);
	lfo4.setRight(true);
	msegA.setRight(true);
	msegB.setRight(true);
	macros.setRight(true);
	filter.setRight(true);
	timbre.setRight(true);
	global.setRight(true);
	proc.globalParams.pitchbendRange->addListener(this);

	modsrc.setHeaderRight(false);
	matrix.setHeaderRight(false);
}

ModEditor::~ModEditor()
{
	proc.globalParams.pitchbendRange->removeListener(this);
	proc.globalParams.mpe->removeListener(this);
}

void ModEditor::setGrid(
    gin::ParamBox *box, float x, float y, float heds, float w, float h)
{
	box->setBounds(static_cast<int>(x * 56.f), static_cast<int>(y * 70.f + 23.f * heds),
	    static_cast<int>(w * 56.f), static_cast<int>(h * 70.f + 23.f));
}

void ModEditor::resized()
{
	setGrid(&lfo1, 0, 0, 0, 5, 2);
	setGrid(&lfo2, 0, 2, 1, 5, 2);
	setGrid(&lfo3, 0, 4, 2, 5, 2);
	setGrid(&lfo4, 0, 6, 3, 5, 2);
	setGrid(&msegA, 5, 0, 0, 8, 4.328571f);
	setGrid(&msegB, 5, 4.328571f, 1, 8, 4.328571f);
	msegA.setBounds(5 * 56, 0, 8 * 56, 326);
	msegB.setBounds(5 * 56, 326, 8 * 56, 326);

	setGrid(&timbre, 13, 0, 0, 3, 2);
	setGrid(&filter, 13, 2, 1, 3, 2);
	setGrid(&global, 13, 4, 2, 3, 2);
	setGrid(&macros, 13, 6, 3, 3, 2);
	modsrc.setBounds(16 * 56, 0, 5 * 56, 326);
	//matrix.setBounds(16 * 56, 326, 5 * 56, 4.328571f * 70 + 23.f);
    matrix.setBounds(16 * 56, 326, 5 * 56, 326);
}

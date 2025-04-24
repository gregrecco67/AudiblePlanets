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
    addAndMakeVisible(orbitViz);
	addAndMakeVisible(liveViz); // toggle
	liveViz.setLookAndFeel(&aplnf);
	proc.globalParams.pitchbendRange->addListener(this);
	proc.globalParams.mpe->addListener(this);
    startTimerHz(frameRate);
	// addAndMakeVisible(liveViz);
    addAndMakeVisible(aux);
    addAndMakeVisible(matrix);
    addAndMakeVisible(mseg);
	addAndMakeVisible(macros);

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


void Editor::valueUpdated(gin::Parameter* param)
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
	orbitViz.setBounds( 8 * 56 + 5, 4 * 70 + 2 * 23, 
		8 * 56, 5 * 70);
	liveViz.setBounds( 8 * 56.f, 4 * 70.f + 2 * 23.f,
		75, 25);

    setGrid(&modsrc,  16,  0, 0, 5, 4.328571f);
    setGrid(&matrix,  16,  4.328571f, 1, 5, 4.328571f);
}

void Editor::timerCallback() {
	auto speed = 0.05f; // proc.orbitParams.speed->getUserValue();
	bool live = liveViz.getToggleState();
    //auto c1S = (APKnob*)osc.c1;
	
	auto defRatio = live ? proc.viz.defRat : proc.viz2.defRat;
    auto defPhaseIncrement = defRatio * phaseIncrement;
    vizDefPhase += defPhaseIncrement * speed;
    if (vizDefPhase > juce::MathConstants<float>::twoPi)
        vizDefPhase -= juce::MathConstants<float>::twoPi;
    auto epi1Ratio = live ? proc.viz.epi1Rat : ((APKnob*)osc.c2)->getSlider().getValue();
    auto vizEpi1PhaseIncrement = epi1Ratio * phaseIncrement;
    vizEpi1Phase += vizEpi1PhaseIncrement * speed;
    if (vizEpi1Phase > juce::MathConstants<float>::twoPi)
        vizEpi1Phase -= juce::MathConstants<float>::twoPi;
    auto epi2Ratio = live ? proc.viz.epi2Rat : (((APKnob*)osc.c3)->getSlider().getValue());
    auto vizEpi2PhaseIncrement = epi2Ratio * phaseIncrement;
    vizEpi2Phase += vizEpi2PhaseIncrement * speed;
    if (vizEpi2Phase > juce::MathConstants<float>::twoPi)
        vizEpi2Phase -= juce::MathConstants<float>::twoPi;
    auto epi3Ratio = live? proc.viz.epi3Rat : (((APKnob*)osc.c4)->getSlider().getValue());
    auto vizEpi3PhaseIncrement = epi3Ratio * phaseIncrement;
    vizEpi3Phase += vizEpi3PhaseIncrement * speed;
    if (vizEpi3Phase > juce::MathConstants<float>::twoPi)
        vizEpi3Phase -= juce::MathConstants<float>::twoPi;
    orbitViz.setEquant(live ? proc.viz.equant : timbre.equant->getSlider().getValue());
	
	orbitViz.setDefRad(live ? proc.viz.defRad : proc.viz2.defRad);
    orbitViz.setEpi1Rad(live ? proc.viz.epi1Rad : ((APKnob*)osc.v2)->getSlider().getValue());
    orbitViz.setEpi2Rad(live ? proc.viz.epi2Rad : ((APKnob*)osc.v3)->getSlider().getValue());
    orbitViz.setEpi3Rad(live ? proc.viz.epi3Rad : ((APKnob*)osc.v4)->getSlider().getValue());
    orbitViz.setAlgo(live ? proc.viz.algo : timbre.algo->getSlider().getValue());
    orbitViz.setDefPhase(vizDefPhase);
    orbitViz.setEpi1Phase(vizEpi1Phase);
    orbitViz.setEpi2Phase(vizEpi2Phase);
    orbitViz.setEpi3Phase(vizEpi3Phase);
	orbitViz.setScale(1.0f);
	orbitViz.setSquash(live ? proc.viz.squash : timbre.squash->getSlider().getValue()); // TODO: create selector for live values visualization
    orbitViz.repaint();
}
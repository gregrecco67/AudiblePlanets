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
    addAndMakeVisible(osc1);
	addAndMakeVisible(osc2);
	addAndMakeVisible(osc3);
	addAndMakeVisible(osc4);

	addAndMakeVisible(env1);
	addAndMakeVisible(env2);
	addAndMakeVisible(env3);
	addAndMakeVisible(env4);
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
    addAndMakeVisible(aux);
    addAndMakeVisible(matrix);
	addAndMakeVisible(volumeBox);
    addAndMakeVisible(speedSlider);
    speedSlider.setRange(0.0, 2.0);
    speedSlider.setSkewFactor(0.5);
    speedSlider.setValue(0.1);
	osc1.setRight(true);
	osc2.setRight(true);
	osc3.setRight(true);
	osc4.setRight(true);

	env1.setRight(true);
	env2.setRight(true);
	env3.setRight(true);
	env4.setRight(true);
	
	filter.setRight(true);
	timbre.setRight(true);
	aux.setRight(true);
	global.setRight(true);
	volumeBox.setRight(true);
    
	startTimerHz(frameRate);
}

Editor::~Editor()
{
	proc.globalParams.pitchbendRange->removeListener(this);
	proc.globalParams.mpe->removeListener(this);
	liveViz.setLookAndFeel(nullptr);
}


void Editor::valueUpdated(gin::Parameter* param)
{
	if (param == proc.globalParams.pitchbendRange || param == proc.globalParams.mpe) {
		proc.updatePitchbend();
		return;
	}
}

void Editor::setGrid(gin::ParamBox* box, float x, float y, float heds, float w, float h) {
	box->setBounds((int)(x * 56.f), (int)(y * 70.f + 23.f * heds), (int)(w * 56.f), (int)(h * 70.f + 23.f));
}

void Editor::resized()
{
    auto area = getLocalBounds();

	if (area.getWidth() > 1186 || area.getHeight() > 725) {
		return;
	}

    setGrid(&osc1,      0,  0, 0, 4, 2);
	setGrid(&osc2,		0,  2, 1, 4, 2);
	setGrid(&osc3,		0,  4, 2, 4, 2);
	setGrid(&osc4,		0,  6, 3, 4, 2);
    setGrid(&env1,      4,  0, 0, 6, 2);
	setGrid(&env2,		4,	2, 1, 6, 2);
	setGrid(&env3,		4,	4, 2, 6, 2);
	setGrid(&env4,		4,	6, 3, 6, 2);
    setGrid(&filter,  10,  0, 0, 2, 2);
	setGrid(&timbre,  12,  0, 0, 2, 2);
	setGrid(&aux,     10,  2, 1, 5, 2);
	setGrid(&global,   14, 0, 0, 2, 2);
	volumeBox.setBounds(15*56, 163, 56, 163);
	orbitViz.setBounds( 10 * 56 + 5, 4 * 70 + 2 * 23, 
		6 * 56, 5 * 70);
	liveViz.setBounds( static_cast<int>(10 * 56.f),
                      static_cast<int>(4 * 70.f + 2 * 23.f),
		75, 25);
    speedSlider.setBounds(14 * 56 , 4 * 70 + 2 * 23, 112, 25);

    setGrid(&modsrc,  16,  0, 0, 5, 4.328571f);
    //setGrid(&matrix,  16,  4.328571f, 1, 5, 4.328571f);
	matrix.setBounds(16 * 56, 326, 5 * 56, 326);
}

void Editor::timerCallback() {
    auto speed = static_cast<float>(speedSlider.getValue());
    //auto c1S = (APKnob*)osc.c1;
    bool live = liveViz.getToggleState();
	
	auto defRatio = live ? proc.viz.defRat : proc.viz2.defRat;
    auto defPhaseIncrement = defRatio * phaseIncrement;
    vizDefPhase += defPhaseIncrement * speed;
    if (vizDefPhase > juce::MathConstants<float>::twoPi)
        vizDefPhase -= juce::MathConstants<float>::twoPi;
    auto epi1Ratio = live ? proc.viz.epi1Rat : proc.viz2.epi1Rat;
    auto vizEpi1PhaseIncrement = epi1Ratio * phaseIncrement;
    vizEpi1Phase += vizEpi1PhaseIncrement * speed;
    if (vizEpi1Phase > juce::MathConstants<float>::twoPi)
        vizEpi1Phase -= juce::MathConstants<float>::twoPi;
    auto epi2Ratio = live ? proc.viz.epi2Rat : proc.viz2.epi2Rat;
    auto vizEpi2PhaseIncrement = epi2Ratio * phaseIncrement;
    vizEpi2Phase += vizEpi2PhaseIncrement * speed;
    if (vizEpi2Phase > juce::MathConstants<float>::twoPi)
        vizEpi2Phase -= juce::MathConstants<float>::twoPi;
    auto epi3Ratio = live ? proc.viz.epi3Rat : proc.viz2.epi3Rat;
    auto vizEpi3PhaseIncrement = epi3Ratio * phaseIncrement;
    vizEpi3Phase += vizEpi3PhaseIncrement * speed;
    if (vizEpi3Phase > juce::MathConstants<float>::twoPi)
        vizEpi3Phase -= juce::MathConstants<float>::twoPi;
    orbitViz.setEquant(live ? proc.viz.equant : proc.viz2.equant);
	
    orbitViz.setDefRad(live ? proc.viz.defRad : proc.viz2.defRad);
    orbitViz.setEpi1Rad(live ? proc.viz.epi1Rad : proc.viz2.epi1Rad);
    orbitViz.setEpi2Rad(live ? proc.viz.epi2Rad : proc.viz2.epi2Rad);
    orbitViz.setEpi3Rad(live ? proc.viz.epi3Rad : proc.viz2.epi3Rad);
    orbitViz.setAlgo(static_cast<int>(live ? proc.viz.algo : proc.viz2.algo));
    orbitViz.setDefPhase(vizDefPhase);
    orbitViz.setEpi1Phase(vizEpi1Phase);
    orbitViz.setEpi2Phase(vizEpi2Phase);
    orbitViz.setEpi3Phase(vizEpi3Phase);
	orbitViz.setScale(1.0f);
    orbitViz.repaint();
}

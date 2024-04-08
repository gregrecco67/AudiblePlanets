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
	addAndMakeVisible(lfo1);
	addAndMakeVisible(lfo2);
	addAndMakeVisible(lfo3);
	addAndMakeVisible(lfo4);
	addAndMakeVisible(env1);
	addAndMakeVisible(env2);
	addAndMakeVisible(env3);
	addAndMakeVisible(env4);
	addAndMakeVisible(filter);
	addAndMakeVisible(mod);
	addAndMakeVisible(global);
	addAndMakeVisible(timbre);
    addAndMakeVisible(orbitViz);
    addAndMakeVisible(orbit);
	proc.globalParams.pitchbendRange->addListener(this);
    startTimerHz(frameRate);
	addAndMakeVisible(liveViz);
	liveViz.setLookAndFeel(&aplnf);
}

Editor::~Editor()
{
	stopTimer();
	proc.globalParams.pitchbendRange->removeListener(this);
	liveViz.setLookAndFeel(nullptr);
}


void Editor::valueUpdated(gin::Parameter* param) // we'll use this to set any other box with the same effect selected to "None"
{
	if (param == proc.globalParams.pitchbendRange) {
		proc.updatePitchbend();
		return;
	}
}

void Editor::timerCallback() {
    auto speed = proc.orbitParams.speed->getUserValue();
	bool live = liveViz.getToggleState();
    auto defRatio = live ? 
		proc.modMatrix.getValue(proc.osc1Params.coarse) + proc.modMatrix.getValue(proc.osc1Params.fine) : 
		proc.osc1Params.coarse->getUserValue() + proc.osc1Params.fine->getUserValue();
    auto defPhaseIncrement = defRatio * phaseIncrement;
    vizDefPhase += defPhaseIncrement * speed;
    if (vizDefPhase > juce::MathConstants<float>::twoPi)
        vizDefPhase -= juce::MathConstants<float>::twoPi;

    auto epi1Ratio = live ?
		proc.modMatrix.getValue(proc.osc2Params.coarse) + proc.modMatrix.getValue(proc.osc2Params.fine) :
		proc.osc2Params.coarse->getUserValue() + proc.osc2Params.fine->getUserValue();
    auto vizEpi1PhaseIncrement = epi1Ratio * phaseIncrement;
    vizEpi1Phase += vizEpi1PhaseIncrement * speed;
    if (vizEpi1Phase > juce::MathConstants<float>::twoPi)
        vizEpi1Phase -= juce::MathConstants<float>::twoPi;

    auto epi2Ratio = live ?
		proc.modMatrix.getValue(proc.osc3Params.coarse) + proc.modMatrix.getValue(proc.osc3Params.fine) :
		proc.osc3Params.coarse->getUserValue()  + proc.osc3Params.fine->getUserValue();
    auto vizEpi2PhaseIncrement = epi2Ratio * phaseIncrement;
    vizEpi2Phase += vizEpi2PhaseIncrement * speed;
    if (vizEpi2Phase > juce::MathConstants<float>::twoPi)
        vizEpi2Phase -= juce::MathConstants<float>::twoPi;

    auto epi3Ratio = live ?
		proc.modMatrix.getValue(proc.osc4Params.coarse) + proc.modMatrix.getValue(proc.osc4Params.fine) :
		proc.osc4Params.coarse->getUserValue()  + proc.osc4Params.fine->getUserValue();
    auto vizEpi3PhaseIncrement = epi3Ratio * phaseIncrement;
    vizEpi3Phase += vizEpi3PhaseIncrement * speed;
    if (vizEpi3Phase > juce::MathConstants<float>::twoPi)
        vizEpi3Phase -= juce::MathConstants<float>::twoPi;
	
	auto equant = live ? proc.modMatrix.getValue(proc.timbreParams.equant) : proc.timbreParams.equant->getUserValue();
	auto vol1 = live ? proc.modMatrix.getValue(proc.osc1Params.volume) : proc.osc1Params.volume->getUserValue();
	auto vol2 = live ? proc.modMatrix.getValue(proc.osc2Params.volume) : proc.osc2Params.volume->getUserValue();
	auto vol3 = live ? proc.modMatrix.getValue(proc.osc3Params.volume) : proc.osc3Params.volume->getUserValue();
	auto vol4 = live ? proc.modMatrix.getValue(proc.osc4Params.volume) : proc.osc4Params.volume->getUserValue();
	auto squash = live ? proc.modMatrix.getValue(proc.globalParams.squash) : proc.globalParams.squash->getUserValue();

    orbitViz.setEquant(equant);
    orbitViz.setDefRad(vol1);
    orbitViz.setEpi1Rad(vol2);
    orbitViz.setEpi2Rad(vol3);
    orbitViz.setEpi3Rad(vol4);
    orbitViz.setAlgo(proc.timbreParams.algo->getUserValueInt());
    orbitViz.setDefPhase(vizDefPhase);
    orbitViz.setEpi1Phase(vizEpi1Phase);
    orbitViz.setEpi2Phase(vizEpi2Phase);
    orbitViz.setEpi3Phase(vizEpi3Phase);
	orbitViz.setScale(proc.orbitParams.scale->getUserValue());
	orbitViz.setSquash(squash);
    orbitViz.repaint();
}

void Editor::resized()
{
    auto area = getLocalBounds();
    auto height = area.getHeight();
    orbitViz.setBounds(area.getRight() - (394), (int)(height * 0.5f), 394, (int)(height * 0.5f));
	liveViz.setBounds(area.getRight() - 389, (int)(height * 0.5f) + 5, 55, 25);
    
    auto f = juce::File (__FILE__).getChildFile("../../assets/layout.json");

    layout.setLayout("layout.json", f);
}

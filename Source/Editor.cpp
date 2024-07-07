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
    addAndMakeVisible(samplerBox);
    addAndMakeVisible(matrix);
    addAndMakeVisible(mseg);
	addAndMakeVisible(macros);
	addAndMakeVisible(volume);
	proc.samplerParams.key->addListener(this);
	proc.samplerParams.start->addListener(this);
	proc.samplerParams.end->addListener(this);
	proc.samplerParams.loopstart->addListener(this);
	proc.samplerParams.loopend->addListener(this);
}

Editor::~Editor()
{
	proc.samplerParams.key->removeListener(this);
	proc.samplerParams.start->removeListener(this);
	proc.samplerParams.end->removeListener(this);
	proc.samplerParams.loopstart->removeListener(this);
	proc.samplerParams.loopend->removeListener(this);
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
	if (param == proc.samplerParams.key && proc.sampler.sound.data != nullptr) {
		proc.sampler.updateBaseNote(proc.samplerParams.key->getUserValueInt());
	}
	else if (param == proc.samplerParams.start) {
		samplerBox.waveform.shouldRedraw = true;
		samplerBox.waveform.repaint();
	}
	else if (param == proc.samplerParams.end) {
		samplerBox.waveform.shouldRedraw = true;
		samplerBox.waveform.repaint();
	}
	else if (param == proc.samplerParams.loopstart) {
		samplerBox.waveform.shouldRedraw = true;
		samplerBox.waveform.repaint();
	}
	else if (param == proc.samplerParams.loopend) {
		samplerBox.waveform.shouldRedraw = true;
		samplerBox.waveform.repaint();
	}
}

//void Editor::timerCallback() {}

void Editor::resized()
{
    auto area = getLocalBounds();

	if (area.getWidth() > 1186 || area.getHeight() > 725) {
		return;
	}

    auto height = area.getHeight();
    //orbitViz.setBounds(area.getRight() - (394), (int)(height * 0.5f), 394, (int)(height * 0.5f));
	//liveViz.setBounds(area.getRight() - 389, (int)(height * 0.5f) + 5, 55, 25);
    
    osc.setBounds(0,0,280,163);
    env.setBounds(282,0,336,163);
	lfo.setBounds(0,163,280,163);
    filter.setBounds(282,163,112,163);
    timbre.setBounds(396,163,222,162);
    global.setBounds(396+54,326,168,163);
    aux.setBounds(620,0,280,163);
    samplerBox.setBounds(620,163,280,210+23+23);
    modsrc.setBounds(620,419,280,233);
    matrix.setBounds(902,0,280,489);
    mseg.setBounds(0, 326, 56*8, 323);
	macros.setBounds(396+54, 491, 168, 163);
	volume.setBounds(902, 491, 280, 163);

    // orbit.setBounds(1074,163,110,163);
}

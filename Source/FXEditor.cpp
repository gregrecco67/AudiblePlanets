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

#include "FXEditor.h"

FXEditor::FXEditor(APAudioProcessor& proc_)
	: proc(proc_)
{
	addAndMakeVisible(fxa1Box);
	addAndMakeVisible(fxa2Box);
	addAndMakeVisible(fxa3Box);
	addAndMakeVisible(fxa4Box);
	addAndMakeVisible(fxb1Box);
	addAndMakeVisible(fxb2Box);
	addAndMakeVisible(fxb3Box);
	addAndMakeVisible(fxb4Box);
	
	addAndMakeVisible(chainSetting);
    addAndMakeVisible(mod);
	addAndMakeVisible(matrix);

	proc.fxOrderParams.chainAtoB->addListener(this);
    proc.fxOrderParams.fxa1->addListener(this);
    proc.fxOrderParams.fxa2->addListener(this);
    proc.fxOrderParams.fxa3->addListener(this);
    proc.fxOrderParams.fxa4->addListener(this);
    proc.fxOrderParams.fxb1->addListener(this);
    proc.fxOrderParams.fxb2->addListener(this);
    proc.fxOrderParams.fxb3->addListener(this);
    proc.fxOrderParams.fxb4->addListener(this);
	proc.globalParams.pitchbendRange->addListener(this);
    
	addAndMakeVisible(fxa1Selector);
	addAndMakeVisible(fxa2Selector);
	addAndMakeVisible(fxa3Selector);
	addAndMakeVisible(fxa4Selector);
	addAndMakeVisible(fxb1Selector);
	addAndMakeVisible(fxb2Selector);
	addAndMakeVisible(fxb3Selector);
	addAndMakeVisible(fxb4Selector);

	valueUpdated(proc.fxOrderParams.fxa1);
	valueUpdated(proc.fxOrderParams.fxa2);
	valueUpdated(proc.fxOrderParams.fxa3);
	valueUpdated(proc.fxOrderParams.fxa4);
	valueUpdated(proc.fxOrderParams.fxb1);
	valueUpdated(proc.fxOrderParams.fxb2);
	valueUpdated(proc.fxOrderParams.fxb3);
	valueUpdated(proc.fxOrderParams.fxb4);
    
    addAndMakeVisible(pbBox);
	addAndMakeVisible(laneAGain);
	addAndMakeVisible(laneBGain);
}


void FXEditor::resized()
{
    mod.setBounds(10, 7, 280, 303);
	matrix.setBounds(10, 320, 280, 320);
	
	fxa1Selector.setBounds(300, 7, 168, 35); // place these above, adjust the panels' size
	fxa2Selector.setBounds(478, 7, 168, 35);
	fxa3Selector.setBounds(656, 7, 168, 35);
	fxa4Selector.setBounds(834, 7, 168, 35);
	
	fxa1Box.setBounds(300, 30, 168, 233);
	fxa2Box.setBounds(478, 30, 168, 233);
	fxa3Box.setBounds(656, 30, 168, 233);
	fxa4Box.setBounds(834, 30, 168, 233);
	
	fxb1Selector.setBounds(300, 320, 168, 35);
	fxb2Selector.setBounds(478, 320, 168, 35);
	fxb3Selector.setBounds(656, 320, 168, 35);
	fxb4Selector.setBounds(834, 320, 168, 35);
	
	fxb1Box.setBounds(300, 343, 168, 233);
	fxb2Box.setBounds(478, 343, 168, 233);
	fxb3Box.setBounds(656, 343, 168, 233);
	fxb4Box.setBounds(834, 343, 168, 233);
	
	chainSetting.setBounds(1009, 289, 168, 35);
    
	laneAGain.setBounds(1065, 113, 70, 87);
	laneBGain.setBounds(1065, 426, 70, 87);

    pbBox.setBounds(1124, 560, 56, 90);
}

void FXEditor::valueUpdated(gin::Parameter* param) // we'll use this to set any other box with the same effect selected to "None"
{
    if (param == proc.globalParams.pitchbendRange) {
        proc.updatePitchbend();
        return;
    }
    if (param == proc.fxOrderParams.fxa1) {
        auto fxa1Choice = param->getUserValueInt();
        fxa1Box.setControls(fxa1Choice);
		removeDuplicates(fxa1Choice, 1);
        return;
    }
	if (param == proc.fxOrderParams.fxa2) {
		auto fxa2Choice = param->getUserValueInt();
		fxa2Box.setControls(fxa2Choice);
		removeDuplicates(fxa2Choice, 2);
        return;
    }
	if (param == proc.fxOrderParams.fxa3) {
		auto fxa3Choice = param->getUserValueInt();
		fxa3Box.setControls(fxa3Choice);
		removeDuplicates(fxa3Choice, 3);
        return;
    }
	if (param == proc.fxOrderParams.fxa4) {
		auto fxa4Choice = param->getUserValueInt();
		fxa4Box.setControls(fxa4Choice);
		removeDuplicates(fxa4Choice, 4);
        return;
	}
	if (param == proc.fxOrderParams.fxb1) {
		auto fxb1Choice = param->getUserValueInt();
		fxb1Box.setControls(fxb1Choice);
		removeDuplicates(fxb1Choice, 5);
        return;
	}
	if (param == proc.fxOrderParams.fxb2) {
		auto fxb2Choice = param->getUserValueInt();
		fxb2Box.setControls(fxb2Choice);
		removeDuplicates(fxb2Choice, 6);
        return;
	}
	if (param == proc.fxOrderParams.fxb3) {
		auto fxb3Choice = param->getUserValueInt();
		fxb3Box.setControls(fxb3Choice);
		removeDuplicates(fxb3Choice, 7);
        return;
	}
	if (param == proc.fxOrderParams.fxb4) {
		auto fxb4Choice = param->getUserValueInt();
		fxb4Box.setControls(fxb4Choice);
		removeDuplicates(fxb4Choice, 8);
        return;
	}
}

void FXEditor::removeDuplicates(int effect, int componentToSkip)
{
	if (proc.fxOrderParams.fxa1->getUserValueInt() == effect) { // abstract this...
		if (componentToSkip != 1) {
			fxa1Box.setControls(0);
			proc.fxOrderParams.fxa1->setValue(0);
		}
	}
	if (proc.fxOrderParams.fxa2->getUserValueInt() == effect) { // abstract this...
		if (componentToSkip != 2) {
			fxa2Box.setControls(0);
			proc.fxOrderParams.fxa2->setValue(0);
		}
	}
	if (proc.fxOrderParams.fxa3->getUserValueInt() == effect) { // abstract this...
		if (componentToSkip != 3) {
			fxa3Box.setControls(0);
			proc.fxOrderParams.fxa3->setValue(0);
		}
	}
	if (proc.fxOrderParams.fxa4->getUserValueInt() == effect) { // abstract this...
		if (componentToSkip != 4) {
			fxa4Box.setControls(0);
			proc.fxOrderParams.fxa4->setValue(0);
		}
	}
	if (proc.fxOrderParams.fxb1->getUserValueInt() == effect) { // abstract this...
		if (componentToSkip != 5) {
			fxb1Box.setControls(0);
			proc.fxOrderParams.fxb1->setValue(0);
		}
	}
	if (proc.fxOrderParams.fxb2->getUserValueInt() == effect) { // abstract this...
		if (componentToSkip != 6) {
			fxb2Box.setControls(0);
			proc.fxOrderParams.fxb2->setValue(0);
		}
	}
	if (proc.fxOrderParams.fxb3->getUserValueInt() == effect) { // abstract this...
		if (componentToSkip != 7) {
			fxb3Box.setControls(0);
			proc.fxOrderParams.fxb3->setValue(0);
		}
	}
	if (proc.fxOrderParams.fxb4->getUserValueInt() == effect) { // abstract this...
		if (componentToSkip != 8) {
			fxb4Box.setControls(0);
			proc.fxOrderParams.fxb4->setValue(0);
		}
	}
}


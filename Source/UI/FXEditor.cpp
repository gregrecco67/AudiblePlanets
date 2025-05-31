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

FXEditor::FXEditor(APAudioProcessor &proc_) : proc(proc_)
{
	addAndMakeVisible(fxa1Box);
	addAndMakeVisible(fxa2Box);
	addAndMakeVisible(fxa3Box);
	addAndMakeVisible(fxa4Box);
	addAndMakeVisible(fxb1Box);
	addAndMakeVisible(fxb2Box);
	addAndMakeVisible(fxb3Box);
	addAndMakeVisible(fxb4Box);
	fxa1Box.setOutline(false);
	fxa2Box.setOutline(false);
	fxa3Box.setOutline(false);
	fxa4Box.setOutline(false);
	fxb1Box.setOutline(false);
	fxb2Box.setOutline(false);
	fxb3Box.setOutline(false);
	fxb4Box.setOutline(false);

	fxa1Selector.setShowName(false);
	fxa2Selector.setShowName(false);
	fxa3Selector.setShowName(false);
	fxa4Selector.setShowName(false);
	fxb1Selector.setShowName(false);
	fxb2Selector.setShowName(false);
	fxb3Selector.setShowName(false);
	fxb4Selector.setShowName(false);

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

	addAndMakeVisible(laneALabel);

	addAndMakeVisible(laneAGain);
	addAndMakeVisible(laneBGain);
	addAndMakeVisible(laneAFreq);
	addAndMakeVisible(laneBFreq);
	addAndMakeVisible(laneAType);
	addAndMakeVisible(laneBType);
	addAndMakeVisible(laneARes);
	addAndMakeVisible(laneBRes);
	addAndMakeVisible(laneAPrePost);
	addAndMakeVisible(laneBPrePost);
	addAndMakeVisible(laneAPan);
	addAndMakeVisible(laneBPan);

	addAndMakeVisible(laneBLabel);

	mod.setHeaderRight(false);
	matrix.setHeaderRight(false);

	setAllControls();
}

FXEditor::~FXEditor()
{
	proc.fxOrderParams.chainAtoB->removeListener(this);
	proc.fxOrderParams.fxa1->removeListener(this);
	proc.fxOrderParams.fxa2->removeListener(this);
	proc.fxOrderParams.fxa3->removeListener(this);
	proc.fxOrderParams.fxa4->removeListener(this);
	proc.fxOrderParams.fxb1->removeListener(this);
	proc.fxOrderParams.fxb2->removeListener(this);
	proc.fxOrderParams.fxb3->removeListener(this);
	proc.fxOrderParams.fxb4->removeListener(this);
}

void FXEditor::setGrid(
    gin::ParamBox *box, float x, float y, float heds, float w, float h)
{
	box->setBounds(static_cast<int>(x * 56.f), static_cast<int>(y * 70.f + 23.f * heds),
	    static_cast<int>(w * 56.f), static_cast<int>(h * 70.f + 23.f));
}

void FXEditor::resized()
{
	//setGrid(&mod, 16, 0, 0, 5, 4.328571f);
    mod.setBounds(16 * 56, 0, 5 * 56, 326);
	matrix.setBounds(16 * 56, 326, 5 * 56, 326);

	fxa1Selector.setBounds(188, 7, 168, 15);
	fxa2Selector.setBounds(366, 7, 168, 15);
	fxa3Selector.setBounds(544, 7, 168, 15);
	fxa4Selector.setBounds(722, 7, 168, 15);

	fxa1Box.setBounds(188, 30, 168, 233);
	fxa2Box.setBounds(366, 30, 168, 233);
	fxa3Box.setBounds(544, 30, 168, 233);
	fxa4Box.setBounds(722, 30, 168, 233);

	fxb1Selector.setBounds(188, 320, 168, 15);
	fxb2Selector.setBounds(366, 320, 168, 15);
	fxb3Selector.setBounds(544, 320, 168, 15);
	fxb4Selector.setBounds(722, 320, 168, 15);

	fxb1Box.setBounds(188, 343, 168, 233);
	fxb2Box.setBounds(366, 343, 168, 233);
	fxb3Box.setBounds(544, 343, 168, 233);
	fxb4Box.setBounds(722, 343, 168, 233);

	laneALabel.setBounds(20, 23, 168, 30);

	laneAFreq.setBounds(20, 60, 56, 70);
	laneARes.setBounds(90, 60, 56, 70);
	laneAGain.setBounds(20, 130, 56, 70);
	laneAPan.setBounds(90, 130, 56, 70);
	laneAType.setBounds(20, 200, 56, 70);
	laneAPrePost.setBounds(90, 200, 56, 70);

	chainSetting.setBounds(10, 289, 168, 35);

	laneBFreq.setBounds(20, 343, 56, 70);
	laneBRes.setBounds(90, 343, 56, 70);
	laneBGain.setBounds(20, 413, 56, 70);
	laneBPan.setBounds(90, 413, 56, 70);
	laneBType.setBounds(20, 487, 56, 70);
	laneBPrePost.setBounds(90, 487, 56, 70);

	laneBLabel.setBounds(20, 564, 168, 30);

	setAllControls();
}

void FXEditor::setAllControls()
{
	fxa1Box.setControls(proc.fxOrderParams.fxa1->getUserValueInt());
	fxa2Box.setControls(proc.fxOrderParams.fxa2->getUserValueInt());
	fxa3Box.setControls(proc.fxOrderParams.fxa3->getUserValueInt());
	fxa4Box.setControls(proc.fxOrderParams.fxa4->getUserValueInt());
	fxb1Box.setControls(proc.fxOrderParams.fxb1->getUserValueInt());
	fxb2Box.setControls(proc.fxOrderParams.fxb2->getUserValueInt());
	fxb3Box.setControls(proc.fxOrderParams.fxb3->getUserValueInt());
	fxb4Box.setControls(proc.fxOrderParams.fxb4->getUserValueInt());
}

void FXEditor::valueUpdated(
    gin::Parameter *param)  // we'll use this to set any other box with the same
                            // effect selected to "None"
{
	if (param == proc.fxOrderParams.fxa1) {
		const auto fxa1Choice = param->getUserValueInt();
		fxa1Box.setControls(fxa1Choice);
		removeDuplicates(fxa1Choice, 1);
	} else if (param == proc.fxOrderParams.fxa2) {
		const auto fxa2Choice = param->getUserValueInt();
		fxa2Box.setControls(fxa2Choice);
		removeDuplicates(fxa2Choice, 2);
	} else if (param == proc.fxOrderParams.fxa3) {
		const auto fxa3Choice = param->getUserValueInt();
		fxa3Box.setControls(fxa3Choice);
		removeDuplicates(fxa3Choice, 3);
	} else if (param == proc.fxOrderParams.fxa4) {
		const auto fxa4Choice = param->getUserValueInt();
		fxa4Box.setControls(fxa4Choice);
		removeDuplicates(fxa4Choice, 4);
	} else if (param == proc.fxOrderParams.fxb1) {
		const auto fxb1Choice = param->getUserValueInt();
		fxb1Box.setControls(fxb1Choice);
		removeDuplicates(fxb1Choice, 5);
	} else if (param == proc.fxOrderParams.fxb2) {
		const auto fxb2Choice = param->getUserValueInt();
		fxb2Box.setControls(fxb2Choice);
		removeDuplicates(fxb2Choice, 6);
	} else if (param == proc.fxOrderParams.fxb3) {
		const auto fxb3Choice = param->getUserValueInt();
		fxb3Box.setControls(fxb3Choice);
		removeDuplicates(fxb3Choice, 7);
	} else if (param == proc.fxOrderParams.fxb4) {
		const auto fxb4Choice = param->getUserValueInt();
		fxb4Box.setControls(fxb4Choice);
		removeDuplicates(fxb4Choice, 8);
	}
	setAllControls();
}

void FXEditor::removeDuplicates(int effect, int componentToSkip)
{
	if (proc.fxOrderParams.fxa1->getUserValueInt() ==
	    effect) {  // abstract this...
		if (componentToSkip != 1) {
			fxa1Box.setControls(0);
			proc.fxOrderParams.fxa1->setValue(0);
		}
	}
	if (proc.fxOrderParams.fxa2->getUserValueInt() ==
	    effect) {  // abstract this...
		if (componentToSkip != 2) {
			fxa2Box.setControls(0);
			proc.fxOrderParams.fxa2->setValue(0);
		}
	}
	if (proc.fxOrderParams.fxa3->getUserValueInt() ==
	    effect) {  // abstract this...
		if (componentToSkip != 3) {
			fxa3Box.setControls(0);
			proc.fxOrderParams.fxa3->setValue(0);
		}
	}
	if (proc.fxOrderParams.fxa4->getUserValueInt() ==
	    effect) {  // abstract this...
		if (componentToSkip != 4) {
			fxa4Box.setControls(0);
			proc.fxOrderParams.fxa4->setValue(0);
		}
	}
	if (proc.fxOrderParams.fxb1->getUserValueInt() ==
	    effect) {  // abstract this...
		if (componentToSkip != 5) {
			fxb1Box.setControls(0);
			proc.fxOrderParams.fxb1->setValue(0);
		}
	}
	if (proc.fxOrderParams.fxb2->getUserValueInt() ==
	    effect) {  // abstract this...
		if (componentToSkip != 6) {
			fxb2Box.setControls(0);
			proc.fxOrderParams.fxb2->setValue(0);
		}
	}
	if (proc.fxOrderParams.fxb3->getUserValueInt() ==
	    effect) {  // abstract this...
		if (componentToSkip != 7) {
			fxb3Box.setControls(0);
			proc.fxOrderParams.fxb3->setValue(0);
		}
	}
	if (proc.fxOrderParams.fxb4->getUserValueInt() ==
	    effect) {  // abstract this...
		if (componentToSkip != 8) {
			fxb4Box.setControls(0);
			proc.fxOrderParams.fxb4->setValue(0);
		}
	}
}

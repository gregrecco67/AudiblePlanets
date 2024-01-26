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
	proc.fxOrderParams.chainAtoB->addListener(this);
    proc.fxOrderParams.fxa1->addListener(this);
    proc.fxOrderParams.fxa2->addListener(this);
    proc.fxOrderParams.fxa3->addListener(this);
    proc.fxOrderParams.fxa4->addListener(this);
    proc.fxOrderParams.fxb1->addListener(this);
    proc.fxOrderParams.fxb2->addListener(this);
    proc.fxOrderParams.fxb3->addListener(this);
    proc.fxOrderParams.fxb4->addListener(this);
    addAndMakeVisible(hello);
	addAndMakeVisible(fxa1Selector);
	addAndMakeVisible(fxa2Selector);
	addAndMakeVisible(fxa3Selector);
	addAndMakeVisible(fxa4Selector);
	addAndMakeVisible(fxb1Selector);
	addAndMakeVisible(fxb2Selector);
	addAndMakeVisible(fxb3Selector);
	addAndMakeVisible(fxb4Selector);
}


void FXEditor::resized()
{
	auto area = getLocalBounds();
	fxa1Box.setBounds(150, 50, 168, 233);
	fxa2Box.setBounds(378, 50, 168, 233);
	fxa3Box.setBounds(606, 50, 168, 233);
	fxa4Box.setBounds(834, 50, 168, 233);
	fxb1Box.setBounds(150, 323, 168, 233);
	fxb2Box.setBounds(378, 323, 168, 233);
	fxb3Box.setBounds(606, 323, 168, 233);
	fxb4Box.setBounds(834, 323, 168, 233);
	chainSetting.setBounds(15, 7, 120, 35);
	hello.setBounds(15, 50, 120, 35);

	fxa1Selector.setBounds(150, 7, 168, 35); // place these above, adjust the panels' size
	fxa2Selector.setBounds(378, 7, 168, 35);
	fxa3Selector.setBounds(606, 7, 168, 35);
	fxa4Selector.setBounds(834, 7, 168, 35);
	fxb1Selector.setBounds(150, 290, 168, 35);
	fxb2Selector.setBounds(378, 290, 168, 35);
	fxb3Selector.setBounds(606, 290, 168, 35);
	fxb4Selector.setBounds(834, 290, 168, 35);

}

void FXEditor::valueUpdated(gin::Parameter* param) // we'll use this to set any other box with the same effect selected to "None"
{
    if (param == proc.fxOrderParams.fxa1) {
        fxa1Box.setControls(param->getUserValueInt());
    }
    if (param == proc.fxOrderParams.fxa2) {
        fxa2Box.setControls(param->getUserValueInt());
    }
    if (param == proc.fxOrderParams.fxa3) {
        fxa3Box.setControls(param->getUserValueInt());
    }
    if (param == proc.fxOrderParams.fxa4) {
        fxa4Box.setControls(param->getUserValueInt());
    }
    if (param == proc.fxOrderParams.fxb1) {
        fxb1Box.setControls(param->getUserValueInt());
    }
    if (param == proc.fxOrderParams.fxb2) {
        fxb2Box.setControls(param->getUserValueInt());
    }
    if (param == proc.fxOrderParams.fxb3) {
        fxb3Box.setControls(param->getUserValueInt());
    }
    if (param == proc.fxOrderParams.fxb4) {
        fxb4Box.setControls(param->getUserValueInt());
    }
}


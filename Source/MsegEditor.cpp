#include "MsegEditor.h"

MsegEditor::MsegEditor(APAudioProcessor& proc_) : proc(proc_)
{
	addAndMakeVisible(msegMatrixBox);
	addAndMakeVisible(msegModBox);
	addAndMakeVisible(mseg1Box);
	addAndMakeVisible(mseg2Box);
	addAndMakeVisible(mseg3Box);
	addAndMakeVisible(mseg4Box);
	startTimerHz(4);
}

void MsegEditor::resized()
{
	mseg1Box.setBounds(0, 0, 450, 323);
	mseg2Box.setBounds(452, 0, 450, 323);
	mseg3Box.setBounds(0, 329, 450, 323);
	mseg4Box.setBounds(452, 329, 450, 323);
	msegModBox.setBounds(905, 0, 270, 323);
	msegMatrixBox.setBounds(905, 329, 270, 323);
}

void MsegEditor::timerCallback()
{
	mseg1Box.msegComponent.markDirty();
	mseg2Box.msegComponent.markDirty();
	mseg3Box.msegComponent.markDirty();
	mseg4Box.msegComponent.markDirty();
}

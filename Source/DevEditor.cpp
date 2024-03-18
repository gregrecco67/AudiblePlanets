#include "DevEditor.h"

DevEditor::DevEditor(APAudioProcessor& proc_) : proc(proc_)
{
	addAndMakeVisible(mseg1Box);
	addAndMakeVisible(mseg2Box);
	addAndMakeVisible(mseg3Box);
	addAndMakeVisible(mseg4Box);
}

void DevEditor::resized()
{
	mseg1Box.setBounds(0, 0, 400, 293);
	mseg2Box.setBounds(0, 298, 400, 293);
	mseg3Box.setBounds(405, 0, 400, 293);
	mseg4Box.setBounds(405, 298, 400, 293);
}



void DevEditor::valueUpdated(gin::Parameter* param)
{
}

#include "MacrosEditor.h"

MacrosEditor::MacrosEditor(APAudioProcessor& proc_) : proc(proc_)
{
	addAndMakeVisible(macro1);
	addAndMakeVisible(macro2);
	addAndMakeVisible(macro3);
	addAndMakeVisible(macro4);
}

void MacrosEditor::resized()
{
	//matrix.setBounds(5, 5, 56 * 8, 70 * 5 + 23);
	macro1.setBounds(0, 0, 450, 323);
	macro2.setBounds(455, 0, 435, 323);
	macro3.setBounds(0, 329, 450, 323);
	macro4.setBounds(455, 329, 435, 323);
}



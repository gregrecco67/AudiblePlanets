#include "MacrosEditor.h"

MacrosEditor::MacrosEditor(APAudioProcessor& proc_) : proc(proc_)
{
	addAndMakeVisible(macro1);
	addAndMakeVisible(macro2);
	addAndMakeVisible(macro3);
	addAndMakeVisible(macro4);
	addAndMakeVisible(aux);
	addAndMakeVisible(macrosModBox);
}

void MacrosEditor::resized()
{
	macro1.setBounds(0, 0, 450, 323);
	macro2.setBounds(455, 0, 450, 323);
	macro3.setBounds(0, 329, 450, 323);
	macro4.setBounds(455, 329, 450, 323);
	aux.setBounds(905, 0, 5*56, 2*70 + 23);
	macrosModBox.setBounds(905, 2*70 + 23, 5*56, 5*70 + 23);
}



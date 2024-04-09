#include "MacrosEditor.h"

MacrosEditor::MacrosEditor(APAudioProcessor& proc_) : proc(proc_)
{
	addAndMakeVisible(matrix);
	addAndMakeVisible(paramSelector1);
	addAndMakeVisible(paramSelector2);
	addAndMakeVisible(paramSelector3);
	addAndMakeVisible(paramSelector4);
		




}

void MacrosEditor::resized()
{
	//matrix.setBounds(5, 5, 56 * 8, 70 * 5 + 23);
	paramSelector1.setBounds(5, 5, 125, 25);
	paramSelector2.setBounds(5, 35, 125, 25);
	paramSelector3.setBounds(5, 65, 125, 25);
	paramSelector4.setBounds(5, 95, 125, 25);
}



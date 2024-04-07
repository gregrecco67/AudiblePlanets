#include "MacrosEditor.h"

MacrosEditor::MacrosEditor(APAudioProcessor& proc_) : proc(proc_)
{
	addAndMakeVisible(matrix);
	
}

void MacrosEditor::resized()
{
	matrix.setBounds(5, 5, 56 * 8, 70 * 5 + 23);
}

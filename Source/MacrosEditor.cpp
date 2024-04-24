#include "MacrosEditor.h"

MacrosEditor::MacrosEditor(APAudioProcessor& proc_) : proc(proc_)
{
	addAndMakeVisible(macro1);
	addAndMakeVisible(macro2);
	addAndMakeVisible(macro3);
	addAndMakeVisible(macro4);
	addAndMakeVisible(aux);
	addAndMakeVisible(macrosModBox);
	addAndMakeVisible(samplerBox);
	proc.samplerParams.key->addListener(this);
	proc.samplerParams.start->addListener(this);
	proc.samplerParams.end->addListener(this);
	proc.samplerParams.loopstart->addListener(this);
	proc.samplerParams.loopend->addListener(this);
}

MacrosEditor::~MacrosEditor()
{
	proc.samplerParams.key->removeListener(this);
	proc.samplerParams.start->removeListener(this);
	proc.samplerParams.end->removeListener(this);
	proc.samplerParams.loopstart->removeListener(this);
	proc.samplerParams.loopend->removeListener(this);
}

void MacrosEditor::resized()
{
	macro1.setBounds(0, 0, 450, 323);
	macro2.setBounds(455, 0, 450, 323);
	macro3.setBounds(0, 329, 450, 323);
	macro4.setBounds(455, 329, 450, 323);
	aux.setBounds(905, 0, 5*56, 2*70 + 23);
	macrosModBox.setBounds(905, 2*70 + 23, 5*56, 233);
	samplerBox.setBounds(905, 396, 5*56, 210 + 23 + 23);
}

bool MacrosEditor::isInterestedInFileDrag(const juce::StringArray& files)
{
	if (files.size() == 1)
	{
		juce::File file{ files[0] };
		if (file.hasFileExtension("wav") 
			|| file.hasFileExtension("aif") 
			|| file.hasFileExtension("mp3") 
			|| file.hasFileExtension("flac") 
			|| file.hasFileExtension("ogg")) { return true; }
	}

	return false;
}

void MacrosEditor::filesDropped(const juce::StringArray& files, int /*x*/, int /*y*/)
{
	juce::File file{ files[0] };
	if (isInterestedInFileDrag(files[0]))
	{
		proc.sampler.loadSound(files[0]);
		samplerBox.waveform.shouldRedraw = true;
		samplerBox.waveform.repaint();
	}
}

void MacrosEditor::valueUpdated(gin::Parameter* param)
{
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



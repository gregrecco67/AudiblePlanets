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
	addAndMakeVisible(sampleFilenameLabel);
	sampleFilenameLabel.setJustificationType(juce::Justification::centred);
	sampleFilenameLabel.setLookAndFeel(&lookAndFeel);
	proc.samplerParams.key->addListener(this);
	proc.samplerParams.start->addListener(this);
	proc.samplerParams.end->addListener(this);
}

void MacrosEditor::resized()
{
	macro1.setBounds(0, 0, 450, 323);
	macro2.setBounds(455, 0, 450, 323);
	macro3.setBounds(0, 329, 450, 323);
	macro4.setBounds(455, 329, 450, 323);
	aux.setBounds(905, 0, 5*56, 2*70 + 23);
	macrosModBox.setBounds(905, 2*70 + 23, 5*56, 303);
	samplerBox.setBounds(905, 466, 5*56, 140 + 23 + 23);
	sampleFilenameLabel.setBounds(905, 466 + 70 + 70 + 23, 5*56, 23);
	startTimerHz(4);
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
		proc.loadSample(files[0]);
		sampleFilenameLabel.setText(file.getFileName(), juce::dontSendNotification);
		samplerBox.waveform.shouldRedraw = true;
		samplerBox.waveform.repaint();
	}
	else { 
		sampleFilenameLabel.setText("Invalid file", juce::dontSendNotification);
		samplerBox.waveform.shouldRedraw = true;
		samplerBox.waveform.repaint();
	}
}

void MacrosEditor::valueUpdated(gin::Parameter* param)
{
	if (param == proc.samplerParams.key && !sampleFilenameLabel.getText().isEmpty()) {
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
}

void MacrosEditor::timerCallback()
{
	juce::String filename{ proc.sampler.sound.name };
	if (!filename.isEmpty()) {
		juce::File file{ filename };
		sampleFilenameLabel.setText(file.getFileName(), juce::dontSendNotification);
	}
	else {
		sampleFilenameLabel.setText("", juce::dontSendNotification);
	}
}



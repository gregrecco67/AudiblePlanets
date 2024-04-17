#pragma once
#include <JuceHeader.h>
#include <gin/gin.h>
#include "AuxSynthVoice.h"

class APAudioProcessor;

class AuxSynth : public gin::Synthesiser
{
public:
	AuxSynth(APAudioProcessor& proc_);
	~AuxSynth() override = default;

	void handleMidiEvent(const juce::MidiMessage& m) override;
	juce::Array<float> getLiveFilterCutoff();

	std::vector<float> getMSEG1Phases();
	std::vector<float> getMSEG2Phases();
	std::vector<float> getMSEG3Phases();
	std::vector<float> getMSEG4Phases();


private:
	APAudioProcessor& proc;

};

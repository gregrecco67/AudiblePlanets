#pragma once
#include <JuceHeader.h>
#include <gin/gin.h>
#include "SynthVoice.h"

class APAudioProcessor;

class APSynth : public gin::Synthesiser
{
public:
    APSynth(APAudioProcessor& proc_);
    ~APSynth() override = default;
    
    void handleMidiEvent(const juce::MidiMessage& m) override;
    juce::Array<float> getLiveFilterCutoff();

	std::vector<float> getMSEG1Phases();
	std::vector<float> getMSEG2Phases();
	std::vector<float> getMSEG3Phases();
	std::vector<float> getMSEG4Phases();
    
private:
    APAudioProcessor& proc;
    
};

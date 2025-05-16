#pragma once

#include <gin_dsp/gin_dsp.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "SynthVoice3.h"

class APAudioProcessor;

class APSynth : public gin::Synthesiser {
public:
	APSynth(APAudioProcessor &proc_);
	~APSynth() override = default;

	void handleMidiEvent(const juce::MidiMessage &m) override;
	juce::Array<float> getLiveFilterCutoff();

	std::vector<float> getMSEG1Phases();
	std::vector<float> getMSEG2Phases();
	std::vector<float> getMSEG3Phases();
	std::vector<float> getMSEG4Phases();
	std::vector<float> getLFO1Phases();
	std::vector<float> getLFO2Phases();
	std::vector<float> getLFO3Phases();
	std::vector<float> getLFO4Phases();

	void setCurrentPlaybackSampleRate(double newRate) override;

	template<typename floatType>
	void renderNextBlock(juce::AudioBuffer<floatType> &outputAudio,
	    const juce::MidiBuffer &inputMidi,
	    int startSample,
	    int numSamples)
	{
		// detach samplerate of this from that of parent class
		// create infrastructure to manage downsampling
		gin::Synthesiser::renderNextBlock(
		    outputAudio, inputMidi, startSample, numSamples);
	}

private:
	APAudioProcessor &proc;
};

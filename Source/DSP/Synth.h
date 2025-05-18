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
	juce::Array<float> getLiveFilterCutoff() const;

	std::vector<float> getMSEG1Phases() const;
	std::vector<float> getMSEG2Phases() const;
	std::vector<float> getMSEG3Phases() const;
	std::vector<float> getMSEG4Phases() const;
	std::vector<float> getLFO1Phases() const;
	std::vector<float> getLFO2Phases() const;
	std::vector<float> getLFO3Phases() const;
	std::vector<float> getLFO4Phases() const;

	void setCurrentPlaybackSampleRate(double newRate) override;

	template<typename floatType>
	void renderNextBlock(juce::AudioBuffer<floatType> &outputAudio,
	    const juce::MidiBuffer &inputMidi,
	    int startSample,
	    int numSamples)
	{
		gin::Synthesiser::renderNextBlock(
		    outputAudio, inputMidi, startSample, numSamples);
	}

private:
	APAudioProcessor &proc;
};

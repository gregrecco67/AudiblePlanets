/*
 * Audible Planets - an expressive, quasi-Ptolemaic semi-modular synthesizer
 *
 * Copyright 2024, Greg Recco
 *
 * Audible Planets is released under the GNU General Public Licence v3
 * or later (GPL-3.0-or-later). The license is found in the "LICENSE"
 * file in the root of this repository, or at
 * https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * All source for Audible Planets is available at
 * https://github.com/gregrecco67/AudiblePlanets
 */

#pragma once

#include <JuceHeader.h>
#include "Envelope.h"
#include "libMTSClient.h"
#include <numbers>
class APAudioProcessor;

using namespace std::numbers;
//==============================================================================
class AuxSynthVoice : public gin::SynthesiserVoice,
	public gin::ModVoice
{
public:
	AuxSynthVoice(APAudioProcessor& p);

	void noteStarted() override;
	void noteRetriggered() override;
	void noteStopped(bool allowTailOff) override;

	void notePressureChanged() override;
	void noteTimbreChanged() override;
	void notePitchbendChanged() override {}
	void noteKeyStateChanged() override {}

	void setCurrentSampleRate(double newRate) override;

	void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

	float getCurrentNote() override { return noteSmoother.getCurrentValue() * 127.0f; }

	bool isVoiceActive() override;

	float getFilterCutoffNormalized();
	// new
	float getMSEG1Phase();
	float getMSEG2Phase();
	float getMSEG3Phase();
	float getMSEG4Phase();

private:
	void updateParams(int blockSize);

	APAudioProcessor& proc;

	// new
	gin::LFO lfo1, lfo2, lfo3, lfo4;
	gin::MSEG mseg1, mseg2, mseg3, mseg4;
	gin::MSEG::Parameters mseg1Params, mseg2Params, mseg3Params, mseg4Params;
	// end new

	gin::BLLTVoicedStereoOscillator osc;

	gin::Filter filter;

	Envelope env1, env2, env3, env4;
	int currentEnv;
	float currentFreq{ 440.f };
		
	float currentMidiNote = -1;
	gin::VoicedStereoOscillatorParams oscParams;

	float osc1Note = 69.0f;
	float osc1Vol = 0.0f;

	float baseAmplitude = 0.15f;

	gin::EasedValueSmoother<float> noteSmoother;

	friend class AuxSynth;
	juce::MPENote curNote;
};

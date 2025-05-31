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

#include <gin_dsp/gin_dsp.h>
#include <gin_plugin/gin_plugin.h>
#include <numbers>
#include "Envelope.h"
#include "third_party/MTS-ESP/libMTSClient.h"
#include <cmath>
class APAudioProcessor;

using namespace std::numbers;
//==============================================================================
class AuxSynthVoice final : public gin::SynthesiserVoice, public gin::ModVoice {
public:
	explicit AuxSynthVoice(APAudioProcessor &p);

	void noteStarted() override;
	void noteRetriggered() override;
	void noteStopped(bool allowTailOff) override;

	void notePressureChanged() override;
	void noteTimbreChanged() override;
	void notePitchbendChanged() override {}
	void noteKeyStateChanged() override {}

	void setCurrentSampleRate(double newRate) override;

	void renderNextBlock(juce::AudioBuffer<float> &outputBuffer,
	                     int startSample,
	                     int numSamples) override;

	float getCurrentNote() override
	{
		return noteSmoother.getCurrentValue() * 127.0f;
	}

	bool isVoiceActive() override { return isActive(); }

	[[nodiscard]] float getFilterCutoffNormalized() const;

	[[nodiscard]] inline float getMSEG1Phase() const { return mseg1.getCurrentPhase(); }
	[[nodiscard]] inline float getMSEG2Phase() const { return mseg2.getCurrentPhase(); }
	[[nodiscard]] inline float getMSEG3Phase() const { return mseg3.getCurrentPhase(); }
	[[nodiscard]] inline float getMSEG4Phase() const { return mseg4.getCurrentPhase(); }

private:
	void updateParams(int blockSize);

	APAudioProcessor &proc;

	gin::BLLTVoicedStereoOscillator osc;
	gin::LFO lfo1, lfo2, lfo3, lfo4;
	gin::MSEG mseg1, mseg2, mseg3, mseg4;
	gin::MSEG::Parameters mseg1Params, mseg2Params, mseg3Params, mseg4Params;

	gin::Filter filter;

	Envelope env1, env2, env3, env4;
	int currentEnv{0};
	float currentFreq{440.f};

	float currentMidiNote = -1;
	gin::VoicedStereoOscillatorParams oscParams;

	float osc1Note = 69.0f;
	float osc1Vol = 0.0f;

	float baseAmplitude = 0.15f;

	gin::EasedValueSmoother<float> noteSmoother;

	friend class AuxSynth;
	juce::MPENote curNote;
	const float maxFreq{20000.f};
};

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
#include <juce_audio_basics/juce_audio_basics.h>
#include <numbers>
#include <random>
#include "Envelope.h"
#include "MTS-ESP/libMTSClient.h"
#include "Oscillator.h"
class APAudioProcessor;

using std::numbers::pi_v;

//==============================================================================
class SynthVoice3 final : public gin::SynthesiserVoice, public gin::ModVoice {
public:
	explicit SynthVoice3(APAudioProcessor &p);

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

	mipp::Reg<float> mix(const mipp::Reg<float>& aReg, const mipp::Reg<float>& bReg, const float mix);

	float getCurrentNote() override
	{
		return noteSmoother.getCurrentValue() * 127.0f;
	}

	bool isVoiceActive() override { return isActive(); }
	
	float getFilterCutoffNormalized() const;
	[[nodiscard]] inline float getMSEG1Phase() const { return mseg1.getCurrentPhase(); }
	[[nodiscard]] inline float getMSEG2Phase() const { return mseg2.getCurrentPhase(); }
	[[nodiscard]] inline float getMSEG3Phase() const { return mseg3.getCurrentPhase(); }
	[[nodiscard]] inline float getMSEG4Phase() const { return mseg4.getCurrentPhase(); }
	inline float getLFO1Phase() { return lfo1.getCurrentPhase(); }
	inline float getLFO2Phase() { return lfo2.getCurrentPhase(); }
	inline float getLFO3Phase() { return lfo3.getCurrentPhase(); }
	inline float getLFO4Phase() { return lfo4.getCurrentPhase(); }
	
	[[nodiscard]] inline Envelope::EnvelopeState getENV1State() const { return env1.getState(); }
	[[nodiscard]] inline Envelope::EnvelopeState getENV2State() const { return env2.getState(); }
	[[nodiscard]] inline Envelope::EnvelopeState getENV3State() const { return env3.getState(); }
	[[nodiscard]] inline Envelope::EnvelopeState getENV4State() const { return env4.getState(); }
	
	gin::Wave waveForChoice(const int choice);

private:
	void updateParams(int blockSize);

	APAudioProcessor &proc;

	gin::Filter filter;
	gin::LFO lfo1, lfo2, lfo3, lfo4;
	gin::MSEG mseg1, mseg2, mseg3, mseg4;
	APOscillator osc1, osc2, osc3, osc4;
	float lastp1{0.f}, lastp2{0.f}, lastp3{0.f}, lastp4{0.f};  // last phase
	gin::MSEG::Parameters mseg1Params, mseg2Params, mseg3Params, mseg4Params;

	Envelope env1, env2, env3, env4;
	std::array<Envelope *, 4> envs{&env1, &env2, &env3, &env4};
	std::array<Envelope *, 4> envsByNum{&env1, &env2, &env3, &env4};

	int filterType{0};

	float osc1xs[128]{0.f};  // at 4x sr, processed down to 32
	float osc1ys[128]{0.f};  // direct output from oscillators
	float osc2xs[128]{0.f};
	float osc2ys[128]{0.f};
	float osc3xs[128]{0.f};
	float osc3ys[128]{0.f};
	float osc4xs[128]{0.f};
	float osc4ys[128]{0.f};

	// the same, as registers
	mipp::Reg<float> osc1x, osc1y, osc2x, osc2y, osc3x, osc3y, osc4x, osc4y;

	// interpreted positions, added together according to algo
	mipp::Reg<float> epi1xs[32]{0.f};
	mipp::Reg<float> epi1ys[32]{0.f};
	mipp::Reg<float> epi2xs[32]{0.f};
	mipp::Reg<float> epi2ys[32]{0.f};
	mipp::Reg<float> epi3xs[32]{0.f};
	mipp::Reg<float> epi3ys[32]{0.f};
	mipp::Reg<float> epi4xs[32]{0.f};
	mipp::Reg<float> epi4ys[32]{0.f};

	int tilUpdate{0};  // only update envelopes/lfo/mseg every 4th block

	// distances and inverse distances
	mipp::Reg<float> dist2sq, dist2, invDist2;
	mipp::Reg<float> dist3sq, dist3, invDist3;
	mipp::Reg<float> dist4sq, dist4, invDist4;

	float currentMidiNote = -1;
	APOscillator::Settings osc1Params, osc2Params, osc3Params, osc4Params;
	float osc1Freq = 0.0f, osc2Freq = 0.0f, osc3Freq = 0.0f, osc4Freq = 0.0f;
	float osc1Vol = 0.0f, osc2Vol = 0.0f, osc3Vol = 0.0f, osc4Vol = 0.0f;
	int algo{0};
	float equant{0.f};

	static constexpr float baseAmplitude = 0.12f;

	gin::EasedValueSmoother<float> noteSmoother;

	double fna0, fnb1, fnz1, fqa0, fqb1, fqz1;

	juce::AudioBuffer<float> synthBuffer;

	mipp::Reg<float> sine2{0.f, 0.f, 0.f, 0.f}, cos2{0.f, 0.f, 0.f, 0.f},
	    sine3{0.f, 0.f, 0.f, 0.f}, cos3{0.f, 0.f, 0.f, 0.f},
	    sine4{0.f, 0.f, 0.f, 0.f}, cos4{0.f, 0.f, 0.f, 0.f};

	mipp::Reg<float> dmSine2{0.f, 0.f, 0.f, 0.f}, dmSine3{0.f, 0.f, 0.f, 0.f},
        dmSine4{0.f,0.f, 0.f, 0.f};
    mipp::Reg<float> dmCos2{0.f, 0.f, 0.f, 0.f}, dmCos3{0.f, 0.f, 0.f, 0.f}, dmCos4{0.f, 0.f, 0.f, 0.f};
	// redundant??
	mipp::Reg<float> sample2L, sample2R, sample3L, sample3R, sample4L, sample4R;

	mipp::Reg<float> sampleL{0.f, 0.f, 0.f, 0.f}, sampleR{0.f, 0.f, 0.f, 0.f};

	float demodMix{0.f}, demodVol{0.f};
	float antipop{0.f};
	mipp::Reg<float> oneFloat{ 1.f, 1.f, 1.f, 1.f };
	mipp::Reg<float> a, b, c, d;
	float bits3[4][2] = { {1, 0}, {1, 0}, {0, 1}, {0, 1} }; // epi2, epi1
	float bits4[4][3] = { {1, 0, 0}, {0, 1, 0}, {1, 0, 0}, {0, 0, 1} }; // epi3, epi2, epi1
	float mb[4][4] = { // mixBits --> sine4, 3, 2, and mix reciprocal
		{1, 0, 0, 1}, {1, 1, 0, 0.5f}, {1, 0, 1, 0.5f}, {1, 1, 1, 1.f / 3.f}
	};

	friend class APSynth;
	juce::MPENote curNote;

	std::random_device rd;
	std::mt19937 gen{rd()};
	std::uniform_real_distribution<> dist{-1.f, 1.f};
	const float maxFreq{20000.f};
};

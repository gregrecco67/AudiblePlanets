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


#include <juce_audio_basics/juce_audio_basics.h>
#include <gin_dsp/gin_dsp.h>
#include <gin_plugin/gin_plugin.h>
#include "QuadOsc.h"
#include "Envelope.h"
#include "libMTSClient.h"
#include <numbers>
#include <random>
class APAudioProcessor;

using namespace std::numbers;
//==============================================================================
class SynthVoice : public gin::SynthesiserVoice,
                   public gin::ModVoice
{
public:
	inline mipp::Reg<float> minimaxSin(mipp::Reg<float> x1);
	inline mipp::Reg<float> mmAtan(mipp::Reg<float> x1);
	inline mipp::Reg<float> fastAtan2(mipp::Reg<float> x, mipp::Reg<float> y);
	SynthVoice(APAudioProcessor& p);
    
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
	float getMSEG1Phase();
	float getMSEG2Phase();
	float getMSEG3Phase();
	float getMSEG4Phase();
  
private:
    void updateParams(int blockSize);

    APAudioProcessor& proc;

    QuadOscillator osc1, osc2, osc3, osc4;

    gin::Filter filter;
    gin::LFO lfo1, lfo2, lfo3, lfo4;
	gin::MSEG mseg1, mseg2, mseg3, mseg4;
	gin::MSEG::Parameters mseg1Params, mseg2Params, mseg3Params, mseg4Params;

    Envelope env1, env2, env3, env4;
    std::array<Envelope*, 4> envs{&env1, &env2, &env3, &env4};
    
	StereoPosition epi1{ 1.0f, 0.0f, 1.0f, 0.0f};
	StereoPosition epi2{ 1.0, 0.0f, 1.0f, 0.0f };
	StereoPosition epi3{ 1.0f, 0.0f, 1.0f, 0.0f };
	StereoPosition epi4{ 1.0f, 0.0f, 1.0f, 0.0f };

	StereoPosition osc1Positions[32];
	StereoPosition osc2Positions[32];
	StereoPosition osc3Positions[32];
	StereoPosition osc4Positions[32];

    float currentMidiNote = -1;
    QuadOscillator::Params osc1Params, osc2Params, osc3Params, osc4Params;
	float osc1Freq = 0.0f, osc2Freq = 0.0f, osc3Freq = 0.0f, osc4Freq = 0.0f;
	float osc1Vol = 0.0f, osc2Vol = 0.0f, osc3Vol = 0.0f, osc4Vol = 0.0f;
	int algo{ 0 };
	float equant{ 0.f }, demodVol{ 2.0f };
    
	static constexpr float baseAmplitude = 0.12f; 

    gin::EasedValueSmoother<float> noteSmoother;
    
	juce::AudioBuffer<float> synthBuffer;

	int n = mipp::N<float>() * 8;
	float epi2xls[32], epi2yls[32], epi2xrs[32], epi2yrs[32],
        epi3xls[32], epi3yls[32], epi3xrs[32], epi3yrs[32],
        epi4xls[32], epi4yls[32], epi4xrs[32], epi4yrs[32];
    
	// constants for minimaxSin
	const mipp::Reg<float> s1 = 0.99999999997884898600402426033768998f;
	const mipp::Reg<float> s2 = -0.166666666088260696413164261885310067f;
	const mipp::Reg<float> s3 = 0.00833333072055773645376566203656709979f;
	const mipp::Reg<float> s4 = -0.000198408328232619552901560108010257242f;
	const mipp::Reg<float> s5 = 2.75239710746326498401791551303359689e-6f;
	const mipp::Reg<float> s6 = -2.3868346521031027639830001794722295e-8f;

	const mipp::Reg<float> t1 = -.011719135f; // constants for mmAtan 
	const mipp::Reg<float> t2 = .052647351f;
	const mipp::Reg<float> t3 = -.11642648f;
	const mipp::Reg<float> t4 = .19354038f;
	const mipp::Reg<float> t5 = -.33262283f;
	const mipp::Reg<float> t6 = .99997722f;

	mipp::Reg<float> epi2xL;
	mipp::Reg<float> epi2yL;
	mipp::Reg<float> epi2xR;
	mipp::Reg<float> epi2yR;
	mipp::Reg<float> epi3xL;
	mipp::Reg<float> epi3yL;
	mipp::Reg<float> epi3xR;
	mipp::Reg<float> epi3yR;
	mipp::Reg<float> epi4xL;
	mipp::Reg<float> epi4yL;
	mipp::Reg<float> epi4xR;
	mipp::Reg<float> epi4yR;

	mipp::Reg<float> atanAngle2L;
	mipp::Reg<float> atanAngle2R;
	mipp::Reg<float> atanAngle3L;
	mipp::Reg<float> atanAngle3R;
	mipp::Reg<float> atanAngle4L;
	mipp::Reg<float> atanAngle4R;

	mipp::Reg<float> sine2L{ 0.f, 0.f, 0.f, 0.f }, sine2R{ 0.f, 0.f, 0.f, 0.f }, sine3L{ 0.f, 0.f, 0.f, 0.f }, sine3R{ 0.f, 0.f, 0.f, 0.f },
		sine4L{ 0.f, 0.f, 0.f, 0.f }, sine4R{ 0.f, 0.f, 0.f, 0.f };
	mipp::Reg<float> square2L{ 0.f, 0.f, 0.f, 0.f }, square2R{ 0.f, 0.f, 0.f, 0.f }, square3L{ 0.f, 0.f, 0.f, 0.f }, square3R{ 0.f, 0.f, 0.f, 0.f },
		square4L{ 0.f, 0.f, 0.f, 0.f }, square4R{ 0.f, 0.f, 0.f, 0.f };
	mipp::Reg<float> saw2L{ 0.f, 0.f, 0.f, 0.f }, saw2R{ 0.f, 0.f, 0.f, 0.f }, saw3L{ 0.f, 0.f, 0.f, 0.f }, saw3R{ 0.f, 0.f, 0.f, 0.f },
		saw4L{ 0.f, 0.f, 0.f, 0.f }, saw4R{ 0.f, 0.f, 0.f, 0.f };

	mipp::Reg<float> sample2L, sample2R, sample3L, sample3R, sample4L, sample4R;

	mipp::Reg<float> modSample2L, demodSample2L, modSample2R, demodSample2R;
	mipp::Reg<float> modSample3L, demodSample3L, modSample3R, demodSample3R;
	mipp::Reg<float> modSample4L, demodSample4L, modSample4R, demodSample4R;

	mipp::Reg<float> sampleL{ 0.f, 0.f, 0.f, 0.f }, sampleR{ 0.f, 0.f, 0.f, 0.f };
	mipp::Reg<float> piReg = pi;

	float maxPos = 0.0f;
	float minPos = 0.0f;

    friend class APSynth;
    juce::MPENote curNote;

	std::random_device rd;
	std::mt19937 gen{ rd() };
	std::uniform_real_distribution<> dist{ -1.f, 1.f };
	
};

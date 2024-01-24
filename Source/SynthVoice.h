#pragma once

#include <JuceHeader.h>
#include "FastMath.hpp"
#define SEMITONE 1.05946309436f
#define SEMITONE_INV 0.94387431268f
#include <numbers>

using namespace std::numbers;

class APAudioProcessor;

//==============================================================================
class SynthVoice : public gin::SynthesiserVoice,
                   public gin::ModVoice
{
public:
    SynthVoice (APAudioProcessor& p);
    
    void noteStarted() override;
    void noteRetriggered() override;
    void noteStopped (bool allowTailOff) override;

    void notePressureChanged() override;
    void noteTimbreChanged() override;
    void notePitchbendChanged() override    {}
    void noteKeyStateChanged() override     {}
    
    void setCurrentSampleRate (double newRate) override;

    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

    bool isVoiceActive() override;

    float getFilterCutoffNormalized();
  
private:
    void updateParams (int blockSize);

	double sampleRate = 44100.0;

    APAudioProcessor& proc;


    gin::Filter filter;
    
    gin::LFO lfo1, lfo2, lfo3, lfo4;
	gin::AnalogADSR env1, env2, env3, env4;
	std::array<int, 4> oscEnvs{0, 1, 2, 3}; // since these can be applied to any osc
	std::array<float, 4> envVals = { 0.0f, 0.0f, 0.0f, 0.0f };

	struct StereoPosition { float xL, yL, xR, yR; };
	StereoPosition epi1{ 1.0f, 0.0f, 1.0f, 0.0f};
	StereoPosition epi2{ 1.0, 0.0f, 1.0f, 0.0f };
	StereoPosition epi3{ 1.0f, 0.0f, 1.0f, 0.0f };
	StereoPosition epi4{ 1.0f, 0.0f, 1.0f, 0.0f };

    float currentMidiNote = -1;
   
	//APBLLTVoicedStereoOscillator osc1, osc2, osc3, osc4;
	//APVoicedStereoOscillatorParams osc1Params, osc2Params, osc3Params, osc4Params;
    
	class Oscillator
	{
	public:
		Oscillator() {}

		float notes[4], freqs[4], phases[4]{ -1.0 }, deltas[4], sampleRate;
		struct Params {
			float note, tones, detune, spread, pan, radius, leftGains[4], rightGains[4]; // radius == volume in old scheme
			bool saw;
		};
		Params params;
		void setParams(Params p) {
            //  note, tones, detune, spread, pan, radius; // radius == volume in old scheme
            // saw;
            params = p;
            
            float baseNote  = params.note - params.detune / 2;
            float noteDelta = params.detune / 3;

            float basePan = params.pan - params.spread;
            float panDelta = (params.spread * 2) / 3;

            for (int i = 0; i < 4; i++)
            {
                float pan = juce::jlimit (-1.0f, 1.0f, basePan + panDelta * i);

                params.leftGains[i]  = params.radius * (1.0f - pan) / 2;
                params.rightGains[i] = params.radius * (1.0f + pan) / 2;

                notes[i] = baseNote + noteDelta * i;
                freqs[i] = std::min(sampleRate / 2.0, 440.0 * std::pow (2.0, (notes[i] - 69.0) / 12.0));
                deltas[i] =  1.0f / (float ((1.0f / freqs[i]) * sampleRate)); // phases are 0-1
            }
        }
		void noteOn() {
            for (int i = 0; i < 4; i++)
            {
                phases[i] = (phases[i] >= 0) ? phases[i] : 0.0f;
            }
        }
		void reset() {
            for (int i = 0; i < 4; i++)
            {
                phases[i] = -1;
            }
        }
		struct Samples { float sinL, cosL, sinR, cosR; };
		Samples getNextSamples() {
            Samples output{0, 0, 0, 0};
            for (int i = 0; i < 4; i++) {
                phases[i] += deltas[i];
                auto sine = FastMath<float>::sineValueForPhaseAndTones(phases[i] * 2 * pi, params.tones);
                auto cosine = FastMath<float>::sineValueForPhaseAndTones((phases[i] + 0.25f) * 2 * pi, params.tones);
                output.sinL += sine * params.leftGains[i];
                output.cosL += cosine * params.leftGains[i];
                output.sinR += sine * params.rightGains[i];
                output.cosR += cosine * params.rightGains[i];
            }
            return output;
        }
	};

	Oscillator osc1, osc2, osc3, osc4;

	juce::AudioBuffer<float> synthBuffer;
	float baseAmplitude = 0.25f;

    gin::EasedValueSmoother<float> noteSmoother;
    
    float ampKeyTrack = 1.0f;
};

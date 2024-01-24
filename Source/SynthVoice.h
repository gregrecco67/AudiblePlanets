#pragma once

#include <JuceHeader.h>
#include "FastMath.hpp"
#define SEMITONE 1.05946309436f
#define SEMITONE_INV 0.94387431268f

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
			float note, tones, detune, spread, pan, radius; // radius == volume in old scheme
			bool saw;
		};
		Params params;
		void setParams(Params p);
		void noteOn();
		void reset();
		struct Samples { float sinL, cosL, sinR, cosR; };
		Samples getNextSamples();
	};

	Oscillator osc1, osc2, osc3, osc4;

	juce::AudioBuffer<float> synthBuffer;
	float baseAmplitude = 0.25f;

    gin::EasedValueSmoother<float> noteSmoother;
    
    float ampKeyTrack = 1.0f;
};

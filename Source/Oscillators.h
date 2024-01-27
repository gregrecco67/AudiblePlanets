#pragma once
#include <JuceHeader.h>
#include "FastMath.hpp"
#define SEMITONE 1.05946309436f
#define SEMITONE_INV 0.94387431268f

enum class Wave
{
	silence = 0,
	sine = 1,
	cosine = 2,
	sawUp = 3,
	sawDown = 4,
	pulse = 5,
	square = 6,
	whiteNoise = 7,
	pinkNoise = 8,
};

//==============================================================================
/** Stereo oscillator. L & R can be at different pitches
*/
class APStereoOscillator
{
public:
    APStereoOscillator () {}

    struct Params
    {
        Wave wave = Wave::sine;
        float leftGain = 1.0;
        float rightGain = 1.0;
		float tones = 1.0;
    };

    void setSampleRate (double sr)  { sampleRate = sr; }
    void noteOn (float phase = -1);

    void process (float freq, const Params& params, juce::AudioSampleBuffer& mainPhaseBuffer, juce::AudioSampleBuffer& quarterPhaseBuffer)
    {
        mainPhaseBuffer.clear();
		quarterPhaseBuffer.clear();
		internalParams = params;
        processAdding (freq, params, mainPhaseBuffer, quarterPhaseBuffer);
    }

	float sineValueForPhaseAndTones(float phase, float tones) {
		float fullTones{ 0.f }, value{ 0.0f };
		float partialToneFraction = std::modf(tones, &fullTones);

		value += FastMath<float>::minimaxSin(phase);

		if (tones > 1.0f && tones < 2.0f)
			value += FastMath<float>::minimaxSin(phase * 2.0f) * partialToneFraction * 0.5f;
		else if (tones > 1.0f)
			value += FastMath<float>::minimaxSin(phase * 2.0f) * 0.5f; // we're over 2, so add the max level of this partial

		if (tones > 2.0f && tones < 3.0f)
			value += FastMath<float>::minimaxSin(phase * 3.0f) * partialToneFraction * 0.33f;
		else if (tones > 2.0f)
			value += FastMath<float>::minimaxSin(phase * 3.0f) * 0.33f;

		if (tones > 3.0f && tones < 4.0f)
			value += FastMath<float>::minimaxSin(phase * 4.0f) * partialToneFraction * 0.25f;
		else if (tones > 3.0f)
			value += FastMath<float>::minimaxSin(phase * 4.0f) * 0.25f;

		if (tones > 4.0f && tones < 5.0f)
			value += FastMath<float>::minimaxSin(phase * 5.0f) * partialToneFraction * 0.2f;
		else if (tones > 4.0f)
			value += FastMath<float>::minimaxSin(phase * 5.0f) * 0.2f;

		if (tones > 5.0f)
			value += FastMath<float>::minimaxSin(phase * 6.0f) * partialToneFraction * 0.16f;

		return value;
	}

    void processAdding (float freq, const Params& params, juce::AudioSampleBuffer& mainPhaseBuffer, juce::AudioSampleBuffer& quarterPhaseBuffer)
    {
		internalParams = params;
        freq = float (std::min((float)sampleRate / 2.0f, freq));
        float delta = 1.0f / (float ((1.0f / freq) * sampleRate));

        int samps = mainPhaseBuffer.getNumSamples();
		jassert(samps == quarterPhaseBuffer.getNumSamples());
        auto mainl = mainPhaseBuffer.getWritePointer (0);
		auto mainr = mainPhaseBuffer.getWritePointer(1);
		auto quarterl = quarterPhaseBuffer.getWritePointer(0);
		auto quarterr = quarterPhaseBuffer.getWritePointer(1);

		if (params.wave == Wave::sine  || params.wave == Wave::cosine)
		{
			for (int i = 0; i < samps; i++)
			{
				auto s1 = sineValueForPhaseAndTones(phase*juce::MathConstants<float>::twoPi, params.tones);
				*mainl++ += s1 * params.leftGain;
				*mainr++ += s1 * params.rightGain;

				auto s2 = sineValueForPhaseAndTones(phase * juce::MathConstants<float>::twoPi + juce::MathConstants<float>::halfPi, params.tones);
				*quarterl++ += s2 * params.leftGain;
				*quarterr++ += s2 * params.rightGain;

				phase += delta;
				while (phase >= 1.0f)
					phase -= 1.0f;
			}
		}
		if (params.wave == Wave::sawUp)
		{
			for (int i = 0; i < samps; i++)
			{
				auto s1 = phase * 2.0f - 1.0f;
				*mainl++ += s1 * params.leftGain;
				*mainr++ += s1 * params.rightGain;

				auto s2 = (phase + 0.25f) * 2.0f - 1.0f;
				*quarterl++ += s2 * params.leftGain;
				*quarterr++ += s2 * params.rightGain;

				phase += delta;
				while (phase >= 1.0f)
					phase -= 1.0f;
			}
		}
	}

private:
    double sampleRate = 44100.0;
    float phase = 0.0f;
	Params internalParams;
};

struct APVoicedOscillatorParams
{
    int voices = 1;
    float pan = 0.0f;
    float spread = 0.0f;
    float detune = 0.0f;
    float gain = 1.0f;
	float tones = 1.0f;
};

//==============================================================================
/** Stereo Oscillator with multiples voices, pan, spread, detune, etc
*/
template<typename O, typename P>
class APVoicedStereoOscillator
{
public:
    APVoicedStereoOscillator() = default;

    void setSampleRate (double sr)
    {
        for (auto o : oscillators)
            o->setSampleRate (sr);
    }

    void noteOn (float phase = -1)
    {
        for (auto o : oscillators)
            o->noteOn (phase);
    }

    void noteOn (float phases[])
    {
        for (auto idx = 0; auto o : oscillators)
            o->noteOn (phases[idx++]);
    }

    void process (float freq, const P& params, juce::AudioSampleBuffer& mainPhaseBuffer, juce::AudioSampleBuffer& quarterPhaseBuffer)
	{
		mainPhaseBuffer.clear();
		quarterPhaseBuffer.clear();
		processAdding(freq, params, mainPhaseBuffer, quarterPhaseBuffer);
    }

    void processAdding (float freq, const P& params, juce::AudioSampleBuffer& mainPhaseBuffer, juce::AudioSampleBuffer& quarterPhaseBuffer)
    {
        typename O::Params p;
        params.init (p);

        if (params.voices == 1)
        {
            p.leftGain  = params.gain * (1.0f - params.pan);
            p.rightGain = params.gain * (1.0f + params.pan);

            oscillators[0]->processAdding (freq, p, mainPhaseBuffer, quarterPhaseBuffer);
        }
        else
        {
			float baseFreq = freq * std::pow(SEMITONE, -params.detune);
            float freqFactor = std::pow(SEMITONE, params.detune / (params.voices - 1)); // !!

            float basePan = params.pan - params.spread;
            float panDelta = (params.spread * 2) / (params.voices - 1);

            for (int i = 0; i < params.voices; i++)
            {
                float pan = juce::jlimit (-1.0f, 1.0f, basePan + panDelta * i);

                p.leftGain  = params.gain * (1.0f - pan) / float (std::sqrt (params.voices));
                p.rightGain = params.gain * (1.0f + pan) / float (std::sqrt (params.voices));

                oscillators[i]->processAdding (baseFreq * (float)std::pow(freqFactor, i), p, mainPhaseBuffer, quarterPhaseBuffer);
            }
        }
    }

protected:
    juce::OwnedArray<O> oscillators;
};

struct APVoicedStereoOscillatorParams : public APVoicedOscillatorParams
{
    Wave wave = Wave::sine;

    inline void init (APStereoOscillator::Params& p) const
    {
        p.wave = wave;
		p.tones = tones;
    }
};

//==============================================================================
/** Stereo Oscillator with multiple voices, pan, spread, detune, etc
 */
class APBLLTVoicedStereoOscillator : public APVoicedStereoOscillator<APStereoOscillator, APVoicedStereoOscillatorParams>
{
public:
    APBLLTVoicedStereoOscillator ()
    {
        for (int i = 0; i < 4; i++)
            oscillators.add (new APStereoOscillator());
    }
};

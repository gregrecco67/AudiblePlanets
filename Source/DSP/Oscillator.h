#pragma once
#include <gin_dsp/gin_dsp.h>
#include <gin_plugin/gin_plugin.h>

struct Position;
class APOscillator  // : public gin::StereoOscillator
{
public:
	APOscillator(gin::BandLimitedLookupTables &bllt_);
	~APOscillator() = default;

	float qrtPhase(const float phase_)
	{
		float p2 = phase_ + 0.25f;
		if (p2 >= 1.0f) {
			p2 -= 1.0f;
		}
		return p2;
	}

	void noteOn(float p = -1)
	{
		if (p >= 0.0f)
			phase = p;
		else
			phase = 0.0f;
	}

	void setSampleRate(double sr)
	{
		sampleRate = sr;
		invSampleRate = 1.0f / sampleRate;
	}

	void bumpPhase(float bump)
	{
		phase += bump;
		if (phase >= 1.0f) {
			phase -= std::trunc(phase);
		}
		while (phase < 0.0f) {
			phase += 1.0f;
		}
	}

	struct Settings {
		gin::Wave wave = gin::Wave::sine;
		float vol = 0.0f;
	};

	void renderFloats(float note,
	    const Settings &settings,
	    float *xs,
	    float *ys,
	    const int numSamples);

	gin::BandLimitedLookupTables &bllt;
	float sampleRate = 44100.0f;
	float invSampleRate = 1.0f / sampleRate;
	float phase = 0.0f;
};

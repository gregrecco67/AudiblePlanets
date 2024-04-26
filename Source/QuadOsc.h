#include <JuceHeader.h>
#include "FastMath.hpp"
#include <numbers>

using std::numbers::pi;
using std::numbers::inv_pi;
using reg = juce::dsp::SIMDRegister<float>;

struct Matrix {
	friend Matrix operator*(const Matrix& m, const float s) { // scalar multiplication
		return { m.a * s, m.b * s, m.c * s, m.d * s };
	}
	float a, b, c, d;
};

struct StereoMatrix {
	Matrix left, right;
	friend StereoMatrix operator*(const StereoMatrix& m, const float s) { // scalar multiplication
			return { m.left * s, m.right * s };
		}
};

struct StereoPosition {
	float xL{ 0.f }, yL{ 0.f }, xR{ 0.f }, yR{ 0.f };
	
	friend StereoPosition operator*(const StereoPosition& p, const StereoMatrix& m) { // apply matrix to position
		return {.xL = m.left.a * p.xL + m.left.b * p.yL,
				.yL = m.left.c * p.xL + m.left.d * p.yL,
				.xR = m.right.a * p.xR + m.right.b * p.yR,
				.yR = m.right.c * p.xR + m.right.d * p.yR };
	}

	friend StereoPosition operator*(const StereoPosition& p, const float s) { // scalar multiplication
			return { p.xL * s, p.yL * s, p.xR * s, p.yR * s };
		}

	StereoPosition operator+(const StereoPosition otherPos) {
		return { this->xL + otherPos.xL, this->yL + otherPos.yL,
			this->xR + otherPos.xR, this->yR + otherPos.yR };
	}
};


enum class Wavetype
{
	sine = 0,
	sawUp = 1,
};

// utility for detune calculations
static inline float semitonePower(float x) { // about 2x faster than std::pow(SEMITONE, x) and accurate to ~ e-06
	float u = 4.6385981e-07f;
	u = u * x + -3.2122109e-05f;
	u = u * x + 0.0016682396f;
	u = u * x + -0.057762265f;
	return u * x + 1.f;
}



static inline float sineValueForPhaseAndTones(float phase_, float tones) {
	float fullTones{ 0.f }, value{ 0.0f };
	float partialToneFraction = std::clamp(std::modf(tones, &fullTones), 0.0f, 1.0f);

	value += FastMath<float>::minimaxSin(phase_);

	if (tones > 1.0f && tones < 2.0f)
		value += FastMath<float>::minimaxSin(phase_ * 2.0f) * partialToneFraction * 0.5f;
	else if (tones > 1.0f)
		value += FastMath<float>::minimaxSin(phase_ * 2.0f) * 0.5f; // we're over 2, so add the max level of this partial

	if (tones > 2.0f && tones < 3.0f)
		value += FastMath<float>::minimaxSin(phase_ * 3.0f) * partialToneFraction * 0.33f;
	else if (tones > 2.0f)
		value += FastMath<float>::minimaxSin(phase_ * 3.0f) * 0.33f;

	if (tones > 3.0f && tones < 4.0f)
		value += FastMath<float>::minimaxSin(phase_ * 4.0f) * partialToneFraction * 0.25f;
	else if (tones > 3.0f)
		value += FastMath<float>::minimaxSin(phase_ * 4.0f) * 0.25f;

	if (tones > 4.0f && tones < 5.0f)
		value += FastMath<float>::minimaxSin(phase_ * 5.0f) * partialToneFraction * 0.2f;
	else if (tones > 4.0f)
		value += FastMath<float>::minimaxSin(phase_ * 5.0f) * 0.2f;

	if (tones > 5.0f)
		value += FastMath<float>::minimaxSin(phase_ * 6.0f) * partialToneFraction * 0.16f;

	return value;
}

class QuadOscillator
{
public:
	QuadOscillator() = default;
	~QuadOscillator() = default;

	const reg regpi = reg(pi);
	const reg regtwopi = reg(2.f * pi);

	reg normalizePhases(reg input) {
		for (int i = 0; i < 4; i++) {
			while (input[i] >= pi) {
				input[i] = input[i] - pi * 2.f;
			}
            while (input[i] < -pi) {
                input[i] = input[i] + pi * 2.f;
            }
		}
		return input;
	}

	const reg a1 = reg(0.99999999997884898600402426033768998f);
	const reg a2 = reg(-0.166666666088260696413164261885310067f);
	const reg a3 = reg(0.00833333072055773645376566203656709979f);
	const reg a4 = reg(-0.000198408328232619552901560108010257242f);
	const reg a5 = reg(2.75239710746326498401791551303359689e-6f);
	const reg a6 = reg(-2.3868346521031027639830001794722295e-8f);

	reg sinesForPhases(reg x1) {
		x1 = normalizePhases(x1);
		reg x2 = x1 * x1;
		return x1 * reg::multiplyAdd(a1, x2, reg::multiplyAdd(a2, x2, reg::multiplyAdd(a3, x2, reg::multiplyAdd(a4, x2, reg::multiplyAdd(a5, x2, a6)))));
	}

	reg sinesForPhasesAndTones(reg p, float t) {
		float fullTones{ 0.f }; float partialToneFraction = std::modf(t, &fullTones);
		reg values = sinesForPhases(p);

        if (t > 1.0f && t < 2.0f)
            values += sinesForPhases(p * 2.0f) * partialToneFraction * 0.5f;
        else if (t > 1.0f)
            values += sinesForPhases(p * 2.0f) * 0.5f; // we're over 2, so add the max level of this partial

        if (t > 2.0f && t < 3.0f)
            values += sinesForPhases(p * 3.0f) * partialToneFraction * 0.33f;
        else if (t > 2.0f)
            values += sinesForPhases(p * 3.0f) * 0.33f;

        if (t > 3.0f && t < 4.0f)
            values += sinesForPhases(p * 4.0f) * partialToneFraction * 0.25f;
        else if (t > 3.0f)
            values += sinesForPhases(p * 4.0f) * 0.25f;

        if (t > 4.0f && t < 5.0f)
            values += sinesForPhases(p * 5.0f) * partialToneFraction * 0.2f;
        else if (t > 4.0f)
            values += sinesForPhases(p * 5.0f) * 0.2f;

        if (t > 5.0f)
            values += sinesForPhases(p * 6.0f) * partialToneFraction * 0.16f;
        
        return values;
	}
    
	juce::dsp::SIMDRegister<float> freqs, phases, phaseIncs, gainsL, gainsR;
	float freq, pan, tones, sampleRate;

	void setSampleRate(double sampleRate_)
	{
		sampleRate = (float)sampleRate_;
		recalculate();
	}

	struct Params {
		int voices = 4;
        Wavetype wave = Wavetype::sine;
		float tones{ 1.0 }, pan{ 0.f }, spread{ 0.f }, detune{ 0.f }, phaseShift{ 0.f };
	};

	Params params;

	void setParams(Params params_)
	{
		params = params_;
		recalculate();
	}

	void recalculate()
	{
		// calculate frequencies and pan positions for our four voices
		float baseFreq = freq * semitonePower(-params.detune); // faster std::pow(SEMITONE, params.detune);
		float freqFactor = semitonePower(params.detune / 3.f);
		float basePan = params.pan - params.spread;
		float panDelta = 2.f * params.spread / 3.f;
		
		for (int i = 0; i < 4; i++)
		{
			float thisPan = juce::jlimit(-1.0f, 1.0f, basePan + panDelta * i);
			gainsL[i] = (1.0f - thisPan) / 2;
			gainsR[i] = (1.0f + thisPan) / 2;
            switch(i) {
                case 0:
                    freqs[i] = baseFreq;
                    break;
                case 1:
                    freqs[i] = baseFreq * freqFactor;
                    break;
                case 2:
                    freqs[i] = baseFreq * freqFactor * freqFactor;
                    break;
                case 3:
                    freqs[i] = baseFreq * freqFactor * freqFactor * freqFactor;
                    break;
            }
            phaseIncs[i] = freqs[i] * (float)pi * 2.0f / sampleRate;
		}
	}

	void renderPositions(const float freq_, const Params params_, StereoPosition positions[], const int numSamples) {
		freq = freq_;
		params = params_;
		recalculate();
		for (int i = 0; i < numSamples; i++) {
			if (params.wave == Wavetype::sine) {
				auto xs = sinesForPhasesAndTones(phases + (params.phaseShift + 0.5f) * (float)pi, params.tones);
				auto ys = sinesForPhasesAndTones(phases + params.phaseShift * (float)pi, params.tones);
				positions[i].xL = (xs * gainsL).sum() * 0.25f;
				positions[i].yL = (ys * gainsL).sum() * 0.25f;
				positions[i].xR = (xs * gainsR).sum() * 0.25f;
				positions[i].yR = (ys * gainsR).sum() * 0.25f;
			}
			if (params.wave == Wavetype::sawUp) {
				auto quarterPhases = phases + 0.25f * (float)pi;
				quarterPhases = normalizePhases(quarterPhases);
				auto xs = quarterPhases * reg(inv_pi);
				auto ys = phases * reg(inv_pi);
				positions[i].xL = (xs * gainsL).sum() * 0.25f;
				positions[i].yL = (ys * gainsL).sum() * 0.25f;
				positions[i].xR = (xs * gainsR).sum() * 0.25f;
				positions[i].yR = (ys * gainsR).sum() * 0.25f;
			}
			phases += phaseIncs;
			phases = normalizePhases(phases);
		}
	}

	void noteOn(float initPhase = 0.f)
	{
		for (int i = 0; i < 4; i++) {
			phases[i] = initPhase;
		}
	}

};

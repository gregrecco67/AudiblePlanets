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

#define _USE_MATH_DEFINES
#include <juce_core/juce_core.h>
#include <cmath>
#include <numbers>

using std::numbers::pi;

class LFO {
public:
	struct LFOParams {
		float frequency{0.2f};
		float sampleRate{44100.f};
	};

	struct LFOValuesByPhase {
		float deg0Value{0.f};
		float deg120Value{0.f};
		float deg240Value{0.f};
	};

	LFO() {
		for (int i = 0; i < 1024; ++i) {
			sineTable[i] = std::sin(i * (2.0 * pi / 1024));
		}
	}

	~LFO() = default;

	// params
	inline void setFrequency(float freq)
	{
		frequency = freq;
		phaseIncrement = frequency / sampleRate;
	}
	inline void setSampleRate(float newRate)
	{
		sampleRate = (newRate > 0) ? newRate : sampleRate;
		phaseIncrement = frequency / sampleRate;
	}

	static inline double lerp(double a, double b, double t)
	{
		return a + (b - a) * t;
	}

	LFOValuesByPhase getNextValues()
	{
		phase += phaseIncrement;
		phase = phase - std::floor(phase);
		auto phase120 = phase + 1.0 / 3.0;
		phase120 = phase120 - std::floor(phase120);
		auto phase240 = phase + 2.0 / 3.0;
		phase240 = phase240 - std::floor(phase240);

		const auto index1 = static_cast<int>(phase * 1024);
		const auto index2 = static_cast<int>(phase120 * 1024);
		const auto index3 = static_cast<int>(phase240 * 1024);
		const double frac1 = phase * 1024 - index1;
		const double frac2 = phase120 * 1024 - index2;
		const double frac3 = phase240 * 1024 - index3;
		values.deg0Value =   static_cast<float>(lerp(sineTable[std::clamp(index1, 0, 1023)], sineTable[std::clamp(index1+1, 0, 1023)], frac1));
		values.deg120Value = static_cast<float>(lerp(sineTable[std::clamp(index2, 0, 1023)], sineTable[std::clamp(index2+1, 0, 1023)], frac2));
		values.deg240Value = static_cast<float>(lerp(sineTable[std::clamp(index3, 0, 1023)], sineTable[std::clamp(index3+1, 0, 1023)], frac3));
		return values;
	}

	inline void initialize()
	{
		phaseIncrement = frequency / sampleRate;
		phase = 0.f;
	}

private:
	LFOValuesByPhase values;
	double frequency{1.0}, sampleRate{44100.0};
	double sineTable[1024];
	double phaseIncrement{0.0};
	double phase{0.0};
};

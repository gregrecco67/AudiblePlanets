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

#include <juce_core/juce_core.h>

class Envelope {
public:
	//==============================================================================
	explicit Envelope(const std::array<double, 1024>& convex_) : convex(convex_)  {
		recalculateRates();
	}

	~Envelope() = default;

	const std::array<double, 1024>& convex;

	//==============================================================================
	enum class State {
		idle,
		attack,
		decay,
		sustain,
		release,
		ADRattack,
		ADRdecay,
		ADRrelease,
		ADRSyncIdle
	};

	struct EnvelopeState {
		State state;
		float phase;
	};

	struct Params {
		Params() = default;

		Params(double attackTimeMs_,
		       double decayTimeMs_,
		       double sustainLevel_,
		       double releaseTimeMs_,
		       double aCurve_,  // 0 = linear, 1 = convex, -1 = concave
		       double dRcurve_,
		       bool repeat_,
		       bool sync_ = false,
		       float syncduration_ = 1.f)
		    : attackTimeMs(attackTimeMs_), decayTimeMs(decayTimeMs_),
		      sustainLevel(sustainLevel_), releaseTimeMs(releaseTimeMs_),
		      aCurve(aCurve_), dRCurve(dRcurve_), repeat(repeat_), sync(sync_),
		      syncduration(syncduration_)
		{
		}

		double attackTimeMs{1.0}, decayTimeMs{1.0}, sustainLevel{0.5},
		    releaseTimeMs{1.0}, aCurve{1.0}, dRCurve{-1.0};
		bool repeat{false}, sync{false};
		float syncduration{1.0};
	};

	[[nodiscard]] inline EnvelopeState getState() const noexcept
	{
		return {state, static_cast<float>(finalOut)};
	}

	inline void setParameters(const Params &newParameters)
	{
		parameters = newParameters;
		recalculateRates();
	}

	[[nodiscard]] inline bool isActive() const noexcept { return state != State::idle; }
	[[nodiscard]] inline float getValue() const { return  static_cast<float>(finalOut); }
	[[nodiscard]] inline float getOutput() const { return static_cast<float>(finalOut); }
	
	inline void setSampleRate(double newSampleRate) noexcept
	{
		if (newSampleRate > 0.0) { sampleRate = newSampleRate; }
		recalculateRates();
	}

	void reset() noexcept { state = State::idle; }

	void noteOn() noexcept;
	void noteOff() noexcept;

	[[nodiscard]] double getValForIdx(double idx, const bool isAttack) const;
	[[nodiscard]] double getIdxForVal(const double val) const;

	float getNextSample() noexcept;
	void advance(const int frames) { for (int i = 0; i < frames; i++) { getNextSample(); } }

private:
	//==============================================================================
	void recalculateRates() noexcept;
	void goToNextState() noexcept;

	//==============================================================================

	State state = State::idle;
	Params parameters;

	double sampleRate = 44100.0;
	double inverseSampleRate = 1.0 / sampleRate;
	double linearIdxVal{0.0}, attackRate{0.0}, decayRate{0.0},
	    releaseRate{0.0}, finalOut{0.0}, releaseStart{0.0};

	float timeSinceStart{0.f}, duration{1.f};
};

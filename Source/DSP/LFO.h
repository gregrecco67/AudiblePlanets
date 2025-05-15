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
#include <math.h>
#include <cmath>

class LFO {
public:
	enum WaveShapes { Sine = 0, Triangle, SawUp, SawDown, Square, Random };
	struct LFOParams {
		float frequency{0.2f};
		int waveShape{0};
		float sampleRate{44100.f};
	};

	struct LFOValuesByPhase {
		float mainPhaseValue{0.f};
		float quarterPhaseValue{0.f};
		float minusQuarterPhaseValue{0.f};
		float halfPhaseValue{0.f};
	};

	struct LFOPhases {
		float mainPhase{0.f};
		float quarterPhase{juce::MathConstants<float>::halfPi};
		float minusQuarterPhase{-juce::MathConstants<float>::halfPi};
		float halfPhase{juce::MathConstants<float>::pi};
	};

	LFO() = default;
	~LFO() = default;

	// params
	void setDepth(float newDepth) { depth = newDepth * 0.01f; }  // 0-100
	void setBipolar(bool input) { isBipolar = input; }
	void setFrequency(float freq)
	{
		frequency = freq;
		phaseIncrement =
		    frequency * juce::MathConstants<float>::twoPi / sampleRate;
	}
	void setWaveShape(int shape) { waveShape = shape; }
	void setSampleRate(float newRate)
	{
		sampleRate = newRate;
		phaseIncrement =
		    frequency * juce::MathConstants<float>::twoPi / sampleRate;
	}

	void setDuration(float duration)
	{
		frequency = 1.0f / duration;
		phaseIncrement =
		    frequency * juce::MathConstants<float>::twoPi / sampleRate;
	}

	// calculations

	float getTriangleValueForPhase(float phase)
	{
		if (phase < 0.f)
			return (phase * 2 / juce::MathConstants<float>::pi) + 1.0f;
		else if (phase > 0.f)
			return 1.f - (phase * 2 / juce::MathConstants<float>::pi);
		else
			return 1.f;
	}

	float minimaxSin(float x1)
	{
		// x1 += M_PI;
		while (x1 > M_PI) {
			x1 -= 2.0f * (float)M_PI;
		}
		while (x1 < -M_PI) {
			x1 += 2.0f * (float)M_PI;
		}
		float x2 = x1 * x1;
		return x1 *
		       (0.99999999997884898600402426033768998f +
		        x2 *
		            (-0.166666666088260696413164261885310067f +
		             x2 *
		                 (0.00833333072055773645376566203656709979f +
		                  x2 *
		                      (-0.000198408328232619552901560108010257242f +
		                       x2 *
		                           (float)(2.75239710746326498401791551303359689e-6 -
		                                   2.3868346521031027639830001794722295e-8 *
		                                       x2)))));
	}

	LFOValuesByPhase calculateValuesForPhases()
	{
		// check whether any phase needs to be wrapped, and if so mark it as
		// flipped and wrap it. If needed, we'll use the info to trigger a new
		// random value.
		while (phases.mainPhase > juce::MathConstants<float>::pi) {
			phases.mainPhase -= juce::MathConstants<float>::twoPi;
			mainPhaseHasFlipped = true;
		}
		while (phases.quarterPhase > juce::MathConstants<float>::pi) {
			phases.quarterPhase -= juce::MathConstants<float>::twoPi;
			quarterPhaseHasFlipped = true;
		}
		while (phases.minusQuarterPhase > juce::MathConstants<float>::pi) {
			phases.minusQuarterPhase -= juce::MathConstants<float>::twoPi;
			minusQuarterPhaseHasFlipped = true;
		}
		while (phases.halfPhase > juce::MathConstants<float>::pi) {
			phases.halfPhase -= juce::MathConstants<float>::twoPi;
			halfPhaseHasFlipped = true;
		}

		// "Sine", "Triangle", "SawUp", "SawDown", "Square", "Random"
		switch (waveShape) {
			case 0:  // sine
				values.mainPhaseValue = minimaxSin(phases.mainPhase);
				values.quarterPhaseValue = minimaxSin(phases.quarterPhase);
				values.minusQuarterPhaseValue =
				    minimaxSin(phases.minusQuarterPhase);
				values.halfPhaseValue = minimaxSin(phases.halfPhase);
				break;
			case 1:  // triangle
				values.mainPhaseValue =
				    getTriangleValueForPhase(phases.mainPhase);
				values.quarterPhaseValue =
				    getTriangleValueForPhase(phases.quarterPhase);
				values.minusQuarterPhaseValue =
				    getTriangleValueForPhase(phases.minusQuarterPhase);
				values.halfPhaseValue =
				    getTriangleValueForPhase(phases.halfPhase);
				break;
			case 2:  // saw up
				values.mainPhaseValue =
				    phases.mainPhase / juce::MathConstants<float>::pi;
				values.quarterPhaseValue =
				    phases.quarterPhase / juce::MathConstants<float>::pi;
				values.minusQuarterPhaseValue =
				    phases.minusQuarterPhase / juce::MathConstants<float>::pi;
				values.halfPhaseValue =
				    phases.halfPhase / juce::MathConstants<float>::pi;
				break;
			case 3:  // saw down
				values.mainPhaseValue =
				    -phases.mainPhase / juce::MathConstants<float>::pi;
				values.quarterPhaseValue =
				    -phases.quarterPhase / juce::MathConstants<float>::pi;
				values.minusQuarterPhaseValue =
				    -phases.minusQuarterPhase / juce::MathConstants<float>::pi;
				values.halfPhaseValue =
				    -phases.halfPhase / juce::MathConstants<float>::pi;
				break;
			case 4:  // square
				values.mainPhaseValue = phases.mainPhase < 0 ? -1.0f : 1.0f;
				values.quarterPhaseValue =
				    phases.quarterPhase < 0 ? -1.0f : 1.0f;
				values.minusQuarterPhaseValue =
				    phases.minusQuarterPhase < 0 ? -1.0f : 1.0f;
				values.halfPhaseValue = phases.halfPhase < 0 ? -1.0f : 1.0f;
				break;
			case 5:  // random
				if (mainPhaseHasFlipped) {
					currentRandomMain = myRand.nextFloat() * 2.0f -
					                    1.0f;  // save for next X samples
					values.mainPhaseValue = currentRandomMain;
					mainPhaseHasFlipped = false;
				} else {
					values.mainPhaseValue = currentRandomMain;
				}
				if (quarterPhaseHasFlipped) {
					currentRandomQuarter = myRand.nextFloat() * 2.0f - 1.0f;
					values.quarterPhaseValue = currentRandomQuarter;
					quarterPhaseHasFlipped = false;
				} else {
					values.quarterPhaseValue = currentRandomQuarter;
				}
				if (minusQuarterPhaseHasFlipped) {
					currentRandomMinusQuarter =
					    myRand.nextFloat() * 2.0f - 1.0f;
					values.minusQuarterPhaseValue = currentRandomMinusQuarter;
					minusQuarterPhaseHasFlipped = false;
				} else {
					values.minusQuarterPhaseValue = currentRandomMinusQuarter;
				}
				if (halfPhaseHasFlipped) {
					currentRandomHalf = myRand.nextFloat() * 2.0f - 1.0f;
					values.halfPhaseValue = currentRandomHalf;
					halfPhaseHasFlipped = false;
				} else {
					values.halfPhaseValue = currentRandomHalf;
				}
				break;
		}

		return values;
	}

	LFOValuesByPhase getCurrentValues()
	{
		if (isBipolar) {
			values.mainPhaseValue = depth * values.mainPhaseValue;
			values.quarterPhaseValue = depth * values.quarterPhaseValue;
			values.minusQuarterPhaseValue =
			    depth * values.minusQuarterPhaseValue;
			values.halfPhaseValue = depth * values.halfPhaseValue;
		} else {
			values.mainPhaseValue =
			    depth * (values.mainPhaseValue + 1.0f) / 2.0f;
			values.quarterPhaseValue =
			    depth * (values.quarterPhaseValue + 1.0f) / 2.0f;
			values.minusQuarterPhaseValue =
			    depth * (values.minusQuarterPhaseValue + 1.0f) / 2.0f;
			values.halfPhaseValue =
			    depth * (values.halfPhaseValue + 1.0f) / 2.0f;
		}
		return values;
	}

	LFOValuesByPhase getNextValues()
	{
		phases.mainPhase += phaseIncrement;
		phases.quarterPhase += phaseIncrement;
		phases.minusQuarterPhase += phaseIncrement;
		phases.halfPhase += phaseIncrement;
		calculateValuesForPhases();
		return getCurrentValues();
	}

	LFOValuesByPhase advanceLFOAndReturnValues(int numSamples)
	{
		for (int i = 0; i < numSamples; i++) {
			phases.mainPhase += phaseIncrement;
			phases.quarterPhase += phaseIncrement;
			phases.minusQuarterPhase += phaseIncrement;
			phases.halfPhase += phaseIncrement;
		}
		calculateValuesForPhases();
		return getCurrentValues();
	}

	void initialize()
	{
		phaseIncrement =
		    frequency * juce::MathConstants<float>::twoPi / sampleRate;
		phases.mainPhase = 0.f;
		phases.quarterPhase = juce::MathConstants<float>::pi * 0.5f;
		phases.halfPhase = juce::MathConstants<float>::pi;
		phases.minusQuarterPhase = juce::MathConstants<float>::pi * 1.5f;

		mainPhaseHasFlipped = false;
		quarterPhaseHasFlipped = false;
		minusQuarterPhaseHasFlipped = false;
		halfPhaseHasFlipped = false;
	}

private:
	LFOValuesByPhase values;
	float frequency{1.f}, sampleRate{44100.f};
	int waveShape{0};
	// bool tsync{ false };
	// float tsyncDuration{ 1.f };

	float phaseIncrement{0.f};
	float currentRandomMain{0.f}, currentRandomQuarter{0.f},
	    currentRandomHalf{0.f}, currentRandomMinusQuarter{0.f};
	LFOPhases phases;

	bool mainPhaseHasFlipped{false}, quarterPhaseHasFlipped{false},
	    minusQuarterPhaseHasFlipped{false}, halfPhaseHasFlipped{false};
	bool isBipolar{true};
	juce::Random myRand{1333734};
	float depth{0.5f};
};

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

// LICENSE for FastMath class and its members fastSin, and fastTanh:
//------------------------------------------------------------------------------
/*
MIT License

Copyright (c) 2023 Mike Jarmy

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#define _USE_MATH_DEFINES
#include <juce_dsp/juce_dsp.h>
#include <cmath>
#include <numbers>

using std::numbers::pi;

//------------------------------------------------------------------------------
// FastMath contains some fast approximations for trigonometric functions.
//------------------------------------------------------------------------------

template<class F>
class FastMath {
public:
	// Sine approximation. Range is [-pi, pi].
	//
	// https://web.archive.org/web/20100613230051/http://www.devmaster.net/forums/showthread.php?t=5784
	// https://www.desmos.com/calculator/f0eryaepsl
	static inline F fastSin(F x)
	{
		static constexpr F B = 4 / juce::MathConstants<float>::pi;
		static constexpr F C = -4 / (juce::MathConstants<float>::pi *
		                             juce::MathConstants<float>::pi);
		static constexpr F P = 0.225f;

		F y = B * x + C * x * std::abs(x);

		// Extra precision.
		y = P * (y * std::abs(y) - y) + y;

		return y;
	}

    static inline float minimaxSin(float x1) {
		x1 = normalizePhase(x1);
        const float x2 = x1 * x1;

        return x1 * (0.99999999997884898600402426033768998f
        + x2 * (-0.166666666088260696413164261885310067f
        + x2 * (0.00833333072055773645376566203656709979f
        + x2 * (-0.000198408328232619552901560108010257242f
        + x2 * (2.75239710746326498401791551303359689e-6f
        - 2.3868346521031027639830001794722295e-8f * x2)))));
    }

	static inline float normalizePhase(float x1)
	{  // set anything to [-pi, pi]
		while (x1 > juce::MathConstants<float>::pi) {
			x1 -= 2.0f * juce::MathConstants<float>::pi;
		}
		while (x1 < -juce::MathConstants<float>::pi) {
			x1 += 2.0f * juce::MathConstants<float>::pi;
		}
		return x1;
	}

	using SIMD = juce::dsp::SIMDRegister<float>;

	static inline juce::dsp::SIMDRegister<float> simdSin(juce::dsp::SIMDRegister<float> x1)
	{
		const juce::dsp::SIMDRegister<float> x2 = x1 * x1;

		return  x1 * (SIMD(0.99999999997884898600402426033768998f) +
		    	x2 * (SIMD(-0.166666666088260696413164261885310067f) +
		        x2 * (SIMD(0.00833333072055773645376566203656709979f) +
				x2 * (SIMD(-0.000198408328232619552901560108010257242f) +
		        x2 * (SIMD(2.75239710746326498401791551303359689e-6f) -
				SIMD(2.3868346521031027639830001794722295e-8f) *
				x2)))));
	}

	FastMath() = default;
	~FastMath() = default;

	FastMath(const FastMath &) = delete;
	FastMath &operator=(const FastMath &) = delete;
};

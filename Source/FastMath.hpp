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
 
 
// LICENSE for FastMath class, fastSin, and fastTanh:
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
#include <cmath>
#define M_PI 3.14159265358979323846

//------------------------------------------------------------------------------
// FastMath contains some fast approximations for trigonometric functions.
//------------------------------------------------------------------------------

template <class F> class FastMath {

  public:

    // Sine approximation. Range is [-pi, pi].
    //
    // https://web.archive.org/web/20100613230051/http://www.devmaster.net/forums/showthread.php?t=5784
    // https://www.desmos.com/calculator/f0eryaepsl
    static inline F fastSin(F x) {
        
        static constexpr F B = 4 / (float)M_PI;
        static constexpr F C = -4 / (float)(M_PI * M_PI);
        static constexpr F P = 0.225f;

        F y = B * x + C * x * std::abs(x);

        // Extra precision.
        y = P * (y * std::abs(y) - y) + y;

        return y;
    }

    // Hyperbolic tangent approximation.
    //
    // https://www.kvraudio.com/forum/viewtopic.php?p=5447225#p5447225
    // https://www.desmos.com/calculator/bjc7zsl4ek
    static inline F fastTanh(const F x) {
        const F ax = std::abs(x);
        const F x2 = x * x;
        const F z =
            x * (0.773062670268356 + ax +
                 (0.757118539838817 + 0.0139332362248817 * x2 * x2) * x2 * ax);

        return z / (0.795956503022967 + std::abs(z));
    }

    static inline float normalizePhase(float x1) { // set anything to [-pi, pi]
        while (x1 > M_PI) {
            x1 -= 2.0f * (float)M_PI;
        }
        while (x1 < -M_PI) {
            x1 += 2.0f * (float)M_PI;
        }
        return x1;
    }

    static inline float minimaxSin(float x1) {
		x1 = normalizePhase(x1);
        float x2 = x1 * x1;

        return x1 * (0.99999999997884898600402426033768998f
        + x2 * (-0.166666666088260696413164261885310067f
        + x2 * (0.00833333072055773645376566203656709979f
        + x2 * (-0.000198408328232619552901560108010257242f
        + x2 * (2.75239710746326498401791551303359689e-6f
        - 2.3868346521031027639830001794722295e-8f * x2)))));
    }

    static inline juce::dsp::SIMDRegister<float> simdSin(juce::dsp::SIMDRegister<float> x1) {
		juce::dsp::SIMDRegister<float> x2 = x1 * x1;

		return x1 * (juce::dsp::SIMDRegister < float>(0.99999999997884898600402426033768998f)
		+ x2 * (juce::dsp::SIMDRegister<float>(-0.166666666088260696413164261885310067f)
		+ x2 * (juce::dsp::SIMDRegister<float>(0.00833333072055773645376566203656709979f)
		+ x2 * (juce::dsp::SIMDRegister<float>(-0.000198408328232619552901560108010257242f)
		+ x2 * (juce::dsp::SIMDRegister<float>(2.75239710746326498401791551303359689e-6f)
		- juce::dsp::SIMDRegister<float>(2.3868346521031027639830001794722295e-8f) * x2)))));
	}

	static inline float minimaxAtan(float a) {
		float b = a * a;
		float u = -0.011719135406045413f;
		u = u * b + 0.052647350616021903f;
		u = u * b + -0.11642648118471723f;
		u = u * b + 0.19354037577295979f;
		u = u * b + -0.33262282784074959f;
		u = u * b + 0.99997721907991632f;
		return u * a;
	}

	static inline float fastAtan2(float x, float y) {
		if (x != 0.0f)
		{
			if (fabsf(x) > fabsf(y)) // all the branches allow us to avoid calling minimaxAtan() outside [-1,1]
				// technique described here: https://mazzo.li/posts/vectorized-atan2.html
			{
				const float z = y / x;
				if (x > 0.0)
				{
					// atan2(y,x) = atan(y/x) if x > 0
					return minimaxAtan(z);
				}
				else if (y >= 0.0)
				{
					// atan2(y,x) = atan(y/x) + PI if x < 0, y >= 0
					return minimaxAtan(z) + (float)M_PI;
				}
				else
				{
					// atan2(y,x) = atan(y/x) - PI if x < 0, y < 0
					return minimaxAtan(z) - (float)M_PI;
				}
			}
			else // Use property atan(y/x) = PI/2 - atan(x/y) if |y/x| > 1.
			{
				const float z = x / y;
				if (y > 0.0)
				{
					// atan2(y,x) = PI/2 - atan(x/y) if |y/x| > 1, y > 0
					return -minimaxAtan(z) + (float)M_PI * 0.5f;
				}
				else
				{
					// atan2(y,x) = -PI/2 - atan(x/y) if |y/x| > 1, y < 0
					return -minimaxAtan(z) - (float)M_PI * 0.5f;
				}
			}
		}
		else
		{
			if (y > 0.0f) // x = 0, y > 0
			{
				return (float)M_PI * 0.5f;
			}
			else if (y < 0.0f) // x = 0, y < 0
			{
				return (float)-M_PI * 0.5f;
			}
		}
		return 0.0f;

	}
    
    
inline static float sineValueForPhaseAndTones(float phase, float tones) {
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



  private:

    FastMath() {}
    ~FastMath() {}

    FastMath(const FastMath&) = delete;
    FastMath& operator=(const FastMath&) = delete;
};

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

#define _USE_MATH_DEFINES
#include <cmath>
#include <memory>
#include <array>
#include <juce_dsp/juce_dsp.h>
#include <gin_dsp/gin_dsp.h>
#include <juce_core/juce_core.h>
#include "LFO.h"
#include "FastMath.hpp"

#pragma once

class ChorusProcessor 
{
public:
    ChorusProcessor() {}		
    ~ChorusProcessor() {}

public:
    void prepare(juce::dsp::ProcessSpec spec) {
        currentSampleRate = (float)spec.sampleRate;
		centerDelayBuffer.setSize(1, 1.0, currentSampleRate);
		leftDelayBuffer.setSize(1, 1.0, currentSampleRate);
		rightDelayBuffer.setSize(1, 1.0, currentSampleRate);
		lfo.setSampleRate(currentSampleRate);
		lfo.setFrequency(15.0f);
		lfo.setWaveShape(LFO::WaveShapes::Sine);
		lfo.initialize();
    }

    void process(juce::dsp::ProcessContextReplacing<float> context) {
        //
        auto& inBlock = context.getOutputBlock();

		auto numSamples = inBlock.getNumSamples();
		auto samplesL = inBlock.getChannelPointer(0);
		auto samplesR = inBlock.getChannelPointer(1);
		
		lfo.setFrequency(lfoRate);
		for (int i = 0; i < static_cast<int>(numSamples); i++)
		{
			auto lfoValues = lfo.getNextValues();
			auto leftDelayTime_ms = std::clamp((lfoValues.mainPhaseValue * 10.0f) + delayTime, 5.f, 30.f);
			auto centerDelayTime_ms = std::clamp((lfoValues.quarterPhaseValue * 10.0f) + delayTime, 5.f, 30.f);
			auto rightDelayTime_ms = std::clamp((lfoValues.halfPhaseValue * 10.0f) + delayTime, 5.f, 30.f);
			
			auto leftIn = samplesL[i];
			auto rightIn = samplesR[i];
			auto leftChorusOut = leftDelayBuffer.readLagrange(0, leftDelayTime_ms / 1000.0f);
			auto centerChorusOut = centerDelayBuffer.readLagrange(0, centerDelayTime_ms / 1000.0f);
			auto rightChorusOut = rightDelayBuffer.readLagrange(0, rightDelayTime_ms / 1000.0f);
			
			leftDelayBuffer.write(0, leftIn + leftChorusOut * feedback);
			centerDelayBuffer.write(0, rightIn * 0.5f + leftIn * 0.5f + centerChorusOut * feedback);
			rightDelayBuffer.write(0, rightIn + rightChorusOut * feedback);
			leftDelayBuffer.writeFinished();
			centerDelayBuffer.writeFinished();
			rightDelayBuffer.writeFinished();
			
			samplesL[i] = (leftChorusOut + centerChorusOut) * wet + dry * leftIn;
			samplesR[i] = (centerChorusOut + rightChorusOut) * wet + dry * rightIn;
		}
    }

    void setRate(float rate) {
        lfoRate = rate;
    }

    void setDepth(float _depth) {
		depth = _depth;
	}

    void setFeedback(float _feedback) {
        feedback = _feedback;
    }

    void setDry(float _dry) {
		dry = _dry;
	}

    void setWet(float _wet) {
		wet = _wet;
	}

	void setCentreDelay(float _delayTime) {
		delayTime = _delayTime;
	}

private:
    float lfoRate{ 0.05f }, depth{ 0.5f }, feedback{ 0.0f }, dry{ 0.5f }, wet{ 0.5f }, delayTime{ 15.f };
    LFO lfo;			///< the modulator
    float currentSampleRate = 44100.f;	///< current sample rate
	gin::DelayLine centerDelayBuffer{ 1 }, leftDelayBuffer{ 1 }, rightDelayBuffer{ 1 };
};

class StereoDelayProcessor
{
public:
    StereoDelayProcessor() = default;
    ~StereoDelayProcessor() = default;

    void prepare(juce::dsp::ProcessSpec spec)
    {
        auto sampleRate = spec.sampleRate;
        
		delayTimeL.reset(sampleRate, .015f);
		delayTimeR.reset(sampleRate, .015f);
		cutoff.reset(sampleRate, .025f);
		delayBuffer_L.setSize(1, 65.0, sampleRate);
		delayBuffer_R.setSize(1, 65.0, sampleRate);
		LPFilter.prepare(spec);
		cutoff.setCurrentAndTargetValue(2000.f);
		LPFilter.setCutoffFrequency(cutoff.getNextValue());
		LPFilter.setResonance(0.707f);
		LPFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    }

    void process(juce::dsp::ProcessContextReplacing<float> context) {
        auto& inBlock = context.getOutputBlock();

		auto numSamples = static_cast<int>(inBlock.getNumSamples());
		auto* leftSamples = inBlock.getChannelPointer(0);
		auto* rightSamples = inBlock.getChannelPointer(1);
		float freezeFactor = freeze ? 0.f : 0.5f;
		cutoff.skip(std::min(numSamples - 1, 0));
		LPFilter.setCutoffFrequency(cutoff.getNextValue());
		delayFB = freeze ? 1.0f : delayFB;
		if (ping) {
			for (int i = 0; i < numSamples; i++) {
				auto dTimeL = std::min(delayTimeL.getNextValue(), 64.0f);
				auto dTimeR = std::min(delayTimeR.getNextValue(), 64.0f);

				float delayedSample_L = delayBuffer_L.readLagrange(0, dTimeL);
				float delayedSample_R = delayBuffer_R.readLagrange(0, dTimeR);
				float inDelay_L = leftSamples[i] * freezeFactor + delayedSample_R * delayFB;
				float inDelay_R = rightSamples[i] * freezeFactor + delayedSample_L * delayFB;

				leftSamples[i] = delayedSample_L * delayWet + leftSamples[i] * delayDry;
				rightSamples[i] = delayedSample_R * delayWet + rightSamples[i] * delayDry;
				delayBuffer_L.write(0, LPFilter.processSample(0, inDelay_L));
				delayBuffer_R.write(0, LPFilter.processSample(1, inDelay_R));
				delayBuffer_L.writeFinished();
				delayBuffer_R.writeFinished();
			}
		}
		else {
			for (int i = 0; i < numSamples; i++) {
				auto dTimeL = std::min(delayTimeL.getNextValue(), 64.0f);
				auto dTimeR = std::min(delayTimeR.getNextValue(), 64.0f);

				float delayedSample_L = delayBuffer_L.readLagrange(0, dTimeL);
				float delayedSample_R = delayBuffer_R.readLagrange(0, dTimeR);
				float inDelay_L = leftSamples[i] * freezeFactor + delayedSample_L * delayFB;
				float inDelay_R = rightSamples[i] * freezeFactor + delayedSample_R * delayFB;

				leftSamples[i] = delayedSample_L * delayWet + leftSamples[i] * delayDry;
				rightSamples[i] = delayedSample_R * delayWet + rightSamples[i] * delayDry;
				delayBuffer_L.write(0, LPFilter.processSample(0, inDelay_L));
				delayBuffer_R.write(0, LPFilter.processSample(1, inDelay_R));
				delayBuffer_L.writeFinished();
				delayBuffer_R.writeFinished();
			}
		}

    }
    void setDry(float dry) {
        delayDry = dry;
    }

    void setWet(float wet) {
        delayWet = wet;
    }

    void setFB(float fb) {
        delayFB = fb;
    }

    void setTimeL(float time) {
        delayTimeL.setTargetValue(time);
    }

    void setTimeR(float time) {
        delayTimeR.setTargetValue(time);
    }

    void setFreeze(bool _freeze) {
        freeze = _freeze;
    }

    void setPing(bool _ping) {
		ping = _ping;
	}

	void setCutoff(float _cutoff) {
		cutoff.setTargetValue(_cutoff);
	}

	void resetBuffers() {
		delayBuffer_L.clear();
		delayBuffer_R.clear();
	}

private:
    juce::AudioBuffer<float> inBuffer;
    float delayDry{ 1.0f }, delayWet{ 0.5f }, delayFB{ 0.5f };
    juce::LinearSmoothedValue<float> delayTimeL{ .40f }, delayTimeR{ .40f }, cutoff{ 2000.f };
	gin::DelayLine delayBuffer_L{ 1 }, delayBuffer_R{ 1 };
    bool freeze{ false }, ping{ true };
	juce::dsp::StateVariableTPTFilter<float> LPFilter;
};

/// PlateReverb license info:

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

//------------------------------------------------------------------------------
// PlateReverb is an implementation of the classic plate reverb algorithm
// described by Jon Dattorro.
//
// Dattorro, J. 1997. "Effect Design Part 1: Reverberators and Other Filters."
// Journal of the Audio Engineering Society, Vol. 45, No. 9
//
// https://ccrma.stanford.edu/~dattorro/EffectDesignPart1.pdf
//
// Parameters:
//
//    mix:        Dry/wet mix.
//    predelay:   Delay before reverb.
//    lowpass:    Apply a lowpass filter before reverb.
//    decay:      How quickly the reverb decays.
//    size:       The size of our imaginary plate.
//    damping:    How much high frequencies are filtered during reverb.
//
//------------------------------------------------------------------------------



template <class F, class I> class PlateReverb 
{
public:

    static constexpr F kMaxPredelay = 0.1f; // seconds
    static constexpr F kMaxSize = 3.0f;

    PlateReverb() {}
    ~PlateReverb() {}

    // Set the sample rate.  Note that we are re-mallocing all of the various
    // delay lines here.
    void setSampleRate(F sampleRate_) {
        sampleRate = sampleRate_;

        // Ratio of our sample rate to the sample rate that is used in
        // Dattorro's paper.
        F r = sampleRate / 29761.0f;

        // Predelay
        predelayLine.reset(new DelayLine((uint32_t)std::ceil(sampleRate * kMaxPredelay)));

        // Lowpass filters
        lowpass.setSampleRate(sampleRate);
        leftTank.damping.setSampleRate(sampleRate);
        rightTank.damping.setSampleRate(sampleRate);

        // Diffusers
        diffusers[0].reset(new DelayAllpass((uint32_t)std::ceil(142 * r), 0.75));
        diffusers[1].reset(new DelayAllpass((uint32_t)std::ceil(107 * r), 0.75));
        diffusers[2].reset(new DelayAllpass((uint32_t)std::ceil(379 * r), 0.625));
        diffusers[3].reset(new DelayAllpass((uint32_t)std::ceil(277 * r), 0.625));

        // Tanks
        F maxModDepth = 8.0f * kMaxSize * r;
        leftTank.resetDelayLines(
            (uint32_t)std::ceil(kMaxSize * 672 * r), -0.7f, // apf1
            maxModDepth,
            (uint32_t)std::ceil(kMaxSize * 4453 * r),      // del1
            (uint32_t)std::ceil(kMaxSize * 1800 * r), 0.5, // apf2
            (uint32_t)std::ceil(kMaxSize * 3720 * r)       // del2
        );
        rightTank.resetDelayLines(
            (uint32_t)std::ceil(kMaxSize * 908 * r), -0.7f, // apf1
            maxModDepth,
            (uint32_t)std::ceil(kMaxSize * 4217 * r),      // del1
            (uint32_t)std::ceil(kMaxSize * 2656 * r), 0.5f, // apf2
            (uint32_t)std::ceil(kMaxSize * 3163 * r)       // del2
        );

        leftTank.lfo.setSampleRate(sampleRate);
        rightTank.lfo.setSampleRate(sampleRate);
        leftTank.lfo.setFrequency(1.0);
        rightTank.lfo.setFrequency(0.95f);

        // Tap points
        baseLeftTaps = {
            266 * r,  // rightTank.del1
            2974 * r, // rightTank.del1
            1913 * r, // rightTank.apf2
            1996 * r, // rightTank.del2
            1990 * r, // leftTank.del1
            187 * r,  // leftTank.apf2
            1066 * r, // leftTank.del2
        };
        baseRightTaps = {
            353 * r,  // leftTank.del1
            3627 * r, // leftTank.del1
            1228 * r, // leftTank.apf2
            2673 * r, // leftTank.del2
            2111 * r, // rightTank.del1
            335 * r,  // rightTank.apf2
            121 * r,  // rightTank.del2
        };
    }

    // Dry/wet mix.
    void setDry(F d /* [0, 1] */) { dry = clamp(d, 0.0, 1.0); }
    void setWet(F w /* [0, 1] */) { wet = clamp(w, 0.0, 1.0); }

    // Delay before reverb.
    void setPredelay(F pd /* in seconds, [0, 0.1] */) {
        predelay = clamp(pd, 0.0, kMaxPredelay) * sampleRate;
    }

    // Apply a lowpass filter before reverb.
    void setLowpass(F cutoff /* Hz */) {
        cutoff = clamp(cutoff, 16.0, 20000.0);
        lowpass.setCutoff(cutoff);
    }

    // How quickly the reverb decays.
    void setDecay(F dr /* [0, 1) */) {
        decayRate = clamp(dr, 0.0f, 0.9999999f);
        leftTank.setDecay(decayRate);
        rightTank.setDecay(decayRate);
    }

    // The size of our imaginary plate.
    //
    // The size parameter scales the delay time for all of the delay lines and
    // APFs in each tank, and for all of the tap points.
    //
    // Note that there is no size parameter in Dattorro's paper; it is an
    // extension to the original algorithm.
    void setSize(F sz /* [0, 2] */) {

        F sizeRatio = clamp(sz, 0.0, kMaxSize) / kMaxSize;

        // Scale the tank delays and APFs in each tank
        leftTank.setSizeRatio(sizeRatio);
        rightTank.setSizeRatio(sizeRatio);

        // Scale the taps
        for (I i = 0; i < kNumTaps; i++) {
            leftTaps[i] = baseLeftTaps[i] * sizeRatio;
            rightTaps[i] = baseRightTaps[i] * sizeRatio;
        }
    }

    // How much high frequencies are filtered during reverb.
    void setDamping(F cutoff /* Hz */) {
        cutoff = clamp(cutoff, 16.0, 20000.0);

        leftTank.damping.setCutoff(cutoff);
        rightTank.damping.setCutoff(cutoff);
    }

    void prepare(juce::dsp::ProcessSpec spec) {
        sampleRate = (float)spec.sampleRate;
		setSampleRate(sampleRate);
		setPredelay(predelay);
    }

    void process(juce::dsp::ProcessContextReplacing<float> context) {
        auto& inBlock = context.getOutputBlock();

		auto numSamples = inBlock.getNumSamples();
		auto outLeft = inBlock.getChannelPointer(0);
		auto outRight = inBlock.getChannelPointer(1);
		auto inLeft = inBlock.getChannelPointer(0);
		auto inRight = inBlock.getChannelPointer(1);
		for (int i = 0; i < static_cast<int>(numSamples); i++) {
			process(inLeft[i], inRight[i], &outLeft[i], &outRight[i]);
		}
    }

    // Process a stereo pair of samples.
    void process(F dryLeft, F dryRight, F* leftOut, F* rightOut) {

        // ---------------------

        // Note that this is "synthetic stereo".  We produce a stereo pair
        // of output samples based on the summed input.
        F sum = dryLeft + dryRight;

        // Predelay
        sum = predelayLine->tapAndPush(predelay, sum);

        // Input lowpass
        sum = lowpass.process(sum);

        // Diffusers
        sum = diffusers[0]->process(sum, diffusers[0]->getSize());
        sum = diffusers[1]->process(sum, diffusers[1]->getSize());
        sum = diffusers[2]->process(sum, diffusers[2]->getSize());
        sum = diffusers[3]->process(sum, diffusers[3]->getSize());

        // Tanks
        F leftIn = sum + rightTank.out * decayRate;
        F rightIn = sum + leftTank.out * decayRate;
        leftTank.process(leftIn);
        rightTank.process(rightIn);

        // Tap for output
        F wetLeft = rightTank.del1->tap(leftTaps[0])   //  266
            + rightTank.del1->tap(leftTaps[1]) // 2974
            - rightTank.apf2->tap(leftTaps[2]) // 1913
            + rightTank.del2->tap(leftTaps[3]) // 1996
            - leftTank.del1->tap(leftTaps[4])  // 1990
            - leftTank.apf2->tap(leftTaps[5])  //  187
            - leftTank.del2->tap(leftTaps[6]); // 1066

        F wetRight = leftTank.del1->tap(rightTaps[0])     //  353
            + leftTank.del1->tap(rightTaps[1])   // 3627
            - leftTank.apf2->tap(rightTaps[2])   // 1228
            + leftTank.del2->tap(rightTaps[3])   // 2673
            - rightTank.del1->tap(rightTaps[4])  // 2111
            - rightTank.apf2->tap(rightTaps[5])  //  335
            - rightTank.del2->tap(rightTaps[6]); //  121

        // Mix
        *leftOut = dryLeft * dry + wetLeft * wet;
        *rightOut = dryRight * dry + wetRight * wet;
    }

private:

    //--------------------------------------------------------------
    // OnePoleFilter
    //--------------------------------------------------------------

    class OnePoleFilter {

    public:

        OnePoleFilter() {}
        ~OnePoleFilter() {}

        void setSampleRate(F sampleRate_) {
            sampleRate = sampleRate_;
            recalc();
        }

        void setCutoff(F cutoff_ /* Hz */) {
            cutoff = cutoff_;
            recalc();
        }

        F process(F x) {
            z = x * a + z * b;
            return z;
        }

    private:

        F sampleRate = 1;
        F cutoff = 0;

        F a = 0;
        F b = 0;
        F z = 0;

        void recalc() {
            b = std::exp(-2 * juce::MathConstants<float>::pi * cutoff / sampleRate);
            a = 1 - b;
        }
    };

    //--------------------------------------------------------------
    // DelayLine
    //--------------------------------------------------------------

    class DelayLine {

    public:

        DelayLine(I size_) : size(size_) {

            // For speed, create a bigger buffer than we really need.
            I bufferSize = ceilPowerOfTwo(size);
            buffer.reset(new F[bufferSize]);
            std::memset(&buffer[0], 0, bufferSize * sizeof(F));

            mask = bufferSize - 1;

            writeIdx = 0;
        }

        ~DelayLine() {}

        inline void push(F val) {
            buffer[writeIdx++] = val;
            writeIdx &= mask;
        }

        inline F tap(F delay /* samples */) {
            // We always want to be able to properly handle any delay value that
            // gets passed in here, without going past the original size.
            jassert(delay <= size);

            I d = (uint32_t)delay;
            F frac = 1 - (delay - d);

            I readIdx = (writeIdx - 1) - d;
            F a = buffer[(readIdx - 1) & mask];
            F b = buffer[readIdx & mask];

            return a + (b - a) * frac;
        }

        // This does read-before-write.
        inline F tapAndPush(F delay, F val) {
            F out = tap(delay);
            push(val);
            return out;
        }

        inline I getSize() { return size; }

    private:

        const I size;

        std::unique_ptr<F[]> buffer;
        I mask;

        I writeIdx;

        static I ceilPowerOfTwo(I n) {
            return (I)std::pow(2, std::ceil(std::log(n) / std::log(2)));
        }
    };

    //------------------------------------------
    // DelayAllpass
    //------------------------------------------

    class DelayAllpass {

    public:

        DelayAllpass(I size_, F gain_) : delayLine(size_), gain(gain_) {}

        ~DelayAllpass() {}

        inline F process(F x, F delay) {
            F wd = delayLine.tap(delay);
            F w = x + gain * wd;
            F y = -gain * w + wd;
            delayLine.push(w);
            return y;
        }

        inline void setGain(F gain_) { gain = gain_; }

        inline F tap(F delay) { return delayLine.tap(delay); }

        inline I getSize() { return delayLine.getSize(); }

    private:

        DelayLine delayLine;
        F gain;
    };

    //--------------------------------------------------------------
    // Lfo
    //--------------------------------------------------------------

    class Lfo {

    public:

        Lfo() {}
        ~Lfo() {}

        void setSampleRate(F sampleRate_) {
            sampleRate = sampleRate_;
            recalc();
        }

        void setFrequency(F freq_) {
            freq = freq_;
            recalc();
        }

        inline F process() {
            F out = -FastMath<F>::fastSin(phase);

            phase += phaseInc;
            if (phase > juce::MathConstants<float>::pi) {
                phase = -juce::MathConstants<float>::pi;
            }

            return out;
        }

    private:

        F sampleRate = 1;
        F freq = 0;

        F phaseInc = 0;
        F phase = (float)-juce::MathConstants<float>::pi;

        void recalc() {
            phaseInc = freq / sampleRate;
            phaseInc *= 2 * juce::MathConstants<float>::pi;
        }
    };

    //------------------------------------------
    // Tank
    //------------------------------------------

    class Tank {

    public:

        Tank() {}
        ~Tank() {}

        void resetDelayLines(
            I apf1Size_, F apf1Gain_, // First APF
            F maxModDepth_,
            I delay1Size_,            // First delay
            I apf2Size_, F apf2Gain_, // Second APF
            I delay2Size_             // Second delay
        ) {
            apf1Size = apf1Size_;
            maxModDepth = maxModDepth_;
            F maxApf1Size = apf1Size + maxModDepth + 1;
            apf1.reset(new DelayAllpass((uint32_t)maxApf1Size, apf1Gain_));

            del1.reset(new DelayLine(delay1Size_));
            apf2.reset(new DelayAllpass(apf2Size_, apf2Gain_));
            del2.reset(new DelayLine(delay2Size_));

            // We've changed the various delay line sizes and associated values,
            // so update the sizeRatio values too.
            recalcSizeRatio();
        }

        void setDecay(F decayRate_) {
            decayRate = decayRate_;
            apf2->setGain(clamp(decayRate + 0.15f, 0.25f, 0.5f));
        }

        void setSizeRatio(F sizeRatio_) {
            sizeRatio = sizeRatio_;
            recalcSizeRatio();
        }

        void process(F val) {

            // APF1: "Controls density of tail."
            val = apf1->process(val, apf1Delay + lfo.process() * modDepth);
            val = del1->tapAndPush(del1Delay, val);

            val = damping.process(val);
            val *= decayRate;

            // APF2: "Decorrelates tank signals."
            val = apf2->process(val, apf2Delay);
            val = del2->tapAndPush(del2Delay, val);

            out = val;
        }

        F out = 0.0;

        std::unique_ptr<DelayAllpass> apf1 = nullptr;
        std::unique_ptr<DelayAllpass> apf2 = nullptr;
        std::unique_ptr<DelayLine> del1 = nullptr;
        std::unique_ptr<DelayLine> del2 = nullptr;
        OnePoleFilter damping;
        Lfo lfo;

    private:

        I apf1Size = 0;
        F maxModDepth = 0;
        F modDepth = 0;

        F apf1Delay = 0;
        F apf2Delay = 0;
        F del1Delay = 0;
        F del2Delay = 0;

        F decayRate = 0;
        F sizeRatio = 0;

        void recalcSizeRatio() {

            apf1Delay = apf1Size * sizeRatio;
            modDepth = maxModDepth * sizeRatio;

            apf2Delay = apf2->getSize() * sizeRatio;
            del1Delay = del1->getSize() * sizeRatio;
            del2Delay = del2->getSize() * sizeRatio;
        }
    };

    //--------------------------------------------------------------
    //--------------------------------------------------------------
    //--------------------------------------------------------------

    F sampleRate = 1.0;

    F dry = 0.0;
    F wet = 0.0;
    F predelay = 0.0;
    F decayRate = 0.0;

    std::unique_ptr<DelayLine> predelayLine = nullptr;
    OnePoleFilter lowpass;
    std::array<std::unique_ptr<DelayAllpass>, 4> diffusers = {
        nullptr, nullptr, nullptr, nullptr };

    Tank leftTank;
    Tank rightTank;

    static const I kNumTaps = 7;
    std::array<F, kNumTaps> baseLeftTaps = {};
    std::array<F, kNumTaps> baseRightTaps = {};
    std::array<F, kNumTaps> leftTaps = {};
    std::array<F, kNumTaps> rightTaps = {};

    static inline F clamp(F val, F low, F high) {
        return std::min(std::max(val, low), high);
    }
};


class GainProcessor
{
public:
    GainProcessor() = default;
    ~GainProcessor() = default;

    void prepare(juce::dsp::ProcessSpec spec) {
		gain.prepare(spec);
		gainLevelSmoothed.reset(spec.sampleRate, 0.02f);
    }

    void process(juce::dsp::ProcessContextReplacing<float> context) {
        auto numSamples = context.getOutputBlock().getNumSamples();
        gainLevelSmoothed.skip((int)numSamples);
        gainLevelSmoothed.setTargetValue(gainLevelSmoothed.getCurrentValue());
        gain.process(context);
    }

    void setGainLevel(float gainLevel) {
		gainLevelSmoothed.setTargetValue(gainLevel);
        gain.setGainDecibels(gainLevelSmoothed.getCurrentValue());
	}

private:
    juce::dsp::Gain<float> gain;
    juce::LinearSmoothedValue<float> gainLevelSmoothed{ 0.0f };
};

class MBFilterProcessor
{
public:
    MBFilterProcessor() = default;
    ~MBFilterProcessor() = default;

    void prepare(juce::dsp::ProcessSpec spec) {
		currentSampleRate = static_cast<float>(spec.sampleRate);
		*iirLS.state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(currentSampleRate, iirLSFrequency, iirLSQ, iirLSGain);
		*iirPeak.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(currentSampleRate, iirPeakFrequency, iirPeakQ, iirPeakGain);
		*iirHS.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(currentSampleRate, iirHSFrequency, iirHSQ, iirHSGain);

		iirLS.prepare(spec);
		iirPeak.prepare(spec);
		iirHS.prepare(spec);
		iirLS.reset();
		iirPeak.reset();
		iirHS.reset();
    }

    void process(juce::dsp::ProcessContextReplacing<float> context) {
        iirLS.process(context);
        iirPeak.process(context);
        iirHS.process(context);
    }

    void setParams(float LSFreq, float LSGain, float LSQ, float PeakFreq, float PeakGain, float PeakQ, float HSFreq, float HSGain, float HSQ) {
		iirLSFrequency = LSFreq;
		iirLSGain = LSGain;
        iirLSQ = LSQ;
		iirPeakFrequency = PeakFreq;
		iirPeakGain = PeakGain;
		iirPeakQ = PeakQ;
		iirHSFrequency = HSFreq;
		iirHSGain = HSGain;
        iirHSQ = HSQ;
        *iirLS.state  = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(currentSampleRate, iirLSFrequency, iirLSQ, iirLSGain);
        *iirPeak.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(currentSampleRate, iirPeakFrequency, iirPeakQ, iirPeakGain);
        *iirHS.state  = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(currentSampleRate, iirHSFrequency, iirHSQ, iirHSGain);
	}

private:
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> iirLS;
    float iirLSFrequency{ 40.0f }, iirLSGain{1.0f}, iirLSQ{1.0f};

    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> iirHS;
    float iirHSFrequency{ 8000.0f }, iirHSGain{ 1.0f }, iirHSQ{ 1.f };

    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> iirPeak;
    float iirPeakFrequency{ 2000.0f }, iirPeakGain{ 1.0f }, iirPeakQ{ 1.0 };

    float currentSampleRate { 44100.0f };
};


// began process of adding a high boost/cut pair around the waveshaper
class WaveShaperProcessor
{
public:
    WaveShaperProcessor() {
        setFunctionToUse(0);
    }
    ~WaveShaperProcessor() = default;

    void prepare(juce::dsp::ProcessSpec spec)
    {
        sampleRate = spec.sampleRate;

        // see https://signalsmith-audio.co.uk/writing/2022/warm-distortion/
		*preBoost.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, 6500.f, 1.0f, 63.0f);
		*postCut.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, 6500.f, 1.0f, 63.0f);
		lowPassPostWet.setCutoffFrequency(2000.0f);
        
		preGain.prepare(spec);
		postGain.prepare(spec);
		preGain.setRampDurationSeconds(0.05);
		postGain.setRampDurationSeconds(0.05);
		lowPassPostWetCutoff.reset(sampleRate, 0.02f);
		lowPassPostWetCutoff.setTargetValue(2000.0f);
		preBoost.prepare(spec);
		postCut.prepare(spec);
		lowPassPostWet.prepare(spec);
		*highPassPost.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 5.0f);
		highPassPost.prepare(spec);
	}

	void process(juce::dsp::ProcessContextReplacing<float> context) {
		int numSamples = static_cast<int>(context.getOutputBlock().getNumSamples());
		
		preBoost.process(context); // high shelf
		
		lowPassPostWetCutoff.skip(numSamples);
		float cutoff = lowPassPostWetCutoff.getCurrentValue();
		lowPassPostWet.setCutoffFrequency(cutoff);
		
		auto* dataL = context.getOutputBlock().getChannelPointer(0);
		auto* dataR = context.getOutputBlock().getChannelPointer(1);
		for (int i = 0; i < numSamples; i++) {
			float left = dataL[i];
			float right = dataR[i];
			dataL[i] = lowPassPostWet.processSample(0, useFunction(preGain.processSample(left))) * wet + left * dry;
			dataR[i] = lowPassPostWet.processSample(1, useFunction(preGain.processSample(right))) * wet + right * dry;
		}
		
		postCut.process(context); // high shelf
		highPassPost.process(context); // high pass
		postGain.process(context); // gain
    }
	
	void setDry(float _dry) {
		dry = _dry;
	}
	
	void setWet(float _wet) {
		wet = _wet;
	}
	
	void setLPCutoff(float freq) {
		lowPassPostWetCutoff.setTargetValue(freq);
	}
    
	void reset() 
	{
        preGain.reset();
		postGain.reset();
	}
	
	void setHighShelfFreqAndQ(float freq, float q) {
		*preBoost.state= *juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, freq, q, 63.0f);
		//*preBoostR.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, freq, q, 63.0f);
		*postCut.state= *juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, freq, q, 0.015849f);
		//*postCutR.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, freq, q, 0.015849f);
	}

    void setFunctionToUse(int function)
    {
		currentFunction = function;
    }

	float useFunction(float x) {
		switch (currentFunction)
		{
		case 0:	// sine
			if (std::abs(x) <= 1.f)
				return std::sin(juce::MathConstants<float>::halfPi * x);
			else if (x > 1.f)
				return 1.f;
			else //if (x < -1.f)
				return -1.f;
			break;
		case 1: // atan 2
			if (std::abs(x) <= 1.f)
				return std::atan(2.f * x) / 1.10714871779409f;
			else if (x > 1.f)
				return 1.f;
			else //if (x < -1.f)
				return -1.f;
			break;
		case 2: // atan 4
			if (std::abs(x) <= 1.f)
				return std::atan(4.f * x) / 1.32581766366803f;
			else if (x > 1.f)
				return 1.f;
			else //if (x < -1.f)
				return -1.f;
			break;
		case 3: // atan 6
			if (std::abs(x) <= 1.f)
				return std::atan(6.f * x) / 1.40564764938027f;
			else if (x > 1.f)
				return 1.f;
			else //if (x < -1.f)
				return -1.f;
			break;
		case 4: // tanh 2
			if (std::abs(x) <= 1.f)
				return std::tanh(2.f * x) / 0.964027580075817f;
			else if (x > 1.f)
				return 1.f;
			else //if (x < -1.f)
				return -1.f;
			break;
		case 5: // tanh 4
			if (std::abs(x) <= 1.f)
				return std::tanh(4.f * x) / 0.999329299739067f;
			else if (x > 1.f)
				return 1.f;
			else //if (x < -1.f)
				return -1.f;
			break;
		case 6: // tanh 6
			if (std::abs(x) <= 1.f)
				return std::tanh(6.f * x) / 0.999987711650796f;
			else if (x > 1.f)
				return 1.f;
			else //if (x < -1.f)
				return -1.f;
			break;
		case 7: // cubic mid
			if (std::abs(x) <= 1.f)
				return (x + x * x * x) * 0.5f;
			else if (x > 1.f)
				return 1.f;
			else //if (x < -1.f)
				return -1.f;
			break;
		case 8: // cubic
			if (std::abs(x) <= 1.f)
				return x * x * x;
			else if (x > 1.f)
				return 1.f;
			else //if (x < -1.f)
				return -1.f;
			break;
		case 9: // cheb3
			if (std::abs(x) <= 1.f)
				return -(4.f * x * x * x - 3.f * x); // negative, so dry and wet don't interfere
			else if (x > 1.f)
				return -1.f; // flipped for continuity with above
			else //if (x < -1.f)
				return 1.f;
			break;
		case 10: // cheb5
			if (std::abs(x) <= 1.f)
				return 16.f * x * x * x * x * x - 20.f * x * x * x + 5.f * x;
			else if (x > 1.f)
				return 1.f;
			else //if (x < -1.f)
				return -1.f;
			break;
		case 11: // "halfwave"
			if (x > 0.f && x < 1.f)
				return std::tanh(1.5f * x) * 1.10479139298f; // 1.104 term is 1/tanh(1.5)
			else if (x > 1.f)
				return 1.f;
			else //if (x < -1.f)
				return 0.f;
			break;
		case 12: // "clipping"
			if (x < -0.7f)
				return -1.f;
			else if (x > 0.7f)
				return 1.f;
			else
				return x * 1.42857142857f;
			break;
		case 13: // "bitcrush"
			if (std::abs(x) < 0.01f)
				return 0.f;
			else {
				float steps = 8.f - 7.f * (drive / 60.f);
				return std::floor(steps * std::atan(2.f * x) * 0.903221025259f) / steps;
			}
			break;
		case 14: // "noise"
			if (std::abs(x) <= 1.f) {
				float p = pinkNoise.nextSample();
				float factor = 1.f - (drive / 80.f);
				if (p >= 0.f)
					return juce::jlimit(-1.f, 1.f, x * std::pow(p, factor));
				else
					return juce::jlimit(-1.f, 1.f, x * -std::pow(-p, factor));
			}
			else
                return juce::jlimit(-1.f, 1.f, x);
			break;
        case 15: // "fullwave"
            if (x < 0.f) {
                return -std::tanh(3.f*x) * 0.99505475368f; // tanh(3x)/tanh(3)
            }
            else {
                return std::tanh(3.f*x) * 0.99505475368f;
            }
            break;
        case 16: // "wavefolder"
            return 4 * (std::abs(0.5f * x + 0.25f - std::round(0.5f * x + 0.25f)) - 0.25f);
            break;
		default:
			return x;
			break;
		}
	}

    void setGain(float pre, float post)
	{
		drive = pre;
        preGain.setGainDecibels(pre);
        postGain.setGainDecibels(juce::jmin(post - (pre * 0.33f), -6.f)); // compensate for preGain but only partly
	}
	
private:
    juce::AudioBuffer<float> inBuffer;

    using Filter = juce::dsp::IIR::Filter<float>;
	Filter preBoostL, preBoostR, postCutL, postCutR, highPassPostL, highPassPostR;

	juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> preBoost, postCut, highPassPost;

	juce::dsp::StateVariableTPTFilter<float> lowPassPostWet;
	juce::SmoothedValue<float> lowPassPostWetCutoff;
	

	double sampleRate{ 44100.0 };
    int currentFunction = -1; // to trigger a change on first setFunctionToUse call
    //juce::dsp::WaveShaper<float> waveShaper;
    float dry{ 0.5f }, wet{ 0.5f };
    juce::dsp::Gain<float> preGain;
    juce::dsp::Gain<float> postGain;
    float drive{ 1.f };
	gin::PinkNoise pinkNoise;
};

class RingModulator
{
public:
	RingModulator() = default;
	~RingModulator() = default;

    struct RingModParams
    {
        float mod1freq{ 10.f }, mod2freq{ 10.f }, 
            shape1{ 0.f }, shape2{ 0.f }, 
            mix1{ 0.f }, mix2{ 0.f }, 
            spread{ 0.f }, highcut{ 20000.f }, lowcut{ 20.f };
    };

    void setParams(RingModParams& params_) {
		params = params_;
	}

    void prepare(juce::dsp::ProcessSpec spec)
    {
        sampleRate = spec.sampleRate;
        oversampledSampleRate = sampleRate * oversampleRatio;
        juce::dsp::ProcessSpec oversampledSpec{ oversampledSampleRate, static_cast<juce::uint32>(spec.maximumBlockSize * oversampleRatio),
            static_cast<juce::uint32>(2) };
        auto samplesPerBlock = spec.maximumBlockSize * oversampleRatio;
        constexpr auto filterType = juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple;
        oversampler = std::make_unique<juce::dsp::Oversampling<float>>(spec.numChannels, oversampleOrder, filterType);
        oversampler->initProcessing((size_t)samplesPerBlock);
        mod1LPCutoff.reset(sampleRate * oversampleRatio, 0.02f);
        mod2LPCutoff.reset(sampleRate * oversampleRatio, 0.02f);
        LP1.prepare(oversampledSpec);
		LP2.prepare(oversampledSpec);
		LP3.prepare(oversampledSpec);
		LP4.prepare(oversampledSpec);
		LP1.setResonance(0.707f);
		LP2.setResonance(0.707f);
		LP3.setResonance(0.707f);
		LP4.setResonance(0.707f);
		LP1.setCutoffFrequency(4000.f);
		LP2.setCutoffFrequency(4000.f);
		LP3.setCutoffFrequency(4000.f);
		LP4.setCutoffFrequency(4000.f);
        inverseOversampledSampleRate = 1.f / oversampledSampleRate;
        highCut1.prepare(oversampledSpec);
		highCut2.prepare(oversampledSpec);
		highCut3.prepare(oversampledSpec);
		highCut4.prepare(oversampledSpec);
		lowCut1.prepare(oversampledSpec);
		lowCut2.prepare(oversampledSpec);
		lowCut3.prepare(oversampledSpec);
		lowCut4.prepare(oversampledSpec);
		highCut1.setCutoffFrequency(params.highcut);
		highCut2.setCutoffFrequency(params.highcut);
		highCut3.setCutoffFrequency(params.highcut);
		highCut4.setCutoffFrequency(params.highcut);
		lowCut1.setCutoffFrequency(params.lowcut);
		lowCut2.setCutoffFrequency(params.lowcut);
		lowCut3.setCutoffFrequency(params.lowcut);
		lowCut4.setCutoffFrequency(params.lowcut);
		highCut1.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
		highCut2.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
		highCut3.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
		highCut4.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
		lowCut1.setType(juce::dsp::StateVariableTPTFilterType::highpass);
		lowCut2.setType(juce::dsp::StateVariableTPTFilterType::highpass);
		lowCut3.setType(juce::dsp::StateVariableTPTFilterType::highpass);
		lowCut4.setType(juce::dsp::StateVariableTPTFilterType::highpass);
        // do something with this, like report it to processor ---> setLatencySamples((int)oversampler->getLatencyInSamples());
    }

    void process(juce::dsp::ProcessContextReplacing<float> context) {
        auto numSamples = context.getOutputBlock().getNumSamples();
        auto oversampledBlock = oversampler->processSamplesUp(context.getOutputBlock());
        // auto oversampledContext = juce::dsp::ProcessContextReplacing<float>(oversampledBlock);
        // do processing in oversampledBuffer

        // 1. prepare derived parameters
		juce::dsp::SIMDRegister<float> mod1freqs{ params.mod1freq }, mod2freqs{ params.mod2freq }, spread{ params.spread }, unity{ 1.f };

        
        mod1freqs = mod1freqs * (unity + spread * semitones);
        mod2freqs = mod2freqs * (unity + spread * semitones);

        mod1PhaseIncs = mod1freqs * juce::dsp::SIMDRegister<float>(2.0f * juce::MathConstants<float>::pi * (float)inverseOversampledSampleRate);
        mod2PhaseIncs = mod2freqs * juce::dsp::SIMDRegister<float>(2.0f * juce::MathConstants<float>::pi * (float)inverseOversampledSampleRate);

        mod1LPCutoff.skip((int)numSamples);
        mod2LPCutoff.skip((int)numSamples);
        
        mod1LPCutoff.setTargetValue(std::clamp(params.mod1freq * 8.f, 20.f, 20000.f)); // a rough 4 octaves above fundamental
		mod2LPCutoff.setTargetValue(std::clamp(params.mod2freq * 8.f, 20.f, 20000.f));

		LP1.setCutoffFrequency(mod1LPCutoff.getNextValue());
		LP2.setCutoffFrequency(mod1LPCutoff.getNextValue());
		LP3.setCutoffFrequency(mod1LPCutoff.getNextValue());
		LP4.setCutoffFrequency(mod1LPCutoff.getNextValue());

        // 2. process modulators
   
        auto* channelDataL = oversampledBlock.getChannelPointer(0);
        auto* channelDataR = oversampledBlock.getChannelPointer(1);

        auto oversampledNumSamples = oversampledBlock.getNumSamples();

        for (int i = 0; i < (int)oversampledNumSamples; i++) {
            auto sin1 = FastMath<float>::simdSin(mod1Phases);
			auto sin2 = FastMath<float>::simdSin(mod2Phases);
			auto square1 = simdSquare(mod1Phases);
			auto square2 = simdSquare(mod2Phases);
			auto saw1 = simdSaw(mod1Phases);
			auto saw2 = simdSaw(mod2Phases);
            

            juce::dsp::SIMDRegister<float> mod1, mod2;
			if (params.shape1 < 0.5f) {
				mod1[0] = sin1[0] * (1.f - params.shape1 * 2.0f) + square1[0] * params.shape1 * 2.0f;
				mod1[1] = sin1[1] * (1.f - params.shape1 * 2.0f) + square1[1] * params.shape1 * 2.0f;
				mod1[2] = sin1[2] * (1.f - params.shape1 * 2.0f) + square1[2] * params.shape1 * 2.0f;
				mod1[3] = sin1[3] * (1.f - params.shape1 * 2.0f) + square1[3] * params.shape1 * 2.0f;
			}
			else {
				mod1[0] = square1[0] * (1.0f - params.shape1) * 2.0f + saw1[0] * (params.shape1 - 0.5f) * 2.f;
				mod1[1] = square1[1] * (1.0f - params.shape1) * 2.0f + saw1[1] * (params.shape1 - 0.5f) * 2.f;
				mod1[2] = square1[2] * (1.0f - params.shape1) * 2.0f + saw1[2] * (params.shape1 - 0.5f) * 2.f;
				mod1[3] = square1[3] * (1.0f - params.shape1) * 2.0f + saw1[3] * (params.shape1 - 0.5f) * 2.f;
			}
			mod1[0] = LP1.processSample(0, mod1[0]);
			mod1[1] = LP1.processSample(1, mod1[1]);
            mod1[2] = LP2.processSample(0, mod1[2]);
			mod1[3] = LP2.processSample(0, mod1[3]);

            if (params.shape2 < 0.5f) {
                mod2[0] = sin2[0] * (1.f - params.shape2 * 2.0f) + square2[0] * params.shape2 * 2.0f;
                mod2[1] = sin2[1] * (1.f - params.shape2 * 2.0f) + square2[1] * params.shape2 * 2.0f;
                mod2[2] = sin2[2] * (1.f - params.shape2 * 2.0f) + square2[2] * params.shape2 * 2.0f;
                mod2[3] = sin2[3] * (1.f - params.shape2 * 2.0f) + square2[3] * params.shape2 * 2.0f;
            }
			else {
				mod2[0] = square2[0] * (1.0f - params.shape2) * 2.0f + saw2[0] * (params.shape2 - 0.5f) * 2.f;
				mod2[1] = square2[1] * (1.0f - params.shape2) * 2.0f + saw2[1] * (params.shape2 - 0.5f) * 2.f;
				mod2[2] = square2[2] * (1.0f - params.shape2) * 2.0f + saw2[2] * (params.shape2 - 0.5f) * 2.f;
				mod2[3] = square2[3] * (1.0f - params.shape2) * 2.0f + saw2[3] * (params.shape2 - 0.5f) * 2.f;
			}
			mod2[0] = LP3.processSample(0, mod2[0]);
			mod2[1] = LP3.processSample(1, mod2[1]);
			mod2[2] = LP4.processSample(0, mod2[2]);
			mod2[3] = LP4.processSample(0, mod2[2]);

            // 3. apply modulators to audio

            auto sampleL = channelDataL[i];
            auto sampleR = channelDataR[i];
            
            // 4. apply filters to multipliers' outputs
            auto modSampleL1Stage1 = sampleL * mod1[0] * params.mix1;
			auto modSampleL2Stage1 = sampleR * mod1[1] * params.mix1;
			auto modSampleR1Stage1 = sampleL * mod1[2] * params.mix1;
			auto modSampleR2Stage1 = sampleR * mod1[3] * params.mix1;
            modSampleL1Stage1 = highCut1.processSample(0, modSampleL1Stage1);
			modSampleL2Stage1 = highCut1.processSample(1, modSampleL2Stage1);
            modSampleR1Stage1 = highCut2.processSample(0, modSampleR1Stage1);
			modSampleR2Stage1 = highCut2.processSample(1, modSampleR2Stage1);
			modSampleL1Stage1 = lowCut1.processSample(0, modSampleL1Stage1);
			modSampleL2Stage1 = lowCut1.processSample(1, modSampleL2Stage1);
			modSampleR1Stage1 = lowCut2.processSample(0, modSampleR1Stage1);
			modSampleR2Stage1 = lowCut2.processSample(1, modSampleR2Stage1);

            auto sampleL1Stage1 = (sampleL * (1.f - params.mix1) + modSampleL1Stage1);
            auto sampleL2Stage1 = (sampleL * (1.f - params.mix1) + modSampleL2Stage1);
            auto sampleR1Stage1 = (sampleR * (1.f - params.mix1) + modSampleR1Stage1);
            auto sampleR2Stage1 = (sampleR * (1.f - params.mix1) + modSampleR2Stage1);

			auto modSampleL1Stage2 = sampleL1Stage1 * mod2[0] * params.mix2;
			auto modSampleL2Stage2 = sampleL2Stage1 * mod2[1] * params.mix2;
			auto modSampleR1Stage2 = sampleR1Stage1 * mod2[2] * params.mix2;
			auto modSampleR2Stage2 = sampleR2Stage1 * mod2[3] * params.mix2;
			modSampleL1Stage2 = highCut3.processSample(0, modSampleL1Stage2);
			modSampleL2Stage2 = highCut3.processSample(1, modSampleL2Stage2);
			modSampleR1Stage2 = highCut4.processSample(0, modSampleR1Stage2);
			modSampleR2Stage2 = highCut4.processSample(1, modSampleR2Stage2);
			modSampleL1Stage2 = lowCut3.processSample(0, modSampleL1Stage2);
			modSampleL2Stage2 = lowCut3.processSample(1, modSampleL2Stage2);
			modSampleR1Stage2 = lowCut4.processSample(0, modSampleR1Stage2);
			modSampleR2Stage2 = lowCut4.processSample(1, modSampleR2Stage2);

            auto sampleL1Stage2 = (sampleL1Stage1 * (1.f - params.mix2) + modSampleL1Stage2);
            auto sampleL2Stage2 = (sampleL2Stage1 * (1.f - params.mix2) + modSampleL2Stage2);
            auto sampleR1Stage2 = (sampleR1Stage1 * (1.f - params.mix2) + modSampleR1Stage2);
            auto sampleR2Stage2 = (sampleR2Stage1 * (1.f - params.mix2) + modSampleR2Stage2);
            
            auto sampleLSum = (sampleL1Stage2 + sampleL2Stage2) * 0.5f;
            auto sampleRSum = (sampleR1Stage2 + sampleR2Stage2) * 0.5f;

            channelDataL[i] = sampleLSum; // sampleLSum;
            channelDataR[i] = sampleRSum; // sampleRSum;

            mod1Phases += mod1PhaseIncs;
            mod2Phases += mod2PhaseIncs;

            for (int k = 0; k < 4; k++) {
				if (mod1Phases[k] > juce::MathConstants<float>::pi)
					mod1Phases[k] = mod1Phases[k] - 2.f * juce::MathConstants<float>::pi;
				if (mod2Phases[k] > juce::MathConstants<float>::pi)
					mod2Phases[k] = mod2Phases[k] - 2.f * juce::MathConstants<float>::pi;
			}
        }

        highCut1.setCutoffFrequency(params.highcut);
        highCut2.setCutoffFrequency(params.highcut);
		highCut3.setCutoffFrequency(params.highcut);
		highCut4.setCutoffFrequency(params.highcut);
		lowCut1.setCutoffFrequency(params.lowcut);
		lowCut2.setCutoffFrequency(params.lowcut);
		lowCut3.setCutoffFrequency(params.lowcut);
		lowCut4.setCutoffFrequency(params.lowcut);
        
        // processed that buffer: downsample it, and SHIP IT OUT!
        oversampler->processSamplesDown(context.getOutputBlock());
    }

    float minimaxSin(float x1) {
        float x2 = x1 * x1;

        return x1 * (0.99999999997884898600402426033768998f
        + x2 * (-0.166666666088260696413164261885310067f
        + x2 * (0.00833333072055773645376566203656709979f
        + x2 * (-0.000198408328232619552901560108010257242f
        + x2 * (2.75239710746326498401791551303359689e-6f
        - 2.3868346521031027639830001794722295e-8f * x2)))));
        
    }

private:
    // external params
    double sampleRate{ 44100.0 };
    
    // utility functions
    juce::dsp::SIMDRegister<float> simdSaw(juce::dsp::SIMDRegister<float> phases) {
        return (phases * (float)M_1_PI) * 2.0f - 1.0f;
	}

    juce::dsp::SIMDRegister<float> simdSquare(juce::dsp::SIMDRegister<float> phases) {
		juce::dsp::SIMDRegister<float> square;
        square[0] = phases[0] > 0.f ? 1.f : -1.f;
		square[1] = phases[1] > 0.f ? 1.f : -1.f;
		square[2] = phases[2] > 0.f ? 1.f : -1.f;
		square[3] = phases[3] > 0.f ? 1.f : -1.f;
        return square;
    }
    
    // internal storage / utility
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
    int oversampleOrder{ 1 }, oversampleRatio{ 2 };
    double oversampledSampleRate{ sampleRate * oversampleRatio };
    double inverseOversampledSampleRate{ 1.0 / oversampledSampleRate };
    RingModParams params;
    juce::dsp::StateVariableTPTFilter<float> LP1, LP2, LP3, LP4; // these are stereo
    juce::dsp::StateVariableTPTFilter<float> highCut1, highCut2, highCut3, highCut4, lowCut1, lowCut2, lowCut3, lowCut4; // post multiply, pre-stagemix
    juce::dsp::SIMDRegister<float> mod1Phases{ 0.0f }, mod2Phases{ 0.0f };
    juce::dsp::SIMDRegister<float> mod1PhaseIncs{ 0.0f }, mod2PhaseIncs{ 0.0f };

	float semis[4] = { 0.12f, 0.06f, -0.0566f, -0.1071f };
	const juce::dsp::SIMDRegister<float> semitones = juce::dsp::SIMDRegister<float>::fromRawArray(semis);


    juce::SmoothedValue<float> mod1LPCutoff, mod2LPCutoff;
};

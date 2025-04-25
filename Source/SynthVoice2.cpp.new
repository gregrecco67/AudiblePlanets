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

//#define MIPP_ALIGNED_LOADS
#include "SynthVoice.h"
#include "PluginProcessor.h"
#include "SynthVoice2.h"

inline mipp::Reg<float> SynthVoice::minimaxSin(mipp::Reg<float> x1) {
	mipp::Reg<float> x2 = x1 * x1;
	return mipp::mul(x1, mipp::fmadd(x2, mipp::fmadd(x2, mipp::fmadd(x2, mipp::fmadd(x2, mipp::fmadd(s6, x2, s5), s4), s3), s2), s1));
}

inline std::array<float, 2> SynthVoice::panWeights(const float in) { // -1 to 1
	return { std::sqrt((in + 1.f) * 0.5f), std::sqrt(1.f - ((in + 1.f) * 0.5f) };
}

inline mipp::Reg<float> SynthVoice::mmAtan(mipp::Reg<float> x1) {
	mipp::Reg<float> x2 = x1 * x1;
	return mipp::mul(x1, mipp::fmadd(x2, mipp::fmadd(x2, mipp::fmadd(x2, mipp::fmadd(x2, mipp::fmadd(x2, t1, t2), t3), t4), t5), t6));
}
inline mipp::Reg<float> SynthVoice::fastAtan2(mipp::Reg<float> x, mipp::Reg<float> y) {
	mipp::Reg<float> out = 0.f;
	mipp::Msk<mipp::N<float>()> xEqZero = mipp::cmpeq<float>(x, 0.0f);
	mipp::Msk<mipp::N<float>()> yEqZero = mipp::cmpeq<float>(y, 0.0f);
	mipp::Msk<mipp::N<float>()> xGtZero = mipp::cmpgt<float>(x, 0.0f);
	mipp::Msk<mipp::N<float>()> yGtEqZero = mipp::cmpge<float>(y, 0.0f);
	mipp::Msk<mipp::N<float>()> yGtZero = mipp::cmpgt<float>(y, 0.0f);
	mipp::Msk<mipp::N<float>()> xGty = mipp::cmpgt<float>(mipp::abs(x), mipp::abs(y));
	mipp::Reg<float> z = mipp::blend<float>(y / x, x / y, xGty);
    mipp::Reg<float> atanZ = mmAtan(z);
	out = mipp::blend<float>(piReg * 0.5f, out, yGtZero & xEqZero); // #1
	out = mipp::blend<float>(piReg * -.5f, out, ~yGtZero & xEqZero); // #2
	out = mipp::blend<float>(atanZ, out, xGty & xGtZero); // #3
	out = mipp::blend<float>(atanZ + piReg, out, xGty & ~xGtZero & yGtEqZero); // #4
	out = mipp::blend<float>(atanZ - piReg, out, xGty & ~xGtZero & ~yGtEqZero); // #5
	out = mipp::blend<float>(atanZ * -1.f + piReg * 0.5f, out, ~xGty & yGtZero); // #6
	out = mipp::blend<float>(atanZ * -1.f - piReg * 0.5f, out, ~xGty & ~yGtZero); // #7
	out = mipp::blend<float>(0.f, out, xEqZero & yEqZero); // #8
	return out;
}

//==============================================================================
SynthVoice::SynthVoice(APAudioProcessor& p)
	: proc(p), mseg1(proc.mseg1Data), mseg2(proc.mseg2Data), mseg3(proc.mseg3Data), mseg4(proc.mseg4Data),
	osc1(p.analogTables, 1), osc2(p.analogTables, 1), osc3(p.analogTables, 1), osc4(p.analogTables, 1)
{
	mseg1.reset();
	mseg2.reset();
	mseg3.reset();
	mseg4.reset();
	filter.setNumChannels(2);
}

void SynthVoice::noteStarted()
{
    curNote = getCurrentlyPlayingNote();

	proc.modMatrix.setPolyValue(*this, proc.randSrc1Poly, static_cast<float>(dist(gen)));
	proc.modMatrix.setPolyValue(*this, proc.randSrc2Poly, static_cast<float>(dist(gen)));
	
	if (MTS_ShouldFilterNote(proc.client, curNote.initialNote, curNote.midiChannel)) {
		return;
	}

	fastKill = false;
	startVoice();

	auto note = getCurrentlyPlayingNote();
	if (glideInfo.fromNote >= 0 && (glideInfo.glissando || glideInfo.portamento))
	{
		noteSmoother.setTime(glideInfo.rate);
		noteSmoother.setValueUnsmoothed(glideInfo.fromNote / 127.0f);
		noteSmoother.setValue(note.initialNote / 127.0f);
	}
	else
	{
		noteSmoother.setValueUnsmoothed(note.initialNote / 127.0f);
	}

	proc.modMatrix.setPolyValue(*this, proc.modSrcVelocity, note.noteOnVelocity.asUnsignedFloat());
	proc.modMatrix.setPolyValue(*this, proc.modSrcTimbre, note.initialTimbre.asUnsignedFloat());
	proc.modMatrix.setPolyValue(*this, proc.modSrcPressure, note.pressure.asUnsignedFloat());

	juce::ScopedValueSetter<bool> svs(disableSmoothing, true);

	filter.reset();

	lfo1.reset();
	lfo2.reset();
	lfo3.reset();
	lfo4.reset();

	lfo1.noteOn();
	lfo2.noteOn();
	lfo3.noteOn();
	lfo4.noteOn();

	updateParams(0);
	snapParams();
	//updateParams(0);
	//snapParams();
	//
	osc1.noteOn(proc.osc1Params.phase->getUserValue());
	osc2.noteOn(proc.osc2Params.phase->getUserValue());
	osc3.noteOn(proc.osc3Params.phase->getUserValue());
	osc4.noteOn(proc.osc4Params.phase->getUserValue());


	env1.noteOn();
	env2.noteOn();
	env3.noteOn();
	env4.noteOn();

	mseg1.noteOn();
	mseg2.noteOn();
	mseg3.noteOn();
	mseg4.noteOn();

}

void SynthVoice::noteRetriggered()
{
	auto note = getCurrentlyPlayingNote();
    curNote = getCurrentlyPlayingNote();

	proc.modMatrix.setPolyValue(*this, proc.randSrc1Poly, static_cast<float>(dist(gen)));
	proc.modMatrix.setPolyValue(*this, proc.randSrc2Poly, static_cast<float>(dist(gen)));

	if (glideInfo.fromNote >= 0 && (glideInfo.glissando || glideInfo.portamento))
	{
		noteSmoother.setTime(glideInfo.rate);
		noteSmoother.setValue(note.initialNote / 127.0f);
	}
	else
	{
		noteSmoother.setValueUnsmoothed(note.initialNote / 127.0f);
	}

	proc.modMatrix.setPolyValue(*this, proc.modSrcVelocity, note.noteOnVelocity.asUnsignedFloat());
	proc.modMatrix.setPolyValue(*this, proc.modSrcTimbre, note.initialTimbre.asUnsignedFloat());
	proc.modMatrix.setPolyValue(*this, proc.modSrcPressure, note.pressure.asUnsignedFloat());

	updateParams(0);

	osc1.noteOn();
	osc2.noteOn();
	osc3.noteOn();
	osc4.noteOn();

	env1.noteOn();
	env2.noteOn();
	env3.noteOn();
	env4.noteOn();

}

void SynthVoice::noteStopped(bool allowTailOff)
{
	env1.noteOff();
	env2.noteOff();
	env3.noteOff();
	env4.noteOff();
	curNote = getCurrentlyPlayingNote();
	proc.modMatrix.setPolyValue(*this, proc.modSrcVelOff, curNote.noteOffVelocity.asUnsignedFloat());
    if (!allowTailOff) {
        clearCurrentNote();
        stopVoice();
    }

}

void SynthVoice::notePressureChanged()
{
	auto note = getCurrentlyPlayingNote();
	proc.modMatrix.setPolyValue(*this, proc.modSrcPressure, note.pressure.asUnsignedFloat());
}

void SynthVoice::noteTimbreChanged()
{
	auto note = getCurrentlyPlayingNote();
	proc.modMatrix.setPolyValue(*this, proc.modSrcTimbre, note.timbre.asUnsignedFloat());
}

void SynthVoice::setCurrentSampleRate(double newRate)
{
	MPESynthesiserVoice::setCurrentSampleRate(newRate);

	osc1.setSampleRate(newRate);
	osc2.setSampleRate(newRate);
	osc3.setSampleRate(newRate);
	osc4.setSampleRate(newRate);

	filter.setSampleRate(newRate);

	lfo1.setSampleRate(newRate);
	lfo2.setSampleRate(newRate);
	lfo3.setSampleRate(newRate);
	lfo4.setSampleRate(newRate);

	noteSmoother.setSampleRate(newRate);
    Envelope::Params p;
    env1.setSampleRate(newRate);
    env1.setParameters(p);
	env2.setSampleRate(newRate);
    env2.setParameters(p);
	env3.setSampleRate(newRate);
    env3.setParameters(p);
	env4.setSampleRate(newRate);
    env4.setParameters(p);

	mseg1.setSampleRate(newRate);
	mseg2.setSampleRate(newRate);
	mseg3.setSampleRate(newRate);
	mseg4.setSampleRate(newRate);
}

void SynthVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
	updateParams(numSamples);

	synthBuffer.setSize(2, numSamples, false, false, true);

	osc1.renderPositions(osc1Freq, osc1Params, osc1Positions, numSamples); 
	osc2.renderPositions(osc2Freq, osc2Params, osc2Positions, numSamples);
	osc3.renderPositions(osc3Freq, osc3Params, osc3Positions, numSamples);
	osc4.renderPositions(osc4Freq, osc4Params, osc4Positions, numSamples);

	// more squash = smaller k, which scales about the perpendicular to the vector to the equant
	float k = 1.f - getValue(proc.globalParams.squash); 
    
	// the whole enchilada
	for (int i = 0; i < numSamples; i++)
    {
        env1.getNextSample(); // advance each envelope, we'll read them below as necessary
        env2.getNextSample();
        env3.getNextSample();
        env4.getNextSample();
        auto a = envs[0]->getOutput(); // read each envelope value from the pointer for each osc
        auto b = envs[1]->getOutput(); // a = current output of envelope assigned to osc1, etc.
        auto c = envs[2]->getOutput(); //
        auto d = envs[3]->getOutput();
        
        // calculate squash matrix
        // get distances in order to normalize vectors
        float distanceL = std::sqrt(osc1Positions[i].xL * osc1Positions[i].xL + (osc1Positions[i].yL - equant) * (osc1Positions[i].yL - equant));
        float distanceR = std::sqrt(osc1Positions[i].xR * osc1Positions[i].xR + (osc1Positions[i].yR - equant) * (osc1Positions[i].yR - equant));
        float invDistanceL = 1.f / (distanceL + .000001f);
        float invDistanceR = 1.f / (distanceR + .000001f);
        // normalized vectors
        float cosThetaL = (osc1Positions[i].yL - equant) * invDistanceL; // what we want is the tangent to the orbit at this point
        float sinThetaL = -osc1Positions[i].xL * invDistanceL; // so swap x and y and negate y
        float cosThetaR = (osc1Positions[i].yR - equant) * invDistanceR;
        float sinThetaR = -osc1Positions[i].xR * invDistanceR; // +.000001f to avoid divide by zero
        float cos2ThetaL = cosThetaL * cosThetaL;
        float cos2ThetaR = cosThetaR * cosThetaR;
        float sin2ThetaL = sinThetaL * sinThetaL;
        float sin2ThetaR = sinThetaR * sinThetaR;
        
        // plug in to transform matrix
        StereoMatrix squash1 = {
            .left = {
                .a = cos2ThetaL + k * sin2ThetaL, .b = cosThetaL * sinThetaL * (1.0f - k),
                .c = cosThetaL * sinThetaL * (1.0f - k), .d = sin2ThetaL + k * cos2ThetaL
            },
                .right = {
                    .a = cos2ThetaR + k * sin2ThetaR, .b = cosThetaR * sinThetaR * (1.0f - k),
                    .c = cosThetaR * sinThetaR * (1.0f - k), .d = sin2ThetaR + k * cos2ThetaR
                }
        };
        
        distanceL = std::sqrt(osc2Positions[i].xL * osc2Positions[i].xL + (osc2Positions[i].yL - equant) * (osc2Positions[i].yL - equant));
        distanceR = std::sqrt(osc2Positions[i].xR * osc2Positions[i].xR + (osc2Positions[i].yR - equant) * (osc2Positions[i].yR - equant));
        invDistanceL = 1.f / (distanceL + .000001f);
        invDistanceR = 1.f / (distanceR + .000001f);
        
        cosThetaL =  (osc2Positions[i].yL - equant) * invDistanceL;
        sinThetaL = -osc2Positions[i].xL			* invDistanceL;
        cosThetaR =  (osc2Positions[i].yR - equant) * invDistanceR;
        sinThetaR = -osc2Positions[i].xR			* invDistanceR;
        cos2ThetaL = cosThetaL * cosThetaL;
        cos2ThetaR = cosThetaR * cosThetaR;
        sin2ThetaL = sinThetaL * sinThetaL;
        sin2ThetaR = sinThetaR * sinThetaR;
        
        StereoMatrix squash2 = {
            .left = {
                .a = cos2ThetaL + k * sin2ThetaL, .b = cosThetaL * sinThetaL * (1.0f - k),
                .c = cosThetaL * sinThetaL * (1.0f - k), .d = sin2ThetaL + k * cos2ThetaL
            },
                .right = {
                    .a = cos2ThetaR + k * sin2ThetaR, .b = cosThetaR * sinThetaR * (1.0f - k),
                    .c = cosThetaR * sinThetaR * (1.0f - k), .d = sin2ThetaR + k * cos2ThetaR
                }
        };
        
        distanceL = std::sqrt(osc3Positions[i].xL * osc3Positions[i].xL + (osc3Positions[i].yL - equant) * (osc3Positions[i].yL - equant));
        distanceR = std::sqrt(osc3Positions[i].xR * osc3Positions[i].xR + (osc3Positions[i].yR - equant) * (osc3Positions[i].yR - equant));
        invDistanceL = 1.f / (distanceL + .000001f);
        invDistanceR = 1.f / (distanceR + .000001f);
        
        cosThetaL =  (osc3Positions[i].yL - equant) * invDistanceL;
        sinThetaL = -osc3Positions[i].xL			* invDistanceL;
        cosThetaR =  (osc3Positions[i].yR - equant) * invDistanceR;
        sinThetaR = -osc3Positions[i].xR			* invDistanceR;
        cos2ThetaL = cosThetaL * cosThetaL;
        cos2ThetaR = cosThetaR * cosThetaR;
        sin2ThetaL = sinThetaL * sinThetaL;
        sin2ThetaR = sinThetaR * sinThetaR;
        
        StereoMatrix squash3 = {
            .left = {
                .a = cos2ThetaL + k * sin2ThetaL, .b = cosThetaL * sinThetaL * (1.0f - k),
                .c = cosThetaL * sinThetaL * (1.0f - k), .d = sin2ThetaL + k * cos2ThetaL
            },
                .right = {
                    .a = cos2ThetaR + k * sin2ThetaR, .b = cosThetaR * sinThetaR * (1.0f - k),
                    .c = cosThetaR * sinThetaR * (1.0f - k), .d = sin2ThetaR + k * cos2ThetaR
                }
        };
        
        
        epi1 = osc1Positions[i] * (a * osc1Vol);
        
        // apply the squash matrix to squash secondary orbits along the tangent of the one they're orbiting
        epi2 = epi1 + ((osc2Positions[i] * squash1) * (b * osc2Vol));
        
        // get bodies 3 & 4 position by algorithm
        if (algo == 0) // 1-2-3-(4)
        {
            epi3 = epi2 + ((osc3Positions[i] * squash2) * (c * osc3Vol));
            epi4 = epi3 + ((osc4Positions[i] * squash3) * (d * osc4Vol));
        }
        if (algo == 1) { // 1-2-(3), 2-(4)
            epi3 = epi2 + ((osc3Positions[i] * squash2) * (c * osc3Vol));
            epi4 = epi2 + ((osc4Positions[i] * squash2) * (d * osc4Vol));
        }
        if (algo == 2) { // 1-(2), 1-3-(4)
            epi3 = epi1 + ((osc3Positions[i] * squash1) * (c * osc3Vol));
            epi4 = epi3 + ((osc4Positions[i] * squash3) * (d * osc4Vol));
        }
        if (algo == 3) { // 1-(2), 1-(3), 1-(4)
            epi3 = epi1 + ((osc3Positions[i] * squash1) * (c * osc3Vol));
            epi4 = epi1 + ((osc4Positions[i] * squash1) * (d * osc4Vol));
        }
        
        epi2xls[i] = epi2.xL; epi2yls[i] = epi2.yL; epi2xrs[i] = epi2.xR; epi2yrs[i] = epi2.yR;
		epi3xls[i] = epi3.xL; epi3yls[i] = epi3.yL; epi3xrs[i] = epi3.xR; epi3yrs[i] = epi3.yR;
		epi4xls[i] = epi4.xL; epi4yls[i] = epi4.yL; epi4xrs[i] = epi4.xR; epi4yrs[i] = epi4.yR;
    }

	auto synthBufferL = synthBuffer.getWritePointer(0);
	auto synthBufferR = synthBuffer.getWritePointer(1);



	for (int i = 0; i < std::ceil(((float)numSamples)/4.f); i++)
    {
		epi2xL = { epi2xls[i * 4], epi2xls[i * 4 + 1], epi2xls[i * 4 + 2], epi2xls[i * 4 + 3] };
		epi2yL = { epi2yls[i * 4], epi2yls[i * 4 + 1], epi2yls[i * 4 + 2], epi2yls[i * 4 + 3] };
		epi2xR = { epi2xrs[i * 4], epi2xrs[i * 4 + 1], epi2xrs[i * 4 + 2], epi2xrs[i * 4 + 3] };
		epi2yR = { epi2yrs[i * 4], epi2yrs[i * 4 + 1], epi2yrs[i * 4 + 2], epi2yrs[i * 4 + 3] };
		epi3xL = { epi3xls[i * 4], epi3xls[i * 4 + 1], epi3xls[i * 4 + 2], epi3xls[i * 4 + 3] };
		epi3yL = { epi3yls[i * 4], epi3yls[i * 4 + 1], epi3yls[i * 4 + 2], epi3yls[i * 4 + 3] };
		epi3xR = { epi3xrs[i * 4], epi3xrs[i * 4 + 1], epi3xrs[i * 4 + 2], epi3xrs[i * 4 + 3] };
		epi3yR = { epi3yrs[i * 4], epi3yrs[i * 4 + 1], epi3yrs[i * 4 + 2], epi3yrs[i * 4 + 3] };
		epi4xL = { epi4xls[i * 4], epi4xls[i * 4 + 1], epi4xls[i * 4 + 2], epi4xls[i * 4 + 3] };
		epi4yL = { epi4yls[i * 4], epi4yls[i * 4 + 1], epi4yls[i * 4 + 2], epi4yls[i * 4 + 3] };
		epi4xR = { epi4xrs[i * 4], epi4xrs[i * 4 + 1], epi4xrs[i * 4 + 2], epi4xrs[i * 4 + 3] };
		epi4yR = { epi4yrs[i * 4], epi4yrs[i * 4 + 1], epi4yrs[i * 4 + 2], epi4yrs[i * 4 + 3] };
        
    // ----------------------------------------
		// interpret bodies' positions by algorithm
		// ----------------------------------------
		
		// 1. get angles
		atanAngle2L = { 0.f, 0.f, 0.f, 0.f };
		atanAngle2R = { 0.f, 0.f, 0.f, 0.f };
		atanAngle3L = { 0.f, 0.f, 0.f, 0.f };
		atanAngle3R = { 0.f, 0.f, 0.f, 0.f };
		atanAngle4L = { 0.f, 0.f, 0.f, 0.f };
		atanAngle4R = { 0.f, 0.f, 0.f, 0.f };

		atanAngle4L = fastAtan2(epi4yL - equant, epi4xL);
		atanAngle4R = fastAtan2(epi4yR - equant, epi4xR);
		if (algo == 1)
		{
			atanAngle3L = fastAtan2(epi3yL - equant, epi3xL);
			atanAngle3R = fastAtan2(epi3yR - equant, epi3xR);
		}
		if (algo == 2) {
			atanAngle2L = fastAtan2(epi2yL - equant, epi2xL);
			atanAngle2R = fastAtan2(epi2yR - equant, epi2xR);
		}
		if (algo == 3) {
			atanAngle2L = fastAtan2(epi2yL - equant, epi2xL);
			atanAngle2R = fastAtan2(epi2yR - equant, epi2xR);
			atanAngle3L = fastAtan2(epi3yL - equant, epi3xL);
			atanAngle3R = fastAtan2(epi3yR - equant, epi3xR);
		}

		// 2. generate component waveforms from angles


		sine4L = minimaxSin(atanAngle4L);
		sine4R = minimaxSin(atanAngle4R);

		//auto atan4LGtZero = atanAngle4L > 0.f;
		//auto atan4RGtZero = atanAngle4R > 0.f;
		square4L = mipp::blend<float>(1.0f, -1.0f, atanAngle4L > 0.f);
		square4R = mipp::blend<float>(1.0f, -1.0f, atanAngle4R > 0.f);
		saw4L = (atanAngle4L * (float)inv_pi) * 2.0f - 1.0f;
		saw4R = (atanAngle4R * (float)inv_pi) * 2.0f - 1.0f;
		if (algo == 1) {
			sine3L = minimaxSin(atanAngle3L);
			sine3R = minimaxSin(atanAngle3R);
			square3L = mipp::blend<float>(1.0f, -1.0f, atanAngle3L > 0.f);
			square3R = mipp::blend<float>(1.0f, -1.0f, atanAngle3R > 0.f);
			saw3L = (atanAngle3L * (float)inv_pi) * 2.0f - 1.0f;
			saw3R = (atanAngle3R * (float)inv_pi) * 2.0f - 1.0f;
		}
		if (algo == 2) {
			sine2L = minimaxSin(atanAngle2L);
			sine2R = minimaxSin(atanAngle2R);
			square2L = mipp::blend<float>(1.0f, -1.0f, atanAngle2L > 0.f);
			square2R = mipp::blend<float>(1.0f, -1.0f, atanAngle2R > 0.f);
			saw2L = (atanAngle2L * (float)inv_pi) * 2.0f - 1.0f;
			saw2R = (atanAngle2R * (float)inv_pi) * 2.0f - 1.0f;
		}
		if (algo == 3) {
			sine2L = minimaxSin(atanAngle2L);
			sine2R = minimaxSin(atanAngle2R);
			square2L = mipp::blend<float>(1.0f, -1.0f, atanAngle2L > 0.f);
			square2R = mipp::blend<float>(1.0f, -1.0f, atanAngle2R > 0.f);
			saw2L = (atanAngle2L * (float)inv_pi) * 2.0f - 1.0f;
			saw2R = (atanAngle2R * (float)inv_pi) * 2.0f - 1.0f;
			sine3L = minimaxSin(atanAngle3L);
			sine3R = minimaxSin(atanAngle3R);
			square3L = mipp::blend<float>(1.0f, -1.0f, atanAngle3L > 0.f);
			square3R = mipp::blend<float>(1.0f, -1.0f, atanAngle3R > 0.f);
			saw3L = (atanAngle3L * (float)inv_pi) * 2.0f - 1.0f;
			saw3R = (atanAngle3R * (float)inv_pi) * 2.0f - 1.0f;
		}

		// 3. mix component waveforms by blend value

		auto blend = getValue(proc.timbreParams.blend);
		if (blend < 0.5f)
		{
			sample2L = (sine2L * (1.f - blend * 2.0f) + square2L * blend * 2.0f);
			sample2R = (sine2R * (1.f - blend * 2.0f) + square2R * blend * 2.0f);
			sample3L = (sine3L * (1.f - blend * 2.0f) + square3L * blend * 2.0f);
			sample3R = (sine3R * (1.f - blend * 2.0f) + square3R * blend * 2.0f);
			sample4L = (sine4L * (1.f - blend * 2.0f) + square4L * blend * 2.0f);
			sample4R = (sine4R * (1.f - blend * 2.0f) + square4R * blend * 2.0f);
		}
		else {
			sample2L = (square2L * (1.0f - blend) * 2.0f + saw2L * (blend - 0.5f) * 2.f);
			sample2R = (square2R * (1.0f - blend) * 2.0f + saw2R * (blend - 0.5f) * 2.f);
			sample3L = (square3L * (1.0f - blend) * 2.0f + saw3L * (blend - 0.5f) * 2.f);
			sample3R = (square3R * (1.0f - blend) * 2.0f + saw3R * (blend - 0.5f) * 2.f);
			sample4L = (square4L * (1.0f - blend) * 2.0f + saw4L * (blend - 0.5f) * 2.f);
			sample4R = (square4R * (1.0f - blend) * 2.0f + saw4R * (blend - 0.5f) * 2.f);
		}

		// 4. compute modulated and demodulated samples, and mix them by demodmix
		modSample2L = sample2L; demodSample2L = sample2L; modSample2R = sample2R; demodSample2R = sample2R;
		modSample3L = sample3L; demodSample3L = sample3L; modSample3R = sample3R; demodSample3R = sample3R;
		modSample4L = sample4L; demodSample4L = sample4L; modSample4R = sample4R; demodSample4R = sample4R;

		// 5. demodulate by considering not just angle, but also magnitude of planet vector
		auto atanDistance2L = mipp::sqrt(epi2xL * epi2xL + (epi2yL + equant) * (epi2yL + equant));
		auto atanDistance2R = mipp::sqrt(epi2xR * epi2xR + (epi2yR + equant) * (epi2yR + equant));
		auto atanDistance3L = mipp::sqrt(epi3xL * epi3xL + (epi3yL + equant) * (epi3yL + equant));
		auto atanDistance3R = mipp::sqrt(epi3xR * epi3xR + (epi3yR + equant) * (epi3yR + equant));
		auto atanDistance4L = mipp::sqrt(epi4xL * epi4xL + (epi4yL + equant) * (epi4yL + equant));
		auto atanDistance4R = mipp::sqrt(epi4xR * epi4xR + (epi4yR + equant) * (epi4yR + equant));

		demodSample2L *= (atanDistance2L * demodVol); // adjustable balancing term
		demodSample2R *= (atanDistance2R * demodVol);
		demodSample3L *= (atanDistance3L * demodVol);
		demodSample3R *= (atanDistance3R * demodVol);
		demodSample4L *= (atanDistance4L * demodVol);
		demodSample4R *= (atanDistance4R * demodVol);

		// since mod samples are angle-only, we need to reapply their envelope values
		modSample2L *= envs[1]->getOutput();
		modSample2R *= envs[1]->getOutput();
		modSample3L *= envs[2]->getOutput();
		modSample3R *= envs[2]->getOutput();
		modSample4L *= envs[3]->getOutput();
		modSample4R *= envs[3]->getOutput();

		// 6. mix by demodmix AND ALGO

		auto demodmix = getValue(proc.timbreParams.demodmix);
		if (algo == 0) {
			sampleL = (modSample4L * (1.0f - demodmix) + demodSample4L * demodmix);
			sampleR = (modSample4R * (1.0f - demodmix) + demodSample4R * demodmix);
		}
		if (algo == 1) {
			sampleL = ((modSample3L + modSample4L) * 0.5f * (1.0f - demodmix) + (demodSample3L + demodSample4L) * 0.5f * demodmix);
			sampleR = ((modSample3R + modSample4R) * 0.5f * (1.0f - demodmix) + (demodSample3R + demodSample4R) * 0.5f * demodmix);
		}
		if (algo == 2) {
			sampleL = ((modSample2L + modSample4L) * 0.5f * (1.0f - demodmix) + (demodSample2L + demodSample4L) * 0.5f * demodmix);
			sampleR = ((modSample2R + modSample4R) * 0.5f * (1.0f - demodmix) + (demodSample2R + demodSample4R) * 0.5f * demodmix);
		}
		if (algo == 3) {
			sampleL = ((modSample2L + modSample3L + modSample4L) * 0.333f * (1.0f - demodmix) + (demodSample2L + demodSample3L + demodSample4L) * 0.333f * demodmix);
			sampleR = ((modSample2R + modSample3R + modSample4R) * 0.333f * (1.0f - demodmix) + (demodSample2R + demodSample3L + demodSample4R) * 0.333f * demodmix);
		}

		sampleL = mipp::sat(sampleL, -1.0f, 1.0f);
		sampleR = mipp::sat(sampleR, -1.0f, 1.0f);

		// 7. SHIP IT OUT
		sampleL.store(&synthBufferL[i * mipp::N<float>()]);
		sampleR.store(&synthBufferR[i * mipp::N<float>()]);
	}

	// Get and apply velocity according to keytrack param
	float velocity = currentlyPlayingNote.noteOnVelocity.asUnsignedFloat();
	float ampKeyTrack = getValue(proc.globalParams.velSens);
	synthBuffer.applyGain(gin::velocityToGain(velocity, ampKeyTrack) * baseAmplitude);
	
	filter.process(synthBuffer);

    bool voiceShouldStop = false;
	switch(algo) {
	case 0:
		if (!envs[3]->isActive()) // 1-2-3-(4)
			voiceShouldStop = true;
		break;
	case 1:
		if (!envs[2]->isActive() && !envs[3]->isActive()) // 1-2-(3), 2-(4)
			voiceShouldStop = true;
		break;
	case 2:
		if (!envs[1]->isActive() && !envs[3]->isActive()) // 1-(2), 1-3-(4)
			voiceShouldStop = true;
		break;
	case 3:
		if (!envs[1]->isActive() && !envs[2]->isActive() && !envs[3]->isActive()) // 1-(2), 1-(3), 1-(4)
			voiceShouldStop = true;
		break;
	}
    if (voiceShouldStop)
	{
		clearCurrentNote();
		stopVoice();
	}

	// Copy synth voice to output
	outputBuffer.addFrom(0, startSample, synthBuffer, 0, 0, numSamples);
	outputBuffer.addFrom(1, startSample, synthBuffer, 1, 0, numSamples);

	finishBlock(numSamples);
}

void SynthVoice::updateParams(int blockSize)
{
	algo = (int)getValue(proc.timbreParams.algo);
	equant = getValue(proc.timbreParams.equant);
	demodVol = getValue(proc.timbreParams.demodVol);

	auto note = getCurrentlyPlayingNote();

	proc.modMatrix.setPolyValue(*this, proc.modSrcNote, note.initialNote / 127.0f);

	currentMidiNote = noteSmoother.getCurrentValue() * 127.0f;
	if (glideInfo.glissando) currentMidiNote = (float)juce::roundToInt(currentMidiNote);

	float dummy;
	float remainder = std::modf(currentMidiNote, &dummy);
	float baseFreq = static_cast<float>(MTS_NoteToFrequency(proc.client, static_cast<char>(currentMidiNote), note.midiChannel));
	baseFreq *= static_cast<float>(std::pow(1.05946309436f, note.totalPitchbendInSemitones * (proc.globalParams.pitchbendRange->getUserValue() / 2.0f) + remainder));
	baseFreq = juce::jlimit(20.0f, 20000.f, baseFreq * getValue(proc.timbreParams.pitch));
	
	if (proc.osc1Params.fixed->isOn()) {
		osc1Freq = ((int)(getValue(proc.osc1Params.coarse) + 0.0001f) + getValue(proc.osc1Params.fine)) * 100.f;
	}
	else {
		osc1Freq = baseFreq * ((int)(getValue(proc.osc1Params.coarse) + 0.0001f) + getValue(proc.osc1Params.fine));
	}
	if (proc.osc2Params.fixed->isOn()) {
		osc2Freq = ((int)(getValue(proc.osc2Params.coarse) + 0.0001f) + getValue(proc.osc2Params.fine)) * 100.f;
	}
	else {
		osc2Freq = baseFreq * ((int)(getValue(proc.osc2Params.coarse) + 0.0001f) + getValue(proc.osc2Params.fine));
	}
	if (proc.osc3Params.fixed->isOn()) {
		osc3Freq = ((int)(getValue(proc.osc3Params.coarse) + 0.0001f) + getValue(proc.osc3Params.fine)) * 100.f;
	}
	else {
		osc3Freq = baseFreq * ((int)(getValue(proc.osc3Params.coarse) + 0.0001f) + getValue(proc.osc3Params.fine));
	}
	if (proc.osc4Params.fixed->isOn()) {
		osc4Freq = ((int)(getValue(proc.osc4Params.coarse) + 0.0001f) + getValue(proc.osc4Params.fine)) * 100.f;
	}
	else {
		osc4Freq = baseFreq * ((int)(getValue(proc.osc4Params.coarse) + 0.0001f) + getValue(proc.osc4Params.fine));
	}

	osc1Params.wave = waveForChoice(int(getValue(proc.osc1Params.wave)));
	osc1Params.tones = getValue(proc.osc1Params.tones);
	osc1Params.pan = getValue(proc.osc1Params.pan);
	osc1Params.phaseShift = getValue(proc.osc1Params.phase);
	osc1Vol = getValue(proc.osc1Params.volume);

	switch(proc.osc1Params.env->getUserValueInt())
	{
	case 0:
		envs[0] = &env1;
		break;
	case 1:
		envs[0] = &env2;
		break;
	case 2:
		envs[0] = &env3;
		break;
	case 3:
		envs[0] = &env4;
		break;
	}

	// osc 2
	// 
	//==================================================

	osc2Params.wave = waveForChoice(int(getValue(proc.osc2Params.wave)));
	osc2Params.tones = getValue(proc.osc2Params.tones);
	osc2Params.pan = getValue(proc.osc2Params.pan);
	osc2Params.phaseShift = getValue(proc.osc2Params.phase);
	osc2Vol = getValue(proc.osc2Params.volume);
	switch(proc.osc2Params.env->getUserValueInt())
	{
	case 0:
		envs[1] = &env1;
		break;
	case 1:
		envs[1] = &env2;
		break;
	case 2:
		envs[1] = &env3;
		break;
	case 3:
		envs[1] = &env4;
		break;
	}

	// osc 3
	// ------------------

	osc3Params.wave = waveForChoice(int(getValue(proc.osc3Params.wave)));
	osc3Params.tones = getValue(proc.osc3Params.tones);
	osc3Params.pan = getValue(proc.osc3Params.pan);
	osc3Params.phaseShift = getValue(proc.osc3Params.phase);
	osc3Vol = getValue(proc.osc3Params.volume);
	switch(proc.osc3Params.env->getUserValueInt())
	{
	case 0:
		envs[2] = &env1;
		break;
	case 1:
		envs[2] = &env2;
		break;
	case 2:
		envs[2] = &env3;
		break;
	case 3:
		envs[2] = &env4;
		break;
	}

	// osc4 
	// -------------
	osc4Params.wave = waveForChoice(int(getValue(proc.osc4Params.wave)));
	osc4Params.tones = getValue(proc.osc4Params.tones);
	osc4Params.pan = getValue(proc.osc4Params.pan);
	osc4Params.phaseShift = getValue(proc.osc4Params.phase);
	osc4Vol = getValue(proc.osc4Params.volume);
	switch(proc.osc4Params.env->getUserValueInt())
	{
	case 0:
		envs[3] = &env1;
		break;
	case 1:
		envs[3] = &env2;
		break;
	case 2:
		envs[3] = &env3;
		break;
	case 3:
		envs[3] = &env4;
		break;
	}


	float noteNum = getValue(proc.filterParams.frequency);
	noteNum += (currentlyPlayingNote.initialNote - 50) * getValue(proc.filterParams.keyTracking);

	float f = gin::getMidiNoteInHertz(noteNum);
	float maxFreq = std::min(20000.0f, float(getSampleRate() / 2));
	f = juce::jlimit(4.0f, maxFreq, f);

	float q = gin::Q / (1.0f - (getValue(proc.filterParams.resonance) / 100.0f) * 0.99f);

	switch (int(proc.filterParams.type->getUserValue()))
	{
	case 0:
		filter.setType(gin::Filter::lowpass);
		filter.setSlope(gin::Filter::db12);
		break;
	case 1:
		filter.setType(gin::Filter::lowpass);
		filter.setSlope(gin::Filter::db24);
		break;
	case 2:
		filter.setType(gin::Filter::highpass);
		filter.setSlope(gin::Filter::db12);
		break;
	case 3:
		filter.setType(gin::Filter::highpass);
		filter.setSlope(gin::Filter::db24);
		break;
	case 4:
		filter.setType(gin::Filter::bandpass);
		filter.setSlope(gin::Filter::db12);
		break;
	case 5:
		filter.setType(gin::Filter::bandpass);
		filter.setSlope(gin::Filter::db24);
		break;
	case 6:
		filter.setType(gin::Filter::notch);
		filter.setSlope(gin::Filter::db12);
		break;
	case 7:
		filter.setType(gin::Filter::notch);
		filter.setSlope(gin::Filter::db24);
		break;
	}

	filter.setParams(f, q);
	
	gin::LFO::Parameters params;
	float freq = 0;
	
	// lfo 1
    if (proc.lfo1Params.sync->getUserValue() > 0.0f)
		freq = 1.0f / gin::NoteDuration::getNoteDurations()[size_t(proc.lfo1Params.beat->getUserValue())].toSeconds(proc.playhead);
	else
		freq = getValue(proc.lfo1Params.rate);
		params.waveShape = (gin::LFO::WaveShape) int(proc.lfo1Params.wave->getUserValue());
	params.frequency = freq;
	params.phase = getValue(proc.lfo1Params.phase);
	params.offset = getValue(proc.lfo1Params.offset);
	params.depth = getValue(proc.lfo1Params.depth);
	params.delay = getValue(proc.lfo1Params.delay);
	params.fade = getValue(proc.lfo1Params.fade);
	lfo1.setParameters(params);
	lfo1.process(blockSize);
	proc.modMatrix.setPolyValue(*this, proc.modSrcLFO1, lfo1.getOutput());

	// lfo 2
	if (proc.lfo2Params.sync->getUserValue() > 0.0f)
		freq = 1.0f / gin::NoteDuration::getNoteDurations()[size_t(proc.lfo2Params.beat->getUserValue())].toSeconds(proc.playhead);
	else
		freq = getValue(proc.lfo2Params.rate);
	params.waveShape = (gin::LFO::WaveShape) int(proc.lfo2Params.wave->getUserValue());
	params.frequency = freq;
	params.phase = getValue(proc.lfo2Params.phase);
	params.offset = getValue(proc.lfo2Params.offset);
	params.depth = getValue(proc.lfo2Params.depth);
	params.delay = getValue(proc.lfo2Params.delay);
	params.fade = getValue(proc.lfo2Params.fade);
	lfo2.setParameters(params);
	lfo2.process(blockSize);
	proc.modMatrix.setPolyValue(*this, proc.modSrcLFO2, lfo2.getOutput());

	// lfo 3
	if (proc.lfo3Params.sync->getUserValue() > 0.0f)
		freq = 1.0f / gin::NoteDuration::getNoteDurations()[size_t(proc.lfo3Params.beat->getUserValue())].toSeconds(proc.playhead);
	else
		freq = getValue(proc.lfo3Params.rate);
	params.waveShape = (gin::LFO::WaveShape) int(proc.lfo3Params.wave->getUserValue());
	params.frequency = freq;
	params.phase = getValue(proc.lfo3Params.phase);
	params.offset = getValue(proc.lfo3Params.offset);
	params.depth = getValue(proc.lfo3Params.depth);
	params.delay = getValue(proc.lfo3Params.delay);
	params.fade = getValue(proc.lfo3Params.fade);
	lfo3.setParameters(params);
	lfo3.process(blockSize);
	proc.modMatrix.setPolyValue(*this, proc.modSrcLFO3, lfo3.getOutput());

	// lfo 4
	if (proc.lfo4Params.sync->getUserValue() > 0.0f)
		freq = 1.0f / gin::NoteDuration::getNoteDurations()[size_t(proc.lfo4Params.beat->getUserValue())].toSeconds(proc.playhead);
	else
		freq = getValue(proc.lfo4Params.rate);
	params.waveShape = (gin::LFO::WaveShape) int(proc.lfo4Params.wave->getUserValue());
	params.frequency = freq;
	params.phase = getValue(proc.lfo4Params.phase);
	params.offset = getValue(proc.lfo4Params.offset);
	params.depth = getValue(proc.lfo4Params.depth);
	params.delay = getValue(proc.lfo4Params.delay);
	params.fade = getValue(proc.lfo4Params.fade);
	lfo4.setParameters(params);
	lfo4.process(blockSize);
	proc.modMatrix.setPolyValue(*this, proc.modSrcLFO4, lfo4.getOutput());


    Envelope::Params p;
	p.attackTimeMs = getValue(proc.env1Params.attack);
	p.decayTimeMs = getValue(proc.env1Params.decay);
	p.sustainLevel = getValue(proc.env1Params.sustain);
	p.releaseTimeMs = fastKill ? 0.01f : getValue(proc.env1Params.release);
	p.aCurve = getValue(proc.env1Params.acurve);
	p.dRCurve = getValue(proc.env1Params.drcurve);
	int mode = proc.env1Params.syncrepeat->getUserValueInt();
	p.sync = (!(mode == 0)); p.repeat = (!(mode == 0));
	if (mode == 1) {
		p.sync = true;
		p.syncduration = gin::NoteDuration::getNoteDurations()[size_t(proc.env1Params.duration->getUserValue())].toSeconds(proc.playhead);
	}
	if (mode == 2) {
		p.sync = true;
		p.syncduration = getValue(proc.env1Params.time);
	}
	env1.setParameters(p);


	p.attackTimeMs = getValue(proc.env2Params.attack);
	p.decayTimeMs = getValue(proc.env2Params.decay);
	p.sustainLevel = getValue(proc.env2Params.sustain);
	p.releaseTimeMs = fastKill ? 0.01f : getValue(proc.env2Params.release);
	p.aCurve = getValue(proc.env2Params.acurve);
	p.dRCurve = getValue(proc.env2Params.drcurve);
	mode = proc.env2Params.syncrepeat->getUserValueInt();
	p.sync = (mode != 0); p.repeat = (mode != 0);
	if (mode == 1) {
		p.sync = true;
		p.syncduration = gin::NoteDuration::getNoteDurations()[size_t(proc.env2Params.duration->getUserValue())].toSeconds(proc.playhead);
	}
	if (mode == 2) {
		p.sync = true;
		p.syncduration = getValue(proc.env2Params.time);
	}
	env2.setParameters(p);


	p.attackTimeMs = getValue(proc.env3Params.attack);
	p.decayTimeMs = getValue(proc.env3Params.decay);
	p.sustainLevel = getValue(proc.env3Params.sustain);
	p.releaseTimeMs = fastKill ? 0.01f : getValue(proc.env3Params.release);
	p.aCurve = getValue(proc.env3Params.acurve);
	p.dRCurve = getValue(proc.env3Params.drcurve);
	mode = proc.env3Params.syncrepeat->getUserValueInt();
	p.sync = (!(mode == 0)); p.repeat = (!(mode == 0));
	if (mode == 1) {
		p.sync = true;
		p.syncduration = gin::NoteDuration::getNoteDurations()[size_t(proc.env3Params.duration->getUserValue())].toSeconds(proc.playhead);
	}
	if (mode == 2) {
		p.sync = true;
		p.syncduration = getValue(proc.env3Params.time);
	}
	env3.setParameters(p);

	p.attackTimeMs = getValue(proc.env4Params.attack);
	p.decayTimeMs = getValue(proc.env4Params.decay);
	p.sustainLevel = getValue(proc.env4Params.sustain);
	p.releaseTimeMs = fastKill ? 0.01f : getValue(proc.env4Params.release);
	p.aCurve = getValue(proc.env4Params.acurve);
	p.dRCurve = getValue(proc.env4Params.drcurve);
	mode = proc.env4Params.syncrepeat->getUserValueInt();
	p.sync = (!(mode == 0)); p.repeat = (!(mode == 0));
	if (mode == 1) {
		p.sync = true;
		p.syncduration = gin::NoteDuration::getNoteDurations()[size_t(proc.env4Params.duration->getUserValue())].toSeconds(proc.playhead);
	}
	if (mode == 2) {
		p.sync = true;
		p.syncduration = getValue(proc.env4Params.time);
	}
	env4.setParameters(p);

	proc.modMatrix.setPolyValue(*this, proc.modSrcEnv1, env1.getOutput());
	proc.modMatrix.setPolyValue(*this, proc.modSrcEnv2, env2.getOutput());
	proc.modMatrix.setPolyValue(*this, proc.modSrcEnv3, env3.getOutput());
	proc.modMatrix.setPolyValue(*this, proc.modSrcEnv4, env4.getOutput());

	noteSmoother.process(blockSize);

	if (proc.mseg1Params.sync->isOn()) {
		mseg1Params.frequency = 1 / gin::NoteDuration::getNoteDurations()[size_t(getValue(proc.mseg1Params.beat))].toSeconds(proc.playhead);
	}
	else {
		mseg1Params.frequency = getValue(proc.mseg1Params.rate);
	}
	mseg1Params.depth = getValue(proc.mseg1Params.depth); //proc.mseg1Params.depth->getUserValue();
	mseg1Params.offset = getValue(proc.mseg1Params.offset);
    mseg1Params.loop = proc.mseg1Params.loop->isOn();

	if (proc.mseg2Params.sync->isOn()) {
		mseg2Params.frequency = 1 / gin::NoteDuration::getNoteDurations()[size_t(getValue(proc.mseg2Params.beat))].toSeconds(proc.playhead);
	}
	else {
		mseg2Params.frequency = getValue(proc.mseg2Params.rate);
	}
	mseg2Params.depth = getValue(proc.mseg2Params.depth);
	mseg2Params.offset = getValue(proc.mseg2Params.offset);
    mseg2Params.loop = proc.mseg2Params.loop->isOn();

	if (proc.mseg3Params.sync->isOn()) {
		mseg3Params.frequency = 1 / gin::NoteDuration::getNoteDurations()[size_t(getValue(proc.mseg3Params.beat))].toSeconds(proc.playhead);
	}
	else {
		mseg3Params.frequency = getValue(proc.mseg3Params.rate);
	}
	mseg3Params.depth = getValue(proc.mseg3Params.depth); //proc.mseg3Params.depth->getUserValue();
	mseg3Params.offset = getValue(proc.mseg3Params.offset);
    mseg3Params.loop = proc.mseg3Params.loop->isOn();

	if (proc.mseg4Params.sync->isOn()) {
		mseg4Params.frequency = 1 / gin::NoteDuration::getNoteDurations()[size_t(getValue(proc.mseg4Params.beat))].toSeconds(proc.playhead);
	}
	else {
		mseg4Params.frequency = getValue(proc.mseg4Params.rate);
	}
	mseg4Params.depth = getValue(proc.mseg4Params.depth);
	mseg4Params.offset = getValue(proc.mseg4Params.offset);
	mseg4Params.loop = proc.mseg4Params.loop->isOn();

	mseg1.setParameters(mseg1Params);
	mseg2.setParameters(mseg2Params);
	mseg3.setParameters(mseg3Params);
	mseg4.setParameters(mseg4Params);

	mseg1.process(blockSize);
	mseg2.process(blockSize);
	mseg3.process(blockSize);
	mseg4.process(blockSize);

	proc.modMatrix.setPolyValue(*this, proc.modSrcMSEG1, mseg1.getOutput());
	proc.modMatrix.setPolyValue(*this, proc.modSrcMSEG2, mseg2.getOutput());
	proc.modMatrix.setPolyValue(*this, proc.modSrcMSEG3, mseg3.getOutput());
	proc.modMatrix.setPolyValue(*this, proc.modSrcMSEG4, mseg4.getOutput());
}

bool SynthVoice::isVoiceActive()
{
	return isActive();
}

float SynthVoice::getFilterCutoffNormalized()
{
	float freq = filter.getFrequency();
	auto range = proc.filterParams.frequency->getUserRange();
	return range.convertTo0to1(juce::jlimit(range.start, range.end, gin::getMidiNoteFromHertz(freq)));
}

float SynthVoice::getMSEG1Phase()
{
	return mseg1.getCurrentPhase();
}

float SynthVoice::getMSEG2Phase()
{
	return mseg2.getCurrentPhase();
}

float SynthVoice::getMSEG3Phase()
{
	return mseg3.getCurrentPhase();
}

float SynthVoice::getMSEG4Phase()
{
	return mseg4.getCurrentPhase();
}

gin::Wave SynthVoice::waveForChoice(int choice)
{
	switch (choice)
	{
	case 0:
		return gin::Wave::sine;
	case 1:
		return gin::Wave::triangle;
	case 2:
		return gin::Wave::square;
	case 3:
		return gin::Wave::sawUp;
	case 4:
		return gin::Wave::pinkNoise;
	case 5:
		return gin::Wave::whiteNoise;
	default:
		return gin::Wave::sine;
	}
}

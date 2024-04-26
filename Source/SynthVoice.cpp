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

#include "SynthVoice.h"
#include "PluginProcessor.h"

//==============================================================================
SynthVoice::SynthVoice(APAudioProcessor& p)
	: proc(p), mseg1(proc.mseg1Data), mseg2(proc.mseg2Data), mseg3(proc.mseg3Data), mseg4(proc.mseg4Data)
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
	gin::ScratchBuffer synthBuffer(2, numSamples);

	// we do this to advance phase, even if we have to overwrite with sidechain (rare)
	osc1.renderPositions(osc1Freq, osc1Params, osc1Positions, numSamples); 
	
	if (proc.globalParams.sidechainEnable->isOn()) { 
		// taking sidechain as the vertical component, we calculate the horizontal component
		// effectively mapping the sidechain to a semicircle
		for (int i = 0; i < numSamples; i++) {
			osc1Positions[i].yL = std::clamp(proc.sidechainSlice.getSample(0, i), -1.f, 1.f);
			osc1Positions[i].yR = std::clamp(proc.sidechainSlice.getSample(1, i), -1.f, 1.f);
			osc1Positions[i].xL = std::sqrt(1.f - osc1Positions[i].yL * osc1Positions[i].yL) * 2.f - 1.f;
			osc1Positions[i].xR = std::sqrt(1.f - osc1Positions[i].yR * osc1Positions[i].yR) * 2.f - 1.f;
		}
	}
	
	osc2.renderPositions(osc2Freq, osc2Params, osc2Positions, numSamples);
	osc3.renderPositions(osc3Freq, osc3Params, osc3Positions, numSamples);
	osc4.renderPositions(osc4Freq, osc4Params, osc4Positions, numSamples);

	// more squash = smaller k, which scales about the tangent to the deferent
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
		// normalized vectors
		float cosThetaL = (osc1Positions[i].yL - equant) /  (distanceL + .000001f); // what we want is the tangent to the orbit at this point
		float sinThetaL = -osc1Positions[i].xL / (distanceL + .000001f); // so swap x and y and negate y
		float cosThetaR = (osc1Positions[i].yR - equant) /  (distanceR + .000001f);
		float sinThetaR = -osc1Positions[i].xR / (distanceR + .000001f); // +.000001f to avoid divide by zero
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

		cosThetaL =  (osc2Positions[i].yL - equant) / (distanceL + .000001f); 
		sinThetaL = -osc2Positions[i].xL / (distanceL + .000001f); 
		cosThetaR =  (osc2Positions[i].yR - equant) / (distanceR + .000001f);
		sinThetaR = -osc2Positions[i].xR / (distanceR + .000001f); 
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
		
		cosThetaL =  (osc3Positions[i].yL - equant) / (distanceL + .000001f); 
		sinThetaL = -osc3Positions[i].xL / (distanceL + .000001f); 
		cosThetaR =  (osc3Positions[i].yR - equant) / (distanceR + .000001f);
		sinThetaR = -osc3Positions[i].xR / (distanceR + .000001f); 
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


		// ----------------------------------------
		// interpret bodies' positions by algorithm
		// ----------------------------------------
		
		// 1. get angles
        float atanAngle2L{0}, atanAngle2R{0}, atanAngle3L{0}, atanAngle3R{0}, atanAngle4L, atanAngle4R;

		atanAngle4L = FastMath<float>::fastAtan2(epi4.yL - equant, epi4.xL);
		atanAngle4R = FastMath<float>::fastAtan2(epi4.yR - equant, epi4.xR);
		if (algo == 1)
		{
			atanAngle3L = FastMath<float>::fastAtan2(epi3.yL - equant, epi3.xL);
			atanAngle3R = FastMath<float>::fastAtan2(epi3.yR - equant, epi3.xR);
		}
		if (algo == 2) {
			atanAngle2L = FastMath<float>::fastAtan2(epi2.yL - equant, epi2.xL);
			atanAngle2R = FastMath<float>::fastAtan2(epi2.yR - equant, epi2.xR);
		}
		if (algo == 3) {
			atanAngle2L = FastMath<float>::fastAtan2(epi2.yL - equant, epi2.xL);
			atanAngle2R = FastMath<float>::fastAtan2(epi2.yR - equant, epi2.xR);
			atanAngle3L = FastMath<float>::fastAtan2(epi3.yL - equant, epi3.xL);
			atanAngle3R = FastMath<float>::fastAtan2(epi3.yR - equant, epi3.xR);
		}

		// 2. generate component waveforms from angles
		float sine2L{ 0.f }, sine2R{ 0.f }, sine3L{ 0.f }, sine3R{ 0.f }, sine4L{ 0.f }, sine4R{ 0.f };
		float square2L{ 0.f }, square2R{ 0.f }, square3L{ 0.f }, square3R{ 0.f }, square4L{ 0.f }, square4R{ 0.f };
		float saw2L{ 0.f }, saw2R{ 0.f }, saw3L{ 0.f }, saw3R{ 0.f }, saw4L{ 0.f }, saw4R{ 0.f };

		sine4L = FastMath<float>::minimaxSin(atanAngle4L);
		sine4R = FastMath<float>::minimaxSin(atanAngle4R);
		square4L = (atanAngle4L > 0.f) ? 1.0f : -1.0f;
		square4R = (atanAngle4R > 0.f) ? 1.0f : -1.0f;
		saw4L = (atanAngle4L * (float)inv_pi) * 2.0f - 1.0f;
		saw4R = (atanAngle4R * (float)inv_pi) * 2.0f - 1.0f;
		if (algo == 1) {
			sine3L = FastMath<float>::minimaxSin(atanAngle3L);
			sine3R = FastMath<float>::minimaxSin(atanAngle3R);
			square3L = (atanAngle3L > 0.f) ? 1.0f : -1.0f;
			square3R = (atanAngle3R > 0.f) ? 1.0f : -1.0f;
			saw3L = (atanAngle3L * (float)inv_pi) * 2.0f - 1.0f;
			saw3R = (atanAngle3R * (float)inv_pi) * 2.0f - 1.0f;
		}
		if (algo == 2) {
			sine2L = FastMath<float>::minimaxSin(atanAngle2L);
			sine2R = FastMath<float>::minimaxSin(atanAngle2R);
			square2L = (atanAngle2L > 0.f) ? 1.0f : -1.0f;
			square2R = (atanAngle2R > 0.f) ? 1.0f : -1.0f;
			saw2L = (atanAngle2L * (float)inv_pi) * 2.0f - 1.0f;
			saw2R = (atanAngle2R * (float)inv_pi) * 2.0f - 1.0f;
		}
		if (algo == 3) {
			sine2L = FastMath<float>::minimaxSin(atanAngle2L);
			sine2R = FastMath<float>::minimaxSin(atanAngle2R);
			square2L = (atanAngle2L > 0.f) ? 1.0f : -1.0f;
			square2R = (atanAngle2R > 0.f) ? 1.0f : -1.0f;
			saw2L = (atanAngle2L * (float)inv_pi) * 2.0f - 1.0f;
			saw2R = (atanAngle2R * (float)inv_pi) * 2.0f - 1.0f;
			sine3L = FastMath<float>::minimaxSin(atanAngle3L);
			sine3R = FastMath<float>::minimaxSin(atanAngle3R);
			square3L = (atanAngle3L > 0.f) ? 1.0f : -1.0f;
			square3R = (atanAngle3R > 0.f) ? 1.0f : -1.0f;
			saw3L = (atanAngle3L * (float)inv_pi) * 2.0f - 1.0f;
			saw3R = (atanAngle3R * (float)inv_pi) * 2.0f - 1.0f;
		}

		// 3. mix component waveforms by blend value
		float sample2L, sample2R, sample3L, sample3R, sample4L, sample4R;
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
		float modSample2L{ sample2L }, demodSample2L{ sample2L }, modSample2R{ sample2R }, demodSample2R{ sample2R };
		float modSample3L{ sample3L }, demodSample3L{ sample3L }, modSample3R{ sample3R }, demodSample3R{ sample3R };
		float modSample4L{ sample4L }, demodSample4L{ sample4L }, modSample4R{ sample4R }, demodSample4R{ sample4R };

		// 5. demodulate by considering not just angle, but also magnitude of planet vector
		auto atanDistance2L = (float)std::sqrt(epi2.xL * epi2.xL + (epi2.yL - equant) * (epi2.yL - equant));
		auto atanDistance2R = (float)std::sqrt(epi2.xR * epi2.xR + (epi2.yR - equant) * (epi2.yR - equant));
		auto atanDistance3L = (float)std::sqrt(epi3.xL * epi3.xL + (epi3.yL - equant) * (epi3.yL - equant));
		auto atanDistance3R = (float)std::sqrt(epi3.xR * epi3.xR + (epi3.yR - equant) * (epi3.yR - equant));
		auto atanDistance4L = (float)std::sqrt(epi4.xL * epi4.xL + (epi4.yL - equant) * (epi4.yL - equant));
		auto atanDistance4R = (float)std::sqrt(epi4.xR * epi4.xR + (epi4.yR - equant) * (epi4.yR - equant));

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
		float sampleL{ 0.f }, sampleR{ 0.f };
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

		// 7. SHIP IT OUT
		synthBuffer.setSample(0, i, sampleL);
		synthBuffer.setSample(1, i, sampleR);
	}

	// Get and apply velocity according to keytrack param
	float velocity = currentlyPlayingNote.noteOnVelocity.asUnsignedFloat();
	float ampKeyTrack = getValue(proc.globalParams.velSens);
	synthBuffer.applyGain(gin::velocityToGain(velocity, ampKeyTrack) * baseAmplitude);
	
	if (proc.filterParams.enable->isOn())
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

	osc1Params.wave = (bool)getValue(proc.osc1Params.saw) ? Wavetype::sawUp : Wavetype::sine;
	osc1Params.tones = getValue(proc.osc1Params.tones);
	osc1Params.pan = getValue(proc.osc1Params.pan);
	osc1Params.spread = getValue(proc.osc1Params.spread) / 100.0f;
	osc1Params.detune = getValue(proc.osc1Params.detune);
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

	osc2Params.wave = (bool)getValue(proc.osc2Params.saw) ? Wavetype::sawUp : Wavetype::sine;
	osc2Params.tones = getValue(proc.osc2Params.tones);
	osc2Params.pan = getValue(proc.osc2Params.pan);
	osc2Params.spread = getValue(proc.osc2Params.spread) / 100.0f;
	osc2Params.detune = getValue(proc.osc2Params.detune);
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

	osc3Params.wave = (bool)getValue(proc.osc3Params.saw) ? Wavetype::sawUp : Wavetype::sine;
	osc3Params.tones = getValue(proc.osc3Params.tones);
	osc3Params.pan = getValue(proc.osc3Params.pan);
	osc3Params.spread = getValue(proc.osc3Params.spread) / 100.0f;
	osc3Params.detune = getValue(proc.osc3Params.detune);
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

	osc4Params.wave = (bool)getValue(proc.osc4Params.saw) ? Wavetype::sawUp : Wavetype::sine;
	osc4Params.tones = getValue(proc.osc4Params.tones);
	osc4Params.pan = getValue(proc.osc4Params.pan);
	osc4Params.spread = getValue(proc.osc4Params.spread) / 100.0f;
	osc4Params.detune = getValue(proc.osc4Params.detune);
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

	if (proc.filterParams.enable->isOn())
	{
		float n = getValue(proc.filterParams.frequency);
		n += (currentlyPlayingNote.initialNote - 50) * getValue(proc.filterParams.keyTracking);

		float f = gin::getMidiNoteInHertz(n);
		float maxFreq = std::min(20000.0f, float(getSampleRate() / 2));
		f = juce::jlimit(4.0f, maxFreq, f);

		float q = gin::Q / (1.0f - (getValue(proc.filterParams.resonance) / 100.0f) * 0.99f);

		switch(int(proc.filterParams.type->getProcValue()))
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
	}

	gin::LFO::Parameters params;
	float freq = 0;
	
	// lfo 1
	if (proc.lfo1Params.sync->getProcValue() > 0.0f)
		freq = 1.0f / gin::NoteDuration::getNoteDurations()[size_t(proc.lfo1Params.beat->getProcValue())].toSeconds(proc.playhead);
	else
		freq = getValue(proc.lfo1Params.rate);
		params.waveShape = (gin::LFO::WaveShape) int(proc.lfo1Params.wave->getProcValue());
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
	if (proc.lfo2Params.sync->getProcValue() > 0.0f)
		freq = 1.0f / gin::NoteDuration::getNoteDurations()[size_t(proc.lfo2Params.beat->getProcValue())].toSeconds(proc.playhead);
	else
		freq = getValue(proc.lfo2Params.rate);
	params.waveShape = (gin::LFO::WaveShape) int(proc.lfo2Params.wave->getProcValue());
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
	if (proc.lfo3Params.sync->getProcValue() > 0.0f)
		freq = 1.0f / gin::NoteDuration::getNoteDurations()[size_t(proc.lfo3Params.beat->getProcValue())].toSeconds(proc.playhead);
	else
		freq = getValue(proc.lfo3Params.rate);
	params.waveShape = (gin::LFO::WaveShape) int(proc.lfo3Params.wave->getProcValue());
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
	if (proc.lfo4Params.sync->getProcValue() > 0.0f)
		freq = 1.0f / gin::NoteDuration::getNoteDurations()[size_t(proc.lfo4Params.beat->getProcValue())].toSeconds(proc.playhead);
	else
		freq = getValue(proc.lfo4Params.rate);
	params.waveShape = (gin::LFO::WaveShape) int(proc.lfo4Params.wave->getProcValue());
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
		p.syncduration = gin::NoteDuration::getNoteDurations()[size_t(getValue(proc.env1Params.duration))].toSeconds(proc.playhead);
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
		p.syncduration = gin::NoteDuration::getNoteDurations()[size_t(getValue(proc.env2Params.duration))].toSeconds(proc.playhead);
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
		p.syncduration = gin::NoteDuration::getNoteDurations()[size_t(getValue(proc.env3Params.duration))].toSeconds(proc.playhead);
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
		p.syncduration = gin::NoteDuration::getNoteDurations()[size_t(getValue(proc.env4Params.duration))].toSeconds(proc.playhead);
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

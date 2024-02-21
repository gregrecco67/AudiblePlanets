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
	: proc(p)
{
	filter.setNumChannels(2);
	osc1Params.voices = 4;
	osc2Params.voices = 4;
	osc3Params.voices = 4;
	osc4Params.voices = 4;
}

void SynthVoice::noteStarted()
{
    curNote = getCurrentlyPlayingNote();
	fastKill = false;
	startVoice();

	auto note = getCurrentlyPlayingNote();
	if (glideInfo.fromNote != -1 && (glideInfo.glissando || glideInfo.portamento))
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
	updateParams(0);
	snapParams();
	//
	osc1.noteOn(proc.osc1Params.phase->getUserValue());
	osc2.noteOn(proc.osc2Params.phase->getUserValue());
	osc3.noteOn(proc.osc3Params.phase->getUserValue());
	osc4.noteOn(proc.osc4Params.phase->getUserValue());


	env1.noteOn();
	env2.noteOn();
	env3.noteOn();
	env4.noteOn();

}

void SynthVoice::noteRetriggered()
{
	auto note = getCurrentlyPlayingNote();
    curNote = getCurrentlyPlayingNote();

	if (glideInfo.fromNote != -1 && (glideInfo.glissando || glideInfo.portamento))
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
}

void SynthVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
	updateParams(numSamples);


	// Run OSC
	gin::ScratchBuffer osc1SineBuffer(2, numSamples);
	gin::ScratchBuffer osc1CosineBuffer(2, numSamples);
	gin::ScratchBuffer osc2SineBuffer(2, numSamples);
	gin::ScratchBuffer osc2CosineBuffer(2, numSamples);
	gin::ScratchBuffer osc3SineBuffer(2, numSamples);
	gin::ScratchBuffer osc3CosineBuffer(2, numSamples);
	gin::ScratchBuffer osc4SineBuffer(2, numSamples);
	gin::ScratchBuffer osc4CosineBuffer(2, numSamples);
	gin::ScratchBuffer synthBuffer(2, numSamples);

	osc1.processAdding(osc1Freq, osc1Params, osc1SineBuffer, osc1CosineBuffer);
	osc2.processAdding(osc2Freq, osc2Params, osc2SineBuffer, osc2CosineBuffer);
	osc3.processAdding(osc3Freq, osc3Params, osc3SineBuffer, osc3CosineBuffer);
	osc4.processAdding(osc4Freq, osc4Params, osc4SineBuffer, osc4CosineBuffer);
	// Apply velocity
	float velocity = currentlyPlayingNote.noteOnVelocity.asUnsignedFloat();
	osc1SineBuffer.applyGain(osc1Vol);
	osc1CosineBuffer.applyGain(osc1Vol);
	osc2SineBuffer.applyGain(osc2Vol);
	osc2CosineBuffer.applyGain(osc2Vol);
	osc3SineBuffer.applyGain(osc3Vol);
	osc3CosineBuffer.applyGain(osc3Vol);
	osc4SineBuffer.applyGain(osc4Vol); 
	osc4CosineBuffer.applyGain(osc4Vol);

	// the whole enchilada
	for (int i = 0; i < numSamples; i++)
	{
        env1.getNextSample();
        env2.getNextSample();
        env3.getNextSample();
        env4.getNextSample();
        // get bodies' position by algorithm
        auto a = envs[0]->getOutput(); // load ALL FOUR env vals once per loop, then look them up
		auto b = envs[1]->getOutput(); // in an envVals array
        auto c = envs[2]->getOutput(); // 
        auto d = envs[3]->getOutput();
        
		epi1 = {
			a * osc1SineBuffer.getSample(0, i), a * osc1CosineBuffer.getSample(0, i),
			a * osc1SineBuffer.getSample(1, i), a * osc1CosineBuffer.getSample(1, i)
		};
		epi2 = {
			epi1.xL + b * osc2SineBuffer.getSample(0, i), epi1.yL + b * osc2CosineBuffer.getSample(0, i),
			epi1.xR + b * osc2SineBuffer.getSample(1, i), epi1.yR + b * osc2CosineBuffer.getSample(1, i)
		};
		if (algo == 0)
		{
			epi3 = {
				epi2.xL + c * osc3SineBuffer.getSample(0, i), epi2.yL + c * osc3CosineBuffer.getSample(0, i),
				epi2.xR + c * osc3SineBuffer.getSample(1, i), epi2.yR + c * osc3CosineBuffer.getSample(1, i)
			};
			epi4 = {
				epi3.xL + d * osc4SineBuffer.getSample(0, i), epi3.yL + d * osc4CosineBuffer.getSample(0, i),
				epi3.xR + d * osc4SineBuffer.getSample(1, i), epi3.yR + d * osc4CosineBuffer.getSample(1, i)
			};
		}
		if (algo == 1) {
			epi3 = {
				epi2.xL + c * osc3SineBuffer.getSample(0, i), epi2.yL + c * osc3CosineBuffer.getSample(0, i),
				epi2.xR + c * osc3SineBuffer.getSample(1, i), epi2.yR + c * osc3CosineBuffer.getSample(1, i)
			};
			epi4 = {
				epi2.xL + d * osc4SineBuffer.getSample(0, i), epi2.yL + d * osc4CosineBuffer.getSample(0, i),
				epi2.xR + d * osc4SineBuffer.getSample(1, i), epi2.yR + d * osc4CosineBuffer.getSample(1, i)
			};
		}
		if (algo == 2) {
			epi3 = {
				epi1.xL + c * osc3SineBuffer.getSample(0, i), epi1.yL + c * osc3CosineBuffer.getSample(0, i),
				epi1.xR + c * osc3SineBuffer.getSample(1, i), epi1.yR + c * osc3CosineBuffer.getSample(1, i)
			};
			epi4 = {
				epi3.xL + d * osc4SineBuffer.getSample(0, i), epi3.yL + d * osc4CosineBuffer.getSample(0, i),
				epi3.xR + d * osc4SineBuffer.getSample(1, i), epi3.yR + d * osc4CosineBuffer.getSample(1, i)
			};
		}
		if (algo == 3) {
			epi3 = {
				epi1.xL + c * osc3SineBuffer.getSample(0, i), epi1.yL + c * osc3CosineBuffer.getSample(0, i),
				epi1.xR + c * osc3SineBuffer.getSample(1, i), epi1.yR + c * osc3CosineBuffer.getSample(1, i)
			};
			epi4 = {
				epi1.xL + d * osc4SineBuffer.getSample(0, i), epi1.yL + d * osc4CosineBuffer.getSample(0, i),
				epi1.xR + d * osc4SineBuffer.getSample(1, i), epi1.yR + d * osc4CosineBuffer.getSample(1, i)
			};
		}


		// interpret bodies' positions by algorithm
		// 1. get angles
        float atanAngle2L{0}, atanAngle2R{0}, atanAngle3L{0}, atanAngle3R{0}, atanAngle4L, atanAngle4R;

		atanAngle4L = FastMath<float>::fastAtan2(epi4.yL + equant, epi4.xL);
		atanAngle4R = FastMath<float>::fastAtan2(epi4.yR + equant, epi4.xR);
		if (algo == 1)
		{
			atanAngle3L = FastMath<float>::fastAtan2(epi3.yL + equant, epi3.xL);
			atanAngle3R = FastMath<float>::fastAtan2(epi3.yR + equant, epi3.xR);
		}
		if (algo == 2) {
			atanAngle2L = FastMath<float>::fastAtan2(epi2.yL + equant, epi2.xL);
			atanAngle2R = FastMath<float>::fastAtan2(epi2.yR + equant, epi2.xR);
		}
		if (algo == 3) {
			atanAngle2L = FastMath<float>::fastAtan2(epi2.yL + equant, epi2.xL);
			atanAngle2R = FastMath<float>::fastAtan2(epi2.yR + equant, epi2.xR);
			atanAngle3L = FastMath<float>::fastAtan2(epi3.yL + equant, epi3.xL);
			atanAngle3R = FastMath<float>::fastAtan2(epi3.yR + equant, epi3.xR);
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
		auto atanDistance2L = (float)std::sqrt(epi2.xL * epi2.xL + (epi2.yL + equant) * (epi2.yL + equant));
		auto atanDistance2R = (float)std::sqrt(epi2.xR * epi2.xR + (epi2.yR + equant) * (epi2.yR + equant));
		auto atanDistance3L = (float)std::sqrt(epi3.xL * epi3.xL + (epi3.yL + equant) * (epi3.yL + equant));
		auto atanDistance3R = (float)std::sqrt(epi3.xR * epi3.xR + (epi3.yR + equant) * (epi3.yR + equant));
		auto atanDistance4L = (float)std::sqrt(epi4.xL * epi4.xL + (epi4.yL + equant) * (epi4.yL + equant));
		auto atanDistance4R = (float)std::sqrt(epi4.xR * epi4.xR + (epi4.yR + equant) * (epi4.yR + equant));

		// distance can be > 1, so scale it down a bit
		demodSample2L *= atanDistance2L * 0.75f;
		demodSample2R *= atanDistance2R * 0.75f;
		demodSample3L *= atanDistance3L * 0.75f;
		demodSample3R *= atanDistance3R * 0.75f;
		demodSample4L *= atanDistance4L * 0.75f;
		demodSample4R *= atanDistance4R * 0.75f;

		// original recipe (before we added demodmix)
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
			sampleL = ((modSample2L + modSample3L + modSample4L) * 0.333f * (1.0f - demodmix) + (demodSample2L + modSample3L + demodSample4L) * 0.333f * demodmix);
			sampleR = ((modSample2R + modSample3R + modSample4R) * 0.333f * (1.0f - demodmix) + (demodSample2R + demodSample3L + demodSample4R) * 0.333f * demodmix);
		}

		// 7. SHIP IT OUT
		synthBuffer.setSample(0, i, sampleL);
		synthBuffer.setSample(1, i, sampleR);
	}

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

	auto note = getCurrentlyPlayingNote();

	proc.modMatrix.setPolyValue(*this, proc.modSrcNote, note.initialNote / 127.0f);

	currentMidiNote = noteSmoother.getCurrentValue() * 127.0f;
	if (glideInfo.glissando) currentMidiNote = (float)juce::roundToInt(currentMidiNote);

    float baseFreq =  (float)gin::getMidiNoteInHertz(currentMidiNote + note.totalPitchbendInSemitones);
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

	osc1Params.wave = (bool)getValue(proc.osc1Params.saw) ? Wave::sawUp : Wave::sine;
	osc1Params.tones = getValue(proc.osc1Params.tones);
	osc1Params.pan = getValue(proc.osc1Params.pan);
	osc1Params.spread = getValue(proc.osc1Params.spread) / 100.0f;
	osc1Params.detune = getValue(proc.osc1Params.detune);
	osc1Params.phaseShift = getValue(proc.osc1Params.phase);
	osc1Vol = getValue(proc.osc1Params.volume);
	switch((int)getValue(proc.osc1Params.env))
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

	osc2Params.wave = (bool)getValue(proc.osc2Params.saw) ? Wave::sawUp : Wave::sine;
	osc2Params.tones = getValue(proc.osc2Params.tones);
	osc2Params.pan = getValue(proc.osc2Params.pan);
	osc2Params.spread = getValue(proc.osc2Params.spread) / 100.0f;
	osc2Params.detune = getValue(proc.osc2Params.detune);
	osc2Params.phaseShift = getValue(proc.osc2Params.phase);
	osc2Vol = getValue(proc.osc2Params.volume);
	switch((int)getValue(proc.osc2Params.env))
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

	osc3Params.wave = (bool)getValue(proc.osc3Params.saw) ? Wave::sawUp : Wave::sine;
	osc3Params.tones = getValue(proc.osc3Params.tones);
	osc3Params.pan = getValue(proc.osc3Params.pan);
	osc3Params.spread = getValue(proc.osc3Params.spread) / 100.0f;
	osc3Params.detune = getValue(proc.osc3Params.detune);
	osc3Params.phaseShift = getValue(proc.osc3Params.phase);
	osc3Vol = getValue(proc.osc3Params.volume);
	switch((int)getValue(proc.osc3Params.env))
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

	osc4Params.wave = (bool)getValue(proc.osc4Params.saw) ? Wave::sawUp : Wave::sine;
	osc4Params.tones = getValue(proc.osc4Params.tones);
	osc4Params.pan = getValue(proc.osc4Params.pan);
	osc4Params.spread = getValue(proc.osc4Params.spread) / 100.0f;
	osc4Params.detune = getValue(proc.osc4Params.detune);
	osc4Params.phaseShift = getValue(proc.osc4Params.phase);
	osc4Vol = getValue(proc.osc4Params.volume);
	switch((int)getValue(proc.osc4Params.env))
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

	ampKeyTrack = getValue(proc.globalParams.velSens);

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
	int mode = (int)getValue(proc.env1Params.syncrepeat);
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
	mode = (int)getValue(proc.env2Params.syncrepeat);
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
	mode = (int)getValue(proc.env3Params.syncrepeat);
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
	mode = (int)getValue(proc.env4Params.syncrepeat);
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

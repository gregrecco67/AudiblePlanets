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

	updateParams(0);
	snapParams();
	updateParams(0);
	snapParams();
	//
	osc1.noteOn();
	osc2.noteOn();
	osc3.noteOn();
	osc4.noteOn();


	env1.reset();
	env1.noteOn();
	env2.reset();
	env2.noteOn();
	env3.reset();
	env3.noteOn();
	env4.reset();
	env4.noteOn();

}

void SynthVoice::noteRetriggered()
{
	auto note = getCurrentlyPlayingNote();

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

	if (!allowTailOff)
	{
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
	proc.modMatrix.setPolyValue(*this, proc.modSrcTimbre, note.initialTimbre.asUnsignedFloat());
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
	env1.setSampleRate(newRate);
	env2.setSampleRate(newRate);
	env3.setSampleRate(newRate);
	env4.setSampleRate(newRate);
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
	float velocity = currentlyPlayingNote.noteOnVelocity.asUnsignedFloat() * baseAmplitude;
	//osc1SineBuffer.applyGain(gin::velocityToGain(velocity, ampKeyTrack));
	//osc1CosineBuffer.applyGain(gin::velocityToGain(velocity, ampKeyTrack));
	//osc2SineBuffer.applyGain(gin::velocityToGain(velocity, ampKeyTrack));
	//osc2CosineBuffer.applyGain(gin::velocityToGain(velocity, ampKeyTrack));
	//osc3SineBuffer.applyGain(gin::velocityToGain(velocity, ampKeyTrack));
	//osc3CosineBuffer.applyGain(gin::velocityToGain(velocity, ampKeyTrack));
	//osc4SineBuffer.applyGain(gin::velocityToGain(velocity, ampKeyTrack));
	//osc4CosineBuffer.applyGain(gin::velocityToGain(velocity, ampKeyTrack));


	// Run ADSR
	envs[0]->processMultiplying(osc1SineBuffer);
	envs[0]->processMultiplying(osc1CosineBuffer);
	envs[1]->processMultiplying(osc2SineBuffer);
	envs[1]->processMultiplying(osc2CosineBuffer);
	envs[2]->processMultiplying(osc3SineBuffer);
	envs[2]->processMultiplying(osc3CosineBuffer);
	envs[3]->processMultiplying(osc4SineBuffer);
	envs[3]->processMultiplying(osc4CosineBuffer);

	// the whole enchilada
	for (int i = 0; i < numSamples; i++)
	{
		// get bodies' position by algorithm

		epi1 = {
			osc1SineBuffer.getSample(0, i), osc1CosineBuffer.getSample(0, i),
			osc1SineBuffer.getSample(1, i), osc1CosineBuffer.getSample(1, i)
		};
		epi2 = {
			epi1.xL + osc2SineBuffer.getSample(0, i), epi1.yL + osc2CosineBuffer.getSample(0, i),
			epi1.xR + osc2SineBuffer.getSample(1, i), epi1.yR + osc2CosineBuffer.getSample(1, i)
		};
		if (algo == 0)
		{
			epi3 = {
				epi2.xL + osc3SineBuffer.getSample(0, i), epi2.yL + osc3CosineBuffer.getSample(0, i),
				epi2.xR + osc3SineBuffer.getSample(1, i), epi2.yR + osc3CosineBuffer.getSample(1, i)
			};
			epi4 = {
				epi3.xL + osc4SineBuffer.getSample(0, i), epi3.yL + osc4CosineBuffer.getSample(0, i),
				epi3.xR + osc4SineBuffer.getSample(1, i), epi3.yR + osc4CosineBuffer.getSample(1, i)
			};
		}
		if (algo == 1) {
			epi3 = {
				epi2.xL + osc3SineBuffer.getSample(0, i), epi2.yL + osc3CosineBuffer.getSample(0, i),
				epi2.xR + osc3SineBuffer.getSample(1, i), epi2.yR + osc3CosineBuffer.getSample(1, i)
			};
			epi4 = {
				epi2.xL + osc4SineBuffer.getSample(0, i), epi2.yL + osc4CosineBuffer.getSample(0, i),
				epi2.xR + osc4SineBuffer.getSample(1, i), epi2.yR + osc4CosineBuffer.getSample(1, i)
			};
		}
		if (algo == 2) {
			epi3 = {
				epi1.xL + osc3SineBuffer.getSample(0, i), epi1.yL + osc3CosineBuffer.getSample(0, i),
				epi1.xR + osc3SineBuffer.getSample(1, i), epi1.yR + osc3CosineBuffer.getSample(1, i)
			};
			epi4 = {
				epi3.xL + osc4SineBuffer.getSample(0, i), epi3.yL + osc4CosineBuffer.getSample(0, i),
				epi3.xR + osc4SineBuffer.getSample(1, i), epi3.yR + osc4CosineBuffer.getSample(1, i)
			};
		}
		if (algo == 3) {
			epi3 = {
				epi1.xL + osc3SineBuffer.getSample(0, i), epi1.yL + osc3CosineBuffer.getSample(0, i),
				epi1.xR + osc3SineBuffer.getSample(1, i), epi1.yR + osc3CosineBuffer.getSample(1, i)
			};
			epi4 = {
				epi1.xL + osc4SineBuffer.getSample(0, i), epi1.yL + osc4CosineBuffer.getSample(0, i),
				epi1.xR + osc4SineBuffer.getSample(1, i), epi1.yR + osc4CosineBuffer.getSample(1, i)
			};
		}


		// interpret bodies' positions by algorithm
		// 1. get angles
		float atanAngle2L, atanAngle2R, atanAngle3L, atanAngle3R, atanAngle4L, atanAngle4R;

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
		saw4L = (atanAngle4L * inv_pi) * 2.0f - 1.0f;
		saw4R = (atanAngle4R * inv_pi) * 2.0f - 1.0f;
		if (algo == 1) {
			sine3L = FastMath<float>::minimaxSin(atanAngle3L);
			sine3R = FastMath<float>::minimaxSin(atanAngle3R);
			square3L = (atanAngle3L > 0.f) ? 1.0f : -1.0f;
			square3R = (atanAngle3R > 0.f) ? 1.0f : -1.0f;
			saw3L = (atanAngle3L * inv_pi) * 2.0f - 1.0f;
			saw3R = (atanAngle3R * inv_pi) * 2.0f - 1.0f;
		}
		if (algo == 2) {
			sine2L = FastMath<float>::minimaxSin(atanAngle2L);
			sine2R = FastMath<float>::minimaxSin(atanAngle2R);
			square2L = (atanAngle2L > 0.f) ? 1.0f : -1.0f;
			square2R = (atanAngle2R > 0.f) ? 1.0f : -1.0f;
			saw2L = (atanAngle2L * inv_pi) * 2.0f - 1.0f;
			saw2R = (atanAngle2R * inv_pi) * 2.0f - 1.0f;
		}
		if (algo == 3) {
			sine2L = FastMath<float>::minimaxSin(atanAngle2L);
			sine2R = FastMath<float>::minimaxSin(atanAngle2R);
			square2L = (atanAngle2L > 0.f) ? 1.0f : -1.0f;
			square2R = (atanAngle2R > 0.f) ? 1.0f : -1.0f;
			saw2L = (atanAngle2L * inv_pi) * 2.0f - 1.0f;
			saw2R = (atanAngle2R * inv_pi) * 2.0f - 1.0f;
			sine3L = FastMath<float>::minimaxSin(atanAngle3L);
			sine3R = FastMath<float>::minimaxSin(atanAngle3R);
			square3L = (atanAngle3L > 0.f) ? 1.0f : -1.0f;
			square3R = (atanAngle3R > 0.f) ? 1.0f : -1.0f;
			saw3L = (atanAngle3L * inv_pi) * 2.0f - 1.0f;
			saw3R = (atanAngle3R * inv_pi) * 2.0f - 1.0f;
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

		// demodulate by considering not just angle, but also magnitude of planet vector
		// this will be the same for both channels, so we can calculate it once
		auto atanDistance2L = (float)std::sqrt(epi2.xL * epi2.xL + (epi2.yL + equant) * (epi2.yL + equant));
		auto atanDistance2R = (float)std::sqrt(epi2.xR * epi2.xR + (epi2.yR + equant) * (epi2.yR + equant));
		auto atanDistance3L = (float)std::sqrt(epi3.xL * epi3.xL + (epi3.yL + equant) * (epi3.yL + equant));
		auto atanDistance3R = (float)std::sqrt(epi3.xR * epi3.xR + (epi3.yR + equant) * (epi3.yR + equant));
		auto atanDistance4L = (float)std::sqrt(epi4.xL * epi4.xL + (epi4.yL + equant) * (epi4.yL + equant));
		auto atanDistance4R = (float)std::sqrt(epi4.xR * epi4.xR + (epi4.yR + equant) * (epi4.yR + equant));

		// trying out a  volume-taming factor
		demodSample2L *= atanDistance2L * 0.75f;
		demodSample2R *= atanDistance2R * 0.75f;
		demodSample3L *= atanDistance3L * 0.75f;
		demodSample3R *= atanDistance3R * 0.75f;
		demodSample4L *= atanDistance4L * 0.75f;
		demodSample4R *= atanDistance4R * 0.75f;

		// original recipe
		modSample2L *= envs[1]->getOutput();
		modSample2R *= envs[1]->getOutput();
		modSample3L *= envs[2]->getOutput();
		modSample3R *= envs[2]->getOutput();
		modSample4L *= envs[3]->getOutput();
		modSample4R *= envs[3]->getOutput();

		// mix by demodmix AND ALGO
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

		// SHIP IT OUT
		synthBuffer.setSample(0, i, sampleL);
		synthBuffer.setSample(1, i, sampleR);
	}

	synthBuffer.applyGain(gin::velocityToGain(velocity, ampKeyTrack));
	
	// Apply filter -- we'll do this after orbital processing
	if (proc.filterParams.enable->isOn())
		filter.process(synthBuffer);

	if (env1.getState() == gin::AnalogADSR::State::idle)
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
	currentMidiNote += float(note.totalPitchbendInSemitones);
	//currentMidiNote += getValue(proc.osc1Params.coarse) + getValue(proc.osc1Params.fine);

	osc1Freq = gin::getMidiNoteInHertz(currentMidiNote) * (getValue(proc.osc1Params.coarse) + getValue(proc.osc1Params.fine));
	osc2Freq = gin::getMidiNoteInHertz(currentMidiNote) * (getValue(proc.osc2Params.coarse) + getValue(proc.osc2Params.fine));
	osc3Freq = gin::getMidiNoteInHertz(currentMidiNote) * (getValue(proc.osc3Params.coarse) + getValue(proc.osc3Params.fine));
	osc4Freq = gin::getMidiNoteInHertz(currentMidiNote) * (getValue(proc.osc4Params.coarse) + getValue(proc.osc4Params.fine));

	osc1Params.wave = getValue(proc.osc1Params.saw) ? Wave::sawUp : Wave::cosine;
	osc1Params.tones = getValue(proc.osc1Params.tones);
	osc1Params.pan = getValue(proc.osc1Params.pan);
	osc1Params.spread = getValue(proc.osc1Params.spread) / 100.0f;
	osc1Params.detune = getValue(proc.osc1Params.detune);
	osc1Params.gain = getValue(proc.osc1Params.radius);
	switch ((int)getValue(proc.osc1Params.env))
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

	osc2Params.wave = getValue(proc.osc2Params.saw) ? Wave::sawUp : Wave::sine;
	osc2Params.tones = getValue(proc.osc2Params.tones);
	osc2Params.pan = getValue(proc.osc2Params.pan);
	osc2Params.spread = getValue(proc.osc2Params.spread) / 100.0f;
	osc2Params.detune = getValue(proc.osc2Params.detune);
	osc2Params.gain = getValue(proc.osc2Params.radius);
	switch ((int)getValue(proc.osc2Params.env))
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

	osc3Params.wave = getValue(proc.osc3Params.saw) ? Wave::sawUp : Wave::sine;
	osc3Params.tones = getValue(proc.osc3Params.tones);
	osc3Params.pan = getValue(proc.osc3Params.pan);
	osc3Params.spread = getValue(proc.osc3Params.spread) / 100.0f;
	osc3Params.detune = getValue(proc.osc3Params.detune);
	osc3Params.gain = getValue(proc.osc3Params.radius);
	switch ((int)getValue(proc.osc3Params.env))
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

	osc4Params.wave = getValue(proc.osc4Params.saw) ? Wave::sawUp : Wave::sine;
	osc4Params.tones = getValue(proc.osc4Params.tones);
	osc4Params.pan = getValue(proc.osc4Params.pan);
	osc4Params.spread = getValue(proc.osc4Params.spread) / 100.0f;
	osc4Params.detune = getValue(proc.osc4Params.detune);
	osc4Params.gain = getValue(proc.osc4Params.radius);
	switch ((int)getValue(proc.osc4Params.env))
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



	//ampKeyTrack = getValue (proc.env1Params.velocityTracking);

	if (proc.filterParams.enable->isOn())
	{
		float n = getValue(proc.filterParams.frequency);
		n += (currentlyPlayingNote.initialNote - 60) * getValue(proc.filterParams.keyTracking);

		float f = gin::getMidiNoteInHertz(n);
		float maxFreq = std::min(20000.0f, float(getSampleRate() / 2));
		f = juce::jlimit(4.0f, maxFreq, f);

		float q = gin::Q / (1.0f - (getValue(proc.filterParams.resonance) / 100.0f) * 0.99f);

		switch (int(proc.filterParams.type->getProcValue()))
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

	env1.setAttack(getValue(proc.env1Params.attack));
	env1.setDecay(getValue(proc.env1Params.decay));
	env1.setSustainLevel(getValue(proc.env1Params.sustain));
	env1.setRelease(fastKill ? 0.01f : getValue(proc.env1Params.release));

	env2.setAttack(getValue(proc.env2Params.attack));
	env2.setDecay(getValue(proc.env2Params.decay));
	env2.setSustainLevel(getValue(proc.env2Params.sustain));
	env2.setRelease(fastKill ? 0.01f : getValue(proc.env2Params.release));

	env3.setAttack(getValue(proc.env3Params.attack));
	env3.setDecay(getValue(proc.env3Params.decay));
	env3.setSustainLevel(getValue(proc.env3Params.sustain));
	env3.setRelease(fastKill ? 0.01f : getValue(proc.env3Params.release));

	env4.setAttack(getValue(proc.env4Params.attack));
	env4.setDecay(getValue(proc.env4Params.decay));
	env4.setSustainLevel(getValue(proc.env4Params.sustain));
	env4.setRelease(fastKill ? 0.01f : getValue(proc.env4Params.release));

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

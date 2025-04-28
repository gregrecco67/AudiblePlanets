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
#include "SynthVoice2.h"
#include "PluginProcessor.h"

inline std::array<float, 2> SynthVoice2::panWeights(const float in) { // -1 to 1
	return { std::sqrt((in + 1.f) * 0.5f), std::sqrt(1.f - ((in + 1.f) * 0.5f)) };
}

//==============================================================================
SynthVoice2::SynthVoice2(APAudioProcessor& p)
	: proc(p), mseg1(proc.mseg1Data), mseg2(proc.mseg2Data), mseg3(proc.mseg3Data), mseg4(proc.mseg4Data),
	osc1(p.analogTables), osc2(p.analogTables), osc3(p.analogTables), osc4(p.analogTables)
{
	mseg1.reset();
	mseg2.reset();
	mseg3.reset();
	mseg4.reset();
	filter.setNumChannels(2);
}

void SynthVoice2::noteStarted()
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

void SynthVoice2::noteRetriggered()
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

	env1.noteOn();
	env2.noteOn();
	env3.noteOn();
	env4.noteOn();
}

void SynthVoice2::noteStopped(bool allowTailOff)
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

void SynthVoice2::notePressureChanged()
{
	auto note = getCurrentlyPlayingNote();
	proc.modMatrix.setPolyValue(*this, proc.modSrcPressure, note.pressure.asUnsignedFloat());
}

void SynthVoice2::noteTimbreChanged()
{
	auto note = getCurrentlyPlayingNote();
	proc.modMatrix.setPolyValue(*this, proc.modSrcTimbre, note.timbre.asUnsignedFloat());
}

void SynthVoice2::setCurrentSampleRate(double newRate)
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

void SynthVoice2::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
	updateParams(numSamples);

	synthBuffer.setSize(2, numSamples, false, false, true);

	// 1. fill positions vectors
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
        float distance = std::sqrt(osc1Positions[i].x * osc1Positions[i].x + (osc1Positions[i].y - equant) * (osc1Positions[i].y - equant));
        float invDistance = 1.f / (distance + .000001f);

        // normalized vectors
        float cosTheta = (osc1Positions[i].y - equant) * invDistance; // what we want is the tangent to the orbit at this point
        float sinTheta = -osc1Positions[i].x * invDistance; // so swap x and y and negate y
        float cos2Theta = cosTheta * cosTheta;
        float sin2Theta = sinTheta * sinTheta;
        
        // plug in to transform matrix
        APMatrix squash1 = {
                .a = cos2Theta + k * sin2Theta, .b = cosTheta * sinTheta * (1.0f - k),
                .c = cosTheta * sinTheta * (1.0f - k), .d = sin2Theta + k * cos2Theta
        };
        
        distance = std::sqrt(osc2Positions[i].x * osc2Positions[i].x + (osc2Positions[i].y - equant) * (osc2Positions[i].y - equant));
        invDistance = 1.f / (distance + .000001f);
        
        cosTheta =  (osc2Positions[i].y - equant) * invDistance;
        sinTheta = -osc2Positions[i].x * invDistance;
        cos2Theta = cosTheta * cosTheta;
        sin2Theta = sinTheta * sinTheta;
        
        APMatrix squash2 = {
			.a = cos2Theta + k * sin2Theta, .b = cosTheta * sinTheta * (1.0f - k),
			.c = cosTheta * sinTheta * (1.0f - k), .d = sin2Theta + k * cos2Theta
        };
        
        distance = std::sqrt(osc3Positions[i].x * osc3Positions[i].x + (osc3Positions[i].y - equant) * (osc3Positions[i].y - equant));
        invDistance = 1.f / (distance + .000001f);
        
        cosTheta =  (osc3Positions[i].y - equant) * invDistance;
        sinTheta = -osc3Positions[i].x * invDistance;
        cos2Theta = cosTheta * cosTheta;
        sin2Theta = sinTheta * sinTheta;
        
        APMatrix squash3 = {
			.a = cos2Theta + k * sin2Theta, .b = cosTheta * sinTheta * (1.0f - k),
			.c = cosTheta * sinTheta * (1.0f - k), .d = sin2Theta + k * cos2Theta
        };
        
        epi1 = osc1Positions[i] * (a * osc1Vol);
        
        // apply the squash matrix to squash secondary orbits along the tangent of the one they're orbiting
        epi2 = epi1 + ((osc2Positions[i] * squash1) * (b * osc2Vol));
        
        // get bodies 3 & 4 position by algorithm
		switch (algo) {
		case 0: // 1-2-3-(4)
			epi3 = epi2 + ((osc3Positions[i] * squash2) * (c * osc3Vol));
			epi4 = epi3 + ((osc4Positions[i] * squash3) * (d * osc4Vol));
			break;
		case 1: // 1-2-(3), 2-(4)
			epi3 = epi2 + ((osc3Positions[i] * squash2) * (c * osc3Vol));
			epi4 = epi2 + ((osc4Positions[i] * squash2) * (d * osc4Vol));
			break;
		case 2:  // 1-(2), 1-3-(4)
			epi3 = epi1 + ((osc3Positions[i] * squash1) * (c * osc3Vol));
			epi4 = epi3 + ((osc4Positions[i] * squash3) * (d * osc4Vol));
			break;
		case 3:  // 1-(2), 1-(3), 1-(4)
			epi3 = epi1 + ((osc3Positions[i] * squash1) * (c * osc3Vol));
			epi4 = epi1 + ((osc4Positions[i] * squash1) * (d * osc4Vol));
			break;
		default:
			epi3 = epi2 + ((osc3Positions[i] * squash2) * (c * osc3Vol));
			epi4 = epi3 + ((osc4Positions[i] * squash3) * (d * osc4Vol));
			break;
		}
        
        epi2xs[i] = epi2.x; epi2ys[i] = epi2.y;
		epi3xs[i] = epi3.x; epi3ys[i] = epi3.y;
		epi4xs[i] = epi4.x; epi4ys[i] = epi4.y;
    }

	auto synthBufferL = synthBuffer.getWritePointer(0);
	auto synthBufferR = synthBuffer.getWritePointer(1);

	for (int i = 0; i < std::ceil(((float)numSamples)/4.f); i++)
    {
		epi2x = { epi2xs[i * 4], epi2xs[i * 4 + 1], epi2xs[i * 4 + 2], epi2xs[i * 4 + 3] };
		epi2y = { epi2ys[i * 4], epi2ys[i * 4 + 1], epi2ys[i * 4 + 2], epi2ys[i * 4 + 3] };
		epi3x = { epi3xs[i * 4], epi3xs[i * 4 + 1], epi3xs[i * 4 + 2], epi3xs[i * 4 + 3] };
		epi3y = { epi3ys[i * 4], epi3ys[i * 4 + 1], epi3ys[i * 4 + 2], epi3ys[i * 4 + 3] };
		epi4x = { epi4xs[i * 4], epi4xs[i * 4 + 1], epi4xs[i * 4 + 2], epi4xs[i * 4 + 3] };
		epi4y = { epi4ys[i * 4], epi4ys[i * 4 + 1], epi4ys[i * 4 + 2], epi4ys[i * 4 + 3] };
		
		// ----------------------------------------
		// interpret bodies' positions by algorithm
		// ----------------------------------------

		auto dist4 = mipp::sqrt((epi4y - equant) * (epi4y - equant) + (epi4x * epi4x)) + .000001f;
		auto dist3 = mipp::sqrt((epi3y - equant) * (epi3y - equant) + (epi3x * epi3x)) + .000001f;
		auto dist2 = mipp::sqrt((epi2y - equant) * (epi2y - equant) + (epi2x * epi2x)) + .000001f;
		sine4 = (epi4y - equant) / dist4;
		cos4 =   epi4x / dist4;
		sine3 = (epi3y - equant) / dist3;
		cos3 = epi3x / dist3;
		sine2 = (epi2y - equant) / dist2;
		cos2 = epi2x / dist2;
		
		demodSample2L = (epi2y - equant) * demodVol; // adjustable balancing term
		demodSample2R = epi2x * demodVol;
		demodSample3L = (epi3y - equant) * demodVol;
		demodSample3R = epi3x * demodVol;
		demodSample4L = (epi4y - equant) * demodVol;
		demodSample4R = epi4x * demodVol;

		// since mod samples are angle-only, we need to reapply their envelope values
		sine2 *= envs[1]->getOutput();
		cos2 *= envs[1]->getOutput();
		sine3 *= envs[2]->getOutput();
		cos3 *= envs[2]->getOutput();
		sine4 *= envs[3]->getOutput();
		cos4 *= envs[3]->getOutput();

		// 6. mix by demodmix AND ALGO

		auto demodmix = getValue(proc.timbreParams.demodmix);
		switch (algo) {
		case 0:
			sampleL = (sine4 * (1.0f - demodmix) + demodSample4L * demodmix);
			sampleR = (cos4 * (1.0f - demodmix) + demodSample4R * demodmix);
			break;
		case 1:
			sampleL = ((sine3 + sine4) * 0.5f * (1.0f - demodmix) + (demodSample3L + demodSample4L) * 0.5f * demodmix);
			sampleR = ((cos3 + cos4) * 0.5f * (1.0f - demodmix) + (demodSample3R + demodSample4R) * 0.5f * demodmix);
			break;
		case 2:
			sampleL = ((sine2 + sine4) * 0.5f * (1.0f - demodmix) + (demodSample2L + demodSample4L) * 0.5f * demodmix);
			sampleR = ((cos2 + cos4) * 0.5f * (1.0f - demodmix) + (demodSample2R + demodSample4R) * 0.5f * demodmix);
			break;
		case 3:
			sampleL = ((sine2 + sine3 + sine4) * 0.333f * (1.0f - demodmix) + (demodSample2L + demodSample3L + demodSample4L) * 0.333f * demodmix);
			sampleR = ((cos2 + cos3 + cos4) * 0.333f * (1.0f - demodmix) + (demodSample2R + demodSample3L + demodSample4R) * 0.333f * demodmix);
			break;
		default:
			sampleL = (sine4 * (1.0f - demodmix) + demodSample4L * demodmix);
			sampleR = (cos4 * (1.0f - demodmix) + demodSample4R * demodmix);
			break;
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

void SynthVoice2::updateParams(int blockSize)
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

bool SynthVoice2::isVoiceActive()
{
	return isActive();
}

float SynthVoice2::getFilterCutoffNormalized()
{
	float freq = filter.getFrequency();
	auto range = proc.filterParams.frequency->getUserRange();
	return range.convertTo0to1(juce::jlimit(range.start, range.end, gin::getMidiNoteFromHertz(freq)));
}

float SynthVoice2::getMSEG1Phase()
{
	return mseg1.getCurrentPhase();
}

float SynthVoice2::getMSEG2Phase()
{
	return mseg2.getCurrentPhase();
}

float SynthVoice2::getMSEG3Phase()
{
	return mseg3.getCurrentPhase();
}

float SynthVoice2::getMSEG4Phase()
{
	return mseg4.getCurrentPhase();
}

gin::Wave SynthVoice2::waveForChoice(int choice)
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

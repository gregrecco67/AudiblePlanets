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

// #define MIPP_ALIGNED_LOADS
#include "PluginProcessor.h"
#include "SynthVoice3.h"

// inline std::array<float, 2> SynthVoice3::panWeights(const float in) { // -1
// to 1 	return { std::sqrt((in + 1.f) * 0.5f), std::sqrt(1.f - ((in + 1.f) *
// 0.5f)) };
// }

//==============================================================================
SynthVoice3::SynthVoice3(APAudioProcessor &p)
    : proc(p), mseg1(proc.mseg1Data), mseg2(proc.mseg2Data),
      mseg3(proc.mseg3Data), mseg4(proc.mseg4Data), osc1(p.upsampledTables),
      osc2(p.upsampledTables), osc3(p.upsampledTables), osc4(p.upsampledTables),
	  env1(p.convex), env2(p.convex), env3(p.convex), env4(p.convex)
{
	mseg1.reset();
	mseg2.reset();
	mseg3.reset();
	mseg4.reset();
	filter.setNumChannels(2);
}

void SynthVoice3::noteStarted()
{
	antipop = 0.f; // ramp up

	curNote = getCurrentlyPlayingNote();

	proc.modMatrix.setPolyValue(*this, proc.randSrc1Poly, static_cast<float>(dist(gen)));
	proc.modMatrix.setPolyValue(*this, proc.randSrc2Poly, static_cast<float>(dist(gen)));

	if (MTS_ShouldFilterNote(proc.client, static_cast<char>(curNote.initialNote), 
		static_cast<char>(curNote.midiChannel))) { return; }

	fastKill = false;
	startVoice();

	auto note = getCurrentlyPlayingNote();
	if (glideInfo.fromNote >= 0 &&
	    (glideInfo.glissando || glideInfo.portamento)) {
		noteSmoother.setTime(glideInfo.rate);
		noteSmoother.setValueUnsmoothed(glideInfo.fromNote / 127.0f);
		noteSmoother.setValue(note.initialNote / 127.0f);
	} else {
		noteSmoother.setValueUnsmoothed(note.initialNote / 127.0f);
	}

	proc.modMatrix.setPolyValue(*this, proc.modSrcVelocity, note.noteOnVelocity.asUnsignedFloat());
	proc.modMatrix.setPolyValue(*this, proc.modSrcTimbre, note.initialTimbre.asUnsignedFloat());
	proc.modMatrix.setPolyValue(*this, proc.modSrcPressure, note.pressure.asUnsignedFloat());

	juce::ScopedValueSetter<bool> svs(disableSmoothing, true);

	filter.reset();

	fnz1 = proc.filterParams.frequency->getUserValue(); // in midi note #
	float q = gin::Q / (1.0f - (proc.filterParams.resonance->getUserValue() / 100.0f) * 0.99f);
	fqz1 = q;

	lfo1.reset();
	lfo2.reset();
	lfo3.reset();
	lfo4.reset();

    updateParams(0);
    snapParams();
    updateParams(0);
    snapParams();

	lfo1.noteOn();
	lfo2.noteOn();
	lfo3.noteOn();
	lfo4.noteOn();

	osc1.noteOn(lastp1 = proc.osc1Params.phase->getUserValue());
	osc2.noteOn(lastp2 = proc.osc2Params.phase->getUserValue());
	osc3.noteOn(lastp3 = proc.osc3Params.phase->getUserValue());
	osc4.noteOn(lastp4 = proc.osc4Params.phase->getUserValue());

	env1.noteOn();
	env2.noteOn();
	env3.noteOn();
	env4.noteOn();

	mseg1.noteOn();
	mseg2.noteOn();
	mseg3.noteOn();
	mseg4.noteOn();
}

void SynthVoice3::noteRetriggered()
{
	// antipop = 0.f;
	auto note = getCurrentlyPlayingNote();
	curNote = getCurrentlyPlayingNote();

	proc.modMatrix.setPolyValue(*this, proc.randSrc1Poly, static_cast<float>(dist(gen)));
	proc.modMatrix.setPolyValue(*this, proc.randSrc2Poly, static_cast<float>(dist(gen)));

	if (glideInfo.fromNote >= 0 &&
	    (glideInfo.glissando || glideInfo.portamento)) {
		noteSmoother.setTime(glideInfo.rate);
		noteSmoother.setValue(note.initialNote / 127.0f);
	} else {
		noteSmoother.setValueUnsmoothed(note.initialNote / 127.0f);
	}

	proc.modMatrix.setPolyValue(
	    *this, proc.modSrcVelocity, note.noteOnVelocity.asUnsignedFloat());
	proc.modMatrix.setPolyValue(
	    *this, proc.modSrcTimbre, note.initialTimbre.asUnsignedFloat());
	proc.modMatrix.setPolyValue(
	    *this, proc.modSrcPressure, note.pressure.asUnsignedFloat());

	updateParams(0);

	env1.noteOn();
	env2.noteOn();
	env3.noteOn();
	env4.noteOn();
    
    lfo1.reset();
    lfo2.reset();
    lfo3.reset();
    lfo4.reset();

    lfo1.noteOn();
    lfo2.noteOn();
    lfo3.noteOn();
    lfo4.noteOn();
    
    mseg1.noteOn();
    mseg2.noteOn();
    mseg3.noteOn();
    mseg4.noteOn();
}

void SynthVoice3::noteStopped(bool allowTailOff)
{
	env1.noteOff();
	env2.noteOff();
	env3.noteOff();
	env4.noteOff();
	curNote = getCurrentlyPlayingNote();
	proc.modMatrix.setPolyValue(
	    *this, proc.modSrcVelOff, curNote.noteOffVelocity.asUnsignedFloat());
	if (!allowTailOff) {
	 	clearCurrentNote();
	 	stopVoice();
	}
}

void SynthVoice3::notePressureChanged()
{
	auto note = getCurrentlyPlayingNote();
	proc.modMatrix.setPolyValue(
	    *this, proc.modSrcPressure, note.pressure.asUnsignedFloat());
}

void SynthVoice3::noteTimbreChanged()
{
	auto note = getCurrentlyPlayingNote();
	proc.modMatrix.setPolyValue(
	    *this, proc.modSrcTimbre, note.timbre.asUnsignedFloat());
}

void SynthVoice3::setCurrentSampleRate(double newRate)
{
	MPESynthesiserVoice::setCurrentSampleRate(newRate);

	auto quarter = newRate * 0.25;
	osc1.setSampleRate(newRate);
	osc2.setSampleRate(newRate);
	osc3.setSampleRate(newRate);
	osc4.setSampleRate(newRate);

	filter.setSampleRate(newRate);
	noteSmoother.setSampleRate(quarter);

	lfo1.setSampleRate(quarter);
	lfo2.setSampleRate(quarter);
	lfo3.setSampleRate(quarter);
	lfo4.setSampleRate(quarter);

	env1.setSampleRate(quarter);
	env2.setSampleRate(quarter);
	env3.setSampleRate(quarter);
	env4.setSampleRate(quarter);
	
	Envelope::Params p;
	env1.setParameters(p);
	env2.setParameters(p);
	env3.setParameters(p);
	env4.setParameters(p);

	mseg1.setSampleRate(quarter);
	mseg2.setSampleRate(quarter);
	mseg3.setSampleRate(quarter);
	mseg4.setSampleRate(quarter);

	fnb1 = std::exp(-2.0 * pi * 3500 / quarter);
	fna0 = 1 - fnb1;
	fnz1 = 95; // proc.filterParams.frequency->getUserValue();
	fqb1 = fnb1;
	fqa0 = fna0;
	fqz1 = 0; // proc.filterParams.resonance->getUserValue();
}

void SynthVoice3::renderNextBlock(juce::AudioBuffer<float> &outputBuffer, int startSample, int numSamples)
{
	updateParams(numSamples);

	synthBuffer.setSize(2, numSamples, false, false, true);
	auto synthBufferL = synthBuffer.getWritePointer(0);
	auto synthBufferR = synthBuffer.getWritePointer(1);

	// fill arrays with quadrature samples
	osc1.renderFloats(osc1Freq, osc1Params, osc1xs, osc1ys, numSamples);
	osc2.renderFloats(osc2Freq, osc2Params, osc2xs, osc2ys, numSamples);
	osc3.renderFloats(osc3Freq, osc3Params, osc3xs, osc3ys, numSamples);
	osc4.renderFloats(osc4Freq, osc4Params, osc4xs, osc4ys, numSamples);

	// the whole enchilada
	for (int i = 0; i < std::ceil((static_cast<float>(numSamples)) / 4.f);i++) 
	{
		a = envs[0]->getOutput();  // assigned in updateParams
		b = envs[1]->getOutput();  // envs[0] ~= env1!, etc.
		c = envs[2]->getOutput();
		d = envs[3]->getOutput();

		env1.getNextSample();
		env2.getNextSample();
		env3.getNextSample();
		env4.getNextSample();

		osc1x.load(&osc1xs[i * 4]);  // vectors of samples
		osc1y.load(&osc1ys[i * 4]);
		osc2x.load(&osc2xs[i * 4]);
		osc2y.load(&osc2ys[i * 4]);
		osc3x.load(&osc3xs[i * 4]);
		osc3y.load(&osc3ys[i * 4]);
		osc4x.load(&osc4xs[i * 4]);
		osc4y.load(&osc4ys[i * 4]);

		epi1xs[i] = osc1x * a;  // apply env
		epi1ys[i] = osc1y * a;

		epi2xs[i] = mipp::fmadd(osc2x, b, epi1xs[i]);  // 2 always based on 1
		epi2ys[i] = mipp::fmadd(osc2y, b, epi1ys[i]);

		// save distance/inverse for modulated sample, where we only care about
		// angle
		dist2sq = mipp::fmadd((epi2ys[i] - equant), (epi2ys[i] - equant), (epi2xs[i] * epi2xs[i]));
		dist2 = mipp::sqrt(dist2sq);
		invDist2 = oneFloat / (dist2 + .000001f);

		epi3xs[i] = mipp::fmadd(osc3x, c, epi2xs[i] * bits3[algo][0] + epi1xs[i] * bits3[algo][1]);
		epi3ys[i] = mipp::fmadd(osc3y, c, epi2ys[i] * bits3[algo][0] + epi1ys[i] * bits3[algo][1]);
		epi4xs[i] = mipp::fmadd(osc4x, d, epi3xs[i] * bits4[algo][0] + epi2xs[i] * bits4[algo][1] + epi1xs[i] * bits4[algo][2]);
		epi4ys[i] = mipp::fmadd(osc4y, d, epi3ys[i] * bits4[algo][0] + epi2ys[i] * bits4[algo][1] + epi1ys[i] * bits4[algo][2]);

		dist3sq = mipp::fmadd((epi3ys[i] - equant), (epi3ys[i] - equant), (epi3xs[i] * epi3xs[i]));
		dist3 = mipp::sqrt(dist3sq);
		invDist3 = oneFloat / (dist3 + .000001f);
		dist4sq = mipp::fmadd((epi4ys[i] - equant), (epi4ys[i] - equant), (epi4xs[i] * epi4xs[i]));
		dist4 = mipp::sqrt(dist4sq);
		invDist4 = oneFloat / (dist4 + .000001f);

		// get sine/cosine directly, without calculating angle, apply envelopes
		auto s4 = epi4ys[i] - (d * equant); // let envelope help tame equant
		auto s3 = epi3ys[i] - (c * equant);
		auto s2 = epi2ys[i] - (b * equant);

		sine4 = s4 * invDist4 * d;
		cos4 = epi4xs[i] * invDist4 * d;
		sine3 = s3 * invDist3 * c;
		cos3 = epi3xs[i] * invDist3 * c;
		sine2 = s2 * invDist2 * b;
		cos2 = epi2xs[i] * invDist2 * b;

		dmCos4 = s4 * dist4;
		dmSine4 = epi4xs[i] * dist4;
		dmCos3 = s3 * dist3;
		dmSine3 = epi3xs[i] * dist3;
		dmCos2 = s2 * dist2;
		dmSine2 = epi2xs[i] * dist2;

		sampleL = mix(
			(sine4 * mb[algo][0] + sine3 * mb[algo][1] + sine2 * mb[algo][2]) * mb[algo][3],
			(dmSine4 * mb[algo][0] + dmSine3 * mb[algo][1] + dmSine2 * mb[algo][2]) * (demodVol * mb[algo][3]),
			demodMix
		);
		sampleR = mix(
			(cos4 * mb[algo][0] + cos3 * mb[algo][1] + cos2 * mb[algo][2]) * mb[algo][3],
			(dmCos4 * mb[algo][0] + dmCos3 * mb[algo][1] + dmCos2 * mb[algo][2]) * (demodVol * mb[algo][3]),
			demodMix
		);

		sampleL = mipp::sat(sampleL, -1.0f, 1.0f) * antipop; // sat's doing some work!
		sampleR = mipp::sat(sampleR, -1.0f, 1.0f) * antipop;

		antipop += .03f;
		antipop = std::min(antipop, 1.0f);

		// SHIP IT OUT
		sampleL.store(&synthBufferL[i * mipp::N<float>()]);
		sampleR.store(&synthBufferR[i * mipp::N<float>()]);
	}

	// Get and apply velocity according to keytrack param
	float velocity = currentlyPlayingNote.noteOnVelocity.asUnsignedFloat();
	float ampKeyTrack = getValue(proc.globalParams.velSens);
	synthBuffer.applyGain(gin::velocityToGain(velocity, ampKeyTrack) * baseAmplitude);
	filter.process(synthBuffer);

	// Add synth voice to output
	outputBuffer.addFrom(0, startSample, synthBuffer, 0, 0, numSamples);
	outputBuffer.addFrom(1, startSample, synthBuffer, 1, 0, numSamples);

	// for timing
	finishBlock(numSamples);
	
	bool voiceShouldStop = false;
	switch (algo) {
		case 0:
			if (!envs[3]->isActive()) {voiceShouldStop = true;}
			break;
		case 1:
			if (!envs[2]->isActive() && !envs[3]->isActive()) {voiceShouldStop = true;}
			break;
		case 2:
			if (!envs[1]->isActive() && !envs[3]->isActive()) {voiceShouldStop = true;}
			break;
		case 3:
			if (!envs[1]->isActive() && !envs[2]->isActive() && !envs[3]->isActive())
			{ voiceShouldStop = true; }
			break;
	}

	if (voiceShouldStop) {
		clearCurrentNote();
		stopVoice();
	}

}

mipp::Reg<float> SynthVoice3::mix(mipp::Reg<float> a, mipp::Reg<float> b, float mix) {
	mipp::Reg<float> mixa = FastMath<float>::minimaxSin(mix * pi_v<float> * 0.5f);
	mipp::Reg<float> mixb = FastMath<float>::minimaxSin((mix - 1.0f) * pi_v<float> * 0.5f);
	return mipp::fmadd(a, mixa, (b * mixb));
}

void SynthVoice3::updateParams(int blockSize)
{
	if (tilUpdate != 0) {
		--tilUpdate;
		return;
	}  // at 4x os, we don't need this every block
	else {
		tilUpdate = 3;
	}  // every 4th to match envelope/lfo/mseg
	algo = static_cast<int>(getValue(proc.timbreParams.algo));
	equant = getValue(proc.timbreParams.equant);
	demodVol = getValue(proc.timbreParams.demodvol);
	demodMix = getValue(proc.timbreParams.demodmix);

	auto note = getCurrentlyPlayingNote();
	proc.modMatrix.setPolyValue(
	    *this, proc.modSrcNote, note.initialNote / 127.0f);
	currentMidiNote = noteSmoother.getCurrentValue() * 127.0f;
	if (glideInfo.glissando)
		currentMidiNote = (float)juce::roundToInt(currentMidiNote);
	noteSmoother.process(blockSize);

	float dummy;
	float remainder = std::modf(currentMidiNote, &dummy);
	float baseFreq = static_cast<float>(MTS_NoteToFrequency(
	    proc.client, static_cast<char>(currentMidiNote), static_cast<char>(note.midiChannel)));
	baseFreq *= static_cast<float>(
		std::pow(1.05946309436f, note.totalPitchbendInSemitones 
		* (proc.globalParams.pitchbendRange->getUserValue() / 2.0f) 
		+ remainder));
	baseFreq = juce::jlimit(20.0f, 20000.f, baseFreq * getValue(proc.timbreParams.pitch));

	if (proc.osc1Params.fixed->isOn()) {
		osc1Freq = std::clamp(
		    (static_cast<int>(getValue(proc.osc1Params.coarse) + 0.0001f)
			+ getValue(proc.osc1Params.fine)) * 100.f, 0.01f, 20000.f);
	} else {
		osc1Freq = baseFreq
			* (static_cast<int>(getValue(proc.osc1Params.coarse) + 0.0001f)
			+ getValue(proc.osc1Params.fine));
	}
	if (proc.osc2Params.fixed->isOn()) {
		osc2Freq = std::clamp(
		    (static_cast<int>(getValue(proc.osc2Params.coarse) + 0.0001f) 
			+ getValue(proc.osc2Params.fine)) * 100.f, 0.01f, 20000.f);
	} else {
		osc2Freq = baseFreq 
			* (static_cast<int>(getValue(proc.osc2Params.coarse) + 0.0001f)
		    + getValue(proc.osc2Params.fine));
	}
	if (proc.osc3Params.fixed->isOn()) {
		osc3Freq = std::clamp(
		    (static_cast<int>(getValue(proc.osc3Params.coarse) + 0.0001f)
			+ getValue(proc.osc3Params.fine)) * 100.f, 0.01f, 20000.f);
	} else {
		osc3Freq = baseFreq 
			* (static_cast<int>(getValue(proc.osc3Params.coarse) + 0.0001f) 
			+ getValue(proc.osc3Params.fine));
	}
	if (proc.osc4Params.fixed->isOn()) {
		osc4Freq = std::clamp(
		    (static_cast<int>(getValue(proc.osc4Params.coarse) + 0.0001f) 
			+ getValue(proc.osc4Params.fine)) 
			* 100.f, 0.01f, 20000.f);
	} else {
		osc4Freq = baseFreq 
			* (static_cast<int>(getValue(proc.osc4Params.coarse) + 0.0001f)
			+ getValue(proc.osc4Params.fine));
	}

	osc1Params.wave = waveForChoice(static_cast<int>(getValue(proc.osc1Params.wave)));
	osc1Params.vol = getValue(proc.osc1Params.volume);
	auto phaseParam = getValue(proc.osc1Params.phase);
	auto diff = phaseParam - lastp1;
	osc1.bumpPhase(diff);
	lastp1 = phaseParam;
	envs[0] = envsByNum[static_cast<size_t>(proc.osc1Params.env->getUserValueInt())];

	// osc 2
	//==================================================

	osc2Params.wave = waveForChoice(static_cast<int>(getValue(proc.osc2Params.wave)));
	osc2Params.vol = getValue(proc.osc2Params.volume);
	phaseParam = getValue(proc.osc2Params.phase);
	diff = phaseParam - lastp2;
	osc2.bumpPhase(diff);
	lastp2 = phaseParam;
	envs[1] = envsByNum[static_cast<size_t>(proc.osc2Params.env->getUserValueInt())];

	// osc 3
	// ------------------

	osc3Params.wave = waveForChoice(static_cast<int>(getValue(proc.osc3Params.wave)));
	osc3Params.vol = getValue(proc.osc3Params.volume);
	phaseParam = getValue(proc.osc3Params.phase);
	diff = phaseParam - lastp3;
	osc3.bumpPhase(diff);
	lastp3 = phaseParam;
	envs[2] = envsByNum[static_cast<size_t>(proc.osc3Params.env->getUserValueInt())];

	// osc4
	// -------------
	osc4Params.wave = waveForChoice(static_cast<int>(getValue(proc.osc4Params.wave)));
	osc4Params.vol = getValue(proc.osc4Params.volume);
	phaseParam = getValue(proc.osc4Params.phase);
	diff = phaseParam - lastp4;
	osc4.bumpPhase(diff);
	lastp4 = phaseParam;
	envs[3] = envsByNum[static_cast<size_t>(proc.osc4Params.env->getUserValueInt())];
	
	switch (proc.filterParams.type->getUserValueInt())
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

// filter
	float noteNum = getValue(proc.filterParams.frequency);
	noteNum += (currentlyPlayingNote.initialNote - 50) *
	           getValue(proc.filterParams.keyTracking);
	float f = gin::getMidiNoteInHertz(noteNum);
	f = juce::jlimit(4.0f, maxFreq, f);
	float q = gin::Q / (1.0f - (getValue(proc.filterParams.resonance) / 100.0f) * 0.99f);

	fnz1 = fna0 * noteNum + fnb1 * fnz1;
	fqz1 = fqa0 * q + fqb1 * fqz1;
	filter.setParams(juce::jlimit<float>(4.f, maxFreq, gin::getMidiNoteInHertz(fnz1)), fqz1);

	gin::LFO::Parameters params;
	float freq = 0;

	// lfo 1
	if (proc.lfo1Params.sync->getUserValue() > 0.0f)
		freq = 1.f / gin::NoteDuration::getNoteDurations()
		    [static_cast<size_t>(proc.lfo1Params.beat->getUserValue())].toSeconds(proc.playhead);
	else
		freq = getValue(proc.lfo1Params.rate);
	params.waveShape = static_cast<gin::LFO::WaveShape>(proc.lfo1Params.wave->getUserValueInt());
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
		freq = 1.f / gin::NoteDuration::getNoteDurations()
			[static_cast<size_t>(proc.lfo2Params.beat->getUserValue())].toSeconds(proc.playhead);
	else
		freq = getValue(proc.lfo2Params.rate);
	params.waveShape = static_cast<gin::LFO::WaveShape>(proc.lfo2Params.wave->getUserValueInt());
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
		freq = 1.f / gin::NoteDuration::getNoteDurations()
		    [static_cast<size_t>(proc.lfo3Params.beat->getUserValue())].toSeconds(proc.playhead);
	else
		freq = getValue(proc.lfo3Params.rate);
	params.waveShape = static_cast<gin::LFO::WaveShape>(proc.lfo3Params.wave->getUserValueInt());
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
		freq = 1.f / gin::NoteDuration::getNoteDurations()
			[static_cast<size_t>(proc.lfo4Params.beat->getUserValue())].toSeconds(proc.playhead);
	else
		freq = getValue(proc.lfo4Params.rate);
	params.waveShape = static_cast<gin::LFO::WaveShape>(proc.lfo4Params.wave->getUserValueInt());
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
	p.sync = (!(mode == 0));
	p.repeat = (!(mode == 0));
	if (mode == 1) {
		p.sync = true;
		p.syncduration = 1.f * gin::NoteDuration::getNoteDurations()
		    [size_t(proc.env1Params.duration->getUserValue())].toSeconds(proc.playhead);
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
	p.sync = (mode != 0);
	p.repeat = (mode != 0);
	if (mode == 1) {
		p.sync = true;
		p.syncduration =
		    1.f * gin::NoteDuration::getNoteDurations()
			[size_t(proc.env2Params.duration->getUserValue())].toSeconds(proc.playhead);
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
	p.sync = (!(mode == 0));
	p.repeat = (!(mode == 0));
	if (mode == 1) {
		p.sync = true;
		p.syncduration = 1.f * gin::NoteDuration::getNoteDurations()
		    [size_t(proc.env3Params.duration->getUserValue())].toSeconds(proc.playhead);
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
	p.sync = (!(mode == 0));
	p.repeat = (!(mode == 0));
	if (mode == 1) {
		p.sync = true;
		p.syncduration = 1.f * gin::NoteDuration::getNoteDurations()
		    [size_t(proc.env4Params.duration->getUserValue())].toSeconds(proc.playhead);
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

	// MSEGs
	if (proc.mseg1Params.sync->isOn()) {
		mseg1Params.frequency = 1.f / gin::NoteDuration::getNoteDurations()
			[size_t(getValue(proc.mseg1Params.beat))].toSeconds(proc.playhead);
	} else {
		mseg1Params.frequency = getValue(proc.mseg1Params.rate);
	}
	mseg1Params.depth = getValue(proc.mseg1Params.depth);  // proc.mseg1Params.depth->getUserValue();
	mseg1Params.offset = getValue(proc.mseg1Params.offset);
	mseg1Params.loop = proc.mseg1Params.loop->isOn();

	if (proc.mseg2Params.sync->isOn()) {
		mseg2Params.frequency = 1.f / gin::NoteDuration::getNoteDurations()
			[size_t(getValue(proc.mseg2Params.beat))].toSeconds(proc.playhead);
	} else {
		mseg2Params.frequency = getValue(proc.mseg2Params.rate);
	}
	mseg2Params.depth = getValue(proc.mseg2Params.depth);
	mseg2Params.offset = getValue(proc.mseg2Params.offset);
	mseg2Params.loop = proc.mseg2Params.loop->isOn();

	if (proc.mseg3Params.sync->isOn()) {
		mseg3Params.frequency = 1.f / gin::NoteDuration::getNoteDurations()
			[size_t(getValue(proc.mseg3Params.beat))].toSeconds(proc.playhead);
	} else {
		mseg3Params.frequency = getValue(proc.mseg3Params.rate);
	}
	mseg3Params.depth = getValue(proc.mseg3Params.depth);  // proc.mseg3Params.depth->getUserValue();
	mseg3Params.offset = getValue(proc.mseg3Params.offset);
	mseg3Params.loop = proc.mseg3Params.loop->isOn();

	if (proc.mseg4Params.sync->isOn()) {
		mseg4Params.frequency = 1.f / gin::NoteDuration::getNoteDurations()
			[size_t(getValue(proc.mseg4Params.beat))].toSeconds(proc.playhead);
	} else {
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

float SynthVoice3::getFilterCutoffNormalized()
{
	float freq = filter.getFrequency();
	auto range = proc.filterParams.frequency->getUserRange();
	return range.convertTo0to1(juce::jlimit(range.start, range.end, gin::getMidiNoteFromHertz(freq)));
}

gin::Wave SynthVoice3::waveForChoice(int choice)
{
	switch (choice) {
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
	}
	return gin::Wave::sine;
}

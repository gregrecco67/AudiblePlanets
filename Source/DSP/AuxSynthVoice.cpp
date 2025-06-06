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

#include "AuxSynthVoice.h"
#include "PluginProcessor.h"

//==============================================================================
AuxSynthVoice::AuxSynthVoice(APAudioProcessor &p)
    : proc(p), osc(proc.analogTables, 8), mseg1(proc.mseg1Data),
      mseg2(proc.mseg2Data), mseg3(proc.mseg3Data), mseg4(proc.mseg4Data),
	  env1(proc.convex), env2(proc.convex), env3(proc.convex), env4(proc.convex)
{
	mseg1.reset();
	mseg2.reset();
	mseg3.reset();
	mseg4.reset();
	filter.setNumChannels(2);
	oscParams.voices = 4;
}

void AuxSynthVoice::noteStarted()
{
	curNote = getCurrentlyPlayingNote();

	if (MTS_ShouldFilterNote(proc.client, static_cast<char>(curNote.initialNote),
        static_cast<char>(curNote.midiChannel))) {
		return;
	}

	fastKill = false;
	startVoice();

	const auto note = getCurrentlyPlayingNote();
	if (glideInfo.fromNote >= 0 &&
	    (glideInfo.glissando || glideInfo.portamento)) {
		noteSmoother.setTime(glideInfo.rate);
		noteSmoother.setValueUnsmoothed(glideInfo.fromNote / 127.0f);
		noteSmoother.setValue(note.initialNote / 127.0f);
	} else {
		noteSmoother.setValueUnsmoothed(note.initialNote / 127.0f);
	}

	proc.modMatrix.setPolyValue(*this, proc.modSrcVelocity,
	                            note.noteOnVelocity.asUnsignedFloat());
	proc.modMatrix.setPolyValue(*this, proc.modSrcTimbre,
	                            note.initialTimbre.asUnsignedFloat());
	proc.modMatrix.setPolyValue(*this, proc.modSrcPressure,
	                            note.pressure.asUnsignedFloat());

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

	osc.noteOn();
	env1.noteOn();
	env2.noteOn();
	env3.noteOn();
	env4.noteOn();

	mseg1.noteOn();
	mseg2.noteOn();
	mseg3.noteOn();
	mseg4.noteOn();
}

void AuxSynthVoice::noteRetriggered()
{
	const auto note = getCurrentlyPlayingNote();
	curNote = getCurrentlyPlayingNote();

	if (glideInfo.fromNote >= 0 &&
	    (glideInfo.glissando || glideInfo.portamento)) {
		noteSmoother.setTime(glideInfo.rate);
		noteSmoother.setValue(note.initialNote / 127.0f);
	} else {
		noteSmoother.setValueUnsmoothed(note.initialNote / 127.0f);
	}

	proc.modMatrix.setPolyValue(*this, proc.modSrcVelocity,
	                            note.noteOnVelocity.asUnsignedFloat());
	proc.modMatrix.setPolyValue(*this, proc.modSrcTimbre,
	                            note.initialTimbre.asUnsignedFloat());
	proc.modMatrix.setPolyValue(*this, proc.modSrcPressure,
	                            note.pressure.asUnsignedFloat());

	updateParams(0);

	osc.noteOn();

	env1.noteOn();
	env2.noteOn();
	env3.noteOn();
	env4.noteOn();
}

void AuxSynthVoice::noteStopped(bool allowTailOff)
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

void AuxSynthVoice::notePressureChanged()
{
	curNote = getCurrentlyPlayingNote();
	const auto note = getCurrentlyPlayingNote();
	proc.modMatrix.setPolyValue(*this, proc.modSrcPressure,
	                            note.pressure.asUnsignedFloat());
}

void AuxSynthVoice::noteTimbreChanged()
{
	curNote = getCurrentlyPlayingNote();
	const auto note = getCurrentlyPlayingNote();
	proc.modMatrix.setPolyValue(*this, proc.modSrcTimbre,
	                            note.timbre.asUnsignedFloat());
}

void AuxSynthVoice::setCurrentSampleRate(double newRate)
{
	MPESynthesiserVoice::setCurrentSampleRate(newRate);

	osc.setSampleRate(newRate);

	filter.setSampleRate(newRate);

	lfo1.setSampleRate(newRate);
	lfo2.setSampleRate(newRate);
	lfo3.setSampleRate(newRate);
	lfo4.setSampleRate(newRate);

	noteSmoother.setSampleRate(newRate);
	constexpr Envelope::Params p;
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

void AuxSynthVoice::renderNextBlock(juce::AudioBuffer<float> &outputBuffer,
                                    int startSample,
                                    int numSamples)
{
	updateParams(numSamples);
	gin::ScratchBuffer scratchBuffer(2, numSamples);

	// oscillator

	// Get and apply velocity according to keytrack param
	const float velocity = currentlyPlayingNote.noteOnVelocity.asUnsignedFloat();
	const float ampKeyTrack = getValue(proc.globalParams.velSens);

	// do the stuff
	float envOut{0.f};
	switch (currentEnv) {
		case 0:
			envOut = env1.getOutput();
			break;
		case 1:
			envOut = env2.getOutput();
			break;
		case 2:
			envOut = env3.getOutput();
			break;
		case 3:
			envOut = env4.getOutput();
			break;
		default:
			envOut = 0.f;
	}

	osc.processAdding(osc1Note, oscParams, scratchBuffer);
	const auto volume = juce::Decibels::decibelsToGain(getValue(proc.auxParams.volume));
	const auto gain = gin::velocityToGain(velocity, ampKeyTrack) * volume * envOut * baseAmplitude;
	scratchBuffer.applyGain(gain);

	filter.process(scratchBuffer);

	// Copy synth voice to output
	outputBuffer.addFrom(0, startSample, scratchBuffer, 0, 0, numSamples);
	outputBuffer.addFrom(1, startSample, scratchBuffer, 1, 0, numSamples);

	bool shouldStop{false};
	switch (currentEnv) {
		case 0:
			shouldStop = !env1.isActive();
			break;
		case 1:
			shouldStop = !env2.isActive();
			break;
		case 2:
			shouldStop = !env3.isActive();
			break;
		case 3:
			shouldStop = !env4.isActive();
			break;
		default:
			jassertfalse;
			shouldStop = true;
	}

	if (shouldStop) {
		clearCurrentNote();
		stopVoice();
	}

	finishBlock(numSamples);
}

void AuxSynthVoice::updateParams(int blockSize)
{
	auto note = getCurrentlyPlayingNote();

	proc.modMatrix.setPolyValue(*this, proc.modSrcNote,
	                            note.initialNote / 127.0f);

	currentMidiNote = noteSmoother.getCurrentValue() * 127.0f;
	if (glideInfo.glissando) { currentMidiNote = static_cast<float>(juce::roundToInt(currentMidiNote)); }

	float dummy;
	float remainder = std::modf(currentMidiNote, &dummy);
	auto baseFreq = static_cast<float>(MTS_NoteToFrequency(proc.client, static_cast<char>(currentMidiNote),
		static_cast<char>(note.midiChannel)));
	if (!proc.auxParams.ignorepb->isOn())
		baseFreq *= static_cast<float>(std::pow(
		    1.05946309436f, note.totalPitchbendInSemitones 
			* (proc.globalParams.pitchbendRange->getUserValue() / 2.0f) 
			+ remainder));
	baseFreq = juce::jlimit(20.0f, 20000.f,
			baseFreq * getValue(proc.timbreParams.pitch)
	        * static_cast<float>(std::pow(2.0f, static_cast<int>(getValue(proc.auxParams.octave)))));
	osc1Note = gin::getMidiNoteFromHertz(baseFreq);

	currentEnv = proc.auxParams.env->getUserValueInt();

	switch (proc.auxParams.wave->getUserValueInt())
		{
		case 0:
			oscParams.wave = gin::Wave::sine;
			break;
		case 1:
			oscParams.wave = gin::Wave::triangle;
			break;
		case 2:
			oscParams.wave = gin::Wave::square;
			break;
		case 3:
			oscParams.wave = gin::Wave::sawUp;
			break;
		case 4:
			oscParams.wave = gin::Wave::pinkNoise;
			break;
		case 5:
			oscParams.wave = gin::Wave::whiteNoise;
			break;
		default:
			oscParams.wave = gin::Wave::sine;
	}

	oscParams.spread = getValue(proc.auxParams.spread) / 100.0f;
	oscParams.detune = getValue(proc.auxParams.detune);

	float n = getValue(proc.auxParams.filtercutoff);
	n += (curNote.initialNote - 50) * getValue(proc.auxParams.filterkeytrack);

	float f = gin::getMidiNoteInHertz(n);
	f = juce::jlimit(4.0f, maxFreq, f);

	float q = gin::Q / (1.0f - (getValue(proc.auxParams.filterres) / 100.0f) * 0.99f);

	switch (proc.auxParams.filtertype->getUserValueInt()) {
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
		default:
			filter.setType(gin::Filter::lowpass);
			filter.setSlope(gin::Filter::db12);
	}

	filter.setParams(f, q);

	gin::LFO::Parameters params;
	float freq = 0;

	// lfo 1
	if (proc.lfo1Params.sync->isOn())
		freq = 1.0f / gin::NoteDuration::getNoteDurations()[static_cast<size_t>(
			proc.lfo1Params.beat->getUserValue())]
			.toSeconds(proc.playhead);
	else
		freq = getValue(proc.lfo1Params.rate);
	params.frequency = freq;
	params.waveShape = static_cast<gin::LFO::WaveShape>(proc.lfo1Params.wave->getUserValueInt());
	params.phase = getValue(proc.lfo1Params.phase);
	params.offset = getValue(proc.lfo1Params.offset);
	params.depth = getValue(proc.lfo1Params.depth);
	params.delay = getValue(proc.lfo1Params.delay);
	params.fade = getValue(proc.lfo1Params.fade);
	lfo1.setParameters(params);
	lfo1.process(blockSize);
	proc.modMatrix.setPolyValue(*this, proc.modSrcLFO1, lfo1.getOutput());

	// lfo 2
	if (proc.lfo2Params.sync->isOn()) {
		freq = 1.0f / gin::NoteDuration::getNoteDurations()
			[static_cast<size_t>(proc.lfo2Params.beat->getUserValue())].toSeconds(proc.playhead);
	}
	else
		freq = getValue(proc.lfo2Params.rate);
	params.frequency = freq;
	params.waveShape = static_cast<gin::LFO::WaveShape>(proc.lfo2Params.wave->getUserValueInt());
	params.phase = getValue(proc.lfo2Params.phase);
	params.offset = getValue(proc.lfo2Params.offset);
	params.depth = getValue(proc.lfo2Params.depth);
	params.delay = getValue(proc.lfo2Params.delay);
	params.fade = getValue(proc.lfo2Params.fade);
	lfo2.setParameters(params);
	lfo2.process(blockSize);
	proc.modMatrix.setPolyValue(*this, proc.modSrcLFO2, lfo2.getOutput());

	// lfo 3
	if (proc.lfo3Params.sync->isOn()) {
		freq = 1.0f / gin::NoteDuration::getNoteDurations()[
			static_cast<size_t>(proc.lfo3Params.beat->getUserValue())].toSeconds(proc.playhead);
	}
	else { freq = getValue(proc.lfo3Params.rate); }
	params.frequency = freq;
	params.waveShape = static_cast<gin::LFO::WaveShape>(proc.lfo3Params.wave->getUserValueInt());
	params.phase = getValue(proc.lfo3Params.phase);
	params.offset = getValue(proc.lfo3Params.offset);
	params.depth = getValue(proc.lfo3Params.depth);
	params.delay = getValue(proc.lfo3Params.delay);
	params.fade = getValue(proc.lfo3Params.fade);
	lfo3.setParameters(params);
	lfo3.process(blockSize);
	proc.modMatrix.setPolyValue(*this, proc.modSrcLFO3, lfo3.getOutput());

	// lfo 4
	if (proc.lfo4Params.sync->isOn()) {
		freq = 1.0f / gin::NoteDuration::getNoteDurations()[static_cast<size_t>(
			proc.lfo4Params.beat->getUserValue())].toSeconds(proc.playhead);
	}
	else
		freq = getValue(proc.lfo4Params.rate);
	params.frequency = freq;
	params.waveShape = static_cast<gin::LFO::WaveShape>(proc.lfo4Params.wave->getUserValueInt());
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
	p.sync = (mode != 0);
	p.repeat = (mode != 0);
	if (mode == 1) {
		p.sync = true;
		p.syncduration = gin::NoteDuration::getNoteDurations()[static_cast<size_t>(
			proc.env1Params.duration->getUserValue())].toSeconds(proc.playhead);
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
		p.syncduration = gin::NoteDuration::getNoteDurations()[static_cast<size_t>(
			proc.env2Params.duration->getUserValue())].toSeconds(proc.playhead);
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
	p.sync = (mode != 0);
	p.repeat = (mode != 0);
	if (mode == 1) {
		p.sync = true;
		p.syncduration = gin::NoteDuration::getNoteDurations()[static_cast<size_t>(
			proc.env3Params.duration->getUserValue())].toSeconds(proc.playhead);
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
	p.sync = (mode != 0);
	p.repeat = (mode != 0);
	if (mode == 1) {
		p.sync = true;
		p.syncduration = gin::NoteDuration::getNoteDurations()[static_cast<size_t>(
			proc.env4Params.duration->getUserValue())].toSeconds(proc.playhead);
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

	for (int i = 0; i < blockSize; ++i) {
		env1.getNextSample();
		env2.getNextSample();
		env3.getNextSample();
		env4.getNextSample();
	}

	noteSmoother.process(blockSize);

	if (proc.mseg1Params.sync->isOn()) {
		mseg1Params.frequency = 1 / gin::NoteDuration::getNoteDurations()[static_cast<size_t>(
			getValue(proc.mseg1Params.beat))].toSeconds(proc.playhead);
	} else {
		mseg1Params.frequency = getValue(proc.mseg1Params.rate);
	}
	mseg1Params.depth = getValue(proc.mseg1Params.depth);
	mseg1Params.offset = getValue(proc.mseg1Params.offset);
	mseg1Params.loop = proc.mseg1Params.loop->isOn();

	if (proc.mseg2Params.sync->isOn()) {
		mseg2Params.frequency = 1 / gin::NoteDuration::getNoteDurations()[static_cast<size_t>(
			getValue(proc.mseg2Params.beat))].toSeconds(proc.playhead);
	} else {
		mseg2Params.frequency = getValue(proc.mseg2Params.rate);
	}
	mseg2Params.depth = getValue(proc.mseg2Params.depth);
	mseg2Params.offset = getValue(proc.mseg2Params.offset);
	mseg2Params.loop = proc.mseg2Params.loop->isOn();

	if (proc.mseg3Params.sync->isOn()) {
		mseg3Params.frequency = 1 / gin::NoteDuration::getNoteDurations()[static_cast<size_t>(
			getValue(proc.mseg3Params.beat))].toSeconds(proc.playhead);
	} else {
		mseg3Params.frequency = getValue(proc.mseg3Params.rate);
	}
	mseg3Params.depth = getValue(proc.mseg3Params.depth);
	mseg3Params.offset = getValue(proc.mseg3Params.offset);
	mseg3Params.loop = proc.mseg3Params.loop->isOn();

	if (proc.mseg4Params.sync->isOn()) {
		mseg4Params.frequency = 1 / gin::NoteDuration::getNoteDurations()[static_cast<size_t>(
			getValue(proc.mseg4Params.beat))].toSeconds(proc.playhead);
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

float AuxSynthVoice::getFilterCutoffNormalized() const {
	const float freq = filter.getFrequency();
	const auto range = proc.filterParams.frequency->getUserRange();
	return range.convertTo0to1(juce::jlimit(range.start, range.end,
		gin::getMidiNoteFromHertz(freq)));
}

#include "SynthVoice.h"
#include "PluginProcessor.h"

//==============================================================================
SynthVoice::SynthVoice (APAudioProcessor& p)
    : proc (p)
{
    filter.setNumChannels (2);
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
        noteSmoother.setTime (glideInfo.rate);
        noteSmoother.setValueUnsmoothed (glideInfo.fromNote / 127.0f);
        noteSmoother.setValue (note.initialNote / 127.0f);
    }
    else
    {
        noteSmoother.setValueUnsmoothed (note.initialNote / 127.0f);
    }

    proc.modMatrix.setPolyValue (*this, proc.modSrcVelocity, note.noteOnVelocity.asUnsignedFloat());
    proc.modMatrix.setPolyValue (*this, proc.modSrcTimbre, note.initialTimbre.asUnsignedFloat());
    proc.modMatrix.setPolyValue (*this, proc.modSrcPressure, note.pressure.asUnsignedFloat());

    juce::ScopedValueSetter<bool> svs (disableSmoothing, true);

    filter.reset();

	lfo1.reset();
	lfo2.reset();
	lfo3.reset();
	lfo4.reset();

    updateParams (0);
    snapParams();
    updateParams (0);
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
        noteSmoother.setTime (glideInfo.rate);
        noteSmoother.setValue (note.initialNote / 127.0f);
    }
    else
    {
        noteSmoother.setValueUnsmoothed (note.initialNote / 127.0f);
    }
    
    proc.modMatrix.setPolyValue (*this, proc.modSrcVelocity, note.noteOnVelocity.asUnsignedFloat());
    proc.modMatrix.setPolyValue (*this, proc.modSrcTimbre, note.initialTimbre.asUnsignedFloat());
    proc.modMatrix.setPolyValue (*this, proc.modSrcPressure, note.pressure.asUnsignedFloat());
    
    updateParams (0);

	osc1.noteOn();
	osc2.noteOn();
	osc3.noteOn();
	osc4.noteOn();

	env1.noteOn();
	env2.noteOn();
	env3.noteOn();
	env4.noteOn();

}

void SynthVoice::noteStopped (bool allowTailOff)
{
	env1.noteOff();
	env2.noteOff();
	env3.noteOff();
	env4.noteOff();

    if (! allowTailOff)
    {
        clearCurrentNote();
        stopVoice();
    }
}

void SynthVoice::notePressureChanged()
{
    auto note = getCurrentlyPlayingNote();
    proc.modMatrix.setPolyValue (*this, proc.modSrcPressure, note.pressure.asUnsignedFloat());
}

void SynthVoice::noteTimbreChanged()
{
    auto note = getCurrentlyPlayingNote();
    proc.modMatrix.setPolyValue (*this, proc.modSrcTimbre, note.initialTimbre.asUnsignedFloat());
}

void SynthVoice::setCurrentSampleRate (double newRate)
{
    MPESynthesiserVoice::setCurrentSampleRate (newRate);

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

void SynthVoice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    updateParams (numSamples);

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
	osc1SineBuffer.applyGain(gin::velocityToGain(velocity, ampKeyTrack));
	osc1CosineBuffer.applyGain(gin::velocityToGain(velocity, ampKeyTrack));
	osc2SineBuffer.applyGain(gin::velocityToGain(velocity, ampKeyTrack));
	osc2CosineBuffer.applyGain(gin::velocityToGain(velocity, ampKeyTrack));
	osc3SineBuffer.applyGain(gin::velocityToGain(velocity, ampKeyTrack));
	osc3CosineBuffer.applyGain(gin::velocityToGain(velocity, ampKeyTrack));
	osc4SineBuffer.applyGain(gin::velocityToGain(velocity, ampKeyTrack));
	osc4CosineBuffer.applyGain(gin::velocityToGain(velocity, ampKeyTrack));
    
	// Apply filter -- we'll do this after orbital processing
	//if (proc.filterParams.enable->isOn())
	//	filter.process(osc1buffer);

	// Run ADSR
	envs[0]->processMultiplying(osc1SineBuffer);
	envs[0]->processMultiplying(osc1CosineBuffer);
	envs[1]->processMultiplying(osc2SineBuffer);
	envs[1]->processMultiplying(osc2CosineBuffer);
	envs[2]->processMultiplying(osc3SineBuffer);
	envs[2]->processMultiplying(osc3CosineBuffer);
	envs[3]->processMultiplying(osc4SineBuffer);
	envs[3]->processMultiplying(osc4CosineBuffer);

	// algo 1
	for (int i = 0; i < numSamples; i++)
	{
		epi1 = {
			osc1SineBuffer.getSample(0, i), osc1CosineBuffer.getSample(0, i),
			osc1SineBuffer.getSample(1, i), osc1CosineBuffer.getSample(1, i)
		};
		epi2 = {
			epi1.xL + osc2SineBuffer.getSample(0, i), epi1.yL + osc2CosineBuffer.getSample(0, i),
			epi1.xR + osc2SineBuffer.getSample(1, i), epi1.yR + osc2CosineBuffer.getSample(1, i)
		};
		epi3 = {
			epi2.xL + osc3SineBuffer.getSample(0, i), epi2.yL + osc3CosineBuffer.getSample(0, i),
			epi2.xR + osc3SineBuffer.getSample(1, i), epi2.yR + osc3CosineBuffer.getSample(1, i)
		};
		epi4 = {
			epi3.xL + osc4SineBuffer.getSample(0, i), epi3.yL + osc4CosineBuffer.getSample(0, i),
			epi3.xR + osc4SineBuffer.getSample(1, i), epi3.yR + osc4CosineBuffer.getSample(1, i)
		};
		
		synthBuffer.setSample(0, i, epi4.xL);
		synthBuffer.setSample(1, i, epi4.xR);
	}


    if (env1.getState() == gin::AnalogADSR::State::idle)
    {
        clearCurrentNote();
        stopVoice();
    }

    // Copy synth voice to output
	outputBuffer.addFrom(0, startSample, synthBuffer, 0, 0, numSamples);
	outputBuffer.addFrom(1, startSample, synthBuffer, 1, 0, numSamples);

    
    finishBlock (numSamples);
}

void SynthVoice::updateParams (int blockSize)
{
    auto note = getCurrentlyPlayingNote();
    
    proc.modMatrix.setPolyValue (*this, proc.modSrcNote, note.initialNote / 127.0f);

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
        float n = getValue (proc.filterParams.frequency);
        n += (currentlyPlayingNote.initialNote - 60) * getValue (proc.filterParams.keyTracking);

        float f = gin::getMidiNoteInHertz (n);
        float maxFreq = std::min (20000.0f, float (getSampleRate() / 2));
        f = juce::jlimit (4.0f, maxFreq, f);

        float q = gin::Q / (1.0f - (getValue (proc.filterParams.resonance) / 100.0f) * 0.99f);

        switch (int (proc.filterParams.type->getProcValue()))
        {
            case 0:
                filter.setType (gin::Filter::lowpass);
                filter.setSlope (gin::Filter::db12);
                break;
            case 1:
                filter.setType (gin::Filter::lowpass);
                filter.setSlope (gin::Filter::db24);
                break;
            case 2:
                filter.setType (gin::Filter::highpass);
                filter.setSlope (gin::Filter::db12);
                break;
            case 3:
                filter.setType (gin::Filter::highpass);
                filter.setSlope (gin::Filter::db24);
                break;
            case 4:
                filter.setType (gin::Filter::bandpass);
                filter.setSlope (gin::Filter::db12);
                break;
            case 5:
                filter.setType (gin::Filter::bandpass);
                filter.setSlope (gin::Filter::db24);
                break;
            case 6:
                filter.setType (gin::Filter::notch);
                filter.setSlope (gin::Filter::db12);
                break;
            case 7:
                filter.setType (gin::Filter::notch);
                filter.setSlope (gin::Filter::db24);
                break;
        }

        filter.setParams (f, q);
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

    env1.setAttack (getValue (proc.env1Params.attack));
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

    noteSmoother.process (blockSize);
}

bool SynthVoice::isVoiceActive()
{
    return isActive();
}

float SynthVoice::getFilterCutoffNormalized()
{
    float freq = filter.getFrequency();
    auto range = proc.filterParams.frequency->getUserRange();
    return range.convertTo0to1 (juce::jlimit (range.start, range.end, gin::getMidiNoteFromHertz (freq)));
}

#include "SynthVoice.h"
#include "PluginProcessor.h"

//==============================================================================
SynthVoice::SynthVoice (APAudioProcessor& p)
    : proc (p)
    , osc1 (proc.analogTables, 4)
{
    filter.setNumChannels (2);
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

    updateParams (0);
    snapParams();
    updateParams (0);
    snapParams();
    //
	osc1.noteOn();


	env1.reset();
	env1.noteOn();
	
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
	env1.noteOn();
}

void SynthVoice::noteStopped (bool allowTailOff)
{
	env1.noteOff();

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

	filter.setSampleRate(newRate);

	lfo1.setSampleRate(newRate);

	noteSmoother.setSampleRate(newRate);
	env1.setSampleRate(newRate);
}

void SynthVoice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    updateParams (numSamples);

	// Run OSC
	gin::ScratchBuffer osc1buffer(2, numSamples);




	osc1.processAdding(currentMidiNote, osc1Params, osc1buffer);

    // Apply velocity
    float velocity = currentlyPlayingNote.noteOnVelocity.asUnsignedFloat();
    osc1buffer.applyGain (gin::velocityToGain (velocity, ampKeyTrack));

    // Apply filter
    if (proc.filterParams.enable->isOn())
        filter.process (osc1buffer);
    
    // Run ADSR
    env1.processMultiplying (osc1buffer);
    
    if (env1.getState() == gin::AnalogADSR::State::idle)
    {
        clearCurrentNote();
        stopVoice();
    }

    // Copy output to synth
    outputBuffer.addFrom (0, startSample, osc1buffer, 0, 0, numSamples);
    outputBuffer.addFrom (1, startSample, osc1buffer, 1, 0, numSamples);
    
    finishBlock (numSamples);
}

void SynthVoice::updateParams (int blockSize)
{
    auto note = getCurrentlyPlayingNote();
    
    proc.modMatrix.setPolyValue (*this, proc.modSrcNote, note.initialNote / 127.0f);

	currentMidiNote = noteSmoother.getCurrentValue() * 127.0f;
	if (glideInfo.glissando) currentMidiNote = (float)juce::roundToInt(currentMidiNote);
	currentMidiNote += float(note.totalPitchbendInSemitones);
	currentMidiNote += getValue(proc.osc1Params.coarse) + getValue(proc.osc1Params.fine);

    osc1Params.wave = gin::Wave::sine;

	osc1Params.pan = getValue(proc.osc1Params.pan);
	osc1Params.spread = getValue(proc.osc1Params.spread) / 100.0f;
	osc1Params.detune = getValue(proc.osc1Params.detune);
	osc1Params.gain = getValue(proc.osc1Params.level);
    
    ampKeyTrack = getValue (proc.env1Params.velocityTracking);

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
    env1.setDecay (getValue (proc.env1Params.decay));
    env1.setSustainLevel (getValue (proc.env1Params.sustain));
    env1.setRelease (fastKill ? 0.01f : getValue (proc.env1Params.release));
    
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

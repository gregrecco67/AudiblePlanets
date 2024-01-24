#include "SynthVoice.h"
#include "PluginProcessor.h"
#include <numbers>

using namespace std::numbers;
//==============================================================================

void SynthVoice::Oscillator::noteOn()
{
	for (int i = 0; i < 4; i++)
	{
		phases[i] = (phases[i] >= 0) ? phases[i] : 0.0f;
	}
}



SynthVoice::SynthVoice (APAudioProcessor& p)
    : proc (p)
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

	// use currentSampleRate to set up oscillators
	osc1.sampleRate = newRate;
	osc2.sampleRate = newRate;
	osc3.sampleRate = newRate;
	osc4.sampleRate = newRate;

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

	synthBuffer.setSize(2, numSamples, false, false, true);
	synthBuffer.clear();
	juce::dsp::AudioBlock<float> synthBlock{ synthBuffer }; // put buffer in a block
	juce::dsp::ProcessContextReplacing<float> synthContext{ synthBlock }; // put that block in a context

	// 2. get write pointers
	auto* bufferL = synthBuffer.getWritePointer(0);
	auto* bufferR = synthBuffer.getWritePointer(1); // "stereo"

	// Apply velocity
	float velocity = currentlyPlayingNote.noteOnVelocity.asUnsignedFloat() * baseAmplitude;
	auto gainLevel = juce::Decibels::decibelsToGain(getValue(proc.globalParams.level));

	// Apply filter -- we'll do this after orbital processing
	//if (proc.filterParams.enable->isOn())
	//	filter.process(osc1buffer);

	// Run ADSR
	

	auto algo = (int)getValue(proc.timbreParams.algo);
	auto demod = getValue(proc.timbreParams.demodmix);
	auto blend = getValue(proc.timbreParams.blend);
	
	envVals[0] = env1.process(numSamples);
	envVals[1] = env2.process(numSamples);
	envVals[2] = env3.process(numSamples);
	envVals[3] = env4.process(numSamples);

	auto equantVal = getValue(proc.timbreParams.equant);
	auto voiceLevel = juce::Decibels::decibelsToGain(getValue(proc.globalParams.level));


	for (int i = 0; i < numSamples; i++)
	{
		auto osc1Samples = osc1.getNextSamples();
		auto osc2Samples = osc2.getNextSamples();
		auto osc3Samples = osc3.getNextSamples();
		auto osc4Samples = osc4.getNextSamples();

		epi1.xL = envVals[oscEnvs[0]] * osc1Samples.cosL;
		epi1.yL = envVals[oscEnvs[0]] * osc1Samples.sinL;
		epi1.xR = envVals[oscEnvs[0]] * osc1Samples.cosR;
		epi1.yR = envVals[oscEnvs[0]] * osc1Samples.sinR;

		epi2.xL = envVals[oscEnvs[1]] * (epi1.xL + osc2Samples.cosL);
		epi2.yL = envVals[oscEnvs[1]] * (epi1.yL + osc2Samples.sinL);
		epi2.xR = envVals[oscEnvs[1]] * (epi1.xR + osc2Samples.cosR);
		epi2.yR = envVals[oscEnvs[1]] * (epi1.yR + osc2Samples.sinR);

		if (algo == 0 || algo == 1)
		{
			epi3.xL = envVals[oscEnvs[2]] * (epi2.xL + osc3Samples.cosL);
			epi3.yL = envVals[oscEnvs[2]] * (epi2.yL + osc3Samples.sinL);
			epi3.xR = envVals[oscEnvs[2]] * (epi2.xR + osc3Samples.cosR);
			epi3.yR = envVals[oscEnvs[2]] * (epi2.yR + osc3Samples.sinR);
		}
		else
		{
			epi3.xL = envVals[oscEnvs[2]] * (epi1.xL + osc3Samples.cosL);
			epi3.yL = envVals[oscEnvs[2]] * (epi1.yL + osc3Samples.sinL);
			epi3.xR = envVals[oscEnvs[2]] * (epi1.xR + osc3Samples.cosR);
			epi3.yR = envVals[oscEnvs[2]] * (epi1.yR + osc3Samples.sinR);
		}
		if (algo == 0 || algo == 2)
			epi4.xL = envVals[oscEnvs[3]] * (epi3.xL + osc4Samples.cosL);
			epi4.yL = envVals[oscEnvs[3]] * (epi3.yL + osc4Samples.sinL);
			epi4.xR = envVals[oscEnvs[3]] * (epi3.xR + osc4Samples.cosR);
			epi4.yR = envVals[oscEnvs[3]] * (epi3.yR + osc4Samples.sinR);
		if (algo == 1)
		{
			epi4.xL = envVals[oscEnvs[3]] * (epi2.xL + osc4Samples.cosL);
			epi4.yL = envVals[oscEnvs[3]] * (epi2.yL + osc4Samples.sinL);
			epi4.xR = envVals[oscEnvs[3]] * (epi2.xR + osc4Samples.cosR);
			epi4.yR = envVals[oscEnvs[3]] * (epi2.yR + osc4Samples.sinR);
		}
		if (algo == 3)
		{
			epi4.xL = envVals[oscEnvs[3]] * (epi1.xL + osc4Samples.cosL);
			epi4.yL = envVals[oscEnvs[3]] * (epi1.yL + osc4Samples.sinL);
			epi4.xR = envVals[oscEnvs[3]] * (epi1.xR + osc4Samples.cosR);
			epi4.yR = envVals[oscEnvs[3]] * (epi1.yR + osc4Samples.sinR);
		}

		// bodies' positions are set, now interpret them
		float atanAngle2L, atanAngle2R, atanAngle3L, atanAngle3R, atanAngle4L, atanAngle4R;
		float sine2L, sine2R, sine3L, sine3R, sine4L, sine4R;
		float square2L, square2R, square3L, square3R, square4L, square4R;
		float saw2L, saw2R, saw3L, saw3R, saw4L, saw4R;

		float sampleL, sampleR;
		if (algo == 0)
		{
			atanAngle4L = FastMath<float>::fastAtan2(epi4.xL, epi4.yL + equantVal);
			atanAngle4R = FastMath<float>::fastAtan2(epi4.xR, epi4.yR + equantVal);
			sine4L = FastMath<float>::fastSin(atanAngle4L);
			sine4R = FastMath<float>::fastSin(atanAngle4R);
			square4L = (atanAngle4L > 0.f) ? 1.0f : -1.0f;
			square4R = (atanAngle4R > 0.f) ? 1.0f : -1.0f;
			saw4L = atanAngle4L * inv_pi;
			saw4R = atanAngle4R * inv_pi;

			if (blend < 0.5)
			{
				sampleL = (sine4L * (1.f - blend * 2.0f) + square4L * blend * 2.0f);
				sampleR = (sine4R * (1.f - blend * 2.0f) + square4R * blend * 2.0f);
			}
			else
			{
				sampleL = (square4L * (1.0f - blend) * 2.0f + saw4L * (blend - 0.5f) * 2.f);
				sampleR = (square4R * (1.0f - blend) * 2.0f + saw4R * (blend - 0.5f) * 2.f);
			}

			float modSampleL = sampleL;
			float demodSampleL = sampleL;
			float modSampleR = sampleR;
			float demodSampleR = sampleR;

			auto atanDistanceL = (float)std::sqrt(epi4.xL * epi4.xL + (epi4.yL + equantVal) * (epi4.yL + equantVal) );
			auto atanDistanceR = (float)std::sqrt(epi4.xR * epi4.xR + (epi4.yR + equantVal) * (epi4.yR + equantVal) );
			demodSampleL *= atanDistanceL;
			demodSampleR *= atanDistanceR;

			// original recipe
			modSampleL *= envVals[oscEnvs[3]]; // i.e., getCurrentValue() for whatever env is selected for osc4 [idx=3], since we now construe the planet as the carrier(!)
			modSampleR *= envVals[oscEnvs[3]];

			// mix by demod amount
			sampleL = (modSampleL * (1.0f - demod) + demodSampleL * demod);
			sampleR = (modSampleR * (1.0f - demod) + demodSampleR * demod);

			bufferL[i] = sampleL * gainLevel;
			bufferR[i] = sampleR * gainLevel;
		}

		//synthBuffer.setSample(0, i, epi4.xL);
		//synthBuffer.setSample(1, i, epi4.xR);
	}


    if (   env1.getState() == gin::AnalogADSR::State::idle 
		&& env2.getState() == gin::AnalogADSR::State::idle
		&& env3.getState() == gin::AnalogADSR::State::idle
		&& env4.getState() == gin::AnalogADSR::State::idle
		)
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

	auto osc1Freq = gin::getMidiNoteInHertz(currentMidiNote) * (getValue(proc.osc1Params.coarse) + getValue(proc.osc1Params.fine));
	auto osc2Freq = gin::getMidiNoteInHertz(currentMidiNote) * (getValue(proc.osc2Params.coarse) + getValue(proc.osc2Params.fine));
	auto osc3Freq = gin::getMidiNoteInHertz(currentMidiNote) * (getValue(proc.osc3Params.coarse) + getValue(proc.osc3Params.fine));
	auto osc4Freq = gin::getMidiNoteInHertz(currentMidiNote) * (getValue(proc.osc4Params.coarse) + getValue(proc.osc4Params.fine));

	Oscillator::Params osc1Params;
	osc1Params.saw = (bool)getValue(proc.osc1Params.saw);
	osc1Params.tones = getValue(proc.osc1Params.tones);
	osc1Params.pan = getValue(proc.osc1Params.pan);
	osc1Params.spread = getValue(proc.osc1Params.spread) / 100.0f;
	osc1Params.detune = getValue(proc.osc1Params.detune);
	osc1Params.radius = getValue(proc.osc1Params.radius);
	oscEnvs[0] = (int)getValue(proc.osc1Params.env); // set env index for osc 1

	Oscillator::Params osc2Params;
	osc2Params.saw = (bool)getValue(proc.osc2Params.saw);
	osc2Params.tones = getValue(proc.osc2Params.tones);
	osc2Params.pan = getValue(proc.osc2Params.pan);
	osc2Params.spread = getValue(proc.osc2Params.spread) / 100.0f;
	osc2Params.detune = getValue(proc.osc2Params.detune);
	osc2Params.radius = getValue(proc.osc2Params.radius);
	oscEnvs[1] = (int)getValue(proc.osc2Params.env);
	
	Oscillator::Params osc3Params;
	osc3Params.saw = (bool)getValue(proc.osc3Params.saw);
	osc3Params.tones = getValue(proc.osc3Params.tones);
	osc3Params.pan = getValue(proc.osc3Params.pan);
	osc3Params.spread = getValue(proc.osc3Params.spread) / 100.0f;
	osc3Params.detune = getValue(proc.osc3Params.detune);
	osc3Params.radius = getValue(proc.osc3Params.radius);
	oscEnvs[2] = (int)getValue(proc.osc3Params.env);

	Oscillator::Params osc4Params;
	osc4Params.saw = (bool)getValue(proc.osc4Params.saw);
	osc4Params.tones = getValue(proc.osc4Params.tones);
	osc4Params.pan = getValue(proc.osc4Params.pan);
	osc4Params.spread = getValue(proc.osc4Params.spread) / 100.0f;
	osc4Params.detune = getValue(proc.osc4Params.detune);
	osc4Params.radius = getValue(proc.osc4Params.radius);
	oscEnvs[3] = (int)getValue(proc.osc4Params.env);
	
	// make this a member of global in pluginprocessor, etc.
    // ampKeyTrack = getValue (proc.env1Params.velocityTracking);

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


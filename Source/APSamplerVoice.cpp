#include "APSamplerVoice.h"
#include "PluginProcessor.h"

APSamplerVoice::APSamplerVoice(APAudioProcessor& p) : proc(p) {
	juce::ADSR::Parameters adsrParams{ 0.01f, 0.0f, 1.0f, 0.05f };
	adsr.setParameters(adsrParams);
}

void APSamplerVoice::noteStarted()
{
	if (sound == nullptr) {
		clearCurrentNote();
		return;
	}
	curNote = getCurrentlyPlayingNote();
	pitchStride = std::pow(2.0, (curNote.initialNote - sound->midiRootNote) / 12.0) 
		* sound->sourceSampleRate / getSampleRate();

	sourceSamplePosition = proc.samplerParams.start->getUserValue() * sound->length;
	lgain = curNote.noteOnVelocity.asUnsignedFloat();
	rgain = curNote.noteOnVelocity.asUnsignedFloat();

	proc.modMatrix.setPolyValue(*this, proc.modSrcVelocity, curNote.noteOnVelocity.asUnsignedFloat());
	proc.modMatrix.setPolyValue(*this, proc.modSrcTimbre, curNote.initialTimbre.asUnsignedFloat());
	proc.modMatrix.setPolyValue(*this, proc.modSrcPressure, curNote.pressure.asUnsignedFloat());

	fastKill = false;
	startVoice();

	if (glideInfo.fromNote >= 0 && (glideInfo.glissando || glideInfo.portamento))
	{
		noteSmoother.setTime(glideInfo.rate);
		noteSmoother.setValueUnsmoothed(glideInfo.fromNote / 127.0f);
		noteSmoother.setValue(curNote.initialNote / 127.0f);
	}
	else
	{
		noteSmoother.setValueUnsmoothed(curNote.initialNote / 127.0f);
	}

	juce::ScopedValueSetter<bool> svs(disableSmoothing, true);

	updateParams(0);
	snapParams();

	adsr.noteOn();
}

void APSamplerVoice::noteRetriggered()
{
	curNote = getCurrentlyPlayingNote();

	proc.modMatrix.setPolyValue(*this, proc.modSrcVelocity, curNote.noteOnVelocity.asUnsignedFloat());
	proc.modMatrix.setPolyValue(*this, proc.modSrcTimbre, curNote.initialTimbre.asUnsignedFloat());
	proc.modMatrix.setPolyValue(*this, proc.modSrcPressure, curNote.pressure.asUnsignedFloat());

	if (glideInfo.fromNote >= 0 && (glideInfo.glissando || glideInfo.portamento))
	{
		noteSmoother.setTime(glideInfo.rate);
		noteSmoother.setValue(curNote.initialNote / 127.0f);
	}
	else
	{
		noteSmoother.setValueUnsmoothed(curNote.initialNote / 127.0f);
	}

	updateParams(0);
}

void APSamplerVoice::noteStopped(bool allowTailOff)
{
	if (allowTailOff)
	{
		adsr.noteOff();
	}
	else
	{
		clearCurrentNote();
	}
}

void APSamplerVoice::setCurrentSampleRate(double newRate)
{
	juce::MPESynthesiserVoice::setCurrentSampleRate(newRate);
	if (newRate > 0.0)
		adsr.setSampleRate(newRate);
}

void APSamplerVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
	if (fastKill)
	{
		clearCurrentNote();
		return;
	}

	if (sound)
	{
		curNote = getCurrentlyPlayingNote();
		float dummy;
		float remainder = std::modf(currentMidiNote, &dummy);

		pitchStride = std::pow(2.0, (curNote.initialNote - sound->midiRootNote) / 12.0)
			* sound->sourceSampleRate / getSampleRate();
		pitchStride *= MTS_RetuningAsRatio(proc.client, curNote.initialNote, curNote.midiChannel);
		pitchStride *= std::pow(1.05946309436f, curNote.totalPitchbendInSemitones * (proc.globalParams.pitchbendRange->getUserValue()/2.0f) + remainder);
		auto& data = *sound->data;
		const float* const inL = data.getReadPointer(0);
		const float* const inR = data.getNumChannels() > 1 ? data.getReadPointer(1) : nullptr;

		float* outL = outputBuffer.getWritePointer(0, startSample);
		float* outR = outputBuffer.getNumChannels() > 1 ? outputBuffer.getWritePointer(1, startSample) : nullptr;

		while (--numSamples >= 0)
		{
			auto pos = (int)sourceSamplePosition;
			auto alpha = (float)(sourceSamplePosition - pos);
			auto invAlpha = 1.0f - alpha;

			// just using a very simple linear interpolation here..
			float l = (inL[pos] * invAlpha + inL[pos + 1] * alpha);
			float r = (inR != nullptr) ? (inR[pos] * invAlpha + inR[pos + 1] * alpha)
				: l;

			auto envelopeValue = adsr.getNextSample();

			l *= lgain * envelopeValue;
			r *= rgain * envelopeValue;

			if (outR != nullptr)
			{
				*outL++ += l;
				*outR++ += r;
			}
			else
			{
				*outL++ += (l + r) * 0.5f;
			}

			sourceSamplePosition += pitchStride;

			if (sourceSamplePosition >= int(sound->length * proc.samplerParams.end->getUserValue()) && proc.samplerParams.loop->isOn())
			{
				sourceSamplePosition = int(sound->length * proc.samplerParams.start->getUserValue());
				continue;
			}

			if (sourceSamplePosition > int(sound->length * proc.samplerParams.end->getUserValue()) || !adsr.isActive())
			{
				clearCurrentNote();
				stopVoice();
				break;
			}
		}
	}
}

void APSamplerVoice::updateParams(int blockSize)
{
}

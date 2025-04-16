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

#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <gin_dsp/gin_dsp.h>
#include <gin_plugin/gin_plugin.h>
#include "libMTSClient.h"

class APSamplerSound {
public:
    juce::String name;
    std::unique_ptr<juce::AudioBuffer<float>> data;
    double sourceSampleRate;
    int length = 0, midiRootNote = 0;
};

class APAudioProcessor;

//==============================================================================
class APSamplerVoice : public gin::SynthesiserVoice,
                   public gin::ModVoice
{
public:
    APSamplerVoice(APAudioProcessor& p);
    
	void noteStarted() override;
	void noteRetriggered() override;
	void noteStopped(bool allowTailOff) override;

    void notePressureChanged() override {}
    void noteTimbreChanged() override {}
    void notePitchbendChanged() override {}
    void noteKeyStateChanged() override {}
    
	void setCurrentSampleRate(double newRate) override;

	void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

    float getCurrentNote() override { return noteSmoother.getCurrentValue() * 127.0f; }

	bool isVoiceActive() override { return isActive(); }

    void setSound(APSamplerSound* sound_) { sound = sound_; }
  
private:
    APAudioProcessor& proc;

    float currentMidiNote = -1;
    
    float freq{0.f};
    
    float baseAmplitude = 0.12f;

    gin::EasedValueSmoother<float> noteSmoother;
    
    friend class APSampler;
    juce::MPENote curNote;
    
	APSamplerSound* sound{ nullptr };

	double pitchStride;
	juce::ADSR adsr;
    
	float sourceSamplePosition = 0;
	float lgain = 0, rgain = 0;

	bool hasLooped{ false };
};


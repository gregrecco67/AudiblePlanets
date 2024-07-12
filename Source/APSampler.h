#pragma once
#include <gin_dsp/gin_dsp.h>
#include <gin_plugin/gin_plugin.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include "APSamplerVoice.h"

class APAudioProcessor;

class APSampler : public gin::Synthesiser
{
public:
    APSampler(APAudioProcessor& proc_);
    ~APSampler() override = default;

    void handleMidiEvent(const juce::MidiMessage& m) override;
    bool loadSound(const juce::String& path);
	void clearSound();
    void updateBaseNote(int note);
    
    APSamplerSound sound;
    APAudioProcessor& proc;
    juce::AudioFormatManager formatManager;
    juce::AudioFormatReader* reader{ nullptr };
};

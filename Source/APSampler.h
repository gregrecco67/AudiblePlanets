#pragma once
#include <JuceHeader.h>
#include "APSamplerVoice.h"

class APAudioProcessor;

class APSampler : public gin::Synthesiser
{
public:
    APSampler(APAudioProcessor& proc_);
    ~APSampler() override = default;

    void handleMidiEvent(const juce::MidiMessage& m) override;
    bool loadSound(const juce::String& path);
    bool loadSoundFile(juce::File& file);
    void updateRange(BigInteger range);
    void updateBaseNote(int note);
    bool shouldPlayNote(int note);
    
    APSamplerSound sound;
    APAudioProcessor& proc;
    juce::AudioFormatManager formatManager;
    juce::AudioFormatReader* reader{ nullptr };
};

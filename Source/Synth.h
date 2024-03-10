#pragma once
#include <JuceHeader.h>
#include <gin/gin.h>
#include "SynthVoice2.h"

class APAudioProcessor;

class APSynth : public gin::Synthesiser
{
public:
    APSynth(APAudioProcessor& proc_);
    ~APSynth() override = default;
    
    void handleMidiEvent(const juce::MidiMessage& m) override;
    juce::Array<float> getLiveFilterCutoff();
    
private:
    APAudioProcessor& proc;
    
};

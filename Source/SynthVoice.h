#pragma once

#include <JuceHeader.h>

class APAudioProcessor;

//==============================================================================
class SynthVoice : public gin::SynthesiserVoice,
                   public gin::ModVoice
{
public:
    SynthVoice (APAudioProcessor& p);
    
    void noteStarted() override;
    void noteRetriggered() override;
    void noteStopped (bool allowTailOff) override;

    void notePressureChanged() override;
    void noteTimbreChanged() override;
    void notePitchbendChanged() override    {}
    void noteKeyStateChanged() override     {}
    
    void setCurrentSampleRate (double newRate) override;

    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

    bool isVoiceActive() override;

    float getFilterCutoffNormalized();
  
private:
    void updateParams (int blockSize);

    APAudioProcessor& proc;

    gin::BLLTVoicedStereoOscillator osc1;

    gin::Filter filter;
    
    gin::LFO lfo1;

    gin::AnalogADSR env1;

    float currentMidiNote;
    gin::VoicedStereoOscillatorParams osc1Params, osc2Params, osc3Params, osc4Params;
    
    gin::EasedValueSmoother<float> noteSmoother;
    
    float ampKeyTrack = 1.0f;
};

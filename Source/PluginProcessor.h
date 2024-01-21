#pragma once

#include <JuceHeader.h>
#include <gin/gin.h>
#include "SynthVoice.h"
#include "Envelope.h"

//==============================================================================
class APAudioProcessor : public gin::Processor,
    public gin::Synthesiser
{
public:
    //==============================================================================
    APAudioProcessor();
    ~APAudioProcessor() override;

    bool supportsMPE() const override { return true; }

    //==============================================================================
    void reset() override;
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    void updateParams(int blockSize);
    void setupModMatrix();

    //==============================================================================
    void handleMidiEvent(const juce::MidiMessage& m) override;
    //==============================================================================
    juce::Array<float> getLiveFilterCutoff();

    void applyEffects(juce::AudioSampleBuffer& buffer);

    // Voice Params
    struct OSCParams
    {
        OSCParams() = default;

        gin::Parameter::Ptr coarse, fine, radius, tones, level, detune, spread, pan, env;

        void setup(APAudioProcessor& p, juce::String number);

        JUCE_DECLARE_NON_COPYABLE(OSCParams)
    };

    struct FilterParams
    {
        FilterParams() = default;

        gin::Parameter::Ptr enable, type, keyTracking, frequency, resonance;

        void setup(APAudioProcessor& p);

        JUCE_DECLARE_NON_COPYABLE(FilterParams)
    };

    struct LFOParams
    {
        LFOParams() = default;

        gin::Parameter::Ptr enable, sync, wave, rate, beat, depth, phase, offset, fade, delay, level;

        void setup(APAudioProcessor& p, String num);
        int num;
        JUCE_DECLARE_NON_COPYABLE(LFOParams)
    };

    struct ADSRParams
    {
        ADSRParams() = default;

        gin::Parameter::Ptr attack, decay, sustain, release, velocityTracking;

        void setup(APAudioProcessor& p, juce::String number);

        JUCE_DECLARE_NON_COPYABLE(ADSRParams)
    };

    //struct EnvelopeParams
    //{
    //    EnvelopeParams() = default;

    //    gin::Parameter::Ptr attack, decay, sustain, release, velocityTracking;

    //    void setup (APAudioProcessor& p, juce::String num);

    //    JUCE_DECLARE_NON_COPYABLE (EnvelopeParams)
    //};

    // Global Params
    struct GlobalParams
    {
        GlobalParams() = default;

        gin::Parameter::Ptr mono, glideMode, glideRate, legato, level, voices, mpe;

        void setup(APAudioProcessor& p);

        JUCE_DECLARE_NON_COPYABLE(GlobalParams)
    };

    //==============================================================================
    gin::ModSrcId modSrcPressure, modSrcTimbre, modScrPitchBend, modSrcLFO1, modSrcLFO2, modSrcLFO3, modSrcLFO4,
        modSrcNote, modSrcVelocity, modSrcMonoLFO1, modSrcMonoLFO2, modSrcMonoLFO3, modSrcMonoLFO4,
        modSrcEnv1, modSrcEnv2, modSrcEnv3, modSrcEnv4;

    //==============================================================================

    OSCParams osc1Params, osc2Params, osc3Params, osc4Params;
    LFOParams lfo1Params, lfo2Params, lfo3Params, lfo4Params;

    //    EnvelopeParams env1Params, env2Params, env3Params, env4Params;
    ADSRParams env1Params, env2Params, env3Params, env4Params;
    FilterParams filterParams;

    GlobalParams globalParams;

    //==============================================================================
    gin::StereoDelay stereoDelay{ 120.1 };
    gin::GainProcessor outputGain;


    gin::BandLimitedLookupTables analogTables;

    //==============================================================================
    gin::ModMatrix modMatrix;

    gin::LFO lfo1, lfo2, lfo3, lfo4;
    std::array<gin::LFO, 4> monoLFOs{lfo1, lfo2, lfo3, lfo4};
    std::array<gin::ModSrcId, 4> monoLFOIds{modSrcMonoLFO1, modSrcMonoLFO2, modSrcMonoLFO3, modSrcMonoLFO4};
    std::array<gin::ModSrcId, 4> lfoIds{modSrcLFO1, modSrcLFO2, modSrcLFO3, modSrcLFO4};

    juce::AudioPlayHead* playhead = nullptr;
    bool presetLoaded = false;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (APAudioProcessor)
};

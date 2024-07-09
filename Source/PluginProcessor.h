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

#include <JuceHeader.h>
#include <gin/gin.h>
#include "Envelope.h"
#include "FXProcessors.h"
#include "Synth.h"
#include "AuxSynth.h"
#include "APSampler.h"

//==============================================================================
class APAudioProcessor : public gin::Processor
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
    bool isBusesLayoutSupported (const BusesLayout& layouts) const;
    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    void updateParams(int blockSize);
    void setupModMatrix();

	void stateUpdated() override;
	void updateState() override;

    void updatePitchbend();
    
    //==============================================================================
    
    //==============================================================================
    juce::Array<float> getLiveFilterCutoff();

    void applyEffects(juce::AudioSampleBuffer& buffer);

    // Voice Params
    struct OSCParams
    {
        OSCParams() = default;

        gin::Parameter::Ptr coarse, fine, volume, tones, detune, spread, pan, env, saw, fixed, phase;

        void setup(APAudioProcessor& p, juce::String number);
		int num;
        JUCE_DECLARE_NON_COPYABLE(OSCParams)
    };

    struct FilterParams
    {
        FilterParams() = default;

        gin::Parameter::Ptr type, keyTracking, frequency, resonance;

        void setup(APAudioProcessor& p);

        JUCE_DECLARE_NON_COPYABLE(FilterParams)
    };

    struct LFOParams
    {
        LFOParams() = default;

        gin::Parameter::Ptr enable, sync, wave, rate, beat, depth, phase, offset, fade, delay, level, env;

        void setup(APAudioProcessor& p, String num);
        int num;
        JUCE_DECLARE_NON_COPYABLE(LFOParams)
    };

    struct ENVParams
    {
        ENVParams() = default;

        gin::Parameter::Ptr attack, decay, sustain, release,
			acurve, drcurve, syncrepeat, time, duration;

        void setup(APAudioProcessor& p, juce::String number);
		int num;
        JUCE_DECLARE_NON_COPYABLE(ENVParams)
    };

	struct TimbreParams
	{
		TimbreParams() = default;

		gin::Parameter::Ptr equant, pitch, blend, demodmix, algo, demodVol;

		void setup(APAudioProcessor& p);

		JUCE_DECLARE_NON_COPYABLE(TimbreParams)
	};

	struct MSEGParams
	{
		MSEGParams() = default;
		gin::Parameter::Ptr sync, rate, beat, depth, offset, phase, enable, xgrid, ygrid, loop, draw, drawmode;
		void setup(APAudioProcessor& p, juce::String number);
		int num;
		JUCE_DECLARE_NON_COPYABLE(MSEGParams)
	};

    // Global Params
    struct GlobalParams
    {
        GlobalParams() = default;

        gin::Parameter::Ptr mono, glideMode, glideRate, legato, level, mpe, velSens, pitchbendRange, sidechainEnable, squash;

        void setup(APAudioProcessor& p);

        JUCE_DECLARE_NON_COPYABLE(GlobalParams)
    };

	struct WaveshaperParams
	{
		WaveshaperParams() = default;

		gin::Parameter::Ptr enable, drive, gain, type, dry, wet, highshelf, hsq, lp;

		void setup(APAudioProcessor& p);
        int pos{-1};
		JUCE_DECLARE_NON_COPYABLE(WaveshaperParams)
	};

	struct CompressorParams
	{
		CompressorParams() = default;

		gin::Parameter::Ptr enable, threshold, ratio, attack, release, knee, input, output, type;

		void setup(APAudioProcessor& p);
        int pos{-1};
		JUCE_DECLARE_NON_COPYABLE(CompressorParams)
	};

	struct StereoDelayParams
	{
		StereoDelayParams() = default;

		gin::Parameter::Ptr enable, timeleft, timeright, beatsleft, beatsright, temposync, freeze, pingpong, feedback, dry, wet, cutoff;

		void setup(APAudioProcessor& p);
        int pos{-1};
		JUCE_DECLARE_NON_COPYABLE(StereoDelayParams)
	};

	struct ChorusParams
	{
		ChorusParams() = default;

		gin::Parameter::Ptr enable, rate, depth, delay, feedback, dry, wet;

		void setup(APAudioProcessor& p);
        int pos{-1};
		JUCE_DECLARE_NON_COPYABLE(ChorusParams)
	};

	struct ReverbParams
	{
		ReverbParams() = default;

		gin::Parameter::Ptr enable, size, decay, damping, lowpass, predelay, dry, wet;

		void setup(APAudioProcessor& p);
        int pos{-1};
		JUCE_DECLARE_NON_COPYABLE(ReverbParams)
	};

	struct MBFilterParams
	{
		MBFilterParams() = default;

		gin::Parameter::Ptr enable, lowshelffreq, lowshelfgain, lowshelfq, peakfreq, peakgain, peakq, highshelffreq, highshelfgain, highshelfq;

		void setup(APAudioProcessor& p);
        int pos{-1};
		JUCE_DECLARE_NON_COPYABLE(MBFilterParams)
	};

	struct RingModParams
	{
		RingModParams() = default;

		gin::Parameter::Ptr enable, modfreq1, shape1, mix1, modfreq2, shape2, mix2, spread, lowcut, highcut;

		void setup(APAudioProcessor& p);
        int pos{-1};
		JUCE_DECLARE_NON_COPYABLE(RingModParams)
	};


	struct GainParams
	{

		GainParams() = default;

		gin::Parameter::Ptr enable, gain;

		void setup(APAudioProcessor& p);
        int pos{-1};
		JUCE_DECLARE_NON_COPYABLE(GainParams)
	};

    struct FXOrderParams
    {
        
        FXOrderParams() = default;
        
        gin::Parameter::Ptr fxa1, fxa2, fxa3, fxa4, fxb1, fxb2, fxb3, fxb4, chainAtoB, laneAGain, laneBGain,
			laneAType, laneAFreq, laneARes, laneBType, laneBFreq, laneBRes, laneAPrePost, laneAPan, laneBPrePost, laneBPan;
        
        void setup(APAudioProcessor& p);
        
        JUCE_DECLARE_NON_COPYABLE(FXOrderParams)
    };
    
	struct MacroParams {
		MacroParams() = default;

		gin::Parameter::Ptr macro1, macro2, macro3, macro4, learning, macro1cc, macro2cc, macro3cc, macro4cc;
		void setup(APAudioProcessor& p);

		JUCE_DECLARE_NON_COPYABLE(MacroParams)
	};

	struct AuxParams {
		AuxParams() = default;

		gin::Parameter::Ptr enable, wave, env, octave, volume, detune, spread, prefx, filtertype,
			filtercutoff, filterres, filterkeytrack, ignorepb;
		void setup(APAudioProcessor& p);

		JUCE_DECLARE_NON_COPYABLE(AuxParams)
	};

	struct SamplerParams {
		SamplerParams() = default;

		gin::Parameter::Ptr enable, volume, loop, key, start, end, loopstart, loopend;
		void setup(APAudioProcessor& p);

		JUCE_DECLARE_NON_COPYABLE(SamplerParams)
	};

	void loadSample(const juce::String& path);
    
    
	gin::ProcessorOptions getOptions();

    //==============================================================================
    gin::ModSrcId modSrcPressure, modSrcTimbre, modSrcMonoPitchbend, modSrcNote, modSrcVelocity,
		modSrcLFO1, modSrcLFO2, modSrcLFO3, modSrcLFO4,
		modSrcMonoLFO1, modSrcMonoLFO2, modSrcMonoLFO3, modSrcMonoLFO4,
        modSrcEnv1, modSrcEnv2, modSrcEnv3, modSrcEnv4,
		modSrcModwheel, modPolyAT, modSrcMSEG1, modSrcMSEG2, modSrcMSEG3, modSrcMSEG4,
		macroSrc1, macroSrc2, macroSrc3;

    //==============================================================================

	OSCParams osc1Params, osc2Params, osc3Params, osc4Params;
    LFOParams lfo1Params, lfo2Params, lfo3Params, lfo4Params;
    ENVParams env1Params, env2Params, env3Params, env4Params; 
    TimbreParams timbreParams;
    FilterParams filterParams;
    GlobalParams globalParams;
	GainParams gainParams;
	WaveshaperParams waveshaperParams;
	CompressorParams compressorParams;
	StereoDelayParams stereoDelayParams;
	ChorusParams chorusParams;

	ReverbParams reverbParams;
	MBFilterParams mbfilterParams;
	RingModParams ringmodParams;
	FXOrderParams fxOrderParams;
	MSEGParams mseg1Params, mseg2Params, mseg3Params, mseg4Params;
	MacroParams macroParams;
	AuxParams auxParams;
	SamplerParams samplerParams;

    //==============================================================================
	GainProcessor effectGain;
	WaveShaperProcessor waveshaper;
	gin::Dynamics compressor;
	StereoDelayProcessor stereoDelay;
	ChorusProcessor chorus;
	PlateReverb<float, uint32_t> reverb;
	MBFilterProcessor mbfilter;
	RingModulator ringmod;
	juce::dsp::Limiter<float> limiter;

    gin::GainProcessor outputGain;

    //==============================================================================
    gin::ModMatrix modMatrix;

    gin::LFO lfo1, lfo2, lfo3, lfo4;
	gin::MSEG::Data mseg1Data, mseg2Data, mseg3Data, mseg4Data;
	std::array<gin::LFO*, 4> monoLFOs{ &lfo1, &lfo2, &lfo3, &lfo4 };
	std::array<gin::ModSrcId*, 4> lfoIds{ &modSrcMonoLFO1, &modSrcMonoLFO2, &modSrcMonoLFO3, &modSrcMonoLFO4 };
    juce::AudioPlayHead* playhead = nullptr;
    bool presetLoaded = false;
	gin::Filter laneAFilter, laneBFilter;
	juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> dcFilter;
	SmoothedValue<float, ValueSmoothingTypes::Multiplicative> laneAFilterCutoff, laneBFilterCutoff;
	int fxa1, fxa2, fxa3, fxa4, fxb1, fxb2, fxb3, fxb4; // effect choices
	std::unordered_set<int> activeEffects;

	gin::LevelTracker levelTracker;
    APSynth synth;
	juce::AudioBuffer<float> sidechainBuffer;
	juce::AudioBuffer<float> sidechainSlice;
	juce::AudioBuffer<float> auxBuffer;
	juce::AudioBuffer<float> auxSlice;
	juce::AudioBuffer<float> samplerBuffer;
	juce::AudioBuffer<float> samplerSlice;

	MTSClient* client;
	String scaleName, learningLabel;


	AuxSynth auxSynth;
	gin::BandLimitedLookupTables analogTables;

	//juce::Synthesiser sampler;
    APSampler sampler;
	juce::AudioFormatManager formatManager;
	juce::AudioFormatReader* reader{ nullptr };

	const int numVoices = 8;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (APAudioProcessor)
};

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

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>
#include <gin_dsp/gin_dsp.h>
#include <gin_plugin/gin_plugin.h>
#include "Envelope.h"
#include "FXProcessors.h"
#include "Synth.h"
#include "AuxSynth.h"
#include <random>
#include "hiir/PolyphaseIir2Designer.h"
#if APPLE
    #include "hiir/Downsampler2x4Neon.h"
    #include "hiir/Downsampler2xNeon.h"
#endif

#if USE_SSE
	#include "hiir/Downsampler2xSse.h"
#endif
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
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    void updateParams(int blockSize);
    void setupModMatrix();

	void stateUpdated() override;
	void updateState() override;

    void updatePitchbend();

    void dnsplStage1(const juce::dsp::AudioBlock<float>& inputBlock, juce::dsp::AudioBlock<float>& outputBlock);
    void dnsplStage2(const juce::dsp::AudioBlock<float>& inputBlock, juce::dsp::AudioBlock<float>& outputBlock);
        
    //==============================================================================
    
    //==============================================================================
    juce::Array<float> getLiveFilterCutoff();

    void applyEffects(juce::AudioSampleBuffer& buffer);

    // Voice Params
    struct OSCParams
    {
        OSCParams() = default;

        gin::Parameter::Ptr coarse, fine, volume, env, wave, fixed, phase;

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

        void setup(APAudioProcessor& p, juce::String num);
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

		gin::Parameter::Ptr equant, pitch, algo;

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

        gin::Parameter::Ptr mono, glideMode, glideRate, legato, level, mpe, velSens, pitchbendRange;

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

	struct LadderParams 
	{
		LadderParams() = default;
		gin::Parameter::Ptr cutoff, reso, drive, type, gain;
		void setup(APAudioProcessor& p);
		int pos{-1};
		JUCE_DECLARE_NON_COPYABLE(LadderParams)
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

		gin::Parameter::Ptr macro1, macro2, macro3, learning, macro1cc, macro2cc, macro3cc;
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

	void newRand();
	gin::ProcessorOptions getOptions();

    //==============================================================================
    gin::ModSrcId modSrcPressure, modSrcTimbre, modSrcMonoPitchbend, modSrcNote, modSrcVelocity, modSrcVelOff,
		modSrcLFO1, modSrcLFO2, modSrcLFO3, modSrcLFO4,
		modSrcMonoLFO1, modSrcMonoLFO2, modSrcMonoLFO3, modSrcMonoLFO4,
        modSrcEnv1, modSrcEnv2, modSrcEnv3, modSrcEnv4,
		modSrcModwheel, modPolyAT, modSrcMSEG1, modSrcMSEG2, modSrcMSEG3, modSrcMSEG4,
		macroSrc1, macroSrc2, macroSrc3,
		randSrc1Mono, randSrc1Poly, randSrc2Mono, randSrc2Poly;

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
	LadderParams ladderParams;
	ReverbParams reverbParams;
	MBFilterParams mbfilterParams;
	RingModParams ringmodParams;
	FXOrderParams fxOrderParams;
	MSEGParams mseg1Params, mseg2Params, mseg3Params, mseg4Params;
	MacroParams macroParams;
	AuxParams auxParams;

    //==============================================================================
	GainProcessor effectGain;
	WaveShaperProcessor waveshaper;
	gin::Dynamics compressor;
	StereoDelayProcessor stereoDelay;
	ChorusProcessor chorus;
	PlateReverb<float, uint32_t> reverb;
	MBFilterProcessor mbfilter;
	RingModulator ringmod;
	LadderFilterProcessor ladder;
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
	juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> laneAFilterCutoff, laneBFilterCutoff;
	int fxa1, fxa2, fxa3, fxa4, fxb1, fxb2, fxb3, fxb4; // effect choices
	std::unordered_set<int> activeEffects;

	gin::LevelTracker levelTracker{20.f};
    APSynth synth;
	juce::AudioBuffer<float> auxBuffer;
	juce::AudioBuffer<float> auxSlice;
	juce::AudioBuffer<float> synthBuffer;    // 2x
	juce::AudioBuffer<float> preSynthBuffer; // 4x

	MTSClient* client;
	juce::String scaleName, learning;

	AuxSynth auxSynth;
	gin::BandLimitedLookupTables analogTables;
	gin::BandLimitedLookupTables upsampledTables;

	const int numVoices = 8;
	
	int tillReset{0};
	const int resetVal{2000};
	struct VizInfo {
		float defRat, epi1Rat, epi2Rat, epi3Rat,
		equant, defRad, epi1Rad, epi2Rad, epi3Rad,
		algo;
	} viz, viz2;

	std::random_device rd;
	std::mt19937 gen{ rd() };
	std::uniform_real_distribution<> dist{ -1.f, 1.f };
	
	// antialiasing downsampling filter stuff
    static constexpr int nbr_coefs1 = 3;
    static constexpr int nbr_coefs2 = 8;
    double coefs1 [nbr_coefs1];
    double coefs2 [nbr_coefs2];
    
#if APPLE
    // hiir::Downsampler2x4Neon <nbr_coefs1> dspl1L, dspl1R;
    // hiir::Downsampler2x4Neon <nbr_coefs2> dspl2L, dspl2R;
    hiir::Downsampler2xNeon <nbr_coefs1> dspl1L, dspl1R;
    hiir::Downsampler2xNeon <nbr_coefs2> dspl2L, dspl2R;
#endif
    
#if USE_SSE
	hiir::Downsampler2xSse <nbr_coefs1> dspl1L, dspl1R;
	hiir::Downsampler2xSse <nbr_coefs2> dspl2L, dspl2R;
#endif

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (APAudioProcessor)
};

#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>


struct DownsamplingStage;

template <typename SampleType>
class  Downsampling
{
public:
    enum FilterType
    {
        filterHalfBandFIREquiripple = 0,
        filterHalfBandPolyphaseIIR,
        numFilterTypes
    };

    explicit Downsampling (size_t numChannels = 1);

    Downsampling (size_t numChannels,
                  size_t factor,
                  FilterType type,
                  bool isMaxQuality = true,
                  bool useIntegerLatency = false);

    ~Downsampling();

    void setUsingIntegerLatency (bool shouldUseIntegerLatency) noexcept;
    float getLatencyInSamples() const noexcept;

    size_t getDownsamplingFactor() const noexcept;

    void initProcessing (size_t maximumNumberOfSamplesBeforeDownsampling);
    void reset() noexcept;

    void processSamplesDown (juce::dsp::AudioBlock<float>& outputBlock) noexcept;

    void addDownsamplingStage (FilterType,
                               float normalisedTransitionWidthUp,   float stopbandAmplitudedBUp,
                               float normalisedTransitionWidthDown, float stopbandAmplitudedBDown);

							   void addDummyDownsamplingStage();

    void clearDownsamplingStages();

    //==============================================================================
    size_t factorDownsampling = 1;
    size_t numChannels = 1;


private:
    //==============================================================================
    void updateDelayLine();
    float getUncompensatedLatency() const noexcept;

    //==============================================================================
    juce::OwnedArray<DownsamplingStage> stages;
    bool isReady = false, shouldUseIntegerLatency = false;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Thiran> delay { 8 };
    float fractionalDelay = 0;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Downsampling)
};
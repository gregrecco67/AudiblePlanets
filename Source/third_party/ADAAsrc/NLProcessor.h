#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include "BaseNL.h"

class NLProcessor
{
public:
    NLProcessor();

    void prepare (double sampleRate, int samplesPerBlock);
    void processBlock (juce::dsp::ProcessContextReplacing<float>& ctxt);

    float getLatencySamples() const noexcept;

private:
    std::atomic<float>* nlParam = nullptr;

    using NLSet = std::vector<std::vector<std::unique_ptr<BaseNL>>>;
    std::vector<NLSet> nlProcs;

    const size_t nChannels;

    double mySampleRate = 44100.0;
    int mySamplesPerBlock = 128;
    int prevOS = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NLProcessor)
};

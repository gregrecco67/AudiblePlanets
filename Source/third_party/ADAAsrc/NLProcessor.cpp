#include "NLProcessor.h"
#include "TanhNL.h"
#include "HardClip.h"
#include "ADAA/ADAA2LUT.h"

template <typename T>
std::vector<std::unique_ptr<BaseNL>> getChannelProcs (int numChannels)
{
    std::vector<std::unique_ptr<BaseNL>> chProcs;
    for (size_t ch = 0; ch < numChannels; ++ch)
        chProcs.push_back (std::make_unique<T>());

    return chProcs;
}

NLProcessor::NLProcessor () :
    nChannels (2)
{

    NLSet hcProcs;
    hcProcs.push_back (getChannelProcs<HardClip<ADAA2LUT<(1 << 18)>>> (nChannels));
    nlProcs.push_back (std::move (hcProcs));

    NLSet tanhProcs;
    tanhProcs.push_back (getChannelProcs<TanhNL<ADAA2LUT<(1 << 18)>>> (nChannels));
    nlProcs.push_back (std::move (tanhProcs));

}

float NLProcessor::getLatencySamples() const noexcept
{

    return 0.f;
}

void NLProcessor::prepare (double sampleRate, int samplesPerBlock)
{
    mySampleRate = sampleRate;
    mySamplesPerBlock = samplesPerBlock;

    for (auto& nlSet : nlProcs)
    {
        for (auto& nl : nlSet)
        {
            for (size_t ch = 0; ch < nChannels; ++ch)
                nl[ch]->prepare (sampleRate, samplesPerBlock);
        }
    }
}

void NLProcessor::processBlock (juce::dsp::ProcessContextReplacing<float>& ctxt)
{
    auto& osBlock = ctxt.getOutputBlock();
    auto numSamples = osBlock.getNumSamples();

    auto& curNLSet = nlProcs[0];
    auto& curNLProc = curNLSet[0];
    for (size_t ch = 0; ch < osBlock.getNumChannels(); ++ch)
        curNLProc[ch]->processBlock (osBlock.getChannelPointer (ch), (int) osBlock.getNumSamples());

}

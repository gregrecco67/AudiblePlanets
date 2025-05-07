#include "Downsampling.h"

/** Abstract class for the provided oversampling stages used internally in
    the Downsampling class.
*/
template <typename SampleType>
struct Downsampling<SampleType>::DownsamplingStage<SampleType>
{
    DownsamplingStage (size_t numChans, size_t newFactor)  : numChannels (numChans), factor (newFactor) {}
    virtual ~DownsamplingStage() {}

    virtual float getLatencyInSamples() const = 0;
    virtual void initProcessing (size_t maximumNumberOfSamplesBeforeDownsampling)
    {
        buffer.setSize (static_cast<int> (numChannels),
                        static_cast<int> (maximumNumberOfSamplesBeforeDownsampling * factor),
                        false, false, true);
    }

    virtual void reset()
    {
        buffer.clear();
    }

    juce::dsp::AudioBlock<SampleType> getProcessedSamples (size_t numSamples)
    {
        return juce::dsp::AudioBlock<SampleType> (buffer).getSubBlock (0, numSamples);
    }

    virtual void processSamplesUp   (const juce::dsp::AudioBlock<const SampleType>&) = 0;
    virtual void processSamplesDown (juce::dsp::AudioBlock<SampleType>&) = 0;

    juce::AudioBuffer<float> buffer;
    size_t numChannels, factor;
};


//==============================================================================
/** Dummy oversampling stage class which simply copies and pastes the input
    signal, which could be equivalent to a "one time" oversampling processing.
*/
template <typename SampleType>
struct DownsamplingDummy final : public Downsampling<SampleType>::DownsamplingStage
{
    using ParentType = typename Downsampling<SampleType>::DownsamplingStage;

    DownsamplingDummy (size_t numChans) : ParentType (numChans, 1) {}

    //==============================================================================
    float getLatencyInSamples() const override
    {
        return 0;
    }

    void processSamplesDown (juce::dsp::AudioBlock<SampleType>& outputBlock) override
    {
        jassert (outputBlock.getNumChannels() <= static_cast<size_t> (ParentType::buffer.getNumChannels()));
        jassert (outputBlock.getNumSamples() * ParentType::factor <= static_cast<size_t> (ParentType::buffer.getNumSamples()));

        outputBlock.copyFrom (ParentType::getProcessedSamples (outputBlock.getNumSamples()));
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DownsamplingDummy)
};

//==============================================================================
/** Downsampling stage class performing 2 times oversampling using the Filter
    Design FIR Equiripple method. The resulting filter is linear phase,
    symmetric, and has every two samples but the middle one equal to zero,
    leading to specific processing optimizations.
*/
template <typename SampleType>
struct Downsampling2TimesEquirippleFIR final : public Downsampling<SampleType>::DownsamplingStage
{
    using ParentType = typename Downsampling<SampleType>::DownsamplingStage;

    Downsampling2TimesEquirippleFIR (size_t numChans,
                                     float normalisedTransitionWidthUp,
                                     float stopbandAmplitudedBUp,
                                     float normalisedTransitionWidthDown,
                                     float stopbandAmplitudedBDown)
        : ParentType (numChans, 2)
    {
        coefficientsUp   = *juce::dsp::FilterDesign<float>::designFIRLowpassHalfBandEquirippleMethod (normalisedTransitionWidthUp,   stopbandAmplitudedBUp);
        coefficientsDown = *juce::dsp::FilterDesign<float>::designFIRLowpassHalfBandEquirippleMethod (normalisedTransitionWidthDown, stopbandAmplitudedBDown);

        auto N = coefficientsUp.getFilterOrder() + 1;
        stateUp.setSize (static_cast<int> (this->numChannels), static_cast<int> (N));

        N = coefficientsDown.getFilterOrder() + 1;
        auto Ndiv2 = N / 2;
        auto Ndiv4 = Ndiv2 / 2;

        stateDown.setSize  (static_cast<int> (this->numChannels), static_cast<int> (N));
        stateDown2.setSize (static_cast<int> (this->numChannels), static_cast<int> (Ndiv4 + 1));

        position.resize (static_cast<int> (this->numChannels));
    }

    //==============================================================================
    float getLatencyInSamples() const override
    {
        return static_cast<float> (coefficientsUp.getFilterOrder() + coefficientsDown.getFilterOrder()) * 0.5f;
    }

    void reset() override
    {
        ParentType::reset();

        stateUp.clear();
        stateDown.clear();
        stateDown2.clear();

        position.fill (0);
    }

    void processSamplesUp (const juce::dsp::AudioBlock<const SampleType>& inputBlock) override
    {
        jassert (inputBlock.getNumChannels() <= static_cast<size_t> (ParentType::buffer.getNumChannels()));
        jassert (inputBlock.getNumSamples() * ParentType::factor <= static_cast<size_t> (ParentType::buffer.getNumSamples()));

        // Initialization
        auto fir = coefficientsUp.getRawCoefficients();
        auto N = coefficientsUp.getFilterOrder() + 1;
        auto Ndiv2 = N / 2;
        auto numSamples = inputBlock.getNumSamples();

        // Processing
        for (size_t channel = 0; channel < inputBlock.getNumChannels(); ++channel)
        {
            auto bufferSamples = ParentType::buffer.getWritePointer (static_cast<int> (channel));
            auto buf = stateUp.getWritePointer (static_cast<int> (channel));
            auto samples = inputBlock.getChannelPointer (channel);

            for (size_t i = 0; i < numSamples; ++i)
            {
                // Input
                buf[N - 1] = 2 * samples[i];

                // Convolution
                auto out = static_cast<float> (0.0);

                for (size_t k = 0; k < Ndiv2; k += 2)
                    out += (buf[k] + buf[N - k - 1]) * fir[k];

                // Outputs
                bufferSamples[i << 1] = out;
                bufferSamples[(i << 1) + 1] = buf[Ndiv2 + 1] * fir[Ndiv2];

                // Shift data
                for (size_t k = 0; k < N - 2; k += 2)
                    buf[k] = buf[k + 2];
            }
        }
    }

    void processSamplesDown (juce::dsp::AudioBlock<SampleType>& outputBlock) override
    {
        jassert (outputBlock.getNumChannels() <= static_cast<size_t> (ParentType::buffer.getNumChannels()));
        jassert (outputBlock.getNumSamples() * ParentType::factor <= static_cast<size_t> (ParentType::buffer.getNumSamples()));

        // Initialization
        auto fir = coefficientsDown.getRawCoefficients();
        auto N = coefficientsDown.getFilterOrder() + 1;
        auto Ndiv2 = N / 2;
        auto Ndiv4 = Ndiv2 / 2;
        auto numSamples = outputBlock.getNumSamples();

        // Processing
        for (size_t channel = 0; channel < outputBlock.getNumChannels(); ++channel)
        {
            auto bufferSamples = ParentType::buffer.getWritePointer (static_cast<int> (channel));
            auto buf = stateDown.getWritePointer (static_cast<int> (channel));
            auto buf2 = stateDown2.getWritePointer (static_cast<int> (channel));
            auto samples = outputBlock.getChannelPointer (channel);
            auto pos = position.getUnchecked (static_cast<int> (channel));

            for (size_t i = 0; i < numSamples; ++i)
            {
                // Input
                buf[N - 1] = bufferSamples[i << 1];

                // Convolution
                auto out = static_cast<SampleType> (0.0);

                for (size_t k = 0; k < Ndiv2; k += 2)
                    out += (buf[k] + buf[N - k - 1]) * fir[k];

                // Output
                out += buf2[pos] * fir[Ndiv2];
                buf2[pos] = bufferSamples[(i << 1) + 1];

                samples[i] = out;

                // Shift data
                for (size_t k = 0; k < N - 2; ++k)
                    buf[k] = buf[k + 2];

                // Circular buffer
                pos = (pos == 0 ? Ndiv4 : pos - 1);
            }

            position.setUnchecked (static_cast<int> (channel), pos);
        }

    }

private:
    //==============================================================================
    juce::dsp::FIR::Coefficients<SampleType> coefficientsUp, coefficientsDown;
    juce::AudioBuffer<SampleType> stateUp, stateDown, stateDown2;
    juce::Array<size_t> position;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Downsampling2TimesEquirippleFIR)
};


//==============================================================================
template <typename SampleType>
Downsampling<SampleType>::Downsampling (size_t newNumChannels)
    : numChannels (newNumChannels)
{
    jassert (numChannels > 0);

    addDummyDownsamplingStage();
}

template <typename SampleType>
Downsampling<SampleType>::Downsampling (size_t newNumChannels, size_t newFactor,
                                        Downsampling::FilterType newType, bool isMaximumQuality,
                                        bool useIntegerLatency)
    : numChannels (newNumChannels), shouldUseIntegerLatency (useIntegerLatency)
{
    if (newFactor == 0)
    {
        addDummyDownsamplingStage();
    }
    else if (newType == FilterType::filterHalfBandPolyphaseIIR)
    {
        for (size_t n = 0; n < newFactor; ++n)
        {
            auto twUp   = (isMaximumQuality ? 0.10f : 0.12f) * (n == 0 ? 0.5f : 1.0f);
            auto twDown = (isMaximumQuality ? 0.12f : 0.15f) * (n == 0 ? 0.5f : 1.0f);

            auto gaindBStartUp    = (isMaximumQuality ? -90.0f : -70.0f);
            auto gaindBStartDown  = (isMaximumQuality ? -75.0f : -60.0f);
            auto gaindBFactorUp   = (isMaximumQuality ? 10.0f  : 8.0f);
            auto gaindBFactorDown = (isMaximumQuality ? 10.0f  : 8.0f);

            addDownsamplingStage (FilterType::filterHalfBandPolyphaseIIR,
                                  twUp, gaindBStartUp + gaindBFactorUp * (float) n,
                                  twDown, gaindBStartDown + gaindBFactorDown * (float) n);
        }
    }
    else if (newType == FilterType::filterHalfBandFIREquiripple)
    {
        for (size_t n = 0; n < newFactor; ++n)
        {
            auto twUp   = (isMaximumQuality ? 0.10f : 0.12f) * (n == 0 ? 0.5f : 1.0f);
            auto twDown = (isMaximumQuality ? 0.12f : 0.15f) * (n == 0 ? 0.5f : 1.0f);

            auto gaindBStartUp    = (isMaximumQuality ? -90.0f : -70.0f);
            auto gaindBStartDown  = (isMaximumQuality ? -75.0f : -60.0f);
            auto gaindBFactorUp   = (isMaximumQuality ? 10.0f  : 8.0f);
            auto gaindBFactorDown = (isMaximumQuality ? 10.0f  : 8.0f);

            addDownsamplingStage (FilterType::filterHalfBandFIREquiripple,
                                  twUp, gaindBStartUp + gaindBFactorUp * (float) n,
                                  twDown, gaindBStartDown + gaindBFactorDown * (float) n);
        }
    }
}

template <typename SampleType>
Downsampling<SampleType>::~Downsampling()
{
    stages.clear();
}

//==============================================================================
template <typename SampleType>
void Downsampling<SampleType>::addDummyDownsamplingStage()
{
    stages.add (new DownsamplingDummy(numChannels));
}

template <typename SampleType>
void Downsampling<SampleType>::addDownsamplingStage (Downsampling::FilterType type,
                                                     float normalisedTransitionWidthUp,
                                                     float stopbandAmplitudedBUp,
                                                     float normalisedTransitionWidthDown,
                                                     float stopbandAmplitudedBDown)
{
    stages.add (new Downsampling2TimesEquirippleFIR<SampleType> (numChannels,
                                                                     normalisedTransitionWidthUp,   stopbandAmplitudedBUp,
                                                                     normalisedTransitionWidthDown, stopbandAmplitudedBDown));

    factorDownsampling *= 2;
}

template <typename SampleType>
void Downsampling<SampleType>::clearDownsamplingStages()
{
    stages.clear();
    factorDownsampling = 1u;
}

//==============================================================================
template <typename SampleType>
void Downsampling<SampleType>::setUsingIntegerLatency (bool useIntegerLatency) noexcept
{
    shouldUseIntegerLatency = useIntegerLatency;
}

template <typename SampleType>
float Downsampling<SampleType>::getLatencyInSamples() const noexcept
{
    auto latency = getUncompensatedLatency();
    return shouldUseIntegerLatency ? latency + fractionalDelay : latency;
}

template <typename SampleType>
float Downsampling<SampleType>::getUncompensatedLatency() const noexcept
{
    auto latency = static_cast<float> (0);
    size_t order = 1;

    for (auto* stage : stages)
    {
        order *= stage->factor;
        latency += stage->getLatencyInSamples() / static_cast<float> (order);
    }

    return latency;
}

template <typename SampleType>
size_t Downsampling<SampleType>::getDownsamplingFactor() const noexcept
{
    return factorDownsampling;
}

//==============================================================================
template <typename SampleType>
void Downsampling<SampleType>::initProcessing (size_t maximumNumberOfSamplesBeforeDownsampling)
{
    jassert (! stages.isEmpty());
    auto currentNumSamples = maximumNumberOfSamplesBeforeDownsampling;

    for (auto* stage : stages)
    {
        stage->initProcessing (currentNumSamples);
        currentNumSamples *= stage->factor;
    }

    juce::dsp::ProcessSpec spec = { 0.0, (juce::uint32) maximumNumberOfSamplesBeforeDownsampling, (juce::uint32) numChannels };
    delay.prepare (spec);
    updateDelayLine();

    isReady = true;
    reset();
}
template <typename SampleType>
void Downsampling<SampleType>::reset() noexcept
{
    jassert (! stages.isEmpty());

    if (isReady)
        for (auto* stage : stages)
           stage->reset();

    delay.reset();
}

template <typename SampleType>
void Downsampling<SampleType>::processSamplesDown (juce::dsp::AudioBlock<SampleType>& outputBlock) noexcept
{
    jassert (! stages.isEmpty());

    if (! isReady)
        return;

    auto currentNumSamples = outputBlock.getNumSamples();

    for (int n = 0; n < stages.size() - 1; ++n)
        currentNumSamples *= stages.getUnchecked (n)->factor;

    for (int n = stages.size() - 1; n > 0; --n)
    {
        auto& stage = *stages.getUnchecked (n);
        auto audioBlock = stages.getUnchecked (n - 1)->getProcessedSamples (currentNumSamples);
        stage.processSamplesDown (audioBlock);

        currentNumSamples /= stage.factor;
    }

    stages.getFirst()->processSamplesDown (outputBlock);

    if (shouldUseIntegerLatency && fractionalDelay > static_cast<float> (0.0))
    {
        auto context = juce::dsp::ProcessContextReplacing<float> (outputBlock);
        delay.process (context);
    }
}

template <typename SampleType>
void Downsampling<SampleType>::updateDelayLine()
{
    auto latency = getUncompensatedLatency();
    fractionalDelay = static_cast<float> (1.0) - (latency - std::floor (latency));

    if (juce::approximatelyEqual (fractionalDelay, static_cast<float> (1.0)))
        fractionalDelay = static_cast<float> (0.0);
    else if (fractionalDelay < static_cast<float> (0.618))
        fractionalDelay += static_cast<float> (1.0);

    delay.setDelay (fractionalDelay);
}

template class Downsampling<float>;
template class Downsampling<double>;
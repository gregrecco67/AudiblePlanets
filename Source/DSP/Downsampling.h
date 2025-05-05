#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>


class Downsampler {
public:
	Downsampler(size_t numChans,
		float normalisedTransitionWidthDown,
		float stopbandAmplitudedBDown)
	{

		// typical coeffs: 
		/*
		
		for (size_t n = 0; n < newFactor; ++n)
		{
			auto twUp = (isMaximumQuality ? 0.10f : 0.12f) * (n == 0 ? 0.5f : 1.0f);
			auto twDown = (isMaximumQuality ? 0.12f : 0.15f) * (n == 0 ? 0.5f : 1.0f);

			auto gaindBStartUp = (isMaximumQuality ? -90.0f : -70.0f);
			auto gaindBStartDown = (isMaximumQuality ? -75.0f : -60.0f);
			auto gaindBFactorUp = (isMaximumQuality ? 10.0f : 8.0f);
			auto gaindBFactorDown = (isMaximumQuality ? 10.0f : 8.0f);

			addOversamplingStage(FilterType::filterHalfBandPolyphaseIIR,
				twUp, gaindBStartUp + gaindBFactorUp * (float)n,
				twDown, gaindBStartDown + gaindBFactorDown * (float)n);
		} 
		
		*/

		coefficientsDown = *juce::dsp::FilterDesign<float>::designFIRLowpassHalfBandEquirippleMethod(normalisedTransitionWidthDown, stopbandAmplitudedBDown);
		N = coefficientsDown.getFilterOrder() + 1;
		auto Ndiv2 = N / 2;
		auto Ndiv4 = Ndiv2 / 2;

		stateDown.setSize(static_cast<int> (this->numChannels), static_cast<int> (N));
		stateDown2.setSize(static_cast<int> (this->numChannels), static_cast<int> (Ndiv4 + 1));

		position.resize(static_cast<int> (this->numChannels));
	}

	float getLatencyInSamples() const override
	{
		return static_cast<float> (coefficientsDown.getFilterOrder()) * 0.5f;
	}

	virtual void initProcessing(size_t maximumNumberOfSamplesBeforeOversampling)
	{
		buffer.setSize(static_cast<int> (numChannels),
			static_cast<int> (maximumNumberOfSamplesBeforeOversampling * factor),
			false, false, true);
	}

	void reset() override
	{
		buffer.clear();
		stateDown.clear();
		stateDown2.clear();
		position.fill(0);
	}

	void loadSamples(juce::AudioBuffer<float>& inputBuffer) {
		jassert(inputBuffer.getNumChannels() <= static_cast<size_t> (ParentType::buffer.getNumChannels()));
		jassert(inputBuffer.getNumSamples() * ParentType::factor <= static_cast<size_t> (ParentType::buffer.getNumSamples()));
		for (size_t channel = 0; channel < inputBuffer.getNumChannels(); ++channel)
		{
			auto bufferSamples = buffer.getWritePointer(static_cast<int>(channel));
			auto inputSamples = inputBuffer.getReadPointer(static_cast<int>(channel));
			for (size_t i = 0; i < inputBuffer.getNumSamples(); ++i)
				bufferSamples[i] = inputSamples[i];
		}
	}

	void processSamplesDown(AudioBlock<SampleType>& outputBlock) override
	{
		jassert(outputBlock.getNumChannels() <= static_cast<size_t> (ParentType::buffer.getNumChannels()));
		jassert(outputBlock.getNumSamples() * ParentType::factor <= static_cast<size_t> (ParentType::buffer.getNumSamples()));

		// Initialization
		auto fir = coefficientsDown.getRawCoefficients();
		auto N = coefficientsDown.getFilterOrder() + 1;
		auto Ndiv2 = N / 2;
		auto Ndiv4 = Ndiv2 / 2;
		auto numSamples = outputBlock.getNumSamples();

		// Processing
		for (size_t channel = 0; channel < outputBlock.getNumChannels(); ++channel)
		{
			auto bufferSamples = buffer.getWritePointer(static_cast<int>(channel));
			auto buf = stateDown.getWritePointer(static_cast<int>(channel));
			auto buf2 = stateDown2.getWritePointer(static_cast<int>(channel));
			auto samples = outputBlock.getChannelPointer(channel);
			auto pos = position.getUnchecked(static_cast<int>(channel));

			for (size_t i = 0; i < numSamples; ++i)
			{
				// Input
				buf[N - 1] = bufferSamples[i << 1];

				// Convolution
				auto out = static_cast<SampleType>(0.0);

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

			position.setUnchecked(static_cast<int>(channel), pos);
		}
private:
	//==============================================================================
	size_t N;
	size_t numChannels = 2;
	juce::dsp::FIR::Coefficients<float> coefficientsDown;
	juce::AudioBuffer<float> stateUp, stateDown, stateDown2;
	juce::Array<size_t> position;

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Downsampler)
};
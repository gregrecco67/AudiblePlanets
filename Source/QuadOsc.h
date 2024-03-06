#include <JuceHeader.h>

enum class Wave
{
	sine = 0,
	sawUp = 1,
};

class QuadOscillator
{
public:
	QuadOscillator() = default;
	~QuadOscillator() = default;

	float freqs[4]{ 200.f }, phases[4]{ 0.f }, phaseIncs[4]{ 0.f }, gainsL[4]{ 0.f }, gainsR[4]{ 0.f };
	float freq, pan, tones, sampleRate;

	void setSampleRate(double sampleRate_)
	{
		sampleRate = (float)sampleRate_;
	}

	struct Params {
		int voices = 4;
		Wave wave = Wave::sine;
		float tones{ 1.0 }, pan{ 0.f }, spread{ 0.f }, detune{ 0.f }, phaseShift{ 0.f };
	};

	void normalizePhases(mipp::Reg<float>& phases_)
	{
		mipp::Msk<mipp::N<float>()> mask = mipp::cmpge<float>(phases_, 1.f);
		phases_ = mipp::blend<float>(phases_ - 1.f, phases_, mask);
	}

	void recalculate()
	{
		for (int i = 0; i < 4; i++) {
			phaseIncs[i] = freqs[i] / sampleRate;
		}
	}




	void processAdding(float freq, Params params, juce::AudioSampleBuffer& mainPhaseBuffer, juce::AudioSampleBuffer& quarterPhaseBuffer) {
		freqs = freq;
		recalculate();
	}

	void noteOn()
	{
		phases = 0.f;
		for (int i = 0; i < 4; i++) {
			phaseIncs[i] = freqs[i] / sampleRate;
		}
	}

};
#include "Oscillator.h"

#include "SynthVoice3.h"

APOscillator::APOscillator(gin::BandLimitedLookupTables &bllt_) : bllt(bllt_)
{
	setSampleRate(sampleRate);  // bllt.setSampleRate(sampleRate);
}

void APOscillator::renderFloats(float freq,
    const Settings &settings,
    float *xs,
    float *ys,
    const int numSamples)
{
	const float delta = freq * invSampleRate;
	for (int i = 0; i < numSamples; i++) {
		xs[i] = bllt.process(settings.wave, freq, phase) * settings.vol;
		ys[i] = bllt.process(settings.wave, freq, qrtPhase(phase)) * settings.vol;
		phase += delta;
		phase -= std::trunc(phase);
	}
}

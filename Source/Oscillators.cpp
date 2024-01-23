#include "Oscillators.h"
//==============================================================================
void APStereoOscillator::noteOn (float phase)
{
	if (phase >= 0) {
		return;
	}
	if (internalParams.wave == Wave::sine) {
		phase = 0;
	}
	if (internalParams.wave == Wave::cosine) {
		phase = juce::MathConstants<float>::halfPi;
	}
}

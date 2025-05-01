#include "Oscillator.h"

#include "SynthVoice2.h"

void APOscillator::renderPositions(float freq, const Params& params, Position positions[], const int numSamples) {
    float delta = 1.0f / (float ((1.0f / freq) * sampleRate));
	float note = gin::getMidiNoteFromHertz<float>(freq);
    
    for (int i = 0; i < numSamples; i++)
    {
        positions[i].x = bllt.process(params.wave, note, phase); // optional: add fold param? do postprocess
		positions[i].y = bllt.process(params.wave, note, qrtPhase(phase));
        phase += delta;
        if (phase >= 1.0f)
            phase -= 1.0f;
    }
}

void APOscillator::renderFloats(float freq, const Params& params, float* xs, float* ys, const int numSamples) {
    float delta = 1.0f / (float ((1.0f / freq) * sampleRate));
    for (int i = 0; i < numSamples; i++)
    {
        xs[i] = bllt.process(params.wave, freq, phase); // optional: add fold param? do postprocess
        ys[i] = bllt.process(params.wave, freq, qrtPhase(phase));
        phase += delta;
        if (phase >= 1.0f)
            phase -= 1.0f;
    }
}
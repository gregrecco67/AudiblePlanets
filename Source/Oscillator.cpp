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
        while (phase >= 1.0f)
            phase -= 1.0f;
    }
}
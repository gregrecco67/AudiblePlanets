#include "Oscillator.h"

#include "SynthVoice2.h"

void APOscillator::renderPositions(float freq, const Params& params, StereoPosition positions[], const int numSamples) {
    float delta = 1.0f / (float ((1.0f / freq) * sampleRate));
	float note = gin::getMidiNoteFromHertz<float>(freq);
    
    for (int i = 0; i < numSamples; i++)
    {
        auto x = bllt.process (params.wave, note, phase, params.pw);
        auto y = bllt.process (params.wave, note, qrtPhase(phase), params.pw);
        postProcess (params, x);
        postProcess (params, y);

        positions[i].xL = x * params.leftGain;
        positions[i].yL = y * params.leftGain;
        positions[i].xR = x * params.rightGain;
        positions[i].yR = y * params.rightGain;
        
        phase += delta;
        while (phase >= 1.0f)
            phase -= 1.0f;
    }
}
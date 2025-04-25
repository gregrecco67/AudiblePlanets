#include "Oscillator.h"

#include "SynthVoice2.h"

void APOscillator::renderPositions(float note, const Params& params, StereoPosition positions[], const int numSamples) {
    float freq = float (std::min (sampleRate / 2.0, 440.0 * std::pow (2.0, (note - 69.0) / 12.0)));
    float delta = 1.0f / (float ((1.0f / freq) * sampleRate));
    
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
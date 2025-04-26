#pragma once
#include <gin_dsp/gin_dsp.h>
#include <gin_plugin/gin_plugin.h>


class StereoPosition;
class APOscillator : public gin::StereoOscillator
{
public:
    APOscillator (gin::BandLimitedLookupTables& bllt_) : gin::StereoOscillator (bllt_) {}
    ~APOscillator() = default;

    // struct Params
    // {
    //     gin::Wave wave = gin::Wave::sine;
    //     float leftGain = 1.0;
    //     float rightGain = 1.0;
    //     float tones = 1.0f;
    //     float pan = 0.0f;
    //     float phaseShift = 0.0f;
    //     float pw = 0.5;
    //     float fold = 0.0f;
    //     float asym = 0.0f;
    // };

    void renderPositions(float note, const Params& params, StereoPosition positions[], const int numSamples);
    
};



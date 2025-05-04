/*
 * Audible Planets - an expressive, quasi-Ptolemaic semi-modular synthesizer
 *
 * Copyright 2024, Greg Recco
 *
 * Audible Planets is released under the GNU General Public Licence v3
 * or later (GPL-3.0-or-later). The license is found in the "LICENSE"
 * file in the root of this repository, or at
 * https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * All source for Audible Planets is available at
 * https://github.com/gregrecco67/AudiblePlanets
 */

 #pragma once

 #include <juce_audio_basics/juce_audio_basics.h>
 #include <gin_dsp/gin_dsp.h>
 #include <gin_plugin/gin_plugin.h>
 #include "Envelope.h"
 #include "libMTSClient.h"
 #include "Oscillator.h"
 #include <numbers>
 #include <random>
 class APAudioProcessor;
 
 using namespace std::numbers;
 
 struct APMatrix {
     //inline friend APMatrix operator*(const APMatrix& m, const float s) { // scalar multiplication
     //    return { m.a * s, m.b * s, m.c * s, m.d * s };
     //}

     float a, b, c, d;
};

struct APRegMatrix {
     inline friend APRegMatrix operator*(const APRegMatrix& m, const float s) { // scalar multiplication
         return { m.a * s, m.b * s, m.c * s, m.d * s };
     }
 
     mipp::Reg<float> a, b, c, d;
};

struct RegPosition {
     mipp::Reg<float> x, y;
 
     inline friend RegPosition operator*(const RegPosition& p, const APRegMatrix& m) { // apply matrix to position
         return { .x = p.x * m.a + p.y * m.b,
                 .y = p.x * m.c + p.y * m.d, };
     }
 
     inline friend RegPosition operator*(const RegPosition& p, const float s) { // scalar multiplication
         return { p.x * s, p.y * s };
     }
 
     inline RegPosition operator+(const RegPosition otherPos) {
         return { this->x + otherPos.x, this->y + otherPos.y };
     }
 }; 


struct Position {
     float x{ 0.f }, y{ 0.f };
 
     inline friend Position operator*(const Position& p, const APMatrix& m) { // apply matrix to position
         return { .x = m.a * p.x + m.b * p.y,
                 .y = m.c * p.x + m.d * p.y, };
     }
 
     inline friend Position operator*(const Position& p, const float s) { // scalar multiplication
         return { p.x * s, p.y * s };
     }
 
     inline Position operator+(const Position otherPos) {
         return { this->x + otherPos.x, this->y + otherPos.y };
     }
 };
 
 
 //==============================================================================
 class SynthVoice3 : public gin::SynthesiserVoice,
                    public gin::ModVoice
 {
 public:
    SynthVoice3(APAudioProcessor& p);
    
    inline std::array<float, 2> panWeights(const float in);

    void noteStarted() override;
    void noteRetriggered() override;
    void noteStopped(bool allowTailOff) override;

    void notePressureChanged() override;
    void noteTimbreChanged() override;
    void notePitchbendChanged() override {}
    void noteKeyStateChanged() override {}
    
    void setCurrentSampleRate(double newRate) override;

    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

    float getCurrentNote() override { return noteSmoother.getCurrentValue() * 127.0f; }

    bool isVoiceActive() override;

    float getFilterCutoffNormalized();
    float getMSEG1Phase();
    float getMSEG2Phase();
    float getMSEG3Phase();
    float getMSEG4Phase();
    gin::Wave waveForChoice(int choice);
    void matrixMultiply(const APMatrix& m, float* x, float* y) {
        for (int i = 0; i < n; i += 4) {
            float tmpX = x[i];
            x[i] = x[i] * m.a + y[i] * m.b;
            y[i] = tmpX * m.c + y[i] * m.d;
        }
    }

private:
    void updateParams(int blockSize);

    APAudioProcessor& proc;

    
    gin::Filter filter;
    gin::LFO lfo1, lfo2, lfo3, lfo4;
    gin::MSEG mseg1, mseg2, mseg3, mseg4;
    APOscillator osc1, osc2, osc3, osc4;
    gin::MSEG::Parameters mseg1Params, mseg2Params, mseg3Params, mseg4Params;

    Envelope env1, env2, env3, env4;
    std::array<Envelope*, 4> envs{&env1, &env2, &env3, &env4};

    float osc1xs[32];
    float osc1ys[32];
    float osc2xs[32];
    float osc2ys[32];
    float osc3xs[32];
    float osc3ys[32];
    float osc4xs[32];
    float osc4ys[32];

    mipp::Reg<float> epi1xs[8];
    mipp::Reg<float> epi1ys[8];
    mipp::Reg<float> epi2xs[8];
    mipp::Reg<float> epi2ys[8];
    mipp::Reg<float> epi3xs[8];
    mipp::Reg<float> epi3ys[8];
    mipp::Reg<float> epi4xs[8];
    mipp::Reg<float> epi4ys[8];

    mipp::Reg<float> epi4sEqnt[8], epi3sEqnt[8], epi2sEqnt[8], epi1sEqnt[8];
     
     mipp::Reg<float> osc1x, osc1y, osc2x, osc2y, osc3x, osc3y, osc4x, osc4y;

    mipp::Reg<float> dist2sq, dist2, invDist2;
    mipp::Reg<float> dist3sq, dist3, invDist3;
    mipp::Reg<float> dist4sq, dist4, invDist4;



    float currentMidiNote = -1;
    APOscillator::Settings osc1Params, osc2Params, osc3Params, osc4Params;
    float osc1Freq = 0.0f, osc2Freq = 0.0f, osc3Freq = 0.0f, osc4Freq = 0.0f;
    float osc1Vol = 0.0f, osc2Vol = 0.0f, osc3Vol = 0.0f, osc4Vol = 0.0f;
    int algo{ 0 };
    float equant{ 0.f };
    
    static constexpr float baseAmplitude = 0.12f; 

    gin::EasedValueSmoother<float> noteSmoother;
    
    juce::AudioBuffer<float> synthBuffer;

    int n = mipp::N<float>() * 8;

    mipp::Reg<float> sine2{ 0.f, 0.f, 0.f, 0.f }, cos2{ 0.f, 0.f, 0.f, 0.f }, sine3{ 0.f, 0.f, 0.f, 0.f }, cos3{ 0.f, 0.f, 0.f, 0.f },
        sine4{ 0.f, 0.f, 0.f, 0.f }, cos4{ 0.f, 0.f, 0.f, 0.f };
    // redundant??
    mipp::Reg<float> sample2L, sample2R, sample3L, sample3R, sample4L, sample4R;

    mipp::Reg<float> sampleL{ 0.f, 0.f, 0.f, 0.f }, sampleR{ 0.f, 0.f, 0.f, 0.f };
    mipp::Reg<float> piReg = static_cast<float>(pi);

    float maxPos = 0.0f;
    float minPos = 0.0f;

    friend class APSynth;
    juce::MPENote curNote;

    std::random_device rd;
    std::mt19937 gen{ rd() };
    std::uniform_real_distribution<> dist{ -1.f, 1.f };
     
 };
 

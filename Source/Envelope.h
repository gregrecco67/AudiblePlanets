#pragma once

#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>

class Envelope
{
public:
    //==============================================================================
    Envelope()
    {

        // generate lookup tables for MMA curves, and linear mapping for convenience
        for (int i = 1; i < 1999; i++)
        {
            // MMA curve transforms
            if ( ((double)i / 2000.0) > 0.996 ) // tail ends blow up, so we go linear for last stretch
                concave[i] = (double)i / 2000.0;
            else
                concave[i] = std::min(-(5.0 / 12.0) * std::log10(1.0 - (double)i / 2000.0), 1.0);

            if ( ((double)i / 2000.0) < 0.004 ) // negative blow-up
                convex[i] = (double)i / 2000.0;
            else
                convex[i] = std::max(1 + (5.0 / 12.0) * std::log10((double)i / 2000.0), 0.0);
        }
        concave[0] = 0.0;
        convex[0] = 0.0;
        concave[1999] = 1.0;
        convex[1999] = 1.0;
        recalculateRates();
    }
    Envelope(const Envelope&) = default;
    ~Envelope() = default;

	//==============================================================================

    struct Params
    {
        Params() = default;

        Params(double attackTimeMs_,
            double decayTimeMs_,
            double sustainLevel_,
            double releaseTimeMs_,
            double aCurve_, // 0 = linear, 1 = convex, -1 = concave
            double dRcurve_,
            bool repeat_,
            bool sync_=false,
            float syncduration_=1.f)
            : attackTimeMs(attackTimeMs_),
            decayTimeMs(decayTimeMs_),
            sustainLevel(sustainLevel_),
            releaseTimeMs(releaseTimeMs_),
            aCurve(aCurve_),
            dRCurve(dRcurve_),
            repeat(repeat_),
            sync(sync_),
            syncduration(syncduration_)
        {
        }

        double attackTimeMs{ 1.0 }, decayTimeMs{ 1.0 }, sustainLevel{ 0.5 }, releaseTimeMs{ 1.0 }, aCurve{ 1.0 }, dRCurve{ -1.0 };
        bool repeat{ false }, sync{ false };
        float syncduration{ 1.0 };
    };

    void setParameters(const Params& newParameters)
    {
        // need to call setSampleRate() first!
        jassert(sampleRate > 0.0);
        parameters = newParameters;
        recalculateRates();
    }

    const Params& getParameters() const noexcept { return parameters; }

    bool isActive() const noexcept { return state != State::idle; }

    float getValue() { return (float)finalOut; }

    void setSampleRate(double newSampleRate) noexcept
    {
        jassert(newSampleRate > 0.0);
        sampleRate = newSampleRate;
        recalculateRates();
    }

    void reset() noexcept
    {
        state = State::idle;
    }

    void noteOn() noexcept
    {
        linearIdxVal = 0.0;
        timeSinceStart = 0.f;
        if(parameters.repeat || parameters.sync)
            state = State::ADRattack;
        else 
			state = State::attack;
		
        
    }

    void noteOff() noexcept
    {
        if (state != State::idle)
        {
            if (parameters.releaseTimeMs > 0.0f && !parameters.repeat)
            {
                linearIdxVal = 1.0;
                releaseStart = finalOut;
                state = State::release;
            }
			else
            {
                reset();
            }
        }
    }


    float getNextSample() noexcept
    {
        timeSinceStart += inverseSampleRate;
        switch (state)
        {
            case State::idle:
            {
                finalOut = 0.0;
                break;
            }

            case State::attack:
            {
                linearIdxVal += attackRate;
                linearIdxVal = std::clamp(linearIdxVal, 0.0, 1.0);

                if (parameters.aCurve > 0.0f) {
                    curveVal = convex[(int)(linearIdxVal * 2000.)];
                    finalOut = std::clamp((1.0 - parameters.aCurve) * linearIdxVal + parameters.aCurve * curveVal, 0.0, 1.0);
                }
                else {
                    curveVal = concave[(int)(linearIdxVal * 2000.)];
                    finalOut = std::clamp((1.0 + parameters.aCurve) * linearIdxVal - parameters.aCurve * curveVal, 0.0, 1.0);
                }

                releaseStart = finalOut; // in case note is released before attack finishes

                if (linearIdxVal >= .999)
                {
                    finalOut = 1.0;
                    releaseStart = 1.0;
                    goToNextState();
                }

                break;
            }

            case State::ADRattack:
            {
                linearIdxVal += attackRate;
                linearIdxVal = std::clamp(linearIdxVal, 0.0, 1.0);

                if (parameters.aCurve > 0.0f) {
                    curveVal = convex[(int)(linearIdxVal * 2000.)];
                    finalOut = std::clamp((1.0 - parameters.aCurve) * linearIdxVal + parameters.aCurve * curveVal, 0.0, 1.0);
                }
                else {
                    curveVal = concave[(int)(linearIdxVal * 2000.)];
                    finalOut = std::clamp((1.0 + parameters.aCurve) * linearIdxVal - parameters.aCurve * curveVal, 0.0, 1.0);
                }

                releaseStart = finalOut; // in case note is released before attack finishes

                if (timeSinceStart >= duration)
				{
					finalOut = 1.0;
					releaseStart = 1.0;
                    noteOn();
				}

                if (linearIdxVal >= .999)
                {
                    finalOut = 1.0;
                    releaseStart = 1.0;
                    goToNextState();
                }

                break;
            }

            case State::decay:
            {
                linearIdxVal -= decayRate;
                linearIdxVal = std::clamp(linearIdxVal, 0.0, 1.0);

                if (parameters.dRCurve > 0.0) {
                    curveVal = convex[(int)(linearIdxVal * 2000.)];
                    unmappedVal = std::clamp(((1.0 - parameters.dRCurve) * linearIdxVal + parameters.dRCurve * curveVal), 0.0, 1.0);
                }
                else {
                    curveVal = concave[(int)(linearIdxVal * 2000.)];
                    unmappedVal = std::clamp(((1.0 + parameters.dRCurve) * linearIdxVal - parameters.dRCurve * curveVal), 0.0, 1.0);
                }
                
                finalOut = juce::jmap(unmappedVal, 0.0, 1.0, parameters.sustainLevel, 1.0);
                releaseStart = finalOut;

                if (finalOut <= parameters.sustainLevel)
                {
                    goToNextState();
                }


                break;
            }

            case State::ADRdecay:
            {
                linearIdxVal -= decayRate;
                linearIdxVal = std::clamp(linearIdxVal, 0.0, 1.0);

                if (parameters.dRCurve > 0.0) {
                    curveVal = convex[(int)(linearIdxVal * 2000.)];
                    unmappedVal = std::clamp(((1.0 - parameters.dRCurve) * linearIdxVal + parameters.dRCurve * curveVal), 0.0, 1.0);
                }
                else {
                    curveVal = concave[(int)(linearIdxVal * 2000.)];
                    unmappedVal = std::clamp(((1.0 + parameters.dRCurve) * linearIdxVal - parameters.dRCurve * curveVal), 0.0, 1.0);
                }

                finalOut = juce::jmap(unmappedVal, 0.0, 1.0, parameters.sustainLevel, 1.0);
                releaseStart = finalOut;

                if (timeSinceStart >= duration)
				{
                    noteOn();
				}

                if (finalOut <= parameters.sustainLevel)
                {
                    goToNextState();
                }


                break;
            }

            case State::sustain:
            {
                linearIdxVal = 1.0;
                finalOut = parameters.sustainLevel;
                releaseStart = finalOut;
                break;
            }

            case State::release:
            {
                linearIdxVal -= releaseRate;
                linearIdxVal = std::clamp(linearIdxVal, 0.0, 1.0);

                if (parameters.dRCurve > 0.0) {
                    curveVal = convex[(int)(linearIdxVal * 2000.)];
                    unmappedVal = std::clamp((1.0 - parameters.dRCurve) * linearIdxVal + parameters.dRCurve * curveVal, 0.0, 1.0);
                }
                else if (parameters.dRCurve <= 0.0) {
                    curveVal = concave[(int)(linearIdxVal * 2000.)];
                    unmappedVal = std::clamp((1.0 + parameters.dRCurve) * linearIdxVal - parameters.dRCurve * curveVal, 0.0, 1.0);
                }

                finalOut = juce::jmap(unmappedVal, 0.0, 1.0, 0.0, releaseStart);
                if (linearIdxVal <= 0.0f)
                    goToNextState();

                break;
            }

            case State::ADRrelease:
            {
                linearIdxVal -= releaseRate;
                linearIdxVal = std::clamp(linearIdxVal, 0.0, 1.0);

                if (parameters.dRCurve > 0.0) {
                    curveVal = convex[(int)(linearIdxVal * 2000.)];
                    unmappedVal = std::clamp((1.0 - parameters.dRCurve) * linearIdxVal + parameters.dRCurve * curveVal, 0.0, 1.0);
                }
                else if (parameters.dRCurve <= 0.0) {
                    curveVal = concave[(int)(linearIdxVal * 2000.)];
                    unmappedVal = std::clamp((1.0 + parameters.dRCurve) * linearIdxVal - parameters.dRCurve * curveVal, 0.0, 1.0);
                }

                finalOut = juce::jmap(unmappedVal, 0.0, 1.0, 0.0, releaseStart);

                if (timeSinceStart >= duration)
					noteOn();

                if (linearIdxVal <= 0.0f)
                    goToNextState();

                break;
            }

            case State::ADRSyncIdle:
			{
				finalOut = 0.0;
                if (timeSinceStart >= duration)
					noteOn();
				break;
			}

        }

        auto out = std::clamp((float)finalOut, 0.0f, 1.0f);
        return out; // envelopeVal;
    }

private:
    //==============================================================================
    void recalculateRates() noexcept
    {
        attackRate = (1.0 / (parameters.attackTimeMs * 0.001 * sampleRate));
        decayRate = (1.0 / (parameters.decayTimeMs * 0.001 * sampleRate));
        releaseRate = (1.0 / (parameters.releaseTimeMs * 0.001 * sampleRate));
        duration = parameters.syncduration;
        inverseSampleRate = 1.0 / sampleRate;
    }

    void goToNextState() noexcept
    {
        if (state == State::attack)
        {
            state = State::decay;
            linearIdxVal = 1.0; // decay and release run from 1 to 0
            return;
        }
        
        if (state == State::ADRattack)
        {
			state = State::ADRdecay;
			linearIdxVal = 1.0; // decay and release run from 1 to 0
			return;
		}

        if (state == State::decay)
        {
            linearIdxVal = 1.0;
            state = State::sustain;
            return;
        }

        if (state == State::ADRdecay)
        {
            linearIdxVal = 1.0;
            state = State::ADRrelease;
            return;
        }


        if (state == State::release)
            reset();

        if (state == State::ADRrelease && !parameters.sync) 
            noteOn();

        if (state == State::ADRrelease && parameters.sync)
			state = State::ADRSyncIdle;

    }



    //==============================================================================
    enum class State { idle, attack, decay, sustain, release, ADRattack, ADRdecay, ADRrelease, ADRSyncIdle };

    State state = State::idle;
    Params parameters;

    double sampleRate = 44100.0;
    double inverseSampleRate = 1.0 / sampleRate;
    double linearIdxVal{ 0.0 }, curveVal{ 0.0 }, attackRate{ 0.0 }, decayRate{ 0.0 }, releaseRate{ 0.0 }, finalOut{ 0.0 }, unmappedVal{ 0.0 }, releaseStart{ 0.0 };
    double convex[2000], concave[2000]; // , linear[2002];
    float timeSinceStart{ 0.f }, duration{ 1.f };

};



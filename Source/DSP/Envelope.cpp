#include "Envelope.h"

double Envelope::convex[2000];
double Envelope::concave[2000];
bool Envelope::isInitialized{false};
//==============================================================================
Envelope::Envelope()
{
	if (!isInitialized) {
		// generate lookup tables for MMA curves
		for (int i = 1; i < 1999; i++) {
			// MMA curve transforms
			if (i / 2000.0 > 0.99) {
				// linear segment avoids blow-up
				concave[i] = 16.6666666666667 * (i / 2000. - 0.99) + 0.8333333333333;
			} else {
				concave[i] = -(5.0 / 12.0) * std::log10(1.0 - (i / 2000.0));
			}

			if (i / 2000.0 < 0.01) {
				// here it comes to save the day
				convex[i] = 16.666666666667 * (i/2000.0);
			} else {
				convex[i] = 1 + (5.0 / 12.0) * std::log10(i / 2000.0);
			}
		}
		concave[0] = 0.0;
		convex[0] = 0.0;
		concave[1999] = 1.0;
		convex[1999] = 1.0;
		isInitialized = true;
	}
	recalculateRates();
}

//==============================================================================

void Envelope::noteOn() noexcept
{
	linearIdxVal = 0.0;
	timeSinceStart = 0.f;
	if (parameters.repeat || parameters.sync)
		state = State::ADRattack;
	else
		state = State::attack;
}

void Envelope::noteOff() noexcept
{
	if (state != State::idle) {
		if (parameters.releaseTimeMs > 0.0f)  // && !parameters.repeat)
		{
			state = State::release;
			linearIdxVal = 1.0;
			releaseStart = finalOut;
		} else {
			reset();
		}
	}
}

float Envelope::getNextSample() noexcept
{
	timeSinceStart += (float)inverseSampleRate;
	switch (state) {
		case State::idle: {
			finalOut = 0.0;
		}
		break;

		case State::attack: {
			linearIdxVal += attackRate;
			linearIdxVal = std::clamp(linearIdxVal, 0.0, 1.0);

			if (parameters.aCurve > 0.0f) {
				curveVal = convex[std::clamp(static_cast<int>(linearIdxVal * 2000.), 0, 1999)];
				finalOut = std::clamp((1.0 - parameters.aCurve) * linearIdxVal + parameters.aCurve * curveVal, 0.0, 1.0);
			} else {
				curveVal = concave[std::clamp(static_cast<int>(linearIdxVal * 2000.), 0, 1999)];
				finalOut = std::clamp((1.0 + parameters.aCurve) * linearIdxVal - parameters.aCurve * curveVal, 0.0, 1.0);
			}

			releaseStart = finalOut;  // in case note is released before attack finishes

			if (linearIdxVal >= .999) {
				finalOut = 1.0;
				releaseStart = 1.0;
				goToNextState();
			}
		}
		break;

		case State::ADRattack: {
			linearIdxVal += attackRate;
			linearIdxVal = std::clamp(linearIdxVal, 0.0, 1.0);

			if (parameters.aCurve > 0.0f) {
				curveVal =
				    convex[std::clamp((int)(linearIdxVal * 2000.), 0, 1999)];
				finalOut = std::clamp((1.0 - parameters.aCurve) * linearIdxVal +
				                          parameters.aCurve * curveVal,
				                      0.0, 1.0);
			} else {
				curveVal =
				    concave[std::clamp((int)(linearIdxVal * 2000.), 0, 1999)];
				finalOut = std::clamp((1.0 + parameters.aCurve) * linearIdxVal -
				                          parameters.aCurve * curveVal,
				                      0.0, 1.0);
			}

			releaseStart = finalOut;  // in case note is released before attack finishes

			if (timeSinceStart >= duration) {
				finalOut = 1.0;
				releaseStart = 1.0;
				noteOn();
			}

			if (linearIdxVal >= .999) {
				finalOut = 1.0;
				releaseStart = 1.0;
				goToNextState();
			}
		}
		break;

		case State::decay: {
			linearIdxVal -= decayRate;
			linearIdxVal = std::clamp(linearIdxVal, 0.0, 1.0);

			if (parameters.dRCurve > 0.0) {
				curveVal = convex[std::clamp((int)(linearIdxVal * 2000.), 0, 1999)];
				unmappedVal = std::clamp(((1.0 - parameters.dRCurve) * linearIdxVal + parameters.dRCurve * curveVal), 0.0, 1.0);
			} else {
				curveVal = concave[std::clamp((int)(linearIdxVal * 2000.), 0, 1999)];
				unmappedVal = std::clamp(((1.0 + parameters.dRCurve) * linearIdxVal - parameters.dRCurve * curveVal), 0.0, 1.0);
			}

			finalOut = juce::jmap(unmappedVal, 0.0, 1.0, parameters.sustainLevel, 1.0);
			releaseStart = finalOut;

			if (finalOut <= parameters.sustainLevel) {
				goToNextState();
			}
		}
		break;

		case State::ADRdecay: {
			linearIdxVal -= decayRate;
			linearIdxVal = std::clamp(linearIdxVal, 0.0, 1.0);

			if (parameters.dRCurve > 0.0) {
				curveVal =
				    convex[std::clamp((int)(linearIdxVal * 2000.), 0, 1999)];
				unmappedVal =
				    std::clamp(((1.0 - parameters.dRCurve) * linearIdxVal +
				                parameters.dRCurve * curveVal),
				               0.0, 1.0);
			} else {
				curveVal =
				    concave[std::clamp((int)(linearIdxVal * 2000.), 0, 1999)];
				unmappedVal =
				    std::clamp(((1.0 + parameters.dRCurve) * linearIdxVal -
				                parameters.dRCurve * curveVal),
				               0.0, 1.0);
			}

			finalOut =
			    juce::jmap(unmappedVal, 0.0, 1.0, parameters.sustainLevel, 1.0);
			releaseStart = finalOut;

			if (timeSinceStart >= duration) {
				noteOn();
			}

			if (finalOut <= parameters.sustainLevel) {
				goToNextState();
			}
		}
		break;

		case State::sustain: {
			linearIdxVal = 1.0;
			finalOut = parameters.sustainLevel;
			releaseStart = finalOut;
		}
		break;

		case State::release: {
			linearIdxVal -= releaseRate;
			linearIdxVal = std::clamp(linearIdxVal, 0.0, 1.0);

			if (parameters.dRCurve > 0.0) {
				curveVal = convex[std::clamp((int)(linearIdxVal * 2000.), 0, 1999)];
				unmappedVal = std::clamp((1.0 - parameters.dRCurve) * linearIdxVal + parameters.dRCurve * curveVal, 0.0, 1.0);
			} else if (parameters.dRCurve <= 0.0) {
				curveVal = concave[std::clamp((int)(linearIdxVal * 2000.), 0, 1999)];
				unmappedVal = std::clamp((1.0 + parameters.dRCurve) * linearIdxVal - parameters.dRCurve * curveVal, 0.0, 1.0);
			}

			finalOut = juce::jmap(unmappedVal, 0.0, 1.0, 0.0, releaseStart);
			if (linearIdxVal <= 0.001f)
				goToNextState();
		}
		break;

		case State::ADRrelease: {
			linearIdxVal -= releaseRate;
			linearIdxVal = std::clamp(linearIdxVal, 0.0, 0.999);

			if (parameters.dRCurve > 0.0) {
				curveVal = convex[std::clamp((int)(linearIdxVal * 2000.), 0, 1999)];
				unmappedVal = std::clamp((1.0 - parameters.dRCurve) * linearIdxVal + parameters.dRCurve * curveVal, 0.0, 1.0);
			} else if (parameters.dRCurve <= 0.0) {
				curveVal = concave[std::clamp((int)(linearIdxVal * 2000.), 0, 1999)];
				unmappedVal = std::clamp((1.0 + parameters.dRCurve) * linearIdxVal - parameters.dRCurve * curveVal, 0.0, 1.0);
			}

			finalOut = juce::jmap(unmappedVal, 0.0, 1.0, 0.0, releaseStart);

			if (timeSinceStart >= duration)
				noteOn();

			if (linearIdxVal <= 0.001f)
				goToNextState();
		}
		break;

		case State::ADRSyncIdle: {
			finalOut = 0.0;
			if (timeSinceStart >= duration)
				noteOn();
		}
		break;

		default: {
		    finalOut = 0.0;
		    state = State::idle;
		}
	}

	auto out = std::clamp((float)finalOut, 0.0f, 1.0f);
	return out;  // envelopeVal;
}

//==============================================================================

void Envelope::recalculateRates() noexcept
{
	attackRate = (1.0 / (parameters.attackTimeMs * sampleRate));
	decayRate = (1.0 / (parameters.decayTimeMs * sampleRate));
	releaseRate = (1.0 / (parameters.releaseTimeMs * sampleRate));
	duration = parameters.syncduration;
	inverseSampleRate = 1.0 / sampleRate;
}

void Envelope::goToNextState() noexcept
{
	if (state == State::attack) {
		state = State::decay;
		linearIdxVal = 1.0;  // decay and release run from 1 to 0
		return;
	}

	if (state == State::ADRattack) {
		state = State::ADRdecay;
		linearIdxVal = 1.0;  // decay and release run from 1 to 0
		return;
	}

	if (state == State::decay) {
		linearIdxVal = 1.0;
		state = State::sustain;
		return;
	}

	if (state == State::ADRdecay) {
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

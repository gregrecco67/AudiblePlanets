#include "Envelope.h"


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
		} 
		else { reset(); }
	}
}

double Envelope::getValForIdx(double idx, bool isAttack)
{
	idx = std::clamp(idx, 0.0, 1.0);
	double c = isAttack ? parameters.aCurve : parameters.dRCurve;
	double cVal;
	if (isAttack)
	{
		if (c > 0) {
			int x = std::clamp(static_cast<int>((1 - idx) * 1024.), 0, 1023);
			auto l1 = convex[x];
			auto l2 = convex[std::min(x + 1, 1023)];
			auto frac = (1 - idx) * 1024.0 - std::floor((1 - idx) * 1024.);
			auto lookup = (1 - frac) * l1 + frac * l2;
			cVal = (1 - c) * idx + c * (1 - lookup);
		}
		else {
			int x = std::clamp(static_cast<int>(idx * 1024.), 0, 1023);
			auto l1 = convex[x];
			auto l2 = convex[std::min(x + 1, 1023)];
			auto frac = idx * 1024.0 - std::floor(idx * 1024.);
			auto lookup = (1 - frac) * l1 + frac * l2;
			cVal = (1 + c) * idx - c * lookup;
		}
	}
	if (!isAttack)
	{
		if (c < 0)
		{
			int x = std::clamp(static_cast<int>(idx * 1024.), 0, 1023);
			auto l1 = convex[x];
			auto l2 = convex[std::min(x + 1, 1023)];
			auto frac = idx * 1024.0 - std::floor(idx * 1024.);
			auto lookup = (1 - frac) * l1 + frac * l2;
			cVal = (1 + c) * idx - c * lookup;
		}
		else {
			int x = std::clamp((int)((1 - idx) * 1024.), 0, 1023);
			auto l1 = 1 - convex[x];
			auto l2 = 1 - convex[std::min(x + 1, 1023)];
			auto frac = idx * 1024.0 - std::floor(idx * 1024.);
			auto lookup = (1 - frac) * l1 + frac * l2;
			cVal = (1 - c) * idx + c * lookup;
		}
	}
	return cVal;
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

		finalOut = getValForIdx(linearIdxVal, true);
		releaseStart = finalOut;  // in case note is released before attack finishes

		if (linearIdxVal >= .999) {
			finalOut = 1.0;
			releaseStart = 1.0;
			goToNextState();
		}
	}
	break;

	case State::ADRattack: {
		finalOut = getValForIdx(linearIdxVal, true);
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

	case State::decay: 
	{
		linearIdxVal -= decayRate;
		linearIdxVal = std::clamp(linearIdxVal, 0.0, 1.0);

		auto unmappedVal = getValForIdx(linearIdxVal, false);
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

		auto unmappedVal = getValForIdx(linearIdxVal, false);
		finalOut = juce::jmap(unmappedVal, 0.0, 1.0, parameters.sustainLevel, 1.0);
		releaseStart = finalOut;

		if (timeSinceStart >= duration) { noteOn(); }

		if (finalOut <= parameters.sustainLevel) { goToNextState(); }
	}
	break;

	case State::sustain: {
		linearIdxVal = 1.0;
		finalOut = parameters.sustainLevel;
		releaseStart = finalOut;
	}
	break;

	case State::release: {
		if (juce::approximatelyEqual(linearIdxVal, 1.0))
		{
			// replace with solver
			linearIdxVal = releaseStart;
		}

		linearIdxVal -= releaseRate;
		linearIdxVal = std::clamp(linearIdxVal, 0.0, 1.0);

		auto unmappedVal = getValForIdx(linearIdxVal, false);
		finalOut = juce::jmap(unmappedVal, 0.0, 1.0, 0.0, releaseStart);
		if (linearIdxVal <= 0.001f)
			goToNextState();
	}
	break;

	case State::ADRrelease: {
		linearIdxVal -= releaseRate;
		linearIdxVal = std::clamp(linearIdxVal, 0.0, 0.999);

		auto unmappedVal = getValForIdx(linearIdxVal, false);
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
	} // switch

	auto out = std::clamp((float)finalOut, 0.0f, 1.0f);
	return out;  // envelopeVal;
}

//==============================================================================

void Envelope::recalculateRates() noexcept
{
	attackRate = std::min((1.0 / (parameters.attackTimeMs * sampleRate)), 1.0);
	decayRate = std::min((1.0 / (parameters.decayTimeMs * sampleRate)), 1.0);
	releaseRate = std::min((1.0 / (parameters.releaseTimeMs * sampleRate)), 1.0);
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

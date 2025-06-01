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

double Envelope::getValForIdx(double idx, const bool isAttack) const {
	idx = std::clamp(idx, 0.0, 1.0);
	const double c = isAttack ? parameters.aCurve : parameters.dRCurve;
	double cVal;
	if (isAttack)
	{
		if (c > 0) {
			const int x = std::clamp(static_cast<int>((1 - idx) * 1024.), 0, 1023);
			const auto l1 = convex[x];
			const auto l2 = convex[std::min(x + 1, 1023)];
			const auto frac = (1 - idx) * 1024.0 - std::floor((1 - idx) * 1024.);
			const auto lookup = (1 - frac) * l1 + frac * l2;
			cVal = (1 - c) * idx + c * (1 - lookup);
		}
		else {
			const int x = std::clamp(static_cast<int>(idx * 1024.), 0, 1023);
			const auto l1 = convex[x];
			const auto l2 = convex[std::min(x + 1, 1023)];
			const auto frac = idx * 1024.0 - std::floor(idx * 1024.);
			const auto lookup = (1 - frac) * l1 + frac * l2;
			cVal = (1 + c) * idx - c * lookup;
		}
	}
	else
	{
		if (c < 0)
		{
			const int x = std::clamp(static_cast<int>(idx * 1024.), 0, 1023);
			const auto l1 = convex[x];
			const auto l2 = convex[std::min(x + 1, 1023)];
			const auto frac = idx * 1024.0 - std::floor(idx * 1024.);
			const auto lookup = (1 - frac) * l1 + frac * l2;
			cVal = (1 + c) * idx - c * lookup;
		}
		else {
			const int x = std::clamp(static_cast<int>((1 - idx) * 1024.), 0, 1023);
			const auto l1 = 1 - convex[x];
			const auto l2 = 1 - convex[std::min(x + 1, 1023)];
			const auto frac = idx * 1024.0 - std::floor(idx * 1024.);
			const auto lookup = (1 - frac) * l1 + frac * l2;
			cVal = (1 - c) * idx + c * lookup;
		}
	}
	return cVal;
}

double Envelope::getIdxForVal(const double val) const
{
	double low{0}, high{1}, mid{0.5};
	constexpr double tol{0.05};
	double diff = getValForIdx(mid, false) - val;
	while (std::abs(diff) > tol) {
		if (diff > 0) {	
		    high = mid;
		    mid = (high + low) * 0.5;
		}
		else { 
		    low = mid;
		    mid = (high + low) * 0.5;
		}
		diff = getValForIdx(mid, false) - val;
	}
	return mid;
}

float Envelope::getNextSample() noexcept
{
	timeSinceStart += static_cast<float>(inverseSampleRate);

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
		linearIdxVal += attackRate;
		linearIdxVal = std::clamp(linearIdxVal, 0.0, 1.0);

		finalOut = getValForIdx(linearIdxVal, true);
		releaseStart = finalOut;  // in case note is released before attack finishes

		if (timeSinceStart >= duration) {
			finalOut = 0.0;
			releaseStart = 0.0;
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

		const auto unmappedVal = getValForIdx(linearIdxVal, false);
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

		const auto unmappedVal = getValForIdx(linearIdxVal, false);
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
			linearIdxVal = getIdxForVal(linearIdxVal);
		}

		linearIdxVal -= releaseRate;
		linearIdxVal = std::clamp(linearIdxVal, 0.0, 1.0);

		const auto unmappedVal = getValForIdx(linearIdxVal, false);
		finalOut = juce::jmap(unmappedVal, 0.0, 1.0, 0.0, releaseStart);
		if (linearIdxVal <= 0.001f)
			goToNextState();
	}
	break;

	case State::ADRrelease: {
		if (juce::approximatelyEqual(linearIdxVal, 1.0))
		{
			linearIdxVal = getIdxForVal(linearIdxVal);
		}
		linearIdxVal -= releaseRate;
		linearIdxVal = std::clamp(linearIdxVal, 0.0, 0.999);

		const auto unmappedVal = getValForIdx(linearIdxVal, false);
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

	const auto out = std::clamp(static_cast<float>(finalOut), 0.0f, 1.0f);
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
	switch (state) {
		case State::attack: {
			state = State::decay;
			linearIdxVal = 1.0;  // decay and release run from 1 to 0
			return;
		}
		case State::ADRattack: {
			state = State::ADRdecay;
			linearIdxVal = 1.0;  // decay and release run from 1 to 0
			return;
		}
		case State::decay: {
			linearIdxVal = 1.0;
			state = State::sustain;
			return;
		}
		case State::ADRdecay: {
			linearIdxVal = 1.0;
			state = State::ADRrelease;
			return;
		}
		case State::release: {
			reset();
			break;
		}
		case State::ADRrelease: {
			if (!parameters.sync)
				noteOn();
			else
				state = State::ADRSyncIdle;
			break;
		}
		default:
			state = State::idle;
	}








}

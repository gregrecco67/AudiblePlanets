#pragma once

#include "BaseNL.h"

template<class Type>
class SoftClip : public BaseNL, public Type {
public:
	SoftClip() { Type::initialise(); }

	virtual ~SoftClip() {}

	void prepare(double, int) override { Type::prepare(); }

	void processBlock(float *x, const int nSamples) override
	{
		for (int n = 0; n < nSamples; ++n)
			x[n] = 0.8f * static_cast<float>(Type::process(static_cast<double>(x[n])));
	}

	inline double func(double x) const noexcept override
	{
		if (std::abs(x) <= 1.f)
		{
			return std::sin(pi * 0.5 * x);
		}
		else
		{
			return signum(x);
		};
	}

	/** First antiderivative of hard clipper */
	inline double func_AD1(double x) const noexcept override
	{
		bool inRange = std::abs(x) <= 1.0;

		return inRange ?
			-p * std::cos(pi * 0.5 * x) + p
		    : x * signum(x) + p - 1;
	}

	/** Second antiderivative of hard clipper */
	inline double func_AD2(double x) const noexcept override
	{
		bool inRange = std::abs(x) <= 1.0f;
		 
		 return inRange ? (2 * pi * x - 4 * std::sin(pi * 0.5 * x)) * inv_pi * inv_pi
			: ((4 - 2 * pi) * x + (pi * x * x * signum(x))) * inv_pi * 0.5 + q * signum(x);
	}

private:
	const double p{2.0 * inv_pi};
	const double q{0.5 - 4 / (pi * pi)};

	/** Signum function to determine the sign of the input. */
	template<typename T>
	inline int signum(T val) const noexcept
	{
		return (T(0) < val) - (val < T(0));
	}

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoftClip)
};

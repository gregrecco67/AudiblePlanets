#pragma once

#include "BaseNL.h"
#include "polylogarithm/Li2.hpp"

template<class Type>
class Fullwave : public BaseNL, public Type {
public:
	Fullwave() { Type::initialise(); }

	virtual ~Fullwave() {}

	void prepare(double, int) override { Type::prepare(); }

	void processBlock(float *x, const int nSamples) override
	{
		for (int n = 0; n < nSamples; ++n)
			x[n] = static_cast<float>(Type::process((double)x[n]));
	}

	inline double func(double x) const noexcept override
	{
		return std::tanh(x) * signum(x);
	}

	inline double func_AD1(double x) const noexcept override
	{
		return std::log(std::cosh(x)) * signum(x);
	}

	inline double func_AD2(double x) const noexcept override
	{
		// NOT FUNCTIONING YET!
		// do not use!

		if (x < 0.0) { 
		const auto expVal = std::exp(2 * x);
			return 0.5 * ((double)polylogarithm::Li2(-expVal) -
			                 -x * (-x + 2.0 * std::log(expVal + 1.) -
			                         2.0 * std::log(std::cosh(-x)))) +
			       (std::pow(juce::MathConstants<double>::pi, 2) / 24.0);
		
		} 
		else {
			const auto expVal = std::exp(-2 * x);
			return 0.5 * ((double)polylogarithm::Li2(-expVal) -
				x * (x + 2.0 * std::log(expVal + 1.) - 2.0 * 
				std::log(std::cosh(x)))) + 
				(std::pow(juce::MathConstants<double>::pi, 2) / 24.0);
		}
	}

	inline double ad2_positive(double x) const noexcept {
		const auto expVal = std::exp(-2 * x);
		return 0.5 * ((double)polylogarithm::Li2(-expVal) -
		x * (x + 2.0 * std::log(expVal + 1.) -
		2.0 * std::log(std::cosh(x)))) +
		(std::pow(juce::MathConstants<double>::pi, 2) / 24.0);
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

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Fullwave)
};

#pragma once

#include "BaseNL.h"

template<class Type>
class Folder : public BaseNL, public Type {
public:
	Folder() { Type::initialise(); }

	virtual ~Folder() {}

	void prepare(double, int) override { Type::prepare(); }

	void processBlock(float *x, const int nSamples) override
	{
		for (int n = 0; n < nSamples; ++n)
			x[n] = static_cast<float>(Type::process(static_cast<double>(x[n])));
	}

	inline double func(double x) const noexcept override
	{
		return std::sin(pi * 0.5 * x * (1 + m));
	}

	inline double func_AD1(double x) const noexcept override
	{
		return -(p/(1+m)) * std::cos(pi * 0.5 * x * (1 + m)) + (p/(1 + m));
	}

	inline double func_AD2(double x) const noexcept override
	{
		return -(4 * std::sin(pi * 0.5 * x * (1 + m)))
				/ (pi * pi * (1 + m) * (1 + m))
				+ (p * x) / (1 + m);
	}

	double m{0.0}; // scale extent of input by (1 + m)

private:
	const double p{2.0 * inv_pi};

	template<typename T>
	inline int signum(T val) const noexcept
	{
		return (T(0) < val) - (val < T(0));
	}

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Folder)
};

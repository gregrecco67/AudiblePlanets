#pragma once

#include "BaseNL.h"
#include "polylogarithm/Li2.hpp"

template <class Type>
class TanhNL : public BaseNL,
               public Type
{
public:
    TanhNL()
    {
        Type::initialise();
    }

    virtual ~TanhNL() {}

    void prepare (double, int) override
    {
        Type::prepare();
    }

    void processBlock (float* x, const int nSamples) override
    {
        for (int n = 0; n < nSamples; ++n)
            x[n] = static_cast<float>(Type::process (static_cast<double>(x[n])));
    }

protected:
    inline double func (double x) const noexcept override
    {
        return std::tanh (x);
    }

    /** First antiderivative of hard clipper */
    inline double func_AD1 (double x) const noexcept override
    {
        return std::log (std::cosh (x));
    }

    /** Second antiderivative of hard clipper */
    inline double func_AD2 (double x) const noexcept override
    {
        // return 0.0;
        const auto expVal = std::exp (-2 * x);
        return 0.5 * ((double) polylogarithm::Li2 (-expVal)
            - x * (x + 2.0 * std::log (expVal + 1.) - 2.0 * std::log (std::cosh (x))))
            + (std::pow (juce::MathConstants<double>::pi, 2) / 24.0);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TanhNL)
};

#pragma once

#include "BaseNL.h"

template <class Type>
class HardClip : public BaseNL,
                 public Type
{
public:
    HardClip()
    {
        Type::initialise();
    }

    virtual ~HardClip() {}

    void prepare (double, int) override
    {
        Type::prepare();
    }

    void processBlock (float* x, const int nSamples) override
    {
        for (int n = 0; n < nSamples; ++n)
            x[n] = static_cast<float>(Type::process (static_cast<double>(x[n])));
    }

    inline double func (double x) const noexcept override
    {
        return juce::jlimit (-1.0, 1.0, x);
    }

    /** First antiderivative of hard clipper */
    inline double func_AD1 (double x) const noexcept override
    {
        bool inRange = std::abs (x) <= 1.0;

        return inRange ?
            x * x / 2.0 :
            x * signum (x) - 0.5;
    }

    /** Second antiderivative of hard clipper */
    inline double func_AD2 (double x) const noexcept override
    {
        bool inRange = std::abs (x) <= 1.0f;

        return inRange ?
            x * x * x / 6.0 :
            ((x * x / 2.0) + (1.0 / 6.0)) * signum (x) - (x / 2.0);
    }

private:
    /** Signum function to determine the sign of the input. */
    template <typename T>
    inline int signum (T val) const noexcept
    {
        return (T (0) < val) - (val < T (0));
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HardClip)
};

#pragma once

#include "StandardNL.h"

namespace LUTConst
{
    constexpr double minVal = -10.0;
    constexpr double maxVal = 10.0;
}

template <size_t N>
class StandardLUT : public StandardNL
{
public:
    StandardLUT() = default;
    virtual ~StandardLUT() {}

    void initialise() override
    {
        lut.initialise ([this] (double x) { return func (x); },
            LUTConst::minVal, LUTConst::maxVal, N);
    }

    inline double process (double x) noexcept override
    {
        return lut.processSample (x);
    }

protected:
    juce::dsp::LookupTableTransform<double> lut;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StandardLUT)
};

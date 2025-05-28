#pragma once

#include "ADAA1.h"
#include "StandardLUT.h"

template <size_t N>
class ADAA1LUT : public ADAA1
{
public:
    ADAA1LUT() = default;
    virtual ~ADAA1LUT() {}

    void initialise() override
    {
        lut.initialise ([this] (double x) { return func (x); },
            LUTConst::minVal, LUTConst::maxVal, N);

        lut_AD1.initialise ([this] (double x) { return func_AD1 (x); },
            2 * LUTConst::minVal, 2 * LUTConst::maxVal, 2 * N);
    }

    void prepare() override
    {
        ADAA1::prepare();
    }

protected:
    inline double nlFunc (double x) const noexcept override { return lut.processSample (x); }
    inline double nlFunc_AD1 (double x) const noexcept override { return lut_AD1.processSample (x); }

    juce::dsp::LookupTableTransform<double> lut;
    juce::dsp::LookupTableTransform<double> lut_AD1;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ADAA1LUT)
};

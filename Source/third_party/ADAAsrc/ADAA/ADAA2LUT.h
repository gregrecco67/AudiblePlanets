#pragma once

#include "ADAA2.h"
#include "ADAA1LUT.h"

template <size_t N>
class ADAA2LUT : public ADAA2
{
public:
    ADAA2LUT() = default;
    virtual ~ADAA2LUT() {}

    void initialise() override
    {
        lut.initialise ([this] (double x) { return func (x); },
            LUTConst::minVal, LUTConst::maxVal, N);

        lut_AD1.initialise ([this] (double x) { return func_AD1 (x); },
            2 * LUTConst::minVal, 2 * LUTConst::maxVal, 4 * N);

        lut_AD2.initialise ([this] (double x) { return func_AD2 (x); },
            4 * LUTConst::minVal, 4 * LUTConst::maxVal, 16 * N);
    }

    void prepare() override
    {
        ADAA2::prepare();
    }

protected:
    inline double nlFunc (double x) const noexcept override { return lut.processSample (x); }
    inline double nlFunc_AD1 (double x) const noexcept override { return lut_AD1.processSample (x); }
    inline double nlFunc_AD2 (double x) const noexcept override { return lut_AD2.processSample (x); }

    juce::dsp::LookupTableTransform<double> lut;
    juce::dsp::LookupTableTransform<double> lut_AD1;
    juce::dsp::LookupTableTransform<double> lut_AD2;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ADAA2LUT)
};

#pragma once

#include "StandardNL.h"

namespace ADAAConst
{
    constexpr double TOL = 1.0e-5;
}

/** Base class for 1st-order ADAA */
class ADAA1 : public StandardNL
{
public:
    ADAA1() = default;
    virtual ~ADAA1() {}

    void prepare() override
    {
        x1 = 0.0;
        ad1_x1 = 0.0;
    }

    inline double process (double x) noexcept override
    {
        bool illCondition = std::abs (x - x1) < ADAAConst::TOL;
        double ad1_x = nlFunc_AD1 (x);

        double y = illCondition ?
            nlFunc (0.5 * (x + x1)) :
            (ad1_x - ad1_x1) / (x - x1);

        ad1_x1 = ad1_x;
        x1 = x;

        return y;
    }

protected:
    virtual inline double nlFunc (double x) const noexcept { return func (x); }
    virtual inline double nlFunc_AD1 (double x) const noexcept { return func_AD1 (x); }

    double x1 = 0.0;
    double ad1_x1 = 0.0;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ADAA1)
};

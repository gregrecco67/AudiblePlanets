// ====================================================================
// This file is part of Polylogarithm.
//
// Polylogarithm is licenced under the MIT License.
// ====================================================================

#pragma once
#include <complex>

namespace polylogarithm {

/// real polylogarithm with n=2 (dilogarithm) with single precision
float Li2(float) noexcept;

/// real polylogarithm with n=2 (dilogarithm) with double precision
double Li2(double) noexcept;

/// real polylogarithm with n=2 (dilogarithm) with long double precision
long double Li2(long double) noexcept;

/// complex polylogarithm with n=2 (dilogarithm) with single precision
std::complex<float> Li2(const std::complex<float>&) noexcept;

/// complex polylogarithm with n=2 (dilogarithm) with double precision
std::complex<double> Li2(const std::complex<double>&) noexcept;

/// complex polylogarithm with n=2 (dilogarithm) with long double precision
std::complex<long double> Li2(const std::complex<long double>&) noexcept;

} // namespace polylogarithm

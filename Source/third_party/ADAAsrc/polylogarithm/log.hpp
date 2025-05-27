// ====================================================================
// This file is part of Polylogarithm.
//
// Polylogarithm is licenced under the MIT License.
// ====================================================================

#pragma once

#include <cmath>
#include <complex>

namespace polylogarithm {


/// returns log(1 + z) for complex z
template <typename T>
std::complex<T> log1p(const std::complex<T>& z) noexcept
{
   const std::complex<T> u = T(1) + z;

   if (std::real(u) == T(1) && std::imag(u) == T(0)) {
      return z;
   } else if (std::real(u) <= T(0)) {
      return std::log(u);
   }

   return std::log(u)*(z/(u - T(1)));
}


/**
 * returns log(z) for complex z
 * @param z complex argument
 * @note Points on the branch cut are treated differently from std::log(z):
 * Points with Re(z) < 0 and Im(z) == -0.0 are mapped to Im(z) == 0.0
 * @return log(z)
 */
template <typename T>
std::complex<T> pos_log(const std::complex<T>& z) noexcept
{
   if (std::imag(z) == T(0) && std::real(z) > T(0)) {
      return { std::log(std::real(z)), T(0) };
   } else if (std::imag(z) == T(0)) {
      return { std::log(-std::real(z)), 4*std::atan(T(1)) };
   }

   return std::log(z);
}


} // namespace polylogarithm

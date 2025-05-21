// ====================================================================
// This file is part of Polylogarithm.
//
// Polylogarithm is licenced under the MIT License.
// ====================================================================

#pragma once

#include <complex>

namespace polylogarithm {

template <typename T, int N>
T horner(T x, const T (&c)[N]) noexcept
{
   T p = c[N - 1];
   for (int i = N - 2; i >= 0; --i) {
      p = p*x + c[i];
   }
   return p;
}


template <int Nstart, typename T, int N>
std::complex<T> horner(const std::complex<T>& z, const T (&coeffs)[N]) noexcept
{
   static_assert(0 <= Nstart && Nstart < N && N >= 2, "invalid array bounds");

   const T rz = std::real(z);
   const T iz = std::imag(z);
   const T r = rz + rz;
   const T s = std::norm(z);
   T a = coeffs[N - 1], b = coeffs[N - 2];

   for (int i = N - 3; i >= Nstart; --i) {
      const T t = a;
      a = b + r*a;
      b = coeffs[i] - s*t;
   }

   return { rz*a + b, iz*a };
}

} // namespace polylogarithm

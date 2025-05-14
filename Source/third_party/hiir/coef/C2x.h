/*****************************************************************************

        C2x.h
        Author: Laurent de Soras, 2024

Sets of coefficients for oversampling (upsampling then downsampling)
Cumulated operations have an integer phase delay in number of samples.
See oversampling.txt for more information.

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law.You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://www.wtfpl.net/ for more details.

*Tab=3***********************************************************************/



#pragma once
#if ! defined (hiir_coef_C2x_HEADER_INCLUDED)
#define hiir_coef_C2x_HEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include <array>



namespace hiir
{
namespace coef
{



// 156 dB
class C2x156
{
public:
	static constexpr float _attn   = 156;  // Attenuation, dB
	static constexpr int   _ovr_l2 = 1;    // Log2 of the oversampling ratio
	static constexpr int   _delay  = 5;    // Phase delay, samples
	static constexpr float _bw     = 19968.f / 44100.f; // Bandwidth

	// Attenuation : 156.686 dB
	// Trans BW    : 0.0472053
	// Phase delay : 5 spl
	// Group delay : 5.14 spl
	// Dly rel freq: 0.0453515
	class X2
	{
	public:
		static constexpr int _nbr_coef = 12;
		typedef std::array <double, _nbr_coef> CoefList;
		static constexpr CoefList _coef_list =
		{
			0.017347915108876406,
			0.067150480426919179,
			0.14330738338179819,
			0.23745131944299824,
			0.34085550201503761,
			0.44601111310335906,
			0.54753112652956148,
			0.6423859124721446,
			0.72968928615804163,
			0.81029959388029904,
			0.88644514917318362,
			0.96150605146543733
		};
	};
};



// -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -



// 125 dB
class C2x125
{
public:
	static constexpr float _attn   = 125;  // Attenuation, dB
	static constexpr int   _ovr_l2 = 1;    // Log2 of the oversampling ratio
	static constexpr int   _delay  = 4;    // Phase delay, samples
	static constexpr float _bw     = 20213.f / 44100.f; // Bandwidth

	// Attenuation : 125.619 dB
	// Trans BW    : 0.0416368
	// Phase delay : 4 spl
	// Group delay : 4.11568 spl
	// Dly rel freq: 0.0453515
	class X2
	{
	public:
		static constexpr int _nbr_coef = 10;
		typedef std::array <double, _nbr_coef> CoefList;
		static constexpr CoefList _coef_list =
		{
			0.026280277370383145,
			0.099994562200117765,
			0.20785425737827937,
			0.33334081139102473,
			0.46167004060691091,
			0.58273462309510859,
			0.69172302956824328,
			0.78828933879250873,
			0.87532862123185262,
			0.9580617608216595
		};
	};
};



// -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -



// 94 dB
class C2x094
{
public:
	static constexpr float _attn   = 94;   // Attenuation, dB
	static constexpr int   _ovr_l2 = 1;    // Log2 of the oversampling ratio
	static constexpr int   _delay  = 3;    // Phase delay, samples
	static constexpr float _bw     = 20534.f / 44100.f; // Bandwidth

	// Attenuation : 94.8751 dB
	// Trans BW    : 0.0343747
	// Phase delay : 3 spl
	// Group delay : 3.09123 spl
	// Dly rel freq: 0.0453515
	class X2
	{
	public:
		static constexpr int _nbr_coef = 8;
		typedef std::array <double, _nbr_coef> CoefList;
		static constexpr CoefList _coef_list =
		{
			0.044076093956155402,
			0.16209555156378622,
			0.32057678606990592,
			0.48526821501990786,
			0.63402005787429128,
			0.75902855561016014,
			0.86299283427175177,
			0.9547836337311687
		};
	};
};



// -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -



// 64 dB
class C2x064
{
public:
	static constexpr float _attn   = 64;   // Attenuation, dB
	static constexpr int   _ovr_l2 = 1;    // Log2 of the oversampling ratio
	static constexpr int   _delay  = 2;    // Phase delay, samples
	static constexpr float _bw     = 20961.f / 44100.f; // Bandwidth

	// Attenuation : 64.6458 dB
	// Trans BW    : 0.0246851
	// Phase delay : 2 spl
	// Group delay : 2.06607 spl
	// Dly rel freq: 0.0453515
	class X2
	{
	public:
		static constexpr int _nbr_coef = 6;
		typedef std::array <double, _nbr_coef> CoefList;
		static constexpr CoefList _coef_list =
		{
			0.086928900551398763,
			0.29505822040137708,
			0.52489392936346835,
			0.7137336652558357,
			0.85080135560651127,
			0.95333447720743869
		};
	};
};



}  // namespace coef
}  // namespace hiir



//#include "hiir/coef/C2x.hpp"



#endif // hiir_coef_C2x_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

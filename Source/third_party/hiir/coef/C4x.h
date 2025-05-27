/*****************************************************************************

        C4x.h
        Author: Laurent de Soras, 2024

Sets of coefficients for oversampling (upsampling then downsampling)
Cumulated operations have an integer phase delay in number of samples.
Cascaded 4x oversampling.
See oversampling.txt for more information.

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law.You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://www.wtfpl.net/ for more details.

*Tab=3***********************************************************************/



#pragma once
#if ! defined (hiir_coef_C4x_HEADER_INCLUDED)
#define hiir_coef_C4x_HEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include <array>



namespace hiir
{
namespace coef
{



// 140 dB
class C4x140
{
public:
	static constexpr float _attn   = 140;  // Attenuation, dB
	static constexpr int   _ovr_l2 = 2;    // Log2 of the oversampling ratio
	static constexpr int   _delay  = 6;    // Phase delay, samples
	static constexpr float _bw     = 20594.f / 44100.f; // Bandwidth

	// Attenuation : 140.387 dB
	// Trans BW    : 0.241371
	// Phase delay : 2.796 spl
	// Group delay : 2.81247 spl
	// Dly rel freq: 0.0226757
	class X4
	{
	public:
		static constexpr int _nbr_coef = 5;
		typedef std::array <double, _nbr_coef> CoefList;
		static constexpr CoefList _coef_list =
		{
			0.029113887601773612,
			0.11638402872809682,
			0.26337786480329456,
			0.47885453461538624,
			0.78984065611473109
		};
	};

	// Attenuation : 140.698 dB
	// Trans BW    : 0.0330036
	// Phase delay : 4.602 spl
	// Group delay : 4.73517 spl
	// Dly rel freq: 0.0453515
	class X2
	{
	public:
		static constexpr int _nbr_coef = 12;
		typedef std::array <double, _nbr_coef> CoefList;
		static constexpr CoefList _coef_list =
		{
			0.021155607771239357,
			0.081229227715837876,
			0.17117329577828599,
			0.27907679095036358,
			0.39326146586620897,
			0.50450550469712818,
			0.60696304442748228,
			0.69802237610653928,
			0.77761801388575091,
			0.84744854091978927,
			0.91036460053334245,
			0.97003180383006626
		};
	};
};



// -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -



// 120 dB
class C4x120
{
public:
	static constexpr float _attn   = 120;  // Attenuation, dB
	static constexpr int   _ovr_l2 = 2;    // Log2 of the oversampling ratio
	static constexpr int   _delay  = 5;    // Phase delay, samples
	static constexpr float _bw     = 20428.f / 44100.f; // Bandwidth

	// Attenuation : 120.958 dB
	// Trans BW    : 0.261666
	// Phase delay : 2.2324 spl
	// Group delay : 2.2459 spl
	// Dly rel freq: 0.0226757
	class X4
	{
	public:
		static constexpr int _nbr_coef = 4;
		typedef std::array <double, _nbr_coef> CoefList;
		static constexpr CoefList _coef_list =
		{
			0.041180778598107023,
			0.1665604775598164,
			0.38702422374344198,
			0.74155297339931314
		};
	};

	// Attenuation : 120.98 dB
	// Trans BW    : 0.0367598
	// Phase delay : 3.8838 spl
	// Group delay : 3.99747 spl
	// Dly rel freq: 0.0453515
	class X2
	{
	public:
		static constexpr int _nbr_coef = 10;
		typedef std::array <double, _nbr_coef> CoefList;
		static constexpr CoefList _coef_list =
		{
			0.028143361249169534,
			0.10666337918578024,
			0.22039215120527197,
			0.35084569997865528,
			0.48197792985533633,
			0.60331147102003924,
			0.7102921937907698,
			0.80307423332343497,
			0.88500411159151648,
			0.96155188130366132
		};
	};
};



// -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -



// 99 dB
class C4x099
{
public:
	static constexpr float _attn   = 99;   // Attenuation, dB
	static constexpr int   _ovr_l2 = 2;    // Log2 of the oversampling ratio
	static constexpr int   _delay  = 4;    // Phase delay, samples
	static constexpr float _bw     = 19084.f / 44100.f; // Bandwidth

	// Attenuation : 99.2112 dB
	// Trans BW    : 0.283623
	// Phase delay : 1.655 spl
	// Group delay : 1.66555 spl
	// Dly rel freq: 0.0226757
	class X4
	{
	public:
		static constexpr int _nbr_coef = 3;
		typedef std::array <double, _nbr_coef> CoefList;
		static constexpr CoefList _coef_list =
		{
			0.064871212918289664,
			0.26990325432357809,
			0.67132720810807256
		};
	};

	// Attenuation : 101.729 dB
	// Trans BW    : 0.0432267
	// Phase delay : 3.1725 spl
	// Group delay : 3.26671 spl
	// Dly rel freq: 0.0453515
	class X2
	{
	public:
		static constexpr int _nbr_coef = 8;
		typedef std::array <double, _nbr_coef> CoefList;
		static constexpr CoefList _coef_list =
		{
			0.038927716817571831,
			0.1447065207203321,
			0.29070001093670539,
			0.44813928150889282,
			0.59667390381274976,
			0.72756709523681729,
			0.84178734600949523,
			0.94699056169241524
		};
	};
};



// -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -



// 77 dB
class C4x077
{
public:
	static constexpr float _attn   = 77;   // Attenuation, dB
	static constexpr int   _ovr_l2 = 2;    // Log2 of the oversampling ratio
	static constexpr int   _delay  = 4;    // Phase delay, samples
	static constexpr float _bw     = 21335.f / 44100.f; // Bandwidth

	// Attenuation : 79.6658 dB
	// Trans BW    : 0.212423
	// Group delay : 2.06 spl
	// Dly rel freq: 0.00566893
	class X4
	{
	public:
		static constexpr int _nbr_coef = 3;
		typedef std::array <double, _nbr_coef> CoefList;
		static constexpr CoefList _coef_list =
		{
			0.07819449364682253,
			0.30699114982473047,
			0.70279393407418822
		};
	};

	// Attenuation : 77.468 dB
	// Trans BW    : 0.016203
	// Group delay : 2.97 spl
	// Dly rel freq: 0.0113379
	class X2
	{
	public:
		static constexpr int _nbr_coef = 8;
		typedef std::array <double, _nbr_coef> CoefList;
		static constexpr CoefList _coef_list =
		{
			0.063197755826959232,
			0.22379856090736752,
			0.41991987445736001,
			0.60001868664013414,
			0.74142961506492877,
			0.84386929889458695,
			0.917212853263219,
			0.97391838508705231
		};
	};
};



}  // namespace coef
}  // namespace hiir



//#include "hiir/coef/C4x.hpp"



#endif // hiir_coef_C4x_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

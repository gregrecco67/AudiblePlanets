/*****************************************************************************

        C8x.h
        Author: Laurent de Soras, 2024

Sets of coefficients for oversampling (upsampling then downsampling)
Cumulated operations have an integer phase delay in number of samples.
Cascaded 8x oversampling.
See oversampling.txt for more information.

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law.You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://www.wtfpl.net/ for more details.

*Tab=3***********************************************************************/



#pragma once
#if ! defined (hiir_coef_C8x_HEADER_INCLUDED)
#define hiir_coef_C8x_HEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include <array>



namespace hiir
{
namespace coef
{



// 133 dB
class C8x133
{
public:
	static constexpr float _attn   = 133;  // Attenuation, dB
	static constexpr int   _ovr_l2 = 3;    // Log2 of the oversampling ratio
	static constexpr int   _delay  = 6;    // Phase delay, samples
	static constexpr float _bw     = 19814.f / 44100.f; // Bandwidth

	// Attenuation : 133.371 dB
	// Trans BW    : 0.373128
	// Phase delay : 1.717 spl
	// Group delay : 1.7196 spl
	// Dly rel freq: 0.0113379
	class X8
	{
	public:
		static constexpr int _nbr_coef = 3;
		typedef std::array <double, _nbr_coef> CoefList;
		static constexpr CoefList _coef_list =
		{
			0.056028689580145966,
			0.2438952031065017,
			0.64749960965360798
		};
	};

	// Attenuation : 133.465 dB
	// Trans BW    : 0.224566
	// Phase delay : 2.76 spl
	// Group delay : 2.77651 spl
	// Dly rel freq: 0.0226757
	class X4
	{
	public:
		static constexpr int _nbr_coef = 5;
		typedef std::array <double, _nbr_coef> CoefList;
		static constexpr CoefList _coef_list =
		{
			0.030628870339135559,
			0.12166431142077823,
			0.27260436967450397,
			0.48939820546688356,
			0.79574591597790167
		};
	};

	// Attenuation : 133.697 dB
	// Trans BW    : 0.0507022
	// Phase delay : 4.19075 spl
	// Group delay : 4.30946 spl
	// Dly rel freq: 0.0453515
	class X2
	{
	public:
		static constexpr int _nbr_coef = 10;
		typedef std::array <double, _nbr_coef> CoefList;
		static constexpr CoefList _coef_list =
		{
			0.02347550947773171,
			0.089865847585516564,
			0.18855110667410455,
			0.30593155751915702,
			0.42927272772713948,
			0.54927694585933118,
			0.6609721246235537,
			0.76339208090846,
			0.85879504640138027,
			0.95202395338793633
		};
	};
};



// -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -



// 129 dB
class C8x129
{
public:
	static constexpr float _attn   = 129;  // Attenuation, dB
	static constexpr int   _ovr_l2 = 3;    // Log2 of the oversampling ratio
	static constexpr int   _delay  = 6;    // Phase delay, samples
	static constexpr float _bw     = 19006.f / 44100.f; // Bandwidth

	// Attenuation : 142.312 dB
	// Trans BW    : 0.3901
	// Phase delay : 1.725 spl
	// Group delay : 1.7276 spl
	// Dly rel freq: 0.0113379
	class X8
	{
	public:
		static constexpr int _nbr_coef = 3;
		typedef std::array <double, _nbr_coef> CoefList;
		static constexpr CoefList _coef_list =
		{
			0.055006689941721712,
			0.24080823797621789,
			0.64456305579326278
		};
	};

	// Attenuation : 129.624 dB
	// Trans BW    : 0.284503
	// Phase delay : 2.2634 spl
	// Group delay : 2.27686 spl
	// Dly rel freq: 0.0226757
	class X4
	{
	public:
		static constexpr int _nbr_coef = 4;
		typedef std::array <double, _nbr_coef> CoefList;
		static constexpr CoefList _coef_list =
		{
			0.039007192037884615,
			0.15936900049878461,
			0.37605311415523662,
			0.7341922816405595
		};
	};

	// Attenuation : 145.17 dB
	// Trans BW    : 0.0646551
	// Phase delay : 4.43705 spl
	// Group delay : 4.55916 spl
	// Dly rel freq: 0.0453515
	class X2
	{
	public:
		static constexpr int _nbr_coef = 10;
		typedef std::array <double, _nbr_coef> CoefList;
		static constexpr CoefList _coef_list =
		{
			0.020259561571818407,
			0.07811926635621437,
			0.165767029808395,
			0.27285653675862664,
			0.38920012889842326,
			0.50682082231815651,
			0.62096457947279504,
			0.73023836303677192,
			0.83631543394763719,
			0.94366551176113678
		};
	};
};



// -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -



// 115 dB
class C8x115
{
public:
	static constexpr float _attn   = 115;  // Attenuation, dB
	static constexpr int   _ovr_l2 = 3;    // Log2 of the oversampling ratio
	static constexpr int   _delay  = 5;    // Phase delay, samples
	static constexpr float _bw     = 19280.f / 44100.f; // Bandwidth

	// Attenuation : 115.108 dB
	// Trans BW    : 0.330575
	// Phase delay : 1.691 spl
	// Group delay : 1.6936 spl
	// Dly rel freq: 0.0113379
	class X8
	{
	public:
		static constexpr int _nbr_coef = 3;
		typedef std::array <double, _nbr_coef> CoefList;
		static constexpr CoefList _coef_list =
		{
			0.059424895788696495,
			0.25402903012559713,
			0.65697309449004115
		};
	};

	// Attenuation : 115.306 dB
	// Trans BW    : 0.245813
	// Phase delay : 2.208 spl
	// Group delay : 2.22153 spl
	// Dly rel freq: 0.0226757
	class X4
	{
	public:
		static constexpr int _nbr_coef = 4;
		typedef std::array <double, _nbr_coef> CoefList;
		static constexpr CoefList _coef_list =
		{
			0.042942076687509392,
			0.17233198394244389,
			0.39570312028031407,
			0.74727683965475022
		};
	};

	// Attenuation : 115.174 dB
	// Trans BW    : 0.0627931
	// Phase delay : 3.47325 spl
	// Group delay : 3.57186 spl
	// Dly rel freq: 0.0453515
	class X2
	{
	public:
		static constexpr int _nbr_coef = 8;
		typedef std::array <double, _nbr_coef> CoefList;
		static constexpr CoefList _coef_list =
		{
			0.031273333212550401,
			0.11822374968950244,
			0.24354908560025812,
			0.38702325277755739,
			0.53245171533105751,
			0.67117027939107099,
			0.80235276007160961,
			0.93204839936599315
		};
	};
};



// -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -



// 96 dB
class C8x096
{
public:
	static constexpr float _attn   = 96;   // Attenuation, dB
	static constexpr int   _ovr_l2 = 3;    // Log2 of the oversampling ratio
	static constexpr int   _delay  = 4;    // Phase delay, samples
	static constexpr float _bw     = 19649.f / 44100.f; // Bandwidth

	// Attenuation : 97.6237 dB
	// Trans BW    : 0.384234
	// Phase delay : 1.1 spl
	// Group delay : 1.1018 spl
	// Dly rel freq: 0.0113379
	class X8
	{
	public:
		static constexpr int _nbr_coef = 2;
		typedef std::array <double, _nbr_coef> CoefList;
		static constexpr CoefList _coef_list =
		{
			0.11145973802055159,
			0.5389183683274309
		};
	};

	// Attenuation : 96.2787 dB
	// Trans BW    : 0.273878
	// Phase delay : 1.645 spl
	// Group delay : 1.65554 spl
	// Dly rel freq: 0.0226757
	class X4
	{
	public:
		static constexpr int _nbr_coef = 3;
		typedef std::array <double, _nbr_coef> CoefList;
		static constexpr CoefList _coef_list =
		{
			0.066279281652775912,
			0.27393598622393989,
			0.67488440000123284
		};
	};

	// Attenuation : 96.0343 dB
	// Trans BW    : 0.0544272
	// Phase delay : 2.9025 spl
	// Group delay : 2.98819 spl
	// Dly rel freq: 0.0453515
	class X2
	{
	public:
		static constexpr int _nbr_coef = 7;
		typedef std::array <double, _nbr_coef> CoefList;
		static constexpr CoefList _coef_list =
		{
			0.043525161938154609,
			0.16076108002341286,
			0.32024325682326032,
			0.48987617815497958,
			0.64952123936442452,
			0.79389404748996595,
			0.9300378703058515
		};
	};
};



}  // namespace coef
}  // namespace hiir



//#include "hiir/coef/C8x.hpp"



#endif // hiir_coef_C8x_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

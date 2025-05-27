/*****************************************************************************

        StageDataSse.h
        Author: Laurent de Soras, 2005

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#pragma once
#if ! defined (hiir_StageDataSse_HEADER_INCLUDED)
#define hiir_StageDataSse_HEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



namespace hiir
{



class StageDataSse
{

public:

	alignas (16) float
	               _coef [4];  // a_{4n+1}, a_{4n}, a_{4n+3}, a_{4n+2}
	alignas (16) float
	               _mem [4];   // y of the stage

}; // class StageDataSse



}  // namespace hiir



#endif   // hiir_StageDataSse_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

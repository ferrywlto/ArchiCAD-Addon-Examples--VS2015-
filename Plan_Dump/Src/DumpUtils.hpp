// *********************************************************************************************************************
// Description:		Interface of the DumpUtils class
//
// Module:			API Development Kit 19; Mac/Win
// Namespace:		-none-
// Contact person:	devsup@graphisoft.hu
//
// SG compatible
// *********************************************************************************************************************

#if !defined (DUMPUTILS_HPP)
#define DUMPUTILS_HPP

#pragma once

// --- Includes	----------------------------------------------------------------

#include	"APIEnvir.h"

#include	"ACAPinc.h"					// also includes APIdefs.h


// --- Constant definitions ----------------------------------------------------

#define		PI					3.14159265358979323846
#define		RADDEG				(180.0/PI)


// --- class DumpUtils --------------------------------------------------------

class DumpUtils {
public:
	static Int32	Element_To_LibPartIndex (const API_Element *elem);
	static void		GetAttrName	   (char* name, API_AttrTypeID typeID, short index, GS::UniString* uniName = NULL);
	static void		Get_DoorVector (const API_DoorType*	door,
									double 				wradius,
									double 				dangle,
									double* 			loc_x,
									double* 			loc_y,
									double* 			angle);
	static void		GetWallData	   (const API_WallType* wall, double* angle, double* radius);
};

#endif

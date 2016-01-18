// *********************************************************************************************************************
// Description:		Implementation of the DumpUtils class
//
// Module:			API Development Kit 19; Mac/Win
// Namespace:		-none-
// Contact person:	devsup@graphisoft.hu
//
// SG compatible
// *********************************************************************************************************************

// --- Includes	----------------------------------------------------------------

#include	"Location.hpp"

#include	"DumpUtils.hpp"

#include	"GenArc2DData.h"


// -----------------------------------------------------------------------------
// Get the name of an Attribute specified by index
// -----------------------------------------------------------------------------

void DumpUtils::GetAttrName (char*				name,
							 API_AttrTypeID		typeID,
							 short 				index,
							 GS::UniString*		uniName)
{
	API_Attribute	attrib;

	BNZeroMemory (&attrib, sizeof (API_Attribute));
	attrib.header.typeID = typeID;
	attrib.header.index = index;
	attrib.header.uniStringNamePtr = uniName;

	if (ACAPI_Attribute_Get (&attrib) == NoError) {
		if (name != NULL)
			strcpy (name, attrib.header.name);
	} else if (name != NULL)
		strcpy (name, "???");

	if (typeID == API_MaterialID && attrib.material.texture.fileLoc != NULL) {
		delete attrib.material.texture.fileLoc;
		attrib.material.texture.fileLoc = NULL;
	}
	return;
}		// GetAttrName


// -----------------------------------------------------------------------------
//	Get the library part index from the element record
// -----------------------------------------------------------------------------

Int32 DumpUtils::Element_To_LibPartIndex (const API_Element* elem)
{
	Int32	libInd;

	switch (elem->header.typeID) {

		case API_WindowID:
		case API_DoorID:		libInd = elem->door.openingBase.libInd;
								break;

		case API_ObjectID:
		case API_LampID:		libInd = elem->lamp.libInd;
								break;

		case API_ZoneID:		libInd = elem->zone.libInd;
								break;

		default:				libInd = 0;
	}

	return libInd;
}		// Element_To_LibPartIndex


// -----------------------------------------------------------------------------
// Get the vector of a door
// -----------------------------------------------------------------------------

void DumpUtils::Get_DoorVector (const API_DoorType*	door,
								double 				wradius,
								double 				dangle,
								double*				loc_x,
								double* 			loc_y,
								double* 			angle)
{
	*loc_x = door->objLoc * cos (dangle);
	*loc_y = door->objLoc * sin (dangle);

	if (fabs (wradius) < EPS)
		*angle = 0.0;
	else
		*angle = atan2 (*loc_y, *loc_x);

	return;
}		// Get_DoorVector


// -----------------------------------------------------------------------------
// Get angle and radius of a wall
// -----------------------------------------------------------------------------

void DumpUtils::GetWallData (const API_WallType*	wall,
							 double*				angle,
							 double*				radius)
{
	Coord	beg, end, origo;

	beg.x = wall->begC.x;
	beg.y = wall->begC.y;
	end.x = wall->endC.x;
	end.y = wall->endC.y;

	if (Geometry::ArcGetOrigo (&beg, &end, wall->angle, &origo)) {
		API_Coord	r;
		r.x = origo.x - wall->begC.x;
		r.y = origo.y - wall->begC.y;
		*radius = sqrt (r.x * r.x + r.y * r.y);

		if (wall->flipped)
			*radius += wall->offsetFromOutside;
		else
			*radius += wall->thickness - wall->offsetFromOutside;
	} else
		*radius = 0;

	double	dx = wall->endC.x - wall->begC.x;
	double	dy = wall->endC.y - wall->begC.y;

	if (fabs (dx) > EPS)
		*angle = atan2 (dy , dx);
	else
		*angle = PI / 2.0;

	return;
}		// GetWallData

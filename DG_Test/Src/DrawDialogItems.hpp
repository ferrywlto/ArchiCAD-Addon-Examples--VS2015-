// *****************************************************************************
// Header file for drawing dialog items in Building Material Dialog
// API Development Kit 19; Mac/Win
//
// Namespaces:        Contact person:
//     -None-						LT
//
// [SG compatible] - Yes
// *****************************************************************************

#include	"APIEnvir.h"
#include	"ACAPinc.h"					// also includes APIdefs.h

#include	"DGModule.hpp"
#include	"DGNativeContexts.hpp"
#include	"FillEffects.hpp"


bool	DrawPen					(NewDisplay::NativeContext& drawContext, const DG::Rect& rect, short penIndex);
bool	DrawBuildingMaterial	(NewDisplay::NativeContext& drawContext, const DG::Rect& rect, short buildMatIndex);

GSErrCode Attribute_Get (API_Attribute* attr, const API_AttrTypeID& typeID, const short index);
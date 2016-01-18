// *****************************************************************************
// Source code for the Element Test Add-On
// API Development Kit 19; Mac/Win
//
//	Main and common functions
//
// Namespaces:		Contact person:
//		-None-
//
// [SG compatible] - Yes
// *****************************************************************************

#include "APIEnvir.h"
#define	_ELEMENT_TEST_TRANSL_


// ---------------------------------- Includes ---------------------------------

#include	<string.h>

#include	"GSRoot.hpp"
#include	"ACAPinc.h"					// also includes APIdefs.h

#include	"APICommon.h"
#include	"Element_Test.h"
#include	"APISpeedTest.hpp"


// ---------------------------------- Types ------------------------------------


// ---------------------------------- Variables --------------------------------

static API_Guid	gGuid = APINULLGuid;
static API_Guid	gLastRenFiltGuid = APINULLGuid;


// ---------------------------------- Prototypes -------------------------------



// =============================================================================
//
// Utility functions
//
// =============================================================================

// -----------------------------------------------------------------------------
// Get an edit vector for editing operations
// -----------------------------------------------------------------------------

bool	GetEditVector (API_Coord3D	*begC,
					   API_Coord3D	*endC,
					   const char	*prompt,
					   bool			begGiven)
{
	API_GetPointType	pointInfo;
	API_GetLineType		lineInfo;
	GSErrCode			err;

	err = NoError;

	if (!begGiven) {
		BNZeroMemory (&pointInfo, sizeof (API_GetPointType));
		BNZeroMemory (&lineInfo, sizeof (API_GetLineType));
		CHCopyC (prompt, pointInfo.prompt);
		err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
	} else
		pointInfo.pos = *begC;

	if (err == NoError) {
		BNZeroMemory (&lineInfo, sizeof (API_GetLineType));
		CHCopyC ("Complete the edit vector", lineInfo.prompt);
		lineInfo.startCoord = pointInfo.pos;
		err = ACAPI_Interface (APIIo_GetLineID, &lineInfo, NULL);
		if (err == NoError) {
			*begC = lineInfo.startCoord;
			*endC = lineInfo.pos;
		}
	}

	if (err != NoError)
		ErrorBeep ("APICmd_GetLineID", err);

	return (bool) (err == NoError);
}		// GetEditVector


// -----------------------------------------------------------------------------
// Get an arc for editing operations
// -----------------------------------------------------------------------------

bool	GetEditArc (API_Coord3D	*begC,
					API_Coord3D *endC,
					API_Coord3D *origC,
					const char	*prompt)
{
	API_GetPointType	pointInfo;
	API_GetLineType		lineInfo;
	API_GetArcType		arcInfo;
	GSErrCode			err;

	BNZeroMemory (&pointInfo, sizeof (API_GetPointType));
	BNZeroMemory (&lineInfo, sizeof (API_GetLineType));
	BNZeroMemory (&arcInfo, sizeof (API_GetArcType));
	CHCopyC (prompt, pointInfo.prompt);
	pointInfo.changeFilter = false;
	pointInfo.changePlane  = false;
	err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);

	if (err == NoError) {
		BNZeroMemory (&lineInfo, sizeof (API_GetLineType));
		CHCopyC ("Enter the arc start point", lineInfo.prompt);
		lineInfo.changeFilter = false;
		lineInfo.changePlane  = false;
		lineInfo.startCoord   = pointInfo.pos;
		err = ACAPI_Interface (APIIo_GetLineID, &lineInfo, NULL);
	}

	if (err == NoError) {
		BNZeroMemory (&arcInfo, sizeof (API_GetArcType));
		CHCopyC ("Enter the arc end point", arcInfo.prompt);
		arcInfo.origo			= lineInfo.startCoord;
		arcInfo.startCoord		= lineInfo.pos;
		arcInfo.startCoordGiven = true;
		err = ACAPI_Interface (APIIo_GetArcID, &arcInfo, NULL);
	}

	if (err != NoError) {
		ErrorBeep ("APIIo_GetArcID", err);
		return (bool) (err == NoError);
	}

	*origC	= arcInfo.origo;
	if (arcInfo.negArc) {
		*begC = arcInfo.pos;
		*endC = arcInfo.startCoord;
	} else {
		*begC = arcInfo.startCoord;
		*endC = arcInfo.pos;
	}

	return (bool) (err == NoError);
}		// GetEditArc


// -----------------------------------------------------------------------------
// Search for the active camset
//  Also search for a camset with the type of the active one.
//  If no other camsets exist, the active index is returned
// -----------------------------------------------------------------------------

void	SearchActiveCamset (API_Guid*	actGuid,
							API_Guid*	perspGuid)
{
	if (actGuid != NULL)
		*actGuid = APINULLGuid;
	if (perspGuid != NULL)
		*perspGuid = APINULLGuid;

	GS::Array<API_Guid>	camSetList;
	if (ACAPI_Element_GetElemList (API_CamSetID, &camSetList) != NoError)
		return;

	API_Element	element;
	for (GS::Array<API_Guid>::ConstIterator it = camSetList.Enumerate (); it != NULL; ++it) {
		BNZeroMemory (&element, sizeof (API_Element));
		element.header.guid = *it;

		const GSErrCode err = ACAPI_Element_Get (&element);
		if (err != NoError) {
			ErrorBeep ("ACAPI_Element_Get (camset)", err);
			continue;
		}

		if (element.camset.active) {
			if (actGuid != NULL)
				*actGuid = *it;
		} else {			/* first from every type which is inactive */
			if (perspGuid != NULL)
				*perspGuid = *it;
		}
	}

	if (*actGuid == APINULLGuid)
		WriteReport_Alert ("No camsets, please create one");
}


// -----------------------------------------------------------------------------
// Dump a polygon
// -----------------------------------------------------------------------------

bool DumpPolygon (const API_Element	*element,
				  short				lineInd,
				  const double		offset,
				  Int32				nCoords,
				  Int32				nSubPolys,
				  Int32				nArcs,
				  API_Coord			**coords,
				  Int32				**subPolys,
				  API_PolyArc		**arcs,
				  bool				createShadow,
				  bool				writeReport)
{
	API_Element	elemL, elemA;
	API_Coord	begC, endC, origC;
	double		angle;
	Int32		j,k, begInd, endInd, arcInd;
	GSErrCode	err;

	if (nCoords < 4  || coords == NULL || subPolys == NULL ||
			(nArcs > 0 && arcs == NULL) || element == NULL)
		return false;

	BNZeroMemory (&elemL, sizeof (API_Element));
	BNZeroMemory (&elemA, sizeof (API_Element));
	elemL.header.typeID = API_LineID;
	elemA.header.typeID = API_ArcID;

	err = ACAPI_Element_GetDefaults (&elemL, NULL);
	err |= ACAPI_Element_GetDefaults (&elemA, NULL);
	if (err != NoError)
		return false;

	elemL.line.ltypeInd   = lineInd;
	elemL.header.floorInd = element->header.floorInd;
	elemL.header.layer    = element->header.layer;

	elemL.arc.ltypeInd    = lineInd;
	elemA.header.floorInd = element->header.floorInd;
	elemA.header.layer    = element->header.layer;
	elemA.arc.ratio		  = 1.0;


	for (j = 1; j <= nSubPolys && err == NoError; j++) {
		if (writeReport && j > 1)
			WriteReport ("  ---");
		begInd = (*subPolys) [j-1] + 1;
		endInd = (*subPolys) [j];

		for (k = begInd; k < endInd && err == NoError; k++) {
			begC = (*coords) [k];
			endC = (*coords) [k+1];
			begC.x += offset;
			endC.x += offset;
			begC.y += offset;
			endC.y += offset;
			if (arcs != NULL && nArcs > 0)
				arcInd = FindArc (*arcs, nArcs, k);
			else
				arcInd = -1;
			if (arcInd < 0) {
				elemL.line.begC = begC;
				elemL.line.endC = endC;
				if (createShadow)
					err = ACAPI_Element_Create (&elemL, NULL);
				if (writeReport)
					WriteReport ("  (%lf, %lf)", begC.x, begC.y);
			} else {
				angle = (*arcs) [arcInd].arcAngle;
				ArcGetOrigo (&begC, &endC, angle, &origC);
				elemA.arc.origC = origC;
				elemA.arc.r = DistCPtr (&origC, &begC);
				if (angle > 0.0) {
					elemA.arc.begAng = ComputeFiPtr (&origC, &begC);
					elemA.arc.endAng = ComputeFiPtr (&origC, &endC);
				} else {
					elemA.arc.endAng = ComputeFiPtr (&origC, &begC);
					elemA.arc.begAng = ComputeFiPtr (&origC, &endC);
				}
				if (createShadow)
					err = ACAPI_Element_Create (&elemA, NULL);
				if (writeReport)
					WriteReport ("  (%lf, %lf) with angle %lf deg", begC.x, begC.y, angle * RADDEG);
			}
		}
	}

	if (err != NoError) {
		ErrorBeep ("Unable to dump polygon", err);
		return false;
	}

	return true;
}		// DumpPolygon


// -----------------------------------------------------------------------------
// ElementsCmd: Create, Delete, Select functions
// -----------------------------------------------------------------------------
GSErrCode __ACENV_CALL ElementsCmd (const API_MenuParams *menuParams)
{
	ACAPI_KeepInMemory (false);

	return ACAPI_CallUndoableCommand ("Element Test API Function",
		[&] () -> GSErrCode {

			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		Do_CountLines ();					break;
				case 2:		Do_CreateLine (gGuid);				break;
				case 3:		Do_GetLineByGuid (gGuid);			break;
				/* ----- */
				case 5:		Do_CreateDetail ();					break;
				case 6:		Do_CreateLabel ();					break;
				case 7:		Do_CreateLabel_Associative ();		break;
				case 8:		Do_CreateZone (0);					break;
				case 9:		Do_CreateZone (1);					break;
				case 10:	Do_CreateZone (2);					break;
				case 11:	Do_CreateCamset ();					break;
				case 12:	Do_CreatePerspCam ();				break;
				case 13:	Do_CreatePicture ();				break;
				case 14:	Do_CreateGroupedLines ();			break;
				case 15:	Do_CreateCutPlane ();				break;
				case 16:	Do_CreateInteriorElevation ();		break;
				case 17:	Do_CreateWindow ();					break;
				case 18:	Do_CreateSkylight ();				break;
				case 19:	Do_CreatePolyRoof ();				break;
				case 20:	Do_CreateExtrudedShell ();			break;
				case 21:	Do_CreateRevolvedShell ();			break;
				case 22:	Do_CreateRuledShell ();				break;
				case 23:	Do_CreateMorph ();					break;
				case 24:	Do_CreateSlab ();					break;
				case 25:	Do_CreateCurvedWall ();				break;
				case 26:	Do_CreateCurvedBeam ();				break;
				case 27:	Do_CreateCurtainWall ();			break;
				case 28:	Do_CreateWord (gLastRenFiltGuid);	break;
				case 29:	Do_CreateDrawing (gGuid);			break;
				case 30:	Do_CreateDrawingFromPlanView ();	break;
				case 31:	Do_CreateDrawingFromSelection ();	break;
				case 32:	Do_CreateChangeMarker ();			break;
				/* ----- */
				case 34:	Do_SelectElems ();					break;
				case 35:	Do_DeleteElems ();					break;
				case 36:	Do_PickupElem ();					break;
				case 37:	Do_ChangeElem ();					break;
				case 38:	Do_ExplodeElem ();					break;
				case 39:	Do_CopyElem ();						break;
				case 40:	Do_PickupProperties ();				break;
				case 41:	Do_FillProperties ();				break;
				/* ----- */
				case 43:	Do_DumpElem (gLastRenFiltGuid);		break;
				/* ----- */
				case 45:	Do_SetElemCategories (false);		break;
				case 46:	Do_SetElemCategories (true);		break;
				/* ----- */
				case 48:	Do_CreateIrregularSlab ();			break;
				case 49:	Do_CreateIrregularMesh ();			break;
				case 50:	Do_CreateIrregularExtShell ();		break;
				case 51:	Do_CreateStaticDimension ();		break;
				case 52:	Do_CreateStaticAngleDimension ();	break;
				default:
					break;
			}

			return NoError;
		});
}		/* ElementsCmd */

// -----------------------------------------------------------------------------
// Elements: Edit Functions
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL ElementsEdit (const API_MenuParams *menuParams)
{
	return ACAPI_CallUndoableCommand ("Element Test API Function",
		[&] () -> GSErrCode {

			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		Do_DragElems (false);				break;
				case 2:		Do_StretchElems (false);			break;
				case 3:		Do_ResizeElems (false);				break;
				case 4:		Do_RotateElems (false);				break;
				case 5:		Do_MirrorElems (false);				break;
				case 6:		Do_ElevElems (false);				break;
				/* ----- */
				case 8:		Do_DragElems (true);				break;
				case 9:		Do_StretchElems (true);				break;
				case 10:	Do_ResizeElems (true);				break;
				case 11:	Do_RotateElems (true);				break;
				case 12:	Do_MirrorElems (true);				break;
				case 13:	Do_ElevElems (true);				break;

				default:										break;
			}

			return NoError;
		});
}		/* ElementsEdit */


// -----------------------------------------------------------------------------
// Elements: Tools Menu Functions
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL ElementsTools (const API_MenuParams *menuParams)
{
	return ACAPI_CallUndoableCommand ("Element Test API Function",
		[&] () -> GSErrCode {

			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		Do_GroupElems (false);				break;
				case 2:		Do_UngroupElems (false);			break;
				case 3:		Do_LockElems (false);				break;
				case 4:		Do_UnlockElems (false);				break;
				case 5:		Do_ForwardElems (false);			break;
				case 6:		Do_BackwardElems (false);			break;
				/* ----- */
				case 8:		Do_GroupElems (true);				break;
				case 9:		Do_UngroupElems (true);				break;
				case 10:	Do_LockElems (true);				break;
				case 11:	Do_UnlockElems (true);				break;
				case 12:	Do_ForwardElems (true);				break;
				case 13:	Do_BackwardElems (true);			break;
				/* ----- */
				case 15:	Do_SuspendGroups ();				break;

				default:										break;
			}

			return NoError;
		});
}		/* ElementsTools */


// -----------------------------------------------------------------------------
// Elements: Modify Functions
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL ElementsModify (const API_MenuParams *menuParams)
{
	return ACAPI_CallUndoableCommand ("Element Test API Function",
		[&] () -> GSErrCode {

			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		Do_Change_ElemInfo ();				break;
				case 2:		Do_Change_ElemParameters (false);	break;
				/* ----- */
				case 4:		Do_Change_ElemParameters (true);	break;
				/* ----- */
				case 6:		Do_Poly_AdjustNode ();				break;
				case 7:		Do_Poly_InsertNode ();				break;
				case 8:		Do_Poly_DeleteNode ();				break;
				case 9:		Do_Poly_DeleteHole ();				break;
				case 10:	Do_Poly_NewHole ();					break;
				/* ----- */
				case 12:	Do_Wall_Edit ();					break;
				case 13:	Do_Column_Edit ();					break;
				case 14:	Do_Beam_Edit ();					break;
				case 15:	Do_Window_Edit ();					break;
				case 16:	Do_Skylight_Edit ();				break;
				case 17:	Do_Object_Edit ();					break;
				case 18:	Do_Ceil_Edit ();					break;
				case 19:	Do_Roof_Edit ();					break;
				case 20:	Do_Mesh_Edit ();					break;
				case 21:	Do_Zone_Edit (1);					break;
				case 22:	Do_Word_Edit (1);					break;
				case 23:	Do_2D_Edit ();						break;
				case 24:	Do_Hotspot_Edit ();					break;
				case 25:	Do_Spline_Edit ();					break;
				case 26:	Do_CutPlane_Edit ();				break;
				case 27:	Do_Hatch_Edit ();					break;
				case 28:	Do_PolyLine_Edit ();				break;
				case 29:	Do_Label_Edit ();					break;
				case 30:	Do_Dimension_Edit ();				break;
				case 31:	Do_LevelDimension_Edit ();			break;
				case 32:	Do_RadialDimension_Edit ();			break;
				case 33:	Do_AngleDimension_Edit ();			break;
				case 34:	Do_Detail_Edit ();					break;
				case 35:	Do_Dimensions_Test ();				break;
				case 36:	Do_Drawing_Edit ();					break;
				case 37:	Do_ChangeInteriorElevation ();		break;
				case 38:	Do_Shell_Edit ();					break;
				case 39:	Do_Shell_Edit2 ();					break;
				case 40:	Do_Morph_Edit ();					break;
				case 41:	Do_ChangeMarker_Edit ();			break;
				/* ----- */
				case 43:	Do_RotateRenovationStatus ();		break;
				default:
							break;
			}

			return NoError;
		});
}		/* ElementsModify */

// -----------------------------------------------------------------------------
// ElementsTrick : useful tricks
// -----------------------------------------------------------------------------
GSErrCode __ACENV_CALL ElementsTrick (const API_MenuParams *menuParams)
{
	return ACAPI_CallUndoableCommand ("Element Test API Function",
		[&] () -> GSErrCode {

			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		Do_AvoidFlashing (1);				break;
				case 2:		Do_AvoidFlashing (2);				break;
				case 3:		Do_AvoidFlashing (3);				break;
				/* ----- */
				case 5:		Do_Speed_CreateSomeWalls ();		break;
				case 6:		Do_Speed_CreateSomeColumns ();		break;
				case 7:		Do_Speed_CreateSomeLabels ();		break;
				case 8:		Do_Speed_CreateSomeHotspots ();		break;
				case 9:		Do_Speed_DeleteAllOneByOne ();		break;
				case 10:	Do_Speed_DeleteAllWithOneCall ();	break;
				default:
					break;
			}

			return NoError;
		});
}		/* ElementsTrick */

// -----------------------------------------------------------------------------
// Elements: Group/Set
// -----------------------------------------------------------------------------
GSErrCode __ACENV_CALL ElementsGroup (const API_MenuParams *menuParams)
{
	return ACAPI_CallUndoableCommand ("Element Test API Function",
		[&] () -> GSErrCode {

			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		Do_ElemSet_Create ();				break;
				case 2:		Do_ElemSet_Delete ();				break;

				default:										break;
			}

			return NoError;
		});
}		/* ElementsGroup */


// -----------------------------------------------------------------------------
// Elements: Solid Operations Functions
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL ElementsSolidOperation (const API_MenuParams *menuParams)
{
	return ACAPI_CallUndoableCommand ("Element Test API Function",
		[&] () -> GSErrCode {

			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		Do_SolidLink_Create ();				break;
				case 2:		Do_SolidLink_Remove ();				break;
				case 3:		Do_SolidLink_Targets ();			break;
				case 4:		Do_SolidLink_Operators ();			break;

				default:										break;
			}

			return NoError;
		});
}		/* ElementsSolidOperation */


// -----------------------------------------------------------------------------
// Elements: Trim Operations Functions
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL ElementsTrim (const API_MenuParams *menuParams)
{
	return ACAPI_CallUndoableCommand ("Element Test API Function",
		[&] () -> GSErrCode {

			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		Do_Trim_Elements ();				break;
				case 2:		Do_Trim_ElementsWith ();			break;
				case 3:		Do_Trim_Remove ();					break;
				case 4:		Do_Trim_GetTrimType ();				break;
				case 5:		Do_Trim_GetTrimmedElements ();		break;
				case 6:		Do_Trim_GetTrimmingElements ();		break;
				case 7:		Do_Merge_Elements ();				break;
				case 8:		Do_Merge_GetMergedElements ();		break;
				case 9:		Do_Merge_Remove ();					break;

				default:										break;
			}

			return NoError;
		});
}		/* ElementsTrim */


// -----------------------------------------------------------------------------
// Elements: Hotlink Functions
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL ElementsHotlink (const API_MenuParams *menuParams)
{
	return ACAPI_CallUndoableCommand ("Element Test API Function",
		[&] () -> GSErrCode {

			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		Do_CreateHotlink ();				break;
				case 2:		Do_UpdateHotlink ();				break;
				case 3:		Do_DeleteHotlink ();				break;
				case 4:		Do_BreakHotlink ();					break;
				case 5:		Do_ListHotlinks ();					break;

				default:										break;
			}

			return NoError;
		});
}		/* ElementsSolidOperation */


// -----------------------------------------------------------------------------
// Elements: Snippets
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL ElementsSnippets (const API_MenuParams *menuParams)
{
	return ACAPI_CallUndoableCommand ("Element Test API Function",
		[&] () -> GSErrCode {

			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		Do_DeleteCamset (true);				break;
				case 2:		Do_DeleteCamset (false);			break;
				/* ----- */
				case 4:		Do_DumpZone ();						break;
				case 5:		Do_DumpWall ();						break;
				case 6:		Do_DumpBeam ();						break;
				case 7:		Do_DumpWindow ();					break;
				case 8:		Do_DumpDoor ();						break;
				case 9:		Do_DumpShell ();					break;
				/* ----- */
				case 11:	Do_NeigToCoord ();					break;
				case 12:	Do_GetBounds ();					break;
				case 13:	Do_ShowSymbolHotspots ();			break;
				/* ----- */
				case 15:	Do_CalcQuantities ();				break;
				case 16:	Do_GetComponents ();				break;
				/* ----- */
				case 18:	Do_SetToolBoxMode ();				break;
				case 19:	Do_ChangeSubtypeSettings ();		break;
				/* ----- */
				case 21:	Do_SplitPolyRoof ();				break;

				default:										break;
			}

			return NoError;
		});
}		/* ElementsSnippets */


// =============================================================================
//
// Required functions
//
// =============================================================================
#ifdef __APPLE__
#pragma mark -
#endif

// -----------------------------------------------------------------------------
// Dependency definitions
// -----------------------------------------------------------------------------
API_AddonType __ACENV_CALL	CheckEnvironment (API_EnvirParams* envir)
{
	if (envir->serverInfo.serverApplication != APIAppl_ArchiCADID)
		return APIAddon_DontRegister;

	ACAPI_Resource_GetLocStr (envir->addOnInfo.name, 32000, 1);
	ACAPI_Resource_GetLocStr (envir->addOnInfo.description, 32000, 2);

	return APIAddon_Preload;
}		/* RegisterAddOn */


// -----------------------------------------------------------------------------
// Interface definitions
// -----------------------------------------------------------------------------
GSErrCode __ACENV_CALL	RegisterInterface (void)
{
	GSErrCode	err;

	//
	// Register menus
	//
	err = ACAPI_Register_Menu (32500, 0, MenuCode_UserDef, MenuFlag_SeparatorBefore);
	err = ACAPI_Register_Menu (32501, 0, MenuCode_UserDef, MenuFlag_Default);
	err = ACAPI_Register_Menu (32502, 0, MenuCode_UserDef, MenuFlag_Default);
	err = ACAPI_Register_Menu (32503, 0, MenuCode_UserDef, MenuFlag_Default);
	err = ACAPI_Register_Menu (32504, 0, MenuCode_UserDef, MenuFlag_Default);
	err = ACAPI_Register_Menu (32505, 0, MenuCode_UserDef, MenuFlag_Default);
	err = ACAPI_Register_Menu (32506, 0, MenuCode_UserDef, MenuFlag_Default);
	err = ACAPI_Register_Menu (32507, 0, MenuCode_UserDef, MenuFlag_Default);
	err = ACAPI_Register_Menu (32508, 0, MenuCode_UserDef, MenuFlag_Default);
	err = ACAPI_Register_Menu (32509, 0, MenuCode_UserDef, MenuFlag_Default);
	err = ACAPI_Register_FileType (1, 'TEXT', '    ', "txt;", 0, 32510, 1, SaveAs2DSupported);
	err = ACAPI_Register_FileType (2, 'TEXT', '    ', "txt;", 0, 32510, 2, Import2DDrawingSupported);

	return err;
}		/* RegisterInterface */


// -----------------------------------------------------------------------------
// Called when the Add-On has been loaded into memory
// to perform an operation
// -----------------------------------------------------------------------------
GSErrCode __ACENV_CALL	Initialize (void)
{
	GSErrCode err = NoError;

	//
	// Install menu handler callbacks
	//
	err = ACAPI_Install_MenuHandler (32500, ElementsCmd);
	err = ACAPI_Install_MenuHandler (32501, ElementsEdit);
	err = ACAPI_Install_MenuHandler (32502, ElementsTools);
	err = ACAPI_Install_MenuHandler (32503, ElementsModify);
	err = ACAPI_Install_MenuHandler (32504, ElementsTrick);
	err = ACAPI_Install_MenuHandler (32505, ElementsGroup);
	err = ACAPI_Install_MenuHandler (32506, ElementsSolidOperation);
	err = ACAPI_Install_MenuHandler (32507, ElementsTrim);
	err = ACAPI_Install_MenuHandler (32508, ElementsHotlink);
	err = ACAPI_Install_MenuHandler (32509, ElementsSnippets);
	err = ACAPI_Install_FileTypeHandler (1, Do_ExportPrimitives);
	err = ACAPI_Install_FileTypeHandler (2, Do_Import);

	return err;
}		/* Initialize */


// -----------------------------------------------------------------------------
// Called when the Add-On is going to be unloaded
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	FreeData (void)
{
	return NoError;
}		/* FreeData */

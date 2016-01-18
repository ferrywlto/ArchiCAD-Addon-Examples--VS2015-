// *****************************************************************************
// Source code for the Element Test Add-On
// API Development Kit 19; Mac/Win
//
//	Code snippets
//
// Namespaces:        Contact person:
//     -None-
//
// [SG compatible] - Yes
// *****************************************************************************

#include "APIEnvir.h"
#define	_ELEMENT_SNIPPETS_TRANSL_


// ---------------------------------- Includes ---------------------------------

#include	<math.h>
#include	<stdio.h>
#include	<string.h>
#include	<time.h>

#include	"GSRoot.hpp"
#include	"ACAPinc.h"					// also includes APIdefs.h

#include	"File.hpp"
#include	"StableArray.hpp"
#include	"AutoPtr.hpp"
#include	"TextOProtocol.hpp"

#include	"Polygon2DData.h"
#include	"Polygon2D.hpp"
#include	"Polygon2DDataConv.h"

#include	"APICommon.h"
#include	"Element_Test.h"
#include	"VAArray.hpp"
#include	"Pair.hpp"


// ---------------------------------- Types ------------------------------------


// ---------------------------------- Variables --------------------------------

static const API_Element	*actZone = NULL;


// ---------------------------------- Prototypes -------------------------------



// =============================================================================
//
// Code snippets
//
// =============================================================================

// -----------------------------------------------------------------------------
// Dump information about connected elements
// -----------------------------------------------------------------------------

static void		DumpConnections (const char *info, Int32 nConn, API_ConnectionGuidItem **conns)
{
	char	s[1024];
	Int32	i;

	if (nConn > 0) {
		strcpy (s, "");
		for (i = 0; i < nConn; i++) {
			sprintf (s, "%s  %s", s, APIGuidToString ((*conns)[i].guid).ToCStr ().Get ());
		}
	} else
		sprintf (s, "---");

	WriteReport ("%s %s", info, s);

	return;
}		// DumpConnections


// -----------------------------------------------------------------------------
// Delete one element from the current database
// -----------------------------------------------------------------------------

static GSErrCode DeleteOneElement (const API_Guid& elemGuid)
{
	API_Elem_Head	**elemHead = (API_Elem_Head **) BMAllocateHandle (sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
	GSErrCode		err = BMError ();
	if (err == NoError) {
		(*elemHead)[0].guid = elemGuid;
		err = ACAPI_Element_Delete (elemHead, 1);
		BMKillHandle ((GSHandle *) &elemHead);
	}

	return err;
}


// -----------------------------------------------------------------------------
// Delete a camera set
// 	 - active:
//   	- true:  Delete the active camera set
//   	- false: Delete a camera set in the background (look for one with the same type as the active)
//   - check the camera settings after execution
//   - the last camera set cannot be deleted (ArchiCAD needs at least one)
// -----------------------------------------------------------------------------

void		Do_DeleteCamset (bool active)
{
	API_Guid	actGuid, perspGuid;
	SearchActiveCamset (&actGuid, &perspGuid);
	if (actGuid == APINULLGuid)
		return;

	const API_Guid	guid = (active ? actGuid : perspGuid);

	if (guid == APINULLGuid) {
		WriteReport ("No other camera set with the same type");
		return;
	}

	const GSErrCode	err = DeleteOneElement (guid);
	if (err != NoError)
		ErrorBeep ("ACAPI_Element_Delete (camera set)", err);
}		// Do_DeleteCamset


// -----------------------------------------------------------------------------
// Room reduction procedure callback
// -----------------------------------------------------------------------------

static void	__ACENV_CALL	RoomRedProc (const API_RoomReductionPolyType *roomRed)
{
	switch (roomRed->type) {
		case APIRoomReduction_Rest:		WriteReport ("  Rest");		break;
		case APIRoomReduction_Wall:		WriteReport ("  Wall");		break;
		case APIRoomReduction_Column:	WriteReport ("  Column");	break;
		case APIRoomReduction_Hatch:	WriteReport ("  Hatch");	break;
		case APIRoomReduction_Gable:	WriteReport ("  Gable");	break;
		default:						WriteReport ("  ????");		break;
	}

	DumpPolygon (actZone, 1, 0.0,
				 roomRed->nCoords, roomRed->nSubPolys, roomRed->nArcs,
				 roomRed->coords, roomRed->subPolys, roomRed->arcs,
				 false, true);

	BMKillHandle ((GSHandle*) &roomRed->coords);
	BMKillHandle ((GSHandle*) &roomRed->subPolys);
	BMKillHandle ((GSHandle*) &roomRed->arcs);
	return;
}		// RoomRedProc


// -----------------------------------------------------------------------------
// Dump information for the clicked zone
//   - important parameters
//   - the room polygon
//   - the applied gables (with their polygons)
//   - assigned construction elements
// Duplicate the zone polygon from lines and arc
// -----------------------------------------------------------------------------

void		Do_DumpZone (void)
{
	API_Element			element;
	API_ElementMemo		memo;
	API_RoomRelation	roomInfo;
	Int32				i, j, k;
	Int32				nPoly, nCoords, nSubPolys, nArcs, nGable;
	double				area;

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

	if (!ClickAnElem ("Click a zone", API_ZoneID, NULL, &element.header.typeID, &element.header.guid)) {
		WriteReport_Alert ("No zone was clicked");
		return;
	}

	GSErrCode err = ACAPI_Element_Get (&element);
	if (err == NoError)
		err = ACAPI_Element_GetMemo (element.header.guid, &memo);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Get/Memo (zone)", err);
		return;
	}

	WriteReport ("Zone: %s", APIGuidToString (element.header.guid).ToCStr ().Get ());

	// -----------------------
	WriteReport ("Polygon:");

	nPoly = 1;
	if (element.zone.manual)
		WriteReport ("  manual");
	else if (element.zone.refLineFlag) {
		WriteReport ("  auto (refLine)");
		nPoly = 2;
	} else
		WriteReport ("  auto");

	area = (*memo.params)[5].value.real;
	WriteReport ("  area: %lf", area);

	for (k = 1; k <= nPoly; k++) {
		if (k == 1) {
			nCoords   = element.zone.poly.nCoords;
			nSubPolys = element.zone.poly.nSubPolys;
			nArcs     = element.zone.poly.nArcs;
			DumpPolygon (&element, 1, 2.0,
					 nCoords, nSubPolys, nArcs,
					 memo.coords, memo.pends, memo.parcs,
					 true, true);
		} else {
			nCoords   = element.zone.refPoly.nCoords;
			nSubPolys = element.zone.refPoly.nSubPolys;
			nArcs     = element.zone.refPoly.nArcs;
			DumpPolygon (&element, 1, 2.0,
					 nCoords, nSubPolys, nArcs,
					 memo.additionalPolyCoords, memo.additionalPolyPends, memo.additionalPolyParcs,
					 true, true);
		}
	}

	// -----------------------
	WriteReport ("Gables:");
	if (memo.gables != NULL) {
		double x0 = (*memo.coords) [1].x;	// parameters to transform gable coordinates back to their absolute positions
		double y0 = (*memo.coords) [1].y;
		double vx = (*memo.coords) [2].x - x0;
		double vy = (*memo.coords) [2].y - y0;
		double len = vx * vx + vy * vy;
		if (len < EPS) {
			vx = 1.0;
			vy = 0.0;
		} else {
			len = sqrt (len);
			vx /= len;
			vy /= len;
		}

		nGable = BMGetHandleSize ((GSHandle) memo.gables) / sizeof (API_Gable);
		for (i = 0; i < nGable; i++) {
			WriteReport ("  [%d] (%f, %f, %f, %f)",
							 i,
							 (*memo.gables)[i].a, (*memo.gables)[i].b, (*memo.gables)[i].c,
							 (*memo.gables)[i].d);
			if ((*memo.gables)[i].coords != NULL) {
				WriteReport ("       Limited to polygon:", nGable);
				nCoords = BMGetHandleSize ((GSHandle) (*memo.gables)[i].coords) / sizeof (API_Coord) - 1;
				nSubPolys = BMGetHandleSize ((GSHandle)(*memo.gables)[i].pends) / sizeof (Int32) - 1;
				Int32 pendInd = 1;
				for (j = 1; j <= nCoords; j++) {
					if (pendInd <= nSubPolys && j == (*(*memo.gables)[i].pends)[pendInd] + 1) {
						WriteReport ("         Hole");
						pendInd++;
					}
					double xCoord = vx * ( (*(*memo.gables)[i].coords)[j].x) - vy * ( (*(*memo.gables)[i].coords[j]).y) + x0;
					double yCoord = vx * ( (*(*memo.gables)[i].coords)[j].y) + vy * ( (*(*memo.gables)[i].coords[j]).x) + y0;
					WriteReport ("         (%lf, %lf)", xCoord, yCoord);
				}
			}
		}
	}

	// -----------------------
	WriteReport ("Construction elements in the zone:");
	err = ACAPI_Element_GetRelations (element.header.guid, API_ZombieElemID, &roomInfo);
	if (err == NoError) {
		WriteReport ("  walls: %d", roomInfo.nWallPart);
		WriteReport ("  columns: %d", roomInfo.nColumn);
		WriteReport ("  objects: %d", roomInfo.nObject);
		WriteReport ("  lamps: %d", roomInfo.nLamp);
		WriteReport ("  windows: %d", roomInfo.nWindow);
		WriteReport ("  doors: %d", roomInfo.nDoor);
		WriteReport ("  niches: %d", roomInfo.nNiche);
		WriteReport ("  beams: %d", roomInfo.nBeamPart);
		WriteReport ("  curtainWall: %d", roomInfo.nCurtainWall);
		WriteReport ("  cwFrame: %d", roomInfo.nCwFrame);
		WriteReport ("  cwPanel: %d", roomInfo.nCwPanel);
		WriteReport ("  cwJunction: %d", roomInfo.nCwJunction);
		WriteReport ("  cwAccessory: %d", roomInfo.nCwAccessory);
		WriteReport ("  cwSegmentPart: %d", roomInfo.nCwSegmentPart);
		WriteReport ("  skylight: %d", roomInfo.nSkylight);
		WriteReport ("  roof: %d", roomInfo.nRoof);
		WriteReport ("  shell: %d", roomInfo.nShell);
		WriteReport ("  morph: %d", roomInfo.nMorph);
	}
	ACAPI_DisposeRoomRelationHdls (&roomInfo);

	// -----------------------
	WriteReport ("Reduction polygons");

	actZone = &element;
	err = ACAPI_Database (APIDb_RoomReductionsID, &element.header.guid, (void *) (GS::IntPtr) RoomRedProc);
	if (err != NoError) {
		ErrorBeep ("APIDb_RoomReductionsID", err);
		return;
	}

	// -----------------------
	ACAPI_DisposeElemMemoHdls (&memo);

	return;
}		// Do_DumpZone


// -----------------------------------------------------------------------------
// Dump information for the clicked wall
//   - the connection polygon
//   - connected elements
// -----------------------------------------------------------------------------

void		Do_DumpWall (void)
{
	API_Element			element;
	API_WallRelation	wallInfo;

	BNZeroMemory (&element, sizeof (API_Element));

	if (!ClickAnElem ("Click a wall", API_WallID, NULL, &element.header.typeID, &element.header.guid)) {
		WriteReport_Alert ("No wall was clicked");
		return;
	}

	GSErrCode err = ACAPI_Element_Get (&element);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Get (wall)", err);
		return;
	}

	err = ACAPI_Element_GetRelations (element.header.guid, API_ZombieElemID, &wallInfo);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetRelations (wall, all data)", err);
		return;
	}

	// -----------------------
	WriteReport ("Connection polygon:");

	DumpPolygon (&element, 1, 0.0,
				 wallInfo.connPoly.nCoords, wallInfo.connPoly.nSubPolys, wallInfo.connPoly.nArcs,
				 wallInfo.coords, wallInfo.pends, wallInfo.parcs,
				 false, true);

	// -----------------------
	WriteReport ("Connected walls:");

	DumpConnections ("  to BegPoint:", wallInfo.nConBeg, wallInfo.conBeg);
	DumpConnections ("  to EndPoint:", wallInfo.nConEnd, wallInfo.conEnd);
	DumpConnections ("  to Ref.Line:", wallInfo.nConRef, wallInfo.conRef);
	DumpConnections ("  on the ends:", wallInfo.nCon,	 wallInfo.con);
	DumpConnections ("  crossing   :", wallInfo.nConX,	 wallInfo.conX);

	ACAPI_DisposeWallRelationHdls (&wallInfo);
	return;
}		// Do_DumpWall


// -----------------------------------------------------------------------------
// Dump information for the clicked beam
//   - the connection polygon
//   - connected elements
// -----------------------------------------------------------------------------

void		Do_DumpBeam (void)
{
	API_Element			element;
	API_BeamRelation	beamInfo;

	BNZeroMemory (&element, sizeof (API_Element));

	if (!ClickAnElem ("Click a beam", API_BeamID, NULL, &element.header.typeID, &element.header.guid)) {
		WriteReport_Alert ("No beam was clicked");
		return;
	}

	GSErrCode err = ACAPI_Element_Get (&element);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Get (beam)", err);
		return;
	}

	err = ACAPI_Element_GetRelations (element.header.guid, API_ZombieElemID, &beamInfo);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetRelations (beam, all data)", err);
		return;
	}

	// -----------------------
	WriteReport ("Connection polygon:");

	DumpPolygon (&element, 1, 0.0,
				 beamInfo.connPoly.nCoords, beamInfo.connPoly.nSubPolys, beamInfo.connPoly.nArcs,
				 beamInfo.coords, beamInfo.pends, beamInfo.parcs,
				 true, true);

	// -----------------------
	WriteReport ("Connected beams:");

	DumpConnections ("  to BegPoint:", beamInfo.nConBeg, beamInfo.conBeg);
	DumpConnections ("  to EndPoint:", beamInfo.nConEnd, beamInfo.conEnd);
	DumpConnections ("  to Ref.Line:", beamInfo.nConRef, beamInfo.conRef);
	DumpConnections ("  on the ends:", beamInfo.nCon,	 beamInfo.con);
	DumpConnections ("  crossing   :", beamInfo.nConX,	 beamInfo.conX);

	ACAPI_DisposeBeamRelationHdls (&beamInfo);
	return;
}		// Do_DumpBeam


// -----------------------------------------------------------------------------
// Dump information for the clicked window
//   - connected elements (rooms, owner wall)
// -----------------------------------------------------------------------------

void		Do_DumpWindow (void)
{
	API_Element			element;
	API_WindowRelation	windowInfo;

	BNZeroMemory (&element, sizeof (API_Element));

	if (!ClickAnElem ("Click a window", API_WindowID, NULL, &element.header.typeID, &element.header.guid)) {
		WriteReport_Alert ("No window was clicked");
		return;
	}

	GSErrCode err = ACAPI_Element_Get (&element);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Get (window)", err);
		return;
	}

	err = ACAPI_Element_GetRelations (element.header.guid, API_ZombieElemID, &windowInfo);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetRelations (window, all data)", err);
		return;
	}

	// -----------------------
	WriteReport ("Owner wall: %s", APIGuidToString (element.window.owner).ToCStr ().Get ());

	// -----------------------
	WriteReport ("Connected rooms:");

	WriteReport ("  fromRoom: %s", APIGuidToString (windowInfo.fromRoom).ToCStr ().Get ());
	WriteReport ("  toRoom  : %s", APIGuidToString (windowInfo.toRoom).ToCStr ().Get ());

	return;
}		// Do_DumpWindow


// -----------------------------------------------------------------------------
// Dump information for the clicked door
//   - connected elements (rooms, owner wall)
// -----------------------------------------------------------------------------

void		Do_DumpDoor (void)
{
	API_Element			element;
	API_DoorRelation	doorInfo;

	BNZeroMemory (&element, sizeof (API_Element));

	if (!ClickAnElem ("Click a door", API_DoorID, NULL, &element.header.typeID, &element.header.guid)) {
		WriteReport_Alert ("No door was clicked");
		return;
	}

	GSErrCode err = ACAPI_Element_Get (&element);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Get (door)", err);
		return;
	}

	err = ACAPI_Element_GetRelations (element.header.guid, API_ZombieElemID, &doorInfo);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetRelations (door, all data)", err);
		return;
	}

	// -----------------------
	WriteReport ("Owner wall: %s", APIGuidToString (element.door.owner).ToCStr ().Get ());

	// -----------------------
	WriteReport ("Connected rooms:");

	WriteReport ("  fromRoom: %s", APIGuidToString (doorInfo.fromRoom).ToCStr ().Get ());
	WriteReport ("  toRoom  : %s", APIGuidToString (doorInfo.toRoom).ToCStr ().Get ());

	return;
}		// Do_DumpDoor


// -----------------------------------------------------------------------------
// Dump information for the clicked shell
// -----------------------------------------------------------------------------

void		Do_DumpShell (void)
{
	API_Element			element;
	API_ShellRelation	shellInfo;

	BNZeroMemory (&element, sizeof (API_Element));

	if (!ClickAnElem ("Click a shell", API_ShellID, NULL, &element.header.typeID, &element.header.guid)) {
		WriteReport_Alert ("No shell was clicked");
		return;
	}

	GSErrCode err = ACAPI_Element_Get (&element);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Get (shell)", err);
		return;
	}

	GS::Array<API_Guid> roomGuids;
	shellInfo.rooms = &roomGuids;

	WriteReport ("Shell: %s", APIGuidToString (element.header.guid).ToCStr ().Get ());

	// -----------------------
	WriteReport ("Construction elements in the shell:");
	err = ACAPI_Element_GetRelations (element.header.guid, API_ZombieElemID, &shellInfo);
	if (err == NoError) {
		ULong roomCnt = roomGuids.GetSize ();
		WriteReport ("  room: %d", roomCnt);
		for (ULong i = 0; i < roomCnt; i++) {
			WriteReport ("     room[%2d]: %s", i, APIGuidToString (roomGuids[i]).ToCStr ().Get ());
		}
	}

	return;
}		// Do_DumpShell


// -----------------------------------------------------------------------------
// Get the coordinate of a special point of the clicked element
//   - "OnNeig" will be refused
// -----------------------------------------------------------------------------

void		Do_NeigToCoord (void)
{
	API_ElemTypeID	typeID;
	API_Neig		theNeig;
	API_Coord3D		coord, c2;
	GSErrCode		err;

	if (!ClickAnElem ("Click a point neig", API_ZombieElemID, &theNeig, &typeID, NULL, &c2)) {
		WriteReport_Alert ("No element was clicked");
		return;
	}

	WriteReport ("%s #%s", ElemID_To_Name (typeID), APIGuidToString (theNeig.guid).ToCStr ().Get ());
	WriteReport ("  typ: %d", theNeig.neigID);
	WriteReport ("  inIndex: %d", theNeig.inIndex);

	err = ACAPI_Goodies (APIAny_NeigToCoordID, &theNeig, &coord);
	if (err == NoError) {
		WriteReport ("  coord: (%f, %f, %f)", coord.x, coord.y, coord.z);
		if (fabs (c2.x - coord.x) > EPS || fabs (c2.y - coord.y) > EPS)
			WriteReport_Alert ("APIAny_NeigToCoordID - bad calculation");
	} else
		ErrorBeep ("APIAny_NeigToCoordID", err);

	return;
}		// Do_NeigToCoord


// -----------------------------------------------------------------------------
// Get the bounding box of an element
//   - 3D element: create a wall to visualize the output
//   - 2D element: create a rectangle to visualize the output
// -----------------------------------------------------------------------------

void		Do_GetBounds (void)
{
	API_Element		element;
	API_Box3D		box;

	BNZeroMemory (&element, sizeof (API_Element));

	if (!ClickAnElem ("Click an elem", API_ZombieElemID, NULL, &element.header.typeID, &element.header.guid)) {
		WriteReport_Alert ("No element was clicked");
		return;
	}

	ACAPI_Element_GetHeader (&element.header);

	GSErrCode err = ACAPI_Database (APIDb_CalcBoundsID, &element, &box);
	if (err != NoError) {
		ErrorBeep ("APIDb_CalcBoundsID", err);
		return;
	}

	WriteReport ("%s GUID:%s", ElemID_To_Name (element.header.typeID), APIGuidToString (element.header.guid).ToCStr ().Get ());

	WriteReport ("  box: (%.2lf, %.2lf, %.2lf) - (%.2lf, %.2lf, %.2lf)",
				 box.xMin, box.yMin, box.zMin,
				 box.xMax, box.yMax, box.zMax);

	err = ACAPI_Element_GetDefaults (&element, NULL);
	if (err != NoError)
		return;


	API_Element		element2;
	API_Elem_Head	**elemHead = NULL;
	API_StoryInfo	storyInfo;

	BNZeroMemory (&element2, sizeof (API_Element));

	if (fabs (box.zMax - box.zMin) > EPS) {
		element2.header.typeID = API_WallID;
		err = ACAPI_Element_GetDefaults (&element2, NULL);
		if (err != NoError)
			return;
		//element2.wall.fillPen = 45;
		//element2.wall.fillBGPen = 77;

		err = ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo, NULL);
		if (err == NoError) {
			box.zMax -= (*(storyInfo.data))[element.header.floorInd].level;
			box.zMin -= (*(storyInfo.data))[element.header.floorInd].level;
			element2.header.floorInd = element.header.floorInd;
		}
		BMKillHandle ((GSHandle *) &storyInfo.data);
		if (err != NoError)
			return;

		element2.header.layer = 1;
		element2.wall.bottomOffset = box.zMin;
		element2.wall.height = box.zMax - box.zMin;
		element2.wall.thickness = box.yMax - box.yMin;
		element2.wall.flipped = false;
		element2.wall.referenceLineLocation = APIWallRefLine_Center;
		element2.wall.begC.x = box.xMin;
		element2.wall.begC.y = (box.yMin + box.yMax) / 2;
		element2.wall.endC.x = box.xMax;
		element2.wall.endC.y = (box.yMin + box.yMax) / 2;

		ACAPI_Element_Create (&element2, NULL);

		elemHead = (API_Elem_Head **) BMAllocateHandle (sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
		err = BMError ();
		if (err == NoError)
			**elemHead = element2.header;

		err = ACAPI_Element_Tool (elemHead, 1, APITool_SendBackward, NULL);
		if (err != NoError)
			ErrorBeep ("ACAPI_Element_Tool (send backward)", err);

		if (elemHead != NULL)
			BMKillHandle ((GSHandle *) &elemHead);

	} else {
		element2.header.typeID = API_LineID;
		element2.header.floorInd = element.header.floorInd;
		element2.header.layer = 1;
		element2.line.linePen = 1;
		element2.line.ltypeInd = 1;
		element2.line.begC.x = box.xMin;
		element2.line.begC.y = box.yMin;
		element2.line.endC.x = box.xMax;
		element2.line.endC.y = box.yMin;
		ACAPI_Element_Create (&element2, NULL);

		element2.line.begC.x = box.xMax;
		element2.line.begC.y = box.yMin;
		element2.line.endC.x = box.xMax;
		element2.line.endC.y = box.yMax;
		ACAPI_Element_Create (&element2, NULL);

		element2.line.begC.x = box.xMax;
		element2.line.begC.y = box.yMax;
		element2.line.endC.x = box.xMin;
		element2.line.endC.y = box.yMax;
		ACAPI_Element_Create (&element2, NULL);

		element2.line.begC.x = box.xMin;
		element2.line.begC.y = box.yMax;
		element2.line.endC.x = box.xMin;
		element2.line.endC.y = box.yMin;
		ACAPI_Element_Create (&element2, NULL);
	}

	return;
}		// Do_GetBounds


// -----------------------------------------------------------------------------
// Get the hotspots of the clicked element, and label them
//   - Works on Symbol, Light, Window, Door, Zone elements only (so far)
// -----------------------------------------------------------------------------

void		Do_ShowSymbolHotspots (void)
{
	API_Element	element;
	BNZeroMemory (&element, sizeof (API_Element));

	if (!ClickAnElem ("Click a point neig", API_ZombieElemID, NULL, &element.header.typeID, &element.header.guid)) {
		WriteReport_Alert ("No element was clicked");
		return;
	}

	GSErrCode err = ACAPI_Element_Get (&element);
	if (err != NoError)
		return;

	GS::Array<API_ElementHotspot>	hotspotArray;
	API_PrimHotspot**				hotspots = NULL;
	Int32		ind;
	Int32		nHotspots = 0;
	Int32		libInd = 0;
	API_Guid	instanceElemGuid = element.header.guid;
	bool		isLibPart = true;

	switch (element.header.typeID) {
		case API_WindowID:
		case API_DoorID:		libInd = element.window.openingBase.libInd;		break;
		case API_SkylightID:	libInd = element.skylight.openingBase.libInd;	break;
		case API_ObjectID:
		case API_LampID:		libInd = element.object.libInd;					break;
		case API_LabelID:		libInd = element.label.u.symbol.libInd;			break;
		case API_ZoneID:		libInd = element.zone.libInd;					break;

		default:				isLibPart = false;								break;		// in this case using ACAPI_Element_GetHotspots
	}

	if (isLibPart) {
		err = ACAPI_LibPart_GetHotspots (libInd, instanceElemGuid, &nHotspots, &hotspots);
		if (err == NoError && hotspots != NULL) {
			BNZeroMemory (&element, sizeof (API_Element));
			element.header.typeID = API_HotspotID;
			err = ACAPI_Element_GetDefaults (&element, NULL);

			if (err == NoError) {
				for (ind = 0; ind < nHotspots; ind++) {			// draw hotspots in local coordinates
					element.hotspot.pen = (short) ind;
					element.hotspot.pos = (*hotspots)[ind].loc;
					ACAPI_Element_Create (&element, NULL);
				}

				for (ind = 0; ind < nHotspots; ind++) {			// draw hotspots according to the neigs
					element.hotspot.pen = (short) ind;
					err = ACAPI_Goodies (APIAny_NeigToCoordID, &(*hotspots)[ind].neig, &element.hotspot.pos);
					if (err == NoError)
						ACAPI_Element_Create (&element, NULL);
				}
			}
		}
		BMKillHandle ((GSHandle*) &hotspots);
	}
	else {
		err = ACAPI_Element_GetHotspots (instanceElemGuid, &hotspotArray);
		if (err == NoError && !hotspotArray.IsEmpty ()) {
			BNZeroMemory (&element, sizeof (API_Element));
			element.header.typeID = API_HotspotID;
			err = ACAPI_Element_GetDefaults (&element, NULL);
			if (err != NoError)
				return;

			for (ind = 0; ind < (Int32) hotspotArray.GetSize (); ind++) {
				API_Neig	neig;
				API_Coord3D	coord;
				hotspotArray[ind].Get (neig, coord);
				element.hotspot.pen = (short) ind;
				element.hotspot.pos.x = coord.x;
				element.hotspot.pos.y = coord.y;
				ACAPI_Element_Create (&element, NULL);
			}
		}
	}

	return;
}		// Do_ShowSymbolHotspots


// -----------------------------------------------------------------------------
// Get quantities of the clicked element
// -----------------------------------------------------------------------------

void		Do_CalcQuantities (void)
{
	API_ElemTypeID		typeID;
	API_Guid			guid;
	API_ElementQuantity	quantity;
	API_QuantityPar		params;
	GSErrCode			err;

	if (!ClickAnElem ("Click an element to calculate quantities", API_ZombieElemID, NULL, &typeID, &guid)) {
		WriteReport_Alert ("No element was clicked");
		return;
	}

	API_Element element;
	BNZeroMemory (&element, sizeof element);
	element.header.typeID = typeID;
	element.header.guid   = guid;

	err = ACAPI_Element_Get (&element);
	if (err != NoError) {
		WriteReport_Alert ("Unknown element");
		return;
	}

	BNZeroMemory (&params, sizeof (API_QuantityPar));
	params.minOpeningSize = EPS;

	GS::Array <API_CompositeQuantity>			composites;
	GS::Array <API_ElemPartQuantity>			elemPartQuantities;
	GS::Array <API_ElemPartCompositeQuantity>	elemPartComposites;

	GS::Array<API_Quantities>	quantities;
	GS::Array<API_Guid>			elemGuids;

	quantities.Push (API_Quantities ());
	quantities[0].elements = &quantity;
	quantities[0].composites = &composites;
	quantities[0].elemPartQuantities = &elemPartQuantities;
	quantities[0].elemPartComposites = &elemPartComposites;
	quantities.Push (quantities[0]);
	quantities.Push (quantities[0]);
	elemGuids.Push (element.header.guid);
	elemGuids.Push (element.header.guid);
	elemGuids.Push (element.header.guid);

	err = ACAPI_Element_GetMoreQuantities (&elemGuids, &params, &quantities, NULL);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetQuantities", err);
		return;
	}

	WriteReport ("%s GUID:%s", ElemID_To_Name (typeID), APIGuidToString (element.header.guid).ToCStr ().Get ());

	switch (typeID) {
		case API_WallID:
					WriteReport ("  volume:   %lf", quantity.wall.volume);
					WriteReport ("  surface1: %lf", quantity.wall.surface1);
					WriteReport ("  surface2: %lf", quantity.wall.surface2);
					WriteReport ("  surface3: %lf", quantity.wall.surface3);
					WriteReport ("  length:   %lf", quantity.wall.length);
					WriteReport ("  windowsSurf:    %lf", quantity.wall.windowsSurf);
					WriteReport ("  doorsSurf:      %lf", quantity.wall.doorsSurf);
					WriteReport ("  emptyholesSurf: %lf", quantity.wall.emptyholesSurf);
					WriteReport ("  columnsVolume:  %lf", quantity.wall.columnsVolume);
					WriteReport ("  windowsWidth:   %d",  quantity.wall.windowsWidth);
					WriteReport ("  doorsWidth:     %lf", quantity.wall.doorsWidth);
					for (UInt32 i = 0; i < composites.GetSize (); i++) {
						API_Attribute	attr;
						GS::UniString	cFlags;
						GSErrCode		err;

						BNZeroMemory (&attr, sizeof(API_Attribute));
						attr.header.typeID = API_BuildingMaterialID;
						attr.header.index = composites[i].buildMatIndices;

						err = ACAPI_Attribute_Get (&attr);
						if (err == NoError) {
							if ((composites[i].flags & APISkin_Core) != 0) {
								cFlags.Assign ("Core");
							} if ((composites[i].flags & APISkin_Finish) != 0) {
								if (!cFlags.IsEmpty ())
									cFlags.Append ("+");
								cFlags.Append ("Finish");
							}
						}

						WriteReport ("  	composite:   %s[%d]		- volume: %lf		- projected area: %lf	- flag: %s", attr.header.name, composites[i].buildMatIndices, composites[i].volumes, composites[i].projectedArea, static_cast<const char*> (cFlags.ToCStr ()));
					}
 					break;
		case API_ColumnID:
					WriteReport ("  coreSurface: %lf", quantity.column.coreSurface);
					WriteReport ("  veneSurface: %lf", quantity.column.veneSurface);
					WriteReport ("  coreVolume:  %lf", quantity.column.coreVolume);
					WriteReport ("  veneVolume:  %lf", quantity.column.veneVolume);
					break;
		case API_BeamID:
					WriteReport ("  rightLength:      %lf", quantity.beam.rightLength);
					WriteReport ("  leftLength:       %lf", quantity.beam.leftLength);
					WriteReport ("  bottomSurface:    %lf", quantity.beam.bottomSurface);
					WriteReport ("  topSurface:       %lf", quantity.beam.topSurface);
					WriteReport ("  edgeSurfaceLeft:  %lf", quantity.beam.edgeSurfaceLeft);
					WriteReport ("  edgeSurfaceRight: %lf", quantity.beam.edgeSurfaceRight);
					WriteReport ("  edgeSurface:      %lf", quantity.beam.edgeSurface);
					WriteReport ("  holesSurface:     %lf", quantity.beam.holesSurface);
					WriteReport ("  holesEdgeSurface: %lf", quantity.beam.holesEdgeSurface);
					WriteReport ("  holesNumber:      %d",  quantity.beam.holesNumber);
					WriteReport ("  volume:           %lf", quantity.beam.volume);
					WriteReport ("  holesVolume:      %lf", quantity.beam.holesVolume);
					break;
		case API_DoorID:
		case API_WindowID:
		case API_SkylightID:
					WriteReport ("  surface:  %lf", quantity.window.surface);
					WriteReport ("  volume:   %lf", quantity.window.volume);
					break;
		case API_ObjectID:
		case API_LampID:
					WriteReport ("  surface:  %lf", quantity.symb.surface);
					WriteReport ("  volume:   %lf", quantity.symb.volume);
					break;
		case API_SlabID:
					WriteReport ("  bottomSurface: %lf", quantity.slab.bottomSurface);
					WriteReport ("  topSurface:    %lf", quantity.slab.topSurface);
					WriteReport ("  edgeSurface:   %lf", quantity.slab.edgeSurface);
					WriteReport ("  volume:        %lf", quantity.slab.volume);
					WriteReport ("  perimeter:     %lf", quantity.slab.perimeter);
					WriteReport ("  holesSurf:     %lf", quantity.slab.holesSurf);
					WriteReport ("  holesPrm:      %lf", quantity.slab.holesPrm);
					for (UInt32 i = 0; i < composites.GetSize (); i++) {
						WriteReport ("  	composite:   %d		- volume: %lf		- projected area: %lf", composites[i].buildMatIndices, composites[i].volumes, composites[i].projectedArea);
					}
					break;
		case API_MeshID:
					WriteReport ("  bottomSurface: %lf", quantity.mesh.bottomSurface);
					WriteReport ("  topSurface:    %lf", quantity.mesh.topSurface);
					WriteReport ("  edgeSurface:   %lf", quantity.mesh.edgeSurface);
					WriteReport ("  volume:        %lf", quantity.mesh.volume);
					WriteReport ("  perimeter:     %lf", quantity.mesh.perimeter);
					WriteReport ("  holesSurf:     %lf", quantity.mesh.holesSurf);
					WriteReport ("  holesPrm:      %lf", quantity.mesh.holesPrm);
					break;
		case API_RoofID:
					WriteReport ("  bottomSurface: %lf", quantity.roof.bottomSurface);
					WriteReport ("  topSurface:    %lf", quantity.roof.topSurface);
					WriteReport ("  edgeSurface:   %lf", quantity.roof.edgeSurface);
					WriteReport ("  volume:        %lf", quantity.roof.volume);
					WriteReport ("  perimeter:     %lf", quantity.roof.perimeter);
					WriteReport ("  holesSurf:     %lf", quantity.roof.holesSurf);
					WriteReport ("  holesPrm:      %lf", quantity.roof.holesPrm);
					for (UInt32 i = 0; i < composites.GetSize (); i++) {
						WriteReport ("  	composite:   %d		- volume: %lf		- projected area: %lf", composites[i].buildMatIndices, composites[i].volumes, composites[i].projectedArea);
					}
					WriteReport ("  elemPartQuantities:      %d", elemPartQuantities.GetSize ());
					for (UInt32 i = 0; i < elemPartQuantities.GetSize (); i++) {
						WriteReport ("  	[%d,%d]		- volume: %lf", elemPartQuantities[i].partId.main, elemPartQuantities[i].partId.sub, elemPartQuantities[i].quantity.roof.volume);
					}
					WriteReport ("  elemPartComposites:      %d", elemPartComposites.GetSize ());
					for (UInt32 i = 0; i < elemPartComposites.GetSize (); i++) {
						WriteReport ("  	[%d,%d]", elemPartComposites[i].partId.main, elemPartComposites[i].partId.sub);
						for (UInt32 j = 0; j < elemPartComposites[i].composites.GetSize (); j++) {
							WriteReport ("  		composite:   %d		- volume: %lf		- projected area: %lf", elemPartComposites[i].composites[j].buildMatIndices, elemPartComposites[i].composites[j].volumes, elemPartComposites[i].composites[j].projectedArea);
						}
					}
					break;
		case API_ShellID:
					WriteReport ("  referenceSurface: %lf", quantity.shell.referenceSurface);
					WriteReport ("  oppositeSurface:    %lf", quantity.shell.oppositeSurface);
					WriteReport ("  edgeSurface:   %lf", quantity.shell.edgeSurface);
					WriteReport ("  volume:        %lf", quantity.shell.volume);
					WriteReport ("  perimeter:     %lf", quantity.shell.perimeter);
					WriteReport ("  holesSurf:     %lf", quantity.shell.holesSurf);
					WriteReport ("  holesPrm:      %lf", quantity.shell.holesPrm);
					for (UInt32 i = 0; i < composites.GetSize (); i++) {
						WriteReport ("  	composite:   %d		- volume: %lf		- projected area: %lf", composites[i].buildMatIndices, composites[i].volumes, composites[i].projectedArea);
					}
					break;
		case API_MorphID:
					WriteReport ("  surface:  %lf", quantity.morph.surface);
					WriteReport ("  volume:   %lf", quantity.morph.volume);
					WriteReport ("  elemPartQuantities:      %d", elemPartQuantities.GetSize ());
					for (UInt32 i = 0; i < elemPartQuantities.GetSize (); i++) {
						WriteReport ("  	floor[%d]		- volume: %lf, floorPlanArea: %lf, baseLevel: %lf, baseHeight: %lf, wholeHeight: %lf",
									elemPartQuantities[i].partId.floor,
									elemPartQuantities[i].quantity.morph.volume,
									elemPartQuantities[i].quantity.morph.floorPlanArea,
									elemPartQuantities[i].quantity.morph.baseLevel,
									elemPartQuantities[i].quantity.morph.baseHeight,
									elemPartQuantities[i].quantity.morph.wholeHeight);
					}
					break;
		case API_HatchID:
					WriteReport ("  surface:    %lf", quantity.hatch.surface);
					WriteReport ("  perimeter:  %lf", quantity.hatch.perimeter);
					WriteReport ("  holesPrm:   %lf", quantity.hatch.holesPrm);
					WriteReport ("  holesSurf:  %lf", quantity.hatch.holesSurf);
					break;
		case API_ZoneID:
					WriteReport ("  area:           %lf", quantity.zone.area);
					WriteReport ("  perimeter:      %lf", quantity.zone.perimeter);
					WriteReport ("  holesPrm:       %lf", quantity.zone.holesPrm);
					WriteReport ("  wallsPrm:       %lf", quantity.zone.wallsPrm);
					WriteReport ("  allCorners:     %d",  quantity.zone.allCorners);
					WriteReport ("  concaveCorners: %d",  quantity.zone.concaveCorners);
					WriteReport ("  wallsSurf:      %lf", quantity.zone.wallsSurf);
					WriteReport ("  doorsWidth:     %lf", quantity.zone.doorsWidth);
					WriteReport ("  doorsSurf:      %lf", quantity.zone.doorsSurf);
					WriteReport ("  windowsWidth:   %lf", quantity.zone.windowsWidth);
					WriteReport ("  windowsSurf:    %lf", quantity.zone.windowsSurf);
					WriteReport ("  baseLevel:      %lf", quantity.zone.baseLevel);
					WriteReport ("  floorThick:     %lf", quantity.zone.floorThick);
					WriteReport ("  height:         %lf", quantity.zone.height);
					WriteReport ("  netarea:        %lf", quantity.zone.netarea);
					WriteReport ("  netperimeter:   %lf", quantity.zone.netperimeter);
					break;
		default:
					WriteReport ("  Please click a 3D element", true);
					break;
	}
	
	API_Neig				**neigs;
	GS::Array<API_Guid>		coverElemGuids;
	API_SelectionInfo		selectionInfo;
	err = ACAPI_Selection_Get (&selectionInfo, &neigs, false);
	UInt32 nSel = BMGetHandleSize ((GSHandle) neigs) / sizeof (API_Neig);
	for (UInt32 i = 0; i < nSel; i++) {
		coverElemGuids.Push ((*neigs)[i].guid);
	}

	BMKillHandle (reinterpret_cast<GSHandle *> (&neigs));

	GS::Array <API_ElemPartSurfaceQuantity>		elemPartSurfaces;
	elemGuids.Pop ();	// We have the same item 3 times. Have no idea why.
	elemGuids.Pop ();
	err = ACAPI_Element_GetSurfaceQuantities (&elemGuids, &coverElemGuids, &elemPartSurfaces);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetSurfaceQuantities", err);
		return;
	} 

	WriteReport ("---------- Exposed Surfaces ----------");
	WriteReport ("%s GUID : %s", ElemID_To_Name (typeID), APIGuidToString (element.header.guid).ToCStr ().Get ());
	for (UInt32 i = 0; i < elemPartSurfaces.GetSize (); i++) {
		GS::UniString surfaceQuantityString;
		surfaceQuantityString += "Surface " + GS::ValueToUniString (i) + GS::UniString (GS::EOL);
		surfaceQuantityString += "  GUID           : " + APIGuidToString (elemPartSurfaces[i].elemGUID) + GS::UniString (GS::EOL);
		surfaceQuantityString += "  ComponentIndex : " + GS::ValueToUniString (elemPartSurfaces[i].componentIndex) + GS::UniString (GS::EOL);
		surfaceQuantityString += "  MaterialIndex  : " + GS::ValueToUniString (elemPartSurfaces[i].materialIndex) + GS::UniString (GS::EOL);
		surfaceQuantityString += "  BuildMatIndex  : " + GS::ValueToUniString (elemPartSurfaces[i].buildMatIdx) + GS::UniString (GS::EOL);
		surfaceQuantityString += "  ExposedSurface : " + GS::ValueToUniString (elemPartSurfaces[i].exposedSurface) + GS::UniString (GS::EOL);
		WriteReport ("%s", surfaceQuantityString.ToCStr ().Get ());	
	}
	WriteReport ("---------- Exposed Surfaces ----------");

	return;
}		// Do_CalcQuantities


// -----------------------------------------------------------------------------
// Get the components of the clicked element
// -----------------------------------------------------------------------------

void		Do_GetComponents (void)
{
	API_Elem_Head			elemHead;
	API_ComponentRefType	**compRefs;
	Int32					nComp, i;
	GSErrCode				err;

	BNZeroMemory (&elemHead, sizeof (API_Elem_Head));

	if (!ClickAnElem ("Click an element to get its components", API_ZombieElemID, NULL, &elemHead.typeID, &elemHead.guid)) {
		WriteReport_Alert ("No element was clicked");
		return;
	}

	ACAPI_Element_GetHeader (&elemHead);

	err = ACAPI_Element_GetComponents (&elemHead, &compRefs, &nComp);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetComponents", err);
		return;
	}

	WriteReport ("%s GUID:%s", ElemID_To_Name (elemHead.typeID), APIGuidToString (elemHead.guid).ToCStr ().Get ());

	for (i = 0; i < nComp; i++) {
		WriteReport ("status : %d, quantity: %.2f", (*compRefs)[i].status, (*compRefs)[i].quantity);
	}

	BMKillHandle ((GSHandle *) &compRefs);

	return;
}		// Do_GetComponents


// -----------------------------------------------------------------------------
// Get and set the active toolbox item
//   - Determine the active toolbox item
//   - Change the active item, to the element clicked
//   - Try ESYM based tools also
// -----------------------------------------------------------------------------

void		Do_SetToolBoxMode (void)
{
	API_Guid				guid;
	API_Element				element;
	API_ToolBoxItem			tboxInfo;
	char					*p;
	GSErrCode				err;

	BNZeroMemory (&tboxInfo, sizeof (API_ToolBoxItem));
	tboxInfo.typeID  = API_ZombieElemID;
	tboxInfo.variationID = APIVarId_Generic;

	err = ACAPI_Environment (APIEnv_GetToolBoxModeID, &tboxInfo, NULL);
	if (err != NoError) {
		ErrorBeep ("APIEnv_GetToolBoxMode", err);
		return;
	}

	if (tboxInfo.typeID == API_ZombieElemID)
		WriteReport ("Previous toolbox mode: Arrow or Marquee");
	else {
		if (tboxInfo.variationID == APIVarId_Generic) {
			WriteReport ("Previous toolbox mode: %s", ElemID_To_Name (tboxInfo.typeID));
		} else {
			p = (char *) &tboxInfo.variationID;
			WriteReport ("Previous toolbox mode: %s  External \"%c%c%c%c\"", ElemID_To_Name (tboxInfo.typeID), p[0], p[1], p[2], p[3]);
		}
	}

	tboxInfo.typeID = API_ZombieElemID;
	tboxInfo.variationID = APIVarId_Generic;
	if (ClickAnElem ("Click an element to set toolbox mode", API_ZombieElemID, NULL, &tboxInfo.typeID, &guid)) {
		if (tboxInfo.typeID == API_ObjectID || tboxInfo.typeID == API_LampID ||
			tboxInfo.typeID == API_WindowID || tboxInfo.typeID == API_DoorID)
		{
			BNZeroMemory (&element, sizeof (API_Element));
			element.header.typeID = tboxInfo.typeID;
			element.header.guid   = guid;
			err = ACAPI_Element_Get (&element);
			if (err == NoError)
				tboxInfo.variationID = element.header.variationID;
		}
	}

	err = ACAPI_Environment (APIEnv_SetToolBoxModeID, &tboxInfo, NULL);
	if (err != NoError) {
		ErrorBeep ("APIEnv_SetToolBoxMode", err);
		return;
	}

	if (tboxInfo.typeID == API_ZombieElemID)
		WriteReport ("Current toolbox mode: Arrow");
	else {
		if (tboxInfo.variationID == APIVarId_Generic) {
			WriteReport ("Current toolbox mode: %s", ElemID_To_Name (tboxInfo.typeID));
		} else {
			p = (char *) &tboxInfo.variationID;
			WriteReport ("Current toolbox mode: %s  External \"%c%c%c%c\"", ElemID_To_Name (tboxInfo.typeID), p[0], p[1], p[2], p[3]);
		}
	}

	return;
}		// Do_SetToolBoxMode


// -----------------------------------------------------------------------------
// Access and modify subtype tool settings
//   - go through the installed 3rd party tools
//   - write a list of their default settings (report layers)
//   - change their default settings (layer to "ESYM layer")
// -----------------------------------------------------------------------------

void		Do_ChangeSubtypeSettings (void)
{
	API_ToolBoxInfo			toolboxInfo;
	Int32					i;
	API_Element				element, mask;
	API_Attribute			attribute;
	char					signature[5];
	GSErrCode				err;

	// create layer
	BNZeroMemory (&attribute, sizeof (API_Attribute));
	attribute.header.typeID = API_LayerID;
	strcpy (attribute.header.name, "ESYM layer");
	err = ACAPI_Attribute_Search (&attribute.header);
	if (err == APIERR_BADNAME)
		err = ACAPI_Attribute_Create (&attribute, NULL);
	if (err != NoError) {
		ErrorBeep ("Error accessing layer \"ESYM layer\"", err);
		return;
	}
	const short attributeIndex = attribute.header.index;

	err = ACAPI_Environment (APIEnv_GetToolBoxInfoID, &toolboxInfo, NULL);
	if (err != NoError) {
		ErrorBeep ("APIEnv_GetToolBoxInfo", err);
		return;
	}

	// scan the tools
	WriteReport ("subtype tools:");
	for (i = 0; i < toolboxInfo.nTools; i++) {
		if ((*toolboxInfo.data)[i].variationID == APIVarId_Generic)		// filter to subtype tools
			continue;

		// report the settings
		BNZeroMemory (&element, sizeof (API_Element));
		switch ((*toolboxInfo.data)[i].typeID) {
			case API_ObjectID:
						element.header.typeID = API_ObjectID;
						element.header.variationID = (*toolboxInfo.data)[i].variationID;
						break;
			case API_LampID:
						element.header.typeID = API_LampID;
						element.header.variationID = (*toolboxInfo.data)[i].variationID;
						break;
			case API_WindowID:
						element.header.typeID = API_WindowID;
						element.header.variationID = (*toolboxInfo.data)[i].variationID;
						break;
			case API_DoorID:
						element.header.typeID = API_DoorID;
						element.header.variationID = (*toolboxInfo.data)[i].variationID;
						break;
			default:
						continue;
						break;
		}
		err = ACAPI_Element_GetDefaults (&element, NULL);
		if (err != NoError) {
			ErrorBeep ("ACAPI_Element_GetDefaults", err);
			return;
		}

		BNZeroMemory (&attribute, sizeof (API_Attribute));
		if (element.header.layer != 0) {
			attribute.header.typeID = API_LayerID;
			attribute.header.index  = element.header.layer;
			err = ACAPI_Attribute_Get (&attribute);
			if (err != NoError) {
				ErrorBeep ("ACAPI_Attribute_Get", err);
				return;
			}
		}

		signature[4] = '\0';
		*((Int32 *) signature) = (*toolboxInfo.data)[i].variationID;
		WriteReport ("[%d] \'%s\'   \"%s\"", i+1, signature, attribute.header.name);

		// change the settings
		ACAPI_ELEMENT_MASK_CLEAR (mask);
		ACAPI_ELEMENT_MASK_SET (mask, API_Elem_Head, layer);

		element.header.layer = attributeIndex;

		err = ACAPI_Element_ChangeDefaults (&element, NULL, &mask);
		if (err != NoError)
			ErrorBeep ("ACAPI_Element_SetDefaults", err);
	}

	BMKillHandle ((GSHandle *) &toolboxInfo.data);

	return;
}		// Do_ChangeSubtypeSettings


// -----------------------------------------------------------------------------
// Converts a Multi-plane Roof into Single-plane roofs, insetting their polygon
//   - skylight holes preserved
// -----------------------------------------------------------------------------

static GSErrCode	ConstructPoly2DDataFromElementMemo (const API_ElementMemo& memo, Geometry::Polygon2DData& polygon2DData)
{
	GSErrCode err = NoError;

	Geometry::InitPolygon2DData (&polygon2DData);

	static_assert (sizeof (API_Coord) == sizeof (Coord), "sizeof (API_Coord) != sizeof (Coord)");
	static_assert (sizeof (API_PolyArc) == sizeof (PolyArcRec), "sizeof (API_PolyArc) != sizeof (PolyArcRec)");

	polygon2DData.nVertices = BMGetHandleSize (reinterpret_cast<GSHandle> (memo.coords)) / sizeof (Coord) - 1;
	polygon2DData.vertices = reinterpret_cast<Coord**> (BMAllocateHandle ((polygon2DData.nVertices + 1) * sizeof (Coord), ALLOCATE_CLEAR, 0));
	if (polygon2DData.vertices != NULL)
		BNCopyMemory (*polygon2DData.vertices, *memo.coords, (polygon2DData.nVertices + 1) * sizeof (Coord));
	else
		err = APIERR_MEMFULL;

	if (err == NoError && memo.parcs != NULL) {
		polygon2DData.nArcs = BMGetHandleSize (reinterpret_cast<GSHandle> (memo.parcs)) / sizeof (PolyArcRec);
		if (polygon2DData.nArcs > 0) {
			polygon2DData.arcs = reinterpret_cast<PolyArcRec**> (BMAllocateHandle ((polygon2DData.nArcs + 1) * sizeof (PolyArcRec), ALLOCATE_CLEAR, 0));
			if (polygon2DData.arcs != NULL)
				BNCopyMemory (*polygon2DData.arcs + 1, *memo.parcs, polygon2DData.nArcs * sizeof (PolyArcRec));
			else
				err = APIERR_MEMFULL;
		}
	}

	if (err == NoError) {
		polygon2DData.nContours = BMGetHandleSize (reinterpret_cast<GSHandle> (memo.pends)) / sizeof (Int32) - 1;
		polygon2DData.contourEnds = reinterpret_cast<UIndex**> (BMAllocateHandle ((polygon2DData.nContours + 1) * sizeof (UIndex), ALLOCATE_CLEAR, 0));
		if (polygon2DData.contourEnds != NULL)
			BNCopyMemory (*polygon2DData.contourEnds, *memo.pends, (polygon2DData.nContours + 1) * sizeof (UIndex));
		else
			err = APIERR_MEMFULL;
	}

	if (err == NoError) {
		Geometry::GetPolygon2DDataBoundBox (polygon2DData, &polygon2DData.boundBox);
		polygon2DData.status.isBoundBoxValid = true;
	} else {
		Geometry::FreePolygon2DData (&polygon2DData);
	}

	return err;
}		// ConstructPoly2DDataFromElementMemo


static GSErrCode	ConvertPoly2DDataToAPIPolygon (const Geometry::Polygon2DData& polygon2DData, API_Polygon& poly, API_ElementMemo& memo)
{
	GSErrCode err = NoError;

	poly.nCoords	= polygon2DData.nVertices;
	poly.nSubPolys	= polygon2DData.nContours;
	poly.nArcs		= polygon2DData.nArcs;

	memo.coords	= reinterpret_cast<API_Coord**>	(BMAllocateHandle ((poly.nCoords + 1) * sizeof (API_Coord), ALLOCATE_CLEAR, 0));
	memo.pends	= reinterpret_cast<Int32**>		(BMAllocateHandle ((poly.nSubPolys + 1) * sizeof (Int32), ALLOCATE_CLEAR, 0));
	if (memo.coords != NULL && memo.pends != NULL) {
		static_assert (sizeof (API_Coord) == sizeof (Coord), "sizeof (API_Coord) != sizeof (Coord)");
		BNCopyMemory (*memo.coords, *polygon2DData.vertices, (poly.nCoords + 1) * sizeof (API_Coord));
		BNCopyMemory (*memo.pends, *polygon2DData.contourEnds, (poly.nSubPolys + 1) * sizeof (Int32));
	} else {
		err = APIERR_MEMFULL;
	}

	if (err == NoError && polygon2DData.arcs != NULL) {
		memo.parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle (poly.nArcs * sizeof (API_PolyArc), ALLOCATE_CLEAR, 0));
		if (memo.parcs != NULL) {
			static_assert (sizeof (API_PolyArc) == sizeof (PolyArcRec), "sizeof (API_PolyArc) != sizeof (PolyArcRec)");
			BNCopyMemory (*memo.parcs, *polygon2DData.arcs, poly.nArcs * sizeof (API_PolyArc));
		} else {
			err = APIERR_MEMFULL;
		}
	}

	return err;
}		// ConvertPoly2DDataToAPIPolygon


static GSErrCode	ShrinkPolygon (Geometry::Polygon2DData& polygon2DData, double offsetValue)
{
	GS::VAArray<Geometry::Polygon2DData> offsPolys;
	GSErrCode err = Geometry::ResizePolygon2DDataContour (polygon2DData, 1, -offsetValue, &offsPolys);

	if (err == NoError && offsPolys.GetItemCount () == 1)
		Geometry::CopyPolygon2DData (&polygon2DData, offsPolys[1]);

	for (UIndex iResPoly = 1; iResPoly <= offsPolys.GetItemCount (); iResPoly++)
		Geometry::FreePolygon2DData (&offsPolys[iResPoly]);

	return err;
}		// ShrinkPolygon


static GSErrCode	AddSkylightHolesToRoofPolygon (Geometry::Polygon2DData& polygon2DData, const GS::Array<API_Guid>& skylights)
{
	Geometry::MultiPolygon2D multi;
	Geometry::ConvertPolygon2DDataToPolygon2D (multi, polygon2DData);
	Geometry::Polygon2D polygon2D = multi.PopLargest ();

	for (UIndex i = 0; i < skylights.GetSize (); i++) {
		Geometry::Polygon2D holePoly;
		if (ACAPI_Goodies (APIAny_GetSkylightHolePolygonID, (void*) &skylights[i], &holePoly) == NoError) {
			Geometry::MultiPolygon2D resultPolys;
			polygon2D.Substract (holePoly, resultPolys);
			if (!resultPolys.IsEmpty ()) {
				polygon2D = resultPolys.PopLargest ();
			}
		}
	}

	Geometry::FreePolygon2DData (&polygon2DData);
	Geometry::ConvertPolygon2DToPolygon2DData (polygon2DData, polygon2D);

	return NoError;
}		// AddSkylightHolesToRoofPolygon


class PolyRoofConverter : public API_ElementDecomposerInterface {
private:
	GS::Array<API_Guid>	skylights;

public:
	PolyRoofConverter (const API_Guid& polyRoofGuid)
	{
		ACAPI_Element_GetConnectedElements (polyRoofGuid, API_SkylightID, &skylights);
	}

	virtual GSErrCode	GeneratePartElement (const API_Guid& /*guid*/, const API_ElemPartId& /*partId*/,
											 const API_Element& partElem, const API_ElementMemo& partMemo) override;
};

GSErrCode	PolyRoofConverter::GeneratePartElement (const API_Guid& /*guid*/, const API_ElemPartId& /*partId*/,
													const API_Element& partElem, const API_ElementMemo& partMemo)
{
	Geometry::Polygon2DData polygon2DData;
	GSErrCode err = ConstructPoly2DDataFromElementMemo (partMemo, polygon2DData);
	if (err == NoError) {
		ShrinkPolygon (polygon2DData, 0.25);
		AddSkylightHolesToRoofPolygon (polygon2DData, skylights);

		API_Element		roofElem = partElem;
		API_ElementMemo	roofMemo;
		BNZeroMemory (&roofMemo, sizeof (API_ElementMemo));
		err = ConvertPoly2DDataToAPIPolygon (polygon2DData, roofElem.roof.u.planeRoof.poly, roofMemo);
		if (err == NoError) {
			//roofElem.roof.shellBase.sectFill = 1;
			roofElem.roof.shellBase.level += 0.5;
			roofElem.roof.shellBase.thickness = 0.01;
			err = ACAPI_Element_Create (&roofElem, &roofMemo);
		}
		ACAPI_DisposeElemMemoHdls (&roofMemo);
	}

	Geometry::FreePolygon2DData (&polygon2DData);
	return NoError;
}


void	Do_SplitPolyRoof (void)
{
	API_Element element;
	BNZeroMemory (&element, sizeof (API_Element));

	if (!ClickAnElem ("Click a roof to modify", API_RoofID, NULL, &element.header.typeID, &element.header.guid)) {
		WriteReport_Alert ("No roof was clicked");
		return;
	}

	GSErrCode err = ACAPI_Element_Get (&element);
	if (err == NoError && element.roof.roofClass == API_PolyRoofID) {
		PolyRoofConverter polyRoofConverter (element.header.guid);
		err = ACAPI_Element_Decompose (element.header.guid, &polyRoofConverter);
	} else {
		WriteReport_Alert ("Please select a Multi-plane Roof");
	}

	return;
}		// Do_SplitPolyRoof


#ifdef __APPLE__
#pragma mark -
#endif

enum State {
	Nowhere = 0,
	InElem,
	InHatchLine,
	InBorder
};

enum SpecPen {
	RoomPen = 1001,
	RGBPen = 1008
};


GS::AutoPtr<IO::File>	gOutput;

static GSErrCode	OpenFile (const API_IOParams *ioParams)
{
	GSErrCode	ret = NoError;

	try {
		gOutput.Reset (new IO::File (*ioParams->fileLoc, IO::File::Create));
		if (gOutput.Get () == NULL)
			throw (1);

		ret = gOutput->GetStatus ();
		if (ret != NoError)
			throw (ret);

		gOutput->SetOutputProtocol (GS::textOProtocol);
		ret = gOutput->Open (IO::File::WriteEmptyMode);
	}
	catch (...) {
		ret = APIERR_CANCEL;
	}

	return ret;
}


static GSErrCode	CloseFile ()
{
	GSErrCode	ret = NoError;

	try {
		if (gOutput->IsOpen ())
			gOutput->Close ();
	}
	catch (...) {
		ret = APIERR_CANCEL;
	}

	gOutput.Reset ();

	return ret;
}

class PrimElem;

const int APIFill_RGB = (int) APIFill_Image + 1;

class HatchLinesCollector {
	API_PrimHatchBorder			border;
	GS::StableArray<PrimElem>	primElems;

	API_FillSubtype				fillSubtype;
	State						state;

	explicit HatchLinesCollector () {
		BNZeroMemory (&border, sizeof (API_PrimHatchBorder));
		state = Nowhere;
	}
	HatchLinesCollector (const HatchLinesCollector&);
	HatchLinesCollector& operator= (const HatchLinesCollector&);

public:
	static HatchLinesCollector&	Instance () {
		static HatchLinesCollector	collector;

		return collector;
	};

	void	SetBorder (const API_PrimHatchBorder& inBorder) {
		border = inBorder;

		API_Attribute	attr;
		BNZeroMemory (&attr, sizeof (API_Attribute));
		attr.header.typeID = API_FilltypeID;
		attr.header.index  = border.fillInd;
		if (ACAPI_Attribute_Get (&attr) == NoError)
			fillSubtype = attr.filltype.subType;
		else
			fillSubtype = APIFill_Vector;	// set to something other than "solid" or "empty"
		if (fillSubtype == APIFill_Empty && border.fillbkgPen == RGBPen)
			fillSubtype = (API_FillSubtype) APIFill_RGB;
	}

	GSErrCode	PushPrimElem (const PrimElem& inElem);

	inline void	SetState (State inState)
	{
		state = inState;
	}

	inline State	GetState (void) const
	{
		return state;
	}

	inline API_RGBColor	GetFillBackgroundColor (void) const
	{
		return border.fillbkgRgb;
	}

	inline API_RGBColor	GetFillForegroundColor (void) const
	{
		return border.fillRgb;
	}

	inline short	GetFillBackgroundPen (void) const
	{
		return border.fillbkgPen;
	}

	inline bool	HasSolidFill (void) const
	{
		return fillSubtype == APIFill_Solid;
	}

	inline bool	HasEmptyFill (void) const
	{
		return fillSubtype == APIFill_Empty || (int) fillSubtype == APIFill_RGB;
	}

	inline bool	HasRGBFill (void) const
	{
		return (int) fillSubtype == APIFill_RGB;
	}

	void	Clear () {
		BNZeroMemory (&border, sizeof (API_PrimHatchBorder));
		primElems.Clear ();
		fillSubtype = APIFill_Vector;
	}

	GSErrCode	Draw ();
};


class PrimElem {

private:
	void Copy (const API_PrimElement* elem, const GSPtr inpar1, const GSPtr inpar2, const GSPtr inpar3) {
		BNCopyMemory (&prim, elem, sizeof (PrimElem));
		if (inpar1 != NULL) {
			par1 = BMpAll (BMpGetSize (inpar1));
			if (par1 != NULL)
				BNCopyMemory (par1, inpar1, BMpGetSize (inpar1));
		} else
			par1 = NULL;
		if (inpar2 != NULL) {
			par2 = BMpAll (BMpGetSize (inpar2));
			if (par2 != NULL)
				BNCopyMemory (par2, inpar2, BMpGetSize (inpar2));
		} else
			par2 = NULL;
		if (inpar3 != NULL) {
			par3 = BMpAll (BMpGetSize (inpar3));
			if (par3 != NULL)
				BNCopyMemory (par3, inpar3, BMpGetSize (inpar3));
		} else
			par3 = NULL;
	}

	void Release () {
		if (par1 != NULL)
			BMpKill (&par1);
		if (par2 != NULL)
			BMpKill (&par2);
		if (par3 != NULL)
			BMpKill (&par3);
	}

	void Init () {
		BNZeroMemory (&prim, sizeof (API_PrimElement));
		par1 = par2 = par3 = NULL;
	}

protected:
	GSErrCode	WriteHeader ();
	GSErrCode	WriteLinetype (short ltypeInd);
	GSErrCode	WriteFillMode (GS::Bool8 solid);

	GSErrCode	DrawPoint ();
	GSErrCode	DrawLine ();
	GSErrCode	DrawArc ();
	GSErrCode	DrawPolyline ();
	GSErrCode	DrawTriangle ();
	GSErrCode	DrawPolygon ();

public:
	API_PrimElement		prim;
	GSPtr	par1, par2, par3;

	explicit PrimElem () {
		this->Init ();
	}

	PrimElem (const PrimElem& rhs) {
		this->Init ();
		this->Copy (reinterpret_cast<const API_PrimElement*> (&rhs), rhs.par1, rhs.par2, rhs.par3);
	}

	PrimElem (const API_PrimElement* inElem, const void* par1, const void *par2, const void *par3)
	{
		this->Init ();
		this->Copy (inElem, (const GSPtr) par1, (const GSPtr) par2, (const GSPtr) par3);
	}

	PrimElem& operator= (const PrimElem& rhs) {
		if (&rhs != this) {
			this->Release ();
			this->Copy (reinterpret_cast<const API_PrimElement*> (&rhs), rhs.par1, rhs.par2, rhs.par3);
		}

		return *this;
	}

	~PrimElem () {
		this->Release ();
	}

	inline void SetPen (short colorIndex) {
		prim.header.pen = colorIndex;
	}

	// Drawable interface
	GSErrCode Draw () {
		GSErrCode	ret = NoError;
		bool		isRGBFill = prim.header.typeID == API_PrimPolyID &&
								HatchLinesCollector::Instance ().GetState () == InBorder &&
								HatchLinesCollector::Instance ().HasRGBFill ();

		if (prim.header.pen == 0 && !isRGBFill)
			return ret;

		switch (prim.header.typeID) {
			case API_PrimPointID:	ret = this->DrawPoint ();		break;
			case API_PrimLineID:	ret = this->DrawLine ();		break;
			case API_PrimArcID:		ret = this->DrawArc ();			break;
	//		case API_PrimTextID:	ret = this->DrawText ();		break;
			case API_PrimPLineID:	ret = this->DrawPolyline ();	break;
			case API_PrimTriID:		ret = this->DrawTriangle ();	break;
	//		case API_PrimPictID:	ret = this->DrawPicture ();		break;
			case API_PrimPolyID:
				if (HatchLinesCollector::Instance ().GetState () == InBorder) {
					// draw background poly instead of border lines
					prim.header.pen = HatchLinesCollector::Instance ().GetFillBackgroundPen ();
					prim.poly.solid = true;
				}
				if (prim.header.pen != 0)
					ret = this->DrawPolygon ();
				break;

			default:
				break;
		}

		return ret;
	}
};


GSErrCode	HatchLinesCollector::PushPrimElem (const PrimElem& inElem)
{
	if (state != InHatchLine)
		return NoError;

	GSErrCode	ret = NoError;
	try {
		primElems.Push (inElem);

		// overwrite pen color of the new primitive; the real color is in the border
		//if (inElem.header.typeID == API_PrimPolyID)
			primElems.GetLast ().SetPen (border.fillPen);
	}
	catch (GS::OutOfMemoryException&) {
		ret = APIERR_MEMFULL;
	}

	return ret;
}


GSErrCode	HatchLinesCollector::Draw ()
{
	if (state != InElem)
		return NoError;

	GSErrCode	ret = NoError;
	for (USize ii = 0; ret == NoError && ii < primElems.GetSize (); ii++)
		ret = primElems[ii].Draw ();

	return ret;
}


GSErrCode	PrimElem::WriteHeader ()
{
	char	buff [128];
	static short	lastLay = -1, lastPen = -2;

	if (lastLay != prim.header.layer) {
		sprintf (buff, "  Layer %d\n", prim.header.layer);
		gOutput->Write (buff);
		lastLay = prim.header.layer;
	}

	if (prim.header.typeID == API_PrimPolyID && (prim.header.pen == RoomPen || prim.header.pen == RGBPen)) {
		API_RGBColor	rgb = HatchLinesCollector::Instance ().GetFillForegroundColor ();
		sprintf (buff, "  C %d,%d,%d\n", (short) (rgb.f_red * 255.0), (short) (rgb.f_green * 255.0), (short) (rgb.f_blue * 255.0));
		gOutput->Write (buff);
	} else if (lastPen != prim.header.pen) {
		sprintf (buff, "  C %d\n", prim.header.pen);
		gOutput->Write (buff);
		lastPen = prim.header.pen;
	}

	return gOutput->GetStatus ();
}


GSErrCode	PrimElem::WriteLinetype (short ltypeInd)
{
	char	buff [64];
	static short	lastLtype = -1;

	if (lastLtype != ltypeInd) {
		sprintf (buff, "  Linetype %d\n", ltypeInd);
		gOutput->Write (buff);
		lastLtype = ltypeInd;
	}

	return gOutput->GetStatus ();
}


GSErrCode	PrimElem::WriteFillMode (GS::Bool8 solid)
{
	char	buff [64];
	static GS::Bool8	lastFill = false;

	if (lastFill != solid) {
		sprintf (buff, "  %s\n", solid ? "F" : "f");
		gOutput->Write (buff);
		lastFill = solid;
	}

	return gOutput->GetStatus ();
}


GSErrCode	PrimElem::DrawPoint ()
{
	char	buff [256];

	GSErrCode	ret = WriteHeader ();
	if (ret == NoError) {
		sprintf (buff, "  Pt %5.2f,%5.2f\n", prim.point.loc.x, prim.point.loc.y);
		gOutput->Write (buff);
		ret = gOutput->GetStatus ();
	}

	return ret;
}


GSErrCode	PrimElem::DrawLine ()
{
	char	buff [256];

	GSErrCode	ret = WriteHeader ();
	if (ret == NoError) {
		ret = WriteLinetype (prim.line.ltypeInd);
	}
	if (ret == NoError) {
		sprintf (buff, "  L %5.2f,%5.2f,%5.2f,%5.2f\n", prim.line.c1.x, prim.line.c1.y, prim.line.c2.x, prim.line.c2.y);
		gOutput->Write (buff);
		ret = gOutput->GetStatus ();
	}

	return ret;
}


GSErrCode	PrimElem::DrawArc ()
{
	char	buff [256];

	GSErrCode	ret = WriteHeader ();
	if (ret == NoError) {
		ret = WriteLinetype (prim.arc.ltypeInd);
	}
	if (ret == NoError) {
		ret = WriteFillMode (prim.arc.solid);
	}
	if (ret == NoError) {
		if (fabs (prim.arc.ratio - 1.0) < EPS) {
			if (prim.arc.whole) {
				sprintf (buff, "  Ci %5.2f,%5.2f,%5.2f\n", prim.arc.r, prim.arc.orig.x, prim.arc.orig.y);
			} else {
				sprintf (buff, "  A %5.2f,%5.2f,%5.2f,%5.2f,%5.2f\n", prim.arc.r, prim.arc.orig.x, prim.arc.orig.y, prim.arc.begAng, prim.arc.endAng);
			}
		} else {
			if (prim.arc.whole) {
				sprintf (buff, "  E %5.2f,%5.2f,%5.2f,%5.2f,%5.2f\n", prim.arc.r, prim.arc.r/prim.arc.ratio, prim.arc.angle, prim.arc.orig.x, prim.arc.orig.y);
			} else {
				sprintf (buff, "  EA %5.2f,%5.2f,%5.2f,%5.2f,%5.2f,%5.2f,%5.2f\n",
						 prim.arc.r, prim.arc.r/prim.arc.ratio, prim.arc.angle, prim.arc.orig.x, prim.arc.orig.y, prim.arc.begAng, prim.arc.endAng);
			}
		}
		gOutput->Write (buff);
		ret = gOutput->GetStatus ();
	}

	return ret;
}


GSErrCode	PrimElem::DrawPolyline ()
{
	char	buff [256];

	GSErrCode	ret = WriteHeader ();
	if (ret == NoError) {
		ret = WriteLinetype (prim.pline.ltypeInd);
	}
	if (ret == NoError) {
		API_Coord	*co = (API_Coord *) par1;
		++co;
		if (par2 != NULL) {
//			API_PolyArc	*arcs = (API_PolyArc *) par2;
			// not now
		} else {
			sprintf (buff, "  Pl %ld ", (GS::LongForStdio) prim.pline.nCoords);
			gOutput->Write (buff);
			for (Int32 ii = 1; ii <= prim.pline.nCoords && ret == NoError; ii++, co++) {
				sprintf (buff, "%5.2f,%5.2f", co->x, co->y);
				gOutput->Write (buff);
				if (ii < prim.pline.nCoords)
					gOutput->Write (",");
				ret = gOutput->GetStatus ();
			}
			gOutput->Write ("\n");
		}
	}

	return ret;
}


GSErrCode	PrimElem::DrawTriangle ()
{
	char	buff [256];

	GSErrCode	ret = WriteHeader ();
	if (ret == NoError) {
		ret = WriteLinetype (prim.tri.ltypeInd);
	}
	if (ret == NoError) {
		ret = WriteFillMode (prim.tri.solid);
	}
	if (ret == NoError) {
		API_Coord	*co = &prim.tri.c[0];
		gOutput->Write ("  P 3 ");
		for (Int32 ii = 1; ii <= 3 && ret == NoError; ii++, co++) {
			sprintf (buff, "%5.2f,%5.2f", co->x, co->y);
			gOutput->Write (buff);
			if (ii < 3)
				gOutput->Write (",");
			ret = gOutput->GetStatus ();
		}
		gOutput->Write ("\n");
	}

	return ret;
}


GSErrCode	PrimElem::DrawPolygon ()
{
	char	buff [256];

	if (prim.header.pen < 0)	// !!! polygon with plan background
		return NoError;

	GSErrCode	ret = WriteHeader ();
	if (ret == NoError) {
		ret = WriteLinetype (prim.poly.ltypeInd);
	}
	if (ret == NoError) {
		ret = WriteFillMode (prim.poly.solid);
	}
	if (ret == NoError) {
		API_Coord	*co = (API_Coord *) par1;
		++co;
//		long		*pe = (long *) par2;
		if (prim.poly.nSubPolys > 1 || prim.poly.nArcs > 0) {
			// we'll have to triangulate
//			API_PolyArc	*arcs = (API_PolyArc *) par3;
			// not now
		} else {
			sprintf (buff, "  %s %ld ", prim.poly.solid ? "P" : "Pl", (GS::LongForStdio) prim.poly.nCoords);
			gOutput->Write (buff);
			for (Int32 ii = 1; ii <= prim.poly.nCoords && ret == NoError; ii++, co++) {
				sprintf (buff, "%5.2f,%5.2f", co->x, co->y);
				gOutput->Write (buff);
				if (ii < prim.poly.nCoords)
					gOutput->Write (",");
				ret = gOutput->GetStatus ();
			}
			gOutput->Write ("\n");
		}
	}

	return ret;
}


static GSErrCode __ACENV_CALL	DrawPrimitives (const API_PrimElement	*primElem,
												const void				*par1,
												const void				*par2,
												const void				*par3)
{
	switch (primElem->header.typeID) {
		case API_PrimPointID:
		case API_PrimLineID:
		case API_PrimArcID:
//		case API_PrimTextID:
		case API_PrimPLineID:
		case API_PrimTriID:
//		case API_PrimPictID:
		case API_PrimPolyID:
			if (HatchLinesCollector::Instance ().GetState () == InHatchLine)
				HatchLinesCollector::Instance ().PushPrimElem (PrimElem (primElem, par1, par2, par3));
			else {
				if (HatchLinesCollector::Instance ().GetState () != InBorder ||
				    (HatchLinesCollector::Instance ().GetState () == InBorder && !HatchLinesCollector::Instance ().HasSolidFill ())) {
						// draw background only if the foreground is not solid fill
					PrimElem	pe (primElem, par1, par2, par3);
					pe.Draw ();
				}
			}
			break;

		case API_PrimCtrl_BegID:
			HatchLinesCollector::Instance ().SetState (InElem);
			break;

		case API_PrimCtrl_EndID:
			HatchLinesCollector::Instance ().SetState (Nowhere);
			break;

		case API_PrimCtrl_HatchLinesBegID:
			HatchLinesCollector::Instance ().SetState (InHatchLine);
			HatchLinesCollector::Instance ().SetBorder (* (API_PrimHatchBorder *) par1);
			break;

		case API_PrimCtrl_HatchLinesEndID:
			HatchLinesCollector::Instance ().SetState (InElem);
			break;

		case API_PrimCtrl_HatchBorderBegID:
			gOutput->Write ("# Background\n");
			HatchLinesCollector::Instance ().SetState (InBorder);
			break;

		case API_PrimCtrl_HatchBorderEndID:
			HatchLinesCollector::Instance ().SetState (InElem);

			if (!HatchLinesCollector::Instance ().HasEmptyFill ()) {
				// draw hatch lines only if the it's not an empty fill
				gOutput->Write ("# Hatch lines\n");
				HatchLinesCollector::Instance ().Draw ();	// now draw the collected hatch lines
			}
			HatchLinesCollector::Instance ().Clear ();	// also clears internal state

			gOutput->Write ("# Hatch border\n");
			break;

		default:
			/* not implemented */
			break;
	}

	return NoError;
}


class SetMasterLayoutOnLayoutGuard {
public:
	SetMasterLayoutOnLayoutGuard (const API_DatabaseUnId& databaseUnId)
	{
		ACAPI_Goodies (APIAny_SetMasterLayoutOnLayoutID, (void*)(&databaseUnId), NULL);
	}
	~SetMasterLayoutOnLayoutGuard ()
	{
		ACAPI_Goodies (APIAny_SetMasterLayoutOnLayoutID, NULL, NULL);
	}
};


// -----------------------------------------------------------------------------
// Dump the 2D model (concentrate on hatches for the moment)
// -----------------------------------------------------------------------------

static GSErrCode DumpPrimitives ()
{
	GSErrCode	ret = ACAPI_Database (APIDb_DrawOrderInitID, NULL, NULL);
	if (ret != NoError)
		return ret;

	API_Element	elem;
	while ((ret = ACAPI_Database (APIDb_DrawOrderGetNextID, (GSPtr)(Int32)(APIFilt_OnVisLayer | APIFilt_OnActFloor), &elem.header)) == NoError) {
		if (elem.header.typeID != API_HatchID && elem.header.typeID != API_DrawingID &&
			elem.header.typeID != API_ZoneID && elem.header.typeID != API_SlabID &&
			elem.header.typeID != API_MeshID && elem.header.typeID != API_RoofID &&
			elem.header.typeID != API_LineID && elem.header.typeID != API_PolyLineID &&
			elem.header.typeID != API_TextID)
			continue;

		ret = ACAPI_Element_ShapePrims (elem.header, DrawPrimitives);
	}
	ACAPI_Database (APIDb_DrawOrderTermID, NULL, NULL);
	if (ret == APIERR_NOMORE)
		ret = NoError;

	return ret;
}		// DumpPrimitives


// -----------------------------------------------------------------------------
// Dump the 2D model
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL Do_ExportPrimitives (const API_IOParams *ioParams)
{
	GSErrCode	err = NoError;

	// open the file
	err = OpenFile (ioParams);
	if (err != NoError)
		return err;

	// check master layout
	bool needMaster = false;
	bool showMasterBelow = false;
	API_DatabaseInfo origDBInfo;
	API_DatabaseInfo masterDBInfo;
	BNZeroMemory (&origDBInfo,		sizeof (API_DatabaseInfo));
	BNZeroMemory (&masterDBInfo,	sizeof (API_DatabaseInfo));
	if (err == NoError) {
		err = ACAPI_Database (APIDb_GetCurrentDatabaseID, &origDBInfo, NULL);
		if (err == NoError && origDBInfo.masterLayoutUnId.elemSetId != APINULLGuid) {
			needMaster = true;
			masterDBInfo.databaseUnId = origDBInfo.masterLayoutUnId;
			masterDBInfo.typeID = APIWind_MasterLayoutID;
			API_LayoutInfo masterLayoutInfo;
			BNZeroMemory (&masterLayoutInfo, sizeof (API_LayoutInfo));
			err = ACAPI_Environment (APIEnv_GetLayoutSetsID, &masterLayoutInfo, &masterDBInfo.databaseUnId);
			showMasterBelow = GS::Bool8ToBool (masterLayoutInfo.showMasterBelow);
			if (masterLayoutInfo.customData != NULL)
				delete masterLayoutInfo.customData;
		}
	}

	// draw master layout before the layout if needed
	if (err == NoError && needMaster && showMasterBelow) {
		// switch to master layout
		err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &masterDBInfo);
		err = ACAPI_Database (APIDb_RebuildCurrentDatabaseID);
		if (err == NoError) {
			SetMasterLayoutOnLayoutGuard setMasterLayoutOnLayoutGuard (origDBInfo.databaseUnId);
			DumpPrimitives ();
		}
		err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &origDBInfo);
	}

	// draw current layout
	if (err == NoError)
		DumpPrimitives ();

	// draw master layout after the layout if needed
	if (err == NoError && needMaster && !showMasterBelow) {
		// switch to master layout
		err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &masterDBInfo);
		err = ACAPI_Database (APIDb_RebuildCurrentDatabaseID);
		if (err == NoError) {
			SetMasterLayoutOnLayoutGuard setMasterLayoutOnLayoutGuard (origDBInfo.databaseUnId);
			DumpPrimitives ();
		}
		err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &origDBInfo);
	}

	// close the file
	err = CloseFile ();

	return err;
}		// Do_ExportPrimitives


// -----------------------------------------------------------------------------
// Import as Drawing callback
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL Do_Import (const API_IOParams *ioParams)
{
	GSErrCode	err = OpenFile (ioParams);

	DBPrintf ("Element_Test :: DoImport called to %s drawing\n", (ioParams->drawingGuid != APINULLGuid) ? "relink" : "place");

	if (err == NoError) {
		err = ACAPI_CallUndoableCommand ("",
			[&] () -> GSErrCode {
				API_Coord	drawingPos;
				drawingPos.x = drawingPos.y = 0.0;
				if (ioParams->drawingGuid != APINULLGuid) {
					API_Element	elem;
					BNZeroMemory (&elem, sizeof (elem));
					elem.header.guid = ioParams->drawingGuid;		// !!!

					if (ACAPI_Element_Get (&elem) == NoError) {
						drawingPos = elem.drawing.pos;
						const GSErrCode	err2 = DeleteOneElement (ioParams->drawingGuid);
						if (err2 == NoError) {
							DBPrintf ("\t\tDrawing %s was deleted successfully.\n", APIGuidToString (elem.header.guid).ToCStr ().Get ());
						} else {
							DBPrintf ("\t\tError %ld deleting drawing %s.\n", (Int32) err2 & 0x0000FFFF, APIGuidToString (elem.header.guid).ToCStr ().Get ());
						}
					}

				}
				Do_CreateDrawing (ioParams->drawingGuid, &drawingPos);

				return NoError;
			});
	}

	if (err == NoError)
		err = CloseFile ();

	return err;
}		// Do_Import

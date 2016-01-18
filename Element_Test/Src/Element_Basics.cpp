// *****************************************************************************
// Source code for the Element Test Add-On
// API Development Kit 19; Mac/Win
//
//	Main and common functions
//
// Namespaces:        Contact person:
//     -None-
//
// [SG compatible] - Yes
// *****************************************************************************

#include "APIEnvir.h"
#define	_ELEMENT_BASICS_TRANSL_


// ---------------------------------- Includes ---------------------------------

#include	<math.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#include	"ACAPinc.h"					// also includes APIdefs.h

#include	"APICommon.h"
#include	"Element_Test.h"

#include	"Location.hpp"
#include	"File.hpp"

#include	"uchar_t.hpp"

#include	"Dumper.h"

#include	"GXImageBase.h"

// ---------------------------------- Types ------------------------------------


// ---------------------------------- Variables --------------------------------


// ---------------------------------- Prototypes -------------------------------



// =============================================================================
//
// Utility functions
//
// =============================================================================

static void		usprintf (GS::uchar_t* dst, Int32 dstsize, const char* fmt, ...)
{
	va_list ap;
	static char	buffer[1024];

	va_start(ap, fmt);
	vsprintf (buffer, fmt, ap);
	va_end(ap);

	GS::UniString	tmp (buffer);
	GS::ucsncpy (dst, tmp.ToUStr(), dstsize / sizeof (GS::UniChar) - 1);
}

// ============================================================================
// Search by UnID
//
// ============================================================================
static bool	 Do_LibpartSearchByUnID (API_ElemTypeID	 typeID, API_AddParType  ***markAddPars, Int32 *origLibInd)
{
	Int32                addParNum;
	API_AddParType       **addPars = NULL;
	double               a, b;
	GSErrCode            err = NoError;
	API_LibPart			 libpart;
	char				 unid1[128];
	char				 unid2[128];
	char				 unid3[128];
	char				 unid4[128];

	switch (typeID) {
		case API_DetailID:
			strcpy (unid1, "{73B06DCB-55C0-4573-8635-8439A8DC3FAE}-{120765D3-2EB2-4CE8-8B65-5C8BEA1FD9DF}");
			strcpy (unid2, "{D8E3B58C-1B61-4660-B556-E59CA780A217}-{4BC17A74-4707-475C-B8C3-4AAEB5E66BE9}");
			strcpy (unid3, "{2C409D23-C2A3-4446-85AE-FFFB501C6F72}-{6F776393-7555-4983-986E-2AF112F75DB2}");	//in ArchiCAD.dll
			strcpy (unid4, "");
			break;
		case API_WorksheetID:
			strcpy (unid1, "{84E14FDD-FE29-497C-9C9F-6BCE7286F0A6}-{FE0304BA-5512-4D12-B6D9-B2CC8D873300}");	// Basic Worksheet Marker
			strcpy (unid2, "");																					// Simple Worksheet Marker
			strcpy (unid3, "");
			strcpy (unid4, "");
			break;
		case API_CutPlaneID:
			strcpy (unid1, "{0650F435-2C8A-4182-B311-271E890AD1AC}-{B6BC3B82-F221-47C2-8D29-8C32B30064E1}");	// Basic Section Marker
			strcpy (unid2, "{6DD7E0E5-C35D-44C6-9C6D-D9090C5C2FBE}-{D05C595C-0842-492D-B979-6D5930FF7094}");	// Basic Section-Elev Marker
			strcpy (unid3, "{A533987C-D38E-4F01-A790-5ACC99A18DCA}-{99992154-2372-4781-B0B1-8E788632014E}");	// Section-Elevation Marker - 70
			strcpy (unid4, "");
			break;
		case API_ElevationID:
			strcpy (unid1, "{8ABB49D8-B53D-4D92-88E6-7467DC20D491}-{055E1FBF-5FE0-426B-9289-16FDB143491B}");	// Basic Section Marker
			strcpy (unid2, "");
			strcpy (unid3, "");
			strcpy (unid4, "");
			break;
		case API_InteriorElevationID:
			strcpy (unid1, "{510BC02A-85AE-4598-8EC6-2F333C26EC5D}-{7F67D33C-66B7-4576-9F45-A69F20AB3D2D}");	// Common IE Marker 1
			strcpy (unid2, "{724B7CEC-92C1-41A3-883E-3100D6F274DB}-{D38663B1-5C13-4288-B951-60D118E429C2}");	// Common IE Marker 2
			strcpy (unid3, "");
			strcpy (unid4, "");
			break;
		case API_DoorID:
			strcpy (unid1, "{292E664B-B671-48AD-BA71-3EF25461DA99}-{311E9D20-7449-4F5E-BB78-43C00FDDE7E5}");
			strcpy (unid2, "{50DC8FE6-BF72-DB44-8B8C-9BC7B68508F7}-{7A1DD325-2EDD-464B-9D11-0BE5021FBF42}");	//in ArchiCAD.dll
			strcpy (unid3, "{036F7C0F-7E36-47B1-8B70-D0CD051029B6}-{867B3F0C-22B5-4ECB-A905-2ADDFCFC1D90}");	//in ArchiCAD.dll
			strcpy (unid4, "");
			break;
		case API_WindowID:
			strcpy (unid1, "{036F7C0F-7E36-47B1-8B70-D0CD051029B6}-{867B3F0C-22B5-4ECB-A905-2ADDFCFC1D90}");
			strcpy (unid2, "{C651965D-E5DA-4920-926D-F873108FEB7F}-{287E3C61-0B8C-4082-BE2A-6AF18E824A7E}");	//in ArchiCAD.dll
			strcpy (unid3, "{8410F22D-3B0E-6943-B4F2-EDDBAA8C91D9}-{C8F52822-AA23-49CD-A256-B67B5A084CDA}");	//in ArchiCAD.dll
			strcpy (unid4, "");
			break;
		case API_SkylightID:
			strcpy (unid1, "{0654C626-430D-11DF-85ED-521F56D89593}-{4CBB1F66-5DD9-45ED-9E5A-ED0DB90E6648}");
			strcpy (unid2, "{4E82DDBC-2782-4A64-9E4F-64C0E772B794}-{46FF0DEF-0F75-42F6-B5DD-6F62913FC6A5}");	//in ArchiCAD.dll
			strcpy (unid3, "{58BDEC28-D93F-4262-95CE-D537B32C7BE6}-{3C3B52C1-432A-4E22-ABD9-F57A4600E8B7}");	//in ArchiCAD.dll
			strcpy (unid4, "");
			break;
		default:
			break;
	}

	BNZeroMemory (&libpart, sizeof (libpart));
	strcpy (libpart.ownUnID, unid1);
	err = ACAPI_LibPart_Search (&libpart, false);
	if (err == NoError && *origLibInd != libpart.index) {
		err = ACAPI_LibPart_GetParams (libpart.index, &a, &b, &addParNum, &addPars);
		if (err == NoError) {
			delete libpart.location;
			*origLibInd = libpart.index;
			*markAddPars = addPars;
			return true;
		}
	}

	BNZeroMemory (&libpart, sizeof (libpart));
	strcpy (libpart.ownUnID, unid2);
	err = ACAPI_LibPart_Search (&libpart, false);
	if (err == NoError && *origLibInd != libpart.index) {
		err = ACAPI_LibPart_GetParams (libpart.index, &a, &b, &addParNum, &addPars);
		if (err == NoError) {
			delete libpart.location;
			*origLibInd = libpart.index;
			*markAddPars = addPars;
			return true;
		}
	}

	BNZeroMemory (&libpart, sizeof (libpart));
	strcpy (libpart.ownUnID, unid3);
	err = ACAPI_LibPart_Search (&libpart, false);
	if (err == NoError && *origLibInd != libpart.index) {
		err = ACAPI_LibPart_GetParams (libpart.index, &a, &b, &addParNum, &addPars);
		if (err == NoError) {
			delete libpart.location;
			*origLibInd = libpart.index;
			*markAddPars = addPars;
			return true;
		}
	}

	BNZeroMemory (&libpart, sizeof (libpart));
	strcpy (libpart.ownUnID, unid4);
	err = ACAPI_LibPart_Search (&libpart, false);
	if (err == NoError && *origLibInd != libpart.index) {
		err = ACAPI_LibPart_GetParams (libpart.index, &a, &b, &addParNum, &addPars);
		if (err == NoError) {
			delete libpart.location;
			*origLibInd = libpart.index;
			*markAddPars = addPars;
			return true;
		}
	}

	markAddPars = NULL;

	return false;
}	// Do_LibpartSearchByUnID


// -----------------------------------------------------------------------------
// Dump owner information
// -----------------------------------------------------------------------------
static void		DumpOwner (const char 				*info,
						   const API_ProjectInfo	*projectInfo,
						   const API_SharingInfo	*sharingInfo,
						   short 					userId)
{
	char		ownerStr[256];
	bool		found;
	Int32		i;

	if (userId == 0) {
		strcpy (ownerStr, "none");
	} else if (userId == projectInfo->userId) {
		strcpy (ownerStr, "me");
	} else {
		found = false;
		for (i = 0; i < sharingInfo->nUsers; i++) {
			if ((*sharingInfo->users)[i].userId == userId) {
				found = true;
				break;
			}
		}
		if (found) {
			GS::UniString fullName ((*sharingInfo->users)[i].fullName);
			sprintf (ownerStr, "\"%s\" connected: %s", fullName.ToCStr ().Get (), (*sharingInfo->users)[i].connected ? "YES" : "NO");
		} else {
			sprintf (ownerStr, "not found");
		}
	}
	WriteReport ("%s %s", info, ownerStr);

	return;
}		// DumpOwner


// -----------------------------------------------------------------------------
// Compare two elements
// -----------------------------------------------------------------------------
static bool	CompareElems (const API_Element& 		elem1,
						  const API_ElementMemo&	memo1)
{
	GSErrCode	err;

	API_ElementMemo memo2;
	BNZeroMemory (&memo2, sizeof (API_ElementMemo));

	if (elem1.header.guid != APINULLGuid) {
		API_Element elem2;
		BNZeroMemory (&elem2, sizeof (API_Element));
		elem2.header.guid = elem1.header.guid;

		err = ACAPI_Element_Get (&elem2);
		if (err == NoError)
			err = ACAPI_Element_GetMemo (elem2.header.guid, &memo2);

		if (err != NoError)
			return false;
	}

	bool same = true;

	const Int32 nHandle = sizeof (API_ElementMemo) / sizeof (GSHandle);
	for (Int32 i = 0; i < nHandle; i++) {
		GSConstHandle h1 = ((GSHandle *) &memo1)[i];
		GSConstHandle h2 = ((GSHandle *) &memo2)[i];
		if (h1 == NULL && h2 == NULL)
			continue;
		if (h1 == NULL || h2 == NULL) {
			same = false;
			DBBREAK ();
			DBPrintf ("CompareElems: memo handles differ: [%d] 0x%08x, %08x", i, h1, h2);
		}
	}

	if (elem1.header.guid != APINULLGuid)
		ACAPI_DisposeElemMemoHdls (&memo2);

	return same;
}		// CompareElems


// -----------------------------------------------------------------------------
// Draw the drawing primitives on the floorplan
// -----------------------------------------------------------------------------
static GSErrCode __ACENV_CALL	Draw_ShapePrims (const API_PrimElement*	primElem,
												 const void*			par1,
												 const void*			par2,
												 const void*			par3)
{
	DumpPrimitive (primElem, par1, par2, par3);
	return NoError;
}		// Draw_ShapePrims


// -----------------------------------------------------------------------------
// Check in the AddPar whether the ChangeMarker Marker has a polygon or not
// -----------------------------------------------------------------------------
static bool	ChangeMarkerMarkerHasPolygon (API_AddParType** api_addPars)
{
	if (DBERROR (api_addPars == NULL))
		return false;

	const USize			nAddPars				  = BMhGetSize ((GSHandle) api_addPars) / Sizeof32 (API_AddParType);
	const GS::UniString	acHasPolygonParameterName = "AC_HasPolygon";

	for (USize i = 0; i < nAddPars; i++) {
		if ((*api_addPars)[i].name == acHasPolygonParameterName)
			return fabs ((*api_addPars)[i].value.real) >= EPS;
	}

	DBBREAK_STR ("No library part parameter named 'AC_HasPolygon' was found.");
	return false;
}


static void	ReplaceEmptyTextWithPredefined (GSHandle& textContent)
{
	const char* predefinedContent = "Default text was empty.";

	if (textContent == NULL || Strlen32 (*textContent) == 0) {
		BMhKill (&textContent);
		textContent = BMhAllClear (Strlen32 (predefinedContent) + 1);
		if (BMError () == NoError)
			strcpy (*textContent, predefinedContent);
	}
}


// =============================================================================
//
// General element functions
//
// =============================================================================
#ifdef __APPLE__
#pragma mark -
#endif

// -----------------------------------------------------------------------------
// Count all lines in the project
//	- use filtering support for fast access
//	- only the current database is scanned
// -----------------------------------------------------------------------------
void	Do_CountLines (void)
{
	API_ProjectInfo		projectInfo;
	Int32				n;
	GSErrCode			err;

	GS::Array<API_Guid> lineList;
	err = ACAPI_Element_GetElemList (API_LineID, &lineList);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetElemList ()", err);
		return;
	}
	WriteReport ("Number of total lines: %u", (GS::UIntForStdio) lineList.GetSize ());

	if (lineList.IsEmpty ())
		return;

	n = 0;
	for (GS::Array<API_Guid>::ConstIterator it = lineList.Enumerate (); it != NULL; ++it) {
		if (ACAPI_Element_Filter (*it, APIFilt_IsEditable))
			n++;
	}
	WriteReport ("Number of editable lines: %d", n);

	n = 0;
	for (GS::Array<API_Guid>::ConstIterator it = lineList.Enumerate (); it != NULL; ++it) {
		if (ACAPI_Element_Filter (*it, APIFilt_OnActFloor))
			n++;
	}
	WriteReport ("Number of lines on the actual floor: %d", n);

	n = 0;
	for (GS::Array<API_Guid>::ConstIterator it = lineList.Enumerate (); it != NULL; ++it) {
		if (ACAPI_Element_Filter (*it, APIFilt_OnVisLayer | APIFilt_OnActFloor))
			n++;
	}
	WriteReport ("Number of visible lines: %d", n);


	BNZeroMemory (&projectInfo, sizeof (API_ProjectInfo));
	err = ACAPI_Environment (APIEnv_ProjectID, &projectInfo, NULL);
	if (err != NoError) {
		ErrorBeep ("APIEnv_ProjectID", err);
		return;
	}

	if (projectInfo.teamwork) {
		n = 0;
		for (GS::Array<API_Guid>::ConstIterator it = lineList.Enumerate (); it != NULL; ++it) {
			if (ACAPI_Element_Filter (*it, APIFilt_InMyWorkspace))
				n++;
		}
		WriteReport ("Number of lines in my workspace: %d", n);
	}

	if (projectInfo.location != NULL)
		delete projectInfo.location;
	if (projectInfo.location_team != NULL)
		delete projectInfo.location_team;
}		// Do_CountLines


// -----------------------------------------------------------------------------
// Create a line using the default settings and user input
// -----------------------------------------------------------------------------
void	Do_CreateLine (API_Guid& guid)
{
	API_Coord			c;
	API_GetLineType		clickInfo;
	API_Element			element;
	GSErrCode			err;

	// input the coordinates
	BNZeroMemory (&clickInfo, sizeof (API_GetLineType));
	if (!ClickAPoint ("Click the line start point", &c))
		return;

	CHCopyC ("Click the line end point", clickInfo.prompt);

	clickInfo.startCoord.x = c.x;
	clickInfo.startCoord.y = c.y;
	err = ACAPI_Interface (APIIo_GetLineID, &clickInfo, NULL);
	if (err != NoError)
		return;

	// real work
	BNZeroMemory (&element, sizeof (API_Element));
	element.header.typeID = API_LineID;
	ACAPI_Element_GetDefaults (&element, NULL);

	element.header.renovationStatus = API_DemolishedStatus;
	element.line.begC.x = clickInfo.startCoord.x;
	element.line.begC.y = clickInfo.startCoord.y;
	element.line.endC.x = clickInfo.pos.x;
	element.line.endC.y = clickInfo.pos.y;
	err = ACAPI_Element_Create (&element, NULL);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Create (line)", err);
		return;
	}

	WriteReport_Alert ("Guid of the line: %s", APIGuidToString (element.header.guid).ToCStr ().Get ());
	guid = element.header.guid;		// store it for later use

	ACAPI_KeepInMemory (true);

	return;
}		// Do_CreateLine


// -----------------------------------------------------------------------------
// Search for a line by unique ID
// -----------------------------------------------------------------------------
void	Do_GetLineByGuid (const API_Guid& guid)
{
	if (guid == GS::NULLGuid) {
		WriteReport_Alert ("# Please call the \"Create Line\" command before");
		return;
	}

	API_Element element;
	BNZeroMemory (&element, sizeof (API_Element));
	element.header.guid = guid;

	const GSErrCode err = ACAPI_Element_Get (&element);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Get (line)", err);
		return;
	}

	GS::UniString	renovationFilterName;
	ACAPI_Goodies (APIAny_GetRenovationFilterNameID, &element.header.renovationFilterGuid, &renovationFilterName);

	WriteReport ("Line:");
	WriteReport ("  guid: %s", APIGuidToString (element.header.guid).ToCStr ().Get ());
	WriteReport ("  begC: (%lf, %lf)", element.line.begC.x, element.line.begC.y);
	WriteReport ("  endC: (%lf, %lf)", element.line.endC.x, element.line.endC.y);
	if (renovationFilterName.GetLength ())
		WriteReport ("  renovation filter: \"%s\"", (const char*) renovationFilterName.ToCStr ());
}		// Do_GetLineByGuid


// -----------------------------------------------------------------------------
// Create a detail element
// -----------------------------------------------------------------------------
void	Do_CreateDetail (void)
{
	API_Element			element;
	API_ElementMemo		memo;
	API_SubElemMemoMask	marker;
	API_AddParType		**markAddPars;
	API_Coord			c2;
	GSErrCode			err = NoError;

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&marker, sizeof (API_SubElemMemoMask));

	element.header.typeID = API_DetailID;
	marker.subType = (API_SubElemMemoMaskType) (APISubElemMemoMask_MainMarker | APISubElemMemoMask_NoParams);

	err = ACAPI_Element_GetDefaultsExt (&element, &memo, 1UL, &marker);
	if (err != NoError) {
		ACAPI_DisposeElemMemoHdls (&memo);
		return;
	}

	if (!Do_LibpartSearchByUnID (API_DetailID, &markAddPars, &marker.subElem.object.libInd)) {
		ACAPI_DisposeElemMemoHdls (&memo);
		return;
	}

	marker.memo.params = markAddPars;

	if (!ClickAPoint ("Place the detail", &c2)) {
		ACAPI_DisposeElemMemoHdls (&memo);
		ACAPI_DisposeElemMemoHdls (&marker.memo);
		return;
	}

	usprintf (element.detail.detailName, sizeof (element.detail.detailName), "Detail_API (%.3f,%.3f)", c2.x, c2.y);
	usprintf (element.detail.detailIdStr, sizeof (element.detail.detailIdStr), "IDstring_API");
	element.detail.linkData.sourceMarker = true;
	element.detail.pos = c2;
	element.detail.poly.nCoords = 5;
	element.detail.poly.nSubPolys = 1;
	element.detail.poly.nArcs = 0;
	memo.coords = (API_Coord**) BMAllocateHandle ((element.detail.poly.nCoords + 1) * sizeof (API_Coord), ALLOCATE_CLEAR, 0);
	memo.pends  = (Int32**) BMAllocateHandle ((element.detail.poly.nSubPolys + 1) * sizeof (Int32), ALLOCATE_CLEAR, 0);
	if (memo.coords != NULL && memo.pends != NULL) {
		(*memo.coords)[1].x = c2.x - 1.0;
		(*memo.coords)[1].y = c2.y;
		(*memo.coords)[2].x = c2.x;
		(*memo.coords)[2].y = c2.y - 1.0;
		(*memo.coords)[3].x = c2.x + 1.0;
		(*memo.coords)[3].y = c2.y;
		(*memo.coords)[4].x = c2.x;
		(*memo.coords)[4].y = c2.y + 1.0;
		(*memo.coords)[5].x = (*memo.coords)[1].x;
		(*memo.coords)[5].y = (*memo.coords)[1].y;

		(*memo.pends)[0] = 0;
		(*memo.pends)[1] = element.detail.poly.nCoords;
	}

	marker.subElem.object.pen = 3;
	marker.subElem.object.useObjPens = true;
	marker.subElem.object.pos.x = c2.x + 1.5;
	marker.subElem.object.pos.y = c2.y + 1.0;

	marker.subType = APISubElemMemoMask_MainMarker;
	err = ACAPI_Element_CreateExt (&element, &memo, 1UL, &marker);
	if (err != NoError)
		ErrorBeep ("ACAPI_Element_CreateExt (Detail)", err);

	ACAPI_DisposeElemMemoHdls (&memo);
	ACAPI_DisposeElemMemoHdls (&marker.memo);

	return;
}		// Do_CreateDetail


// -----------------------------------------------------------------------------
// Create a Change Marker element
// -----------------------------------------------------------------------------
void	Do_CreateChangeMarker (void)
{
	API_Element element;
	BNZeroMemory (&element, sizeof (API_Element));
	element.header.typeID = API_ChangeMarkerID;

	API_SubElemMemoMask	marker;
	BNZeroMemory (&marker, sizeof (API_SubElemMemoMask));
	marker.subType = (API_SubElemMemoMaskType) (APISubElemMemoMask_MainMarker);

	API_ElementMemo memo;
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

	if (ACAPI_Element_GetDefaultsExt (&element, &memo, 1UL, &marker) != NoError) {
		ACAPI_DisposeElemMemoHdls (&memo);
		return;
	}

	API_Coord c2;
	if (!ClickAPoint ("Place the ChangeMarker", &c2)) {
		ACAPI_DisposeElemMemoHdls (&memo);
		ACAPI_DisposeElemMemoHdls (&marker.memo);
		return;
	}

	usprintf (element.changeMarker.changeId, sizeof (element.changeMarker.changeId), "Test CM Id");
	usprintf (element.changeMarker.changeName, sizeof (element.changeMarker.changeName), "Test CM Name (%.3f, %.3f)", c2.x, c2.y);
	element.changeMarker.linkType = APICMLT_CreateNewChange;

	if (ChangeMarkerMarkerHasPolygon (marker.memo.params)) {
		element.changeMarker.poly.nCoords = 5;
		element.changeMarker.poly.nSubPolys = 1;
		element.changeMarker.poly.nArcs = 0;
		memo.coords = (API_Coord**) BMAllocateHandle ((element.changeMarker.poly.nCoords + 1) * sizeof (API_Coord), ALLOCATE_CLEAR, 0);
		memo.pends  = (Int32**) BMAllocateHandle ((element.changeMarker.poly.nSubPolys + 1) * sizeof (Int32), ALLOCATE_CLEAR, 0);
		if (memo.coords != NULL && memo.pends != NULL) {
			(*memo.coords)[1].x = c2.x - 1.0;
			(*memo.coords)[1].y = c2.y;
			(*memo.coords)[2].x = c2.x;
			(*memo.coords)[2].y = c2.y - 1.0;
			(*memo.coords)[3].x = c2.x + 1.0;
			(*memo.coords)[3].y = c2.y;
			(*memo.coords)[4].x = c2.x;
			(*memo.coords)[4].y = c2.y + 1.0;
			(*memo.coords)[5].x = (*memo.coords)[1].x;
			(*memo.coords)[5].y = (*memo.coords)[1].y;

			(*memo.pends)[0] = 0;
			(*memo.pends)[1] = element.changeMarker.poly.nCoords;
		}
	} else {
		element.changeMarker.pos = c2;
	}

	marker.subElem.object.pen = 3;
	marker.subElem.object.useObjPens = true;
	marker.subElem.object.pos.x = c2.x + 1.5;
	marker.subElem.object.pos.y = c2.y + 1.0;

	marker.subType = APISubElemMemoMask_MainMarker;
	const GSErrCode err = ACAPI_Element_CreateExt (&element, &memo, 1UL, &marker);
	if (err != NoError)
		ErrorBeep ("ACAPI_Element_CreateExt (ChangeMarker)", err);

	ACAPI_DisposeElemMemoHdls (&memo);
	ACAPI_DisposeElemMemoHdls (&marker.memo);
}


// -----------------------------------------------------------------------------
// Create a static dimension with leaderline
// -----------------------------------------------------------------------------
void	Do_CreateStaticDimension (void)
{
	API_Element			element;
	API_ElementMemo		memo;
	GSErrCode			err = NoError;

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

	element.header.typeID = API_DimensionID;

	err = ACAPI_Element_GetDefaults (&element, &memo);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetDefaults (API_DimensionID)", err);
		ACAPI_DisposeElemMemoHdls (&memo);
		return;
	}

	element.dimension.dimAppear			= APIApp_Normal;
	element.dimension.textPos			= APIPos_Above;
	element.dimension.textWay			= APIDir_Parallel;
	element.dimension.defStaticDim		= true;
	element.dimension.usedIn3D			= false;
	element.dimension.horizontalText	= false;

	element.dimension.nDimElem			= 4;
	element.dimension.refC.x			= 0.0;
	element.dimension.refC.y			= 10.0;
	element.dimension.direction.x		= 1.0;
	element.dimension.direction.y		= 0.0;

	memo.dimElems = reinterpret_cast<API_DimElem**> (BMhAllClear (element.dimension.nDimElem * sizeof (API_DimElem)));
	if (memo.dimElems == NULL || *memo.dimElems == NULL) {
		ACAPI_DisposeElemMemoHdls (&memo);
		return;
	}

	double prevX = element.dimension.refC.x;
	for (Int32 i = 0; i < element.dimension.nDimElem; ++i) {
		API_DimElem& dimElem = (*memo.dimElems)[i];
		if (i == 0)
			dimElem.base.loc.x			= prevX;
		else
			dimElem.base.loc.x			= prevX + 5.0 + (i - 1);
		dimElem.base.loc.y				= element.dimension.refC.y - 5.0;
		dimElem.note					= element.dimension.defNote;
		dimElem.witnessVal				= element.dimension.defWitnessVal;
		dimElem.witnessForm				= element.dimension.defWitnessForm;
		dimElem.fixedPos				= true;
		dimElem.pos.x					= dimElem.base.loc.x;
		dimElem.pos.y					= element.dimension.refC.y;
		if (i == 1) {
			dimElem.note.fixPos			= false;
		}

		if (i == 2) {
			dimElem.note.fixPos			= true;
			dimElem.note.pos.x			= dimElem.pos.x - 1.0;
			dimElem.note.pos.y			= dimElem.pos.y + 1.0;
			dimElem.note.noteAngle		= PI / 4.0;
		}

		if (i == 3) {
			dimElem.note.fixPos			= true;
			dimElem.note.useLeaderLine	= true;
			dimElem.note.anchorAtTextEnd = false;
			dimElem.note.begC.x			= dimElem.pos.x - 2.5;
			dimElem.note.begC.y			= dimElem.pos.y;
			dimElem.note.midC.x			= dimElem.note.begC.x + 1.0;
			dimElem.note.midC.y			= dimElem.note.begC.y + 2.0;
			dimElem.note.endC.x			= dimElem.note.midC.x + 5.0;
			dimElem.note.endC.y			= dimElem.note.midC.y;
		}

		prevX = dimElem.base.loc.x;
	}

	err = ACAPI_Element_Create (&element, &memo);
	if (err != NoError)
		ErrorBeep ("ACAPI_Element_Create (Static Dimension)", err);

	ACAPI_DisposeElemMemoHdls (&memo);
}


// -----------------------------------------------------------------------------
// Create a static angle dimension with leaderline
// -----------------------------------------------------------------------------
void	Do_CreateStaticAngleDimension (void)
{
	API_Element			element;
	API_ElementMemo		memo;
	GSErrCode			err = NoError;

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

	element.header.typeID = API_AngleDimensionID;

	err = ACAPI_Element_GetDefaults (&element, &memo);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetDefaults (API_AngleDimensionID)", err);
		ACAPI_DisposeElemMemoHdls (&memo);
		return;
	}

	element.angleDimension.textPos				= APIPos_Above;
	element.angleDimension.textWay				= APIDir_Parallel;
	element.angleDimension.smallArc				= true;

	element.angleDimension.base[0].loc.x		= 0.0;
	element.angleDimension.base[0].loc.y		= 0.0;
	element.angleDimension.base[1].loc.x		= 5.0;
	element.angleDimension.base[1].loc.y		= 0.0;
	element.angleDimension.base[2].loc.x		= 0.0;
	element.angleDimension.base[2].loc.y		= 0.0;
	element.angleDimension.base[3].loc.x		= 5.0;
	element.angleDimension.base[3].loc.y		= 5.0;

	element.angleDimension.pos.x				= 5.0 * cos (PI / 8.0);
	element.angleDimension.pos.y				= 5.0 * sin (PI / 8.0);
	element.angleDimension.origo.x				= 0.0;
	element.angleDimension.origo.y				= 0.0;

	element.angleDimension.note.fixPos			= true;
	element.angleDimension.note.useLeaderLine	= true;
	element.angleDimension.note.anchorAtTextEnd = false;
	element.angleDimension.note.begC.x			= element.angleDimension.pos.x;
	element.angleDimension.note.begC.y			= element.angleDimension.pos.y;
	element.angleDimension.note.midC.x			= element.angleDimension.note.begC.x + 1.0;
	element.angleDimension.note.midC.y			= element.angleDimension.note.begC.y + 2.0;
	element.angleDimension.note.endC.x			= element.angleDimension.note.midC.x + 5.0;
	element.angleDimension.note.endC.y			= element.angleDimension.note.midC.y;

	err = ACAPI_Element_Create (&element, &memo);
	if (err != NoError)
		ErrorBeep ("ACAPI_Element_Create (Static Angle Dimension)", err);

	ACAPI_DisposeElemMemoHdls (&memo);
}

// -----------------------------------------------------------------------------
// Create a worksheet element
// -----------------------------------------------------------------------------
void	Do_CreateWorksheet (void)
{
	API_Element			element;
	API_ElementMemo		memo;
	API_SubElemMemoMask	marker;
	API_AddParType		**markAddPars;
	API_Coord			c2;
	GSErrCode			err = NoError;

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&marker, sizeof (API_SubElemMemoMask));

	element.header.typeID = API_WorksheetID;
	marker.subType = (API_SubElemMemoMaskType) (APISubElemMemoMask_MainMarker | APISubElemMemoMask_NoParams);

	err = ACAPI_Element_GetDefaultsExt (&element, &memo, 1UL, &marker);
	if (err != NoError) {
		ACAPI_DisposeElemMemoHdls (&memo);
		return;
	}

	if (!Do_LibpartSearchByUnID (API_WorksheetID, &markAddPars, &marker.subElem.object.libInd)) {
		ACAPI_DisposeElemMemoHdls (&memo);
		return;
	}

	marker.memo.params = markAddPars;

	if (!ClickAPoint ("Place the worksheet", &c2)) {
		ACAPI_DisposeElemMemoHdls (&memo);
		ACAPI_DisposeElemMemoHdls (&marker.memo);
		return;
	}

	usprintf (element.worksheet.detailName, sizeof (element.worksheet.detailName), "Worksheet_API (%.3f,%.3f)", c2.x, c2.y);
	usprintf (element.worksheet.detailIdStr, sizeof (element.worksheet.detailIdStr), "IDstring_API");
	element.worksheet.pos = c2;
	element.worksheet.poly.nCoords = 5;
	element.worksheet.poly.nSubPolys = 1;
	element.worksheet.poly.nArcs = 0;
	memo.coords = (API_Coord**) BMAllocateHandle ((element.worksheet.poly.nCoords + 1) * sizeof (API_Coord), ALLOCATE_CLEAR, 0);
	memo.pends  = (Int32**) BMAllocateHandle ((element.worksheet.poly.nSubPolys + 1) * sizeof (Int32), ALLOCATE_CLEAR, 0);
	if (memo.coords != NULL && memo.pends != NULL) {
		(*memo.coords)[1].x = c2.x - 1.0;
		(*memo.coords)[1].y = c2.y;
		(*memo.coords)[2].x = c2.x;
		(*memo.coords)[2].y = c2.y - 1.0;
		(*memo.coords)[3].x = c2.x + 1.0;
		(*memo.coords)[3].y = c2.y;
		(*memo.coords)[4].x = c2.x;
		(*memo.coords)[4].y = c2.y + 1.0;
		(*memo.coords)[5].x = (*memo.coords)[1].x;
		(*memo.coords)[5].y = (*memo.coords)[1].y;

		(*memo.pends)[0] = 0;
		(*memo.pends)[1] = element.worksheet.poly.nCoords;
	}

	marker.subElem.object.pen = 3;
	marker.subElem.object.useObjPens = true;
	marker.subElem.object.pos.x = c2.x + 1.5;
	marker.subElem.object.pos.y = c2.y + 1.0;

	marker.subType = APISubElemMemoMask_MainMarker;
	err = ACAPI_Element_CreateExt (&element, &memo, 1UL, &marker);
	if (err != NoError)
		ErrorBeep ("ACAPI_Element_CreateExt (Worksheet)", err);

	ACAPI_DisposeElemMemoHdls (&memo);
	ACAPI_DisposeElemMemoHdls (&marker.memo);

	return;
}		// Do_CreateWorksheet


// -----------------------------------------------------------------------------
// Create an independent Label with default settings
//   - the label is created with the default settings
//   - try it both in text and symbol mode
//   - the empty string in text mode will be substituted by a predefined string
// -----------------------------------------------------------------------------
void	Do_CreateLabel (void)
{
	API_Coord c;
	if (!ClickAPoint ("Click label reference point", &c))
		return;

	GSErrCode			err;
	API_Element			element;
	API_ElementMemo		memo;

	BNZeroMemory (&element, sizeof (API_Element));
	element.header.typeID = API_LabelID;

	err = ACAPI_Element_GetDefaults (&element, &memo);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetDefaults", err);
		return;
	}

	element.label.parent		= APINULLGuid;
	element.label.begC			= c;
	element.label.midC.x		= c.x + 1.0;
	element.label.midC.y		= c.y + 0.5;
	element.label.endC.x		= c.x + 3.0;
	element.label.endC.y		= c.y + 0.5;

	if (element.label.labelClass == APILblClass_Text) {
		ReplaceEmptyTextWithPredefined (memo.textContent);
		element.label.u.text.nonBreaking = true;
	}

	err = ACAPI_Element_Create (&element, &memo);
	if (err != NoError)
		ErrorBeep ("ACAPI_Element_Create (Label)", err);

	ACAPI_DisposeElemMemoHdls (&memo);
}		// Do_CreateLabel


// -----------------------------------------------------------------------------
// Create an associative Label with default settings
//   - the label is created with the default settings
//   - try it both in text and symbol mode
//   - the empty string in text mode will be substituted by a predefined string
// -----------------------------------------------------------------------------
void	Do_CreateLabel_Associative (void)
{
	API_ElemTypeID	typeID;
	API_Guid		guid;
	API_Coord3D		begC3D;
	if (!ClickAnElem ("Click the element to assign the Label to. The clicked point will be the \"begC\" of the Label.", API_ZombieElemID, NULL, &typeID, &guid, &begC3D)) {
		WriteReport_Alert ("No element was clicked.");
		return;
	}

	GSErrCode		err;
	API_Element		element;
	API_ElementMemo	memo;

	BNZeroMemory (&element, sizeof (API_Element));
	element.header.typeID    = API_LabelID;
	element.label.parentType = typeID;

	err = ACAPI_Element_GetDefaults (&element, &memo);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetDefaults", err);
		return;
	}

	element.label.begC.x = begC3D.x;
	element.label.begC.y = begC3D.y;

	if (ClickAPoint ("Click the \"midC\" of the Label. (Esc = default position)", &element.label.midC) &&
		ClickAPoint ("Click the \"endC\" of the Label. (Esc = default position)", &element.label.endC))
		element.label.createAtDefaultPosition = false;
	else
		element.label.createAtDefaultPosition = true;

	element.label.parent = guid;

	if (element.label.labelClass == APILblClass_Text) {
		ReplaceEmptyTextWithPredefined (memo.textContent);
		element.label.u.text.nonBreaking = true;
	}

	err = ACAPI_Element_Create (&element, &memo);
	if (err != NoError)
		ErrorBeep ("ACAPI_Element_Create (Associative Label)", err);

	ACAPI_DisposeElemMemoHdls (&memo);
}		// Do_CreateLabel_Associative


// -----------------------------------------------------------------------------
// Return actual story number
// -----------------------------------------------------------------------------
short GetActStory ()
{
	API_StoryInfo	storyInfo;
	GSErrCode		err;
	short			actStory = 0;

	err = ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo, NULL);
	if (err == NoError) {
		actStory = storyInfo.actStory;
		BMKillHandle ((GSHandle*) &storyInfo.data);
	}
	return actStory;
}

// -----------------------------------------------------------------------------
// Create a zone with the given geometry
// -----------------------------------------------------------------------------
GSErrCode	CreateZoneWithGeometry (const API_Coord&		pos,
									const API_Polygon&		poly,
									const API_Coord*		coords,
									const Int32*			pends,
									const API_PolyArc*		parcs,
									const double			height,
									const API_Coord&		posText,
									const GS::UniString&	roomName,
									const GS::UniString&	roomNoStr,
									API_Guid*				newZoneGuid)
{
	API_Element			element;
	API_ElementMemo		memo;
	GSErrCode			err;

	BNZeroMemory (&element, sizeof (element));
	BNZeroMemory (&memo, sizeof (memo));

	element.header.typeID = API_ZoneID;

	err = ACAPI_Element_GetDefaults (&element, &memo);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetDefaults (Zone)", err);
		return err;
	}

	element.header.typeID	= API_ZoneID;
	element.header.floorInd	= GetActStory ();
	element.zone.manual		= true;
	element.zone.poly		= poly;
	memo.coords	= reinterpret_cast<API_Coord**>		(BMAllocateHandle ((poly.nCoords + 1) * sizeof (API_Coord), ALLOCATE_CLEAR, 0));
	memo.pends	= reinterpret_cast<Int32**>			(BMAllocateHandle ((poly.nSubPolys + 1) * sizeof (Int32), ALLOCATE_CLEAR, 0));
	memo.parcs	= reinterpret_cast<API_PolyArc**>	(BMAllocateHandle (poly.nArcs * sizeof (API_PolyArc), ALLOCATE_CLEAR, 0));
	if (memo.coords && memo.pends && memo.parcs) {
		for (Int32 i = 1; i <= poly.nCoords; i++) {
			(*memo.coords)[i].x = coords[i].x + pos.x;
			(*memo.coords)[i].y = coords[i].y + pos.y;

		}

		for (Int32 i = 0; i <= poly.nSubPolys; i++) {
			(*memo.pends)[i] = pends[i];
		}

		for (Int32 i = 0; i < poly.nArcs; i++) {
			(*memo.parcs)[i].begIndex  = parcs[i].begIndex;
			(*memo.parcs)[i].endIndex  = parcs[i].endIndex;
			(*memo.parcs)[i].arcAngle  = parcs[i].arcAngle;
		}

		GS::snuprintf (element.zone.roomName, sizeof (element.zone.roomName), roomName.ToUStr ());
		GS::snuprintf (element.zone.roomNoStr, sizeof (element.zone.roomNoStr), roomNoStr.ToUStr ());
		element.zone.pos.x		= posText.x + pos.x;
		element.zone.pos.y		= posText.y + pos.y;

		element.zone.roomHeight	= height;

		err = ACAPI_Element_Create (&element, &memo);
		if (err != NoError) {
			ErrorBeep ("ACAPI_Element_Create (Zone)", err);
			*newZoneGuid = APINULLGuid;
		} else
			*newZoneGuid = element.header.guid;
	}

	ACAPI_DisposeElemMemoHdls (&memo);

	return err;
}

// -----------------------------------------------------------------------------
// Create an automatic zone with the default settings
// -----------------------------------------------------------------------------
GSErrCode	CreateAutoZone (const API_Coord&		pos,
							const GS::UniString&	roomName,
							const GS::UniString&	roomNoStr,
							API_Guid*				newZoneGuid)
{
	API_Element			element;
	API_ElementMemo		memo;
	GSErrCode			err;

	GS::Array<API_Guid> zoneList;
	ACAPI_Element_GetElemList (API_ZoneID, &zoneList);

	BNZeroMemory (&element, sizeof (element));
	BNZeroMemory (&memo, sizeof (memo));

	element.header.typeID = API_ZoneID;

	err = ACAPI_Element_GetDefaults (&element, &memo);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetDefaults (Zone)", err);
		return err;
	}

	element.header.typeID = API_ZoneID;
	element.zone.catInd   = 1;
	element.zone.manual   = false;

	GS::snuprintf (element.zone.roomName, sizeof (element.zone.roomName), roomName.ToUStr ());
	GS::snuprintf (element.zone.roomNoStr, sizeof (element.zone.roomNoStr), roomNoStr.ToUStr ());

	element.zone.pos.x = pos.x;
	element.zone.pos.y = pos.y;
	element.zone.refPos.x = pos.x;
	element.zone.refPos.y = pos.y;

	err = ACAPI_Element_Create (&element, &memo);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Create (Zone)", err);
		*newZoneGuid = APINULLGuid;
	} else
		*newZoneGuid = element.header.guid;

	ACAPI_DisposeElemMemoHdls (&memo);

	return err;
}		// CreateAutoZone

// -----------------------------------------------------------------------------
// Create a zone with the default settings to the given point
//   - geometryID:
//        0 = Automatic
//        1 = Octagon
//        2 = Square with hole and arcs
// -----------------------------------------------------------------------------
void Do_CreateZone (short geometryID)
{
	GSErrCode			err;
	API_Coord			pos;

	if (!ClickAPoint ("Click zone reference point", &pos))
		return;

	API_Polygon		poly;
	API_Coord**		coords = NULL;
	Int32**			pends = NULL;
	API_PolyArc**	parcs = NULL;
	double			height;

	GS::UniString		roomName;
	GS::Array<API_Guid> zoneList;
	ACAPI_Element_GetElemList (API_ZoneID, &zoneList);
	GS::UniString		roomNoStr	= GS::UniString::Printf ("%lu", (GS::ULongForStdio) zoneList.GetSize () + 1);
	API_Coord			posText;

	API_Guid		newZoneGuid = APINULLGuid;

	// Create the given geometry or automatic
	switch (geometryID) {
		case 0:
			roomName = GS::UniString::Printf ("AutoZone_%lu", (GS::ULongForStdio) zoneList.GetSize () + 1);

			err = CreateAutoZone (pos, roomName, roomNoStr, &newZoneGuid);
			if (err != NoError)
				ErrorBeep ("CreateAutoZone", err);
			break;

		case 1:
			roomName		= GS::UniString ("Octagon");
			poly.nCoords	= 9;
			poly.nSubPolys	= 1;
			poly.nArcs		= 0;

			coords	= reinterpret_cast<API_Coord**>		(BMAllocateHandle ((poly.nCoords + 1) * sizeof (API_Coord), ALLOCATE_CLEAR, 0));
			pends	= reinterpret_cast<Int32**>			(BMAllocateHandle ((poly.nSubPolys + 1) * sizeof (Int32), ALLOCATE_CLEAR, 0));
			parcs	= reinterpret_cast<API_PolyArc**>	(BMAllocateHandle (poly.nArcs * sizeof (API_PolyArc), ALLOCATE_CLEAR, 0));
			if (coords && pends && parcs) {
				Int32 i = 1;
				// 9 coords, because the last one must be equal the first one
				(*coords)[i].x = 0.0; (*coords)[i].y = 0.0; i++;
				(*coords)[i].x = 4.0; (*coords)[i].y = 0.0; i++;
				(*coords)[i].x = 6.0; (*coords)[i].y = 2.0; i++;
				(*coords)[i].x = 6.0; (*coords)[i].y = 6.0; i++;
				(*coords)[i].x = 4.0; (*coords)[i].y = 8.0; i++;
				(*coords)[i].x = 0.0; (*coords)[i].y = 8.0; i++;
				(*coords)[i].x =-2.0; (*coords)[i].y = 6.0; i++;
				(*coords)[i].x =-2.0; (*coords)[i].y = 2.0; i++;
				(*coords)[i].x = 0.0; (*coords)[i].y = 0.0; (*pends)[1] = i; i++;

				height = 1.5;

				posText.x = 2.0; posText.y = 4.0;

				err = CreateZoneWithGeometry (pos, poly, *coords, *pends, *parcs, height, posText, roomName, roomNoStr, &newZoneGuid);
				if (err != NoError)
					ErrorBeep ("CreateZoneWithGeometry (Octagon)", err);
			} else {
				WriteReport_Alert ("BMAllocateHandle failed");
			}
			break;

		case 2:
		default:
			roomName		= GS::UniString ("With hole and arcs");
			poly.nCoords	= 10;
			poly.nSubPolys	= 2;
			poly.nArcs		= 2;

			coords	= reinterpret_cast<API_Coord**>		(BMAllocateHandle ((poly.nCoords + 1) * sizeof (API_Coord), ALLOCATE_CLEAR, 0));
			pends	= reinterpret_cast<Int32**>			(BMAllocateHandle ((poly.nSubPolys + 1) * sizeof (Int32), ALLOCATE_CLEAR, 0));
			parcs	= reinterpret_cast<API_PolyArc**>	(BMAllocateHandle (poly.nArcs * sizeof (API_PolyArc), ALLOCATE_CLEAR, 0));
			if (coords && pends && parcs) {
				Int32 i = 1;
				// Contour square polygon
				(*coords)[i].x = 0.0; (*coords)[i].y = 0.0; i++;
				(*coords)[i].x = 4.0; (*coords)[i].y = 0.0; i++;
				(*coords)[i].x = 4.0; (*coords)[i].y = 4.0; i++;
				(*coords)[i].x = 0.0; (*coords)[i].y = 4.0; i++;
				(*coords)[i].x = 0.0; (*coords)[i].y = 0.0; (*pends)[1] = i; i++;

				// Hole polygon
				(*coords)[i].x = 1.0; (*coords)[i].y = 1.0; i++;
				(*coords)[i].x = 3.0; (*coords)[i].y = 1.0; i++;
				(*coords)[i].x = 3.0; (*coords)[i].y = 3.0; i++;
				(*coords)[i].x = 1.0; (*coords)[i].y = 3.0; i++;
				(*coords)[i].x = 1.0; (*coords)[i].y = 1.0; (*pends)[2] = i; i++;

				i = 0;
				// Two arcs:
				// between the 1. and 2.
				(*parcs)[i].begIndex  = 1; (*parcs)[i].endIndex  = 2; (*parcs)[i].arcAngle  = 90.0 * DEGRAD; i++;
				// between the 7. and 8.
				(*parcs)[i].begIndex  = 7; (*parcs)[i].endIndex  = 8; (*parcs)[i].arcAngle  = 45.0 * DEGRAD; i++;

				height = 2.0;

				posText.x = 1.0; posText.y = 1.0;

				err = CreateZoneWithGeometry (pos, poly, *coords, *pends, *parcs, height, posText, roomName, roomNoStr, &newZoneGuid);
				if (err != NoError)
					ErrorBeep ("CreateZoneWithGeometry (Square with hole and arcs)", err);
			} else {
				WriteReport_Alert ("BMAllocateHandle failed");
			}
			break;
	}

	if (coords)
		BMKillHandle ((GSHandle*) &coords);
	if (pends)
		BMKillHandle ((GSHandle*) &pends);
	if (parcs)
		BMKillHandle ((GSHandle*) &parcs);

}		// Do_CreateZone


// -----------------------------------------------------------------------------
// Create a camera set
//   - same type as the active path
//   - camera set is activated
//   - check Camera Settings if it is open
// -----------------------------------------------------------------------------
void	Do_CreateCamset (void)
{
	API_Element		element;
	API_Guid		actGuid;
	GSErrCode		err;

	SearchActiveCamset (&actGuid, NULL);
	if (actGuid == APINULLGuid)
		return;

	BNZeroMemory (&element, sizeof (API_Element));

	element.header.typeID  = API_CamSetID;

	GS::ucscpy (element.camset.name, L("Perspective Camset from API"));
	element.camset.perspPars.openedPath = true;
	element.camset.perspPars.bezierPath = false;
	element.camset.perspPars.smoothTarget = true;
	element.camset.perspPars.pen = 15;
	element.camset.perspPars.inFrames = 7;

	element.camset.active = true;

	err = ACAPI_Element_Create (&element, NULL);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Create (camera set)", err);
		return;
	}

	return;
}		// Do_CreateCamset


// -----------------------------------------------------------------------------
// Create a perspective camera
//   - camera click: insert after
//   - empty click: create a new camera set for the camera
// -----------------------------------------------------------------------------
void	Do_CreatePerspCam (void)
{
	API_ElemTypeID	typeID;
	API_Guid		prevGuid, camSetGuid;
	API_Coord		pos, target;
	double			cameraZ, targetZ;
	API_Guid		guid;
	API_Coord3D		pos3D;
	GSErrCode		err;

	err = NoError;

	if (!ClickAnElem ("Click a camera to insert a new camera after or click a place to create a new path/camera", API_ZombieElemID, NULL, &typeID, &guid, &pos3D)) {
		WriteReport_Alert ("No element was clicked");
		return;
	}

	API_Element element;
	BNZeroMemory (&element, sizeof (API_Element));

	if (typeID == API_CameraID) {
		element.header.guid = guid;
		ACAPI_Element_Get (&element);

		prevGuid   = element.header.guid;
		camSetGuid = element.camera.camSetGuid;

		pos    = element.camera.perspCam.persp.pos;
		target = element.camera.perspCam.persp.target;
		pos.x += 5.0;
		target.x += 3.0;

		cameraZ = element.camera.perspCam.persp.cameraZ + 1.0;
		targetZ = element.camera.perspCam.persp.targetZ + 1.0;
	} else {
		element.header.typeID  = API_CamSetID;

		GS::ucscpy (element.camset.name, L("Perspective Camset from API"));

		element.camset.perspPars.openedPath = true;
		element.camset.perspPars.bezierPath = false;
		element.camset.perspPars.smoothTarget = false;
		element.camset.perspPars.pen = 15;
		element.camset.perspPars.inFrames = 7;

		element.camset.active = true;

		err = ACAPI_Element_Create (&element, NULL);
		if (err != NoError) {
			ErrorBeep ("ACAPI_Element_Create (persp. camset)", err);
			return;
		}

		prevGuid = APINULLGuid;
		camSetGuid = element.header.guid;

		pos.x = pos3D.x;
		pos.y = pos3D.y;
		target.x = pos.x;
		target.y = pos.y + 3.0;

		cameraZ = 0.5;
		targetZ = 1.0;
	}

	element.header.typeID  = API_CameraID;

	err = ACAPI_Element_GetDefaults (&element, NULL);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetDefaults", err);
		return;
	}

	element.camera.perspCam.persp.viewCone = 82.0 * DEGRAD;
	element.camera.perspCam.persp.pos = pos;
	element.camera.perspCam.persp.target = target;
	element.camera.perspCam.persp.cameraZ = cameraZ;
	element.camera.perspCam.persp.targetZ = targetZ;

	element.camera.perspCam.waitFrames = 6;
	element.camera.perspCam.pen = 33;

	element.camera.perspCam.prevCam = prevGuid;
	element.camera.perspCam.nextCam = APINULLGuid;
	element.camera.perspCam.lenPrev = 2.0;
	element.camera.perspCam.lenNext = 1.0;
	element.camera.perspCam.dirAng = PI / 2.0;

	element.camera.camSetGuid = camSetGuid;

	err = ACAPI_Element_Create (&element, NULL);
	if (err != NoError)
		ErrorBeep ("ACAPI_Element_Create (persp. camera)", err);
}		// Do_CreatePerspCam


// -----------------------------------------------------------------------------
// Place a picture (TIFF)
//   - it must be called "picture1.tif" and located at: "D:\"
// -----------------------------------------------------------------------------
void	Do_CreatePicture (void)
{
	API_Element			element;
	API_ElementMemo		memo;
	API_Coord			c;
	IO::Location		location;
	USize				nBytes;
	USize				rwnum = 0;
	Int32				hSize, vSize, hRes, vRes, pixelBitNum;
	GSErrCode			err;

	BNZeroMemory (&element, sizeof (element));
	BNZeroMemory (&memo, sizeof (memo));

	if (!ClickAPoint ("Click picture reference point", &c))
		return;

	element.header.typeID = API_PictureID;
	err = ACAPI_Element_GetDefaults (&element, NULL);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetDefaults (picture)", err);
		return;
	}

	location.Set ("D:\\picture1.tif");

	err = GX::ImageBase::GetFileInfo (location, &hSize, &vSize, &hRes, &vRes, &pixelBitNum);
	if (err != NoError) {
		ErrorBeep ("GX::ImageBase::GetFileInfo", err);
		return;
	}

	element.picture.usePixelSize = true;
	element.picture.mirrored = false;
	element.picture.destBox.xMin = c.x;
	element.picture.destBox.yMin = c.y;
	element.picture.pixelSizeX = (short) (hSize);
	element.picture.pixelSizeY = (short) (vSize);
	element.picture.rotAngle = 0.0;
	element.picture.anchorPoint = APIAnc_LB;
	element.picture.storageFormat = APIPictForm_TIFF;
	element.picture.transparent = false;

	GS::ucscpy (element.picture.pictName, L("First try"));

	IO::File file (location);
	file.Open (IO::File::ReadMode);

	if (file.GetDataLength (&nBytes) == NoError) {
		memo.pictHdl = BMAllocateHandle (nBytes, ALLOCATE_CLEAR, 0);
		err = BMError ();
		if (err == NoError) {
			DBPrintf ("Tiff file size: %d", nBytes);

			file.ReadBin (*memo.pictHdl, nBytes, &rwnum);
			if (rwnum != nBytes)
				err = ErrIO;
		}
	} else
		err = ErrIO;

	file.Close ();


	if (err == NoError) {
		err = ACAPI_Element_Create (&element, &memo);
		if (err != NoError)
			ErrorBeep ("ACAPI_Element_Create (picture)", err);
	}

	if (memo.pictHdl != NULL) {
		ACAPI_DisposeElemMemoHdls (&memo);
	}

	return;
}		// Do_CreatePicture


// -----------------------------------------------------------------------------
// Create a group from lines with user input
// -----------------------------------------------------------------------------
void	Do_CreateGroupedLines (void)
{
	API_Coord			c;
	API_GetPolyType		polyInfo;
	API_Element			element;
	Int32				i;
	GS::Array<API_Guid> group;
	GSErrCode			err;

	if (!ClickAPoint ("Enter the first node of the polyline", &c))
		return;

	BNZeroMemory (&polyInfo, sizeof (API_GetPolyType));
	strcpy (polyInfo.prompt, "Enter the next node of the polyline");
	polyInfo.method = APIPolyGetMethod_Polyline;
	polyInfo.startCoord.x = c.x;
	polyInfo.startCoord.y = c.y;

	err = ACAPI_Interface (APIIo_GetPolyID, &polyInfo, NULL);
	if (err != NoError)
		return;

	BNZeroMemory (&element, sizeof (API_Element));
	element.header.typeID = API_LineID;
	ACAPI_Element_GetDefaults (&element, NULL);

	for (i = 1; i < polyInfo.nCoords; i++) {
		element.line.begC = (*polyInfo.coords)[i];
		element.line.endC = (*polyInfo.coords)[i + 1];

		err = ACAPI_Element_Create (&element, NULL);
		if (err != NoError)
			ErrorBeep ("ACAPI_Element_Create (line)", err);
		else
			group.Push (element.header.guid);
	}

	API_Guid groupGuid = APINULLGuid;
	err = ACAPI_ElementGroup_Create (group, &groupGuid);
	if (err != NoError) {
		ErrorBeep ("ACAPI_ElementGroup_Create (group, groupGuid)", err);
	} else {
		GSHandle textHdl;
		const char* text = "Grouped lines test.";
		textHdl = BMAllocateHandle (Strlen32 (text) + 1, ALLOCATE_CLEAR, 0);
		if (DBVERIFY (textHdl != NULL)) {
			CHCopyC (text, *textHdl);

			API_UserData userData;
			BNZeroMemory (&userData, sizeof (API_UserData));

			userData.dataVersion	= 1;
			userData.platformSign	= GS::Act_Platform_Sign;
			userData.dataHdl		= textHdl;

			err = ACAPI_ElementGroup_SetUserData (groupGuid, &userData);
			if (err != NoError)
				ErrorBeep ("ACAPI_ElementGroup_SetUserData (groupGuid, &userData)", err);
		}

		BMKillHandle ((GSHandle*) &textHdl);
	}

	BMKillHandle ((GSHandle*) &polyInfo.coords);
	BMKillHandle ((GSHandle*) &polyInfo.parcs);

	return;
}		// Do_CreateGroupedLines


// -----------------------------------------------------------------------------
// Create a cutting plane
//
// -----------------------------------------------------------------------------
void	Do_CreateCutPlane (void)
{
	API_Element			element;
	API_ElementMemo		memo;
	API_SubElemMemoMask	marker;
	API_Coord			*coords;
	GSErrCode			err;

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&marker, sizeof (API_SubElemMemoMask));

	element.header.typeID = API_CutPlaneID;
	marker.subType = (API_SubElemMemoMaskType) (APISubElemMemoMask_MainMarker);

	err = ACAPI_Element_GetDefaultsExt (&element, &memo, 1UL, &marker);
	if (err != NoError)
		return;

	marker.subElem.object.useObjPens = true;
	GS::ucscpy (element.cutPlane.segment.cutPlName, L("Section_API"));
	GS::ucscpy (element.cutPlane.segment.cutPlIdStr, L("IDstring_API"));

	element.cutPlane.segment.depth += 1;
	element.cutPlane.segment.leftDir = (!element.cutPlane.segment.leftDir);

	// Get the first point
	API_GetPointType pointInfo;
	BNZeroMemory (&pointInfo, sizeof (API_GetPointType));
	CHCopyC ("Click the first point of your cutplane", pointInfo.prompt);
	ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);

	// Get the line
	API_GetLineType lineInfo;
	BNZeroMemory (&lineInfo, sizeof (API_GetLineType));
	CHCopyC ("Click the second point of your cutplane", lineInfo.prompt);
	lineInfo.startCoord = pointInfo.pos;
	ACAPI_Interface (APIIo_GetLineID, &lineInfo, NULL);

	coords = (API_Coord*) BMpAll (2 * sizeof (API_Coord));

	coords[0].x = lineInfo.startCoord.x;
	coords[0].y = lineInfo.startCoord.y;
	coords[1].x = lineInfo.pos.x;
	coords[1].y = lineInfo.pos.y;

	memo.sectionLineCoords = coords;
	element.cutPlane.segment.nCoord = 2;
	marker.subType = APISubElemMemoMask_MainMarker;
	element.cutPlane.linkData.sourceMarker = true;

	err = ACAPI_Element_CreateExt (&element, &memo, 1UL, &marker);

	ACAPI_DisposeElemMemoHdls (&memo);
	ACAPI_DisposeElemMemoHdls (&marker.memo);

	return;
}		// Do_CreateCutPlane


// -----------------------------------------------------------------------------
// Create an interior elevation
//
// -----------------------------------------------------------------------------
void	Do_CreateInteriorElevation (void)
{
	API_Element			element;
	API_ElementMemo		memo;
	API_SubElemMemoMask	marker;
	API_AddParType		**markAddPars;
	double				a, b;
	Int32				addParNum;
	GSErrCode			err;

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&marker, sizeof (API_SubElemMemoMask));

	element.header.typeID = API_InteriorElevationID;
	marker.subType = (API_SubElemMemoMaskType) (APISubElemMemoMask_MainMarker | APISubElemMemoMask_NoParams);

	err = ACAPI_Element_GetDefaultsExt (&element, &memo, 1UL, &marker);
	if (err != NoError)
		return;

	err = ACAPI_LibPart_GetParams (marker.subElem.object.libInd, &a, &b, &addParNum, &markAddPars);
	if (err != NoError)
		return;

	marker.memo.params = markAddPars;
	marker.subElem.object.useObjPens = true;
	GS::ucscpy (element.interiorElevation.segment.cutPlName, L("IntElev_API"));
	GS::ucscpy (element.interiorElevation.segment.cutPlIdStr, L("IDstring_API"));

	element.interiorElevation.segment.depth   = 0.75;
#if defined (__GNUC__)
	element.interiorElevation.segment.leftDir = (GS::Bool8) (random () & 0x01);
#else
	element.interiorElevation.segment.leftDir = (GS::Bool8) (rand () & 0x01);
#endif

	// Get the first point
	API_GetPointType pointInfo;
	BNZeroMemory (&pointInfo, sizeof (API_GetPointType));
	CHCopyC ("Click the first point of your interior elevation", pointInfo.prompt);
	ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);

	// Get the polyline
	API_GetPolyType	polyInfo;
	BNZeroMemory (&polyInfo, sizeof (polyInfo));
	CHCopyC ("Click the points of your interior elevation", polyInfo.prompt);
	polyInfo.method = APIPolyGetMethod_Polyline;
	polyInfo.startCoord = pointInfo.pos;
	if (ACAPI_Interface (APIIo_GetPolyID, &polyInfo, NULL) == NoError) {
		ULong	lastInd = polyInfo.nCoords;

		if (!polyInfo.polylineWas)
			lastInd--;

		element.interiorElevation.segment.nCoord = (short) lastInd;
		memo.sectionLineCoords = reinterpret_cast<API_Coord*> (BMAllocatePtr (lastInd * sizeof (API_Coord), ALLOCATE_CLEAR, 0));
		memo.segments = reinterpret_cast<API_SectionSegment*> (BMAllocatePtr ((lastInd - 1) * sizeof (API_SectionSegment), ALLOCATE_CLEAR, 0));
		if (memo.sectionLineCoords != NULL && memo.segments != NULL) {
			for (ULong ii = 1; ii <= lastInd; ii++) {
				memo.sectionLineCoords[ii-1] = (*polyInfo.coords)[ii];
			}
			for (ULong i = 0; i < lastInd-1; i++) {
				memo.segments[i].horizontalOffset = 0.5;
			}

			marker.subType = APISubElemMemoMask_MainMarker;
			err = ACAPI_Element_CreateExt (&element, &memo, 1UL, &marker);
		} else
			err = APIERR_MEMFULL;

		BMKillHandle (reinterpret_cast<GSHandle*> (&polyInfo.coords));
		BMKillHandle (reinterpret_cast<GSHandle*> (&polyInfo.parcs));
	}

	ACAPI_DisposeElemMemoHdls (&memo);
	ACAPI_DisposeElemMemoHdls (&marker.memo);
}	// Do_CreateInteriorElevation


// -----------------------------------------------------------------------------
// Create a window in the clicked wall
//	- does not work on polygonal walls
//	- try to place both an empty opening and a real window object
// -----------------------------------------------------------------------------
void	Do_CreateWindow (void)
{
	API_ElemTypeID		typeID;
	API_Guid			wallGuid;
	API_Element			element;
	API_ElementMemo		memo;
	API_SubElemMemoMask	marker;
	API_Coord3D			c3;
	API_Coord			c2, begC;
	API_AddParType		**markAddPars;
	GSErrCode			err = NoError;

	if (!ClickAnElem ("Click a wall to place a window", API_WallID, NULL, &typeID, &wallGuid, &c3)) {
		WriteReport_Alert ("Please click a wall");
		return;
	}

	BNZeroMemory (&element, sizeof (API_Element));
	element.header.typeID = API_WallID;
	element.header.guid   = wallGuid;
	err = ACAPI_Element_Get (&element);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Get", err);
		return;
	}

	if (element.wall.type == APIWtyp_Poly) {
		WriteReport_Alert ("No way to put openings into polygonal walls");
		return;
	}

	begC = element.wall.begC;		// arched walls should be handled differently

	c2.x = c3.x;
	c2.y = c3.y;

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&marker, sizeof (API_SubElemMemoMask));

	element.header.typeID = API_WindowID;
	marker.subType = APISubElemMemoMask_MainMarker;

	err = ACAPI_Element_GetDefaultsExt (&element, &memo, 1UL, &marker);
	if (err != NoError) {
		ACAPI_DisposeElemMemoHdls (&memo);
		ACAPI_DisposeElemMemoHdls (&marker.memo);
		return;
	}

	if (!Do_LibpartSearchByUnID (API_WindowID, &markAddPars, &marker.subElem.object.libInd)) {
		ACAPI_DisposeElemMemoHdls (&memo);
		ACAPI_DisposeElemMemoHdls (&marker.memo);
		return;
	}

	marker.memo.params = markAddPars;
	marker.subElem.object.pen = 166;
	marker.subElem.object.useObjPens = true;

	element.window.objLoc = DistCPtr (&c2, &begC);
	element.window.owner = wallGuid;
	err = ACAPI_Element_CreateExt (&element, &memo, 1UL, &marker);
	if (err != NoError)
		ErrorBeep ("ACAPI_Element_CreateExt", err);

	ACAPI_DisposeElemMemoHdls (&memo);
	ACAPI_DisposeElemMemoHdls (&marker.memo);

	return;
}		// Do_CreateWindow


// -----------------------------------------------------------------------------
// Create a skylight in the clicked roof
// -----------------------------------------------------------------------------
void	Do_CreateSkylight (void)
{
	API_ElemTypeID		typeID;
	API_Guid			guid;
	API_Element			element;
	API_ElementMemo		memo;
	API_Coord3D			c3;
	GSErrCode			err = NoError;

	if (!ClickAnElem ("Click on a roof or a shell to place a skylight", API_ZombieElemID, NULL, &typeID, &guid, &c3) ||
		(typeID != API_RoofID && typeID != API_ShellID))
	{
		WriteReport_Alert ("Please click a roof or a shell element");
		return;
	}

	API_Coord c2;
	c2.x = c3.x;
	c2.y = c3.y;

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

	element.header.typeID = API_SkylightID;

	err = ACAPI_Element_GetDefaults (&element, &memo);
	if (err != NoError) {
		ACAPI_DisposeElemMemoHdls (&memo);
		return;
	}

	element.skylight.owner = guid;
	element.skylight.anchorPosition = c2;
	element.skylight.anchorLevel = 2.0;

	err = ACAPI_Element_Create (&element, &memo);
	if (err != NoError)
		ErrorBeep ("ACAPI_Element_Create", err);

	ACAPI_DisposeElemMemoHdls (&memo);

	return;
}		// Do_CreateSkylight


// -----------------------------------------------------------------------------
// Create a Multi-plane Roof
// -----------------------------------------------------------------------------
void	Do_CreatePolyRoof (void)
{
	API_Coord centerPoint;
	if (!ClickAPoint ("Click a center point to create a Roof", &centerPoint))
		return;

	API_Element element;
	BNZeroMemory (&element, sizeof (API_Element));
	element.header.typeID = API_RoofID;
	element.roof.roofClass = API_PolyRoofID;
	GSErrCode err = ACAPI_Element_GetDefaults (&element, NULL);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetDefaults (roof)", err);
		return;
	}

	API_ElementMemo memo;
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

	// constructing pivot polygon data
	element.roof.u.polyRoof.pivotPolygon.nCoords	= 8;
	element.roof.u.polyRoof.pivotPolygon.nSubPolys	= 2;
	element.roof.u.polyRoof.pivotPolygon.nArcs		= 2;

	memo.additionalPolyCoords = reinterpret_cast<API_Coord**> (BMAllocateHandle ((element.roof.u.polyRoof.pivotPolygon.nCoords + 1) * sizeof (API_Coord), ALLOCATE_CLEAR, 0));
	memo.additionalPolyPends = reinterpret_cast<Int32**> (BMAllocateHandle ((element.roof.u.polyRoof.pivotPolygon.nSubPolys + 1) * sizeof (Int32), ALLOCATE_CLEAR, 0));
	memo.additionalPolyParcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle (element.roof.u.polyRoof.pivotPolygon.nArcs * sizeof (API_PolyArc), ALLOCATE_CLEAR, 0));
	if (memo.additionalPolyCoords == NULL || memo.additionalPolyPends == NULL || memo.additionalPolyParcs == NULL) {
		ErrorBeep ("Not enough memory to create roof pivot polygon data", APIERR_MEMFULL);
		ACAPI_DisposeElemMemoHdls (&memo);
		return;
	}

	(*memo.additionalPolyCoords)[1].x = centerPoint.x + 10.0;
	(*memo.additionalPolyCoords)[1].y = centerPoint.x + 10.0;
	(*memo.additionalPolyCoords)[2].x = centerPoint.x + 10.0;
	(*memo.additionalPolyCoords)[2].y = centerPoint.x - 10.0;
	(*memo.additionalPolyCoords)[3].x = centerPoint.x - 10.0;
	(*memo.additionalPolyCoords)[3].y = centerPoint.x - 10.0;
	(*memo.additionalPolyCoords)[4].x = centerPoint.x - 10.0;
	(*memo.additionalPolyCoords)[4].y = centerPoint.x + 10.0;
	(*memo.additionalPolyCoords)[5] = (*memo.additionalPolyCoords)[1];
	(*memo.additionalPolyPends)[1] = 5;
	(*memo.additionalPolyCoords)[6].x = centerPoint.x + 5.0;
	(*memo.additionalPolyCoords)[6].y = centerPoint.x + 0.0;
	(*memo.additionalPolyCoords)[7].x = centerPoint.x - 5.0;
	(*memo.additionalPolyCoords)[7].y = centerPoint.x + 0.0;
	(*memo.additionalPolyCoords)[8] = (*memo.additionalPolyCoords)[6];
	(*memo.additionalPolyPends)[2] = 8;
	(*memo.additionalPolyParcs)[0].begIndex = 6;							// makes a circle-shaped hole in the pivot polygon
	(*memo.additionalPolyParcs)[0].endIndex = 7;
	(*memo.additionalPolyParcs)[0].arcAngle = PI;
	(*memo.additionalPolyParcs)[1].begIndex = 7;
	(*memo.additionalPolyParcs)[1].endIndex = 8;
	(*memo.additionalPolyParcs)[1].arcAngle = PI;

	// setting eaves overhang and plane levels
	element.roof.u.polyRoof.overHangType = API_OffsetOverhang;		// contourPolygon will be calculated automatically by offsetting the pivot polygon
	element.roof.u.polyRoof.eavesOverHang = 1.0;
	element.roof.u.polyRoof.levelNum = 16;
	for (Int32 i = 0; i < element.roof.u.polyRoof.levelNum; i++) {
		element.roof.u.polyRoof.levelData[i].levelAngle = 5.0 * DEGRAD * (i + 1);
		element.roof.u.polyRoof.levelData[i].levelHeight = 0.05 * (i + 1);
	}

	// create the roof element
	err = ACAPI_Element_Create (&element, &memo);
	if (err != NoError)
		ErrorBeep ("ACAPI_Element_Create (roof)", err);

	ACAPI_DisposeElemMemoHdls (&memo);

	return;
}		// Do_CreatePolyRoof


// -----------------------------------------------------------------------------
// Create an Extruded Shell
// -----------------------------------------------------------------------------
void	Do_CreateExtrudedShell (void)
{
	API_Coord centerPoint;
	if (!ClickAPoint ("Click a center point to create a junction of Extruded Shells", &centerPoint))
		return;

	API_Element element;
	BNZeroMemory (&element, sizeof (API_Element));
	element.header.typeID = API_ShellID;
	element.shell.shellClass = API_ExtrudedShellID;
	GSErrCode err = ACAPI_Element_GetDefaults (&element, NULL);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetDefaults (shell)", err);
		return;
	}

	API_ElementMemo memo;
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

	element.shell.u.extrudedShell.shellShape.nCoords	= 5;
	element.shell.u.extrudedShell.shellShape.nSubPolys	= 1;
	element.shell.u.extrudedShell.shellShape.nArcs		= 1;

	memo.shellShapes[0].coords = (API_Coord**) BMAllocateHandle ((element.shell.u.extrudedShell.shellShape.nCoords + 1) * sizeof (API_Coord), ALLOCATE_CLEAR, 0);
	memo.shellShapes[0].pends = reinterpret_cast<Int32**> (BMAllocateHandle ((element.shell.u.extrudedShell.shellShape.nSubPolys + 1) * sizeof (Int32), ALLOCATE_CLEAR, 0));
	memo.shellShapes[0].parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle (element.shell.u.extrudedShell.shellShape.nArcs * sizeof (API_PolyArc), ALLOCATE_CLEAR, 0));
	memo.shellShapes[0].bodyFlags = (GS::Bool8**) BMAllocateHandle ((element.shell.u.extrudedShell.shellShape.nCoords + 1) * sizeof (GS::Bool8), ALLOCATE_CLEAR, 0);
	if (memo.shellShapes[0].coords == NULL || memo.shellShapes[0].pends == NULL || memo.shellShapes[0].parcs == NULL || memo.shellShapes[0].bodyFlags == NULL) {
		ErrorBeep ("Not enough memory to create shell polygon data", APIERR_MEMFULL);
		ACAPI_DisposeElemMemoHdls (&memo);
		return;
	}

	(*memo.shellShapes[0].coords)[1].x = -2.0;
	(*memo.shellShapes[0].coords)[1].y =  0.0;
	(*memo.shellShapes[0].coords)[2].x = -2.5;
	(*memo.shellShapes[0].coords)[2].y =  2.0;
	(*memo.shellShapes[0].coords)[3].x =  2.5;
	(*memo.shellShapes[0].coords)[3].y =  2.0;
	(*memo.shellShapes[0].coords)[4].x =  2.0;
	(*memo.shellShapes[0].coords)[4].y =  0.0;
	(*memo.shellShapes[0].coords)[5] =  (*memo.shellShapes[0].coords)[1];
	(*memo.shellShapes[0].pends)[1] = element.shell.u.extrudedShell.shellShape.nCoords;
	(*memo.shellShapes[0].parcs)[0].begIndex = 2;
	(*memo.shellShapes[0].parcs)[0].endIndex = 3;
	(*memo.shellShapes[0].parcs)[0].arcAngle = -200 * DEGRAD;
	(*memo.shellShapes[0].bodyFlags)[1] = true;
	(*memo.shellShapes[0].bodyFlags)[2] = true;
	(*memo.shellShapes[0].bodyFlags)[3] = true;
	(*memo.shellShapes[0].bodyFlags)[4] = false;
	(*memo.shellShapes[0].bodyFlags)[5] = (*memo.shellShapes[0].bodyFlags)[1];

	element.shell.u.extrudedShell.begPlaneTilt = 120 * DEGRAD;
	element.shell.u.extrudedShell.endPlaneTilt =  60 * DEGRAD;

	GS::Array<API_Guid> createdShells;

	// create 3 shell elements and connect them
	for (Int32 i = 0; i < 3 && err == NoError; i++) {
		element.shell.u.extrudedShell.begC.x = centerPoint.x - 10.0 * cos (i * 2*PI/3);
		element.shell.u.extrudedShell.begC.y = centerPoint.y - 10.0 * sin (i * 2*PI/3);
		element.shell.u.extrudedShell.extrusionVector.x = 20.0 * cos (i * 2*PI/3);
		element.shell.u.extrudedShell.extrusionVector.y = 20.0 * sin (i * 2*PI/3);
		element.shell.u.extrudedShell.extrusionVector.z = 0.0;
		err = ACAPI_Element_Create (&element, &memo);
		if (err != NoError)
			ErrorBeep ("ACAPI_Element_Create (extruded)", err);
		else
			createdShells.Push (element.header.guid);
	}

	if (err == NoError)
		ACAPI_Element_Trim_Elements (createdShells);

	ACAPI_DisposeElemMemoHdls (&memo);

	return;
}		// Do_CreateExtrudedShell


// -----------------------------------------------------------------------------
// Create a Revolved Shell
// -----------------------------------------------------------------------------
void	Do_CreateRevolvedShell (void)
{
	API_Coord centerPoint;
	if (!ClickAPoint ("Click a center point to create a Revolved Shell", &centerPoint))
		return;

	API_Element element;
	BNZeroMemory (&element, sizeof (API_Element));
	element.header.typeID = API_ShellID;
	element.shell.shellClass = API_RevolvedShellID;
	GSErrCode err = ACAPI_Element_GetDefaults (&element, NULL);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetDefaults (shell)", err);
		return;
	}

	double* tmx = element.shell.basePlane.tmx;
	tmx[ 0]	= 1.0;				tmx[ 4]	= 0.0;				tmx[ 8]	= 0.0;
	tmx[ 1]	= 0.0;				tmx[ 5]	= 0.0;				tmx[ 9]	= 1.0;
	tmx[ 2]	= 0.0;				tmx[ 6]	=-1.0;				tmx[10]	= 0.0;
	tmx[ 3]	= centerPoint.x;	tmx[ 7]	= centerPoint.y;	tmx[11]	= 0.0;

	element.shell.isFlipped = true;

	element.shell.u.revolvedShell.slantAngle			= 0;
	element.shell.u.revolvedShell.revolutionAngle		= 360 * DEGRAD;
	element.shell.u.revolvedShell.distortionAngle		= 90 * DEGRAD;
	element.shell.u.revolvedShell.segmentedSurfaces		= false;
	element.shell.u.revolvedShell.segmentType			= APIShellBase_SegmentsByCircle;
	element.shell.u.revolvedShell.segmentsByCircle		= 36;
	BNZeroMemory (&element.shell.u.revolvedShell.axisBase, sizeof (API_Tranmat));
	element.shell.u.revolvedShell.axisBase.tmx[0]		= 1.0;
	element.shell.u.revolvedShell.axisBase.tmx[6]		= 1.0;
	element.shell.u.revolvedShell.axisBase.tmx[9]		= -1.0;
	element.shell.u.revolvedShell.distortionVector.x	= 0.0;
	element.shell.u.revolvedShell.distortionVector.y	= 0.0;
	element.shell.u.revolvedShell.begAngle				= 0.0;

	API_ElementMemo memo;
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

	// constructing the revolving profile polyline
	element.shell.u.revolvedShell.shellShape.nCoords	= 13;
	element.shell.u.revolvedShell.shellShape.nSubPolys	= 1;
	element.shell.u.revolvedShell.shellShape.nArcs		= 3;

	memo.shellShapes[0].coords = (API_Coord**) BMAllocateHandle ((element.shell.u.revolvedShell.shellShape.nCoords + 1) * sizeof (API_Coord), ALLOCATE_CLEAR, 0);
	memo.shellShapes[0].pends = reinterpret_cast<Int32**> (BMAllocateHandle ((element.shell.u.revolvedShell.shellShape.nSubPolys + 1) * sizeof (Int32), ALLOCATE_CLEAR, 0));
	memo.shellShapes[0].parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle (element.shell.u.revolvedShell.shellShape.nArcs * sizeof (API_PolyArc), ALLOCATE_CLEAR, 0));
	memo.shellShapes[0].bodyFlags = (GS::Bool8**) BMAllocateHandle ((element.shell.u.revolvedShell.shellShape.nCoords + 1) * sizeof (GS::Bool8), ALLOCATE_CLEAR, 0);
	if (memo.shellShapes[0].coords == NULL || memo.shellShapes[0].pends == NULL || memo.shellShapes[0].parcs == NULL || memo.shellShapes[0].bodyFlags == NULL) {
		ErrorBeep ("Not enough memory to create shell polygon data", APIERR_MEMFULL);
		ACAPI_DisposeElemMemoHdls (&memo);
		return;
	}

	(*memo.shellShapes[0].coords)[ 1].x =  0.0;
	(*memo.shellShapes[0].coords)[ 1].y =  0.0;
	(*memo.shellShapes[0].coords)[ 2].x =  0.4;
	(*memo.shellShapes[0].coords)[ 2].y =  5.0;
	(*memo.shellShapes[0].coords)[ 3].x =  1.0;
	(*memo.shellShapes[0].coords)[ 3].y =  5.0;
	(*memo.shellShapes[0].coords)[ 4].x =  1.0;
	(*memo.shellShapes[0].coords)[ 4].y =  6.0;
	(*memo.shellShapes[0].coords)[ 5].x =  1.7;
	(*memo.shellShapes[0].coords)[ 5].y =  6.0;
	(*memo.shellShapes[0].coords)[ 6].x =  1.7;
	(*memo.shellShapes[0].coords)[ 6].y =  7.0;
	(*memo.shellShapes[0].coords)[ 7].x =  2.4;
	(*memo.shellShapes[0].coords)[ 7].y =  7.0;
	(*memo.shellShapes[0].coords)[ 8].x =  2.4;
	(*memo.shellShapes[0].coords)[ 8].y =  7.4;
	(*memo.shellShapes[0].coords)[ 9].x =  0.0;
	(*memo.shellShapes[0].coords)[ 9].y =  7.7;
	(*memo.shellShapes[0].coords)[10].x =  0.0;
	(*memo.shellShapes[0].coords)[10].y =  8.0;
	(*memo.shellShapes[0].coords)[11].x =  8.0;
	(*memo.shellShapes[0].coords)[11].y = 10.0;
	(*memo.shellShapes[0].coords)[12].x = 12.0;
	(*memo.shellShapes[0].coords)[12].y =  0.0;
	(*memo.shellShapes[0].coords)[13] = (*memo.shellShapes[0].coords)[1];
	(*memo.shellShapes[0].pends)[1] = element.shell.u.revolvedShell.shellShape.nCoords;
	(*memo.shellShapes[0].parcs)[0].begIndex = 1;
	(*memo.shellShapes[0].parcs)[0].endIndex = 2;
	(*memo.shellShapes[0].parcs)[0].arcAngle = -0.143099565651258;
	(*memo.shellShapes[0].parcs)[1].begIndex = 10;
	(*memo.shellShapes[0].parcs)[1].endIndex = 11;
	(*memo.shellShapes[0].parcs)[1].arcAngle = 0.566476134070805;
	(*memo.shellShapes[0].parcs)[2].begIndex = 11;
	(*memo.shellShapes[0].parcs)[2].endIndex = 12;
	(*memo.shellShapes[0].parcs)[2].arcAngle = 0.385936923743763;
	(*memo.shellShapes[0].bodyFlags)[ 1] = true;
	(*memo.shellShapes[0].bodyFlags)[ 2] = true;
	(*memo.shellShapes[0].bodyFlags)[ 3] = true;
	(*memo.shellShapes[0].bodyFlags)[ 4] = true;
	(*memo.shellShapes[0].bodyFlags)[ 5] = true;
	(*memo.shellShapes[0].bodyFlags)[ 6] = true;
	(*memo.shellShapes[0].bodyFlags)[ 7] = true;
	(*memo.shellShapes[0].bodyFlags)[ 8] = true;
	(*memo.shellShapes[0].bodyFlags)[ 9] = true;
	(*memo.shellShapes[0].bodyFlags)[10] = true;
	(*memo.shellShapes[0].bodyFlags)[11] = true;
	(*memo.shellShapes[0].bodyFlags)[12] = false;
	(*memo.shellShapes[0].bodyFlags)[13] = (*memo.shellShapes[0].bodyFlags)[1];

	// constructing the shell contour data
	element.shell.hasContour = false;		// this shell will not be clipped
	element.shell.numHoles = 1;				// but will have a hole

	USize nContours = element.shell.numHoles + (element.shell.hasContour ? 1 : 0);
	memo.shellContours = (API_ShellContourData *) BMAllocatePtr (nContours * sizeof (API_ShellContourData), ALLOCATE_CLEAR, 0);
	if (memo.shellContours == NULL) {
		ErrorBeep ("Not enough memory to create shell contour data", APIERR_MEMFULL);
		ACAPI_DisposeElemMemoHdls (&memo);
		return;
	}

	memo.shellContours[0].poly.nCoords = 5;
	memo.shellContours[0].poly.nSubPolys = 1;
	memo.shellContours[0].poly.nArcs = 1;
	memo.shellContours[0].coords = (API_Coord**) BMAllocateHandle ((memo.shellContours[0].poly.nCoords + 1) * sizeof (API_Coord), ALLOCATE_CLEAR, 0);
	memo.shellContours[0].pends = reinterpret_cast<Int32**> (BMAllocateHandle ((memo.shellContours[0].poly.nSubPolys + 1) * sizeof (Int32), ALLOCATE_CLEAR, 0));
	memo.shellContours[0].parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle (memo.shellContours[0].poly.nArcs * sizeof (API_PolyArc), ALLOCATE_CLEAR, 0));
	if (memo.shellContours[0].coords == NULL || memo.shellContours[0].pends == NULL || memo.shellContours[0].parcs == NULL) {
		ErrorBeep ("Not enough memory to create shell contour data", APIERR_MEMFULL);
		ACAPI_DisposeElemMemoHdls (&memo);
		return;
	}

	(*memo.shellContours[0].coords)[1].x = -1.5;
	(*memo.shellContours[0].coords)[1].y = -0.3;
	(*memo.shellContours[0].coords)[2].x = -1.5;
	(*memo.shellContours[0].coords)[2].y =  3.1;
	(*memo.shellContours[0].coords)[3].x =  1.5;
	(*memo.shellContours[0].coords)[3].y =  3.1;
	(*memo.shellContours[0].coords)[4].x =  1.5;
	(*memo.shellContours[0].coords)[4].y = -0.3;
	(*memo.shellContours[0].coords)[5] = (*memo.shellContours[0].coords)[1];
	(*memo.shellContours[0].pends)[1] = 5;
	(*memo.shellContours[0].parcs)[0].begIndex = 2;
	(*memo.shellContours[0].parcs)[0].endIndex = 3;
	(*memo.shellContours[0].parcs)[0].arcAngle = -240.0 * DEGRAD;

	memo.shellContours[0].height = -5.2;
	tmx = memo.shellContours[0].plane.tmx;
	tmx[ 0] = 1.0;		tmx[ 4] = 0.0;		tmx[ 8] = 0.0;
	tmx[ 1] = 0.0;		tmx[ 5] = 1.0;		tmx[ 9] = 0.0;
	tmx[ 2] = 0.0;		tmx[ 6] = 0.0;		tmx[10] = 1.0;
	tmx[ 3] = 0.0;		tmx[ 7] = 0.0;		tmx[11] = 10.0;

	// create the shell element
	err = ACAPI_Element_Create (&element, &memo);
	if (err != NoError)
		ErrorBeep ("ACAPI_Element_Create (revolved)", err);

	ACAPI_DisposeElemMemoHdls (&memo);

	return;
}		// Do_CreateRevolvedShell


// -----------------------------------------------------------------------------
// Create a Ruled Shell
// -----------------------------------------------------------------------------
void	Do_CreateRuledShell (void)
{
	API_Coord centerPoint;
	if (!ClickAPoint ("Click a center point to create a Ruled Shell", &centerPoint))
		return;

	API_Element element;
	BNZeroMemory (&element, sizeof (API_Element));
	element.header.typeID = API_ShellID;
	element.shell.shellClass = API_RuledShellID;
	GSErrCode err = ACAPI_Element_GetDefaults (&element, NULL);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetDefaults (shell)", err);
		return;
	}

	double dx = 10.0;
	double dy = 10.0;

	double* tmx = element.shell.basePlane.tmx;
	tmx[ 0]	= 1.0;						tmx[ 4]	= 0.0;						tmx[ 8]	= 0.0;
	tmx[ 1]	= 0.0;						tmx[ 5]	= cos (45*DEGRAD);			tmx[ 9]	= sin (45*DEGRAD);
	tmx[ 2]	= 0.0;						tmx[ 6]	= -sin (45*DEGRAD);			tmx[10]	= cos (45*DEGRAD);
	tmx[ 3]	= centerPoint.x - dx/2.0;	tmx[ 7]	= centerPoint.y - dy/2.0;	tmx[11]	= 0.0;

	API_ElementMemo memo;
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

	// constructing the two ruling profiles
	element.shell.u.ruledShell.shellShape1.nCoords		= 3;
	element.shell.u.ruledShell.shellShape1.nSubPolys	= 1;
	element.shell.u.ruledShell.shellShape1.nArcs		= 0;
	tmx = element.shell.u.ruledShell.plane1.tmx;
	tmx[ 0]	=  0.0;		tmx[ 4]	=  0.0;		tmx[ 8]	=  1.0;
	tmx[ 1]	=  0.0;		tmx[ 5]	=  1.0;		tmx[ 9]	=  0.0;
	tmx[ 2]	= -1.0;		tmx[ 6]	=  0.0;		tmx[10]	=  0.0;
	tmx[ 3]	=  0.0;		tmx[ 7]	=  0.0;		tmx[11]	=  0.0;

	element.shell.u.ruledShell.shellShape2.nCoords		= 3;
	element.shell.u.ruledShell.shellShape2.nSubPolys	= 1;
	element.shell.u.ruledShell.shellShape2.nArcs		= 0;
	tmx = element.shell.u.ruledShell.plane2.tmx;
	tmx[ 0]	=  0.0;		tmx[ 4]	=  0.0;		tmx[ 8]	= -1.0;
	tmx[ 1]	=  0.0;		tmx[ 5]	=  1.0;		tmx[ 9]	=  0.0;
	tmx[ 2]	=  1.0;		tmx[ 6]	=  0.0;		tmx[10]	=  0.0;
	tmx[ 3]	=  dx;		tmx[ 7]	=  0.0;		tmx[11]	=  3.0;

	memo.shellShapes[0].coords = (API_Coord**) BMAllocateHandle ((element.shell.u.ruledShell.shellShape1.nCoords + 1) * sizeof (API_Coord), ALLOCATE_CLEAR, 0);
	memo.shellShapes[0].pends = reinterpret_cast<Int32**> (BMAllocateHandle ((element.shell.u.ruledShell.shellShape1.nSubPolys + 1) * sizeof (Int32), ALLOCATE_CLEAR, 0));
	memo.shellShapes[0].bodyFlags = (GS::Bool8**) BMAllocateHandle ((element.shell.u.ruledShell.shellShape1.nCoords + 1) * sizeof (GS::Bool8), ALLOCATE_CLEAR, 0);
	memo.shellShapes[1].coords = (API_Coord**) BMAllocateHandle ((element.shell.u.ruledShell.shellShape2.nCoords + 1) * sizeof (API_Coord), ALLOCATE_CLEAR, 0);
	memo.shellShapes[1].pends = reinterpret_cast<Int32**> (BMAllocateHandle ((element.shell.u.ruledShell.shellShape2.nSubPolys + 1) * sizeof (Int32), ALLOCATE_CLEAR, 0));
	memo.shellShapes[1].bodyFlags = (GS::Bool8**) BMAllocateHandle ((element.shell.u.ruledShell.shellShape2.nCoords + 1) * sizeof (GS::Bool8), ALLOCATE_CLEAR, 0);
	if (memo.shellShapes[0].coords == NULL || memo.shellShapes[0].pends == NULL || memo.shellShapes[0].bodyFlags == NULL ||
		memo.shellShapes[1].coords == NULL || memo.shellShapes[1].pends == NULL || memo.shellShapes[1].bodyFlags == NULL)
	{
		ErrorBeep ("Not enough memory to create shell polygon data", APIERR_MEMFULL);
		ACAPI_DisposeElemMemoHdls (&memo);
		return;
	}

	(*memo.shellShapes[0].coords)[1].x = 4.0;
	(*memo.shellShapes[0].coords)[1].y = 0.0;
	(*memo.shellShapes[0].coords)[2].x = 0.0;
	(*memo.shellShapes[0].coords)[2].y = dy;
	(*memo.shellShapes[0].coords)[3] = (*memo.shellShapes[0].coords)[1];

	(*memo.shellShapes[0].pends)[1] = element.shell.u.ruledShell.shellShape1.nCoords;

	(*memo.shellShapes[0].bodyFlags)[ 1] = true;
	(*memo.shellShapes[0].bodyFlags)[ 2] = false;
	(*memo.shellShapes[0].bodyFlags)[ 3] = (*memo.shellShapes[0].bodyFlags)[ 1];

	(*memo.shellShapes[1].coords)[1].x = 4.0;
	(*memo.shellShapes[1].coords)[1].y = 0.0;
	(*memo.shellShapes[1].coords)[2].x = 0.0;
	(*memo.shellShapes[1].coords)[2].y = dy;
	(*memo.shellShapes[1].coords)[3] = (*memo.shellShapes[1].coords)[1];

	(*memo.shellShapes[1].pends)[1] = element.shell.u.ruledShell.shellShape2.nCoords;

	(*memo.shellShapes[1].bodyFlags)[ 1] = true;
	(*memo.shellShapes[1].bodyFlags)[ 2] = false;
	(*memo.shellShapes[1].bodyFlags)[ 3] = (*memo.shellShapes[1].bodyFlags)[ 1];

	// constructing the shell contour data
	element.shell.hasContour = true;
	memo.shellContours = (API_ShellContourData *) BMAllocatePtr (sizeof (API_ShellContourData), ALLOCATE_CLEAR, 0);
	if (memo.shellContours == NULL) {
		ErrorBeep ("Not enough memory to create shell contour data", APIERR_MEMFULL);
		ACAPI_DisposeElemMemoHdls (&memo);
		return;
	}

	memo.shellContours[0].poly.nCoords = 3;
	memo.shellContours[0].poly.nSubPolys = 1;
	memo.shellContours[0].poly.nArcs = 2;
	memo.shellContours[0].coords = (API_Coord**) BMAllocateHandle ((memo.shellContours[0].poly.nCoords + 1) * sizeof (API_Coord), ALLOCATE_CLEAR, 0);
	memo.shellContours[0].pends = reinterpret_cast<Int32**> (BMAllocateHandle ((memo.shellContours[0].poly.nSubPolys + 1) * sizeof (Int32), ALLOCATE_CLEAR, 0));
	memo.shellContours[0].parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle (memo.shellContours[0].poly.nArcs * sizeof (API_PolyArc), ALLOCATE_CLEAR, 0));
	if (memo.shellContours[0].coords == NULL || memo.shellContours[0].pends == NULL || memo.shellContours[0].parcs == NULL) {
		ErrorBeep ("Not enough memory to create shell contour data", APIERR_MEMFULL);
		ACAPI_DisposeElemMemoHdls (&memo);
		return;
	}

	(*memo.shellContours[0].coords)[1].x = 0.0;
	(*memo.shellContours[0].coords)[1].y = dy;
	(*memo.shellContours[0].coords)[2].x = dx;
	(*memo.shellContours[0].coords)[2].y = 0.0;
	(*memo.shellContours[0].coords)[3] = (*memo.shellContours[0].coords)[1];
	(*memo.shellContours[0].pends)[1] = 3;
	(*memo.shellContours[0].parcs)[0].begIndex = 1;
	(*memo.shellContours[0].parcs)[0].endIndex = 2;
	(*memo.shellContours[0].parcs)[0].arcAngle = PI/2;
	(*memo.shellContours[0].parcs)[1].begIndex = 2;
	(*memo.shellContours[0].parcs)[1].endIndex = 3;
	(*memo.shellContours[0].parcs)[1].arcAngle = PI/2;

	tmx = memo.shellContours[0].plane.tmx;
	tmx[ 0] = 1.0;		tmx[ 4] = 0.0;		tmx[ 8] = 0.0;
	tmx[ 1] = 0.0;		tmx[ 5] = 1.0;		tmx[ 9] = 0.0;
	tmx[ 2] = 0.0;		tmx[ 6] = 0.0;		tmx[10] = 1.0;
	tmx[ 3] = 0.0;		tmx[ 7] = 0.0;		tmx[11] = 0.0;

	// create the shell element
	err = ACAPI_Element_Create (&element, &memo);
	if (err != NoError)
		ErrorBeep ("ACAPI_Element_Create (ruled)", err);

	ACAPI_DisposeElemMemoHdls (&memo);

	return;
}		// Do_CreateRuledShell


// -----------------------------------------------------------------------------
// Create a Morph
// -----------------------------------------------------------------------------
void	Do_CreateMorph (void)
{
	API_Coord referencePoint;
	if (!ClickAPoint ("Click a point to create a Morph", &referencePoint))
		return;

	API_Element element;
	BNZeroMemory (&element, sizeof (API_Element));
	element.header.typeID = API_MorphID;
	GSErrCode err = ACAPI_Element_GetDefaults (&element, NULL);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetDefaults (morph)", err);
		return;
	}

	double* tmx = element.morph.tranmat.tmx;
	tmx[ 0]	= 1.0;				tmx[ 4]	= 0.0;				tmx[ 8]	= 0.0;
	tmx[ 1]	= 0.0;				tmx[ 5]	= 1.0;				tmx[ 9]	= 0.0;
	tmx[ 2]	= 0.0;				tmx[ 6]	= 0.0;				tmx[10]	= 1.0;
	tmx[ 3]	= referencePoint.x;	tmx[ 7]	= referencePoint.y;	tmx[11]	= 1.0;

	// build the body structure
	void* bodyData = NULL;
	ACAPI_Body_Create (NULL, NULL, &bodyData);
	if (bodyData == NULL) {
		ErrorBeep ("bodyData == NULL", APIERR_MEMFULL);
		return;
	}

	// define the vertices
	double dx = 3.0, dy = 2.0, dz = 1.0;	// the dimensions of the morph element to be created
	API_Coord3D coords [] = { {     0.0,     0.0,     0.0  },	// #1
							  {      dx,     0.0,     0.0  },	// #2
							  {     0.0,      dy,     0.0  },	// #3
							  {     0.0,     0.0,      dz  },	// #4
							  {  dx/4.0,     0.0,  dz/4.0  },	// #5
							  {  dx/2.0,     0.0,  dz/4.0  },	// #6
							  {  dx/4.0,     0.0,  dz/2.0  },	// #7
							  {     0.0,  dy/2.0,  dz/4.0  },	// #8
							  {     0.0,  dy/4.0,  dz/4.0  },	// #9
							  {     0.0,  dy/4.0,  dz/2.0  } };	// #10
	UInt32 vertices[10];
	for (UIndex i = 0; i < 10; i++)
		ACAPI_Body_AddVertex (bodyData, coords[i], vertices[i]);

	// connect the vertices to determine edges
	Int32 edges[15];
	ACAPI_Body_AddEdge (bodyData, vertices[0], vertices[1], edges[0]);
	ACAPI_Body_AddEdge (bodyData, vertices[0], vertices[2], edges[1]);
	ACAPI_Body_AddEdge (bodyData, vertices[0], vertices[3], edges[2]);
	ACAPI_Body_AddEdge (bodyData, vertices[1], vertices[2], edges[3]);
	ACAPI_Body_AddEdge (bodyData, vertices[1], vertices[3], edges[4]);
	ACAPI_Body_AddEdge (bodyData, vertices[2], vertices[3], edges[5]);

	ACAPI_Body_AddEdge (bodyData, vertices[4], vertices[5], edges[6]);
	ACAPI_Body_AddEdge (bodyData, vertices[5], vertices[6], edges[7]);
	ACAPI_Body_AddEdge (bodyData, vertices[6], vertices[4], edges[8]);

	ACAPI_Body_AddEdge (bodyData, vertices[7], vertices[8], edges[9]);
	ACAPI_Body_AddEdge (bodyData, vertices[8], vertices[9], edges[10]);
	ACAPI_Body_AddEdge (bodyData, vertices[9], vertices[7], edges[11]);

	ACAPI_Body_AddEdge (bodyData, vertices[4], vertices[8], edges[12]);
	ACAPI_Body_AddEdge (bodyData, vertices[6], vertices[9], edges[13]);
	ACAPI_Body_AddEdge (bodyData, vertices[5], vertices[7], edges[14]);

	// add polygon normal vector
	Int32	polyNormals[1];
	API_Vector3D normal;
	normal.x = normal.z = 0.0;
	normal.y = 1.0;
	ACAPI_Body_AddPolyNormal (bodyData, normal, polyNormals[0]);

	// determine polygons from edges
	API_MaterialOverrideType material;
	material.overrideMaterial = true;

	UInt32 polygons[7];
	Int32 polyEdges[7];
	polyEdges[0] = edges[0];
	polyEdges[1] = edges[4];
	polyEdges[2] = -edges[2];
	polyEdges[3] = 0;
	polyEdges[4] = -edges[8];
	polyEdges[5] = -edges[7];
	polyEdges[6] = -edges[6];
	material.material = 1;
	ACAPI_Body_AddPolygon (bodyData, polyEdges, 7, -polyNormals[0], material, polygons[0]);

	polyEdges[0] = edges[1];
	polyEdges[1] = -edges[3];
	polyEdges[2] = -edges[0];
	material.material = 2;
	ACAPI_Body_AddPolygon (bodyData, polyEdges, 3, 0, material, polygons[1]);

	polyEdges[0] = -edges[4];
	polyEdges[1] = edges[3];
	polyEdges[2] = edges[5];
	material.material = 3;
	ACAPI_Body_AddPolygon (bodyData, polyEdges, 3, 0, material, polygons[2]);

	polyEdges[0] = edges[2];
	polyEdges[1] = -edges[5];
	polyEdges[2] = -edges[1];
	polyEdges[3] = 0;
	polyEdges[4] = -edges[11];
	polyEdges[5] = -edges[10];
	polyEdges[6] = -edges[9];
	material.material = 4;
	ACAPI_Body_AddPolygon (bodyData, polyEdges, 7, 0, material, polygons[3]);

	material.overrideMaterial = false;

	polyEdges[0] = edges[6];
	polyEdges[1] = edges[14];
	polyEdges[2] = edges[9];
	polyEdges[3] = -edges[12];
	material.material = 5;
	ACAPI_Body_AddPolygon (bodyData, polyEdges, 4, 0, material, polygons[4]);

	polyEdges[0] = edges[7];
	polyEdges[1] = edges[13];
	polyEdges[2] = edges[11];
	polyEdges[3] = -edges[14];
	material.material = 6;
	ACAPI_Body_AddPolygon (bodyData, polyEdges, 4, 0, material, polygons[5]);

	polyEdges[0] = -edges[13];
	polyEdges[1] = edges[8];
	polyEdges[2] = edges[12];
	polyEdges[3] = edges[10];
	material.material = 7;
	ACAPI_Body_AddPolygon (bodyData, polyEdges, 4, 0, material, polygons[6]);

	// close the body and copy it to the memo
	API_ElementMemo memo;
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	ACAPI_Body_Finish (bodyData, &memo.morphBody, &memo.morphMaterialMapTable);
	ACAPI_Body_Dispose (&bodyData);

	// create the morph element
	err = ACAPI_Element_Create (&element, &memo);
	if (err != NoError)
		ErrorBeep ("ACAPI_Element_Create (morph)", err);

	ACAPI_DisposeElemMemoHdls (&memo);

	return;
}		// Do_CreateMorph


// -----------------------------------------------------------------------------
// Create a special shaped slab with custom edge trims
// -----------------------------------------------------------------------------
void	Do_CreateSlab (void)
{
	API_Coord centerPoint;
	if (!ClickAPoint ("Click a center point to create a Slab", &centerPoint))
		return;

	API_Element element;
	BNZeroMemory (&element, sizeof (API_Element));
	element.header.typeID = API_SlabID;
	GSErrCode err = ACAPI_Element_GetDefaults (&element, NULL);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetDefaults (slab)", err);
		return;
	}

	element.slab.poly.nCoords	= 65;
	element.slab.poly.nSubPolys	= 9;
	element.slab.poly.nArcs		= 24;

	API_ElementMemo memo;
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	memo.coords = reinterpret_cast<API_Coord**> (BMAllocateHandle ((element.slab.poly.nCoords + 1) * sizeof (API_Coord), ALLOCATE_CLEAR, 0));
	memo.pends = reinterpret_cast<Int32**> (BMAllocateHandle ((element.slab.poly.nSubPolys + 1) * sizeof (Int32), ALLOCATE_CLEAR, 0));
	memo.parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle (element.slab.poly.nArcs * sizeof (API_PolyArc), ALLOCATE_CLEAR, 0));
	if (memo.coords == NULL || memo.pends == NULL || memo.parcs == NULL) {
		ErrorBeep ("Not enough memory to create slab polygon data", APIERR_MEMFULL);
		ACAPI_DisposeElemMemoHdls (&memo);
		return;
	}

	memo.edgeTrims = reinterpret_cast<API_EdgeTrim**> (BMAllocateHandle ((element.slab.poly.nCoords + 1) * sizeof (API_EdgeTrim), ALLOCATE_CLEAR, 0));
	memo.sideMaterials = reinterpret_cast<API_MaterialOverrideType*> (BMAllocatePtr ((element.slab.poly.nCoords + 1) * sizeof (API_MaterialOverrideType), ALLOCATE_CLEAR, 0));
	if (memo.edgeTrims == NULL || memo.sideMaterials == NULL) {
		ErrorBeep ("Not enough memory to create slab edge data", APIERR_MEMFULL);
		ACAPI_DisposeElemMemoHdls (&memo);
		return;
	}

	double divisionAngle = PI/12.0;
	double sideAngle1 = 120 * DEGRAD;
	double sideAngle2 = 160 * DEGRAD;

	Int32 iCoord = 1;
	for (Int32 i = 0; i < 24; i++) {						// slab contour coordinates
		double radius = ((i % 3 == 0) ? 6.0 : 4.0);
		(*memo.coords)[iCoord].x = centerPoint.x + radius * cos (i * divisionAngle);
		(*memo.coords)[iCoord].y = centerPoint.y + radius * sin (i * divisionAngle);
		(*memo.edgeTrims)[iCoord].sideType = APIEdgeTrim_CustomAngle;
		(*memo.edgeTrims)[iCoord].sideAngle = ((i % 3 == 1) ? sideAngle2 : sideAngle1);
		memo.sideMaterials[iCoord] = ((i % 3 == 1) ? element.slab.topMat : element.slab.sideMat);
		++iCoord;
	}
	(*memo.coords)[iCoord] = (*memo.coords)[1];
	(*memo.pends)[1] = iCoord;
	(*memo.edgeTrims)[iCoord].sideType = (*memo.edgeTrims)[1].sideType;
	(*memo.edgeTrims)[iCoord].sideAngle = (*memo.edgeTrims)[1].sideAngle;
	memo.sideMaterials[iCoord] = memo.sideMaterials[1];
	++iCoord;

	Int32 iArc = 0;
	for (Int32 i = 0; i < 8; i++) {							// curved edges of the slab contour
		(*memo.parcs)[iArc].begIndex = i * 3 + 2;
		(*memo.parcs)[iArc].endIndex = i * 3 + 3;
		(*memo.parcs)[iArc].arcAngle = divisionAngle;
		++iArc;
	}

	double outerRadius = 3.5;								// coordinates of the slab holes
	double innerRadius = 0.5;
	for (Int32 i = 0; i < 8; i++) {
		Int32 iStart = iCoord;
		double angle = (i * 3 + 1) * divisionAngle;
		(*memo.coords)[iCoord].x = centerPoint.x + outerRadius * cos (angle);
		(*memo.coords)[iCoord].y = centerPoint.y + outerRadius * sin (angle);
		(*memo.edgeTrims)[iCoord].sideType = APIEdgeTrim_CustomAngle;
		(*memo.edgeTrims)[iCoord].sideAngle = sideAngle1;
		memo.sideMaterials[iCoord] = element.slab.sideMat;
		++iCoord;
		(*memo.coords)[iCoord].x = centerPoint.x + innerRadius * cos (angle);
		(*memo.coords)[iCoord].y = centerPoint.y + innerRadius * sin (angle);
		(*memo.edgeTrims)[iCoord].sideType = APIEdgeTrim_CustomAngle;
		(*memo.edgeTrims)[iCoord].sideAngle = sideAngle2;
		memo.sideMaterials[iCoord] = element.slab.topMat;
		++iCoord;

		angle = (i * 3 + 2) * divisionAngle;
		(*memo.coords)[iCoord].x = centerPoint.x + innerRadius * cos (angle);
		(*memo.coords)[iCoord].y = centerPoint.y + innerRadius * sin (angle);
		(*memo.edgeTrims)[iCoord].sideType = APIEdgeTrim_CustomAngle;
		(*memo.edgeTrims)[iCoord].sideAngle = sideAngle1;
		memo.sideMaterials[iCoord] = element.slab.sideMat;
		++iCoord;
		(*memo.coords)[iCoord].x = centerPoint.x + outerRadius * cos (angle);
		(*memo.coords)[iCoord].y = centerPoint.y + outerRadius * sin (angle);
		(*memo.edgeTrims)[iCoord].sideType = APIEdgeTrim_CustomAngle;
		(*memo.edgeTrims)[iCoord].sideAngle = sideAngle2;
		memo.sideMaterials[iCoord] = element.slab.topMat;
		++iCoord;

		// kontur lezarasa
		(*memo.coords)[iCoord] = (*memo.coords)[iStart];
		(*memo.pends)[i + 2] = iCoord;
		(*memo.edgeTrims)[iCoord].sideType = (*memo.edgeTrims)[iStart].sideType;
		(*memo.edgeTrims)[iCoord].sideAngle = (*memo.edgeTrims)[iStart].sideAngle;
		memo.sideMaterials[iCoord] = memo.sideMaterials[iStart];
		++iCoord;

		(*memo.parcs)[iArc].begIndex = iStart + 1;			// curved edges of the slab holes
		(*memo.parcs)[iArc].endIndex = iStart + 2;
		(*memo.parcs)[iArc].arcAngle = divisionAngle;
		++iArc;
		(*memo.parcs)[iArc].begIndex = iStart + 3;
		(*memo.parcs)[iArc].endIndex = iStart + 4;
		(*memo.parcs)[iArc].arcAngle = -divisionAngle;
		++iArc;
	}

	err = ACAPI_Element_Create (&element, &memo);
	if (err != NoError)
		ErrorBeep ("ACAPI_Element_Create (slab)", err);

	ACAPI_DisposeElemMemoHdls (&memo);

	return;
}		// Do_CreateSlab


// -----------------------------------------------------------------------------
// Create a curved wall using the default settings and user input
// -----------------------------------------------------------------------------
void		Do_CreateCurvedWall (void)
{
	API_Coord			c;
	API_Element			element, mask;
	GSErrCode			err;

	BNZeroMemory (&element, sizeof (API_Element));
	element.header.typeID = API_WallID;
	ACAPI_Element_GetDefaults (&element, NULL);

	// input the arc
	if (!GetAnArc ("Input an arc", &c, &element.wall.begC, &element.wall.endC))
		return;

	double	fa = ComputeFiPtr (&c, &element.wall.begC);
	double	fe = ComputeFiPtr (&c, &element.wall.endC);
	while (fa + EPS >= 2*PI)
		fa -= 2*PI;
	while (fe + EPS >= 2*PI)
		fe -= 2*PI;
	element.wall.angle = fa - fe;

	err = ACAPI_Element_Create (&element, NULL);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Create (curved wall)", err);
		return;
	}

	ACAPI_Element_Get (&element);
	WriteReport_Alert ("Guid of the curved wall: %s", APIGuidToString (element.header.guid).ToCStr ().Get ());

	err = ACAPI_CallUndoableCommand ("Modify curved wall",
		[&] () -> GSErrCode {
			ACAPI_ELEMENT_MASK_CLEAR (mask);
			double	len = DistCPtr (&c, &element.wall.endC);
			fe = fa + element.wall.angle * 3/4;
			fa += element.wall.angle/4;
			element.wall.angle /= 2;
			element.wall.endC.x = c.x + len * cos (fe);
			element.wall.endC.y = c.y + len * sin (fe);
			element.wall.begC.x = c.x + len * cos (fa);
			element.wall.begC.y = c.y + len * sin (fa);
			ACAPI_ELEMENT_MASK_SET (mask, API_WallType, begC);
			ACAPI_ELEMENT_MASK_SET (mask, API_WallType, endC);
			ACAPI_ELEMENT_MASK_SET (mask, API_WallType, angle);

			return ACAPI_Element_Change (&element, &mask, NULL, 0, true);
		});

	if (err != NoError)
		ErrorBeep ("ACAPI_Element_Change (curved wall)", err);

	return;
}		// Do_CreateCurvedWall


// -----------------------------------------------------------------------------
// Create a curved beam using the default settings and user input
// -----------------------------------------------------------------------------
void		Do_CreateCurvedBeam (void)
{
	API_Coord		centrePos, startPos, endPos;
	bool			isArcNegative;
	API_Element		element;
	GSErrCode		err;

	BNZeroMemory (&element, sizeof (API_Element));
	element.header.typeID = API_BeamID;
	ACAPI_Element_GetDefaults (&element, NULL);

	// input the arc
	if (!GetAnArc ("Input an arc", &centrePos, &startPos, &endPos, &isArcNegative))
		return;

	element.beam.begC = isArcNegative ? endPos : startPos;
	element.beam.endC = isArcNegative ? startPos : endPos;
	double	fa = ComputeFiPtr (&centrePos, &element.beam.begC, !isArcNegative);
	double	fe = ComputeFiPtr (&centrePos, &element.beam.endC, isArcNegative);
	element.beam.curveAngle = fe - fa;
	while (element.beam.curveAngle + EPS >= 2*PI)
		element.beam.curveAngle -= 2*PI;
	while (element.beam.curveAngle - EPS <= -2*PI)
		element.beam.curveAngle += 2*PI;

	// drill circular holes into the beam
	API_ElementMemo memo;
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	const double holeDistance = 0.5;
	double radius = DistCPtr (&centrePos, &element.beam.begC);
	double length = radius * fabs (element.beam.curveAngle);
	Int32 nHoles = (Int32) (length / holeDistance);
	if (nHoles > 0) {
		memo.beamHoles = reinterpret_cast<API_Beam_Hole**> (BMAllocateHandle (nHoles * sizeof (API_Beam_Hole), ALLOCATE_CLEAR, 0));
		if (memo.beamHoles != NULL) {
			for (Int32 i = 0; i < nHoles; ++i) {
				(*memo.beamHoles)[i].holeType		= APIBHole_Circular;
				(*memo.beamHoles)[i].holeContureOn	= element.beam.holeContureOn;
				(*memo.beamHoles)[i].holeID			= i + 1;
				(*memo.beamHoles)[i].centerx		= (i + 1) * holeDistance;
				(*memo.beamHoles)[i].centerz		= element.beam.holeLevel;
				(*memo.beamHoles)[i].width			= element.beam.holeWidth;
				(*memo.beamHoles)[i].height			= element.beam.holeHeight;
			}
		}
	}

	// create the beam
	err = ACAPI_Element_Create (&element, &memo);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Create (curved beam)", err);
		return;
	}

	ACAPI_DisposeElemMemoHdls (&memo);

	return;
}		// Do_CreateCurvedBeam


// -----------------------------------------------------------------------------
// Create a Curtain Wall on the floor plan
// -----------------------------------------------------------------------------
void	Do_CreateCurtainWall (void)
{
	API_Element			element;
	API_ElementMemo		memo;
	API_Coord			c;
	API_GetPolyType		polyInfo;
	GSErrCode			err;

	if (!ClickAPoint ("Enter the first corner of the curtain wall", &c))
		return;

	BNZeroMemory (&polyInfo, sizeof (API_GetPolyType));
	polyInfo.useStandardPetPalette = true;
	strcpy (polyInfo.prompt, "Enter the next corner of the curtain wall");
	polyInfo.method = APIPolyGetMethod_Polyline;
	polyInfo.startCoord.x = c.x;
	polyInfo.startCoord.y = c.y;

	err = ACAPI_Interface (APIIo_GetPolyID, &polyInfo, NULL);
	if (err != NoError)
		return;

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	element.header.typeID = API_CurtainWallID;
	err = ACAPI_Element_GetDefaults (&element, &memo);
		if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetMemo", err);
		return;
	}

	memo.coords = polyInfo.coords;
	memo.parcs = polyInfo.parcs;
	element.curtainWall.nSegments = BMhGetSize (reinterpret_cast<GSHandle> (memo.coords)) / Sizeof32 (API_Coord) - 2;

	// Modify segment data
	element.curtainWall.segmentData.primaryPatternNum += 2;
	element.curtainWall.segmentData.secondaryPatternNum++;
	element.curtainWall.segmentData.panelPatternNum = element.curtainWall.segmentData.primaryPatternNum * element.curtainWall.segmentData.secondaryPatternNum;
	memo.cWSegPrimaryPattern   = (double*) BMReallocPtr ((GSPtr) memo.cWSegPrimaryPattern, element.curtainWall.segmentData.primaryPatternNum*sizeof (double), REALLOC_MOVEABLE, 0);
	if (memo.cWSegPrimaryPattern != NULL) {
		memo.cWSegPrimaryPattern[0] = 0.5;
		memo.cWSegPrimaryPattern[element.curtainWall.segmentData.primaryPatternNum-2] = memo.cWSegPrimaryPattern[element.curtainWall.segmentData.primaryPatternNum-1] = 0.75;
	}
	memo.cWSegSecondaryPattern = (double*) BMReallocPtr ((GSPtr) memo.cWSegSecondaryPattern, element.curtainWall.segmentData.secondaryPatternNum*sizeof (double), REALLOC_MOVEABLE, 0);
	if (memo.cWSegSecondaryPattern != NULL) {
		memo.cWSegSecondaryPattern[0] = 0.35;
		memo.cWSegSecondaryPattern[element.curtainWall.segmentData.secondaryPatternNum-1] = 1;
	}
	memo.cWSegPanelPattern     = (GS::Bool8*) BMReallocPtr ((GSPtr) memo.cWSegPanelPattern, element.curtainWall.segmentData.panelPatternNum*sizeof (GS::Bool8), REALLOC_MOVEABLE, 0);
	if (memo.cWSegPanelPattern != NULL) {
		for (UInt32 ii = 0; ii < element.curtainWall.segmentData.panelPatternNum; ++ii)
			memo.cWSegPanelPattern[ii] = GS::Bool8 (ii % 3);
	}

	err = ACAPI_Element_Create (&element, &memo);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Create", err);
		return;
	}

	ACAPI_DisposeElemMemoHdls (&memo);

	return;
}	// Do_CreateCurtainWall


// -----------------------------------------------------------------------------
// Create meshes from irregular polygon
// -----------------------------------------------------------------------------
void	Do_CreateIrregularMesh (void)
{
	API_Coord centerPoint;
	if (!ClickAPoint ("Click a center point to create a Mesh", &centerPoint))
		return;

	API_Element element;
	BNZeroMemory (&element, sizeof (API_Element));
	element.header.typeID = API_MeshID;
	GSErrCode err = ACAPI_Element_GetDefaults (&element, NULL);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetDefaults (mesh)", err);
		return;
	}

	element.mesh.poly.nCoords	= 5;
	element.mesh.poly.nSubPolys	= 1;
	element.mesh.poly.nArcs		= 0;

	API_ElementMemo memo;
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	memo.coords = reinterpret_cast<API_Coord**> (BMAllocateHandle ((element.mesh.poly.nCoords + 1) * sizeof (API_Coord), ALLOCATE_CLEAR, 0));
	memo.pends = reinterpret_cast<Int32**> (BMAllocateHandle ((element.mesh.poly.nSubPolys + 1) * sizeof (Int32), ALLOCATE_CLEAR, 0));
	memo.parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle (element.mesh.poly.nArcs * sizeof (API_PolyArc), ALLOCATE_CLEAR, 0));
	if (memo.coords == NULL || memo.pends == NULL || memo.parcs == NULL) {
		ErrorBeep ("Not enough memory to create mesh polygon data", APIERR_MEMFULL);
		ACAPI_DisposeElemMemoHdls (&memo);
		return;
	}

	memo.meshPolyZ = reinterpret_cast<double**> (BMAllocateHandle ((element.mesh.poly.nCoords + 1) * sizeof (double), ALLOCATE_CLEAR, 0));
	if (memo.meshPolyZ == NULL) {
		ErrorBeep ("Not enough memory to create mesh vertex data", APIERR_MEMFULL);
		ACAPI_DisposeElemMemoHdls (&memo);
		return;
	}

	(*memo.coords)[1].x = centerPoint.x + 0.0;
	(*memo.coords)[1].y = centerPoint.y + 0.0;
	(*memo.coords)[2].x = centerPoint.x + 5.0;
	(*memo.coords)[2].y = centerPoint.y + 3.0;
	(*memo.coords)[3].x = centerPoint.x + 5.0;
	(*memo.coords)[3].y = centerPoint.y + 0.0;
	(*memo.coords)[4].x = centerPoint.x + 0.0;
	(*memo.coords)[4].y = centerPoint.y + 2.0;
	(*memo.coords)[element.mesh.poly.nCoords] = (*memo.coords)[1];

	(*memo.pends)[1] = element.mesh.poly.nCoords;

	(*memo.meshPolyZ)[1] = 1.0;
	(*memo.meshPolyZ)[2] = 2.0;
	(*memo.meshPolyZ)[3] = 3.0;
	(*memo.meshPolyZ)[4] = 4.0;
	(*memo.meshPolyZ)[5] = (*memo.meshPolyZ)[1];

	err = ACAPI_Element_Create (&element, &memo);
	if (err == APIERR_IRREGULARPOLY) {
		API_RegularizedPoly poly;
		BNZeroMemory (&poly, sizeof (API_RegularizedPoly));
		poly.coords = memo.coords;
		poly.pends = memo.pends;
		poly.parcs = memo.parcs;
		poly.vertexIDs = memo.vertexIDs;
		poly.needVertexAncestry = 1;
		Int32 nResult = 0;
		API_RegularizedPoly** polys = NULL;
		err = ACAPI_Goodies (APIAny_RegularizePolygonID, &poly, &nResult, &polys);
		if (err != NoError)
			ErrorBeep ("ACAPI_Element_Create (mesh) regularize", err);
		if (err == NoError) {
			for (Int32 i = 0; i < nResult; i++) {
				element.mesh.poly.nCoords = BMhGetSize (reinterpret_cast<GSHandle> ((*polys)[i].coords)) / sizeof (API_Coord) - 1;
				element.mesh.poly.nSubPolys = BMhGetSize (reinterpret_cast<GSHandle> ((*polys)[i].pends)) / sizeof (Int32) - 1;
				element.mesh.poly.nArcs = BMhGetSize (reinterpret_cast<GSHandle> ((*polys)[i].parcs)) / sizeof (API_PolyArc);

				API_ElementMemo tmpMemo;
				BNZeroMemory (&tmpMemo, sizeof (API_ElementMemo));
				tmpMemo.coords = (*polys)[i].coords;
				tmpMemo.pends = (*polys)[i].pends;
				tmpMemo.parcs = (*polys)[i].parcs;
				tmpMemo.vertexIDs = (*polys)[i].vertexIDs;

				tmpMemo.meshPolyZ = reinterpret_cast<double**> (BMAllocateHandle ((element.mesh.poly.nCoords + 1) * sizeof (double), ALLOCATE_CLEAR, 0));
				if (tmpMemo.meshPolyZ == NULL) {
					ErrorBeep ("Not enough memory to create mesh vertex data", APIERR_MEMFULL);
					BMKillHandle (reinterpret_cast<GSHandle*> (&tmpMemo.meshPolyZ));
					return;
				}

				for (Int32 j = 1; j <= element.mesh.poly.nCoords; j++) {
					Int32 oldVertexIndex = (*(*polys)[i].vertexAncestry)[j];
					if (oldVertexIndex == 0) {				// new vertex after regularization, apply default
						(*tmpMemo.meshPolyZ)[j] = 0.0;
					} else {
						(*tmpMemo.meshPolyZ)[j] = (*memo.meshPolyZ)[oldVertexIndex];
					}
				}

				err = ACAPI_Element_Create (&element, &tmpMemo);
				if (err != NoError)
					ErrorBeep ("ACAPI_Element_Create (mesh) pieces", err);
				ACAPI_Goodies (APIAny_DisposeRegularizedPolyID, &(*polys)[i]);
				BMKillHandle (reinterpret_cast<GSHandle*> (&tmpMemo.meshPolyZ));
			}
		}
		BMKillHandle (reinterpret_cast<GSHandle*> (&polys));
	}
	if (err != NoError)
		ErrorBeep ("ACAPI_Element_Create (mesh)", err);

	ACAPI_DisposeElemMemoHdls (&memo);

	return;
}		// Do_CreateIrregularMesh


// -----------------------------------------------------------------------------
// Create slabs from irregular polygon
// -----------------------------------------------------------------------------
void	Do_CreateIrregularSlab (void)
{
	API_Coord centerPoint;
	if (!ClickAPoint ("Click a center point to create a Slab", &centerPoint))
		return;

	API_Element element;
	BNZeroMemory (&element, sizeof (API_Element));
	element.header.typeID = API_SlabID;
	GSErrCode err = ACAPI_Element_GetDefaults (&element, NULL);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetDefaults (slab)", err);
		return;
	}

	element.slab.poly.nCoords	= 5;
	element.slab.poly.nSubPolys	= 1;
	element.slab.poly.nArcs		= 0;

	API_ElementMemo memo;
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	memo.coords = reinterpret_cast<API_Coord**> (BMAllocateHandle ((element.slab.poly.nCoords + 1) * sizeof (API_Coord), ALLOCATE_CLEAR, 0));
	memo.pends = reinterpret_cast<Int32**> (BMAllocateHandle ((element.slab.poly.nSubPolys + 1) * sizeof (Int32), ALLOCATE_CLEAR, 0));
	memo.parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle (element.slab.poly.nArcs * sizeof (API_PolyArc), ALLOCATE_CLEAR, 0));
	if (memo.coords == NULL || memo.pends == NULL || memo.parcs == NULL) {
		ErrorBeep ("Not enough memory to create slab polygon data", APIERR_MEMFULL);
		ACAPI_DisposeElemMemoHdls (&memo);
		return;
	}

	memo.edgeTrims = reinterpret_cast<API_EdgeTrim**> (BMAllocateHandle ((element.slab.poly.nCoords + 1) * sizeof (API_EdgeTrim), ALLOCATE_CLEAR, 0));
	memo.sideMaterials = reinterpret_cast<API_MaterialOverrideType*> (BMAllocatePtr ((element.slab.poly.nCoords + 1) * sizeof (API_MaterialOverrideType), ALLOCATE_CLEAR, 0));
	if (memo.edgeTrims == NULL || memo.sideMaterials == NULL) {
		ErrorBeep ("Not enough memory to create slab edge data", APIERR_MEMFULL);
		ACAPI_DisposeElemMemoHdls (&memo);
		return;
	}

	(*memo.coords)[1].x = centerPoint.x + 0.0;
	(*memo.coords)[1].y = centerPoint.y + 0.0;
	(*memo.coords)[2].x = centerPoint.x + 5.0;
	(*memo.coords)[2].y = centerPoint.y + 3.0;
	(*memo.coords)[3].x = centerPoint.x + 5.0;
	(*memo.coords)[3].y = centerPoint.y + 0.0;
	(*memo.coords)[4].x = centerPoint.x + 0.0;
	(*memo.coords)[4].y = centerPoint.y + 2.0;
	(*memo.coords)[element.slab.poly.nCoords] = (*memo.coords)[1];

	(*memo.pends)[1] = element.slab.poly.nCoords;

	(*memo.edgeTrims)[1].sideType = APIEdgeTrim_CustomAngle;
	(*memo.edgeTrims)[1].sideAngle = 120 * DEGRAD;
	memo.sideMaterials[1] = element.slab.botMat;
	(*memo.edgeTrims)[2].sideType = APIEdgeTrim_CustomAngle;
	(*memo.edgeTrims)[2].sideAngle = 45 * DEGRAD;
	memo.sideMaterials[2] = element.slab.topMat;
	(*memo.edgeTrims)[3].sideType = APIEdgeTrim_CustomAngle;
	(*memo.edgeTrims)[3].sideAngle = 120 * DEGRAD;
	memo.sideMaterials[3] = element.slab.botMat;
	(*memo.edgeTrims)[4].sideType = APIEdgeTrim_CustomAngle;
	(*memo.edgeTrims)[4].sideAngle = 45 * DEGRAD;
	memo.sideMaterials[4] = element.slab.topMat;

	(*memo.edgeTrims)[element.slab.poly.nCoords].sideType = (*memo.edgeTrims)[1].sideType;
	(*memo.edgeTrims)[element.slab.poly.nCoords].sideAngle = (*memo.edgeTrims)[1].sideAngle;
	memo.sideMaterials[element.slab.poly.nCoords] = memo.sideMaterials[1];

	err = ACAPI_Element_Create (&element, &memo);
	if (err == APIERR_IRREGULARPOLY) {
		API_RegularizedPoly poly;
		BNZeroMemory (&poly, sizeof (API_RegularizedPoly));
		poly.coords = memo.coords;
		poly.pends = memo.pends;
		poly.parcs = memo.parcs;
		poly.vertexIDs = memo.vertexIDs;
		poly.needEdgeAncestry = 1;
		Int32 nResult = 0;
		API_RegularizedPoly** polys = NULL;
		err = ACAPI_Goodies (APIAny_RegularizePolygonID, &poly, &nResult, &polys);
		if (err != NoError)
			ErrorBeep ("ACAPI_Element_Create (slab) regularize", err);
		if (err == NoError) {
			for (Int32 i = 0; i < nResult; i++) {
				element.slab.poly.nCoords = BMhGetSize (reinterpret_cast<GSHandle> ((*polys)[i].coords)) / sizeof (API_Coord) - 1;
				element.slab.poly.nSubPolys = BMhGetSize (reinterpret_cast<GSHandle> ((*polys)[i].pends)) / sizeof (Int32) - 1;
				element.slab.poly.nArcs = BMhGetSize (reinterpret_cast<GSHandle> ((*polys)[i].parcs)) / sizeof (API_PolyArc);

				API_ElementMemo tmpMemo;
				BNZeroMemory (&tmpMemo, sizeof (API_ElementMemo));
				tmpMemo.coords = (*polys)[i].coords;
				tmpMemo.pends = (*polys)[i].pends;
				tmpMemo.parcs = (*polys)[i].parcs;
				tmpMemo.vertexIDs = (*polys)[i].vertexIDs;

				tmpMemo.edgeTrims = reinterpret_cast<API_EdgeTrim**> (BMAllocateHandle ((element.slab.poly.nCoords + 1) * sizeof (API_EdgeTrim), ALLOCATE_CLEAR, 0));
				tmpMemo.sideMaterials = reinterpret_cast<API_MaterialOverrideType*> (BMAllocatePtr ((element.slab.poly.nCoords + 1) * sizeof (API_MaterialOverrideType), ALLOCATE_CLEAR, 0));
				if (tmpMemo.edgeTrims == NULL || tmpMemo.sideMaterials == NULL) {
					ErrorBeep ("Not enough memory to create slab edge data", APIERR_MEMFULL);
					BMKillHandle (reinterpret_cast<GSHandle*> (&tmpMemo.edgeTrims));
					BMpFree (reinterpret_cast<GSPtr> (tmpMemo.sideMaterials));
					return;
				}

				for (Int32 j = 1; j <= element.slab.poly.nCoords; j++) {
					Int32 oldEdgeIndex = (*(*polys)[i].edgeAncestry)[j];
					if (oldEdgeIndex == 0) {				// new edge after regularization, apply default
						(*tmpMemo.edgeTrims)[j] .sideType = APIEdgeTrim_Vertical;
						(*tmpMemo.edgeTrims)[j] .sideAngle = PI / 4.0;
						tmpMemo.sideMaterials[j] = element.slab.sideMat;
					} else {
						(*tmpMemo.edgeTrims)[j] = (*memo.edgeTrims)[oldEdgeIndex];
						tmpMemo.sideMaterials[j] = memo.sideMaterials[oldEdgeIndex];
					}
				}

				err = ACAPI_Element_Create (&element, &tmpMemo);
				if (err != NoError)
					ErrorBeep ("ACAPI_Element_Create (slab) pieces", err);
				ACAPI_Goodies (APIAny_DisposeRegularizedPolyID, &(*polys)[i]);
				BMKillHandle (reinterpret_cast<GSHandle*> (&tmpMemo.edgeTrims));
				BMpFree (reinterpret_cast<GSPtr> (tmpMemo.sideMaterials));
			}
		}
		BMKillHandle (reinterpret_cast<GSHandle*> (&polys));
	}
	if (err != NoError)
		ErrorBeep ("ACAPI_Element_Create (slab)", err);

	ACAPI_DisposeElemMemoHdls (&memo);

	return;
}		// Do_CreateIrregularSlab


// -----------------------------------------------------------------------------
// Create an extruded shell from irregular polyline
// -----------------------------------------------------------------------------
void 	Do_CreateIrregularExtShell (void)
{
	API_Coord centerPoint;
	if (!ClickAPoint ("Click a center point to create a junction of Extruded Shells", &centerPoint))
		return;

	API_Element element;
	BNZeroMemory (&element, sizeof (API_Element));
	element.header.typeID = API_ShellID;
	element.shell.shellClass = API_ExtrudedShellID;
	GSErrCode err = ACAPI_Element_GetDefaults (&element, NULL);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetDefaults (shell)", err);
		return;
	}

	API_ElementMemo memo;
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

	element.shell.u.extrudedShell.shellShape.nCoords	= 6;
	element.shell.u.extrudedShell.shellShape.nSubPolys	= 1;
	element.shell.u.extrudedShell.shellShape.nArcs		= 1;

	element.shell.u.extrudedShell.begC.x = centerPoint.x;
	element.shell.u.extrudedShell.begC.y = centerPoint.y;
	element.shell.u.extrudedShell.extrusionVector.x = 20.0;
	element.shell.u.extrudedShell.extrusionVector.y = 0.0;
	element.shell.u.extrudedShell.extrusionVector.z = 0.0;

	memo.shellShapes[0].coords = (API_Coord**) BMAllocateHandle ((element.shell.u.extrudedShell.shellShape.nCoords + 1) * sizeof (API_Coord), ALLOCATE_CLEAR, 0);
	memo.shellShapes[0].pends = reinterpret_cast<Int32**> (BMAllocateHandle ((element.shell.u.extrudedShell.shellShape.nSubPolys + 1) * sizeof (Int32), ALLOCATE_CLEAR, 0));
	memo.shellShapes[0].parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle (element.shell.u.extrudedShell.shellShape.nArcs * sizeof (API_PolyArc), ALLOCATE_CLEAR, 0));
	memo.shellShapes[0].bodyFlags = (GS::Bool8**) BMAllocateHandle ((element.shell.u.extrudedShell.shellShape.nCoords + 1) * sizeof (GS::Bool8), ALLOCATE_CLEAR, 0);
	if (memo.shellShapes[0].coords == NULL || memo.shellShapes[0].pends == NULL || memo.shellShapes[0].parcs == NULL || memo.shellShapes[0].bodyFlags == NULL) {
		ErrorBeep ("Not enough memory to create shell polygon data", APIERR_MEMFULL);
		ACAPI_DisposeElemMemoHdls (&memo);
		return;
	}

	(*memo.shellShapes[0].coords)[1].x = -2.0;
	(*memo.shellShapes[0].coords)[1].y =  0.0;
	(*memo.shellShapes[0].coords)[2].x = -2.5;
	(*memo.shellShapes[0].coords)[2].y =  2.0;
	(*memo.shellShapes[0].coords)[3].x =  2.5;
	(*memo.shellShapes[0].coords)[3].y =  2.0;
	(*memo.shellShapes[0].coords)[4].x =  2.0;
	(*memo.shellShapes[0].coords)[4].y =  0.0;
	(*memo.shellShapes[0].coords)[5].x =  2.0;
	(*memo.shellShapes[0].coords)[5].y =  0.0;
	(*memo.shellShapes[0].coords)[6] = (*memo.shellShapes[0].coords)[1];
	(*memo.shellShapes[0].pends)[1] = element.shell.u.extrudedShell.shellShape.nCoords;
	(*memo.shellShapes[0].parcs)[0].begIndex = 2;
	(*memo.shellShapes[0].parcs)[0].endIndex = 3;
	(*memo.shellShapes[0].parcs)[0].arcAngle = -200 * DEGRAD;
	(*memo.shellShapes[0].bodyFlags)[1] = true;
	(*memo.shellShapes[0].bodyFlags)[2] = true;
	(*memo.shellShapes[0].bodyFlags)[3] = true;
	(*memo.shellShapes[0].bodyFlags)[4] = false;
	(*memo.shellShapes[0].bodyFlags)[5] = false;
	(*memo.shellShapes[0].bodyFlags)[6] = (*memo.shellShapes[0].bodyFlags)[1];

	element.shell.u.extrudedShell.begPlaneTilt = 120 * DEGRAD;
	element.shell.u.extrudedShell.endPlaneTilt =  60 * DEGRAD;

	// create shell element
	err = ACAPI_Element_Create (&element, &memo);
	if (err == APIERR_IRREGULARPOLY) {
		API_RegularizedPoly poly;
		API_RegularizedPoly resPoly;
		BNZeroMemory (&poly, sizeof (API_RegularizedPoly));
		BNZeroMemory (&resPoly, sizeof (API_RegularizedPoly));
		poly.coords = memo.shellShapes[0].coords;
		poly.pends = memo.shellShapes[0].pends;
		poly.parcs = memo.shellShapes[0].parcs;
		poly.vertexIDs = memo.shellShapes[0].vertexIDs;
		poly.needVertexAncestry = 1;
		poly.needEdgeAncestry = 1;
		err = ACAPI_Goodies (APIAny_RegularizePolylineID, &poly, &resPoly);
		if (err != NoError) {
			ErrorBeep ("ACAPI_Element_Create (extShell) regularize", err);
		} else {
			element.shell.u.extrudedShell.shellShape.nCoords = BMhGetSize (reinterpret_cast<GSHandle> (resPoly.coords)) / sizeof (API_Coord) - 1;
			element.shell.u.extrudedShell.shellShape.nSubPolys = BMhGetSize (reinterpret_cast<GSHandle> (resPoly.pends)) / sizeof (Int32) - 1;
			element.shell.u.extrudedShell.shellShape.nArcs = BMhGetSize (reinterpret_cast<GSHandle> (resPoly.parcs)) / sizeof (API_PolyArc);

			API_ElementMemo tmpMemo;
			BNZeroMemory (&tmpMemo, sizeof (API_ElementMemo));
			tmpMemo.shellShapes[0].coords = resPoly.coords;
			tmpMemo.shellShapes[0].pends = resPoly.pends;
			tmpMemo.shellShapes[0].parcs = resPoly.parcs;
			tmpMemo.shellShapes[0].vertexIDs = resPoly.vertexIDs;

			tmpMemo.shellShapes[0].bodyFlags = (GS::Bool8**) BMAllocateHandle ((element.shell.u.extrudedShell.shellShape.nCoords + 1) * sizeof (GS::Bool8), ALLOCATE_CLEAR, 0);

			if (tmpMemo.shellShapes[0].bodyFlags == NULL) {
				ErrorBeep ("Not enough memory to create extruded shell edge data", APIERR_MEMFULL);
				BMKillHandle (reinterpret_cast<GSHandle*> (&tmpMemo.shellShapes[0].bodyFlags));
				return;
			}

			for (Int32 j = 1; j <= element.shell.u.extrudedShell.shellShape.nCoords; j++) {
				Int32 oldEdgeIndex = (*resPoly.edgeAncestry)[j];
				if (oldEdgeIndex == 0)				// new edge after regularization, apply default
					(*tmpMemo.shellShapes[0].bodyFlags)[j] = 0;
				else
					(*tmpMemo.shellShapes[0].bodyFlags)[j] = (*memo.shellShapes[0].bodyFlags)[oldEdgeIndex];
			}

			err = ACAPI_Element_Create (&element, &tmpMemo);
			if (err != NoError)
				ErrorBeep ("ACAPI_Element_Create (extruded shell)", err);
			ACAPI_Goodies (APIAny_DisposeRegularizedPolyID, &resPoly);
			BMKillHandle (reinterpret_cast<GSHandle*> (&tmpMemo.shellShapes[0].bodyFlags));
		}
	} else if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Create (extruded shell)", err);
	}

	ACAPI_DisposeElemMemoHdls (&memo);

	return;
}		// Do_CreateIrregularExtShell


// -----------------------------------------------------------------------------
// Set a paragraph
// -----------------------------------------------------------------------------
static GSErrCode SetParagraph (API_ParagraphType** paragraph, UInt32 parNum, Int32 from, Int32 range, API_JustID just, double firstIndent,
							   double indent, double rightIndent, double spacing, Int32 numOfTabs, Int32 numOfRuns,
							   Int32 numOfeolPos)
{
	if (paragraph == NULL || parNum >= (BMhGetSize (reinterpret_cast<GSHandle> (paragraph)) / sizeof (API_ParagraphType)))
		return APIERR_BADPARS;

	if (numOfTabs < 1 || numOfRuns < 1 || numOfeolPos < 0)
		return APIERR_BADPARS;

	(*paragraph)[parNum].from  		 = from;
	(*paragraph)[parNum].range 		 = range;
	(*paragraph)[parNum].just  		 = just;
	(*paragraph)[parNum].firstIndent = firstIndent;
	(*paragraph)[parNum].indent 	 = indent;
	(*paragraph)[parNum].rightIndent = rightIndent;
	(*paragraph)[parNum].spacing 	 = spacing;

	(*paragraph)[parNum].tab = reinterpret_cast<API_TabType*> (BMAllocatePtr (numOfTabs * sizeof (API_TabType), ALLOCATE_CLEAR, 0));
	if ((*paragraph)[parNum].tab == NULL)
		return APIERR_MEMFULL;

	(*paragraph)[parNum].run = reinterpret_cast<API_RunType*> (BMAllocatePtr (numOfRuns * sizeof (API_RunType), ALLOCATE_CLEAR, 0));
	if ((*paragraph)[parNum].run == NULL)
		return APIERR_MEMFULL;

	if (numOfeolPos > 0) {
		(*paragraph)[parNum].eolPos = reinterpret_cast<Int32*> (BMAllocatePtr (numOfeolPos * sizeof (Int32), ALLOCATE_CLEAR, 0));
		if ((*paragraph)[parNum].eolPos == NULL)
			return APIERR_MEMFULL;
	}

	return NoError;
}


// -----------------------------------------------------------------------------
// Set a tab of a paragraph
// -----------------------------------------------------------------------------
static GSErrCode SetTab (API_ParagraphType** paragraph, UInt32 parNum, UInt32 tabNum, API_TabID	type, double pos)
{
	if (paragraph == NULL || parNum >= (BMhGetSize (reinterpret_cast<GSHandle> (paragraph)) / sizeof (API_ParagraphType)))
		return APIERR_BADPARS;

	if (tabNum >= BMGetPtrSize (reinterpret_cast<GSPtr> ((*paragraph)[parNum].tab)) / sizeof (API_TabType))
		return APIERR_BADPARS;

	(*paragraph)[parNum].tab[tabNum].type = type;
	(*paragraph)[parNum].tab[tabNum].pos  = pos;

	return NoError;
}


// -----------------------------------------------------------------------------
// Set a run of a paragraph
// -----------------------------------------------------------------------------
static GSErrCode SetRun (API_ParagraphType** paragraph, UInt32 parNum, UInt32 runNum, Int32 from, Int32 range, short pen, unsigned short faceBits,
						 short font, unsigned short effectBits, double size)
{
	if (paragraph == NULL || parNum >= (BMhGetSize (reinterpret_cast<GSHandle> (paragraph)) / sizeof (API_ParagraphType)))
		return APIERR_BADPARS;

	if (runNum >= BMGetPtrSize (reinterpret_cast<GSPtr> ((*paragraph)[parNum].run)) / sizeof (API_RunType))
		return APIERR_BADPARS;

	(*paragraph)[parNum].run[runNum].from	    = from;
	(*paragraph)[parNum].run[runNum].range	    = range;
	(*paragraph)[parNum].run[runNum].pen	    = pen;
	(*paragraph)[parNum].run[runNum].faceBits   = faceBits;
	(*paragraph)[parNum].run[runNum].font	    = font;
	(*paragraph)[parNum].run[runNum].effectBits = effectBits;
	(*paragraph)[parNum].run[runNum].size	    = size;

	return NoError;
}


// -----------------------------------------------------------------------------
// Set EOL array of a paragraph
// -----------------------------------------------------------------------------
static GSErrCode SetEOL (API_ParagraphType** paragraph, UInt32 parNum, UInt32 eolNum, Int32 offset)
{
	if (paragraph == NULL || parNum >= (BMhGetSize (reinterpret_cast<GSHandle> (paragraph)) / sizeof (API_ParagraphType)))
		return APIERR_BADPARS;

	if (eolNum >= BMGetPtrSize (reinterpret_cast<GSPtr> ((*paragraph)[parNum].eolPos)) / sizeof (Int32))
		return APIERR_BADPARS;

	if (offset < 0)
		return APIERR_BADPARS;

	(*paragraph)[parNum].eolPos[eolNum] = offset;

	return NoError;
}


// -----------------------------------------------------------------------------
// Create a multistyle text
// -----------------------------------------------------------------------------
static GSErrCode CreateMultiTextElement (const API_Coord& pos, double scale = 1.0, const API_Guid& renFiltGuid = APINULLGuid)
{
	API_Element			element;
	API_ElementMemo		memo;
	GSErrCode			err;
	const char			*text = { "This multistyle\nword was created\nby the Element Test example project." };
	Int32				numOfParagraphs = 3;

	BNZeroMemory (&element, sizeof (element));
	BNZeroMemory (&memo, sizeof (memo));
	memo.textContent = BMhAll (Strlen32 (text) + 1);
	if (memo.textContent == NULL)
		return APIERR_MEMFULL;

	strcpy (*memo.textContent, text);

	element.header.typeID = API_TextID;

	memo.paragraphs = reinterpret_cast<API_ParagraphType**> (BMhAll (numOfParagraphs * sizeof (API_ParagraphType)));
	if (memo.paragraphs == NULL) {
		ACAPI_DisposeElemMemoHdls (&memo);
		return APIERR_MEMFULL;
	}

	err = ACAPI_Element_GetDefaults (&element, NULL);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetDefaults (Text)", err);
		ACAPI_DisposeElemMemoHdls (&memo);
		return err;
	}

	element.header.renovationStatus = API_ExistingStatus;
	element.header.renovationFilterGuid = renFiltGuid;		// APINULLGuid is handled internally

	element.text.loc.x = pos.x;
	element.text.loc.y = pos.y;
	element.text.anchor = APIAnc_LB;
	element.text.multiStyle = true;
	element.text.nonBreaking = false;
	element.text.useEolPos = true;
	element.text.width = 150 * scale;
	element.text.charCode = CC_Default;

	err = SetParagraph (memo.paragraphs, 0, 0, 15, APIJust_Left, 2.0 * scale, 0, 0, -1.0, 1, 2, 1);
	if (err == NoError) {
		err = SetRun (memo.paragraphs, 0, 0, 0, 5, 3, APIFace_Plain, element.text.font, 0, 4.0 * scale);		// "This "
		if (err == NoError)
			err = SetRun (memo.paragraphs, 0, 1, 5, 10, 5, APIFace_Bold, element.text.font, 0, 4.5 * scale);	// "multistyle"
		if (err == NoError)
			err = SetTab (memo.paragraphs, 0, 0, APITab_Left, 2.0 * scale);
		if (err == NoError)
			err = SetEOL (memo.paragraphs, 0, 0, 14);
	}

	if (err == NoError) {
		err = SetParagraph (memo.paragraphs, 1, 16, 16, APIJust_Right, 2.0 * scale, 0, 0, -1.5, 1, 2, 1);
		if (err == NoError) {
			err = SetRun (memo.paragraphs, 1, 0, 0, 9, 7, APIFace_Plain, element.text.font, 0, 4.0 * scale);	// "word was "
			if (err == NoError)
				err = SetRun (memo.paragraphs, 1, 1, 9, 7, 9, APIFace_Italic, element.text.font, APIEffect_StrikeOut, 3.0 * scale);
			if (err == NoError)																					// "created"
				err = SetTab (memo.paragraphs, 1, 0, APITab_Left, 0.0);
			if (err == NoError)
				err = SetEOL (memo.paragraphs, 1, 0, 15);
		}
	}

	if (err == NoError) {
		err = SetParagraph (memo.paragraphs, 2, 33, 36, APIJust_Center, 0, 0, 0, -1.0, 1, 3, 2);
		if (err == NoError) {
			err = SetRun (memo.paragraphs, 2, 0, 0, 19, 11, APIFace_Underline, element.text.font, 0, 4.0 * scale);
			if (err == NoError)																					// "by the Element Test"
				err = SetRun (memo.paragraphs, 2, 1, 19, 9, 13, APIFace_Plain, element.text.font, APIEffect_StrikeOut, 6.0 * scale);
			if (err == NoError)																					// " example "
				err = SetRun (memo.paragraphs, 2, 2, 28, 8, 15, APIFace_Bold, element.text.font, APIEffect_SubScript, 5.0 * scale);
			if (err == NoError)																					// "project."
				err = SetTab (memo.paragraphs, 2, 0, APITab_Left, 0.0);
			if (err == NoError)
				err = SetEOL (memo.paragraphs, 2, 0, 21);
			if (err == NoError)
				err = SetEOL (memo.paragraphs, 2, 1, 35);
		}
	}

	if (err == NoError) {
		err = ACAPI_Element_Create (&element, &memo);
		if (err != NoError)
			ErrorBeep ("ACAPI_Element_Create (text)", err);
	}

	ACAPI_DisposeElemMemoHdls (&memo);

	return err;
}		// CreateMultiTextElement


// -----------------------------------------------------------------------------
// Place a multistyle text element
// -----------------------------------------------------------------------------
void	Do_CreateWord (const API_Guid& renFiltGuid)
{
	API_Coord	c;

	if (!ClickAPoint ("Enter the left bottom position of the text", &c))
		return;

	CreateMultiTextElement (c, 1.0, renFiltGuid);

	return;
}		// Do_CreateWord


// -----------------------------------------------------------------------------
// Create a drawing element
// -----------------------------------------------------------------------------
void	Do_CreateDrawing (const API_Guid& elemGuid, API_Coord* pos /*= NULL*/)
{
	API_Element		element;
	API_ElementMemo	memo;
	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

	element.header.typeID = API_DrawingID;
	GSErrCode err = ACAPI_Element_GetDefaults (&element, &memo);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetDefaults (drawing)", err);
		ACAPI_DisposeElemMemoHdls (&memo);
		return;
	}

	if (pos != NULL)
		element.drawing.pos = *pos;

	 if (pos == NULL &&
		 !ClickAPoint ("Enter the left bottom position of the drawing", &element.drawing.pos)) {
	 	ACAPI_DisposeElemMemoHdls (&memo);
		return;
	}

	// create drawing data
	GSPtr		drawingData = NULL;
	double		scale = 1.0;
	API_PenType	**pens = NULL;

	short			count;
	ACAPI_Attribute_GetNum (API_PenTableID, &count);
	for (short ii = 1; ii <= count; ++ii) {
		API_Attribute	attr;
		BNZeroMemory (&attr, sizeof (API_Attribute));
		attr.header.typeID = API_PenTableID;
		if (ACAPI_Attribute_Get (&attr) == NoError && attr.penTable.inEffectForModel) {
			API_AttributeDefExt		defs;
			ACAPI_Attribute_GetDefExt (API_PenTableID, ii, &defs);
			pens = defs.penTable_Items;
			break;
		}
	}

	err = ACAPI_Database (APIDb_StartDrawingDataID, &scale, (void*) pens);
	if (pens != NULL)
		BMhKill (reinterpret_cast<GSHandle*> (&pens));
	if (err == NoError) {
		API_Element	elemInDrawing;
		BNZeroMemory (&elemInDrawing, sizeof (API_Element));
		elemInDrawing.header.typeID = API_LineID;
		elemInDrawing.header.layer = 2;
		elemInDrawing.line.linePen = 3;
		elemInDrawing.line.ltypeInd = 5;

		elemInDrawing.line.begC.x = 0.3;
		elemInDrawing.line.begC.y = 0.2;
		elemInDrawing.line.endC.x = 0.4;
		elemInDrawing.line.endC.y = 0.3;
		err = ACAPI_Element_Create (&elemInDrawing, NULL);		// line is created into the drawing

		if (err == NoError) {
			elemInDrawing.line.linePen = 6;
			elemInDrawing.line.ltypeInd = 10;
			elemInDrawing.line.begC.x = 0.2;
			elemInDrawing.line.begC.y = 0.3;
			elemInDrawing.line.endC.x = 0.3;
			elemInDrawing.line.endC.y = 0.4;
			err = ACAPI_Element_Create (&elemInDrawing, NULL);	// another line is created into the drawing
		}

		if (err == NoError) {
			BNZeroMemory (&elemInDrawing, sizeof (API_Element));
			elemInDrawing.header.typeID = API_CircleID;
			elemInDrawing.header.layer = 5;
			elemInDrawing.circle.linePen = 4;
			elemInDrawing.circle.ltypeInd = 2;

			elemInDrawing.circle.origC.x = 0.25;
			elemInDrawing.circle.origC.y = 0.15;
			elemInDrawing.circle.r = 0.09;
			elemInDrawing.circle.ratio = 0.8;
			err = ACAPI_Element_Create (&elemInDrawing, NULL);	// ellipse is created into the drawing
		}

		if (err == NoError) {
			API_Coord	pos;
			pos.x = 0.02;
			pos.y = 0.02;
			err = CreateMultiTextElement (pos);					// multitext element is created into the drawing
		}

		if (err == NoError)
			err = ACAPI_Database (APIDb_StopDrawingDataID, &drawingData, &element.drawing.bounds);
	}

	if (err != NoError || drawingData == NULL) {
		ACAPI_DisposeElemMemoHdls (&memo);
		return;
	}

	ACAPI_DisposeElemMemoHdls (&memo);

	// create drawing element
	element.header.typeID = API_DrawingID;
	element.header.guid = elemGuid;
	CHCopyC ("Drawing element from Element Test test add-on", element.drawing.name);
	element.drawing.numberingType = APINumbering_ByLayout;
	element.drawing.nameType = APIName_CustomName;
	element.drawing.ratio = 1.0;
	element.drawing.anchorPoint = APIAnc_LB;
	element.drawing.isCutWithFrame = true;
	element.drawing.penTableUsageMode = APIPenTableUsageMode_UseOwnPenTable;

	double dx = 0.05 * (element.drawing.bounds.xMax - element.drawing.bounds.xMin);		// add 10% padding
	double dy = 0.05 * (element.drawing.bounds.yMax - element.drawing.bounds.yMin);
	element.drawing.bounds.xMax += dx;
	element.drawing.bounds.yMax += dy;
	element.drawing.bounds.xMin -= dx;
	element.drawing.bounds.yMin -= dy;

	memo.drawingData = drawingData;

	{	// clip with polygon
		double width	= element.drawing.bounds.xMax - element.drawing.bounds.xMin;
		double height	= element.drawing.bounds.yMax - element.drawing.bounds.yMin;
		double offsX	= element.drawing.pos.x;
		double offsY	= element.drawing.pos.y;
		switch (element.drawing.anchorPoint) {
			case APIAnc_LT:		offsY -= (element.drawing.bounds.yMax - element.drawing.bounds.yMin);
								break;
			case APIAnc_MT:		offsX -= (element.drawing.bounds.xMax - element.drawing.bounds.xMin) / 2.0;
								offsY -= (element.drawing.bounds.yMax - element.drawing.bounds.yMin);
								break;
			case APIAnc_RT:		offsX -= (element.drawing.bounds.xMax - element.drawing.bounds.xMin);
								offsY -= (element.drawing.bounds.yMax - element.drawing.bounds.yMin);
								break;
			case APIAnc_LM:		offsY -= (element.drawing.bounds.yMax - element.drawing.bounds.yMin) / 2.0;
								break;
			case APIAnc_MM:		offsX -= (element.drawing.bounds.xMax - element.drawing.bounds.xMin) / 2.0;
								offsY -= (element.drawing.bounds.yMax - element.drawing.bounds.yMin) / 2.0;
								break;
			case APIAnc_RM:		offsX -= (element.drawing.bounds.xMax - element.drawing.bounds.xMin);
								offsY -= (element.drawing.bounds.yMax - element.drawing.bounds.yMin) / 2.0;
								break;
			case APIAnc_LB:		break;
			case APIAnc_MB:		offsX -= (element.drawing.bounds.xMax - element.drawing.bounds.xMin) / 2.0;
								break;
			case APIAnc_RB:		offsX -= (element.drawing.bounds.xMax - element.drawing.bounds.xMin);
								break;
		}

		element.drawing.poly.nCoords = 6;
		element.drawing.poly.nSubPolys = 1;
		element.drawing.poly.nArcs = 0;
		memo.coords = (API_Coord**) BMAllocateHandle ((element.drawing.poly.nCoords + 1) * sizeof (API_Coord), ALLOCATE_CLEAR, 0);
		memo.pends = (Int32**) BMAllocateHandle ((element.drawing.poly.nSubPolys + 1) * sizeof (Int32), ALLOCATE_CLEAR, 0);
		if (memo.coords != NULL && memo.pends != NULL) {
			(*memo.coords)[1].x = offsX;
			(*memo.coords)[1].y = offsY;
			(*memo.coords)[2].x = offsX + width;
			(*memo.coords)[2].y = offsY + 0.2 * height;
			(*memo.coords)[3].x = offsX + width;
			(*memo.coords)[3].y = offsY + height;
			(*memo.coords)[4].x = offsX + 0.8 * width;
			(*memo.coords)[4].y = offsY + 0.8 * height;
			(*memo.coords)[5].x = offsX;
			(*memo.coords)[5].y = offsY + height;
			(*memo.coords)[6].x = (*memo.coords)[1].x;
			(*memo.coords)[6].y = (*memo.coords)[1].y;

			(*memo.pends)[0] = 0;
			(*memo.pends)[1] = element.drawing.poly.nCoords;
		}
	}

	// add title
	API_LibPart linearTitle;

	BNZeroMemory (&linearTitle, sizeof (linearTitle));
	GS::UniString	uName ("Linear Drawing Title 10");
	GS::ucscpy (linearTitle.docu_UName, uName.ToUStr ());


	err = ACAPI_LibPart_Search (&linearTitle, false);
	if (err == NoError) {
		delete linearTitle.location;

		element.drawing.title.libInd = linearTitle.index;
		element.drawing.title.useUniformTextFormat = true;
		element.drawing.title.useUniformSymbolPens = true;
		element.drawing.title.pen = 88;

		double           aParam   = 0.0;
		double           bParam   = 0.0;
		Int32             paramNum = 0;
		API_AddParType** addPars  = NULL;

		err = ACAPI_LibPart_GetParams (linearTitle.index, &aParam, &bParam, &paramNum, &addPars);
		if (err == NoError) {
			bool customFlagModified = false;
			bool customNameModified = false;

			for (Int32 i = 0; i < paramNum; i++) {
				if (CHCompareCStrings ("gs_drawing_name_custom", (*addPars)[i].name) == 0) {
					(*addPars)[i].value.real = 1.0;
					customFlagModified = true;
				}
				if (CHCompareCStrings ("gs_drawing_name_custom_text", (*addPars)[i].name) == 0) {
					GS::UniString tmpUStr ("TestName");
					GS::ucscpy ((*addPars)[i].value.uStr, tmpUStr.ToUStr());
					customNameModified = true;
				}
				if (customFlagModified && customNameModified) {
					break;
				}
			}

			memo.params = addPars;
		}
	}

	err = ACAPI_Element_Create (&element, &memo);

	ACAPI_DisposeElemMemoHdls (&memo);

	if (err != NoError)
		ErrorBeep ("ACAPI_Element_Create (drawing)", err);

	return;
}		// Do_CreateDrawing


// -----------------------------------------------------------------------------
// Create a drawing element on a layout with a link to a floor plan view
// -----------------------------------------------------------------------------
void	Do_CreateDrawingFromPlanView (void)
{
	API_Element element;
	BNZeroMemory (&element, sizeof (API_Element));

	element.header.typeID = API_DrawingID;
	GSErrCode err = ACAPI_Element_GetDefaults (&element, NULL);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetDefaults (drawing)", err);
		return;
	}

	// create drawing data
	API_NavigatorItem** navItems = NULL;
	API_NavigatorItem navItem;
	BNZeroMemory (&navItem, sizeof (API_NavigatorItem));
	navItem.mapId = API_PublicViewMap;
	navItem.itemType = API_StoryNavItem;

	err = ACAPI_Environment (APIEnv_SearchNavigatorItemID, &navItem, &navItems);
	if (err != NoError || navItems == NULL)
		return;

	Int32 n = BMhGetSize ((GSHandle) navItems) / Sizeof32 (API_NavigatorItem);
	if (n > 0) {
		element.drawing.drawingGuid = (*navItems)[n - 1].guid;	// link to the last floor plan view
	}
	BMhKill ((GSHandle *) &navItems);

	// create drawing element
	element.header.typeID = API_DrawingID;
	CHCopyC ("Drawing element from Element Test test add-on", element.drawing.name);
	element.drawing.nameType = APIName_CustomName;
	element.drawing.ratio = 1.0;
	element.drawing.anchorPoint = APIAnc_MM;
	element.drawing.pos.x = 0.5;
	element.drawing.pos.y = 0.0;

	API_ElementMemo memo;
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

	err = ACAPI_Element_Create (&element, &memo);

	ACAPI_DisposeElemMemoHdls (&memo);

	if (err != NoError)
		ErrorBeep ("ACAPI_Element_Create (drawing)", err);

	return;
}		// Do_CreateDrawingFromPlanView


// -----------------------------------------------------------------------------
// Create a drawing element from selected elements
// -----------------------------------------------------------------------------
void	Do_CreateDrawingFromSelection (void)
{
	API_SelectionInfo 	selectionInfo;
	API_Element			element;
	API_ElementMemo		memo;
	API_Neig**			selNeig;
	GSErrCode			err;

	// Get selection
	err = ACAPI_Selection_Get (&selectionInfo, &selNeig, true);
	if (err != NoError && err != APIERR_NOSEL) {
		ErrorBeep ("ACAPI_Selection_GetInfo", err);
		return;
	}

	if (selectionInfo.typeID == API_SelEmpty) {
		WriteReport_Alert ("No selected elements");
		return;
	}

	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);

	// create drawing data
	double		scale = 1.0;

	API_PenType	**pens = reinterpret_cast<API_PenType **> (BMhAllClear (256 * sizeof (API_PenType)));
	if (pens != NULL) {
		API_PenType		*pen = *pens;
		for (short ii = 1; ii <= 255; ii++, pen++) {
			pen->head.typeID = API_PenID;
			pen->head.index  = ii;
			pen->rgb.f_red   = ii / 255.0;
			pen->rgb.f_green = ii / 255.0;
			pen->rgb.f_blue  = ii / 255.0;
			pen->width       = 0.025;
		}
	}

	err = ACAPI_Database (APIDb_StartDrawingDataID, &scale, (void *) pens);
	BMhKill (reinterpret_cast<GSHandle*> (&pens));

	for (Int32 i = 0; i < selectionInfo.sel_nElemEdit; i++) {
		// Get selected element
		BNZeroMemory (&element, sizeof(element));
		BNZeroMemory (&memo, sizeof(memo));

		element.header.guid = (*selNeig)[i].guid;
		if (ACAPI_Element_Get (&element) != NoError)
			continue;

		if (element.header.hasMemo && ACAPI_Element_GetMemo (element.header.guid, &memo) != NoError)
			continue;

		// Add to drawing
		err = ACAPI_Element_Create (&element, element.header.hasMemo ? &memo : NULL);
		if (err != NoError) {
			char sBuf[256];
			sprintf (sBuf, "Element type %s, failed to add to drawing: %d\nOnly 2D elements can be placed on drawing.", ElemID_To_Name (element.header.typeID), (int) err);
			WriteReport_Alert (sBuf);
		}

		ACAPI_DisposeElemMemoHdls (&memo);
	}

	BMKillHandle ((GSHandle *) &selNeig);

	// Initialize drawing element
	BNZeroMemory (&element, sizeof (API_Element));

	element.header.typeID = API_DrawingID;
	err = ACAPI_Element_GetDefaults (&element, NULL);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetDefaults (drawing)", err);
		return;
	}

	// Get the drawing data
	GSPtr drawingData = NULL;

	err = ACAPI_Database (APIDb_StopDrawingDataID, &drawingData, &element.drawing.bounds);
	if (err != NoError || drawingData == NULL) {
		ErrorBeep ("APIDb_StopDrawingDataID", err);
		return;
	}

	if (!ClickAPoint ("Enter the left bottom position of the drawing", &element.drawing.pos)) {
		BMKillPtr ((GSPtr *) &drawingData);
		return;
	}

	// create drawing element
	element.header.typeID = API_DrawingID;
	CHCopyC ("Drawing element from Element Test test add-on", element.drawing.name);
	element.drawing.nameType = APIName_CustomName;
	element.drawing.ratio = 1.0;
	element.drawing.anchorPoint = APIAnc_LB;
	element.drawing.isCutWithFrame = true;

	double dx = 0.05 * (element.drawing.bounds.xMax - element.drawing.bounds.xMin);		// add 10% padding
	double dy = 0.05 * (element.drawing.bounds.yMax - element.drawing.bounds.yMin);
	element.drawing.bounds.xMax += dx;
	element.drawing.bounds.yMax += dy;
	element.drawing.bounds.xMin -= dx;
	element.drawing.bounds.yMin -= dy;

	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	memo.drawingData = drawingData;

	err = ACAPI_Element_Create (&element, &memo);

	ACAPI_DisposeElemMemoHdls (&memo);

	if (err != NoError)
		ErrorBeep ("ACAPI_Element_Create (drawing)", err);

	return;
}		// Do_CreateDrawingFromSelection


// -----------------------------------------------------------------------------
// Select elements by clicking on them
//   - select other elements of the same group also
//   - check locked layer
//   - check hidden layer
// Deselect all if no element was clicked
// -----------------------------------------------------------------------------
void		Do_SelectElems (void)
{
	API_Neig		theNeig;
	GSErrCode		err;

	if (!ClickAnElem ("Click an elem to select", API_ZombieElemID, &theNeig)) {
		WriteReport_Alert ("No element was clicked");
		return;
	}

	API_Neig** neigHdl = (API_Neig**) BMhAll (sizeof (API_Neig));

	if (neigHdl != NULL) {
		**neigHdl = theNeig;
		err = ACAPI_Element_Select (neigHdl, 1, true);
		BMhKill ((GSHandle*) &neigHdl);
	} else
		err = APIERR_MEMFULL;

	if (err != NoError)
		ErrorBeep ("ACAPI_Element_Select", err);

	return;
}		// Do_SelectElems


// -----------------------------------------------------------------------------
// Delete elements by clicking on them
//   - delete the other elements in the same group also
//   - check locked layer
//   - check hidden layer
//   - associated elements should be deleted also
// -----------------------------------------------------------------------------
void		Do_DeleteElems (void)
{
	Int32				nItem;
	API_Elem_Head		**elemHeads;
	GSErrCode			err;

	elemHeads = ClickElements_ElemHead ("Click elements to delete", API_ZombieElemID, &nItem);
	if (elemHeads == NULL) {
		WriteReport_Alert ("No element was clicked");
		return;
	}

	err = ACAPI_Element_Delete (elemHeads, nItem);
	if (err != NoError)
		ErrorBeep ("ACAPI_Element_Delete", err);

	BMKillHandle ((GSHandle *) &elemHeads);

	return;
}		// Do_DeleteElems


// -----------------------------------------------------------------------------
// Pick up element defaults from an instance
//   - check the appropriate setting dialog after execution
//   - check all element types
// -----------------------------------------------------------------------------
void		Do_PickupElem (void)
{
	API_ElemTypeID		typeID;
	API_Guid			guid;
	API_Element			element, mask;
	API_ElementMemo		memo;
	API_SubElemMemoMask	marker;
	GSErrCode			err;

	if (!ClickAnElem ("Click an element", API_ZombieElemID, NULL, &typeID, &guid)) {
		WriteReport_Alert ("No element was clicked");
		return;
	}

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&marker, sizeof (API_SubElemMemoMask));

	ACAPI_ELEMENT_MASK_SETFULL (mask);
	ACAPI_ELEMENT_MASK_SETFULL (marker.mask);

	element.header.guid		= guid;
	marker.subElem.header.typeID = API_ObjectID;

	err = ACAPI_Element_Get (&element);
	if (err == NoError && element.header.hasMemo)
		err = ACAPI_Element_GetMemo (element.header.guid, &memo);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Get/Memo", err);
		return;
	}

	switch (typeID) {
		case API_WindowID:
			marker.subElem.header.guid = element.window.openingBase.markGuid;
			break;
		case API_DoorID:
			marker.subElem.header.guid = element.door.openingBase.markGuid;
			break;
		case API_SkylightID:
			marker.subElem.header.guid = element.skylight.openingBase.markGuid;
			break;
		case API_CutPlaneID:
			marker.subElem.header.guid = element.cutPlane.segment.begMarkerId;
			break;
		case API_ElevationID:
			marker.subElem.header.guid = element.elevation.segment.midMarkerId;
			break;
		case API_InteriorElevationID:
			if (element.interiorElevation.nSegments > 0 && memo.segments != NULL) {
				marker.subElem.header.guid = memo.segments[0].endMarkerId;
				element.interiorElevation.segment = memo.segments[0];
			}
			break;
		case API_DetailID:
			marker.subElem.header.guid = element.detail.markId;
			break;
		case API_ChangeMarkerID:
			marker.subElem.header.guid = element.changeMarker.markerGuid;
			break;
		case API_WorksheetID:
			marker.subElem.header.guid = element.worksheet.markId;
			break;
		default:
			err = ACAPI_Element_ChangeDefaults (&element, &memo, &mask);
			if (err != NoError)
				ErrorBeep ("ACAPI_Element_ChangeDefaults", err);
			ACAPI_DisposeElemMemoHdls (&memo);
			return;
	}

	if (marker.subElem.header.guid != APINULLGuid) {
		err = ACAPI_Element_Get (&marker.subElem);
		if (err == NoError)
			err = ACAPI_Element_GetMemo (marker.subElem.header.guid, &marker.memo);
		if (err != NoError) {
			ErrorBeep ("ACAPI_Element_Get/Memo", err);
			return;
		}

		marker.subType = APISubElemMemoMask_MainMarker;
		err = ACAPI_Element_ChangeDefaultsExt (&element, &memo, &mask, 1UL, &marker);
		if (err != NoError)
			ErrorBeep ("ACAPI_Element_ChangeDefaults", err);
	} else {
		marker.subElem.object.libInd = 0;
		ACAPI_ELEMENT_MASK_SET (marker.mask, API_ObjectType, libInd);
		marker.subType = (API_SubElemMemoMaskType) (APISubElemMemoMask_MainMarker | APISubElemMemoMask_NoParams);
		err = ACAPI_Element_ChangeDefaultsExt (&element, &memo, &mask, 1UL, &marker);
		if (err != NoError)
			ErrorBeep ("ACAPI_Element_ChangeDefaults", err);
	}

	ACAPI_DisposeElemMemoHdls (&memo);
	ACAPI_DisposeElemMemoHdls (&marker.memo);
}		// Do_PickupElem


// -----------------------------------------------------------------------------
// Change the clicked element settings to match the default
//   - type is given by the first clicked element
//   - check the appropriate setting dialog after execution
// -----------------------------------------------------------------------------
void		Do_ChangeElem (void)
{
	API_Element			element,  mask;
	API_ElementMemo		memo;
	API_SubElemMemoMask	markers[2];
	API_ElemTypeID		typeID;
	API_Elem_Head		head;
	API_Guid			guid;
	GSErrCode			err;

	if (!ClickAnElem ("Click an element", API_ZombieElemID, NULL, &typeID, &guid)) {
		WriteReport_Alert ("No element was clicked");
		return;
	}

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&markers, 2 * sizeof (API_SubElemMemoMask));

	ACAPI_ELEMENT_MASK_SETFULL (mask);

	element.header.typeID = typeID;
	element.header.guid   = guid;
	err = ACAPI_Element_Get (&element);	//for variationID
	if (err != NoError)
		return;

	head = element.header;

	if (typeID == API_WindowID || typeID == API_DoorID || typeID == API_SkylightID ||
		typeID == API_CutPlaneID || typeID == API_ElevationID || typeID == API_InteriorElevationID ||
		typeID == API_DetailID || typeID == API_ChangeMarkerID || typeID == API_WorksheetID)
	{
			markers[0].subType = APISubElemMemoMask_MainMarker;
			markers[1].subType = APISubElemMemoMask_SHMarker;
			err = ACAPI_Element_GetDefaultsExt (&element, &memo, 2UL, markers);
			if (err == NoError) {
				element.header = head;
				err = ACAPI_Element_ChangeExt (&element, &element, &memo, APIMemoMask_AddPars,
											   2UL, markers, /* withDel */ true, ACAPI_ELEMENT_CHANGEEXT_ALLSEGMENTS);
			}
	} else {
		err = ACAPI_Element_GetDefaults (&element, &memo);
		if (err == NoError) {
			element.header = head;
			err = ACAPI_Element_Change (&element, &element, &memo, APIMemoMask_AddPars, true);
		}
	}

	ACAPI_DisposeElemMemoHdls (&memo);
	ACAPI_DisposeElemMemoHdls (&markers[0].memo);
	ACAPI_DisposeElemMemoHdls (&markers[1].memo);

	return;
}		// Do_ChangeElem


// -----------------------------------------------------------------------------
// Draw (explode) the clicked element from primitives
//   - shape primitives listed in the report window
//   - primitives are returned as ArchiCAD would draw the given element onto the screen
// -----------------------------------------------------------------------------
void		Do_ExplodeElem (void)
{
	API_ElemTypeID		typeID;
	API_Guid			guid;
	GSErrCode			err;

	if (!ClickAnElem ("Click an elem to explode", API_ZombieElemID, NULL, &typeID, &guid)) {
		WriteReport_Alert ("No element was clicked");
		return;
	}

	API_Elem_Head elemHead;
	BNZeroMemory (&elemHead, sizeof (API_Elem_Head));
	elemHead.typeID = typeID;
	elemHead.guid   = guid;
	err = ACAPI_Element_ShapePrims (elemHead, Draw_ShapePrims);
	if (err != NoError)
		ErrorBeep ("ACAPI_Element_ShapePrims", err);

	return;
}		// Do_ExplodeElem


// -----------------------------------------------------------------------------
// Copy the clicked element to the story above
//   - the story must exist
//   - it must be in our workspace
//   - associated elements are not handled
// There are some elements which will be refused
//   - Doors/Windows: openings must be on the same floor as the parent wall
//   - Dimensions:    the same
//   - Auto Labels:   the same
// -----------------------------------------------------------------------------
void		Do_CopyElem (void)
{
	API_Element			element;
	API_ElementMemo		memo;
	GSErrCode			err;

	API_ElemTypeID	typeID;
	API_Guid		guid;
	if (!ClickAnElem ("Click an element", API_ZombieElemID, NULL, &typeID, &guid)) {
		WriteReport_Alert ("No element was clicked");
		return;
	}

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

	element.header.typeID	= typeID;
	element.header.guid		= guid;

	err = ACAPI_Element_Get (&element);
	if (err == NoError)
		err = ACAPI_Element_GetMemo (element.header.guid, &memo);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Get/Memo", err);
		return;
	}

	if (err == NoError) {
		element.header.floorInd ++;
		err = ACAPI_Element_Create (&element, &memo);
		if (err != NoError)
			ErrorBeep ("ACAPI_Element_Create", err);
	}

	// ---- consistency check
	if (err == NoError)
		CompareElems (element, memo);
	// ----

	ACAPI_DisposeElemMemoHdls (&memo);
}		// Do_CopyElem


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void		Do_PickupProperties (void)
{
	API_ElemTypeID		typeID;
	API_Guid			guid;
	API_Elem_Head		head;
	GSErrCode			err;

	if (!ClickAnElem ("Click an element", API_ZombieElemID, NULL, &typeID, &guid)) {
		WriteReport_Alert ("No element was clicked");
		return;
	}

	Int32				inviduallyLibInd = 0;
	bool				criteria = false;

	BNZeroMemory (&head, sizeof (head));
	head.typeID = typeID;
	head.guid = guid;

	err = ACAPI_Element_GetLinkedProperties (&head, &criteria, &inviduallyLibInd, NULL, NULL);
	if (err == NoError) {

		BNZeroMemory (&head, sizeof (head));
		head.typeID = typeID;
		//index = 0 //defaults
		err = ACAPI_Element_SetLinkedProperties (&head, criteria, inviduallyLibInd);
	}
	return;
}		// Do_PickupProperties


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void		Do_FillProperties (void)
{
	API_ElemTypeID		typeID;
	API_Guid			guid;
	API_Elem_Head		head;
	GSErrCode			err;

	if (!ClickAnElem ("Click an element", API_ZombieElemID, NULL, &typeID, &guid)) {
		WriteReport_Alert ("No element was clicked");
		return;
	}

	Int32				inviduallyLibInd = 0;
	bool				criteria = false;

	BNZeroMemory (&head, sizeof (head));
	head.typeID = typeID;
	//head.guid = guid;	// defaults

	err = ACAPI_Element_GetLinkedProperties (&head, &criteria, &inviduallyLibInd, NULL, NULL);

	if (err == NoError) {
		BNZeroMemory (&head, sizeof (head));
		head.typeID	= typeID;
		head.guid	= guid;

		err = ACAPI_Element_SetLinkedProperties (&head, criteria, inviduallyLibInd);
	}
	return;
}		// Do_FillProperties


// -----------------------------------------------------------------------------
// Dump element category value
// -----------------------------------------------------------------------------
void		Do_DumpElemCategoryValue (const API_ElemCategoryValue& elemCategoryValue)
{
	WriteReport ("   %s   : %s (%s)", GS::UniString (elemCategoryValue.category.name).ToCStr ().Get (), GS::UniString (elemCategoryValue.name).ToCStr ().Get (), APIGuidToString (elemCategoryValue.guid).ToCStr ().Get ());
}


// -----------------------------------------------------------------------------
// Dump element's categories
// -----------------------------------------------------------------------------
void		Do_DumpElemCategories (const API_Guid& elemGuid, const API_ElemTypeID& typeID, const API_ElemVariationID& variationID, bool dumpDefaults)
{
	GSErrCode			err = NoError;

	GS::Array<API_ElemCategory> categoryList;
	ACAPI_Database (APIDb_GetElementCategoriesID, &categoryList);

	categoryList.Enumerate ([&] (const API_ElemCategory& category) {
		if (category.categoryID != API_ElemCategory_BRI) {
			API_ElemCategoryValue	elemCategoryValue;

			if (dumpDefaults) {
				err = ACAPI_Element_GetCategoryValueDefault (typeID, variationID, category, &elemCategoryValue);
				if (err == NoError)
					Do_DumpElemCategoryValue (elemCategoryValue);
				else
					ErrorBeep ("ACAPI_Element_GetCategoryValueDefault ()", err);
			} else {
				err = ACAPI_Element_GetCategoryValue (elemGuid, category, &elemCategoryValue);
				if (err == NoError)
					Do_DumpElemCategoryValue (elemCategoryValue);
				else
					ErrorBeep ("ACAPI_Element_GetCategoryValue ()", err);
			}
		}
	});
}


// -----------------------------------------------------------------------------
// Dump basic information of the clicked element into the Report Window
//   - check in Teamwork mode also
// -----------------------------------------------------------------------------
void		Do_DumpElem (API_Guid& renFiltGuid)
{
	API_ElemTypeID	typeID;
	API_Guid		guid;
	API_Element		element;
	API_ProjectInfo	projectInfo;
	API_SharingInfo	sharingInfo;
	GSErrCode		err;

	if (!ClickAnElem ("Click an elem to get info for", API_ZombieElemID, NULL, &typeID, &guid)) {
		WriteReport_Alert ("No element was clicked");
		return;
	}

	BNZeroMemory (&element, sizeof (API_Element));
	element.header.guid = guid;

	err = ACAPI_Element_Get (&element);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Get", err);
		return;
	}

	WriteReport ("Dump: %s GUID:%s", ElemID_To_Name (typeID), APIGuidToString (element.header.guid).ToCStr ().Get ());

	BNZeroMemory (&projectInfo, sizeof (API_ProjectInfo));
	ACAPI_Environment (APIEnv_ProjectID, &projectInfo, NULL);
	if (projectInfo.location != NULL)
		delete projectInfo.location;
	if (projectInfo.location_team != NULL)
		delete projectInfo.location_team;

	BNZeroMemory (&sharingInfo, sizeof (API_SharingInfo));
	if (projectInfo.userId != element.header.userId) {
		WriteReport ("  out of my workspace");
		err = ACAPI_Environment (APIEnv_ProjectSharingID, &sharingInfo, NULL);
		if (err != NoError) {
			ErrorBeep ("APIEnv_ProjectSharingID", err);
			return;
		}
	}

	GS::UniString	typeName, renovationStatusName, renovationFilterName;
	ACAPI_Goodies (APIAny_GetElemTypeNameID, (void *) (GS::IntPtr) element.header.typeID, &typeName);
	ACAPI_Goodies (APIAny_GetRenovationStatusNameID, (void *) (GS::IntPtr) element.header.renovationStatus, &renovationStatusName);
	ACAPI_Goodies (APIAny_GetRenovationFilterNameID, &element.header.renovationFilterGuid, &renovationFilterName);

	WriteReport ("  typeID     : (%03d) %s", element.header.typeID, static_cast<const char *> (typeName.ToCStr ()));
	WriteReport ("  guid       : %s", APIGuidToString (element.header.guid).ToCStr ().Get ());
	WriteReport ("  modiStamp  : %d", element.header.modiStamp);
	WriteReport ("  groupGuid  : %s", APIGuidToString (element.header.groupGuid).ToCStr ().Get ());
	WriteReport ("  floorInd   : %d", element.header.floorInd);
	WriteReport ("  layer      : %d", element.header.layer);
	WriteReport ("  hasMemo    : %d", element.header.hasMemo);
	WriteReport ("  drwIndex   : %d", element.header.drwIndex);
	WriteReport ("  userId     : %d", element.header.userId);
	WriteReport ("  lockId     : %d", element.header.lockId);
	WriteReport ("  renovation status name : %s", static_cast<const char *> (renovationStatusName.ToCStr ()));
	WriteReport ("  renovation filter name : %s", static_cast<const char *> (renovationFilterName.ToCStr ()));
	WriteReport ("  Categories :");
	Do_DumpElemCategories (element.header.guid, element.header.typeID, element.header.variationID, false);
	WriteReport ("  Default categories :");
	Do_DumpElemCategories (element.header.guid, element.header.typeID, element.header.variationID, true);

	if (projectInfo.teamwork) {
		DumpOwner ("owned by  :", &projectInfo, &sharingInfo, element.header.userId);
		DumpOwner ("locked by :", &projectInfo, &sharingInfo, element.header.lockId);
	}

	if (sharingInfo.users != NULL)
		BMhKill (reinterpret_cast<GSHandle*>(&sharingInfo.users));

	renFiltGuid = element.header.renovationFilterGuid;

	ACAPI_KeepInMemory (true);

	return;
}		// Do_DumpElem


// -----------------------------------------------------------------------------
// Set clicked element's following categories:
//  - Structural Function
//  - Position
//  - Element Classification
// -----------------------------------------------------------------------------
void		Do_SetElemCategories (bool changeDefaults)
{
	API_ElemTypeID	typeID;
	API_Guid		guid;
	API_Element		element;
	GSErrCode		err;

	if (!ClickAnElem ("Click an elem to set categories", API_ZombieElemID, NULL, &typeID, &guid)) {
		WriteReport_Alert ("No element was clicked");
		return;
	}

	BNZeroMemory (&element, sizeof (API_Element));
	element.header.guid = guid;

	err = ACAPI_Element_Get (&element);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Get", err);
		return;
	}

	WriteReport ("Do_SetElemCategories:");
	WriteReport ("Clicked element: %s GUID:%s", ElemID_To_Name (typeID), APIGuidToString (element.header.guid).ToCStr ().Get ());

	WriteReport ("  Old categories :");
	Do_DumpElemCategories (element.header.guid, element.header.typeID, element.header.variationID, false);
	WriteReport ("  Old default categories :");
	Do_DumpElemCategories (element.header.guid, element.header.typeID, element.header.variationID, true);

	GS::Array<API_ElemCategory> categoryList;
	ACAPI_Database (APIDb_GetElementCategoriesID, &categoryList);

	categoryList.Enumerate ([&] (const API_ElemCategory& category) {
		if (category.categoryID == API_ElemCategory_StructuralFunction || category.categoryID == API_ElemCategory_Position || category.categoryID == API_ElemCategory_ElementClassification) {
			GS::Array<API_ElemCategoryValue> categoryValueList;
			ACAPI_Database (APIDb_GetElementCategoryValuesID, (void*) &category, &categoryValueList);
			if (!categoryValueList.IsEmpty ()) {
				if (changeDefaults) {
					err = ACAPI_Element_SetCategoryValueDefault (element.header.typeID, element.header.variationID, category,	categoryValueList[categoryValueList.GetSize () - 1]);
					if (err != NoError)
						ErrorBeep ("ACAPI_Element_SetCategoryDefaults ()", err);
				} else {
					err = ACAPI_Element_SetCategoryValue (element.header.guid, category,	categoryValueList[0]);
					if (err != NoError)
						ErrorBeep ("ACAPI_Element_SetCategoryValue ()", err);
				}
			}
		}
	});

	WriteReport ("  New categories :");
	Do_DumpElemCategories (element.header.guid, element.header.typeID, element.header.variationID, false);
	WriteReport ("  New default categories :");
	Do_DumpElemCategories (element.header.guid, element.header.typeID, element.header.variationID, true);

	return;
}		// Do_SetElemCategories

// *****************************************************************************
// Source code for the Goodie Functions Add-On
// API Development Kit 19; Mac/Win
//
// Namespaces:			Contact person:
//		-None-
//
// [SG compatible] - Yes
// *****************************************************************************

#include "APIEnvir.h"
#define	_GOODIE_FUNCTIONS_TRANSL_


// ---------------------------------- Includes ---------------------------------

#include	<stdlib.h>
#include	<string.h>
#include	<math.h>

#include	"ACAPinc.h"				// also includes APIdefs.h
#include	"DG.h"
#include	"DGFileDlg.hpp"
#include	"FileTypeManager.hpp"
#include	"FileSystem.hpp"

#include	"APICommon.h"

#include	"UniString.hpp"
#include	"Array.hpp"

// ---------------------------------- Types ------------------------------------

// ---------------------------------- Variables --------------------------------
#define PROJ_INFO_AUTOTEXT_CUSTOM_TYPE_STR        "autotext-"


// ---------------------------------- Prototypes -------------------------------



// =============================================================================
//
// Main functions
//
// =============================================================================

// -----------------------------------------------------------------------------
// Select a BMP file with a file dialog
// -----------------------------------------------------------------------------

static bool	Select_a_BMPFile (IO::Location* fileLoc)
{
	bool			reply;
	DGTypePopupItem	popup;

	popup.text = L("BMP File");
	popup.extensions = L("bmp");
	popup.macType = 'BMP ';

	reply = DGGetOpenFile (fileLoc, 1, &popup, NULL, "Select a BMP file");

	return reply;
}		// Select_a_BMPFile


// -----------------------------------------------------------------------------
// Read a BMP file
// -----------------------------------------------------------------------------

static GSErrCode	Read_a_BMPFile (const IO::Location* fileLoc, GSHandle* buffer)
{
	UInt32	bufSize = 0;
	GSErrCode		err = NoError, errMem = NoError;

	IO::File	bmpFile (*fileLoc);
	err = bmpFile.GetStatus ();
	if (err != NoError) {
		ErrorBeep ("IO::File contructor failed", err);
		return err;
	}

	err = bmpFile.Open (IO::File::ReadMode);
	if (err == NoError)
		err = bmpFile.GetDataLength (&bufSize);
	if (err == NoError) {
		*buffer = BMAllocateHandle (bufSize, ALLOCATE_CLEAR, 0);
		errMem = BMError ();
		if (errMem == NoError) {
			err = bmpFile.ReadBin (**buffer, bufSize, &bufSize);
		} else
			ErrorBeep ("BMAllocateHandle", err);
	}

	err = bmpFile.Close ();

	if (err == NoError)
		err = errMem;

	return err;
}		// Read_a_BMPFile


// -----------------------------------------------------------------------------
// Write a GIF file
// -----------------------------------------------------------------------------

static GSErrCode	Write_a_GIFFile (const IO::Location* fileLoc, GSHandle buffer)
{
	IO::Location	gifDef;
	IO::Name		gifName;
	UInt32	bufSize;
	GSErrCode		err;

	// Make the specification of the GIF file (overwrite the extension)
	// Copy the file specification of the BMP file
	gifDef = *fileLoc;

	// Overwrite its extension with ".Gif"
	gifDef.GetLastLocalName (&gifName);
	// no name length check here
	if (gifName.HasExtension ()) {
		// replace extension
		gifName.DeleteExtension ();
		gifName.AppendExtension ("gif");
	} else {
		// append ".gif"
		gifName.Append (".gif");
	}
	gifDef.SetLastLocalName (gifName);

	bool contains = false;
	err = IO::fileSystem.Contains (gifDef, &contains);
	if (err != NoError || contains) {
		WriteReport_Alert ("The file \"%s\" already exists", static_cast<const char*> (gifName.ToString ().ToCStr ()));
		return err;
	}

	// Check if there is an existing GIF file there with the same name
	// default action is OnNotFound::Fail
	IO::File	gifFile (gifDef, IO::File::Create);
	err = gifFile.GetStatus ();
	if (err != NoError) {
		DBBREAK_STR ("File status error!");
		return err;
	}

	// Write the buffer into the file
	err = gifFile.Open (IO::File::WriteEmptyMode);
	if (err == NoError) {
		bufSize = BMGetHandleSize (buffer);
		err = gifFile.WriteBin (*buffer, bufSize, &bufSize);

		gifFile.Close ();
	}

	return err;
}		// Write_a_GIFFile


// -----------------------------------------------------------------------------
// Convert a Windows bitmap (BMP) file into GIF format
//	 - Select a BMP file using a file open dialog
//	 - The GIF file will be created to the same folder with the same name
// If a matching GIF file exists, it won't be overwritten
// -----------------------------------------------------------------------------

static void		Do_BMPtoGIF (void)
{
	IO::Location		fileLoc;
	API_PictureHandle	bmpHandle, gifHandle;
	GSErrCode			err;

	// Get a file specification in fileDef (on macintosh convert it to FISpec_Flat)
	if (!Select_a_BMPFile (&fileLoc))
		return;

	BNZeroMemory (&bmpHandle, sizeof (bmpHandle));
	bmpHandle.storageFormat = APIPictForm_Bitmap;

	// Read the specified file
	err = Read_a_BMPFile (&fileLoc, &bmpHandle.pictHdl);

	// Convert the BMP into a GIF
	if (err == NoError) {
		BNZeroMemory (&gifHandle, sizeof (gifHandle));
		gifHandle.storageFormat = APIPictForm_GIF;
		err = ACAPI_Goodies (APIAny_ConvertPictureID, &bmpHandle, &gifHandle);
		if (err == NoError) {
			if (gifHandle.pictHdl != NULL) {
				Write_a_GIFFile (&fileLoc, gifHandle.pictHdl);
			} else {
				DBBREAK_STR ("No error is reported by API, yet gifHandle.pictHdl is NULL");
			}
		}
		BMKillHandle (&gifHandle.pictHdl);
	}

	BMKillHandle (&bmpHandle.pictHdl);

	return;
}		// Do_BMPtoGIF


// -----------------------------------------------------------------------------
// Select the topmost polygonal element on the plan
// Elements processed by this function are:
//   - Slabs
//   - Roofs
//   - Meshes
//   - Hatches
//   - Rooms
// -----------------------------------------------------------------------------

static void		Do_SelectTopmostSlab (void)
{
	API_Elem_Head		head;
	API_Neig			**neigs;
	API_ElemSearchPars	pars;
	GSErrCode			err;

	BNZeroMemory (&pars, sizeof (API_ElemSearchPars));
	pars.typeID = API_SlabID;
	pars.z = 10.0;
	pars.filterBits = APIFilt_OnVisLayer | APIFilt_OnActFloor;

	if (!ClickAPoint ("Click somewhere", &pars.loc))
		return;

	BNZeroMemory (&head, sizeof (API_Elem_Head));
	head.typeID = API_SlabID;

	err = ACAPI_Goodies (APIAny_SearchElementByCoordID, &pars, &head.guid);
	if (err == NoError) {
		neigs = (API_Neig **) BMAllocateHandle (sizeof (API_Neig), ALLOCATE_CLEAR, 0);
		if (neigs == NULL)
			return;
		ElemHead_To_Neig (*neigs, &head);
		ACAPI_Element_Select (neigs, 1, true);
		BMKillHandle ((GSHandle *) &neigs);
	} else
		ErrorBeep ("APIAny_SearchElementByCoordID", err);

	return;
}		// Do_SelectTopmostElement


// -----------------------------------------------------------------------------
// Triangulate the clicked slab's polygon
// 	 - the edges of the triangles will be added as lines
// -----------------------------------------------------------------------------

static void		Do_TriangulateSlab (void)
{
	API_ElemTypeID		typeID;
	API_Guid			guid;
	API_Element			element, slab;
	API_ElementMemo		memo;
	API_Coord**			coords;
	Int32				i, n, j;
	GSErrCode			err = NoError;

	if (!ClickAnElem ("Click a slab to triangulate", API_SlabID, NULL, &typeID, &guid)) {
		WriteReport_Alert ("No slab was clicked");
		return;
	}

	BNZeroMemory (&slab, sizeof (API_Element));
	slab.header.typeID = API_SlabID;
	slab.header.guid   = guid;
	err = ACAPI_Element_Get (&slab);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Get", err);
		return;
	}

	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	err = ACAPI_Element_GetMemo (slab.header.guid, &memo, APIMemoMask_Polygon);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetMemo_Masked", err);
		return;
	}

	ACAPI_CallUndoableCommand ("Triangulate",
		[&] () -> GSErrCode {
			coords = NULL;

			GSErrCode err1 = ACAPI_Goodies (APIAny_TriangulatePolyID, &memo, &coords);
			if (err1 == NoError) {
				n = BMGetHandleSize ((GSHandle) coords) / (3 * sizeof (API_Coord));
				for (i = 0; i < n && err1 == NoError; i++) {
					BNZeroMemory (&element, sizeof (API_Element));
					element.header.typeID = API_LineID;
					element.header.layer  = 1;
					element.header.floorInd = slab.header.floorInd;
					element.line.ltypeInd = 1;
					for (j = 0; j < 3 && err1 == NoError; j++) {
						element.line.begC.x = (*coords) [3 * i + j].x;
						element.line.begC.y = (*coords) [3 * i + j].y;
						element.line.endC.x = (*coords) [3 * i + (j + 1) % 3].x;
						element.line.endC.y = (*coords) [3 * i + (j + 1) % 3].y;
						err1 = ACAPI_Element_Create (&element, NULL);
					}
				}
				BMKillHandle ((GSHandle *) &coords);
			} else
				ErrorBeep ("APIAny_TriangulatePolyID", err1);

			return err1;
		});

	return;
}		// Do_TriangulateSlab


// -----------------------------------------------------------------------------
// Find the quality of a spline approximation
//	 - Click a spline and enter a polyline
// 	 - the maximal distance between the spline and the poly-vertices will be calculated
// Spline FIT POINTS must be clicked
// Internal algorithm:
//	 - go through the spline segments
//	 - determine what polyline nodes should be measured relatively to the segment
//	 - APIAny_GetSplineMaxDistID returns the maximal distance of those nodes
// Suggestions:
// 	- make every nth vertex a fit point
//    if the spline has inflexion points, they should be the primary fit points
//	- make each spline-dir parallel to the vector
//    between the neighbors of the vertex made to be the fit point
//	- try different lenPrev, lenNext values to get the best result
// TIP: the segments can be tuned independently (unlike B-Splines)
// -----------------------------------------------------------------------------

static bool	FindVertsOfSegment (API_TestSplineFit *test, const API_GetPolyType *pline)
{
	API_Coord	begC, endC, c;
	Int32		i, i1, i2, j;

	begC = (*test->splCoords)[test->segment - 1];
	endC = (*test->splCoords)[test->segment];

	// Find poly vertices close enough to the endpoints of the spline segment to be examined
	i1 = i2 = -1;
	for (i = 1; i < pline->nCoords; i++) {
		c = (*pline->coords)[i];
		if (fabs (c.x - begC.x) < EPS && fabs (c.y - begC.y) < EPS)
			i1 = i;
		if (fabs (c.x - endC.x) < EPS && fabs (c.y - endC.y) < EPS)
			i2 = i;
	}
	DBPrintf ("FindVertsOfSegment: (%d, %d)\n", i1, i2);
	if (i1 < 1 || i2 < 1)
		return (false);			// At least one of them didn't match!!! (this segment cannot be processed)

	// Copy the poly vertices BETWEEN i1 and i2 to the test structure (only these vertices should be examined)
	if (i1 < i2) {
		for (i = i1 + 1, j = 0; i < i2; i++, j++)
			(*test->polyCoords)[j] = (*pline->coords)[i];
		test->nCoords = i2 - i1 - 1;
	} else {
		for (i = i2 + 1, j = 0; i < i1; i++, j++)
			(*test->polyCoords)[j] = (*pline->coords)[i];
		test->nCoords = i1 - i2 - 1;
	}

	return (test->nCoords > 0);
}		// FindVertsOfSegment


static void		Do_SplineApprox (void)
{
	API_ElementMemo		memo;
	API_GetPolyType		pline;
	API_TestSplineFit	test;
	double				dist, maxDist;
	API_Guid			guid;
	GSErrCode			err;

	if (!ClickAnElem ("Click a spline to trace", API_SplineID, NULL, NULL, &guid)) {
		WriteReport_Alert ("No spline was clicked");
		return;
	}

	err = ACAPI_Element_GetMemo (guid, &memo);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetMemo (spline)", err);
		return;
	}

	BNZeroMemory (&pline, sizeof (API_GetPolyType));
	strcpy (pline.prompt, "Please enter the tracing polyline");
	pline.method = APIPolyGetMethod_Polyline;
	pline.startCoord.x = (*memo.coords)[0].x;
	pline.startCoord.y = (*memo.coords)[0].y;

	err = ACAPI_Interface (APIIo_GetPolyID, &pline, NULL);
	if (err != NoError) {
		ACAPI_DisposeElemMemoHdls (&memo);
		return;
	}

	// Let's see your approximation qualities...
	BNZeroMemory (&test, sizeof (API_TestSplineFit));

	test.splCoords = memo.coords;
	test.splDirs = memo.bezierDirs;
	test.nFit = BMGetHandleSize ((GSHandle) memo.coords) / sizeof (API_Coord);

	test.polyCoords = (API_Coord **) BMAllocateHandle ((pline.nCoords - 2) * sizeof (API_Coord), ALLOCATE_CLEAR, 0);

	maxDist = -1.0;
	for (test.segment = 1; test.segment < test.nFit; test.segment++) {			// scan spline segments
		if (FindVertsOfSegment (&test, &pline)) {								// determine nodes in interests
			err = ACAPI_Goodies (APIAny_GetSplineMaxDistID, &test, &dist);
			if (err == NoError) {
				WriteReport ("[%d] dist: %lf", test.segment, dist);
				if (dist > maxDist)												// handle the maximal distance
					maxDist = dist;
			} else
				ErrorBeep ("APIAny_GetSplineMaxDistID", err);
		} else
			WriteReport ("[%d] Spline segment skipped", test.segment);
	}

	WriteReport_Alert ("Max. distance: %lf", maxDist);

	BMKillHandle ((GSHandle *) &test.polyCoords);
	BMKillHandle ((GSHandle *) &pline.coords);
	BMKillHandle ((GSHandle *) &pline.parcs);
	ACAPI_DisposeElemMemoHdls (&memo);

	return;
}		// Do_SplineApprox


// -----------------------------------------------------------------------------
// Deletes the specified Project Info entry
// -----------------------------------------------------------------------------
static void	Do_DeleteAutotext (void)
{
	GS::Array<GS::ArrayFB<GS::UniString, 3> >	autoTexts;
	API_AutotextType	type = APIAutoText_Custom;

	GSErrCode err = ACAPI_Goodies (APIAny_GetAutoTextsID, &autoTexts, (void *) (GS::IntPtr) type);

	if (err == NoError && !autoTexts.IsEmpty ()) {
		// delete last custom entry
		const GS::UniString& dbKey = autoTexts[autoTexts.GetSize () - 1][1];
		err = ACAPI_Goodies (APIAny_DeleteAnAutoTextID, (void*)((const char*) dbKey.ToCStr ()), NULL);
		if (err != NoError)
			DBPrintf ("APIAny_DeleteAnAutoTextID function failed.");
	}

	return;
}


// -----------------------------------------------------------------------------
// Creates a custom Project Info entry
// -----------------------------------------------------------------------------
static void	Do_CreateAutotext (void)
{
	const char* uiKey = {"Created Via API"};
	GS::Guid guid;
	guid.Generate ();
	API_Guid dbKey = GSGuid2APIGuid (guid);

	GSErrCode err = ACAPI_Goodies (APIAny_CreateAnAutoTextID, &dbKey, (void*)uiKey);
	if (err != NoError)
		DBPrintf ("APIAny_CreateAnAutoTextID function failed.");

	if (err == NoError) {
		GS::UniString dbKey (PROJ_INFO_AUTOTEXT_CUSTOM_TYPE_STR);
		dbKey.Append (guid.ToUniString ());
		GS::UniString value ("custom text");
		err = ACAPI_Goodies (APIAny_SetAnAutoTextID, &dbKey, &value);
		if (err != NoError)
			DBPrintf ("APIAny_SetAnAutoTextID function failed.");
	}

	return;
}	// Do_CreateAutotext


// -----------------------------------------------------------------------------
// Get the autotext values from ArchiCAD
// -----------------------------------------------------------------------------

static void	Do_GetAutotexts (const GS::UniString* thisKeyOnly = NULL)
{
	GS::Array<GS::ArrayFB<GS::UniString, 3> >	autotexts;

	API_AutotextType	type = APIAutoText_All;

	GSErrCode	err = ACAPI_Goodies (APIAny_GetAutoTextsID, &autotexts, (void *) (GS::IntPtr) type);
	if (err == NoError) {
		WriteReport ("+--------------------------------+--------------------------------+--------------------------------+");
		WriteReport ("+          User Interface        +          Database Key          +              Value             +");
		WriteReport ("+--------------------------------+--------------------------------+--------------------------------+");

		for (ULong ii = 0; ii < autotexts.GetSize (); ii++) {
			if (thisKeyOnly == NULL || *thisKeyOnly == autotexts[ii][1]) {
				char	s0[32], s1[32], s2[32];
				autotexts[ii][0].DeleteAll (GS::UniChar ('%'));
				autotexts[ii][1].DeleteAll (GS::UniChar ('%'));
				autotexts[ii][2].DeleteAll (GS::UniChar ('%'));
				CHTruncate (autotexts[ii][0].ToCStr (), s0, sizeof (s0));
				CHTruncate (autotexts[ii][1].ToCStr (), s1, sizeof (s1));
				CHTruncate (autotexts[ii][2].ToCStr (), s2, sizeof (s2));

				WriteReport ("+%32s+%32s+%32s+", s0, s1, s2);
			}
		}

		WriteReport ("+--------------------------------+--------------------------------+--------------------------------+");
	} else {
		WriteReport_Alert ("Error %s in APIAny_GetAutoTextsID", ErrID_To_Name (err));
	}
}


// -----------------------------------------------------------------------------
// Set the value of an autotext in ArchiCAD
// -----------------------------------------------------------------------------

static void	Do_SetAutotexts (void)
{
	GS::UniString	dbKey ("CAD_TECHNICIAN_FULLNAME"), value ("John Doe");

	// Round 0 -- dump autotext for this key
	Do_GetAutotexts (&dbKey);

	// Round 1 -- clear autotext value
	GSErrCode	err = ACAPI_Goodies (APIAny_SetAnAutoTextID, &dbKey, NULL);
	if (err == NoError) {
		Do_GetAutotexts (&dbKey);
	} else {
		WriteReport_Alert ("Error %s in APIAny_SetAnAutoTextID", ErrID_To_Name (err));
	}

	// Round 2 -- set it to something meaningful
	err = ACAPI_Goodies (APIAny_SetAnAutoTextID, &dbKey, &value);
	if (err == NoError) {
		Do_GetAutotexts (&dbKey);
	} else {
		WriteReport_Alert ("Error %s in APIAny_SetAnAutoTextID", ErrID_To_Name (err));
	}

	// Round 3 -- test for  error code
	err = ACAPI_Goodies (APIAny_SetAnAutoTextID, NULL, NULL);
	if (err == NoError) {
		Do_GetAutotexts (&dbKey);
	} else {
		if (err == APIERR_BADPARS)
			WriteReport ("APIAny_SetAnAutoTextID returned correct error code");
		else
			WriteReport_Alert ("Error %s in APIAny_SetAnAutoTextID", ErrID_To_Name (err));
	}
}


// -----------------------------------------------------------------------------
// Get the library part own unique ID from the element fast
// -----------------------------------------------------------------------------

static void	Do_GetLibPartUnID (void)
{
	API_ElemTypeID		typeID;
	API_Guid			guid;
	API_Element			element;
	API_LibPart			libPart;
	API_ToolBoxItem		tbItem;
	char				lpfUnID[128] = {0};
	GSErrCode			error;

	if (!ClickAnElem ("Click any library part based element", API_ZombieElemID, NULL, &typeID, &guid)) {
		WriteReport_Alert ("No element was clicked");
		return;
	}
	if (typeID != API_ObjectID && typeID != API_LampID && typeID != API_WindowID && typeID != API_DoorID && typeID != API_SkylightID) {
		WriteReport_Alert ("Bad element type");
		return;
	}

	BNZeroMemory (&element, Sizeof32 (element));
	element.header.typeID = typeID;
	element.header.guid   = guid;

	// old method
	if (ACAPI_Element_Get (&element) != NoError) {
		WriteReport_Alert ("Can't get element");
		return;
	}

	BNZeroMemory (&libPart, sizeof (libPart));
	switch (typeID) {
		case API_ObjectID:
			libPart.typeID = APILib_ObjectID;
			libPart.index = element.object.libInd;
			break;

		case API_LampID:
			libPart.typeID = APILib_LampID;
			libPart.index = element.object.libInd;
			break;

		case API_WindowID:
			libPart.typeID = APILib_WindowID;
			libPart.index = element.window.openingBase.libInd;
			break;

		case API_DoorID:
			libPart.typeID = APILib_DoorID;
			libPart.index = element.window.openingBase.libInd;
			break;

		case API_SkylightID:
			libPart.typeID = APILib_SkylightID;
			libPart.index = element.skylight.openingBase.libInd;
			break;

		default:
			break;
	}

	error = ACAPI_LibPart_Get (&libPart);
	if (error != NoError) {
		if (error == APIERR_MISSINGDEF)
			WriteReport_Alert ("Error code: APIERR_MISSINGDEF \nDocumentName: %s \nTypeID: %d \nOwnerID: %d \nIndex: %d \nMissingDef: %d", (const char *)GS::UniString(libPart.docu_UName).ToCStr(), libPart.typeID, libPart.ownerID, libPart.index, libPart.missingDef);
		else
			WriteReport_Alert ("Can't get library part");
		return;
	}
	delete libPart.location;

	WriteReport ("Do_GetLibPartUnID():: library part's own ID (old way): %s", libPart.ownUnID);

	// new method
	BNZeroMemory (&element, Sizeof32 (element));
	element.header.typeID = typeID;
	element.header.guid   = guid;

	if (ACAPI_Element_GetHeader (&element.header) != NoError) {
		WriteReport_Alert ("Can't get element header");
		return;
	}

	if (ACAPI_Goodies (APIAny_GetElemLibPartUnIdID, &element.header, lpfUnID) == NoError) {
		WriteReport ("Do_GetLibPartUnID():: library part's own ID (new way): %s", lpfUnID);
	}

	if (ACAPI_Goodies (APIAny_GetLibPartToolVariationID, &libPart, &tbItem) == NoError) {
		char *p = (char *) &tbItem.variationID;
		if (tbItem.typeID != API_ZombieElemID) {
			if (p[0] != 0)
				WriteReport ("Do_GetLibPartUnID():: creator tool & variation: %s \"%c%c%c%c\"", ElemID_To_Name (tbItem.typeID), p[3], p[2], p[1], p[0]);
			else
				WriteReport ("Do_GetLibPartUnID():: creator tool: %s", ElemID_To_Name (tbItem.typeID));
		}
	}
}


#ifdef __APPLE__
#pragma mark -
#endif

// -----------------------------------------------------------------------------
// Handles menu commands
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL MenuCommandHandler (const API_MenuParams *menuParams)
{
	switch (menuParams->menuItemRef.menuResID) {
		case 32500:
			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		Do_BMPtoGIF ();				break;
				case 2:		Do_SelectTopmostSlab ();	break;
				case 3:		Do_TriangulateSlab ();		break;
				case 4:		Do_SplineApprox ();			break;
				case 5:		Do_GetAutotexts ();			break;
				case 6:		Do_SetAutotexts ();			break;
				case 7:		Do_CreateAutotext ();		break;
				case 8:		Do_DeleteAutotext ();		break;
				case 9:		Do_GetLibPartUnID ();		break;
			}
			break;
	}

	return NoError;
}		// MenuCommandHandler


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

API_AddonType	__ACENV_CALL	CheckEnvironment (API_EnvirParams* envir)
{
	if (envir->serverInfo.serverApplication != APIAppl_ArchiCADID)
		return APIAddon_DontRegister;

	ACAPI_Resource_GetLocStr (envir->addOnInfo.name, 32000, 1);
	ACAPI_Resource_GetLocStr (envir->addOnInfo.description, 32000, 2);

	return APIAddon_Normal;
}		// CheckEnvironment


// -----------------------------------------------------------------------------
// Interface definitions
// -----------------------------------------------------------------------------

GSErrCode	__ACENV_CALL	RegisterInterface (void)
{
	GSErrCode err;

	err = ACAPI_Register_Menu (32500, 32520, MenuCode_UserDef, MenuFlag_Default);
	if (err != NoError)
		DBPrintf ("Goodie_Functions:: RegisterInterface() ACAPI_Register_Menu failed\n");

	return err;
}		// RegisterInterface


// -----------------------------------------------------------------------------
// Called when the Add-On has been loaded into memory
// to perform an operation
// -----------------------------------------------------------------------------

GSErrCode	__ACENV_CALL Initialize	(void)
{
	GSErrCode err = ACAPI_Install_MenuHandler (32500, MenuCommandHandler);
	if (err != NoError)
		DBPrintf ("Goodie_Functions:: Initialize() ACAPI_Install_MenuHandler failed\n");

	return err;
}		// Initialize


// -----------------------------------------------------------------------------
// FreeData
//		called when the Add-On is going to be unloaded
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	FreeData (void)

{
	return NoError;
}		// FreeData

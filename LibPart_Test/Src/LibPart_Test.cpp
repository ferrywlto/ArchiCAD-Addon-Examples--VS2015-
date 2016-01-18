// *****************************************************************************
// Source code for the LibPart Test Add-On
// API Development Kit 19; Mac/Win
//
// Namespaces:		 Contact person:
//		 -None-
//
// [SG compatible] - Yes
// *****************************************************************************

#include "APIEnvir.h"

// =============================================================================
//
// System Includes
//
// =============================================================================

#include	<stdio.h>

// =============================================================================
//
// API Includes
//
// =============================================================================

#include	"Location.hpp"
#include	"Folder.hpp"
#include	"File.hpp"
#include	"FileSystem.hpp"
#include	"ACAPinc.h"		// also includes APIdefs.h
#include	"APICommon.h"	// also includes GSRoot.hpp


// -----------------------------------------------------------------------------
//  Retrieves a valid location for creating library part
// -----------------------------------------------------------------------------

static GSErrCode	GetLocation (IO::Location*& loc, bool useEmbeddedLibrary)
{
	GS::Array<API_LibraryInfo>	libInfo;
	loc = NULL;

	GSErrCode err = NoError;

	if (useEmbeddedLibrary) {
		Int32 embeddedLibraryIndex = -1;
		// get embedded library location
		if (ACAPI_Environment (APIEnv_GetLibrariesID, &libInfo, &embeddedLibraryIndex) == NoError && embeddedLibraryIndex >= 0) {
			try {
				loc = new IO::Location (libInfo[embeddedLibraryIndex].location);
			} catch (std::bad_alloc&) {
				return APIERR_MEMFULL;
			}

			if (loc != NULL) {
				IO::Location ownFolderLoc (*loc);
				ownFolderLoc.AppendToLocal (IO::Name ("LibPart_Test Library"));
				err = IO::fileSystem.CreateFolder (ownFolderLoc);
				if (err == NoError || err == IO::FileSystem::TargetExists)
					loc->AppendToLocal (IO::Name ("LibPart_Test Library"));
			}
		}
	} else {
		// register our own folder and create the library part in it
		if (ACAPI_Environment (APIEnv_GetLibrariesID, &libInfo) == NoError) {
			IO::Location folderLoc;
			API_SpecFolderID specID = API_UserDocumentsFolderID;
			ACAPI_Environment (APIEnv_GetSpecFolderID, &specID, &folderLoc);
			folderLoc.AppendToLocal (IO::Name ("LibPart_Test Library"));
			IO::Folder destFolder (folderLoc, IO::Folder::Create);
			if (destFolder.GetStatus () != NoError || !destFolder.IsWriteable ())
				return APIERR_GENERAL;

			loc = new IO::Location (folderLoc);

			for (UInt32 ii = 0; ii < libInfo.GetSize (); ii++) {
				if (folderLoc == libInfo[ii].location)
					return NoError;
			}

			try {
				API_LibraryInfo		li;
				li.location = folderLoc;

				libInfo.Push (li);
			}
			catch (const GS::OutOfMemoryException&) {
				DBBREAK_STR ("Not enough memory");
				return APIERR_MEMFULL;
			}

			ACAPI_Environment (APIEnv_SetLibrariesID, &libInfo);
		}
	}

	return NoError;
}

// =============================================================================
//
// Library Part functions
//
// =============================================================================

// -----------------------------------------------------------------------------
//  Set LibPart preview picture
//   Allowed to call between the ACAPI_LibPart_Create and ACAPI_LibPart_Save functions only
// -----------------------------------------------------------------------------

void SetPreviewPictureToLibPart (const IO::Location& picFileLoc)
{
	char**	bufHdl = NULL;

	try {
		USize	size;
		char	mimeStr[16];
		char	hdlState;

		IO::File picFile (picFileLoc);
		if (picFile.GetStatus () != NoError)
			throw 1; // can't read given picture

		// read picture data from file to buf:
		if (picFile.GetDataLength (&size) != NoError)
			throw 1;

		sprintf (mimeStr, "image/gif"); // prefer gif, but you can change gif to jpeg or png
		bufHdl = (char**) BMAllocateHandle (size + Strlen32 (mimeStr) + 1, 0, 0);
		if (bufHdl == NULL)
			throw 1;

		hdlState = BMModifyHandleState (bufHdl, HANDLE_STATE_LOCK, 0);
		// insert mime type string first, and then the picture
		CHCopyC (mimeStr, *bufHdl);
		if (picFile.Open (IO::File::ReadMode) != NoError)
			throw 1;
		if (picFile.ReadBin ((*bufHdl) + (strlen (mimeStr) + 1), size) != NoError)
			throw 1;
		BMModifyHandleState (bufHdl, hdlState, 0);
		picFile.Close ();

		// add preview picture to libpart:
		API_LibPartSection section;
		BNZeroMemory (&section, sizeof (API_LibPartSection));
		section.sectType = API_SectInfoGIF;
		if (ACAPI_LibPart_AddSection (&section, bufHdl, NULL))
			throw 2;

		BMKillHandle ((GSHandle *) &bufHdl);
	} catch (...) {
		if (bufHdl != NULL)
			BMKillHandle ((GSHandle *) &bufHdl);
		ACAPI_WriteReport ("Error while attaching preview picture!", true);
	}
}

// -----------------------------------------------------------------------------
//  Sets the given LibPart as the default object
// -----------------------------------------------------------------------------

GSErrCode	SetLibPartAsDefaultObject (const API_LibPart& libPart)
{
	GSErrCode			err = NoError;
	API_Element			element;
	API_Element			mask;
	API_ElementMemo		memo;

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

	element.header.typeID = API_ObjectID;

	API_ParamOwnerType paramOwner;
	BNZeroMemory (&paramOwner, sizeof (API_ParamOwnerType));
	paramOwner.libInd = libPart.index;
	err = ACAPI_Goodies (APIAny_OpenParametersID, &paramOwner, NULL);
	if (err == NoError) {
		API_GetParamsType getParams;
		BNZeroMemory (&getParams, sizeof (API_GetParamsType));
		err = ACAPI_Goodies (APIAny_GetActParametersID, &getParams, NULL);
		if (err == NoError) {
			ACAPI_DisposeAddParHdl (&memo.params);
			memo.params = getParams.params;
		}
		ACAPI_Goodies (APIAny_CloseParametersID, NULL, NULL);
	}

	element.object.libInd = libPart.index;
	ACAPI_ELEMENT_MASK_CLEAR (mask);
	ACAPI_ELEMENT_MASK_SET (mask, API_ObjectType, libInd);

	err = ACAPI_Element_ChangeDefaults (&element, &memo, &mask);
	if (err != NoError)
		WriteReport ("ACAPI_Element_ChangeDefaults failed");

	ACAPI_DisposeElemMemoHdls (&memo);

	return err;
}		// SetLibPartAsDefaultObject

// -----------------------------------------------------------------------------
//  Creates a simple Library Part
// -----------------------------------------------------------------------------

static void		Do_CreateLibraryPart (void)
{
	GSErrCode err = NoError;

	API_LibPart libPart;
	BNZeroMemory (&libPart, sizeof (API_LibPart));
	libPart.typeID = APILib_ObjectID;
	libPart.isTemplate = false;
	libPart.isPlaceable = true;
	CHCopyC ("{103E8D2C-8230-42E1-9597-46F84CCE28C0}-{00000000-0000-0000-0000-000000000000}", libPart.parentUnID);	// Model Element subtype

	GSTimeRecord timeRecord;
	TIGetTimeRecord (0, &timeRecord, TI_CURRENT_TIME);
	UInt32 fraction = TIGetTicks () % TIGetTicksPerSec ();
	GS::snuprintf (libPart.docu_UName, sizeof (libPart.docu_UName) / sizeof (GS::uchar_t), L("LPTest_%d-%02d-%02d_%02d%02d%02d_%d"),
					timeRecord.year, timeRecord.month, timeRecord.day, timeRecord.hour, timeRecord.minute, timeRecord.second, fraction);

	err = GetLocation (libPart.location, true);
	if (err != NoError) {
		WriteReport ("Library Part creation failed");
		return;
	}

	ACAPI_Environment (APIEnv_OverwriteLibPartID, (void *) (Int32) true, NULL);
	err = ACAPI_LibPart_Create (&libPart);
	ACAPI_Environment (APIEnv_OverwriteLibPartID, (void *) (Int32) false, NULL);

	if (err == NoError) {
		char buffer[1000];

		API_LibPartSection section;

		// Comment script section
		BNZeroMemory (&section, sizeof (API_LibPartSection));
		section.sectType = API_SectComText;
		ACAPI_LibPart_NewSection (&section);
		sprintf (buffer, "Library Part written by LibPart_Test add-on");
		ACAPI_LibPart_WriteSection (Strlen32 (buffer), buffer);
		ACAPI_LibPart_EndSection ();

		// Master script section
		BNZeroMemory (&section, sizeof (API_LibPartSection));
		section.sectType = API_Sect1DScript;
		ACAPI_LibPart_NewSection (&section);
		buffer[0] = '\0';
		ACAPI_LibPart_WriteSection (Strlen32 (buffer), buffer);
		ACAPI_LibPart_EndSection ();

		// 3D script section
		BNZeroMemory (&section, sizeof (API_LibPartSection));
		section.sectType = API_Sect3DScript;
		ACAPI_LibPart_NewSection (&section);
		sprintf (buffer, "MATERIAL mat%s%s", GS::EOL, GS::EOL);
		ACAPI_LibPart_WriteSection (Strlen32 (buffer), buffer);
		sprintf (buffer, "BLOCK a, b, 1%s", GS::EOL);
		ACAPI_LibPart_WriteSection (Strlen32 (buffer), buffer);
		sprintf (buffer, "ADD a * 0.5, b* 0.5, 1%s", GS::EOL);
		ACAPI_LibPart_WriteSection (Strlen32 (buffer), buffer);
		sprintf (buffer, "CYLIND zzyzx - 3, MIN (a, b) * 0.5%s", GS::EOL);
		ACAPI_LibPart_WriteSection (Strlen32 (buffer), buffer);
		sprintf (buffer, "ADDZ zzyzx - 3%s", GS::EOL);
		ACAPI_LibPart_WriteSection (Strlen32 (buffer), buffer);
		sprintf (buffer, "CONE 2, MIN (a, b) * 0.5, 0.0, 90, 90%s", GS::EOL);
		ACAPI_LibPart_WriteSection (Strlen32 (buffer), buffer);
		ACAPI_LibPart_EndSection ();

		// 2D script section
		BNZeroMemory (&section, sizeof (API_LibPartSection));
		section.sectType = API_Sect2DScript;
		ACAPI_LibPart_NewSection (&section);
		sprintf (buffer, "PROJECT2 3, 270, 2%s", GS::EOL);
		ACAPI_LibPart_WriteSection (Strlen32 (buffer), buffer);
		ACAPI_LibPart_EndSection ();

		// Parameter script section
		BNZeroMemory (&section, sizeof (API_LibPartSection));
		section.sectType = API_SectVLScript;
		ACAPI_LibPart_NewSection (&section);
		sprintf (buffer, "VALUES \"zzyzx\" RANGE [6,]%s", GS::EOL);
		ACAPI_LibPart_WriteSection (Strlen32 (buffer), buffer);
		ACAPI_LibPart_EndSection ();

		// Parameters section
		BNZeroMemory (&section, sizeof (API_LibPartSection));
		section.sectType = API_SectParamDef;

		short nPars = 4;
		API_AddParType** addPars = reinterpret_cast<API_AddParType**>(BMAllocateHandle (nPars * sizeof (API_AddParType), ALLOCATE_CLEAR, 0));
		if (addPars != NULL) {
			API_AddParType* pAddPar = &(*addPars)[0];
			pAddPar->typeID = APIParT_Mater;
			pAddPar->typeMod = 0;
			CHTruncate ("mat", pAddPar->name, sizeof (pAddPar->name));
			GS::ucscpy (pAddPar->uDescname, L("Material"));
			pAddPar->value.real = 1;

			pAddPar = &(*addPars)[1];
			pAddPar->typeID = APIParT_Length;
			pAddPar->typeMod = 0;
			CHTruncate ("len", pAddPar->name, sizeof (pAddPar->name));
			GS::ucscpy (pAddPar->uDescname, L("Length"));
			pAddPar->value.real = 2.5;

			pAddPar = &(*addPars)[2];
			pAddPar->typeID = APIParT_CString;
			pAddPar->typeMod = 0;
			CHTruncate ("myStr", pAddPar->name, sizeof (pAddPar->name));
			GS::ucscpy (pAddPar->uDescname, L("String parameter"));
			GS::ucscpy (pAddPar->value.uStr, L("this is a string"));

			pAddPar = &(*addPars)[3];
			pAddPar->typeID = APIParT_RealNum;
			pAddPar->typeMod = API_ParArray;
			pAddPar->dim1 = 3;
			pAddPar->dim2 = 4;
			CHTruncate ("matrix", pAddPar->name, sizeof (pAddPar->name));
			GS::ucscpy (pAddPar->uDescname, L("Array parameter with real numbers"));
			pAddPar->value.array = BMAllocateHandle (pAddPar->dim1 * pAddPar->dim2 * sizeof (double), ALLOCATE_CLEAR, 0);
			double** arrHdl = reinterpret_cast<double**>(pAddPar->value.array);
			for (Int32 k = 0; k < pAddPar->dim1; k++)
				for (Int32 j = 0; j < pAddPar->dim2; j++)
					(*arrHdl)[k * pAddPar->dim2 + j] = (k == j ? 1.1 : 0.0);

			double aa = 1.0;
			double bb = 1.0;
			GSHandle sectionHdl = NULL;
			ACAPI_LibPart_GetSect_ParamDef (&libPart, addPars, &aa, &bb, NULL, &sectionHdl);

			API_LibPartDetails details;
			BNZeroMemory (&details, sizeof (API_LibPartDetails));
			details.object.autoHotspot = false;
			ACAPI_LibPart_SetDetails_ParamDef (&libPart, sectionHdl, &details);

			ACAPI_LibPart_AddSection (&section, sectionHdl, NULL);

			BMKillHandle (reinterpret_cast<GSHandle*>(&arrHdl));
			BMKillHandle (reinterpret_cast<GSHandle*>(&addPars));
			BMKillHandle (&sectionHdl);
		} else {
			err = APIERR_MEMFULL;
		}

		IO::Location    	folderLoc;
		API_SpecFolderID	specID = API_UserDocumentsFolderID;
		ACAPI_Environment (APIEnv_GetSpecFolderID, &specID, &folderLoc);
		SetPreviewPictureToLibPart (IO::Location (folderLoc, (IO::Name) "mygif.gif"));

		// Save the constructed library part
		if (err == NoError)
			err = ACAPI_LibPart_Save (&libPart);

		if (libPart.location != NULL) {
			delete libPart.location;
			libPart.location = NULL;
		}
	}

	if (err == NoError) {
		WriteReport ("Library Part \"%s\" created", static_cast<const char*> (GS::UniString (libPart.docu_UName).ToCStr ()));

		err = SetLibPartAsDefaultObject (libPart);
		if (err == NoError)
			WriteReport ("Default object changed successfully");
	} else
		WriteReport ("Library Part creation failed");

	return;
}		// Do_CreateLibraryPart


// -----------------------------------------------------------------------------
//  Get polygon from the currently selected element
// -----------------------------------------------------------------------------

static GSErrCode	GetSelectedElementPolygon (API_Polygon* poly, API_ElementMemo* memo)
{
	GSErrCode			err = NoError;
	API_SelectionInfo 	selectionInfo;
	API_Neig			**selNeigs = NULL;
	API_ElemTypeID		typeID;

	BNZeroMemory (&selectionInfo, sizeof (API_SelectionInfo));
	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, false);
	if (err != NoError) {
		BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
		if (err != APIERR_NOSEL)
			WriteReport ("ACAPI_Selection_Get failed");
		return err;
	}

	err = APIERR_NOSEL;
	for (Int32 i = 0; i < selectionInfo.sel_nElem; i++) {
		typeID = Neig_To_ElemID ((*selNeigs)[i].neigID);
		if (typeID == API_HatchID) {
			API_Element	element;
			BNZeroMemory (&element, sizeof (API_Element));
			element.header.guid  = (*selNeigs)[i].guid;
			err = ACAPI_Element_Get (&element);
			if (err == NoError && memo != NULL)
				err = ACAPI_Element_GetMemo (element.header.guid, memo, APIMemoMask_Polygon);
			if (poly != NULL)
				*poly = element.hatch.poly;
			if (err == NoError)
				break;
			else if (memo != NULL)
				ACAPI_DisposeElemMemoHdls (memo);
		}
	}

	BMKillHandle ((GSHandle *) &selNeigs);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);

	return err;
}		// GetSelectedElementPolygon


// -----------------------------------------------------------------------------
//  Creates a Window Library Part using the given coordinates
// -----------------------------------------------------------------------------

static GSErrCode	MakeWindowLibpart (API_LibPart& libPart, const API_Coord* coords, const API_Polygon& poly)
{
	GSErrCode err = NoError;

	API_Box polyBoundBox = { 1E9, 1E9, -1E9, -1E9 };

	if (coords == NULL || poly.nCoords < 3) {
		WriteReport ("Insufficient polygon parameters");
		return APIERR_BADPARS;
	} else {
		for (Int32 i = 1; i <= poly.nCoords; i++) {
			if (polyBoundBox.xMin > coords[i].x)
				polyBoundBox.xMin = coords[i].x;
			if (polyBoundBox.yMin > coords[i].y)
				polyBoundBox.yMin = coords[i].y;
			if (polyBoundBox.xMax < coords[i].x)
				polyBoundBox.xMax = coords[i].x;
			if (polyBoundBox.yMax < coords[i].y)
				polyBoundBox.yMax = coords[i].y;
		}
	}

	double aa = polyBoundBox.xMax - polyBoundBox.xMin;
	double bb = polyBoundBox.yMax - polyBoundBox.yMin;
	if (aa < EPS || bb < EPS)
		return APIERR_BADPOLY;

	BNZeroMemory (&libPart, sizeof (API_LibPart));
	libPart.typeID = APILib_WindowID;
	libPart.isTemplate = false;
	libPart.isPlaceable = true;

	CHCopyC ("{4ABD0A6E-634B-4931-B3AA-9BEE01F35CDF}-{00000000-0000-0000-0000-000000000000}", libPart.parentUnID);	// Window (Wall) subtype

	if (err == NoError) {
		UInt32 counter = 0;
		do {
			GS::snuprintf (libPart.docu_UName, sizeof (libPart.docu_UName) / sizeof (GS::uchar_t), L("LPTest_Window_%04d"), ++counter);
			err = ACAPI_LibPart_Search (&libPart, false);
			if (libPart.location != NULL) {
				delete libPart.location;
				libPart.location = NULL;
			}
		} while (err == NoError && counter < 1000);

		if (err != NoError) {
			err = GetLocation (libPart.location, false);
			if (err != NoError)
				return err;
			err = ACAPI_LibPart_Create (&libPart);
		} else
			err = APIERR_MEMFULL;
	}

	if (err == NoError) {
		char buffer[1000];

		API_LibPartSection section;

		// Comment script section
		BNZeroMemory (&section, sizeof (API_LibPartSection));
		section.sectType = API_SectComText;
		ACAPI_LibPart_NewSection (&section);
		sprintf (buffer, "Library Part written by LibPart_Test add-on");
		ACAPI_LibPart_WriteSection (Strlen32 (buffer), buffer);
		ACAPI_LibPart_EndSection ();

		// Master script section
		BNZeroMemory (&section, sizeof (API_LibPartSection));
		section.sectType = API_Sect1DScript;
		ACAPI_LibPart_NewSection (&section);
		buffer[0] = '\0';
		ACAPI_LibPart_WriteSection (Strlen32 (buffer), buffer);
		ACAPI_LibPart_EndSection ();

		// 3D script section
		BNZeroMemory (&section, sizeof (API_LibPartSection));
		section.sectType = API_Sect3DScript;
		ACAPI_LibPart_NewSection (&section);

		for (Int32 i = 1; i <= poly.nCoords; i++) {
			double xCoord = (coords[i].x - polyBoundBox.xMin - 0.5 * aa) / aa;
			double yCoord = (coords[i].y - polyBoundBox.yMin) / bb;
			sprintf (buffer, "PUT %.4f * a, %.4f * b, 15%s", xCoord, yCoord, GS::EOL);
			ACAPI_LibPart_WriteSection (Strlen32 (buffer), buffer);
		}
		sprintf (buffer, "%sMATERIAL gs_frame_mat%s", GS::EOL, GS::EOL);
		ACAPI_LibPart_WriteSection (Strlen32 (buffer), buffer);
		sprintf (buffer, "WALLHOLE NSP/3, 1, USE (NSP)%s%s", GS::EOL, GS::EOL);
		ACAPI_LibPart_WriteSection (Strlen32 (buffer), buffer);
		sprintf (buffer, "ADDZ WALL_THICKNESS/2%s", GS::EOL);
		ACAPI_LibPart_WriteSection (Strlen32 (buffer), buffer);
		sprintf (buffer, "MATERIAL gs_glass_mat%s", GS::EOL);
		ACAPI_LibPart_WriteSection (Strlen32 (buffer), buffer);
		sprintf (buffer, "PRISM_ NSP/3, gs_glass_thk, USE (NSP)%s", GS::EOL);
		ACAPI_LibPart_WriteSection (Strlen32 (buffer), buffer);
		ACAPI_LibPart_EndSection ();

		// 2D script section
		BNZeroMemory (&section, sizeof (API_LibPartSection));
		section.sectType = API_Sect2DScript;
		ACAPI_LibPart_NewSection (&section);
		sprintf (buffer, "LINE2 -a/2, 0, -a/2, -WALL_THICKNESS%s", GS::EOL);
		ACAPI_LibPart_WriteSection (Strlen32 (buffer), buffer);
		sprintf (buffer, "LINE2  a/2, 0,  a/2, -WALL_THICKNESS%s", GS::EOL);
		ACAPI_LibPart_WriteSection (Strlen32 (buffer), buffer);
		sprintf (buffer, "LINE2 -a/2, -WALL_THICKNESS/2, a/2, -WALL_THICKNESS/2%s", GS::EOL);
		ACAPI_LibPart_WriteSection (Strlen32 (buffer), buffer);
		ACAPI_LibPart_EndSection ();

		// Parameters section
		BNZeroMemory (&section, sizeof (API_LibPartSection));
		section.sectType = API_SectParamDef;

		GSHandle sectionHdl = NULL;
		ACAPI_LibPart_GetSect_ParamDef (&libPart, NULL, &aa, &bb, NULL, &sectionHdl);

		API_LibPartDetails details;
		BNZeroMemory (&details, sizeof (API_LibPartDetails));
		ACAPI_LibPart_SetDetails_ParamDef (&libPart, sectionHdl, &details);

		ACAPI_LibPart_AddSection (&section, sectionHdl, NULL);
		BMKillHandle (&sectionHdl);

		// Save the constructed library part
		if (err == NoError)
			err = ACAPI_LibPart_Save (&libPart);

		if (libPart.location != NULL) {
			delete libPart.location;
			libPart.location = NULL;
		}
	}

	return err;
}		// MakeWindowLibpart


// -----------------------------------------------------------------------------
//  Puts the specified Window into the clicked Wall
// -----------------------------------------------------------------------------

static GSErrCode	PutWindowIntoWall (const API_LibPart& libPart, const API_Guid& wallGuid, const API_Coord& coord)
{
	GSErrCode			err = NoError;
	API_Element			element;
	API_ElementMemo		memo;

	BNZeroMemory (&element, sizeof (API_Element));
	element.header.guid = wallGuid;
	err = ACAPI_Element_Get (&element);
	if (err != NoError) {
		WriteReport ("ACAPI_Element_Get failed");
		return err;
	}

	API_Coord wallBegC = element.wall.begC;

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

	element.header.typeID = API_WindowID;

	err = ACAPI_Element_GetDefaults (&element, &memo);
	if (err != NoError) {
		ACAPI_DisposeElemMemoHdls (&memo);
		WriteReport ("ACAPI_Element_GetDefaults failed");
		return err;
	}

	API_ParamOwnerType paramOwner;
	BNZeroMemory (&paramOwner, sizeof (API_ParamOwnerType));
	paramOwner.libInd = libPart.index;
	err = ACAPI_Goodies (APIAny_OpenParametersID, &paramOwner, NULL);
	if (err == NoError) {
		API_GetParamsType getParams;
		BNZeroMemory (&getParams, sizeof (API_GetParamsType));
		err = ACAPI_Goodies (APIAny_GetActParametersID, &getParams, NULL);
		if (err == NoError) {
			ACAPI_DisposeAddParHdl (&memo.params);
			memo.params = getParams.params;
		}
		ACAPI_Goodies (APIAny_CloseParametersID, NULL, NULL);
	}

	element.window.objLoc = DistCPtr (&coord, &wallBegC);		// curved walls should be handled differently
	element.window.owner  = wallGuid;
	element.window.openingBase.libInd = libPart.index;

	err = ACAPI_CallUndoableCommand ("Place Window",
		[&] () -> GSErrCode {
			GSErrCode err1 = ACAPI_Element_Create (&element, &memo);
			if (err1 != NoError)
				WriteReport ("ACAPI_Element_Create failed");
			return err1;
		});

	ACAPI_DisposeElemMemoHdls (&memo);

	return err;
}		// PutWindowIntoWall


// -----------------------------------------------------------------------------
//  Creates a Window from a Hatch polygon and puts it into the clicked wall
// -----------------------------------------------------------------------------

static void		Do_CreateWindowFromHatch (void)
{
	GSErrCode err = NoError;

	// Calculate the Window shape based on the selected polygonal element
	API_Polygon		poly;
	API_ElementMemo	memo;
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

	err = GetSelectedElementPolygon (&poly, &memo);
	if (err != NoError) {
		WriteReport_Alert ("Could not get polygonal shape from the selected element");
		ACAPI_DisposeElemMemoHdls (&memo);
		return;
	}

	// Get the position to place the Window element
	API_ElemTypeID	typeID = API_ZombieElemID;
	API_Guid		wallGuid = APINULLGuid;
	API_Coord3D		coord3D;

	if (!ClickAnElem ("Click on a Wall to place a Window", API_WallID, NULL, &typeID, &wallGuid, &coord3D)|| typeID != API_WallID || wallGuid == APINULLGuid) {
		WriteReport ("Could not find Wall element at the clicked point");
		ACAPI_DisposeElemMemoHdls (&memo);
		return;
	}

	// Create the Libpary Part
	API_LibPart libPart;

	if (err == NoError)
		err = MakeWindowLibpart (libPart, *memo.coords, poly);
	else
		WriteReport ("Could not create Window Library Part");

	ACAPI_DisposeElemMemoHdls (&memo);

	// Put the created Window into the Wall
	if (err == NoError) {
		API_Coord coord;
		coord.x = coord3D.x;
		coord.y = coord3D.y;

		err = PutWindowIntoWall (libPart, wallGuid, coord);
	} else
		WriteReport ("Could not put Window into Wall");

	if (err == NoError)
		WriteReport ("Window \"%s\" was successfully created and put into Wall", static_cast<const char*> (GS::UniString (libPart.docu_UName).ToCStr ()));

	return;
}		// Do_CreateWindowFromHatch


#ifdef __APPLE__
#pragma mark -
#endif

// -----------------------------------------------------------------------------
// MenuCommandHandler
//		called to perform the user-asked command
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL MenuCommandHandler (const API_MenuParams *menuParams)
{
	switch (menuParams->menuItemRef.menuResID) {
		case 32500:		/* Library Parts */
				switch (menuParams->menuItemRef.itemIndex) {
					case 1:		Do_CreateLibraryPart ();			break;
					case 2:		Do_CreateWindowFromHatch ();		break;
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

// -----------------------------------------------------------------------------
// Dependency definitions
// -----------------------------------------------------------------------------

API_AddonType	__ACDLL_CALL	CheckEnvironment (API_EnvirParams* envir)
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

GSErrCode	__ACDLL_CALL	RegisterInterface (void)
{
	GSErrCode err = ACAPI_Register_Menu (32500, 32600, MenuCode_UserDef, MenuFlag_SeparatorBefore);
	return err;
}		// RegisterInterface


// -----------------------------------------------------------------------------
// Initialize
//		called after the Add-On has been loaded into memory
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	Initialize (void)
{
	GSErrCode err = ACAPI_Install_MenuHandler (32500, MenuCommandHandler);
	if (err != NoError)
		DBPrintf ("LibPart Test:: Initialize() ACAPI_Install_MenuHandler failed\n");

	return NoError;
}		// Initialize


// -----------------------------------------------------------------------------
// FreeData
//		called when the Add-On is going to be unloaded
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	FreeData (void)
{
	return NoError;
}		// FreeData

// *****************************************************************************
// Description:		Source code for the Database Control Add-On
//
// Module:			API Development Kit 12; Mac/Win
// Namespace:		-none-
// Contact person:	devsup@graphisoft.hu
//
// SG compatible
// *****************************************************************************

#include "APIEnvir.h"
#define	_DATABASE_CONTROL_TRANSL_


// --- Includes ----------------------------------------------------------------

#include	<string.h>

#include	"ACAPinc.h"
#include	"APICommon.h"

#include	"Array.hpp"
#include	"CH.hpp"
#include	"IV.hpp"
#include	"Location.hpp"

#include	"Resource.h"


// --- Type definitions --------------------------------------------------------

typedef struct {
	bool		usedCell;
	bool		isText;
	bool		filler_1;
	bool		filler_2;
} MyWindType;

typedef struct {
	unsigned short	platform;
	unsigned short	filler;
	UInt32	lastDetail;
	UInt32	lastWorksheet;
	UInt32	lastLayout;
	UInt32	last3DDocument;
} PrefsData;



// --- Variables declarations --------------------------------------------------

static GS::Array<MyWindType>	myWindows;
static Int32			nText = 0;
static Int32			nPaint = 0;
static Int32			old_nText = 0;
static Int32			old_nPaint = 0;
static PrefsData	*myPrefs = NULL;


// --- Function Prototypes -----------------------------------------------------

static void		Do_RebuildMyWindow (short userRefCon);
static void		Do_ActivateMyWindow (short userRefCon);
static void		Do_CloseMyWindow (Int32 userRefCon, bool isText);

static void		DisableEnableCloseCmd (void);

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


// =============================================================================
//
// Handling Custom Window Notifications
//
// =============================================================================

static GSErrCode __ACENV_CALL	TestCustomWindowHandler (short userRefCon, API_NotifyWindowEventID notifID)
{
	char	msgStr[256];

	switch (notifID) {
		case APINotifyWindow_Activate:	Do_ActivateMyWindow (userRefCon);
										sprintf (msgStr, "<APINotifyWindow_Activate> %d", userRefCon);
										break;
		case APINotifyWindow_Rebuild:	Do_RebuildMyWindow (userRefCon);
										sprintf (msgStr, "<APINotifyWindow_Rebuild> %d", userRefCon);
										break;
		case APINotifyWindow_Close:		Do_CloseMyWindow (userRefCon, false);
										sprintf (msgStr, "<APINotifyWindow_Close> %d", userRefCon);
										break;
	}

	ACAPI_WriteReport (msgStr, false);

	return NoError;
}


// =============================================================================
//
// Manage default windows/databases
//
// =============================================================================

// -----------------------------------------------------------------------------
// Scan all the section databases and duplicate the line elements
//	 - duplicated elements will have another color
//   - the endpoins of duplicated elements will be moved by a vector (0.5, 0.5)
// Place a line to each section marker on the floorplan
//	 - the begPoint will match to section marker
//   - the endPoint will be moved by a vector (0.5, 0.5) multiplied by
//     the number of lines counted in the related section database
// You will get an alert for each section that hasn't been opened yet
// -----------------------------------------------------------------------------

static void	Do_ListSectionLines (void)
{
	API_Element			element;
	API_DatabaseInfo	db_sect, db_plan;
	GSErrCode			err;
	char				msgStr[256];

	ACAPI_Database (APIDb_GetCurrentDatabaseID, &db_plan, NULL);

	ACAPI_WriteReport ("Sections:", false);

	GS::Array<API_Guid>  cutPlaneList;
	if (ACAPI_Element_GetElemList (API_CutPlaneID, &cutPlaneList) != NoError) {
		ACAPI_WriteReport ("Error in ACAPI_Element_GetElemList ().", true);
		return;
	}

	for (GS::Array<API_Guid>::ConstIterator it = cutPlaneList.Enumerate (); it != NULL; ++it) {
		BNZeroMemory (&element, sizeof (API_Element));
		element.header.guid = *it;

		err = ACAPI_Element_Get (&element);
		if (err != NoError) {
			sprintf (msgStr, "Error in ACAPI_Element_Get: %d", (int) err);
			ACAPI_WriteReport (msgStr, true);
			continue;
		}

		if (!element.header.hasMemo || element.cutPlane.segment.nCoord < 2)
			continue;

		API_ElementMemo memo;
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		err = ACAPI_Element_GetMemo (element.header.guid, &memo);
		if (err != NoError) {
			sprintf (msgStr, "Error in ACAPI_Element_GetMemo: %d", (int) err);
			ACAPI_WriteReport (msgStr, true);
			continue;
		}

		const API_Coord c1 = memo.sectionLineCoords[0];
		const API_Coord c2 = memo.sectionLineCoords[1];
		ACAPI_DisposeElemMemoHdls (&memo);

		BNZeroMemory (&db_sect, sizeof (API_DatabaseInfo));
		db_sect.typeID = APIWind_SectionID;
		db_sect.databaseUnId  = element.cutPlane.segment.databaseID;

		err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &db_sect, NULL);
		if (err == APIERR_NOTINIT) {
			sprintf (msgStr, "  [%s]  not generated  elemSetId:%s", APIGuidToString (*it).ToCStr ().Get (), APIGuidToString (element.cutPlane.segment.databaseID.elemSetId).ToCStr ().Get ());
			ACAPI_WriteReport (msgStr, false);
			continue;
		}

		sprintf (msgStr, "  [%s]  %s  elemSetId:%s", APIGuidToString (*it).ToCStr ().Get (), element.cutPlane.segment.windOpened ? "open" : "closed",
				 APIGuidToString (element.cutPlane.segment.databaseID.elemSetId).ToCStr ().Get ());
		ACAPI_WriteReport (msgStr, false);

		GS::Array<API_Guid>  lineList;
		if (ACAPI_Element_GetElemList (API_LineID, &lineList) != NoError) {
			ACAPI_WriteReport ("Error in ACAPI_Element_GetElemList ().", true);
			return;
		}

		for (GS::Array<API_Guid>::ConstIterator jt = lineList.Enumerate (); jt != NULL; ++jt) {
			BNZeroMemory (&element, sizeof (API_Element));
			element.header.guid = *jt;

			err = ACAPI_Element_Get (&element);
			if (err == NoError) {
				element.line.linePen++;
				element.line.endC.x += 0.5;
				element.line.endC.y += 0.5;
				err = ACAPI_Element_Create (&element, NULL);
			}
		}
		err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &db_plan, NULL);

		element.header.layer = 1;
		element.line.begC    = c1;
		element.line.endC.x  = c2.x + lineList.GetSize () * 0.2;
		element.line.endC.y  = c2.y + lineList.GetSize () * 0.2;
		err = ACAPI_Element_Create (&element, NULL);
	}
}		// Do_ListSectionLines



// =============================================================================
//
// Manage custom windows/databases
//
// =============================================================================
#ifdef __APPLE__
#pragma mark -
#endif

// -----------------------------------------------------------------------------
// Search for a window in the registry
// -----------------------------------------------------------------------------

static UInt32	GetFirstUnusedIndex (void)
{
	UInt32	i;

	for (i = 0; i < myWindows.GetSize (); i++) {
		if (!myWindows[i].usedCell)
			return i;
	}

	myWindows.SetSize (i + 1);

	return i;
}		// GetFirstUnusedIndex


// -----------------------------------------------------------------------------
// Search for a window in the registry
// -----------------------------------------------------------------------------

static Int32		SearchAWindow (bool isText)
{
	UInt32	i;

	for (i = 0; i < myWindows.GetSize (); i++) {
		if (!myWindows[i].usedCell)
			continue;
		if (myWindows[i].isText == isText)
			return (Int32) i;
	}

	return -1;
}		// SearchAWindow


// -----------------------------------------------------------------------------
// Disable/Enable a menu item
// -----------------------------------------------------------------------------

static void		DisableEnableMenuItemCmd (short menuResID, short itemIndex, bool disable)
{
	API_MenuItemRef		itemRef;
	GSFlags				itemFlags;

	BNZeroMemory (&itemRef, sizeof (API_MenuItemRef));
	itemRef.menuResID = menuResID;
	itemRef.itemIndex = itemIndex;
	itemFlags = 0;

	if (ACAPI_Interface (APIIo_GetMenuItemFlagsID, &itemRef, &itemFlags) != NoError)
		return;

	if (disable)
		itemFlags |= API_MenuItemDisabled;
	else
		itemFlags &= ~API_MenuItemDisabled;

	ACAPI_Interface (APIIo_SetMenuItemFlagsID, &itemRef, &itemFlags);

	return;
}		// DisEnableMenuItem


// -----------------------------------------------------------------------------
// Manage the availability of window close commands
// -----------------------------------------------------------------------------

void		DisableEnableCloseCmd (void)
{
	if (old_nText != nText && (old_nText == 0 || nText == 0))
		DisableEnableMenuItemCmd (IDR_OwnMenu, ID_MENU_CloseText, (nText <= 0));

	if (old_nPaint != nPaint && (old_nPaint == 0 || nPaint == 0))
		DisableEnableMenuItemCmd (IDR_OwnMenu, ID_MENU_CloseDraw, (nPaint <= 0));

	if ((old_nText + old_nPaint != nText + nPaint) && (old_nText + old_nPaint <= 0 || nText + nPaint <= 0))
		DisableEnableMenuItemCmd (IDR_OwnMenu, ID_MENU_CloseAll, (nText + nPaint <= 0));

	old_nText = nText;
	old_nPaint = nPaint;

	return;
}		// DisableEnableCloseCmd


// -----------------------------------------------------------------------------
// Rebuild my window
//	 - draw: copy all lines from the floorplan
//	 - text: count lines on the floorplan
// !!! Switching to another database is a relatively expensive operation !!!
//	 - this example uses this service extensively
//	 - it just for better understanding of the database structure
//	 - should be optimized: ie. collect lines in advance then copy all in one step
// -----------------------------------------------------------------------------
void		Do_RebuildMyWindow (short userRefCon)
{
	API_DatabaseInfo	planDB;
	bool				isText;
	API_Element			element;
	GSErrCode			err;

	isText = myWindows[userRefCon].isText;

	BNZeroMemory (&planDB, sizeof (API_DatabaseInfo));
	planDB.typeID = APIWind_FloorPlanID;

	if (isText) {
		API_WindowInfo      myWD;
		API_DatabaseInfo	origDB;

		// save original
		ACAPI_Database (APIDb_GetCurrentDatabaseID, &origDB, NULL);

		// reset content
		BNZeroMemory (&myWD, sizeof (API_WindowInfo));
		myWD.typeID = APIWind_MyTextID;
		myWD.index  = userRefCon;
		ACAPI_Database (APIDb_ResetCurrentWindowID, &myWD, NULL);

		// switch to floorplan
		err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &planDB, NULL);
		GS::Array<API_Guid> lineList;
		ACAPI_Element_GetElemList (API_LineID, &lineList);

		GS::Array<API_Guid>  visibleLinesList;
		if (ACAPI_Element_GetElemList (API_LineID, &visibleLinesList, APIFilt_OnVisLayer | APIFilt_OnActFloor) != NoError) {
			ACAPI_WriteReport ("Error in ACAPI_Element_GetElemList ().", true);
			return;
		}

		// write content
		char	buffer[256];
		sprintf (buffer, "Number of lines: %ld\n", (GS::LongForStdio) lineList.GetSize ());
		ACAPI_Database (APIDb_AddTextWindowContentID, &myWD, buffer);
		sprintf (buffer, "Number of visible lines: %ld\n", (GS::LongForStdio) visibleLinesList.GetSize ());
		ACAPI_Database (APIDb_AddTextWindowContentID, &myWD, buffer);

		// reset original
		err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &origDB, NULL);

	} else {
		API_DatabaseInfo	myDB;

		// database should be mine
		ACAPI_Database (APIDb_GetCurrentDatabaseID, &myDB, NULL);
		if (myDB.typeID != APIWind_MyDrawID || myDB.index != userRefCon) {
			DBBREAK_STR ("Wrong active database");
			return;
		}

		// reset content
		ACAPI_Database (APIDb_ResetCurrentDatabaseID, NULL, NULL);

		// switch to floorplan
		err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &planDB, NULL);
		GS::Array<API_Guid>  visibleLinesList;
		if (ACAPI_Element_GetElemList (API_LineID, &visibleLinesList, APIFilt_OnVisLayer | APIFilt_OnActFloor) != NoError) {
			ACAPI_WriteReport ("Error in ACAPI_Element_GetElemList ().", true);
			return;
		}

		API_GridType		grid;
		ACAPI_Database (APIDb_GetGridSettingsID, &grid, NULL);
		grid.gridColor.f_red   = 1.0;
		grid.gridColor.f_green = 0.0;
		grid.gridColor.f_blue  = 0.0;

		// switch to mine
		err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &myDB, NULL);

		for (GS::Array<API_Guid>::ConstIterator it = visibleLinesList.Enumerate (); it != NULL; ++it) {
			BNZeroMemory (&element, sizeof (API_Element));
			element.header.guid = *it;

			// switch to floorplan
			err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &planDB, NULL);

			err = ACAPI_Element_Get (&element);
			if (err == NoError) {
				element.header.floorInd = 0;

				// switch to mine
				err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &myDB, NULL);
				err = ACAPI_Element_Create (&element, NULL);
			}
			if (err != NoError) {
				DBPrintf ("Line with GUID %s cannot be moved.\n", APIGuidToString (*it).ToCStr ().Get ());
				err = NoError;
			}
		}

		// reset original (mine)
		ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &myDB, NULL);

		ACAPI_Database (APIDb_ChangeGridSettingsID, &grid, NULL);
		ACAPI_Database (APIDb_RebuildCurrentDatabaseID, NULL, NULL);
	}
}		// Do_RebuildMyWindow


// -----------------------------------------------------------------------------
// Activate my window
// -----------------------------------------------------------------------------

void		Do_ActivateMyWindow (short userRefCon)
{
	Do_RebuildMyWindow (userRefCon);

	return;
}		// Do_ActivateMyWindow


// -----------------------------------------------------------------------------
// Create a custom window from API
// -----------------------------------------------------------------------------

static void		Do_OpenMyWindow (bool isText)
{
	API_NewWindowPars	wPars;
	Int32				userRefCon;
	GSErrCode			err;
	char				msgStr[256];

	userRefCon = GetFirstUnusedIndex ();

	BNZeroMemory (&wPars, sizeof (API_NewWindowPars));

	wPars.typeID = isText ? APIWind_MyTextID : APIWind_MyDrawID;
	wPars.userRefCon = (short) userRefCon;

	GS::snuprintf (wPars.wTitle, sizeof (wPars.wTitle) / sizeof (GS::uchar_t), L("API window: %ld"), userRefCon);
	GS::ucscpy (wPars.wFile, L("API window"));

	err = ACAPI_Database (APIDb_NewWindowID, &wPars, (void *) TestCustomWindowHandler);

	if (err == NoError) {
		MyWindType	myWind;

		BNZeroMemory (&myWind, sizeof (MyWindType));
		myWind.usedCell = true;
		myWind.isText = isText;
		myWindows [userRefCon] = myWind;
		Do_RebuildMyWindow (wPars.userRefCon);
		if (isText)
			nText++;
		else
			nPaint++;
	} else if (err == APIERR_WINDEXIST) {
		sprintf (msgStr, "API window with userRefCon %ld already exists.", (GS::LongForStdio) userRefCon);
		ACAPI_WriteReport (msgStr, true);
	} else {
		sprintf (msgStr, "Cannot open %s window with userRefCon %ld.", (isText ? "APIWind_MyTextID" : "APIWind_MyDrawID"), (GS::LongForStdio) userRefCon);
		ACAPI_WriteReport (msgStr, true);
	}

	DisableEnableCloseCmd ();

	return;
}		// Do_OpenMyWindow


// -----------------------------------------------------------------------------
// Close my custom window
// -----------------------------------------------------------------------------

void		Do_CloseMyWindow (Int32 userRefCon, bool isText)
{
	if (userRefCon == -1) {		// from menu
		userRefCon = SearchAWindow (isText);
		if (userRefCon == -1)
			return;

		API_WindowInfo		wPars;
		BNZeroMemory (&wPars, sizeof (API_WindowInfo));
		wPars.typeID = myWindows[userRefCon].isText ? APIWind_MyTextID : APIWind_MyDrawID;
		wPars.index  = userRefCon;

		GSErrCode			err;
		myWindows[userRefCon].usedCell = false;		// ACAPI will notify the add-on on the closing, it shouldn't decrement the counter twice
		err = ACAPI_Database (APIDb_CloseWindowID, &wPars, NULL);
		if (err != NoError)
			DBPrintf ("Database Control:: ACAPI_Database(APIDb_CloseWindowID) failed\n");

	} else {				// from event
		if (!myWindows[userRefCon].usedCell)			// we know it's closed, don't do anything
			return;
		myWindows[userRefCon].usedCell = false;
		isText = myWindows[userRefCon].isText;
	}

	if (isText)
		nText--;
	else
		nPaint--;

	DisableEnableCloseCmd ();

	return;
}		// Do_CloseMyWindow


// -----------------------------------------------------------------------------
// Close all windows
// -----------------------------------------------------------------------------

static void		Do_CloseAllWindows (void)
{
	while (nText > 0)
		Do_CloseMyWindow (-1, true);

	while (nPaint > 0)
		Do_CloseMyWindow (-1, false);

	return;
}		// Do_CloseAllWindows


// -----------------------------------------------------------------------------
// Create a new independent detail
// -----------------------------------------------------------------------------


static void		Do_CreateDetail ()
{

	API_DatabaseInfo		dbInfo;
	BNZeroMemory (&dbInfo, sizeof (API_DatabaseInfo));
	dbInfo.typeID = APIWind_DetailID;
	usprintf (dbInfo.name, sizeof (dbInfo.name), "Dooku %ld", ++myPrefs->lastDetail);
	usprintf (dbInfo.ref, sizeof (dbInfo.ref), "D%02ld", myPrefs->lastDetail);


	if (ACAPI_Database (APIDb_NewDatabaseID, &dbInfo, NULL) == NoError) {
		DBPrintf ("DatabaseControl :: The ID of the new detail database is \"%s\"\n",
			APIGuidToString (dbInfo.databaseUnId.elemSetId).ToCStr ().Get ());
	}

	return;
}		// Do_CreateDetail


// -----------------------------------------------------------------------------
// Create a new independent worksheet
// -----------------------------------------------------------------------------


static void		Do_CreateWorksheet ()
{

	API_DatabaseInfo		dbInfo;
	BNZeroMemory (&dbInfo, sizeof (API_DatabaseInfo));
	dbInfo.typeID = APIWind_WorksheetID;
	usprintf (dbInfo.name, sizeof (dbInfo.name), "Zaphod %ld", ++myPrefs->lastWorksheet);
	usprintf (dbInfo.ref, sizeof (dbInfo.ref), "W%02ld", myPrefs->lastWorksheet);


	if (ACAPI_Database (APIDb_NewDatabaseID, &dbInfo, NULL) == NoError) {
		DBPrintf ("DatabaseControl :: The ID of the new worksheet database is \"%s\"\n",
			APIGuidToString (dbInfo.databaseUnId.elemSetId).ToCStr ().Get ());
	}

	return;
}		// Do_CreateWorksheet


// -----------------------------------------------------------------------------
// Create a new 3D Document
// -----------------------------------------------------------------------------


static void		Do_Create3DDocument ()
{

	API_DatabaseInfo		dbInfo;
	BNZeroMemory (&dbInfo, sizeof (API_DatabaseInfo));
	dbInfo.typeID = APIWind_DocumentFrom3DID;
	usprintf (dbInfo.name, sizeof (dbInfo.name), "Document3D %ld", ++myPrefs->last3DDocument);
	usprintf (dbInfo.ref, sizeof (dbInfo.ref), "3D%02ld", myPrefs->last3DDocument);


	if (ACAPI_Database (APIDb_NewDatabaseID, &dbInfo, NULL) == NoError) {
		DBPrintf ("DatabaseControl :: The ID of the new Document 3D database is \"%s\"\n",
			APIGuidToString (dbInfo.databaseUnId.elemSetId).ToCStr ().Get ());
	}

	return;
}		// Do_Create3DDocument


// -----------------------------------------------------------------------------
// Create a new layout with master
// -----------------------------------------------------------------------------

static void		Do_CreateLayout ()
{
	API_DatabaseInfo		dbInfo;
	BNZeroMemory (&dbInfo, sizeof (API_DatabaseInfo));
	dbInfo.typeID = APIWind_MasterLayoutID;
	usprintf (dbInfo.name, sizeof (dbInfo.name), "Yoda %ld", ++myPrefs->lastLayout);

	if (ACAPI_Database (APIDb_NewDatabaseID, &dbInfo, NULL) == NoError) {
		DBPrintf ("DatabaseControl :: The ID of the new master layout database is \"%s\"\n",
			APIGuidToString (dbInfo.databaseUnId.elemSetId).ToCStr ().Get ());

		dbInfo.typeID = APIWind_LayoutID;
		dbInfo.masterLayoutUnId = dbInfo.databaseUnId;
		usprintf (dbInfo.name, sizeof (dbInfo.name), "Luke %ld", myPrefs->lastLayout);
		if (ACAPI_Database (APIDb_NewDatabaseID, &dbInfo, NULL) == NoError)
			DBPrintf ("DatabaseControl :: The ID of the new layout database is \"%s\"\n",
				APIGuidToString (dbInfo.databaseUnId.elemSetId).ToCStr ().Get ());
	}

	return;
}		// Do_CreateLayout


// -----------------------------------------------------------------------------
// Rename the last independent detail or layout database
// -----------------------------------------------------------------------------

static void		Do_RenameDatabase (API_DatabaseID commandID)
{
	API_DatabaseUnId	*dbases = NULL;

	if (ACAPI_Database (commandID, &dbases, NULL) == NoError) {
		GSSize nDbases = BMpGetSize (reinterpret_cast<GSPtr>(dbases)) / Sizeof32 (API_DatabaseUnId);
		if (nDbases > 0) {
			API_DatabaseInfo api_dbPars;
			BNZeroMemory (&api_dbPars, sizeof (API_DatabaseInfo));

			api_dbPars.databaseUnId = dbases[nDbases - 1];
			if (ACAPI_Database (APIDb_GetDatabaseInfoID, &api_dbPars, NULL) == NoError) {
				switch (commandID) {
					case APIDb_GetDetailDatabasesID:		usprintf (api_dbPars.name, sizeof (api_dbPars.name), "Dooku %ld", ++myPrefs->lastDetail);
															usprintf (api_dbPars.ref, sizeof (api_dbPars.ref), "D%02ld", myPrefs->lastDetail);
															break;
					case APIDb_GetWorksheetDatabasesID:		usprintf (api_dbPars.name, sizeof (api_dbPars.name), "Zaphod %ld", ++myPrefs->lastWorksheet);
															usprintf (api_dbPars.ref, sizeof (api_dbPars.ref), "W%02ld", myPrefs->lastWorksheet);
															break;
					case APIDb_GetDocumentFrom3DDatabasesID: usprintf (api_dbPars.name, sizeof (api_dbPars.name), "Document3D %ld", ++myPrefs->last3DDocument);
															 usprintf (api_dbPars.ref, sizeof (api_dbPars.ref), "3D%02ld", myPrefs->last3DDocument);
															 break;
					case APIDb_GetLayoutDatabasesID:		usprintf (api_dbPars.name, sizeof (api_dbPars.name), "Luke %ld", ++myPrefs->lastLayout);
															{
																API_LayoutInfo	layoutInfo;			// temporary here
																BNZeroMemory (&layoutInfo, sizeof (API_LayoutInfo));
																if (ACAPI_Environment (APIEnv_GetLayoutSetsID, &layoutInfo, &api_dbPars.databaseUnId) == NoError) {
																	double origVal = layoutInfo.sizeX;
																	layoutInfo.sizeX = layoutInfo.sizeY;
																	layoutInfo.sizeY = origVal;
																	origVal = layoutInfo.leftMargin;
																	layoutInfo.leftMargin = layoutInfo.topMargin;
																	layoutInfo.topMargin = origVal;
																	origVal = layoutInfo.rightMargin;
																	layoutInfo.rightMargin = layoutInfo.bottomMargin;
																	layoutInfo.bottomMargin = origVal;
																	layoutInfo.customLayoutNumbering = true;
																	CHCopyC ("NUMBER", layoutInfo.customLayoutNumber);
																	if (layoutInfo.customData != NULL) {
																		for (auto it = layoutInfo.customData->EnumeratePairs (); it != NULL; ++it) {
																			*it->value += " - Modified via API";
																		}
																	}
																	ACAPI_Environment (APIEnv_ChangeLayoutSetsID, &layoutInfo, &api_dbPars.databaseUnId);
																	if (layoutInfo.customData != NULL)
																		delete layoutInfo.customData;
																}
															}
															break;
					case APIDb_GetMasterLayoutDatabasesID:	usprintf (api_dbPars.name, sizeof (api_dbPars.name), "Yoda %ld", ++myPrefs->lastLayout);
															break;
					default:								break;
				}

				ACAPI_Database (APIDb_ModifyDatabaseID, &api_dbPars, NULL);
			}
		}
	}

	if (dbases != NULL)
		BMpFree (reinterpret_cast<GSPtr>(dbases));

	return;

}		// Do_RenameDatabase


// -----------------------------------------------------------------------------
// Delete the last independent detail or layout database
// -----------------------------------------------------------------------------

static void		Do_DeleteDatabase (API_DatabaseID commandID)
{
	API_DatabaseUnId	*dbases = NULL;

	if (ACAPI_Database (commandID, &dbases, NULL) == NoError) {
		GSSize nDbases = BMpGetSize (reinterpret_cast<GSPtr>(dbases)) / Sizeof32 (API_DatabaseUnId);
		if (nDbases > 0) {
			API_DatabaseInfo api_dbPars;
			BNZeroMemory (&api_dbPars, sizeof (API_DatabaseInfo));

			api_dbPars.databaseUnId = dbases[nDbases - 1];
			ACAPI_Database (APIDb_GetDatabaseInfoID, &api_dbPars, NULL);

			if (ACAPI_Database (APIDb_DeleteDatabaseID, &api_dbPars, NULL) != NoError) {
				char msgStr[512];
				sprintf (msgStr, "Cannot delete database: \"%s\"", (const char *) GS::UniString (api_dbPars.title).ToCStr ());
				ACAPI_WriteReport (msgStr, true);
			}
		}
	}

	if (dbases != NULL)
		BMpFree (reinterpret_cast<GSPtr>(dbases));

	return;
}		// Do_DeleteDatabase


// -----------------------------------------------------------------------------
// CreateSubset
// -----------------------------------------------------------------------------

static void		Do_CreateSubset ()
{
	API_SubSet subSet;
	BNZeroMemory (&subSet, sizeof (API_SubSet));
	GSErrCode err = ACAPI_Database (APIDb_GetSubSetDefaultID, &subSet, NULL);
	if (err == NoError) {
		GS::UniString ownPrefix ("API");
		GS::ucsncpy (subSet.ownPrefix, ownPrefix.ToUStr (), sizeof (subSet.ownPrefix) / sizeof (GS::uchar_t) - 1);
		subSet.addOwnPrefix = true;

		GS::UniString name ("This Subset was created via API");
		GS::ucsncpy (subSet.name, name.ToUStr (), sizeof (subSet.name) / sizeof (GS::uchar_t) - 1);

		err = ACAPI_Database (APIDb_CreateSubSetID, (void*)&subSet, NULL);
		if (err != NoError)
			DBPrintf ("APIDb_CreateSubSetID function failed");
	}

	return;
}		// Do_CreateSubset


// -----------------------------------------------------------------------------
// Dump info for certain databases
// -----------------------------------------------------------------------------

static void		Do_ListDatabases (API_DatabaseID commandID, const char * typeStr)
{
	API_DatabaseUnId	*dbases = NULL;

	if (ACAPI_Database (commandID, &dbases, NULL) == NoError) {
		GSSize nDbases = BMpGetSize (reinterpret_cast<GSPtr>(dbases)) / Sizeof32 (API_DatabaseUnId);
		DBPrintf ("DatabaseControl :: The number of %s databases is %d\n", typeStr, nDbases);

		bool	isMasterLayout = CHCompareCStrings (typeStr, "master layout") == 0;
		bool	isLayout = CHCompareCStrings (typeStr, "layout") == 0;
		for (GSIndex i = 0; i < nDbases; i++) {
			API_DatabaseInfo dbPars;
			BNZeroMemory (&dbPars, sizeof (API_DatabaseInfo));
			dbPars.databaseUnId = dbases[i];

			if (ACAPI_Database (APIDb_GetDatabaseInfoID, &dbPars, NULL) == NoError) {
				DBPrintf ("DatabaseControl :: %s index[%d] unId: \"%s\" title:\"%s\" name:\"%s\" ref:\"%s\"\n", typeStr, dbPars.index,
							APIGuidToString (dbPars.databaseUnId.elemSetId).ToCStr ().Get (), dbPars.title, dbPars.name, dbPars.ref);
				if (isMasterLayout || isLayout) {
					API_LayoutInfo	layoutSets;
					if (ACAPI_Environment (APIEnv_GetLayoutSetsID, &layoutSets, &dbPars.databaseUnId, NULL) == NoError) {
						if (layoutSets.customData != NULL) {
							delete layoutSets.customData;
							layoutSets.customData = NULL;
						}

						if (layoutSets.layoutPageNumber <= 1 || isMasterLayout) {
							DBPrintf ("                   size (mm, x-y): %5.1f x %5.1f; margins (mm, l-r-t-b): %5.1f, %5.1f, %5.1f, %5.1f\n",
								layoutSets.sizeX, layoutSets.sizeY,
								layoutSets.leftMargin, layoutSets.rightMargin, layoutSets.topMargin, layoutSets.bottomMargin);
							if (isLayout) {
								DBPrintf ("Do not include this Layout in Numbering: %s\n", layoutSets.doNotIncludeInNumbering ? "true" : "false");
								DBPrintf ("Is custom Layout Number: %s\n", layoutSets.customLayoutNumbering ? "true" : "false");
								DBPrintf ("Custom Layout Number: %s\n", layoutSets.customLayoutNumber);
							}
						}
						else {
							DBPrintf ("                   size (mm, x-y): %5.1f x %5.1f; margins (mm, l-r-t-b): %5.1f, %5.1f, %5.1f, %5.1f\n",
								layoutSets.sizeX, layoutSets.sizeY,
								layoutSets.leftMargin, layoutSets.rightMargin, layoutSets.topMargin, layoutSets.bottomMargin);
							if (isLayout) {
								DBPrintf ("Do not include this Layout in Numbering: %s\n", layoutSets.doNotIncludeInNumbering ? "true" : "false");
								DBPrintf ("Is custom Layout Number: %s\n", layoutSets.customLayoutNumbering ? "true" : "false");
								DBPrintf ("Custom Layout Number: %s\n", layoutSets.customLayoutNumber);
							}

							ULong actPgidx;
							for (actPgidx = 1; actPgidx <= layoutSets.layoutPageNumber; actPgidx++) {
								ACAPI_Environment (APIEnv_GetLayoutSetsID, &layoutSets, &dbPars.databaseUnId, &actPgidx);
								if (layoutSets.customData != NULL) {
									delete layoutSets.customData;
									layoutSets.customData = NULL;
								}
								DBPrintf ("    %d: %s\n", actPgidx, layoutSets.layoutName);
							}
						}
					}
				}
			}
		}
	}

	if (dbases != NULL)
		BMpFree (reinterpret_cast<GSPtr>(dbases));

	return;
}		// Do_ListDatabases


// -----------------------------------------------------------------------------
// Change 3D Document settings
// -----------------------------------------------------------------------------

static void		Do_Change3DDocument ()
{
	API_DatabaseUnId	*dbases = NULL;

	if (ACAPI_Database (APIDb_GetDocumentFrom3DDatabasesID, &dbases, NULL) == NoError) {
		GSSize nDbases = BMpGetSize (reinterpret_cast<GSPtr>(dbases)) / Sizeof32 (API_DatabaseUnId);

		if (nDbases > 0) {
			API_DatabaseInfo dbPars;
			API_DocumentFrom3DType documentFrom3DType;
			BNZeroMemory (&dbPars, sizeof (API_DatabaseInfo));
			dbPars.databaseUnId = dbases[0];

			GSErrCode err = ACAPI_Environment (APIEnv_GetDocumentFrom3DSettingsID, &dbPars.databaseUnId, &documentFrom3DType);
			if (err == NoError) {
				documentFrom3DType.attributeOptions.shouldUseUniformCutAttributes = !documentFrom3DType.attributeOptions.shouldUseUniformCutAttributes;
				err = ACAPI_Environment (APIEnv_ChangeDocumentFrom3DSettingsID, &dbPars.databaseUnId, &documentFrom3DType);
				if (err != NoError)
					DBPrintf ("APIEnv_ChangeDocumentFrom3DSettingsID returns with error code");
			} else {
				DBPrintf ("APIEnv_GetDocumentFrom3DSettingsID returns with error code");
			}

			BMKillHandle (reinterpret_cast<GSHandle*> (&documentFrom3DType.cutSetting.shapes));
		}
	}

	if (dbases != NULL)
		BMpFree (reinterpret_cast<GSPtr>(dbases));
}


// -----------------------------------------------------------------------------
// Dump info for drawing links in the current databasee
// -----------------------------------------------------------------------------

static void		Do_ListDrawingLinks ()
{
	GSErrCode	err = NoError;

	GS::IntPtr	store = 1;
	err = ACAPI_Database (APIDb_StoreViewSettingsID, (void *) store, NULL);
	if (err != NoError) {
		store = -1;
		err = NoError;
	}

	API_DatabaseInfo	currDB;
	BNZeroMemory (&currDB, sizeof (API_DatabaseInfo));
	ACAPI_Database (APIDb_GetCurrentDatabaseID, &currDB, NULL);

	GS::Array<API_Guid>  drawingList;
	if (ACAPI_Element_GetElemList (API_DrawingID, &drawingList) != NoError) {
		ACAPI_WriteReport ("Error in ACAPI_Element_GetElemList ().", true);
		return;
	}

	for (GS::Array<API_Guid>::ConstIterator it = drawingList.Enumerate (); it != NULL; ++it) {
		API_Element		elem;
		BNZeroMemory (&elem, sizeof (API_Element));
		elem.header.guid = *it;

		err = ACAPI_Element_Get (&elem);
		if (err != NoError) {
			DBPrintf ("DatabaseControl :: Do_ListDrawingLinks :: Cannot get Drawing with GUID %s.\n", APIGuidToString (*it).ToCStr ().Get ());
			continue;
		}

		API_DrawingLinkInfo		drwLinkInfo;
		err = ACAPI_Database (APIDb_GetDrawingLinkID, (void*) (&(elem.header.guid)), &drwLinkInfo);
		if (err != NoError) {
			DBPrintf ("DatabaseControl :: Do_ListDrawingLinks :: Cannot get link information for Drawing with GUID %s.\n", APIGuidToString (*it).ToCStr ().Get ());
			if (drwLinkInfo.linkPath != NULL)
				delete drwLinkInfo.linkPath;
			if (drwLinkInfo.viewPath != NULL)
				BMKillPtr (&(drwLinkInfo.viewPath));
			continue;
		}

		DBPrintf ("\tlink No. %ld :", elem.drawing.linkUId);
		if (drwLinkInfo.linkPath != NULL) {
			IO::Path		path;
			drwLinkInfo.linkPath->ToDisplayText (&path);
			DBPrintf ("\t%s", (const char *) path);
			delete drwLinkInfo.linkPath;
		}
		if (drwLinkInfo.viewPath != NULL)
			BMKillPtr (&(drwLinkInfo.viewPath));
		if (drwLinkInfo.linkTypeID == API_DrawingLink_InternalViewID)
			DBPrintf ("\t (internal %s, guid: %s)", (drwLinkInfo.viewType == API_ViewNodePerspective || drwLinkInfo.viewType == API_ViewNodeAxonometry) ? "3D view" : "view", APIGuidToString (drwLinkInfo.linkGuid).ToCStr ().Get ());

		API_DatabaseInfo	dbInfo;
		BNZeroMemory (&dbInfo, sizeof (API_DatabaseInfo));
		dbInfo.typeID = APIWind_DrawingID;
		dbInfo.linkedElement  = elem.header.guid;
		if (ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &dbInfo, NULL) == NoError) {
			GS::Array<API_Guid> elemList;
			ACAPI_Element_GetElemList (API_LineID, &elemList);
			DBPrintf ("\t(number of lines = %ld,", elemList.GetSize ());

			short	visible, onStory, inView;
			visible = onStory = inView = 0;
			for (GS::Array<API_Guid>::ConstIterator it = elemList.Enumerate (); it != NULL; ++it) {
				if (ACAPI_Element_Filter (*it, APIFilt_OnVisLayer))
					++visible;
				if (ACAPI_Element_Filter (*it, APIFilt_OnActFloor))
					++onStory;
				if (ACAPI_Element_Filter (*it, APIFilt_InCroppedView))
					++inView;
			}
			DBPrintf ("%d visible, %d on the view\'s floor, %d in the cropped view", visible, onStory, inView);

			// Switch back to the original database, because APIDb_ChangeCurrentDatabaseID tries to fetch the drawing element
			//	from the layout's database internally
			ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &currDB, NULL);
		}

		DBPrintf ("\n");

		DBPrintf ("\tName: %s", drwLinkInfo.name);
		DBPrintf ("\n");
		DBPrintf ("\tNumber %s", drwLinkInfo.number);
		DBPrintf ("\n");

		short userId = 0;
		if (ACAPI_Database (APIDb_GetTWOwnerID, &(dbInfo.databaseUnId), &userId) == NoError)
			DBPrintf ("\tTeamWork owner ID: %d\n", userId);
	}

	if (store == 1) {
		store = 0;
		ACAPI_Database (APIDb_StoreViewSettingsID, (void *) store, NULL);
	}
}		// Do_ListDrawingLinks


// -----------------------------------------------------------------------------
// Change all drawings (enumerate layout databases)
// -----------------------------------------------------------------------------

static void		Do_ChangeAllDrawings (void)
{
	API_DatabaseUnId	*dbases = NULL;
	API_DatabaseInfo	db_orig;
	GSErrCode			err = ACAPI_Database (APIDb_GetCurrentDatabaseID, &db_orig, NULL);
	if (err != NoError)
		return;

	err = ACAPI_CallUndoableCommand ("Drawing was changed",
		[&] () -> GSErrCode {
			API_DatabaseInfo	db_origLayout;
			bool				undoWasOpened = false;
			GSSize				nDbases = 0;
			GSErrCode			err = NoError;


			BNZeroMemory (&db_orig, sizeof (API_DatabaseInfo));
			BNZeroMemory (&db_origLayout, sizeof (API_DatabaseInfo));

			if (ACAPI_Database (APIDb_GetLayoutDatabasesID, &dbases, NULL) == NoError) {
				nDbases = BMpGetSize (reinterpret_cast<GSPtr>(dbases)) / Sizeof32 (API_DatabaseUnId);
				DBPrintf ("DatabaseControl :: The number of layout databases is %d\n", nDbases);

				short penTableCount;
				API_Attribute attribute;
				BNZeroMemory (&attribute, sizeof (API_Attribute));
				attribute.header.typeID = API_PenTableID;
				short penTableIdx;
				err = ACAPI_Attribute_GetNum (API_PenTableID, &penTableCount);
				for (penTableIdx = penTableCount; penTableIdx > 0; --penTableIdx) {
					attribute.header.index = penTableIdx;
					if (ACAPI_Attribute_Get (&attribute) == NoError)
						break;
				}

				if (penTableIdx > 0) {
					penTableCount = penTableIdx;
				} else {
					DBPrintf ("Count of penTables is zero");
					return err;
				}

				for (GSIndex i = 0; i < nDbases; i++) {
					API_DatabaseInfo dbPars;
					BNZeroMemory (&dbPars, sizeof (API_DatabaseInfo));
					dbPars.databaseUnId = dbases[i];
					dbPars.typeID = APIWind_LayoutID;


					err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &dbPars, NULL);
					if (err != NoError) {
						DBPrintf ("APIDb_ChangeCurrentDatabaseID returns with error code");
						break;
					}

					if (!undoWasOpened) {
						undoWasOpened = true;
						db_origLayout = dbPars;
					}

					GS::Array<API_Guid>  drawingList;
					err = ACAPI_Element_GetElemList (API_DrawingID, &drawingList);
					if (err == NoError) {
						API_Element mask;
						ACAPI_ELEMENT_MASK_CLEAR (mask);
						ACAPI_ELEMENT_MASK_SET (mask, API_DrawingType, penTableUsageMode);
						ACAPI_ELEMENT_MASK_SET (mask, API_DrawingType, penTableIndex);
						ACAPI_ELEMENT_MASK_SET (mask, API_DrawingType, nameType);
						ACAPI_ELEMENT_MASK_SET (mask, API_DrawingType, numberingType);
						ACAPI_ELEMENT_MASK_SET (mask, API_DrawingType, name);
						ACAPI_ELEMENT_MASK_SET (mask, API_DrawingType, customNumber);

						API_Element element;
						for (GS::Array<API_Guid>::ConstIterator it = drawingList.Enumerate (); it != NULL; ++it) {
							BNZeroMemory (&element, sizeof (API_Element));
							element.header.guid = *it;
							err = ACAPI_Element_Get (&element);
							if (err == NoError) {
								element.drawing.penTableUsageMode = APIPenTableUsageMode_UsePenTableIndex;
								element.drawing.penTableIndex = penTableCount;
								element.drawing.nameType = APIName_CustomName;
								element.drawing.numberingType = APINumbering_CustomNum;
								CHCopyC ("NAME", element.drawing.name);
								CHCopyC ("NUMBER", element.drawing.customNumber);
								err = ACAPI_Element_Change (&element, &mask, NULL, 0, true);
							}
						}
					}
				}
			}

			if (nDbases > 1) {
				err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &db_origLayout, NULL);
				DBPrintf ("APIDb_ChangeCurrentDatabaseID returns with error code");
			}

			return err;
		});

	ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &db_orig, NULL);

	if (dbases != NULL)
		BMpFree (reinterpret_cast<GSPtr> (dbases));
}		// Do_ChangeAllDrawings


// -----------------------------------------------------------------------------
// Change the Master Layout of the current Layout
// -----------------------------------------------------------------------------

static void		Do_ChangeMaster ()
{
	API_DatabaseInfo	db_Layout;
	ACAPI_Database (APIDb_GetCurrentDatabaseID, &db_Layout, NULL);

	if (db_Layout.typeID != APIWind_LayoutID)
		return;

	API_DatabaseUnId	*dbases = NULL;

	if (ACAPI_Database (APIDb_GetMasterLayoutDatabasesID, &dbases, NULL) == NoError) {
		GSSize nDbases = BMpGetSize (reinterpret_cast<GSPtr>(dbases)) / Sizeof32 (API_DatabaseUnId);
		if (nDbases > 1) {
			for (GSIndex i = 0; i < nDbases; i++) {
				API_DatabaseInfo api_dbPars;
				BNZeroMemory (&api_dbPars, sizeof (API_DatabaseInfo));
				api_dbPars.databaseUnId = dbases[i];
				if (ACAPI_Database (APIDb_GetDatabaseInfoID, &api_dbPars, NULL) == NoError) {
					if (db_Layout.masterLayoutUnId.elemSetId == api_dbPars.databaseUnId.elemSetId)
						continue;

					db_Layout.masterLayoutUnId = api_dbPars.databaseUnId;
					ACAPI_Database (APIDb_ModifyDatabaseID, &db_Layout, NULL);
					break;
				}
			}
		}
	}

	if (dbases != NULL)
		BMpFree (reinterpret_cast<GSPtr>(dbases));

	return;
}


// -----------------------------------------------------------------------------
// Get all dominant Changes
// -----------------------------------------------------------------------------

static void		Do_GetAllChanges (void)
{
	GS::Array<API_RVMChange> changes;
	GSErrCode err = ACAPI_Database (APIDb_GetRVMChangesID, &changes);
	if (err != NoError) {
		WriteReport ("Do_GetAllChanges: error occured!");
		return;
	}

	if (changes.IsEmpty ()) {
		WriteReport ("There are no changes!");
		return;
	}

	WriteReport ("# Changes:");

	char buffer[256];
	for (GS::Array<API_RVMChange>::ConstIterator it = changes.Enumerate(); it != NULL; ++it) {
		const API_RVMChange& change = *it;
		sprintf (buffer, "ID: %s, Description: %s", change.id.ToCStr ().Get (), change.description.ToCStr ().Get ());
		WriteReport (buffer);
	}
}


// -----------------------------------------------------------------------------
// Get the custom scheme of Changes
// -----------------------------------------------------------------------------

static void		Do_GetChangeCustomScheme (void)
{
	GS::Array<API_RVMChange> changes;
	GSErrCode err = ACAPI_Database (APIDb_GetRVMChangesID, &changes);
	if (err != NoError) {
		WriteReport ("Do_GetChangeCustomScheme: error occured!");
		return;
	}

	if (changes.IsEmpty ()) {
		WriteReport ("There are no changes!");
		return;
	}

	GS::HashTable<API_Guid, GS::UniString> changeCustomSchemes;
	err = ACAPI_Database (APIDb_GetRVMChangeCustomSchemeID, &changeCustomSchemes);
	if (err != NoError) {
		WriteReport ("Do_GetChangeCustomScheme: error occured!");
		return;
	}

	if (changeCustomSchemes.IsEmpty ()) {
		WriteReport ("There are no custom schemes for changes!");
		return;
	}

	WriteReport ("# Changes:");

	GS::UniString buffer;
	for (GS::Array<API_RVMChange>::ConstIterator it = changes.Enumerate(); it != NULL; ++it) {
		const API_RVMChange& change = *it;
		buffer.Clear ();
		buffer = GS::UniString::Printf ("ID: %T, ", change.id.ToPrintf ());
		bool firstLoop = true;
		for (GS::HashTable<API_Guid, GS::UniString>::PairIterator cIt = changeCustomSchemes.EnumeratePairs (); cIt != NULL; ++cIt) {
			const API_Guid&			guid				= *cIt->key;
			const GS::UniString&	customFieldName		= *cIt->value;
			GS::UniString			customFieldValue;

			if (!firstLoop)
				buffer.Append (", ");

			change.customData.Get (guid, &customFieldValue);
			buffer.Append (customFieldName + ": " + customFieldValue);

			firstLoop = false;
		}

		WriteReport (buffer.ToCStr ().Get ());
	}
}


// -----------------------------------------------------------------------------
// Get all Issues
// -----------------------------------------------------------------------------

static void		Do_GetAllIssues (void)
{
	GS::Array<API_RVMIssue> issues;
	GSErrCode err = ACAPI_Database (APIDb_GetRVMIssuesID, &issues);
	if (err != NoError) {
		WriteReport ("Do_GetAllIssues: error occured!");
		return;
	}

	if (issues.IsEmpty ()) {
		WriteReport ("There are no issues!");
		return;
	}

	WriteReport ("# Issues:");

	char buffer[256];
	for (GS::Array<API_RVMIssue>::ConstIterator it = issues.Enumerate(); it != NULL; ++it) {
		const API_RVMIssue& issue = *it;
		sprintf (buffer, "ID: %s, Description: %s", issue.id.ToCStr ().Get (), issue.description.ToCStr ().Get ());
		WriteReport (buffer);
	}
}


// -----------------------------------------------------------------------------
// Get the custom scheme of Issues
// -----------------------------------------------------------------------------

static void		Do_GetIssueCustomScheme (void)
{
	GS::Array<API_RVMIssue> issues;
	GSErrCode err = ACAPI_Database (APIDb_GetRVMIssuesID, &issues);
	if (err != NoError) {
		WriteReport ("Do_GetIssueCustomScheme: error occured!");
		return;
	}

	if (issues.IsEmpty ()) {
		WriteReport ("There are no issues!");
		return;
	}

	GS::HashTable<API_Guid, GS::UniString> issueCustomSchemes;
	err = ACAPI_Database (APIDb_GetRVMIssueCustomSchemeID, &issueCustomSchemes);
	if (err != NoError) {
		WriteReport ("Do_GetIssueCustomScheme: error occured!");
		return;
	}

	if (issueCustomSchemes.IsEmpty ()) {
		WriteReport ("There are no custom schemes for issues!");
		return;
	}

	WriteReport ("# Issues:");

	GS::UniString buffer;
	for (GS::Array<API_RVMIssue>::ConstIterator it = issues.Enumerate(); it != NULL; ++it) {
		const API_RVMIssue& issue = *it;
		buffer.Clear ();
		buffer = GS::UniString::Printf ("ID: %T, ", issue.id.ToPrintf ());
		bool firstLoop = true;
		for (GS::HashTable<API_Guid, GS::UniString>::PairIterator cIt = issueCustomSchemes.EnumeratePairs (); cIt != NULL; ++cIt) {
			const API_Guid&			guid				= *cIt->key;
			const GS::UniString&	customFieldName		= *cIt->value;
			GS::UniString			customFieldValue;

			if (!firstLoop)
				buffer.Append (", ");

			issue.customData.Get (guid, &customFieldValue);
			buffer.Append (customFieldName + ": " + customFieldValue);

			firstLoop = false;
		}

		WriteReport (buffer.ToCStr ().Get ());
	}
}


// -----------------------------------------------------------------------------
// Get all Document Revision
// -----------------------------------------------------------------------------

static void		Do_GetAllDocumentRevisions (void)
{
	GS::Array<API_RVMDocumentRevision> revisions;
	GSErrCode err = ACAPI_Database (APIDb_GetRVMDocumentRevisionsID, &revisions);
	if (err != NoError) {
		WriteReport ("Do_GetAllDocumentRevisions: error occured!");
		return;
	}

	if (revisions.IsEmpty ()) {
		WriteReport ("There are no document revisions!");
		return;
	}

	WriteReport ("# Document revisions:");

	char buffer[256];
	for (GS::Array<API_RVMDocumentRevision>::ConstIterator it = revisions.Enumerate(); it != NULL; ++it) {
		const API_RVMDocumentRevision& revision = *it;
		sprintf (buffer, "ID: %s, Layout name: %s", revision.id.ToCStr ().Get (), revision.layoutInfo.name.ToCStr ().Get ());
		WriteReport (buffer);
	}
}


// -----------------------------------------------------------------------------
// Get the Document Revisions of the given Issue
// -----------------------------------------------------------------------------

static void		Do_GetIssueDocumentRevisions (void)
{
	GS::Array<API_RVMIssue> issues;
	GSErrCode err = ACAPI_Database (APIDb_GetRVMIssuesID, &issues);
	if (err != NoError) {
		WriteReport ("Do_GetIssueDocumentRevisions: error occured!");
		return;
	}

	if (issues.IsEmpty ()) {
		WriteReport ("There are no issues!");
		return;
	}

	GS::Array<API_RVMDocumentRevision> revisions;
	err = ACAPI_Database (APIDb_GetRVMIssueDocumentRevisionsID, &(issues[0].guid), &revisions);
	if (err != NoError) {
		WriteReport ("Do_GetIssueDocumentRevisions: error occured!");
		return;
	}

	char buffer[256];

	if (revisions.IsEmpty ()) {
		sprintf (buffer, "There are no document revisions in {%s} issue!", APIGuid2GSGuid (issues[0].guid).ToUniString ().ToCStr ().Get ());
		WriteReport (buffer);
		return;
	}

	sprintf (buffer, "# Document revisions in {%s} issue:", APIGuid2GSGuid (issues[0].guid).ToUniString ().ToCStr ().Get ());
	WriteReport (buffer);

	for (GS::Array<API_RVMDocumentRevision>::ConstIterator it = revisions.Enumerate(); it != NULL; ++it) {
		const API_RVMDocumentRevision& revision = *it;
		sprintf (buffer, "ID: %s, Layout name: %s", revision.finalId.ToCStr ().Get (), revision.layoutInfo.name.ToCStr ().Get ());
		WriteReport (buffer);
	}
}


// -----------------------------------------------------------------------------
// Get the Changes of the given DocumentRevision
// -----------------------------------------------------------------------------

static void		Do_GetDocumentRevisionChanges (void)
{
	GS::Array<API_RVMDocumentRevision> revisions;
	GSErrCode err = ACAPI_Database (APIDb_GetRVMDocumentRevisionsID, &revisions);
	if (err != NoError) {
		WriteReport ("Do_GetDocumentRevisionChanges: error occured!");
		return;
	}

	if (revisions.IsEmpty ()) {
		WriteReport ("There are no document revisions!");
		return;
	}

	GS::Array<API_RVMChange> changes;
	err = ACAPI_Database (APIDb_GetRVMDocumentRevisionChangesID, &(revisions[0].guid), &changes);
	if (err != NoError) {
		WriteReport ("Do_GetDocumentRevisionChanges: error occured!");
		return;
	}

	char buffer[256];

	if (changes.IsEmpty ()) {
		sprintf (buffer, "There are no changes in {%s} document revision!", APIGuid2GSGuid (revisions[0].guid).ToUniString ().ToCStr ().Get ());
		WriteReport (buffer);
		return;
	}

	sprintf (buffer, "# Changes in {%s} document revision:", APIGuid2GSGuid (revisions[0].guid).ToUniString ().ToCStr ().Get ());
	WriteReport (buffer);

	for (GS::Array<API_RVMChange>::ConstIterator it = changes.Enumerate(); it != NULL; ++it) {
		const API_RVMChange& change = *it;
		sprintf (buffer, "ID: %s, Description: %s", change.id.ToCStr ().Get (), change.description.ToCStr ().Get ());
		WriteReport (buffer);
	}
}


// -----------------------------------------------------------------------------
// Get the Changes from the current Document Revision of the given Layout
// -----------------------------------------------------------------------------

static void		Do_GetLayoutCurrentRevisionChanges (void)
{
	API_DatabaseInfo dbInfo;
	BNZeroMemory (&dbInfo, sizeof (API_DatabaseInfo));
	GSErrCode err = ACAPI_Database (APIDb_GetCurrentDatabaseID, &dbInfo, NULL);
	if (err != NoError) {
		WriteReport ("Do_GetLayoutCurrentRevisionChanges: error occured!");
		return;
	}

	if (dbInfo.typeID != APIWind_LayoutID) {
		WriteReport ("Current database not a layout database!");
		return;
	}

	GS::Array<API_RVMChange> changes;
	err = ACAPI_Database (APIDb_GetRVMLayoutCurrentRevisionChangesID, &(dbInfo.databaseUnId), &changes);
	if (err != NoError) {
		WriteReport ("Do_GetLayoutCurrentRevisionChanges: error occured!");
		return;
	}

	char buffer[256];

	if (changes.IsEmpty ()) {
		sprintf (buffer, "There are no changes in current revision of {%s} layout!", APIGuid2GSGuid (dbInfo.databaseUnId.elemSetId).ToUniString ().ToCStr ().Get ());
		WriteReport (buffer);
		return;
	}

	sprintf (buffer, "# Changes in current revision of {%s} layout:", APIGuid2GSGuid (dbInfo.databaseUnId.elemSetId).ToUniString ().ToCStr ().Get ());
	WriteReport (buffer);

	for (GS::Array<API_RVMChange>::ConstIterator it = changes.Enumerate(); it != NULL; ++it) {
		const API_RVMChange& change = *it;
		sprintf (buffer, "ID: %s, Description: %s", change.id.ToCStr ().Get (), change.description.ToCStr ().Get ());
		WriteReport (buffer);
	}
}


// -----------------------------------------------------------------------------
// InitPreferences
//		initialize preference data
// -----------------------------------------------------------------------------
static void	InitPreferences (void)
{
	myPrefs = new PrefsData;
	if (myPrefs != NULL) {
		Int32	version;
		GSSize	nBytes;

		ACAPI_GetPreferences (&version, &nBytes, NULL);
		if (version == 1 && nBytes == sizeof (PrefsData)) {
			ACAPI_GetPreferences (&version, &nBytes, myPrefs);
			if (myPrefs->platform != GS::Act_Platform_Sign) {
				IVULong ((GS::PlatformSign) (myPrefs->platform), &myPrefs->lastDetail);
				IVULong ((GS::PlatformSign) (myPrefs->platform), &myPrefs->lastWorksheet);
				IVULong ((GS::PlatformSign) (myPrefs->platform), &myPrefs->lastLayout);
				IVULong ((GS::PlatformSign) (myPrefs->platform), &myPrefs->last3DDocument);
				myPrefs->platform = GS::Act_Platform_Sign;
			}
		} else {
			myPrefs->platform = GS::Act_Platform_Sign;
			myPrefs->lastDetail = myPrefs->lastWorksheet = myPrefs->lastLayout = myPrefs->last3DDocument = 0;
		}
	}
}		// InitPreferences


// -----------------------------------------------------------------------------
// Menu command handler
// -----------------------------------------------------------------------------

static GSErrCode __ACENV_CALL	MenuCommandProc (const API_MenuParams* menuParams)
{
	DBPrintf ("Database Control Add-On User Menu called, ResID: %d, Item No.: %d\n",
				menuParams->menuItemRef.menuResID, menuParams->menuItemRef.itemIndex);

	switch (menuParams->menuItemRef.itemIndex) {

		case ID_MENU_ListSections:		Do_ListSectionLines ();													break;
		// -------

		case ID_MENU_OpenText:			Do_OpenMyWindow (true);													break;
		case ID_MENU_CloseText:			Do_CloseMyWindow (-1, true);											break;
		case ID_MENU_OpenDraw:			Do_OpenMyWindow (false);												break;
		case ID_MENU_CloseDraw:			Do_CloseMyWindow (-1, false);											break;
		case ID_MENU_CloseAll:			Do_CloseAllWindows ();													break;
		// -------
		case ID_MENU_CreateNewDetail:	Do_CreateDetail ();														break;
		case ID_MENU_RenameDetail:		Do_RenameDatabase (APIDb_GetDetailDatabasesID);							break;
		case ID_MENU_DeleteDetail:		Do_DeleteDatabase (APIDb_GetDetailDatabasesID);							break;
		case ID_MENU_ListDetails:		Do_ListDatabases (APIDb_GetDetailDatabasesID, "detail");				break;
		// -------
		case ID_MENU_CreateNewWorksheet:	Do_CreateWorksheet ();												break;
		case ID_MENU_RenameWorksheet:		Do_RenameDatabase (APIDb_GetWorksheetDatabasesID);					break;
		case ID_MENU_DeleteWorksheet:		Do_DeleteDatabase (APIDb_GetWorksheetDatabasesID);					break;
		case ID_MENU_ListWorksheets:		Do_ListDatabases (APIDb_GetWorksheetDatabasesID, "worksheet");		break;
		// -------
		case ID_MENU_CreateNew3DDocument:	Do_Create3DDocument ();												break;
		case ID_MENU_Rename3DDocument:		Do_RenameDatabase (APIDb_GetDocumentFrom3DDatabasesID);				break;
		case ID_MENU_Delete3DDocument:		Do_DeleteDatabase (APIDb_GetDocumentFrom3DDatabasesID);				break;
		case ID_MENU_List3DDocuments:		Do_ListDatabases (APIDb_GetDocumentFrom3DDatabasesID, "3DDocument");break;
		case ID_MENU_Change3DDocument:		Do_Change3DDocument ();												break;
		// -------
		case ID_MENU_CreateNewLayout:	Do_CreateLayout ();														break;
		case ID_MENU_RenameLayout:		Do_RenameDatabase (APIDb_GetLayoutDatabasesID);							break;
		case ID_MENU_DeleteLayout:		Do_DeleteDatabase (APIDb_GetLayoutDatabasesID);							break;
		case ID_MENU_ListLayouts:		Do_ListDatabases (APIDb_GetLayoutDatabasesID, "layout");
										Do_ListDatabases (APIDb_GetMasterLayoutDatabasesID, "master layout");	break;
		case ID_MENU_ListDrawingLinks:	Do_ListDrawingLinks ();													break;
		case ID_MENU_ChangeAllDrawings:	Do_ChangeAllDrawings ();												break;
		case ID_MENU_ChangeMaster:		Do_ChangeMaster ();														break;
		// -------
		case ID_MENU_ListSections_New:			Do_ListDatabases (APIDb_GetSectionDatabasesID, "section");		break;
		case ID_MENU_ListElevations:			Do_ListDatabases (APIDb_GetElevationDatabasesID, "elevation");	break;
		case ID_MENU_ListInteriorElevations:	Do_ListDatabases (APIDb_GetInteriorElevationDatabasesID,
																  "interior elevation");						break;
		case ID_MENU_CreateSubset:				Do_CreateSubset ();												break;
		// -------
		case ID_MENU_GetAllChanges:						Do_GetAllChanges ();									break;
		case ID_MENU_GetChangeCustomScheme:				Do_GetChangeCustomScheme ();							break;
		case ID_MENU_GetAllIssues:						Do_GetAllIssues ();										break;
		case ID_MENU_GetIssueCustomScheme:				Do_GetIssueCustomScheme ();								break;
		case ID_MENU_GetAllDocumentRevisions:			Do_GetAllDocumentRevisions ();							break;
		case ID_MENU_GetIssueDocumentRevisions:			Do_GetIssueDocumentRevisions ();						break;
		case ID_MENU_GetDocumentRevisionChanges:		Do_GetDocumentRevisionChanges ();						break;
		case ID_MENU_GetLayoutCurrentRevisionChanges:	Do_GetLayoutCurrentRevisionChanges ();					break;
	}

	return NoError;
}


// =============================================================================
//
// Required functions
//
// =============================================================================
#ifdef __APPLE__
#pragma mark -
#endif

//------------------------------------------------------------------------------
// Called when the Add-On is going to be registered
//------------------------------------------------------------------------------
API_AddonType	__ACENV_CALL	CheckEnvironment (API_EnvirParams* envirParams)
{
	if (envirParams->serverInfo.serverApplication != APIAppl_ArchiCADID)
		return APIAddon_DontRegister;

	// Fill in the necessary information
	ACAPI_Resource_GetLocStr (envirParams->addOnInfo.name, IDR_AddOnDescStrings, IDS_AddOnName);
	ACAPI_Resource_GetLocStr (envirParams->addOnInfo.description, IDR_AddOnDescStrings, IDS_AddOnDesc);

	return APIAddon_Normal;
}


//------------------------------------------------------------------------------
// Interface definitions
//------------------------------------------------------------------------------
GSErrCode __ACENV_CALL	RegisterInterface (void)
{
	GSErrCode	err;

	// Register menus
	err = ACAPI_Register_Menu (IDR_OwnMenu, IDR_OwnMenuStrs, MenuCode_UserDef, MenuFlag_SeparatorBefore); //or MenuFlag_Default

	return err;
}


// -----------------------------------------------------------------------------
// Initialize
//		called after the Add-On has been loaded into memory
// -----------------------------------------------------------------------------
GSErrCode __ACENV_CALL	Initialize (void)
{
	GSErrCode	err;

	// pass the handler function pointer to ArchiCAD
	err = ACAPI_Install_MenuHandler (IDR_OwnMenu, MenuCommandProc);
    if (err != NoError)
        DBPrintf ("Initialize():: ACAPI_Install_MenuHandler failed\n");


	InitPreferences ();

	return err;
}


//------------------------------------------------------------------------------
// Called when the Add-On is going to be unloaded
//------------------------------------------------------------------------------
GSErrCode __ACENV_CALL	FreeData (void)
{

	if (myPrefs != NULL) {
		ACAPI_SetPreferences (1L, sizeof (PrefsData), (const void *) myPrefs);
		delete myPrefs;
	}

	return NoError;
}

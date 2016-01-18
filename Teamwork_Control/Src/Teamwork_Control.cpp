// *****************************************************************************
// Source code for the Teamwork Control Add-On
// API Development Kit 19; Mac/Win
//
// Namespaces:			Contact person:
//		-None-
//
// [SG compatible] - Yes
// *****************************************************************************

#include "APIEnvir.h"
#define	_TEAMWORK_CONTROL_TRANSL_

// ---------------------------------- Includes ---------------------------------

// from GSRoot
#include "UniString.hpp"

// from ACAPI
#include "ACAPinc.h"					// also includes APIdefs.h
#include "APICommon.h"


// ---------------------------------- Types ------------------------------------

#define CitiesGuid				(APIGuidFromString ("5D8068E2-7430-4871-9D67-E06001F256A1"))
#define CompositesGuid			(APIGuidFromString ("A36401CC-77B1-4410-BCEB-A7684706A17F"))
#define FavoritesGuid			(APIGuidFromString ("F97013BD-662E-42a1-A749-9C73CD2D0790"))
#define FillTypesGuid			(APIGuidFromString ("4C036A66-C2FF-4c3b-9FF4-F45A810B5F84"))
#define LayerSettingsGuid		(ACAPI_TeamworkControl_FindLockableObjectSet ("LayerSettingsDialog"))
#define LineTypesGuid			(APIGuidFromString ("702A8569-EA8F-4de8-900C-696980FB13D6"))
#define MarkupStylesGuid		(APIGuidFromString ("156BCF98-CFA6-4be0-BC2C-8252D640A9FB"))
#define MaterialsGuid			(APIGuidFromString ("5B6A4F99-C72D-4811-90A5-6D696E1AB51F"))
#define MEPSystemsGuid			(APIGuidFromString ("258B2630-3098-48ea-8923-F712214FBDAE"))
#define ModelViewOptionsGuid	(APIGuidFromString ("13E263C8-692B-494b-84E3-2B4BD0A77332"))
#define OperationProfilesGuid	(APIGuidFromString ("0E6DC7E2-5AFC-4309-AB31-2A790CF57A53"))
#define PenTablesGuid			(APIGuidFromString ("08B4B9BB-3DD6-4ea1-A084-80D80B8B7742"))
#define ProfilesGuid			(APIGuidFromString ("4779D92D-ACFB-429d-91E5-1D585B9D2CE5"))
#define ProjectInfoGuid			(APIGuidFromString ("D13F8A89-2AEC-4c32-B04E-85A5393F9C47"))
#define ProjectPreferencesGuid	(ACAPI_TeamworkControl_FindLockableObjectSet ("PreferencesDialog"))
#define ZoneCategoriesGuid		(APIGuidFromString ("B83F2FD1-0AD4-4c41-A8EB-6D7558B0A120"))
#define BuildingMaterialsGuid	(APIGuidFromString ("50477294-5E20-4349-920B-EFC18BF54A0C"))


// ---------------------------------- Variables --------------------------------


// ---------------------------------- Prototypes -------------------------------


// =============================================================================
//
// Testing Teamwork Reservation functions of Elements and Lockable Object Sets
//
// =============================================================================

static void	ReserveOrRelease_Attributes (const API_Guid& objectSetGuid, const GS::UniString& objectSetName, bool showAlert = true)
{
	bool hasCreateRight = ACAPI_TeamworkControl_HasCreateRight (objectSetGuid);
	bool hasDeleteModifyRight = ACAPI_TeamworkControl_HasDeleteModifyRight (objectSetGuid);
	if (!hasCreateRight && !hasDeleteModifyRight) {
		GS::UniString reportString ("You have insufficient privileges to modify ");
		reportString.Append (objectSetName);
		ACAPI_WriteReport (reportString.ToCStr ().Get (), true);
		return;
	}

	GS::UniString reportString (objectSetName);
	GS::PagedArray<GS::UniString> conflicts;
	API_LockableStatus lockableStatus = ACAPI_TeamworkControl_GetLockableStatus (objectSetGuid, &conflicts);

	if (lockableStatus == APILockableStatus_Free) {					// Free for Reservation - let's reserve it
		GSErrCode errCode = ACAPI_TeamworkControl_ReserveLockable (objectSetGuid, &conflicts);
		if (errCode == NoError)
			reportString.Append (" - Reservation succeeded");
		else
			reportString.Append (" - Reservation failed");

		lockableStatus = ACAPI_TeamworkControl_GetLockableStatus (objectSetGuid, &conflicts);

	} else if (lockableStatus == APILockableStatus_Editable) {		// Editable - let's release it
		GSErrCode errCode = ACAPI_TeamworkControl_ReleaseLockable (objectSetGuid);
		if (errCode == NoError)
			reportString.Append (" - Release succeeded");
		else
			reportString.Append (" - Release failed");

		lockableStatus = ACAPI_TeamworkControl_GetLockableStatus (objectSetGuid, &conflicts);
	}

	reportString.Append ("\n  Teamwork status: ");
	switch (lockableStatus) {
		case APILockableStatus_Free:			reportString.Append ("Free for Reservation");
												break;

		case APILockableStatus_Editable:		reportString.Append ("Editable");
												break;

		case APILockableStatus_Locked:			if (conflicts.IsEmpty ()) {
													reportString.Append ("Locked by other member");
												} else {
													reportString.Append ("Locked by ");
													reportString.Append (conflicts[0]);
												}
												break;

		case APILockableStatus_NotAvailable:	reportString.Append ("Not available");
												break;

		case APILockableStatus_NotExist:		reportString.Append ("Not exist");
												break;
	}

	ACAPI_WriteReport (reportString.ToCStr ().Get (), showAlert);
}		// ReserveOrRelease_Attributes


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------

static void	Reserve_SelectedElements (void)
{
	API_SelectionInfo 	selectionInfo;
	API_Neig			**selNeigs = NULL;

	BNZeroMemory (&selectionInfo, sizeof (selectionInfo));
	GSErrCode err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, false);
	if (err == NoError) {
		GS::PagedArray<API_Guid>		objectIdArray;
		GS::HashTable<API_Guid, short>	conflicts;

		for (Int32 i = 0; i < selectionInfo.sel_nElem; i++)
			 objectIdArray.Push ((*selNeigs)[i].guid);

		if (!objectIdArray.IsEmpty ()) {
			ACAPI_TeamworkControl_ReserveElements (objectIdArray, &conflicts);
			if (!conflicts.IsEmpty ()) {
				char reportString[256];
				sprintf (reportString, "Teamwork Control failed to reserve %lu element(s)", (GS::ULongForStdio)conflicts.GetSize ());
				ACAPI_WriteReport (reportString, true);
			}
		}
	}

	BMKillHandle ((GSHandle *) &selNeigs);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
}		// Reserve_SelectedElements


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------

static void	Release_SelectedElements (void)
{
	API_SelectionInfo 	selectionInfo;
	API_Neig			**selNeigs = NULL;

	BNZeroMemory (&selectionInfo, sizeof (selectionInfo));
	GSErrCode err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, false);
	if (err == NoError) {
		GS::PagedArray<API_Guid> objectIdArray;

		for (Int32 i = 0; i < selectionInfo.sel_nElem; i++)
			 objectIdArray.Push ((*selNeigs)[i].guid);

		if (!objectIdArray.IsEmpty ())
			ACAPI_TeamworkControl_ReleaseElements (objectIdArray);
	}

	BMKillHandle ((GSHandle *) &selNeigs);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
}		// Release_SelectedElements


// =============================================================================
//
// Testing Reservation notifications of selected Elements and Lockable Object Sets
//
// =============================================================================

static GSErrCode __ACENV_CALL	ElementReservationChangeHandler (const GS::HashTable<API_Guid, short>&	/*reserved*/,
																 const GS::HashSet<API_Guid>&			released,
																 const GS::HashSet<API_Guid>&			/*deleted*/)
{
	if (released.IsEmpty ())
		return NoError;

	GS::UniString reportString = GS::UniString::Printf ("=   Teamwork Control Example add-on attempts to Reserve %d elements", released.GetSize ());
	ACAPI_WriteReport (reportString.ToCStr ().Get (), false);

	GS::PagedArray<API_Guid> objectIdArray;
	for (GS::HashSet<API_Guid>::ConstIterator it = released.Enumerate (); it != NULL; ++it)
		objectIdArray.Push (*it);

	return ACAPI_TeamworkControl_ReserveElements (objectIdArray);
}		// ElementReservationChangeHandler


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------

static void	Watch_SelectedElementsReservation (short menuResID, short itemIndex)
{
	if (!GetMenuItemMark (menuResID, itemIndex)) {							// install element reservation change handler
		API_SelectionInfo 	selectionInfo;
		API_Neig			**selNeigs = NULL;

		BNZeroMemory (&selectionInfo, sizeof (selectionInfo));
		GSErrCode err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, false);
		if (err == NoError) {
			GS::HashSet<API_Guid> filterElementsInterestedOnly;
			for (Int32 i = 0; i < selectionInfo.sel_nElem; i++)
				 filterElementsInterestedOnly.Add ((*selNeigs)[i].guid);	// notifications will be sent for these elements only

			if (!filterElementsInterestedOnly.IsEmpty ()) {
				ACAPI_Notify_CatchElementReservationChange (ElementReservationChangeHandler, &filterElementsInterestedOnly);
				InvertMenuItemMark (menuResID, itemIndex);
			}
		}

		BMKillHandle ((GSHandle *) &selNeigs);
		BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);

	} else {
		ACAPI_Notify_CatchElementReservationChange (NULL);					// stop watching element reservation changes
		InvertMenuItemMark (menuResID, itemIndex);
	}
}		// Watch_SelectedElementsReservation


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------

static GSErrCode __ACENV_CALL	LockableReservationChangeHandler (const API_Guid& objectId, short ownerId)
{
	if (ownerId != 0)		// interested only in release actions
		return NoError;

	GS::UniString objectSetName;
	if (objectId == LayerSettingsGuid)
		objectSetName = "Layer Settings";
	else if (objectId == LineTypesGuid)
		objectSetName = "Line Types";
	else if (objectId == FillTypesGuid)
		objectSetName = "Fill Types";
	else if (objectId == CompositesGuid)
		objectSetName = "Composites";
	else if (objectId == MaterialsGuid)
		objectSetName = "Surfaces";
	else if (objectId == BuildingMaterialsGuid)
		objectSetName = "Building Materials";
	else
		objectSetName = "an unexpected object";

	GS::UniString reportString = GS::UniString::Printf ("=   Teamwork Control Example add-on attempts to Reserve %T", objectSetName.ToPrintf ());
	ACAPI_WriteReport (reportString.ToCStr ().Get (), false);

	ReserveOrRelease_Attributes (objectId, objectSetName, false);

	return NoError;
}		// LockableReservationChangeHandler


// -----------------------------------------------------------------------------
// Install lockable object reservation change handler
// to reserve them when they get released by others
// -----------------------------------------------------------------------------
static void	Watch_AttributesReservation (short menuResID, short itemIndex)
{
	if (!GetMenuItemMark (menuResID, itemIndex)) {
		GS::UniString reportString;
		GS::HashSet<API_Guid> filterLockablesInterestedOnly;
		API_Guid objectSetGuid = LayerSettingsGuid;
		if (ACAPI_TeamworkControl_GetLockableStatus (objectSetGuid) == APILockableStatus_Locked) {
			filterLockablesInterestedOnly.Add (objectSetGuid);				// watch Layer Settings
			reportString.Append ("Layer Settings");
		}

		objectSetGuid = LineTypesGuid;
		if (ACAPI_TeamworkControl_GetLockableStatus (objectSetGuid) == APILockableStatus_Locked) {
			filterLockablesInterestedOnly.Add (objectSetGuid);				// watch Line Types
			if (!reportString.IsEmpty ())
				reportString.Append (", ");
			reportString.Append ("Line Types");
		}

		objectSetGuid = FillTypesGuid;
		if (ACAPI_TeamworkControl_GetLockableStatus (objectSetGuid) == APILockableStatus_Locked) {
			filterLockablesInterestedOnly.Add (objectSetGuid);				// watch Fill Types
			if (!reportString.IsEmpty ())
				reportString.Append (", ");
			reportString.Append ("Fill Types");
		}

		objectSetGuid = CompositesGuid;
		if (ACAPI_TeamworkControl_GetLockableStatus (objectSetGuid) == APILockableStatus_Locked) {
			filterLockablesInterestedOnly.Add (objectSetGuid);				// watch Composites
			if (!reportString.IsEmpty ())
				reportString.Append (", ");
			reportString.Append ("Composites");
		}

		objectSetGuid = MaterialsGuid;
		if (ACAPI_TeamworkControl_GetLockableStatus (objectSetGuid) == APILockableStatus_Locked) {
			filterLockablesInterestedOnly.Add (objectSetGuid);				// watch Materials
			if (!reportString.IsEmpty ())
				reportString.Append (", ");
			reportString.Append ("Surfaces");
		}

		objectSetGuid = BuildingMaterialsGuid;
		if (ACAPI_TeamworkControl_GetLockableStatus (objectSetGuid) == APILockableStatus_Locked) {
			filterLockablesInterestedOnly.Add (objectSetGuid);				// watch Building Materials
			if (!reportString.IsEmpty ())
				reportString.Append (", ");
			reportString.Append ("Building Materials");
		}

		if (!filterLockablesInterestedOnly.IsEmpty ()) {
			reportString.Append (" will be reserved automatically when they get released by the current locker user(s).");
			ACAPI_WriteReport (reportString.ToCStr ().Get (), true);
			ACAPI_Notify_CatchLockableReservationChange (LockableReservationChangeHandler, &filterLockablesInterestedOnly);
			InvertMenuItemMark (menuResID, itemIndex);
		}

	} else {
		ACAPI_Notify_CatchLockableReservationChange (NULL);					// stop watching attribute reservation changes
		InvertMenuItemMark (menuResID, itemIndex);
	}
}		// Watch_AttributesReservation


// -----------------------------------------------------------------------------
// Callback function on menu command event
// -----------------------------------------------------------------------------
GSErrCode	__ACENV_CALL	MenuCommandHandler (const API_MenuParams *params)
{
	if (!ACAPI_TeamworkControl_HasConnection ()) {
		ACAPI_WriteReport ("This function is available only in Teamwork mode", true);
		return NoError;
	}

	if (!ACAPI_TeamworkControl_IsOnline ()) {
		ACAPI_WriteReport ("Sorry, the BIM Server is currently not available", true);
		return NoError;
	}

	switch (params->menuItemRef.menuResID) {
		case 32500:									// "Teamwork Control"
			switch (params->menuItemRef.itemIndex) {
				case 1:		ReserveOrRelease_Attributes (LayerSettingsGuid,		"Layer Settings");		break;
				case 2:		ReserveOrRelease_Attributes (LineTypesGuid,			"Line Types");			break;
				case 3:		ReserveOrRelease_Attributes (FillTypesGuid,			"Fill Types");			break;
				case 4:		ReserveOrRelease_Attributes (CompositesGuid,		"Composites");			break;
				case 5:		ReserveOrRelease_Attributes (PenTablesGuid,			"Pen Sets");			break;
				case 6:		ReserveOrRelease_Attributes (MaterialsGuid,			"Surfaces");			break;
				case 7:		ReserveOrRelease_Attributes (BuildingMaterialsGuid,	"Building Materials");	break;
				case 8:		ReserveOrRelease_Attributes (ZoneCategoriesGuid,	"Zone Categories");		break;
				case 9:		ReserveOrRelease_Attributes (MarkupStylesGuid,		"Mark-Up Styles");		break;
				case 10:	ReserveOrRelease_Attributes (ProfilesGuid,			"Profiles");			break;
				case 11:	ReserveOrRelease_Attributes (FavoritesGuid,			"Favorites");			break;
				/* ----- */
				case 13:	Reserve_SelectedElements ();	break;
				case 14:	Release_SelectedElements ();	break;
				/* ----- */
				case 16:	Watch_SelectedElementsReservation (params->menuItemRef.menuResID, (short) params->menuItemRef.itemIndex);		break;
				case 17:	Watch_AttributesReservation (params->menuItemRef.menuResID, (short) params->menuItemRef.itemIndex);				break;
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


//------------------------------------------------------
// Dependency definitions
//------------------------------------------------------
API_AddonType	__ACENV_CALL	CheckEnvironment (API_EnvirParams* envir)
{
	if (envir->serverInfo.serverApplication != APIAppl_ArchiCADID)
		return APIAddon_DontRegister;

	ACAPI_Resource_GetLocStr (envir->addOnInfo.name, 32000, 1);
	ACAPI_Resource_GetLocStr (envir->addOnInfo.description, 32000, 2);

	return APIAddon_Normal;
}		// CheckEnvironment


//------------------------------------------------------
// Interface definitions
//------------------------------------------------------
GSErrCode	__ACENV_CALL	RegisterInterface (void)
{
	return ACAPI_Register_Menu (32500, 32600, MenuCode_UserDef, MenuFlag_Default);
}		// RegisterInterface


// -----------------------------------------------------------------------------
// Initialize
//		called after the Add-On has been loaded into memory
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	Initialize (void)
{
	return ACAPI_Install_MenuHandler (32500, MenuCommandHandler);
}		// Initialize


// -----------------------------------------------------------------------------
// FreeData
//		called when the Add-On is going to be unloaded
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	FreeData (void)
{
	return NoError;
}		// FreeData

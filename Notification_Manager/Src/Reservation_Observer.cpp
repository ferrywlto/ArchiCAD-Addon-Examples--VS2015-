// *****************************************************************************
// Description:		Source code for the Notification Manager Add-On
//
// Module:			API Development Kit 19; Mac/Win
// Namespace:		-none-
// Contact person:	devsup@graphisoft.hu
//
// SG compatible
// *****************************************************************************
#include "APIEnvir.h"
#define _RESERVATION_OBSERVER_TRANSL_

// ------------------------------ Includes -------------------------------------

#include	<stdio.h>			/* sprintf */

#include	"ACAPinc.h"
#include	"UniString.hpp"
#include	"Location.hpp"

#include	"Notification_Manager.h"


// ------------------------------ Constants ------------------------------------

// -------------------------------- Types --------------------------------------

// ------------------------------ Variables ------------------------------------

// ------------------------------ Prototypes -----------------------------------

// -----------------------------------------------------------------------------
// GetTeamworkMembers
//
//  collect information of joined Teamwork members
// -----------------------------------------------------------------------------
static GSErrCode	GetTeamworkMembers (GS::HashTable<short, API_UserInfo>& userInfoTable, short& myUserId)
{
	API_ProjectInfo	projectInfo;
	BNZeroMemory (&projectInfo, sizeof (API_ProjectInfo));
	GSErrCode err = ACAPI_Environment (APIEnv_ProjectID, &projectInfo);
	if (projectInfo.location != NULL)
		delete projectInfo.location;
	if (projectInfo.location_team != NULL)
		delete projectInfo.location_team;
	if (err != NoError)
		return err;

	myUserId = projectInfo.userId;

	API_SharingInfo	sharingInfo;
	BNZeroMemory (&sharingInfo, sizeof (API_SharingInfo));
	err = ACAPI_Environment (APIEnv_ProjectSharingID, &sharingInfo);
	if (err == NoError && sharingInfo.users != NULL) {
		for (Int32 i = 0; i < sharingInfo.nUsers; i++)
			userInfoTable.Add (((*sharingInfo.users)[i]).userId, (*sharingInfo.users)[i]);
	}
	if (sharingInfo.users != NULL)
		BMhKill (reinterpret_cast<GSHandle*>(&sharingInfo.users));

	return err;
}		/* GetTeamworkMembers */


// -----------------------------------------------------------------------------
// PrintElementInfo
//
//  print reservation information of a given element
// -----------------------------------------------------------------------------
static void		PrintElementInfo (const GS::HashTable<short, API_UserInfo>&	userInfoTable,
								  short										myUserId,
								  const char*								actionStr,
								  const API_Guid&							guid,
								  short										elementOwnerId = 0)
{
	API_Elem_Head elemHead;
	BNZeroMemory (&elemHead, sizeof (API_Elem_Head));
	elemHead.guid = guid;

	if (ACAPI_Element_GetHeader (&elemHead) == NoError) {
		GS::UniString elemTypeName;
		ACAPI_Goodies (APIAny_GetElemTypeNameID, (void*) elemHead.typeID, &elemTypeName);

		GS::UniString actionByUserStr (actionStr);
		if (userInfoTable.ContainsKey (elementOwnerId)) {
			if (elementOwnerId == myUserId) {
				actionByUserStr.Append (" by me (");
				actionByUserStr.Append (userInfoTable[elementOwnerId].fullName);
				actionByUserStr.Append (")");
			} else {
				actionByUserStr.Append (" by ");
				actionByUserStr.Append (userInfoTable[elementOwnerId].fullName);
			}
		}

		const GS::UniString reportString = GS::UniString::Printf ("=  %T {%T} is %T", elemTypeName.ToPrintf (), APIGuidToString (guid).ToPrintf (), actionByUserStr.ToPrintf ());
		ACAPI_WriteReport (reportString.ToCStr ().Get (), false);
	}
}		/* PrintElementInfo */


// -----------------------------------------------------------------------------
// ElementReservationChangeHandler
//
//  lists the recent element reservation changes in Teamwork
// -----------------------------------------------------------------------------
static GSErrCode __ACENV_CALL	ElementReservationChangeHandler (const GS::HashTable<API_Guid, short>&	reserved,
																 const GS::HashSet<API_Guid>&			released,
																 const GS::HashSet<API_Guid>&			deleted)
{
	GS::HashTable<short, API_UserInfo> userInfoTable;
	short myUserId = 0;

	GSErrCode err = GetTeamworkMembers (userInfoTable, myUserId);
	if (err != NoError)
		return err;

	ACAPI_WriteReport ("=== Attention: Workspace reservation has been changed ===============", false);

	for (GS::HashTable<API_Guid, short>::ConstPairIterator it = reserved.EnumeratePairs (); it != NULL; ++it) {
		PrintElementInfo (userInfoTable, myUserId, "reserved", *(it->key), *(it->value));
	}

	for (GS::HashSet<API_Guid>::ConstIterator it = released.Enumerate (); it != NULL; ++it) {
		PrintElementInfo (userInfoTable, myUserId, "released", *it);
	}

	for (GS::HashSet<API_Guid>::ConstIterator it = deleted.Enumerate (); it != NULL; ++it) {
		PrintElementInfo (userInfoTable, myUserId, "deleted", *it);
	}

    return NoError;
}		/* ElementReservationChangeHandler */


// -----------------------------------------------------------------------------
// LockableReservationChangeHandler
//
//  lists the recent lockable object reservation changes in Teamwork
// -----------------------------------------------------------------------------
static GSErrCode __ACENV_CALL	LockableReservationChangeHandler (const API_Guid& objectId, short ownerId)
{
	GS::HashTable<short, API_UserInfo> userInfoTable;
	short myUserId = 0;

	GSErrCode err = GetTeamworkMembers (userInfoTable, myUserId);
	if (err != NoError)
		return err;

	GS::HashTable<API_Guid, GS::UniString> lockableObjectSets;
	lockableObjectSets.Add (APIGuidFromString ("5D8068E2-7430-4871-9D67-E06001F256A1"),				"Cities");
	lockableObjectSets.Add (APIGuidFromString ("A36401CC-77B1-4410-BCEB-A7684706A17F"),				"Composites");
	lockableObjectSets.Add (APIGuidFromString ("F97013BD-662E-42a1-A749-9C73CD2D0790"),				"Favorites");
	lockableObjectSets.Add (APIGuidFromString ("4C036A66-C2FF-4c3b-9FF4-F45A810B5F84"),				"Fill Types");
	lockableObjectSets.Add (ACAPI_TeamworkControl_FindLockableObjectSet ("LayerSettingsDialog"),	"Layer Settings");
	lockableObjectSets.Add (APIGuidFromString ("702A8569-EA8F-4de8-900C-696980FB13D6"),				"Line Types");
	lockableObjectSets.Add (APIGuidFromString ("156BCF98-CFA6-4be0-BC2C-8252D640A9FB"),				"Markup Styles");
	lockableObjectSets.Add (APIGuidFromString ("5B6A4F99-C72D-4811-90A5-6D696E1AB51F"),				"Surfaces");
	lockableObjectSets.Add (APIGuidFromString ("258B2630-3098-48ea-8923-F712214FBDAE"),				"MEP Systems");
	lockableObjectSets.Add (APIGuidFromString ("13E263C8-692B-494b-84E3-2B4BD0A77332"),				"Model View Options");
	lockableObjectSets.Add (APIGuidFromString ("0E6DC7E2-5AFC-4309-AB31-2A790CF57A53"),				"Operation Profiles");
	lockableObjectSets.Add (APIGuidFromString ("08B4B9BB-3DD6-4ea1-A084-80D80B8B7742"),				"Pen Tables");
	lockableObjectSets.Add (APIGuidFromString ("4779D92D-ACFB-429d-91E5-1D585B9D2CE5"),				"Profiles");
	lockableObjectSets.Add (APIGuidFromString ("D13F8A89-2AEC-4c32-B04E-85A5393F9C47"),				"Project Info");
	lockableObjectSets.Add (ACAPI_TeamworkControl_FindLockableObjectSet ("PreferencesDialog"),		"Project Preferences");
	lockableObjectSets.Add (APIGuidFromString ("B83F2FD1-0AD4-4c41-A8EB-6D7558B0A120"),				"Zone Categories");
	lockableObjectSets.Add (APIGuidFromString ("50477294-5E20-4349-920B-EFC18BF54A0C"),				"Building Materials");

	GS::UniString lockableObjectSetName;
	if (!lockableObjectSets.Get (objectId, &lockableObjectSetName))
		lockableObjectSetName = GS::UniString::Printf ("Unknown object {%T}", APIGuid2GSGuid (objectId).ToUniString ().ToPrintf ());

	ACAPI_WriteReport ("=== Attention: Workspace reservation has been changed ===============", false);

	GS::UniString actionByUserStr (ownerId > 0 ? "reserved" : "released");
	if (userInfoTable.ContainsKey (ownerId)) {
		if (ownerId == myUserId) {
			actionByUserStr.Append (" by me (");
			actionByUserStr.Append (userInfoTable[ownerId].fullName);
			actionByUserStr.Append (")");
		} else {
			actionByUserStr.Append (" by ");
			actionByUserStr.Append (userInfoTable[ownerId].fullName);
		}
	}

	GS::UniString reportString = GS::UniString::Printf ("=  %T got %T", lockableObjectSetName.ToPrintf (), actionByUserStr.ToPrintf ());
	ACAPI_WriteReport (reportString.ToCStr ().Get (), false);

    return NoError;
}		/* LockableReservationChangeHandler */


// ============================================================================
// Install Notification Handlers
//
//
// ============================================================================
void	Do_ReservationMonitor (bool switchOn)
{
	if (switchOn) {
		ACAPI_Notify_CatchElementReservationChange (ElementReservationChangeHandler);
		ACAPI_Notify_CatchLockableReservationChange (LockableReservationChangeHandler);
	} else {
		ACAPI_Notify_CatchElementReservationChange (NULL);
		ACAPI_Notify_CatchLockableReservationChange (NULL);
	}

	return;
}		/* Do_ReservationMonitor */

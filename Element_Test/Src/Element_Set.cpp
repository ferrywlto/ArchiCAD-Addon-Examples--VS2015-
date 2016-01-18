// *****************************************************************************
// Source code for the Element Test Add-On
// API Development Kit 19; Mac/Win
//
// Element Set manager functions
//
// Namespaces:        Contact person:
//     -None-
//
// [SG compatible] - Yes
// *****************************************************************************

#define _ELEMENT_OBSERVER_TRANSL_

#if !defined (ACExtension)
#define	ACExtension
#endif

#if defined (_MSC_VER)
	#if !defined (WINDOWS)
		#define WINDOWS
	#endif
#endif

#if defined (WINDOWS)
	#include "Win32Interface.hpp"
	#pragma warning (disable: 4068)
#endif

// ------------------------------ Includes -------------------------------------

#include	<stdio.h>			/* sprintf */
#include	"GSRoot.hpp"
#include	"ACAPinc.h"
#include	"Element_Test.h"
#include	"UniString.hpp"


// ------------------------------ Constants ------------------------------------

// -------------------------------- Types --------------------------------------

// ------------------------------ Variables ------------------------------------

// ------------------------------ Prototypes -----------------------------------


void Do_ElemSet_Create (void)
{
	GS::Array<API_Guid>  guids;

	// get the guids of a Wall, Column, Beam and Window
	GS::Array<API_Guid>  wallList;
	ACAPI_Element_GetElemList (API_WallID, &wallList);
	if (!wallList.IsEmpty ())
		guids.Push (wallList[0]);
	else
		guids.Push (APINULLGuid);

	GS::Array<API_Guid>  columnList;
	ACAPI_Element_GetElemList (API_ColumnID, &columnList);
	if (!columnList.IsEmpty ())
		guids.Push (columnList[0]);
	else
		guids.Push (APINULLGuid);

	GS::Array<API_Guid>  beamList;
	ACAPI_Element_GetElemList (API_BeamID, &beamList);
	if (!beamList.IsEmpty ())
		guids.Push (beamList[0]);
	else
		guids.Push (APINULLGuid);

	GS::Array<API_Guid>  windowList;
	ACAPI_Element_GetElemList (API_WindowID, &windowList);
	if (!windowList.IsEmpty ())
		guids.Push (windowList[0]);
	else
		guids.Push (APINULLGuid);

	// write a note into the user data
	API_UserData userData;
	BNZeroMemory (&userData, sizeof (userData));
	userData.dataVersion  = 4;
	userData.platformSign = GS::Act_Platform_Sign;
	userData.dataHdl      = BMAllocateHandle (1024, ALLOCATE_CLEAR, 0);
	if (userData.dataHdl != NULL)
		sprintf (*userData.dataHdl, "Original element count: 4, guids [%s,%s,%s,%s]", APIGuidToString (guids[0]).ToCStr ().Get (), APIGuidToString (guids[1]).ToCStr ().Get (), APIGuidToString (guids[2]).ToCStr ().Get (), APIGuidToString (guids[3]).ToCStr ().Get ());

	API_Guid setGuid;
	GSErrCode err = ACAPI_ElementSet_Create (&guids, &userData, &setGuid);
	if (err != NoError) {
		DBPrintf ("Create Element Set error: %d\n", err);
	} else {
		DBPrintf ("Created Element Set (%s)\n", APIGuidToString (setGuid).ToCStr ().Get ());
	}

	BMKillHandle (&userData.dataHdl);
}		/* ElemSet_Create */

void Do_ElemSet_Delete (void)
{
	// get the guid of a Wall
	GS::Array<API_Guid>  wallList;
	if (ACAPI_Element_GetElemList (API_WallID, &wallList) != NoError)
		return;

	if (wallList.IsEmpty ())
		return;

	GS::Array<API_Guid> setGuids;
	GSErrCode	err = ACAPI_ElementSet_Identify (wallList[0], &setGuids);
	USize		nSet = (err == NoError) ? setGuids.GetSize () : 0;

	for (USize i = 0; i < nSet; i++) {
		err = ACAPI_ElementSet_Delete (setGuids[i]);
		if (err != NoError) {
			DBPrintf ("Delete Element Set error: %d\n", err);
		} else {
			DBPrintf ("Deleted Element Set %lu (%s)\n", i, APIGuidToString (setGuids[i]).ToCStr ().Get ());
		}
	}
}		/* ElemSet_Delete */

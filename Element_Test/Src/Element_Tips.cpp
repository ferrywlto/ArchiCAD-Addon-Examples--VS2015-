// *****************************************************************************
// Source code for the Element Test Add-On
// API Development Kit 19; Mac/Win
//
//	Tips & Tricks
//
// Namespaces:        Contact person:
//     -None-
//
// [SG compatible] - Yes
// *****************************************************************************

#include "APIEnvir.h"
#define	_ELEMENT_TIPS_TRANSL_


// ---------------------------------- Includes ---------------------------------

#include	<stdio.h>
#include	<string.h>
#include	<time.h>

#include	"GSRoot.hpp"
#include	"ACAPinc.h"					// also includes APIdefs.h

#include	"APICommon.h"
#include	"Element_Test.h"


// ---------------------------------- Types ------------------------------------


// ---------------------------------- Variables --------------------------------


// ---------------------------------- Prototypes -------------------------------



// -----------------------------------------------------------------------------
// Disable visual feedback to speed up operations
//   - operations may results in disturbing screen update
//   - do it in one step if applicable
//
// Delete the openings of the clicked wall
// mode:
//	 - 1 (bad way): 		non-acceptable flash in each delete operation
//	 - 2 (solution #1): 	do the operation with one API call
//	 - 3 (solution #2): 	do it in separate calls, but care about screen updates
// -----------------------------------------------------------------------------

void		Do_AvoidFlashing (short mode)
{
	API_Element		element;
	API_Elem_Head	wallHead, **elemHeads, **oneHead;
	API_Guid		guid;
	Int32			i, n;
	GSErrCode		err;

	if (!ClickAnElem ("Click a wall with many windows in it", API_WallID, NULL, NULL, &guid)) {
		WriteReport_Alert ("No wall was clicked");
		return;
	}

	BNZeroMemory (&element, sizeof (API_Element));
	element.header.typeID = API_WallID;
	element.header.guid   = guid;
	err = ACAPI_Element_Get (&element);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Get (wall)", err);
		return;
	}

	n = 0;
	elemHeads = NULL;
	wallHead = element.header;

	// collect the openings
	GS::Array<API_Guid>	wallWindows;
	GS::Array<API_Guid>	wallDoors;
	err = ACAPI_Element_GetConnectedElements (wallHead.guid, API_WindowID, &wallWindows);
	if (err == NoError)
		err = ACAPI_Element_GetConnectedElements (wallHead.guid, API_DoorID, &wallDoors);

	if (err == NoError) {
		GSSize nWinds = wallWindows.GetSize ();
		GSSize nDoors = wallDoors.GetSize ();
		if (nWinds + nDoors > 0) {
			elemHeads = (API_Elem_Head **) BMAllocateHandle ((nWinds + nDoors) * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
			if (elemHeads != NULL) {
				for (i = 0; i < nWinds; i++)
					(*elemHeads)[n++].guid = wallWindows[i];
				for (i = 0; i < nDoors; i++)
					(*elemHeads)[n++].guid = wallDoors[i];
			}
		}
	}

	if (err != NoError || elemHeads == NULL)
		return;

	// do the operation
	if (mode == 3) {		// best way: do the operation in one API call
		err = ACAPI_Element_Delete (elemHeads, n);
		if (err != NoError)
			ErrorBeep ("ACAPI_Element_Delete", err);

	} else {				// the operation is split into separate API calls
		oneHead = (API_Elem_Head **) BMAllocateHandle (sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
		err = BMError ();
		if (err != NoError) {
			BMKillHandle ((GSHandle *) &elemHeads);
			return;
		}

		for (i = 0; i < n; i++)	 {
			**oneHead = (*elemHeads)[i];
			err = ACAPI_Element_Delete (oneHead, 1);
			if (err == NoError)
				WriteReport ("  %s GUID:%s  deleted", ElemID_To_Name ((**oneHead).typeID), APIGuidToString ((**oneHead).guid).ToCStr ().Get ());
			else
				WriteReport ("  %s GUID:%s  ERROR: %d", ElemID_To_Name ((**oneHead).typeID), APIGuidToString ((**oneHead).guid).ToCStr ().Get (), err);
		}

		if (mode == 2) {
			ACAPI_Database (APIDb_DrawElementID, &wallHead, NULL);		// put the edited wall onto the screen again
		}

		BMKillHandle ((GSHandle *) &oneHead);
	}

	BMKillHandle ((GSHandle *) &elemHeads);

	return;
}		// Do_AvoidFlashing

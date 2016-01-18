// *****************************************************************************
// Source code for the Element Test Add-On
// API Development Kit 19; Mac/Win
//
// Namespaces:        Contact person:
//     -None-
//
// [SG compatible] - Yes
// *****************************************************************************

#include "APIEnvir.h"
#define	_ELEMENT_EDIT_TRANSL_


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



// =============================================================================
//
// Utility functions
//
// =============================================================================

// -----------------------------------------------------------------------------
// Prepare a report for a group of elements being edited
// -----------------------------------------------------------------------------

static API_Neig**	EditReport_Prepare (API_Neig **items)
{
	Int32		length;
	API_Neig	**items_save;
	GSErrCode	err;

	length = BMGetHandleSize ((GSHandle) items);

	items_save = (API_Neig **) BMAllocateHandle (length, ALLOCATE_CLEAR, 0);
	err = BMError ();
	if (err == NoError)
		BNCopyMemory (*items_save, *items, length);

	return items_save;
}		// EditReport_Prepare


// -----------------------------------------------------------------------------
// Write a report for a group of elements being edited
// -----------------------------------------------------------------------------

static void		EditReport (Int32 nItem, API_Neig** items, API_Neig** items_save)
{
	for (Int32 i = 0; i < nItem; i++) {
		const API_ElemTypeID typeID = Neig_To_ElemID ((*items)[i].neigID);

		API_Elem_Head elemHeadSave;
		memset (&elemHeadSave, 0, sizeof (API_Elem_Head));
		elemHeadSave.guid = (*items_save)[i].guid;

		ACAPI_Element_GetHeader (&elemHeadSave);
		if ((*items)[i].guid == APINULLGuid) {
			WriteReport ("[%d]  %s GUID:%s - refused", i+1, ElemID_To_Name (typeID), APIGuidToString (elemHeadSave.guid).ToCStr ().Get ());
		} else {
			API_Elem_Head elemHead;
			memset (&elemHead, 0, sizeof (API_Elem_Head));
			elemHead.guid = (*items)[i].guid;
			ACAPI_Element_GetHeader (&elemHead);
			WriteReport ("[%d]  %s GUID:%s -> GUID:%s", i+1, ElemID_To_Name (typeID), APIGuidToString (elemHeadSave.guid).ToCStr ().Get (), APIGuidToString (elemHead.guid).ToCStr ().Get ());
		}
	}
}		// EditReport


// =============================================================================
//
// Utility functions
//
// =============================================================================
#ifdef __APPLE__
#pragma mark -
#endif

// -----------------------------------------------------------------------------
// Drag some elements
//   - non editable elements should not be touched
//   - other group members should be added automatically
//   - dimensions, labels must be updated automatically
//   - selection dots must be updated automatically
// Check points:
//   - Teamwork workspaces
//   - locked layers
//   - invidual locking
// -----------------------------------------------------------------------------

void		Do_DragElems (bool withInput)
{
	API_EditPars	editPars;
	API_Neig		**items, **items_save;
	API_Coord3D		begC, endC;
	Int32			nItem;
	GSErrCode		err = NoError;

	if (withInput) {
		items = ClickElements_Neig ("Click elements to drag", API_ZombieElemID, &nItem);
		if (items == NULL)
			return;
		items_save = EditReport_Prepare (items);
	} else {
		nItem = 0;
		items = NULL;
		items_save = NULL;
	}

	if (GetEditVector (&begC, &endC, "Enter drag reference point", false)) {
		BNZeroMemory (&editPars, sizeof (API_EditPars));
		editPars.typeID = APIEdit_Drag;
		editPars.withDelete = false;
		editPars.begC = begC;
		editPars.endC = endC;
		editPars.endC.z += 2.0;

		err = ACAPI_Element_Edit (items, nItem, &editPars);
		if (err != NoError)
			ErrorBeep ("ACAPI_Element_Edit (drag)", err);

		if (err == NoError && withInput)
			EditReport (nItem, items, items_save);
	}

	BMKillHandle ((GSHandle *) &items);
	BMKillHandle ((GSHandle *) &items_save);

	return;
}		// Do_DragElems


// -----------------------------------------------------------------------------
// Stretch some elements
//   - non editable elements should not be touched
//   - other group members should be added automatically
//   - dimensions, labels must be updated automatically
//   - selection dots must be updated automatically
// Check points:
//   - Teamwork workspaces
//   - locked layers
//   - invidual locking
// -----------------------------------------------------------------------------

void		Do_StretchElems (bool withInput)
{
	API_EditPars	editPars;
	API_Neig		**items, **items_save;
	API_Coord3D		begC, endC;
	Int32			nItem;
	GSErrCode		err = NoError;

	if (withInput) {
		items = ClickElements_Neig ("Click elements to stretch", API_ZombieElemID, &nItem);
		if (items == NULL)
			return;
		items_save = EditReport_Prepare (items);
	} else {
		nItem = 0;
		items = NULL;
		items_save = NULL;
	}

	if (GetEditVector (&begC, &endC, "Enter stretch reference point", false)) {
		BNZeroMemory (&editPars, sizeof (API_EditPars));
		editPars.typeID = APIEdit_Stretch;
		editPars.begC = begC;
		editPars.endC = endC;

		err = ACAPI_Element_Edit (items, nItem, &editPars);
		if (err != NoError)
			ErrorBeep ("ACAPI_Element_Edit (stretch)", err);

		if (err == NoError && withInput)
			EditReport (nItem, items, items_save);
	}

	BMKillHandle ((GSHandle *) &items);
	BMKillHandle ((GSHandle *) &items_save);

	return;
}		// Do_StretchElems


// -----------------------------------------------------------------------------
// Resize some elements
//   - non editable elements should not be touched
//   - other group members should be added automatically
//   - dimensions, labels must be updated automatically
//   - selection dots must be updated automatically
// Check points:
//   - Teamwork workspaces
//   - locked layers
//   - invidual locking
// -----------------------------------------------------------------------------

void		Do_ResizeElems (bool withInput)
{
	API_EditPars	editPars;
	API_Neig		**items, **items_save;
	API_Coord3D		begC, endC;
	Int32			nItem;
	GSErrCode		err = NoError;

	if (withInput) {
		items = ClickElements_Neig ("Click elements to resize", API_ZombieElemID, &nItem);
		if (items == NULL)
			return;
		items_save = EditReport_Prepare (items);
	} else {
		nItem = 0;
		items = NULL;
		items_save = NULL;
	}

	if (GetEditVector (&begC, &endC, "Enter resize reference point", false)) {
		BNZeroMemory (&editPars, sizeof (API_EditPars));
		editPars.typeID = APIEdit_Resize;
		editPars.begC = begC;
		editPars.endC = endC;

		if (GetEditVector (&begC, &endC, "Enter resize reference point2", false)) {
			editPars.typeID = APIEdit_Resize;
			editPars.withDelete = true;
			editPars.endC2.x = editPars.begC.x - begC.x + endC.x;
			editPars.endC2.y = editPars.begC.y - begC.y + endC.y;

			err = ACAPI_Element_Edit (items, nItem, &editPars);
			if (err != NoError)
				ErrorBeep ("ACAPI_Element_Edit (resize)", err);

		if (err == NoError && withInput)
			EditReport (nItem, items, items_save);
		}
	}

	BMKillHandle ((GSHandle *) &items);
	BMKillHandle ((GSHandle *) &items_save);

	return;
}		// Do_ResizeElems


// -----------------------------------------------------------------------------
// Rotate some elements
//   - non editable elements should not be touched
//   - other group members should be added automatically
//   - dimensions, labels must be updated automatically
//   - selection dots must be updated automatically
// Check points:
//   - Teamwork workspaces
//   - locked layers
//   - invidual locking
// -----------------------------------------------------------------------------

void		Do_RotateElems (bool withInput)
{
	API_EditPars	editPars;
	API_Neig		**items, **items_save;
	API_Coord3D		begC, endC, origC;
	Int32			nItem;
	GSErrCode		err = NoError;

	if (withInput) {
		items = ClickElements_Neig ("Click elements to rotate", API_ZombieElemID, &nItem);
		if (items == NULL)
			return;
		items_save = EditReport_Prepare (items);
	} else {
		nItem = 0;
		items = NULL;
		items_save = NULL;
	}

	if (GetEditArc (&begC, &endC, &origC,  "Enter rotation reference point")) {
		BNZeroMemory (&editPars, sizeof (API_EditPars));
		editPars.typeID = APIEdit_Rotate;
		editPars.begC = begC;
		editPars.endC = endC;
		editPars.origC.x = origC.x;
		editPars.origC.y = origC.y;
		editPars.withDelete = true;
		err = ACAPI_Element_Edit (items, nItem, &editPars);
		if (err != NoError)
			ErrorBeep ("ACAPI_Element_Edit (rotate)", err);

		if (err == NoError && withInput)
			EditReport (nItem, items, items_save);
	}

	BMKillHandle ((GSHandle *) &items_save);
	BMKillHandle ((GSHandle *) &items);

	return;
}		// Do_RotateElems



// -----------------------------------------------------------------------------
// Mirror some elements
//   - non editable elements should not be touched
//   - other group members should be added automatically
//   - dimensions, labels must be updated automatically
//   - selection dots must be updated automatically
// Check points:
//   - Teamwork workspaces
//   - locked layers
//   - invidual locking
// -----------------------------------------------------------------------------

void		Do_MirrorElems (bool withInput)
{
	API_EditPars	editPars;
	API_Neig		**items, **items_save;
	API_Coord3D		begC, endC;
	Int32			nItem;
	GSErrCode		err = NoError;

	if (withInput) {
		items = ClickElements_Neig ("Click elements to mirror", API_ZombieElemID, &nItem);
		if (items == NULL)
			return;
		items_save = EditReport_Prepare (items);
	} else {
		nItem = 0;
		items = NULL;
		items_save = NULL;
	}

	if (GetEditVector (&begC, &endC, "Enter mirror reference point", false)) {
		BNZeroMemory (&editPars, sizeof (API_EditPars));
		editPars.typeID = APIEdit_Mirror;
		editPars.begC = begC;
		editPars.endC = endC;
		editPars.withDelete = true;
		err = ACAPI_Element_Edit (items, nItem, &editPars);
		if (err != NoError)
			ErrorBeep ("ACAPI_Element_Edit (mirror)", err);

		if (err == NoError && withInput)
			EditReport (nItem, items, items_save);
	}

	BMKillHandle ((GSHandle *) &items);
	BMKillHandle ((GSHandle *) &items_save);

	return;
}		// Do_MirrorElems


// -----------------------------------------------------------------------------
// Elevate some elements
//   - non editable elements should not be touched
//   - other group members should be added automatically
//   - dimensions, labels must be updated automatically
//   - selection dots must be updated automatically
// Check points:
//   - Teamwork workspaces
//   - locked layers
//   - invidual locking
// -----------------------------------------------------------------------------

void		Do_ElevElems (bool withInput)
{
	API_EditPars	editPars;
	API_Neig		**items, **items_save;
	GSErrCode		err;
	Int32			nItem;

	if (withInput) {
		items = ClickElements_Neig ("Click elements to elevate (by fixed 2m)", API_ZombieElemID, &nItem);
		if (items == NULL)
			return;
		items_save = EditReport_Prepare (items);
	} else {
		nItem = 0;
		items = NULL;
		items_save = NULL;
	}

	BNZeroMemory (&editPars, sizeof (API_EditPars));
	editPars.typeID = APIEdit_Elevate;
	editPars.withDelete = true;
	editPars.endC.z = 2.0;							/* begC.z = 0.0 */
	err = ACAPI_Element_Edit (items, nItem, &editPars);
	if (err != NoError)
		ErrorBeep ("ACAPI_Element_Edit (elevate)", err);

	if (err == NoError && withInput)
		EditReport (nItem, items, items_save);

	BMKillHandle ((GSHandle *) &items);
	BMKillHandle ((GSHandle *) &items_save);

	return;
}		// Do_ElevElems

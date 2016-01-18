// *****************************************************************************
// Source code for the Element Test Add-On
// API Development Kit 19; Mac/Win
//
//	Tool operations on elements
//
// Namespaces:        Contact person:
//     -None-
//
// [SG compatible] - Yes
// *****************************************************************************

#include "APIEnvir.h"
#define	_ELEMENT_TOOLS_TRANSL_


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
// Apply tools on elements
//
// =============================================================================

// -----------------------------------------------------------------------------
// Group the clicked or selected elements
//   - non editable elements should not be touched
//   - other group members should be added automatically
//   - selection dots must be updated automatically
// Check points
//   - Teamwork workspaces
//   - locked layers
//   - invidual locking
// -----------------------------------------------------------------------------

void		Do_GroupElems (bool	withInput)
{
	API_Elem_Head	**elemHeads;
	Int32			nItem;
	GSErrCode		err;

	if (withInput) {
		elemHeads = ClickElements_ElemHead ("Click elements to group", API_ZombieElemID, &nItem);
	} else {
		nItem = 0;
		elemHeads = NULL;
	}

	err = ACAPI_Element_Tool (elemHeads, nItem, APITool_Group, NULL);
	if (err != NoError)
		ErrorBeep ("ACAPI_Element_Tool (group)", err);

	if (elemHeads != NULL)
		BMKillHandle ((GSHandle *) &elemHeads);

	return;
}		// Do_GroupElems


// -----------------------------------------------------------------------------
// Ungroup the clicked or selected elements
//   - non editable elements should not be touched
//   - other group members should be added automatically
//   - selection dots must be updated automatically
// Check points
//   - Teamwork workspaces
//   - locked layers
//   - invidual locking
// -----------------------------------------------------------------------------

void		Do_UngroupElems (bool	withInput)
{
	API_Elem_Head	**elemHeads;
	Int32			nItem;
	GSErrCode		err;

	if (withInput) {
		elemHeads = ClickElements_ElemHead ("Click groups to ungroup", API_ZombieElemID, &nItem);
	} else {
		nItem = 0;
		elemHeads = NULL;
	}

	err = ACAPI_Element_Tool (elemHeads, nItem, APITool_Ungroup, NULL);
	if (err != NoError)
		ErrorBeep ("ACAPI_Element_Tool (ungroup)", err);

	if (elemHeads != NULL)
		BMKillHandle ((GSHandle *) &elemHeads);

	return;
}		// Do_UngroupElems


// -----------------------------------------------------------------------------
// Lock the clicked or selected elements
//   - non editable elements should not be touched
//   - other group members should be added automatically
//   - selection dots must be updated automatically
// Check points
//   - Teamwork workspaces
//   - locked layers
//   - invidual locking
// -----------------------------------------------------------------------------

void		Do_LockElems (bool	withInput)
{
	API_Elem_Head	**elemHeads;
	Int32			nItem;
	GSErrCode		err;

	if (withInput) {
		elemHeads = ClickElements_ElemHead ("Click elements to lock", API_ZombieElemID, &nItem);
	} else {
		nItem = 0;
		elemHeads = NULL;
	}

	err = ACAPI_Element_Tool (elemHeads, nItem, APITool_Lock, NULL);
	if (err != NoError)
		ErrorBeep ("ACAPI_Element_Tool (lock)", err);

	if (elemHeads != NULL)
		BMKillHandle ((GSHandle *) &elemHeads);

	return;
}		// Do_LockElems


// -----------------------------------------------------------------------------
// Unlock the clicked or selected elements
//   - non editable elements should not be touched
//   - other group members should be added automatically
//   - selection dots must be updated automatically
// Check points
//   - Teamwork workspaces
//   - locked layers
//   - invidual locking
// -----------------------------------------------------------------------------

void		Do_UnlockElems (bool	withInput)
{
	API_Elem_Head	**elemHeads;
	Int32			nItem;
	GSErrCode		err;

	if (withInput) {
		elemHeads = ClickElements_ElemHead ("Click elements to unlock", API_ZombieElemID, &nItem);
	} else {
		nItem = 0;
		elemHeads = NULL;
	}

	err = ACAPI_Element_Tool (elemHeads, nItem, APITool_Unlock, NULL);
	if (err != NoError)
		ErrorBeep ("ACAPI_Element_Tool (unlock)", err);

	if (elemHeads != NULL)
		BMKillHandle ((GSHandle *) &elemHeads);

	return;
}


// -----------------------------------------------------------------------------
// Bring forward the clicked or selected elements
//   - non editable elements should not be touched
//   - other group members should be added automatically
//   - selection dots must be updated automatically
// Check points
//   - Teamwork workspaces
//   - locked layers
//   - invidual locking
// -----------------------------------------------------------------------------

void		Do_ForwardElems (bool withInput)
{
	API_Elem_Head	**elemHeads;
	Int32			nItem;
	GSErrCode		err;

	if (withInput) {
		elemHeads = ClickElements_ElemHead ("Click elements to bring forward", API_ZombieElemID, &nItem);
	} else {
		nItem = 0;
		elemHeads = NULL;
	}

	err = ACAPI_Element_Tool (elemHeads, nItem, APITool_BringForward, NULL);
	if (err != NoError)
		ErrorBeep ("ACAPI_Element_Tool (bring forward)", err);

	if (elemHeads != NULL)
		BMKillHandle ((GSHandle *) &elemHeads);

	return;
}


// -----------------------------------------------------------------------------
// Send backward the clicked or selected elements
//   - non editable elements should not be touched
//   - other group members should be added automatically
//   - selection dots must be updated automatically
// Check points
//   - Teamwork workspaces
//   - locked layers
//   - invidual locking
// -----------------------------------------------------------------------------

void		Do_BackwardElems (bool withInput)
{
	API_Elem_Head	**elemHeads;
	Int32			nItem;
	GSErrCode		err;

	if (withInput) {
		elemHeads = ClickElements_ElemHead ("Click elements to send backward", API_ZombieElemID, &nItem);
	} else {
		nItem = 0;
		elemHeads = NULL;
	}

	err = ACAPI_Element_Tool (elemHeads, nItem, APITool_SendBackward, NULL);
	if (err != NoError)
		ErrorBeep ("ACAPI_Element_Tool (send backward)", err);

	if (elemHeads != NULL)
		BMKillHandle ((GSHandle *) &elemHeads);

	return;
}		// Do_BackwardElems


// -----------------------------------------------------------------------------
// Toggle the Suspend Groups switch
//   - selection dots must be updated automatically
//   - menu must be updated
// -----------------------------------------------------------------------------

void		Do_SuspendGroups (void)
{
	GSErrCode	err;

	err = ACAPI_Element_Tool (NULL, 0, APITool_SuspendGroups, NULL);
	if (err != NoError)
		ErrorBeep ("ACAPI_Element_Tool (suspend group)", err);

	return;
}		// Do_SuspendGroups

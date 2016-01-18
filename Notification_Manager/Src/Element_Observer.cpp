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
#define _ELEMENT_OBSERVER_TRANSL_

// ------------------------------ Includes -------------------------------------

#include	<stdio.h>			/* sprintf */

#include	"ACAPinc.h"

#include	"Notification_Manager.h"
#include	"APICommon.h"
#include	"UniString.hpp"

// ------------------------------ Constants ------------------------------------

// -------------------------------- Types --------------------------------------

// ------------------------------ Variables ------------------------------------

static	bool	allNewElements = false;

// ------------------------------ Prototypes -----------------------------------


// -----------------------------------------------------------------------------
// Convert API_ActTranPars to API_EditPars
// -----------------------------------------------------------------------------
static bool	ActTranPars_To_EditPars (const API_ActTranPars *actTranPars, API_EditPars *editPars)
{
	BNZeroMemory (editPars, sizeof (API_EditPars));
	editPars->withDelete = true;
	editPars->typeID = (API_EditCmdID) 0;

	switch (actTranPars->typeID) {
		case APIEdit_Drag:
			editPars->typeID = APIEdit_Drag;
			editPars->endC.x = actTranPars->theDisp.x;
			editPars->endC.y = actTranPars->theDisp.y;
			editPars->endC.z = actTranPars->theDispZ;
			break;
		case APIEdit_Rotate:
			editPars->typeID = APIEdit_Rotate;
			editPars->origC = actTranPars->theOrigo;
			editPars->begC.x = editPars->origC.x + 1.0;
			editPars->begC.y = editPars->origC.y;
			editPars->endC.x = editPars->origC.x + actTranPars->theCosA;
			editPars->endC.y = editPars->origC.y + actTranPars->theSinA;
			break;
		case APIEdit_Mirror:
			editPars->typeID = APIEdit_Mirror;
			editPars->begC.x = actTranPars->theOrigo.x;
			editPars->begC.y = actTranPars->theOrigo.y;
			editPars->endC.x = actTranPars->theOrigo.x + actTranPars->theAxeVect.x;
			editPars->endC.y = actTranPars->theOrigo.y + actTranPars->theAxeVect.y;
			break;
		case APIEdit_Stretch:
			editPars->typeID = APIEdit_Stretch;
			editPars->begC.x = actTranPars->theOrigo.x;
			editPars->begC.y = actTranPars->theOrigo.y;
			editPars->endC.x = actTranPars->theOrigo.x + actTranPars->theDisp.x;
			editPars->endC.y = actTranPars->theOrigo.y + actTranPars->theDisp.y;
			break;
		case APIEdit_PDirStretch:
			editPars->typeID = APIEdit_PDirStretch;
			editPars->begC.x = actTranPars->theOrigo.x;
			editPars->begC.y = actTranPars->theOrigo.y;
			editPars->endC.x = actTranPars->theOrigo.x + actTranPars->theDisp.x;
			editPars->endC.y = actTranPars->theOrigo.y + actTranPars->theDisp.y;
			break;
		case APIEdit_PHeightStretch:
			editPars->typeID = APIEdit_PHeightStretch;
			editPars->begC.x = actTranPars->theOrigo.x;
			editPars->begC.y = actTranPars->theOrigo.y;
			editPars->endC.x = actTranPars->theOrigo.x + actTranPars->theDisp.x;
			editPars->endC.y = actTranPars->theOrigo.y + actTranPars->theDisp.y;
			break;
		case APIEdit_Resize:
			editPars->typeID = APIEdit_Resize;
			editPars->begC.x = actTranPars->theOrigo.x;
			editPars->begC.y = actTranPars->theOrigo.y;
			editPars->endC.x = actTranPars->theOrigo.x + 1;
			editPars->endC.y = actTranPars->theOrigo.y;
			editPars->endC2.x = actTranPars->theOrigo.x + actTranPars->theRatio;
			editPars->endC2.y = actTranPars->theOrigo.y;
			break;
		case APIEdit_Elevate:
			editPars->typeID = APIEdit_Elevate;
			editPars->endC.z = actTranPars->theDispZ;
			break;
		case APIEdit_VertStretch:
			editPars->typeID = APIEdit_VertStretch;
			editPars->endC.z = actTranPars->theDispZ;
			break;
		case APIEdit_General:
			break;
	}

	return editPars->typeID != (API_EditCmdID) 0;
}	// ActTranPars_To_EditPars


// -----------------------------------------------------------------------------
// Do_ElementEdit
//	edit the linked elements
// -----------------------------------------------------------------------------
static GSErrCode Do_ElementEdit (API_Elem_Head *header)
{
	API_Guid		elemGuid = APINULLGuid;
	Int32			nLinks = 0;
	API_Guid**		linkToList = NULL;
	API_ActTranPars actTranPars;
	API_EditPars	editPars;
	GSErrCode		err;


	BNZeroMemory (&actTranPars, sizeof (API_ActTranPars));
	BNZeroMemory (&editPars, sizeof (API_EditPars));
	ACAPI_Notify_GetTranParams (&actTranPars);
	ActTranPars_To_EditPars (&actTranPars, &editPars);

	elemGuid = header->guid;
	err = ACAPI_Element_GetLinks (elemGuid, &linkToList, &nLinks);
	if (linkToList == NULL || nLinks == 0)
		return NoError;

	if (err == NoError) {
		if (err == NoError) {
			API_Neig**	neigHdl = reinterpret_cast<API_Neig**> (BMAllocateHandle (nLinks * sizeof (API_Neig), ALLOCATE_CLEAR, 0));
			if (neigHdl == NULL)
				return APIERR_MEMFULL;

			for (GSIndex index = 0; index < nLinks; index++) {
				API_Elem_Head elemHead;
				BNZeroMemory (&elemHead, sizeof (API_Elem_Head));
				elemHead.guid = (*linkToList)[index];
				ACAPI_Element_GetHeader (&elemHead);
				ElemHead_To_Neig (&(*neigHdl)[index], &elemHead);
			}

			err = ACAPI_Element_Edit (neigHdl, nLinks, &editPars);
			BMKillHandle (reinterpret_cast<GSHandle*> (&neigHdl));
		}
	}
	BMKillHandle (reinterpret_cast<GSHandle*> (&linkToList));

	return err;
}	// Do_ElementEdit


// -----------------------------------------------------------------------------
// ElementEventHandlerProc
//
// -----------------------------------------------------------------------------
GSErrCode __ACENV_CALL	ElementEventHandlerProc (const API_NotifyElementType *elemType)
{
	GSErrCode		err = NoError;
	char			msgStr[256];
	char			elemStr[32];

	if (elemType->notifID == APINotifyElement_BeginEvents || elemType->notifID == APINotifyElement_EndEvents) {
		API_DatabaseInfo api_dbPars;
		BNZeroMemory (&api_dbPars, sizeof (API_DatabaseInfo));
		api_dbPars.databaseUnId = elemType->databaseId;
		ACAPI_Database (APIDb_GetDatabaseInfoID, &api_dbPars, NULL);
		sprintf (msgStr, "### Element_Manager: %s notification on database \"%s\"",
				(elemType->notifID == APINotifyElement_BeginEvents) ? "Begin Events" : "End Events",
				(const char *) GS::UniString (api_dbPars.title).ToCStr ());

	} else if (GetElementTypeString (elemType->elemHead.typeID, elemStr)) {
		char				elemGuidStr[64];
		char				parentElemGuidStr[64];
		API_Element			parentElement;
		API_ElementUserData	parentUserData;

		BNZeroMemory (&parentElement, sizeof (API_Element));
		BNZeroMemory (&parentUserData, sizeof (API_ElementUserData));
		ACAPI_Notify_GetParentElement (&parentElement, NULL, 0, &parentUserData);
		BMKillHandle (&parentUserData.dataHdl);

		CHTruncate (APIGuid2GSGuid (elemType->elemHead.guid).ToUniString ().ToCStr (), elemGuidStr, sizeof (elemGuidStr));
		CHTruncate (APIGuid2GSGuid (parentElement.header.guid).ToUniString ().ToCStr (), parentElemGuidStr, sizeof (parentElemGuidStr));

		switch (elemType->notifID) {
			case APINotifyElement_New:
						if (!allNewElements)
							break;

						if (parentElement.header.guid != APINULLGuid)
							sprintf (msgStr, "### Element_Manager: <%s> created {%s} as a copy of {%s}", elemStr, elemGuidStr, parentElemGuidStr);
						else
							sprintf (msgStr, "### Element_Manager: <%s> created {%s}", elemStr, elemGuidStr);

						err = ACAPI_Element_AttachObserver (const_cast<API_Elem_Head*> (&elemType->elemHead), 0);
						if (err == APIERR_LINKEXIST)
							err = NoError;
						break;

			case APINotifyElement_Copy:
						if (!allNewElements)
							break;

						if (parentElement.header.guid != APINULLGuid) {
							sprintf (msgStr, "### Element_Manager: <%s> copied {%s} from {%s}", elemStr, elemGuidStr, parentElemGuidStr);

							err = ACAPI_Element_AttachObserver (const_cast<API_Elem_Head*> (&elemType->elemHead), 0);
							if (err == APIERR_LINKEXIST)
								err = NoError;
						}
						break;

			case APINotifyElement_Change:
						if (parentElement.header.guid != APINULLGuid)
							sprintf (msgStr, "### Element_Manager: <%s> changed {%s} -> {%s}", elemStr, parentElemGuidStr, elemGuidStr);
						else
							sprintf (msgStr, "### Element_Manager: <%s> window/door changed {%s}", elemStr, elemGuidStr);
						break;

			case APINotifyElement_Edit:
						if (parentElement.header.guid != APINULLGuid)
							sprintf (msgStr, "### Element_Manager: <%s> edited {%s} -> {%s}", elemStr, parentElemGuidStr, elemGuidStr);
						else
							sprintf (msgStr, "### Element_Manager: <%s> edited in place {%s}", elemStr, elemGuidStr);

						err = Do_ElementEdit (&parentElement.header);
						break;

			case APINotifyElement_Delete:
						sprintf (msgStr, "### Element_Manager: <%s> deleted {%s}", elemStr, elemGuidStr);
						break;

			case APINotifyElement_Undo_Created:
						sprintf (msgStr, "### Element_Manager: undone <%s> created {%s} ", elemStr, elemGuidStr);
						break;

			case APINotifyElement_Undo_Modified:
						sprintf (msgStr, "### Element_Manager: undone <%s> modified {%s}", elemStr, elemGuidStr);
						break;

			case APINotifyElement_Undo_Deleted:
						sprintf (msgStr, "### Element_Manager: undone <%s> deleted {%s}", elemStr, elemGuidStr);
						break;

			case APINotifyElement_Redo_Created:
						sprintf (msgStr, "### Element_Manager: redone <%s> created {%s}", elemStr, elemGuidStr);
						break;

			case APINotifyElement_Redo_Modified:
						sprintf (msgStr, "### Element_Manager: redone <%s> modified {%s}", elemStr, elemGuidStr);
						break;

			case APINotifyElement_Redo_Deleted:
						sprintf (msgStr, "### Element_Manager: redone <%s> deleted {%s}", elemStr, elemGuidStr);
						break;

			default:
						break;
		}
	}

	ACAPI_WriteReport (msgStr, false);

	return err;
}	// ElementEventHandlerProc


// ============================================================================
// Do_ElementMonitor
//
//	observe all newly created elements
// ============================================================================
void	Do_ElementMonitor (bool switchOn)
{
	if (switchOn) {
		ACAPI_Notify_CatchNewElement (NULL, ElementEventHandlerProc);			// for all elements
		ACAPI_Notify_InstallElementObserver	(ElementEventHandlerProc);			// observe all newly created elements
		allNewElements = true;
	} else {
		ACAPI_Notify_CatchNewElement (NULL, NULL);
		ACAPI_Notify_InstallElementObserver	(NULL);
		allNewElements = false;
	}

	return;
}	// Do_ElementMonitor


// ============================================================================
// Do_ClickedElementMonitor
//
//	observe clicked element
// ============================================================================
void	Do_ClickedElementMonitor (bool switchOn)
{
	API_Elem_Head elemHead;
	GSErrCode err = NoError;

	BNZeroMemory (&elemHead, sizeof (API_Elem_Head));
	if (!ClickAnElem ("Click an element", API_ZombieElemID, NULL, &elemHead.typeID, &elemHead.guid)) {
		WriteReport_Alert ("No element was clicked");
		return;
	}

	if (switchOn) {
		ACAPI_Notify_InstallElementObserver	(ElementEventHandlerProc);
		err = ACAPI_Element_AttachObserver (&elemHead, 0);
		if (err == APIERR_LINKEXIST)
			err = NoError;

	} else {
		err = ACAPI_Element_DetachObserver (&elemHead);
	}

	return;
}	// Do_ClickedElementMonitor


// ============================================================================
// Do_ListMonitoredElements
//
//	list observed elements
// ============================================================================
void	Do_ListMonitoredElements (void)
{
	API_Elem_Head**	ppHeads;
	Int32			nElems;
	GSErrCode		err = ACAPI_Element_GetObservedElements (&ppHeads, &nElems);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetObservedElements", err);
		return;
	}

	for (Int32 i = 0; i < nElems; i++) {
		const API_Elem_Head& refHead = (*ppHeads)[i];
		WriteReport ("%s guid=%s", ElemID_To_Name (refHead.typeID), APIGuidToString (refHead.guid).ToCStr ().Get ());
	}

	BMKillHandle ((GSHandle *) &ppHeads);
}	// Do_ListMonitoredElements

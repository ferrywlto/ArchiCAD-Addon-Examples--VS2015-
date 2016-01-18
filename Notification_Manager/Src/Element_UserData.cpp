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
#define	_ELEMENT_USERDATA_TRANSL_


// ---------------------------------- Includes ---------------------------------

#include	<stdio.h>
#include	<string.h>
#include	<time.h>

#include	"ACAPinc.h"					// also includes APIdefs.h

#include	"APICommon.h"
#include	"Notification_Manager.h"
#include	"UniString.hpp"


// ---------------------------------- Types ------------------------------------


// ---------------------------------- Variables --------------------------------


// ---------------------------------- Prototypes -------------------------------

// =============================================================================
//
// Assign custom data to elements
//
// =============================================================================

// -----------------------------------------------------------------------------
// Add user data to all selected elements
//   - store element index
//   - data marked to drop automatically on editing
// -----------------------------------------------------------------------------

void		Do_MarkSelElems (void)
{
	API_SelectionInfo 	selectionInfo;
	API_Elem_Head		elemHead;
	API_ElementUserData userData;
	API_Neig**			selNeig;
	GSErrCode			err;

	err = ACAPI_Selection_Get (&selectionInfo, &selNeig, true);
	if (err != NoError && err != APIERR_NOSEL) {
		ErrorBeep ("ACAPI_Selection_GetInfo", err);
		return;
	}

	if (selectionInfo.typeID == API_SelEmpty) {
		WriteReport_Alert ("No selected elements");
		return;
	}

	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);

	BNZeroMemory (&elemHead, sizeof (API_Elem_Head));
	BNZeroMemory (&userData, sizeof (API_ElementUserData));

	userData.dataVersion = 1;
	userData.platformSign = GS::Act_Platform_Sign;
	userData.dataHdl = BMAllocateHandle (0, ALLOCATE_CLEAR, 0);
	if (userData.dataHdl == NULL)
		return;

	char	timeStr[128];
	TIGetTimeString (TIGetTime (), timeStr, TI_SHORT_DATE_FORMAT | TI_SHORT_TIME_FORMAT);

	//SpeedTest
	TIReset (0, "ACAPI_Element_SetUserData - SpeedTest");
	TIStart (0);

	for (Int32 i = 0; i < selectionInfo.sel_nElemEdit; i++) {
		elemHead.typeID = Neig_To_ElemID ((*selNeig)[i].neigID);
		elemHead.guid   = (*selNeig)[i].guid;

		//--------------------------------------------------------
		char dataStr [300];
		sprintf (dataStr, "%s GUID:%s <%s>", ElemID_To_Name (elemHead.typeID), APIGuidToString (elemHead.guid).ToCStr ().Get (), timeStr);
		Int32 dataLen = Strlen32 (dataStr) + 1;
		userData.dataHdl = BMReallocHandle (userData.dataHdl, dataLen, REALLOC_FULLCLEAR, 0);
		if (userData.dataHdl == NULL)
			break;					// error handling...

		CHCopyC (dataStr, *userData.dataHdl);

		userData.flags = APIUserDataFlag_FillWith | APIUserDataFlag_Pickup | APIUserDataFlag_UndoAble;

		err = ACAPI_Element_SetUserData (&elemHead, &userData);
		if (err != NoError)
			ErrorBeep ("ACAPI_Element_SetUserData", err);

	}

	//SpeedTest
	TIStop (0);
	TIPrintTimers ();


	BMKillHandle (&userData.dataHdl);
	BMKillHandle ((GSHandle *) &selNeig);
}		// Do_MarkSelElems


// -----------------------------------------------------------------------------
// List the user data of all selected elements
//   - element index was stored
//   - data marked to drop automatically on editing
// -----------------------------------------------------------------------------

void		Do_ListOwnedElements (void)
{
	//SpeedTest
	TIReset (1, "ACAPI_Element_GetUserData - SpeedTest");
	TIStart (1);

	for (Int32 longtype = (Int32) API_FirstElemType; longtype <= (Int32) API_LastElemType; longtype++) {
		const API_ElemTypeID typeID = (API_ElemTypeID) longtype;
		GS::Array<API_Guid> elemList;
		GSErrCode err = ACAPI_Element_GetElemList (typeID, &elemList);
		if (err != NoError) {
			ErrorBeep ("ACAPI_Element_GetElemList", err);
			continue;
		}

		API_Elem_Head		elemHead;
		API_ElementUserData	userData;
		BNZeroMemory (&userData, sizeof (API_ElementUserData));

		for (GS::Array<API_Guid>::ConstIterator it = elemList.Enumerate (); it != NULL; ++it) {
			BNZeroMemory (&elemHead, sizeof (API_Elem_Head));
			elemHead.guid = *it;
			err = ACAPI_Element_GetUserData (&elemHead, &userData);
			if (err == NoError) {
				char	dataStr[256];
				CHTruncate (*userData.dataHdl, dataStr, BMGetHandleSize (userData.dataHdl));
				WriteReport ("Userdata in %s {%s}: \"%s\" (version: %d, platform: %s)",
							 ElemID_To_Name (typeID),
							 APIGuidToString (*it).ToCStr ().Get (),
							 dataStr,
							 userData.dataVersion,
							 (userData.platformSign == GS::Win_Platform_Sign) ? "Win" : "Mac");
				BMKillHandle (&userData.dataHdl);
			}
		}
	}

	//SpeedTest
	TIStop (1);
	TIPrintTimers ();
}		// Do_ListOwnedElements


// -----------------------------------------------------------------------------
// Attach a URL to an existing element
//	 - url is stored with the user data mechanism
// -----------------------------------------------------------------------------

void		Do_AttachElementURLRef (void)
{
	API_Elem_Head		elemHead;
	API_ElemURLRef		urlRef;
	GSErrCode			err;

	BNZeroMemory (&elemHead, sizeof (API_Elem_Head));
	BNZeroMemory (&urlRef, sizeof (API_ElemURLRef));

	if (!ClickAnElem ("Click an element", API_ZombieElemID, NULL, &elemHead.typeID, &elemHead.guid)) {
		WriteReport_Alert ("No element was clicked");
		return;
	}

	CHTruncate ("http://www.graphisoft.com/ftp/gdl/GDLPubDemo/Watch/object.html", urlRef.urlLink, sizeof (urlRef.urlLink));
	CHTruncate ("Graphisoft - Object Technology - Demo Zone", urlRef.description, sizeof (urlRef.description));

	err = ACAPI_Database (APIDb_SetElementURLRefID, &elemHead, &urlRef);
	if (err != NoError)
		ErrorBeep ("APIDb_SetElementURLRef", err);

	return;
}		// Do_AttachElementURLRef


// -----------------------------------------------------------------------------
// Retrieve the URL assigned to an element
// -----------------------------------------------------------------------------

void		Do_ListElementURLRef (void)
{
	API_Elem_Head		elemHead;
	API_ElemURLRef		urlRef;
	GSErrCode			err;

	BNZeroMemory (&elemHead, sizeof (API_Elem_Head));
	BNZeroMemory (&urlRef, sizeof (API_ElemURLRef));

	if (!ClickAnElem ("Click an element", API_ZombieElemID, NULL, &elemHead.typeID, &elemHead.guid)) {
		WriteReport_Alert ("No element was clicked");
		return;
	}

	err = ACAPI_Database (APIDb_GetElementURLRefID, &elemHead, &urlRef);
	if (err == NoError) {
		WriteReport ("%s GUID:%s", ElemID_To_Name (elemHead.typeID), APIGuidToString (elemHead.guid).ToCStr ().Get ());
		WriteReport ("  URL:  %s", urlRef.urlLink);
		WriteReport ("  desc: %s", urlRef.description);
	} else if (err == APIERR_NOUSERDATA) {
		WriteReport_Alert ("No URL linked to the element");
	} else {
		ErrorBeep ("APIDb_GetElementURLRef", err);
	}
}		// Do_ListElementURLRef


// -----------------------------------------------------------------------------
// Link the selected elements to the clicked element
// Remove links of clicked element if no selection
// -----------------------------------------------------------------------------

void		Do_LinkElements (void)
{
	API_Element			element;
	API_SelectionInfo 	selectionInfo;
	API_Neig**			selNeig;
	GSErrCode			err;

	BNZeroMemory (&element, sizeof (API_Element));
	if (!ClickAnElem ("Click an element", API_ZombieElemID, NULL, &element.header.typeID, &element.header.guid)) {
		WriteReport_Alert ("No element was clicked");
		return;
	}

	err = ACAPI_Element_Get (&element);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Get", err);
		return;
	}

	//-------------------------------------------------
	const API_ElemTypeID typeIDFrom	= element.header.typeID;
	const API_Guid		 linkFrom	= element.header.guid;
	API_Guid**			 linkToList	= NULL;
	GS::UniString		 linkList;

	Int32 nLinks;
	err = ACAPI_Element_GetLinks (linkFrom, &linkToList, &nLinks);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_GetLinks", err);
		return;
	}

	for (GSIndex index = 0; index < nLinks; index++) {
		GSFlags	linkFlags = 0;
		err = ACAPI_Element_GetLinkFlags (linkFrom, (*linkToList)[index], &linkFlags);
		if (err != NoError)
			linkFlags = 0xFFFFFFFF;

		char flagString[13] = { 0 };
		sprintf (flagString, "[0x%08x]", static_cast<unsigned int> (linkFlags));

		if (!linkList.IsEmpty ()) {
			linkList += GS::UniString (", ");
		}
		linkList += APIGuid2GSGuid ((*linkToList)[index]).ToUniString ();
		linkList += GS::UniString (" ");
		linkList += GS::UniString (flagString);

		err = ACAPI_Element_Unlink (linkFrom, (*linkToList)[index]);
		if (err != NoError) {
			ErrorBeep ("ACAPI_Element_Unlink", err);
		}
	}
	BMKillHandle ((GSHandle *) &linkToList);

	if (nLinks > 0)
		WriteReport ("LINK - Elements previously linked to %s {%s}: %s",
					 ElemID_To_Name (typeIDFrom), APIGuidToString (linkFrom).ToCStr ().Get (), linkList.ToCStr ().Get ());
	else
		WriteReport ("LINK - No elements were previously linked to %s {%s}",
					 ElemID_To_Name (typeIDFrom), APIGuidToString (linkFrom).ToCStr ().Get ());

	//-------------------------------------------------
	err = ACAPI_Selection_Get (&selectionInfo, &selNeig, true);
	if (err != NoError && err != APIERR_NOSEL) {
		ErrorBeep ("ACAPI_Selection_GetInfo", err);
		return;
	}

	nLinks = 0;
	linkList.Clear ();

	if (selectionInfo.typeID != API_SelEmpty) {
		for (Int32 i = 0; i < selectionInfo.sel_nElemEdit; i++) {
			err = ACAPI_Element_Link (linkFrom, (*selNeig)[i].guid, 0);
			if (err == NoError) {
				if (!linkList.IsEmpty ()) {
					linkList += GS::UniString (", ");
				}
				linkList += APIGuid2GSGuid ((*selNeig)[i].guid).ToUniString ();

				nLinks++;
			}
		}
	}

	if (nLinks > 0)
		WriteReport ("       Elements currently linked to %s {%s}: %s",
					 ElemID_To_Name (typeIDFrom), APIGuidToString (linkFrom).ToCStr ().Get (), linkList.ToCStr ().Get ());
	else
		WriteReport ("       No elements are currently linked to %s {%s}",
					 ElemID_To_Name (typeIDFrom), APIGuidToString (linkFrom).ToCStr ().Get ());

	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	BMKillHandle ((GSHandle *) &selNeig);
}		// Do_LinkElements

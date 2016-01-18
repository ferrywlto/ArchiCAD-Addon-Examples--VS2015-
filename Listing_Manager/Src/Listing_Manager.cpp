// *****************************************************************************
// Source code for the Listing Manager Add-On
// API Development Kit 19; Mac/Win
//
// Namespaces:			Contact person:
//		-None-
//
// [SG compatible] - Yes
// *****************************************************************************

#include "APIEnvir.h"
#define	_LISTING_MANAGER_TRANSL_

// ---------------------------------- Includes ---------------------------------

#include	<stdio.h>
#include	<string.h>

#include	"DG.h"
#include	"ACAPinc.h"					// also includes APIdefs.h

#include	"DGModule.hpp"

// ---------------------------------- Types ------------------------------------

#define DBSetPopupItem			 4
#define TypeIDPopupItem			 6
#define	SearchListItem			 7
#define KeyEditTextItem			 9
#define CodeEditTextItem		11
#define FilterRadioItem			15
#define AllRadioItem			16

#define	NAME_LEN				(API_DBCodeLen - 1)
#define	NAME_LEN2				(API_DBCodeLen / 2 - 1)

typedef struct {
	API_ListDataID	typeID;
	char			name[NAME_LEN];
	char			code[NAME_LEN2];
	char			keycode[NAME_LEN2];		/* only component and descriptor */
} MyListType;


// ---------------------------------- Variables --------------------------------

static MyListType	**listHdl;
static Int32			nlistHdl;


// ---------------------------------- Prototypes -------------------------------




// =============================================================================
//
// Dialog control functions
//
// =============================================================================

// -----------------------------------------------------------------------------
// Set the elements of the list item
// -----------------------------------------------------------------------------

static void SetDGList (short dialId, Int32 last)
{
	char	str[NAME_LEN * 4];
	Int32	i;

	for (i = 0; i < last; i++) {

		sprintf (str, "%s\t%s\t%s", (*listHdl)[i].keycode,
									(*listHdl)[i].code,
									(*listHdl)[i].name);

		DGListSetItemText (dialId, SearchListItem, (short) (i + 1), str);
	}

	return;
}		// SetDGList


// -----------------------------------------------------------------------------
// Initialize the list item
// -----------------------------------------------------------------------------

static GSErrCode	InitList ()
{
	GSErrCode	err;

	nlistHdl = 16;
	listHdl	= (MyListType **) BMAllocateHandle (nlistHdl * sizeof (MyListType),
												ALLOCATE_CLEAR, 0);
	err = BMError ();

	return err;
}		// InitList


// -----------------------------------------------------------------------------
// Append elements to the list item
// -----------------------------------------------------------------------------

static GSErrCode	AppendList (API_ListData *listdata, Int32 *index)
{
	MyListType	list;
	GSErrCode		err;

	if (listdata == NULL ||  index == NULL || *index < 0)
		return APIERR_BADPARS;

	if (*index >= nlistHdl) {
		nlistHdl += 16;
		listHdl = (MyListType **) BMReallocHandle ((GSHandle) listHdl,
												   nlistHdl * sizeof (MyListType),
												   0, 0);
		err = BMError ();
		if (err != NoError)
			return err;
	}

	BNZeroMemory (&list, sizeof (MyListType));
	list.typeID = listdata->header.typeID;

	switch (listdata->header.typeID) {
		case API_KeyID:
			CHTruncate (listdata->key.name, list.name, NAME_LEN);
			CHTruncate (listdata->key.code, list.code, NAME_LEN2);
			break;

		case API_UnitID:
			CHTruncate (listdata->unit.name, list.name, NAME_LEN);
			CHTruncate (listdata->unit.code, list.code, NAME_LEN2);
			break;

		case API_ComponentID:
			CHTruncate (listdata->component.name, list.name, NAME_LEN);
			CHTruncate (listdata->component.code, list.code, NAME_LEN2);
			CHTruncate (listdata->component.keycode, list.keycode, NAME_LEN2);
			break;

		case API_DescriptorID:
			{
				Int32	i;
				char	c;

				for (i = 0;		; i++) {
					c = (*(listdata->descriptor.name))[i];
					if (c < ' ' || i >= NAME_LEN)
						break;
					list.name[i] = c;
				}
				list.name[i] = 0;
				CHTruncate (listdata->descriptor.code, list.code, NAME_LEN2);
				CHTruncate (listdata->descriptor.keycode, list.keycode, NAME_LEN2);
			}
			break;

		default:
			return APIERR_BADPARS;
	}

	(*listHdl)[*index] = list;
	(*index)++;

	return NoError;
}		// AppendList


// -----------------------------------------------------------------------------
// Search the list for a parameter
// -----------------------------------------------------------------------------

static void		Search_ListData (API_ListData *param, Int32 *count, bool filter)
{
	API_ListData	listdata;
	Int32			i;
	Int32			beginIndex, endIndex;
	GSErrCode		err;

	listdata	= *param;
	*count		= 0;
	err			= NoError;

	/* ACAPI_ListData_Search fails if keyCode is an empty string */

	/* ------- Search it ------- */
	if (filter && (param->header.typeID == API_UnitID || param->header.typeID == API_KeyID ||
		(param->header.typeID == API_ComponentID && strlen (param->component.code) != 0) ||
		(param->header.typeID == API_DescriptorID && strlen (param->descriptor.code) != 0))) {

		err = ACAPI_ListData_Search (&listdata);

		if (err == NoError)
			err = ACAPI_ListData_Get (&listdata);

		if (err == NoError) {
			err = AppendList (&listdata, count);
			if (listdata.header.typeID == API_DescriptorID)
				BMKillHandle (&listdata.descriptor.name);
		}
	} else {

	/* ------ Search list ------ */
		if (filter) {
			err = ACAPI_ListData_Search (&listdata);
			if (err != NoError)
				return;

			beginIndex = listdata.header.index;
		} else
			beginIndex = 1;

		err = ACAPI_ListData_GetNum (param->header.setIndex, param->header.typeID, &endIndex);
		if (err != NoError)
			return;

		BNZeroMemory (&listdata, sizeof (API_ListData));
		listdata.header.typeID	 = param->header.typeID;
		listdata.header.setIndex = param->header.setIndex;

		for (i = beginIndex; i <= endIndex; i++) {

			listdata.header.index		= i;
			err = ACAPI_ListData_Get (&listdata);
			if (err != NoError)
				break;

			if (filter) {
				if (listdata.header.typeID == API_ComponentID &&
					!CHEqualCStrings (listdata.component.keycode, param->component.keycode))
					break;

				if	(listdata.header.typeID == API_DescriptorID &&
					!CHEqualCStrings (listdata.descriptor.keycode, param->descriptor.keycode)) {

					BMKillHandle (&listdata.descriptor.name);
					break;
				}
			}

			if (err == NoError) {
				err = AppendList (&listdata, count);
				if (listdata.header.typeID == API_DescriptorID)
					BMKillHandle (&listdata.descriptor.name);
			}

			if (err != NoError)
				return;
		}
	}

	return;
}		// Search_ListData


// -----------------------------------------------------------------------------
// Set up the list item's tabulator fields
// -----------------------------------------------------------------------------

static void SetTabData (short dialId)
{
	DGListTabData		lTabData[5];
	API_Rect			lBox;
	short				width;

	DGGetItemRect (dialId, SearchListItem, &lBox.left, &lBox.top, &lBox.right, &lBox.bottom);

	width = (short) (lBox.right - lBox.left);
	lTabData[0].begPos		= 0;
	lTabData[0].endPos		= (short) (width / 4);
	lTabData[0].justFlag	= DG_IS_LEFT;
	lTabData[0].truncFlag	= DG_IS_TRUNCEND;
	lTabData[0].hasSeparator = false;
	lTabData[0].disabled	= false;

	lTabData[1].begPos		= lTabData[0].endPos;
	lTabData[1].endPos		= (short) (lTabData[1].begPos + width / 4);
	lTabData[1].justFlag	= DG_IS_LEFT;
	lTabData[1].truncFlag	= DG_IS_TRUNCEND;
	lTabData[1].hasSeparator = false;
	lTabData[1].disabled	= false;

	lTabData[2].begPos		= lTabData[1].endPos;
	lTabData[2].endPos		= (short) (lBox.right - 44);
	lTabData[2].justFlag	= DG_IS_LEFT;
	lTabData[2].truncFlag	= DG_IS_TRUNCEND;
	lTabData[2].hasSeparator = false;
	lTabData[2].disabled	= false;

	DGListSetTabData (dialId, SearchListItem, 3, &lTabData[0]);

	return;
}		// SetTabData


// -----------------------------------------------------------------------------
// Set/adjust the dialog items
// -----------------------------------------------------------------------------

static void ListDataDlg_Set (short 	dialId, DGUserData userData)
{
	API_ListData	listdata;
	char			keyStr[NAME_LEN2];
	char			codeStr[NAME_LEN2];
	Int32			count;
	Int32			i;
	bool			filter;

	if (userData == 0L) {
		return;
	}

	listdata = *((API_ListData *) (userData));

	listdata.header.setIndex = (short) DGPopUpGetItemUserData (dialId, DBSetPopupItem,
				(short) DGGetItemValLong (dialId, DBSetPopupItem));

	GS::UniString temp = DGGetItemText (dialId, KeyEditTextItem);
	CHTruncate (UniStringToConstCString (temp), keyStr, NAME_LEN2);
	temp = DGGetItemText (dialId, CodeEditTextItem);
	CHTruncate (UniStringToConstCString (temp), codeStr, NAME_LEN2);

	switch (DGGetItemValLong (dialId, TypeIDPopupItem)) {

		case 1:		listdata.header.typeID = API_ComponentID;
					CHCopyC (keyStr, listdata.component.keycode);
					CHCopyC (codeStr, listdata.component.code);
					DGEnableItem (dialId, KeyEditTextItem);
					break;

		case 2:		listdata.header.typeID = API_DescriptorID;
					CHCopyC (keyStr, listdata.descriptor.keycode);
					CHCopyC (codeStr, listdata.descriptor.code);
					DGEnableItem (dialId, KeyEditTextItem);
					break;

		case 3:		listdata.header.typeID = API_KeyID;
					CHCopyC (codeStr, listdata.key.code);
					DGDisableItem (dialId, KeyEditTextItem);
					break;

		case 4:		listdata.header.typeID = API_UnitID;
					CHCopyC (codeStr, listdata.unit.code);
					DGDisableItem (dialId, KeyEditTextItem);
					break;
	}

	DGListDeleteItem (dialId, SearchListItem, DG_ALL_ITEMS);

	filter = (DGGetCheckedRadio (dialId, 1) == FilterRadioItem);
	Search_ListData (&listdata, &count, filter);

	if (count < 1 || listHdl == NULL)
		return;

	for (i = 1; i <= count; i++)
		DGListInsertItem (dialId, SearchListItem, DG_LIST_BOTTOM);

	SetDGList (dialId, count);
	DGListSelectItem (dialId, SearchListItem, 1);

	return;
}		// ListDataDlg_Set


// -----------------------------------------------------------------------------
// Main dialog callback function
// -----------------------------------------------------------------------------

static short DGCALLBACK ListDataDlg_CB (short			message,
										short			dialId,
										short			item,
										DGUserData		userData,
										DGMessageData	msgData)
{
UNUSED_PARAMETER (msgData);

	short	result;

	result = 0;
	switch (message) {
		case DG_MSG_INIT:
			{	API_ListDataSetType		listdataSet;
				Int32					maxDBSet;
				Int32					i;
				GSErrCode				err;

				err = NoError;
				DGPopUpDeleteItem (dialId, DBSetPopupItem, DG_ALL_ITEMS);

				maxDBSet = ACAPI_ListData_GetSetNum ();
				for (i = 1; i <= maxDBSet && err == NoError; i++) {

					BNZeroMemory (&listdataSet, sizeof (API_ListDataSetType));
					listdataSet.setIndex = i;
					err = ACAPI_ListData_GetSet (&listdataSet);

					if (err == NoError) {
						DGPopUpInsertItem (dialId, DBSetPopupItem, DG_LIST_BOTTOM);
						DGPopUpSetItemText (dialId, DBSetPopupItem, DG_LIST_BOTTOM, listdataSet.setName);
						DGPopUpSetItemUserData (dialId, DBSetPopupItem, DG_LIST_BOTTOM, i);
					}
				}

				if (err == NoError) {
					DGSetItemText (dialId, KeyEditTextItem, "");
					DGSetItemText (dialId, CodeEditTextItem, "");
					DGSetItemValLong (dialId, AllRadioItem, 1);

					SetTabData (dialId);
					ListDataDlg_Set (dialId, userData);
				}
			}
			break;

		case DG_MSG_CLICK:
			if (item == DG_OK || item == DG_CANCEL || item == DG_CLOSEBOX) {
				result = item;
			}
			break;

		case DG_MSG_CHANGE:
			switch (item) {
				case FilterRadioItem:
				case AllRadioItem:
				case DBSetPopupItem:
				case TypeIDPopupItem:
				case KeyEditTextItem:
				case CodeEditTextItem:
					ListDataDlg_Set (dialId, userData);
					break;
			}
			break;
	}

	return (result);
}		// ListDataDlg_CB



// =============================================================================
//
// Main functions
//
// =============================================================================

#ifdef __APPLE__
#pragma mark -
#endif

// -----------------------------------------------------------------------------
// Call the list data dialog
// -----------------------------------------------------------------------------

static void		Do_ListData (void)
{
	API_ListData	param;

	BNZeroMemory (&param, sizeof (param));
	param.header.setIndex = 1;
	param.header.typeID   = API_ComponentID;

	DGModalDialog (ACAPI_GetOwnResModule (), 32510, ACAPI_GetOwnResModule (), ListDataDlg_CB, (DGUserData) (&param));

	return;
}		// Do_ListData


// -----------------------------------------------------------------------------
// MenuCommandHandler
//		called to perform the user-asked command
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	MenuCommandHandler (const API_MenuParams *params)
{
UNUSED_PARAMETER (params);

	Do_ListData ();

	return NoError;
}	// DoCommand


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
}		/* CheckEnvironment */


//------------------------------------------------------
// Interface definitions
//------------------------------------------------------
GSErrCode	__ACENV_CALL	RegisterInterface (void)
{
	ACAPI_Register_Menu (32500, 0, MenuCode_Calculate, MenuFlag_Default);
		return NoError;
}		/* RegisterInterface */


//------------------------------------------------------
// Called when the Add-On has been loaded into memory
// to perform an operation
//------------------------------------------------------
GSErrCode	__ACENV_CALL Initialize	(void)
{
	GSErrCode err = ACAPI_Install_MenuHandler (32500, MenuCommandHandler);
	if (err != NoError)
		DBPrintf ("Listing Manager:: Initialize() ACAPI_Install_MenuHandler failed\n");

	err =  InitList ();

	return err;
}		/* Initialize */


// -----------------------------------------------------------------------------
// FreeData
//		called when the Add-On is going to be unloaded
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	FreeData (void)
{
	BMKillHandle ((GSHandle *)  &listHdl);

	return NoError;
}	// FreeData

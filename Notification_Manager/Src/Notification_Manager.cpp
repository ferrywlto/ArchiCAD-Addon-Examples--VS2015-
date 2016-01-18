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
#define	_NOTIFICATION_MANAGER_TRANSL_


// ---------------------------------- Includes ---------------------------------

#include	<string.h>
#include	"ACAPinc.h"		// also includes APIdefs.h

#include	"APICommon.h"
#include	"Notification_Manager.h"

#include	"UniString.hpp"

// ---------------------------------- Types ------------------------------------

#define	 Menu_ProjectChanges		1
#define	 Menu_DefaultMonitor		3
#define	 Menu_ElementMonitor		4
#define	 Menu_ToolMonitor			5
#define	 Menu_SelectionMonitor		6
#define	 Menu_ReservationMonitor	7

// ---------------------------------- Variables --------------------------------

static	bool	projectEventsEnabled = false;
static	bool	defaultMonitorEnabled = false;
static	bool	elementMonitorEnabled = false;
static	bool	toolMonitorEnabled = false;
static	bool	selectionMonitorEnabled = false;
static	bool	reservationMonitorEnabled = false;

// ---------------------------------- Prototypes -------------------------------

// =============================================================================
//
// Utility functions
//
// =============================================================================


// -----------------------------------------------------------------------------
// GetElementTypeString
//	helper function
// -----------------------------------------------------------------------------
bool	GetElementTypeString (API_ElemTypeID typeID, char *elemStr)
{
	GS::UniString	ustr;
	GSErrCode	err = ACAPI_Goodies (APIAny_GetElemTypeNameID, (void*) typeID, &ustr);
	if (err == NoError) {
		CHTruncate (ustr.ToCStr (), elemStr, ELEMSTR_LEN - 1);
		return true;
	}
	return false;
}


// -----------------------------------------------------------------------------
// DoCommand
//		called to perform the user-asked command
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	MenuCommandHandler (const API_MenuParams *menuParams)
{
	GSErrCode err = ACAPI_CallUndoableCommand ("API Test",
		[&] () -> GSErrCode {

			switch (menuParams->menuItemRef.itemIndex) {
				case 1:
					Do_CatchProjectEvent (!projectEventsEnabled);
					InvertMenuItemMark (32500, Menu_ProjectChanges);
					projectEventsEnabled = !projectEventsEnabled;
					break;
				/* ----- */
				case 3:
					Do_DefaultMonitor (!defaultMonitorEnabled);
					InvertMenuItemMark (32500, Menu_DefaultMonitor);
					defaultMonitorEnabled = !defaultMonitorEnabled;
					break;
				case 4:
					Do_ElementMonitor (!elementMonitorEnabled);
					InvertMenuItemMark (32500, Menu_ElementMonitor);
					elementMonitorEnabled = !elementMonitorEnabled;
					break;
				case 5:
					Do_ToolMonitor (!toolMonitorEnabled);
					InvertMenuItemMark (32500, Menu_ToolMonitor);
					toolMonitorEnabled = !toolMonitorEnabled;
					break;
				case 6:
					Do_SelectionMonitor (!selectionMonitorEnabled);
					InvertMenuItemMark (32500, Menu_SelectionMonitor);
					selectionMonitorEnabled = !selectionMonitorEnabled;
					break;
				case 7:
					Do_ReservationMonitor (!reservationMonitorEnabled);
					InvertMenuItemMark (32500, Menu_ReservationMonitor);
					reservationMonitorEnabled = !reservationMonitorEnabled;
					break;
				/* ----- */
				case 9:
					Do_ClickedElementMonitor (true);
					break;
				case 10:
					Do_ClickedElementMonitor (false);
					break;
				case 11:
					Do_ListMonitoredElements ();
					break;
				/* ----- */
				case 13:
					Do_MarkSelElems ();
					break;
				case 14:
					Do_ListOwnedElements ();
					break;
				/* ----- */
				case 16:
					Do_AttachElementURLRef ();
					break;
				case 17:
					Do_ListElementURLRef ();
					break;
				/* ----- */
				case 19:
					Do_LinkElements ();
					break;
			}

			return NoError;
		});

	return err;
}		// MenuCommandHandler


// =============================================================================
//
// Required functions
//
// =============================================================================
#ifdef __APPLE__
#pragma mark -
#endif

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
	ACAPI_Register_Menu (32500, 0, MenuCode_UserDef, MenuFlag_SeparatorBefore);

	return NoError;
}		// RegisterInterface


// -----------------------------------------------------------------------------
// Initialize
//		called after the Add-On has been loaded into memory
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	Initialize (void)
{
	ACAPI_Install_MenuHandler (32500, MenuCommandHandler);

	return NoError;
}		// Initialize


// -----------------------------------------------------------------------------
// FreeData
//		called when the Add-On is going to be unloaded
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	FreeData (void)
{
	return NoError;
}		// FreeData

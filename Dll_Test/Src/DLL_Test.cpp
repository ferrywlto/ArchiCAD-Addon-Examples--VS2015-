// *****************************************************************************
// Source code for the Dll Test Add-On
// API Development Kit 19; Win
//
// Namespaces:			Contact person:
//	PanelTest
//
// [SG compatible] - Yes
// *****************************************************************************

// --- Includes ----------------------------------------------------------------

#include "ACAPlib_dll.h"

// -----------------------------------------------------------------------------
//
// callback functions
//
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL APIMenuCommandProc_Main (const API_MenuParams *menuParams)
{
	if (menuParams->menuItemRef.menuResID == 32500) {
		if (menuParams->menuItemRef.itemIndex == 1) {
			char buffer[256];
			ACAPI_Resource_GetLocStr (buffer, 32710, 2, ACAPI_GetOwnResModule());
			ACAPI_WriteReport (buffer, false);
		}
	}
	return NoError;
}		// APIMenuCommandProc_Main

GSErrCode __ACENV_CALL	APIProjectEventHandler (API_NotifyEventID notifID, Int32 /*param*/)
{
	char buffer[256];

	switch (notifID) {
		case APINotify_New:
			ACAPI_Resource_GetLocStr (buffer, 32710, 1, ACAPI_GetOwnResModule());
			ACAPI_WriteReport (buffer, false);
			break;

		default:
			break;
	}

	return NoError;
}		// APIProjectEventHandler

// -----------------------------------------------------------------------------
// Checking environment
// -----------------------------------------------------------------------------

API_AddonType	__ACENV_CALL	CheckEnvironment (API_EnvirParams* envir)
{
	if (envir->serverInfo.serverApplication != APIAppl_ArchiCADID)
		return APIAddon_DontRegister;

	ACAPI_Resource_GetLocStr (envir->addOnInfo.name, 32700, 1, ACAPI_GetOwnResModule());
	ACAPI_Resource_GetLocStr (envir->addOnInfo.description, 32700, 2, ACAPI_GetOwnResModule());

	return APIAddon_Preload;
}


// -----------------------------------------------------------------------------
// Interface definitions
// -----------------------------------------------------------------------------

GSErrCode	__ACENV_CALL	RegisterInterface (void)
{
	GSErrCode err = NoError;
	err |= ACAPI_Register_Menu (32500, 32600, MenuCode_UserDef, MenuFlag_Default);

	return err;
}


// -----------------------------------------------------------------------------
// Called after the Add-On has been loaded into memory
// -----------------------------------------------------------------------------

GSErrCode	__ACENV_CALL Initialize	(void)
{
	GSErrCode err = NoError;
	err |= ACAPI_Install_MenuHandler (32500, APIMenuCommandProc_Main);
	err |= ACAPI_Notify_CatchProjectEvent(APINotify_New, APIProjectEventHandler);
	return err;
}


// -----------------------------------------------------------------------------
// Called when the Add-On is going to be unloaded
// -----------------------------------------------------------------------------

GSErrCode	__ACENV_CALL FreeData	(void)
{
	return NoError;
}

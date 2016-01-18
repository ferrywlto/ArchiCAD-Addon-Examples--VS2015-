// *****************************************************************************
// File:			SQL.cpp
//
// Description:		SQL addon required add-on functions and callbacks
//
// Project:			APITools/SQL
//
// Namespace:		-
//
// Contact person:	BLA
//
// SG compatible
// *****************************************************************************

// --- Includes ----------------------------------------------------------------

#include "SQL.hpp"
#include "ExecuteQueryDialog.hpp"
#include "DGModule.hpp"

// -----------------------------------------------------------------------------
//
// Required functions
//
// -----------------------------------------------------------------------------

using namespace SQL;

GSErrCode __ACENV_CALL APIMenuCommandProc_Main (const API_MenuParams *menuParams)
{
	if (menuParams->menuItemRef.menuResID == 32500) {
		if (menuParams->menuItemRef.itemIndex == 1) {
			CExecuteQueryDialog dlg;
			dlg.Invoke ();
		}
	}
	return NoError;
}		// APIMenuCommandProc_Main

#ifdef WINDOWS
GSErrCode __ACENV_CALL APIMenuCommandProc_Lister (const API_MenuParams *menuParams)
{
	if (menuParams->menuItemRef.menuResID == 32501) {
		// Lister
	}
	return NoError;
}		// APIMenuCommandProc_Lister
#endif

// -----------------------------------------------------------------------------
// Dependency definitions
// -----------------------------------------------------------------------------

API_AddonType	__ACENV_CALL	CheckEnvironment (API_EnvirParams* envir)
{
	if (envir->serverInfo.serverApplication != APIAppl_ArchiCADID)
		return APIAddon_DontRegister;

	ACAPI_Resource_GetLocStr (envir->addOnInfo.name, 32000, 1);
	ACAPI_Resource_GetLocStr (envir->addOnInfo.description, 32000, 2);

	return APIAddon_Normal;			// miért volt ez PreLoad-os? - jg020917
}


// -----------------------------------------------------------------------------
// Interface definitions
// -----------------------------------------------------------------------------

GSErrCode	__ACENV_CALL	RegisterInterface (void)
{
	GSErrCode err = ACAPI_Register_Menu (32500, 32600, MenuCode_UserDef, MenuFlag_Default);

#ifdef WINDOWS
	if (err == NoError) {
		 err = ACAPI_Register_Menu (32501, 32601, MenuCode_Calculate, MenuFlag_Default);
	}
#endif

	return err;
}


// -----------------------------------------------------------------------------
// Called after the Add-On has been loaded into memory
// -----------------------------------------------------------------------------

GSErrCode	__ACENV_CALL Initialize	(void)
{
	GSErrCode err = ACAPI_Install_MenuHandler (32500, APIMenuCommandProc_Main);

#ifdef WINDOWS
	if (err == NoError) {
		 err = ACAPI_Install_MenuHandler (32501, APIMenuCommandProc_Lister);
	}
#endif

	return err;
}


// -----------------------------------------------------------------------------
// Called when the Add-On is going to be unloaded
// -----------------------------------------------------------------------------

GSErrCode	__ACENV_CALL FreeData	(void)
{
	return NoError;
}

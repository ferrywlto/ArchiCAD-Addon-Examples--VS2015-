// *****************************************************************************
// Source code for the Panel Test Add-On
// API Development Kit 19; Mac/Win
//
// Namespaces:			Contact person:
//	PanelTest
//
// [SG compatible] - Yes
// *****************************************************************************

// --- Includes ----------------------------------------------------------------

#include "APIEnvir.h"
#include "ACAPinc.h"
#include "APIdefs.h"

#include "Panel_Test_Resource.h"
#include "Panels.hpp"

#include "DGModule.hpp"

// -----------------------------------------------------------------------------
//
// Global variables and definitions
//
// -----------------------------------------------------------------------------

using namespace PanelTest;

TestSettingsPanel*	settingsPanel  = NULL;
TestAttributePanel*	attributePanel = NULL;
TestInfoBoxPanel*	infoBoxPanel   = NULL;

enum {
	SettingsPanelRefCon					= 1,
	AttributePanelRefCon				= 2,
	InfoBoxPanelRefCon					= 3,

	SettingsPanelRegistered				= 1 << SettingsPanelRefCon,
	AttributePanelRegistered			= 1 << AttributePanelRefCon,
	InfoBoxPanelRegistered				= 1 << InfoBoxPanelRefCon
};

int	registrationSuccess = 0;

// -----------------------------------------------------------------------------
// Create tabpage callback function
// -----------------------------------------------------------------------------

static	GSErrCode	__ACENV_CALL	CreatePageCallback (Int32 refCon, const void* tabControl, void* data, void** tabPage)
{
	bool success = false;
	const DG::TabControl*	control = reinterpret_cast<const DG::TabControl*>(tabControl);
	DG::TabPage**			page = reinterpret_cast<DG::TabPage**>(tabPage);

	switch (refCon) {
		case SettingsPanelRefCon:		if (settingsPanel != NULL)
											success = settingsPanel->CreatePage (*control, reinterpret_cast<TBUI::IAPIToolUIData*>(data), page);
										break;

		case AttributePanelRefCon:		if (attributePanel != NULL)
											success = attributePanel->CreatePage (*control, reinterpret_cast<VBAD::IAPIAttrUIData*>(data), page);
										break;

		case InfoBoxPanelRefCon:		if (infoBoxPanel != NULL)
											success = infoBoxPanel->CreatePage (*control, reinterpret_cast<TBUI::IAPIToolUIData*>(data), page);
										break;
	}

	return (success ? NoError : (GSErrCode) APIERR_GENERAL);
}


// -----------------------------------------------------------------------------
// Destroy tabpage callback function
// -----------------------------------------------------------------------------

static	GSErrCode	__ACENV_CALL	DestroyPageCallback (Int32 refCon, void* /*tabPage*/)
{
	switch (refCon) {
		case SettingsPanelRefCon:		if (settingsPanel != NULL)
											settingsPanel->DestroyPage ();
										break;

		case AttributePanelRefCon:		if (attributePanel != NULL)
											attributePanel->DestroyPage ();
										break;

		case InfoBoxPanelRefCon:		if (infoBoxPanel != NULL)
											infoBoxPanel->DestroyPage ();
										break;
	}

	return NoError;
}


// -----------------------------------------------------------------------------
//
// Required functions
//
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Dependency definitions
// -----------------------------------------------------------------------------

API_AddonType	__ACENV_CALL	CheckEnvironment (API_EnvirParams* envir)
{
	if (envir->serverInfo.serverApplication != APIAppl_ArchiCADID)
		return APIAddon_DontRegister;

	ACAPI_Resource_GetLocStr (envir->addOnInfo.name, 32000, 1);
	ACAPI_Resource_GetLocStr (envir->addOnInfo.description, 32000, 2);

	return APIAddon_Preload;
}


// -----------------------------------------------------------------------------
// Interface definitions
// -----------------------------------------------------------------------------

GSErrCode	__ACENV_CALL	RegisterInterface (void)
{
	GSErrCode err = NoError;

	// ------ Custom Rolldown Panel into the Wall Settings dialog
	err = ACAPI_Register_SettingsPanel (SettingsPanelRefCon, API_WallID, APIVarId_Generic, IDS_SETTINGSPAGE_ICON, IDS_SETTINGSPAGE_NAME, SettingsPageId);
	if (err != NoError) {
		DBPrintf ("Panel_Test add-on: Cannot register element settings panel\n");
	} else
		registrationSuccess |= SettingsPanelRegistered;

	// ------ Custom Rolldown Panel into the Material Settings dialog
	err = ACAPI_Register_AttributePanel (AttributePanelRefCon, API_MaterialID, IDS_ATTRIBUTEPAGE_ICON, IDS_ATTRIBUTEPAGE_NAME, AttributePageId, 0);
	if (err != NoError) {
		DBPrintf ("Panel_Test add-on: Cannot register attribute settings panel\n");
	} else
		registrationSuccess |= AttributePanelRegistered;

	// ------ Custom Panel into the Info Box
	err = ACAPI_Register_InfoBoxPanel (InfoBoxPanelRefCon, API_WallID, APIVarId_Generic, IDS_INFOBOXPAGE_NAME, InfoBoxPageId);
	if (err != NoError) {
		DBPrintf ("Panel_Test add-on: Cannot register info box panel\n");
	} else
		registrationSuccess |= InfoBoxPanelRegistered;

	return NoError;
}


// -----------------------------------------------------------------------------
// Called after the Add-On has been loaded into memory
// -----------------------------------------------------------------------------

GSErrCode	__ACENV_CALL Initialize	(void)
{
	GSErrCode err = NoError;

	// ------ Custom Rolldown Panel into the Wall Settings dialog
	if (registrationSuccess & SettingsPanelRegistered) {
		try {
			settingsPanel = new TestSettingsPanel (SettingsPanelRefCon);
		}
		catch (...) {
			DBPrintf ("Panel_Test add-on: settingsPanel construction failed\n");
			settingsPanel = NULL;
		}

		if (settingsPanel != NULL) {
			err = ACAPI_Install_PanelHandler (settingsPanel->GetRefCon (), CreatePageCallback, DestroyPageCallback);
			if (err != NoError) {
				DBPrintf ("Panel_Test add-on: Element settings panel handler initialization failed\n");
			}
		}
	}

	// ------ Custom Rolldown Panel into the Material Settings dialog
	if (registrationSuccess & AttributePanelRegistered) {
		try {
			attributePanel = new TestAttributePanel (AttributePanelRefCon);
		}
		catch (...) {
			DBPrintf ("Panel_Test add-on: attributePanel construction failed\n");
			attributePanel = NULL;
		}

		if (attributePanel != NULL) {
			err = ACAPI_Install_PanelHandler (attributePanel->GetRefCon (), CreatePageCallback, DestroyPageCallback);
			if (err != NoError) {
				DBPrintf ("Panel_Test add-on: Attribute settings panel handler initialization failed\n");
			}
		}
	}

	// ------ Custom Panel into the Info Box
	if (registrationSuccess & InfoBoxPanelRegistered) {
		try {
			infoBoxPanel = new TestInfoBoxPanel (InfoBoxPanelRefCon);
		}
		catch (...) {
			DBPrintf ("Panel_Test add-on: infoBoxPanel construction failed\n");
			infoBoxPanel = NULL;
		}

		if (infoBoxPanel != NULL) {
			err = ACAPI_Install_PanelHandler (infoBoxPanel->GetRefCon (), CreatePageCallback, DestroyPageCallback);
			if (err != NoError) {
				DBPrintf ("Panel_Test add-on: Info box panel handler initialization failed\n");
			}
		}
	}

	return NoError;
}


// -----------------------------------------------------------------------------
// Called when the Add-On is going to be unloaded
// -----------------------------------------------------------------------------

GSErrCode	__ACENV_CALL FreeData	(void)
{
	if (settingsPanel != NULL)
		delete settingsPanel;
	if (attributePanel != NULL)
		delete attributePanel;
	if (infoBoxPanel != NULL)
		delete infoBoxPanel;

	return NoError;
}

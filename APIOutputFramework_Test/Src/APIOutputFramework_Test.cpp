/**
 * @file Contains the functions required by ArchiCAD.
 */

#include "APIOutputFramework_Test.hpp"
#include "Array.hpp"
#include "BFSTreeSelector.hpp"
#include "TextFileOutputManager.hpp"
#include "APIOutputApplication.hpp"
#include "DrawOrderElementContainer.hpp"
#include "ElementIterator.hpp"
#include "WWOOutputAdapterFactory.hpp"
#include "AttributeProxy.hpp"
#include "NullTypeAttributeAdapterFactory.hpp"
#include "FindByGuid.hpp"


enum {
	FILE_TYPE_MENU_ID		= 32500,

	FIND_GUID_MENU_ID		= 32501,
	FIND_GUID_MENU_HELP_ID	= 32502
};

/**
 * Dependency definitions. Function name is fixed.
 *
 * @param envir [in] ArchiCAD environment values.
 * @return The Add-On loading type.
 */
API_AddonType	__ACENV_CALL	CheckEnvironment (API_EnvirParams* envir)
{
	ACAPI_Resource_GetLocStr (envir->addOnInfo.name, 32000, 1);
	ACAPI_Resource_GetLocStr (envir->addOnInfo.description, 32000, 2);

	return APIAddon_Normal;
}		// CheckEnvironment ()

/**
 * Interface definitions. Function name is fixed.
 *
 * @return ArchiCAD error code.
 */
GSErrCode	__ACENV_CALL	RegisterInterface (void)
{
	//
	// Register a file type
	//
	GSErrCode err = ACAPI_Register_FileType (1, 'TEXT', 0, "wwo", 0, FILE_TYPE_MENU_ID, 1, SaveAs2DSupported);

	if (err == NoError) {
		err = ACAPI_Register_Menu (FIND_GUID_MENU_ID, FIND_GUID_MENU_HELP_ID, MenuCode_Extras, MenuFlag_SeparatorBefore);
	}

	return err;
}		// RegisterInterface ()

/**
 * Save as command handler function. Function name is NOT fixed. There can be
 * more than one of these functions. Please check the API Development Kit
 * documentation for more information.
 *
 * @param params [in] Parameters of the menu command.
 * @return ArchiCAD error code.
 */
GSErrCode __ACENV_CALL	IOCommandHandler (const API_IOParams* ioParams)
{
	GS::Array<API_DatabaseInfo> floorPlanArray;
	floorPlanArray.Push (GSAPI::DatabaseGraph::Instance ().SelectDatabases (GSAPI::BFSTreeSelector ())[0]);

	// we need output only on the floor plan
	GSAPI::BFSTreeSelector selector (floorPlanArray);
	TextFileOutputManager outputManager (*ioParams->fileLoc);

	// we want only walls and openings
	API_ElemFilter wwoElemFilter;
	BNZeroMemory (&wwoElemFilter, sizeof (wwoElemFilter));
	wwoElemFilter[API_WallID] = true;
	wwoElemFilter[API_WindowID] = true;
	wwoElemFilter[API_DoorID] = true;

	// we want only the pen colors
	GSAPI::AttributeFilter wwoAttributeFilter;
	BNZeroMemory (&wwoAttributeFilter, sizeof (wwoAttributeFilter));
	wwoAttributeFilter[API_PenID] = true;

	GSAPI::APIOutputApplication outputApplication;
	// we want to output the pen color attributes inline during element output, so no Attribute Adapter is needed -> use NullTypeAttributeAdapterFactory
	GSErrCode gsError = outputApplication.OutputElements<GSAPI::DrawOrderElementContainer<GSAPI::ElementIterator>,
														 WWOOutputAdapterFactory,
														 GSAPI::AttributeProxy<GSAPI::BFSTreeSelector>,
														 GSAPI::NullTypeAttributeAdapterFactory> (outputManager, wwoElemFilter, selector, wwoAttributeFilter, GSAPI::InlineOutput_Index);
	return gsError;
}		// IOCommandHandler ()

/**
 * Menu command: "Find by GUID"
 *
 * @return ArchiCAD error code.
 */
GSErrCode	__ACENV_CALL	MenuHandler (const API_MenuParams *menuParams)
{
	GSErrCode err = NoError;

    if (menuParams->menuItemRef.menuResID == FIND_GUID_MENU_ID)
		err = FindDialog::FindElemByGuid ();

	return err;
}

/**
 * Called after the Add-On has been loaded into memory. Function name is fixed.
 *
 * @return ArchiCAD error code.
 */
GSErrCode __ACENV_CALL	Initialize (void)
{
	GSErrCode err = ACAPI_Install_FileTypeHandler (1, IOCommandHandler);

	if (err == NoError) {
		err = ACAPI_Install_MenuHandler (FIND_GUID_MENU_ID, MenuHandler);
	}

	return err;
}		// Initialize ()

/**
 * Called when the Add-On is going to be unloaded. Function name is fixed.
 *
 * @return ArchiCAD error code.
 */
GSErrCode __ACENV_CALL	FreeData (void)

{

	return NoError;
}		// FreeData ()

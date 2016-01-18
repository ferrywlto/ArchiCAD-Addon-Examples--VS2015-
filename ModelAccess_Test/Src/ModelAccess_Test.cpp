// *****************************************************************************
// Source code for the ModelAccess Test Add-On
// API Development Kit 19; Mac/Win
//
// Namespaces:			Contact person:
//
//
// [SG compatible] - Yes
// *****************************************************************************

// ---------------------------------- Includes ---------------------------------

#include "APIEnvir.h"
#include "ACAPinc.h"

#include "Location.hpp"
#include "FileSystem.hpp"
#include "EXP.h"
#include "Model3D/Model3DMain.hpp"
#include "Model.hpp"

#include "ModelAccess_Test_Common.hpp"
#include "ModelAccess_Test_Exporter.hpp"
#include "ModelAccess_Test_Resources.hpp"
#include "Sight.hpp"

#if defined (_MSC_VER)
#pragma warning (disable: 4702)		// unreachable code
#endif

// ---------------------------- Function definitions ---------------------------

namespace WRL {
IMPLEMENT_EXCEPTION_CLASS (
		Cancel,
		GS::RootException,
		Error
)
}

GSErrCode	__ACENV_CALL	StartExport (const API_IOParams* ioParams, Modeler::SightPtr sight);

// -----------------------------------------------------------------------------
// Start the export
// -----------------------------------------------------------------------------
GSErrCode	__ACENV_CALL	StartExport (const API_IOParams* ioParams, Modeler::SightPtr sight)
{
	ModelerAPI::Model model;
	EXPGetModel (sight, &model);

	try {
		WRL::MAExporter	exporter (&model, ioParams);
		exporter.ExportScene ();
	}

	catch (WRL::Cancel&) {
		IO::Location	fileLoc;
		fileLoc = *ioParams->fileLoc;
		IO::fileSystem.Delete (fileLoc);

		return APIERR_CANCEL;
	}

	catch (...) {
		IO::Location	fileLoc;
		fileLoc = *ioParams->fileLoc;
		IO::fileSystem.Delete (fileLoc);

		return APIERR_GENERAL;
	}

	return NoError;
}


// =============================================================================
//
// Required functions
//
// =============================================================================

#ifdef __APPLE__
#pragma mark -
#endif

// -----------------------------------------------------------------------------
// Dependency definitions
// -----------------------------------------------------------------------------
API_AddonType	__ACENV_CALL	CheckEnvironment (API_EnvirParams* envir)
{
	RSGetIndString (envir->addOnInfo.name, WRL::AddonInfoStringTable, WRL::AddonName, ACAPI_GetOwnResModule ());
	RSGetIndString (envir->addOnInfo.description, WRL::AddonInfoStringTable, WRL::AddonDescription, ACAPI_GetOwnResModule ());

	return APIAddon_Normal;
}


// -----------------------------------------------------------------------------
// Interface definitions
// -----------------------------------------------------------------------------
GSErrCode	__ACENV_CALL	RegisterInterface (void)
{
	return ACAPI_Register_FileType (1, 'TEXT', 'xxxx', "txt", 0, (short) WRL::AddonInfoStringTable, (short) WRL::Format, SaveAs3DSupported);
}


// -----------------------------------------------------------------------------
// Called when the Add-On has been loaded into memory
// to perform an operation
// -----------------------------------------------------------------------------
GSErrCode	__ACENV_CALL	Initialize	(void)
{
	GSErrCode	errCode = ACAPI_Install_FileTypeHandler3D (1, StartExport);
	if (errCode != NoError)
		DBPrintf ("ModelAccess_Test:: Initialize() ACAPI_Install_FileTypeHandler3D failed\n");

	return errCode;
}


// -----------------------------------------------------------------------------
// FreeData
//		called when the Add-On is going to be unloaded
// -----------------------------------------------------------------------------
GSErrCode __ACENV_CALL		FreeData (void)
{
	return NoError;
}

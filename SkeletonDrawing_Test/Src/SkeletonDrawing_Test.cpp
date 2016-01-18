// *****************************************************************************
// Source code for the SkeletonDrawing_Test Add-On
// API Development Kit 19; Mac/Win
//
// Namespaces:			Contact person: PP
//		-None-
//
// [SG compatible] - Yes
// *****************************************************************************

#include "APIEnvir.h"


// ---------------------------------- Includes ---------------------------------

#include	"ACAPinc.h"					// also includes APIdefs.h


// =============================================================================
//
// Main functions
//
// =============================================================================


typedef struct {
	unsigned short	platform;
	unsigned short	filler;
	UInt32	last3DDocument;
} PrefsData;

static PrefsData	*prefs = NULL;

// -----------------------------------------------------------------------------
// Add a 3DD
// -----------------------------------------------------------------------------

static void		Create3DD (bool skeleton)
{
	API_DatabaseInfo dbInfo;
	BNZeroMemory (&dbInfo, sizeof (API_DatabaseInfo));
	dbInfo.typeID = APIWind_DocumentFrom3DID;

	GS::snuprintf (dbInfo.name, sizeof (dbInfo.name), "Skeleton Drawing Test #%d (%s)", prefs->last3DDocument, skeleton ? "Skeleton" : "3DD");
	GS::snuprintf (dbInfo.ref, sizeof (dbInfo.ref), "3DD #%d", prefs->last3DDocument++);

	if (ACAPI_Database (APIDb_NewDatabaseID, &dbInfo, NULL) == NoError) {
		DBPrintf ("DatabaseControl :: The ID of the new Document 3D ElemSet is \"%s\"\n",
			APIGuidToString (dbInfo.databaseUnId.elemSetId).ToCStr ().Get ());
	}

	API_DocumentFrom3DType documentFrom3DType;

	GSErrCode err = ACAPI_Environment (APIEnv_GetDocumentFrom3DSettingsID, &dbInfo.databaseUnId, &documentFrom3DType);
	if (err == NoError) {
		documentFrom3DType.docSrc						= skeleton ? API_FloorPlan : API_DDD;
		documentFrom3DType.skeletonOptions.viewSrc		= API_Top;
		documentFrom3DType.skeletonOptions.floorIdx		= 0;
		documentFrom3DType.skeletonOptions.cutHeight	= 0.5;
		documentFrom3DType.skeletonOptions.floorOffset	= 1;
		documentFrom3DType.skeletonOptions.cutOffset	= 1.5;

		err = ACAPI_Environment (APIEnv_ChangeDocumentFrom3DSettingsID, &dbInfo.databaseUnId, &documentFrom3DType);
		if (err != NoError)
			DBPrintf ("APIEnv_ChangeDocumentFrom3DSettingsID returns with error code");
	} else {
		DBPrintf ("APIEnv_GetDocumentFrom3DSettingsID returns with error code");
	}

	BMhFree ((GSHandle) documentFrom3DType.cutSetting.shapes);
 
 	return;
}		// AddSkeleton


// -----------------------------------------------------------------------------
// Handles menu commands
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL MenuCommandHandler (const API_MenuParams *menuParams)
{
	switch (menuParams->menuItemRef.menuResID) {
		case 32500:
			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		Create3DD (true);	break;
				case 2:		Create3DD (false);	break;
			}
			break;
	}


	return NoError;
}		// MenuCommandHandler


// =============================================================================
//
// Required functions
//
// =============================================================================


// -----------------------------------------------------------------------------
// InitPreferences
//		initialize preference data
// -----------------------------------------------------------------------------
static void	InitPreferences (void)
{
	prefs = new PrefsData;
	if (prefs != NULL) {
		Int32	version;
		GSSize	nBytes;

		ACAPI_GetPreferences (&version, &nBytes, NULL);
		if (version == 1 && nBytes == sizeof (PrefsData)) {
			ACAPI_GetPreferences (&version, &nBytes, prefs);
			if (prefs->platform != GS::Act_Platform_Sign) {
				IVULong ((GS::PlatformSign) (prefs->platform), &prefs->last3DDocument);
				prefs->platform = GS::Act_Platform_Sign;
			}
		} else {
			prefs->platform = GS::Act_Platform_Sign;
			prefs->last3DDocument = 1;
		}
	}
}		// InitPreferences


// -----------------------------------------------------------------------------
// Dependency definitions
// -----------------------------------------------------------------------------

API_AddonType	__ACENV_CALL	CheckEnvironment (API_EnvirParams* envir)
{
	if (envir->serverInfo.serverApplication != APIAppl_ArchiCADID)
		return APIAddon_DontRegister;
		
	ACAPI_Resource_GetLocStr (envir->addOnInfo.name, 32000, 1);
	ACAPI_Resource_GetLocStr (envir->addOnInfo.description, 32000, 2);

	return APIAddon_Normal;
}		// CheckEnvironment


// -----------------------------------------------------------------------------
// Interface definitions
// -----------------------------------------------------------------------------

GSErrCode	__ACENV_CALL	RegisterInterface (void)
{
	GSErrCode err = NoError;
	err = ACAPI_Register_Menu (32500, 0, MenuCode_UserDef, MenuFlag_SeparatorBefore);

	return err;
}		// RegisterInterface


// -----------------------------------------------------------------------------
// Called when the Add-On has been loaded into memory
// to perform an operation
// -----------------------------------------------------------------------------

GSErrCode	__ACENV_CALL Initialize	(void)
{
	GSErrCode err = ACAPI_Install_MenuHandler (32500, MenuCommandHandler);
	if (err != NoError)
		DBPrintf ("SkeletonDrawing_Test:: Initialize () ACAPI_Install_MenuHandler failed\n");

	InitPreferences ();

	return err;
}		// Initialize


// -----------------------------------------------------------------------------
// FreeData
//		called when the Add-On is going to be unloaded
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	FreeData (void)

{
	if (prefs != NULL) {
		ACAPI_SetPreferences (1L, sizeof (PrefsData), (const void *) prefs);
		delete prefs;
	}

	return NoError;
}		// FreeData

// *****************************************************************************
// Source code for the Communication Manager Add-On
// API Development Kit 19; Mac/Win
//
// Namespaces:			Contact person:
//		-None-
//
// [SG compatible] - Yes
// *****************************************************************************

#include "APIEnvir.h"
#define	_COMMUNICATION_MANAGER_TRANSL_

// ---------------------------------- Includes ---------------------------------

#include	"ACAPinc.h"					// also includes APIdefs.h

#include	"MDIDs_APICD.h"
#include	"APICommon.h"
#include	"Communication_Client.h"
#include	"Location.hpp"
#include	"Folder.hpp"
#include	"StringConversion.hpp"
#include	"DGFileDialog.hpp"


// ---------------------------------- Types ------------------------------------


// ---------------------------------- Variables --------------------------------


// ---------------------------------- Prototypes -------------------------------



// =============================================================================
//
//
//
// =============================================================================

// -----------------------------------------------------------------------------
// Test communication between us and the client
// -----------------------------------------------------------------------------

static void		Do_CMD1_Test (void)
{
	API_ModulID		mdid;
	GSErrCode		err;

	BNZeroMemory (&mdid, sizeof (API_ModulID));
	mdid.developerID = 'xxxx';								// <--- Bad data (invalid Module ID). ACAPI_TestCommand should fail
	mdid.localID = MDID_APICD_CommunicationClient;
	err = ACAPI_Command_Test (&mdid, CmdID_PutText, 0);
	DBASSERT (err == APIERR_MODULNOTINSTALLED);


	BNZeroMemory (&mdid, sizeof (API_ModulID));
	mdid.developerID = MDID_APICD;
	mdid.localID = MDID_APICD_CommunicationClient;
	err = ACAPI_Command_Test (&mdid, 'xxxx', 0);			// <--- Bad data (invalid command ID). ACAPI_Command_Test should fail
	DBASSERT (err == APIERR_MODULCMDNOTSUPPORTED);

	BNZeroMemory (&mdid, sizeof (API_ModulID));
	mdid.developerID = MDID_APICD;
	mdid.localID = MDID_APICD_CommunicationClient;
	err = ACAPI_Command_Test (&mdid, CmdID_PutText, 2);		// <--- Bad data (too new command version). ACAPI_Command_Test should fail
	DBASSERT (err == APIERR_MODULCMDVERSNOTSUPPORTED);

	err = ACAPI_Command_Test (&mdid, CmdID_PutText, 1);
	if (err != NoError) {
		switch (err) {
			case APIERR_MODULNOTINSTALLED:
						WriteReport_Alert ("Target add-on is not installed");
						break;
			case APIERR_MODULCMDNOTSUPPORTED:
						WriteReport_Alert ("Bad command ID was passed");
						break;
			case APIERR_MODULCMDVERSNOTSUPPORTED:
						WriteReport_Alert ("Bad command version");
						break;
		}
	}

	return;
}		// Do_CMD1_Test


// -----------------------------------------------------------------------------
// Call the Communication Client module
// -----------------------------------------------------------------------------

static void		Do_CMD1_Call (void)
{
	bool				passParameters, avoidDialog;
	GSHandle			parHdl;
	API_MDCLParameter	par;
	API_ModulID			mdid;
	GSErrCode			err;

	passParameters = GetMenuItemMark (32500, 1);
	avoidDialog    = GetMenuItemMark (32500, 2);

	if (passParameters) {
		err = ACAPI_Goodies (APIAny_InitMDCLParameterListID, &parHdl, NULL);
		if (err != NoError)
			ErrorBeep ("APIAny_InitMDCLParameterListID", err);

		if (err == NoError) {
			par.name = "pen";
			par.type = MDCLPar_int;
			par.int_par = 14;
			err = ACAPI_Goodies (APIAny_AddMDCLParameterID, parHdl, &par);
			if (err != NoError)
				ErrorBeep ("APIAny_AddMDCLParameterID", err);
		}
		if (err == NoError) {
			par.name = "size";
			par.type = MDCLPar_float;
			par.float_par = 6.5;
			err = ACAPI_Goodies (APIAny_AddMDCLParameterID, parHdl, &par);
			if (err != NoError)
				ErrorBeep ("APIAny_AddMDCLParameterID", err);
		}
		if (err == NoError) {
			par.name = "content";
			par.type = MDCLPar_string;
			par.string_par = "This string is coming from another module";
			err = ACAPI_Goodies (APIAny_AddMDCLParameterID, parHdl, &par);
			if (err != NoError)
				ErrorBeep ("APIAny_AddMDCLParameterID", err);
		}
	} else
		parHdl = NULL;

	BNZeroMemory (&mdid, sizeof (API_ModulID));
	mdid.developerID = MDID_APICD;
	mdid.localID = MDID_APICD_CommunicationClient;
	err = ACAPI_Command_Call (&mdid, CmdID_PutText, 1, parHdl, NULL, avoidDialog);

	if (parHdl != NULL) {
		err = ACAPI_Goodies (APIAny_FreeMDCLParameterListID, &parHdl, NULL);
		if (err != NoError)
			ErrorBeep ("APIAny_FreeMDCLParameterListID", err);
	}

	return;
}		// Do_CMD1_Call


// -----------------------------------------------------------------------------
// Create a link to the Communication Client
// -----------------------------------------------------------------------------

static void		Do_LinkToClient (void)
{
	API_ModulID			mdid;
	CallBackTbl_Client	procTbl;
	GSErrCode			err;

	BNZeroMemory (&mdid, sizeof (API_ModulID));
	mdid.developerID = MDID_APICD;
	mdid.localID = MDID_APICD_CommunicationClient;

	err = ACAPI_Command_Call (&mdid, CmdID_BegBinaryLink, 1, NULL, (GSPtr) &procTbl, false);
	if (err != NoError) {
		ErrorBeep ("CmdID_BegBinaryLink", err);
		return;
	}

	(*procTbl.counterProc) ();
	(*procTbl.counterProc) ();
	(*procTbl.counterProc) ();
	(*procTbl.counterProc) ();

	err = ACAPI_Command_Call (&mdid, CmdID_EndBinaryLink, 1, NULL, NULL, false);
	if (err != NoError) {
		ErrorBeep ("CmdID_BegBinaryLink", err);
		return;
	}

	return;
}		// Do_LinkToClient


// -----------------------------------------------------------------------------
// Create a link to the 3D Manager
// -----------------------------------------------------------------------------

static void		Do_Dump3DModel (void)
{
	API_ModulID		mdid;
	GSErrCode		err;

	BNZeroMemory (&mdid, sizeof (API_ModulID));
	mdid.developerID = MDID_APICD;
	mdid.localID = MDID_APICD_3DTest;

	err = ACAPI_Command_Call (&mdid, 'DUMP', 1, NULL, NULL, false);
	if (err != NoError) {
		ErrorBeep ("\'DUMP\'", err);
		return;
	}

	return;
}		// Do_Dump3DModel


// -----------------------------------------------------------------------------
// Save a given project as DWG file in external ArchiCAD
// -----------------------------------------------------------------------------
static Int32	waitingForProjectCounter = 0;

GSErrCode __ACENV_CALL	ExternalCallBackProc (GSHandle parHdl, GSErrCode returnCode)
{
	if (waitingForProjectCounter <= 0)
		return APIERR_GENERAL;				// unexpected call

	waitingForProjectCounter--;

	if (returnCode != NoError)
		return NoError;

	if (parHdl == NULL)
		return APIERR_GENERAL;

	Int32		nPars, i;
	GSErrCode	err;

	err = ACAPI_Goodies (APIAny_GetMDCLParameterNumID, parHdl, &nPars);
	if (err != NoError)
		return err;

	IO::Location	outfile, configFile;
	for (i = 1; i <= nPars; i++) {
		API_MDCLParameter	par;
		BNZeroMemory (&par, sizeof (par));
		par.index = i;
		err = ACAPI_Goodies (APIAny_GetMDCLParameterID, parHdl, &par);
		if (err != NoError)
			return err;
		if (CHCompareCStrings (par.name, "FileName", CS_CaseSensitive) == 0)
			outfile.Set (par.string_par);
		else if (CHCompareCStrings (par.name, "ConfigFile", CS_CaseSensitive) == 0)
			configFile.Set (par.string_par);
	}

	if (!outfile.IsLegal ())
		return APIERR_GENERAL;

	IO::Path outPath;
	outfile.ToDisplayText (&outPath);
	char msgStr[1024];
	sprintf (msgStr, "DXF/DWG file \"%s\" was saved in external ArchiCAD.", (const char *)outPath);
	ACAPI_WriteReport (msgStr, false);

	return NoError;
}


static void		Do_CallExternalSaveAsDWG (void)
{
	API_ModulID	mdid = { 1198731108, 1322668197 };		// DXF/DWG add-on
	API_MDCLParameter	par;
	GSHandle			parHdl;
	GSErrCode			err;
	IO::Location		configFile;

	waitingForProjectCounter = 0;

	// local call DWG add-on to retrieve the default translator
	err = ACAPI_Goodies (APIAny_InitMDCLParameterListID, &parHdl, NULL);
	if (err != NoError)
		return;

	BNZeroMemory (&par, sizeof (par));
	par.name = "Mode";
	par.type = MDCLPar_string;
	par.string_par = "Default";
	err = ACAPI_Goodies (APIAny_AddMDCLParameterID, parHdl, &par);
	if (err == NoError)
		err = ACAPI_Command_Call (&mdid, 'GDCO', 1, parHdl, NULL, false);	// get default translator

	if (err == NoError) {
		Int32 nPars = 0;
		err = ACAPI_Goodies (APIAny_GetMDCLParameterNumID, parHdl, &nPars);
		if (err == NoError && nPars == 1) {
			BNZeroMemory (&par, sizeof (par));
			par.index = 1;
			err = ACAPI_Goodies (APIAny_GetMDCLParameterID, parHdl, &par);
			if (err == NoError && par.type == MDCLPar_string && par.string_par[0] != 0) {
				configFile.Set (par.string_par);
			}
		}
	}

	ACAPI_Goodies (APIAny_FreeMDCLParameterListID, &parHdl, NULL);

	if (!configFile.IsValid ())
		return;


	// external call DWG add-on to save from external projects
	IO::Location projectFileLoc[2];
	API_SpecFolderID specID = API_ApplicationFolderID;
	ACAPI_Environment (APIEnv_GetSpecFolderID, &specID, &projectFileLoc[0]);
	projectFileLoc[0].AppendToLocal (IO::Name ("ArchiCAD Examples"));
	projectFileLoc[0].AppendToLocal (IO::Name ("Orchard Project"));
	projectFileLoc[0].AppendToLocal (IO::Name ("Orchard Project.pla"));
	ACAPI_Environment (APIEnv_GetSpecFolderID, &specID, &projectFileLoc[1]);
	projectFileLoc[1].AppendToLocal (IO::Name ("ArchiCAD Examples"));
	projectFileLoc[1].AppendToLocal (IO::Name ("Residential House"));
	projectFileLoc[1].AppendToLocal (IO::Name ("Residential House.pla"));

	IO::Location outputLoc[2];
	specID = API_UserDocumentsFolderID;
	ACAPI_Environment (APIEnv_GetSpecFolderID, &specID, &(outputLoc[0]));
	outputLoc[0].AppendToLocal (IO::Name ("Orchard Project.DWG"));
	ACAPI_Environment (APIEnv_GetSpecFolderID, &specID, &(outputLoc[1]));
	outputLoc[1].AppendToLocal (IO::Name ("Residential House.DWG"));

	char str[512];
	IO::URL url;
	for (Int32 indProj = 0; indProj < 2; indProj++) {
		outputLoc[indProj].ToURL (&url);
		err = ACAPI_Goodies (APIAny_InitMDCLParameterListID, &parHdl, NULL);
		if (err != NoError)
			break;

		BNZeroMemory (&par, sizeof (par));
		par.name = "FileName";
		par.type = MDCLPar_string;
		CHTruncate ((const char*) url, str, sizeof (str));
		par.string_par = str;
		err = ACAPI_Goodies (APIAny_AddMDCLParameterID, parHdl, &par);
		if (err != NoError)
			break;

		BNZeroMemory (&par, sizeof (par));
		par.name = "ConfigFile";
		par.type = MDCLPar_string;
		configFile.ToURL (&url);
		CHTruncate ((const char*) url, str, sizeof (str));
		par.string_par = str;
		err = ACAPI_Goodies (APIAny_AddMDCLParameterID, parHdl, &par);
		if (err != NoError)
			break;

		err = ACAPI_Command_ExternalCall (&(projectFileLoc[indProj]), &mdid, 'SAV2', 1, parHdl, true, ExternalCallBackProc);
		if (err == NoError)
			waitingForProjectCounter++;

		ACAPI_Goodies (APIAny_FreeMDCLParameterListID, &parHdl, NULL);
	}

	Int32 numOfTasks = waitingForProjectCounter;
	if (numOfTasks > 0) {
		Int32 nPhase = 1;
		GS::UniString title ("Exporting External Projects");
		ACAPI_Interface (APIIo_InitProcessWindowID, &title, &nPhase);
		GS::UniString subtitle ("Please wait...");
		ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &numOfTasks);
	}

	// waiting for the answers
	Int32 procValue = numOfTasks - waitingForProjectCounter;
	while (waitingForProjectCounter > 0) {
		if (procValue != numOfTasks - waitingForProjectCounter) {
			procValue = numOfTasks - waitingForProjectCounter;
			ACAPI_Interface (APIIo_SetProcessValueID, &procValue, NULL);
		}
		TIWait (0.05);		// let other threads run in the meanwhile
		if (ACAPI_Interface (APIIo_IsProcessCanceledID, NULL, NULL))
			break;
	}

	if (numOfTasks > 0)
		ACAPI_Interface (APIIo_CloseProcessWindowID, NULL, NULL);

	return;
}		// Do_CallExternalSaveAsDWG


// -----------------------------------------------------------------------------
// Open the selected DWG file into a library part
// -----------------------------------------------------------------------------
static bool	GetOpenFile (IO::Location*	dloc, const char* fileExtensions, const GS::UniString& filterText)
{
	FTM::TypeID	retID;
	FTM::FileTypeManager	ftman ("MyManager");
	FTM::FileType			type (NULL, fileExtensions, 0, 0, 0);
	FTM::TypeID				id = FTM::FileTypeManager::SearchForType (type);
	if (id == FTM::UnknownType)
		id = ftman.AddType (type);

	DG::FileDialog	dlg (DG::FileDialog::OpenFile);			// Open only 1 file
	UIndex	i = dlg.AddFilter (id, DG::FileDialog::DisplayExtensions);	// Force it on mac...
	dlg.SetFilterText (i, filterText);

	if (dlg.Invoke ()) {
		*dloc = dlg.GetSelectedFile ();
		return (true);
	} else
		return (false);
}

static void		Do_CallOpenDWGToObject (void)
{
	API_ModulID	mdid = { 1198731108, 1322668197 };		// DXF/DWG add-on
	API_MDCLParameter	par;
	GSHandle			parHdl;
	GSErrCode			err;

	IO::Location dwgFileLoc;
	if (!GetOpenFile (&dwgFileLoc, "dwg", "*.dwg"))
		return;

	IO::Name	dwgName;
	dwgFileLoc.GetLastLocalName (&dwgName);

	IO::URL url;
	dwgFileLoc.ToURL (&url);

	API_LibPart	libPart;
	BNZeroMemory (&libPart, sizeof (API_LibPart));
	GS::ucscpy (libPart.docu_UName, dwgName.GetBase ().ToUStr ());
	libPart.typeID = APILib_ObjectID;
	libPart.isTemplate = false;
	libPart.isPlaceable = true;

	CHCopyC ("{103E8D2C-8230-42E1-9597-46F84CCE28C0}-{00000000-0000-0000-0000-000000000000}", libPart.parentUnID);	// Model Element subtype

	IO::Location fileLoc;
	API_SpecFolderID specID = API_UserDocumentsFolderID;
	ACAPI_Environment (APIEnv_GetSpecFolderID, &specID, &fileLoc);
	fileLoc.AppendToLocal (IO::Name ("DWGLibPart"));

	libPart.location = &fileLoc;
	err = ACAPI_LibPart_Create (&libPart);

	// call the Dxf add-on
	err = ACAPI_Goodies (APIAny_InitMDCLParameterListID, &parHdl, NULL);
	if (err == NoError) {
		char	str[512] = {0};
		BNZeroMemory (&par, sizeof (par));
		par.name = "FileName";
		par.type = MDCLPar_string;
		CHTruncate ((const char*) url, str, sizeof (str));
		par.string_par = str;
		err = ACAPI_Goodies (APIAny_AddMDCLParameterID, parHdl, &par);

		if (err == NoError)
			err = ACAPI_Command_Call (&mdid, 'OOBJ', 1, parHdl, NULL, true);
	}

	ACAPI_Goodies (APIAny_FreeMDCLParameterListID, &parHdl, NULL);

	if (err == NoError)
		err = ACAPI_LibPart_Save (&libPart);

	libPart.location = NULL;

	return;
}		// Do_CallOpenDWGToObject


static void		Do_CallImportPointCloud (void)
{
	IO::Location pcToImportLoc;		// location of the point cloud file to import
	IO::Location pcTargetLoc;		// target folder of the GSPC file

	if (!GetOpenFile (&pcToImportLoc, "e57", "*.e57"))
		return;

	pcTargetLoc = pcToImportLoc;
	pcTargetLoc.DeleteLastLocalName ();
	pcTargetLoc.AppendToLocal (IO::Name ("resultGSPCs"));
	IO::Folder pcTargetFolder (pcTargetLoc, IO::Folder::Create);
	if (DBERROR (pcTargetFolder.GetStatus () != NoError))
		return;

	API_ModulID pcInMdid = { 1198731108, 797613762 };	// MDID of the Point Cloud Importer add-on

	if (DBERROR (ACAPI_Command_Test (&pcInMdid, 'PCIN', 1L) != NoError))	//current version
		return;

	if (DBERROR (ACAPI_Command_Test (&pcInMdid, 'PCIN', 2L) == NoError))	//2 shouldn't be supported (if it is, test it too!)
		return;

	GSHandle parHdl = NULL;		// handle of command call parameters

	GSErrCode err = ACAPI_Goodies (APIAny_InitMDCLParameterListID, &parHdl, NULL, NULL);
	if (err == NoError) {
		API_MDCLParameter par;	// parameter struct
		BNZeroMemory (&par, sizeof (API_MDCLParameter));
		par.name = "PCToImportLoc";
		par.type = MDCLPar_pointer;
		par.ptr_par = &pcToImportLoc;
		err = ACAPI_Goodies (APIAny_AddMDCLParameterID, parHdl, &par);

		if (err == NoError) {
			BNZeroMemory (&par, sizeof (API_MDCLParameter));
			par.name = "PCTargetLoc";
			par.type = MDCLPar_pointer;
			par.ptr_par = &pcTargetLoc;
			err = ACAPI_Goodies (APIAny_AddMDCLParameterID, parHdl, &par);
		}

		if (err == NoError) {
			API_Coord3D offset;
			err = ACAPI_Command_Call (&pcInMdid, 'PCIN', 1L, parHdl, (GSPtr)&offset, true);
			if (DBERROR (err != NoError)) {
				DGAlert (DG_INFORMATION, "ACAPI_Command_Call failed", "GSPC was not created.", "", "OK");
			} else {
				DGAlert (DG_INFORMATION,
						 "ACAPI_Command_Call succeeded",
						 "GSPC was created in \"resultGSPCs\" folder next to the e57 file.",
						 (GS::UniString ("Offsets:") +
						 GS::UniString ("  X:") + GS::ValueToUniString (offset.x) +
						 GS::UniString (", Y:") + GS::ValueToUniString (offset.y) +
						 GS::UniString (", Z:") + GS::ValueToUniString (offset.z)),
						 "OK");
			}
		}

		err = ACAPI_Goodies (APIAny_FreeMDCLParameterListID, &parHdl, NULL);
	}

	DBASSERT (err == NoError);

	return;
}		// Do_CallImportPointCloud


// -----------------------------------------------------------------------------
// DoCommand
//		called to perform the user-asked command
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	MenuCommandHandler (const API_MenuParams *params)
{
	switch (params->menuItemRef.itemIndex) {
		case 1:		InvertMenuItemMark (32500, 1);		break;
		case 2:		InvertMenuItemMark (32500, 2);		break;
		/* ----- */
		case 4:		Do_CMD1_Test ();					break;
		case 5:		Do_CMD1_Call ();					break;
		/* ----- */
		case 7:		Do_LinkToClient ();					break;
		/* ----- */
		case 9:		Do_Dump3DModel ();					break;
		/* ----- */
		case 11:	Do_CallExternalSaveAsDWG ();		break;
		case 12:	Do_CallOpenDWGToObject ();			break;
		/* ----- */
		case 14:	Do_CallImportPointCloud ();			break;
	}

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
	ACAPI_Register_Menu (32500, 0, MenuCode_UserDef, MenuFlag_Default);

	return NoError;
}		/* RegisterInterface */


// -----------------------------------------------------------------------------
// Initialize
//		called after the Add-On has been loaded into memory
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	Initialize (void)
{
	GSErrCode err = ACAPI_Install_MenuHandler (32500, MenuCommandHandler);
	if (err != NoError)
		DBPrintf ("Communication_Manager::Initialize() ACAPI_Install_MenuHandler failed\n");

	return err;
}	// Initialize


// -----------------------------------------------------------------------------
// FreeData
//		called when the Add-On is going to be unloaded
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	FreeData (void)
{
	return NoError;
}	// FreeData

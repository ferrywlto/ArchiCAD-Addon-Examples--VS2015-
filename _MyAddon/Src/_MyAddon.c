// *****************************************************************************
// Source code for the 3D Test Add-On
// API Development Kit 19; Mac/Win
//
// Namespaces:			Contact person:
//		-None-
//
// [SG compatible] - Yes
// *****************************************************************************
#include "APIEnvir.h"
// ---------------------------------- Includes ---------------------------------
#include "ACAPinc.h"					// also includes APIdefs.h
#include "APICommon.h"
#include "Location.hpp"
#include "ResourceIDs.hpp"
#include "DumpFunctions.hpp"
#include <string>
// =============================================================================
//
// Main functions
//
// =============================================================================


// -----------------------------------------------------------------------------
// Get the 3D representation of the clicked element
// -----------------------------------------------------------------------------

static void		Do_Get3DOfClickedElem (void)
{
	API_ElemTypeID			 typeID;
	API_Guid				 guid;
	API_ElemInfo3D 			 info3D;
	Int32					 ibody;
	bool					 detailed;
	GS::Array<API_Elem_Head> elems;
	API_Elem_Head			 elem;
	API_ElementMemo			 memo;
	API_Coord				 dbOffset;
	GSErrCode				 err = NoError;

	WriteReport ("# Get the 3D model of the clicked element");
	WriteReport ("#   - the 3D window don't have to be open or updated");
	WriteReport ("#   - only the first 5 bodies are Dumped in detail");

	BNZeroMemory (&elem, sizeof (API_Elem_Head));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

	if (!ClickAnElem ("Click an element to Dump its 3D representation", API_ZombieElemID, NULL, &typeID, &guid)) {
		WriteReport_Alert ("No element was clicked");
		return;
	}

	ACAPI_Database (APIDb_GetOffsetID, &dbOffset, NULL);
	if (typeID == API_CurtainWallID) {
		Int32 idx;
		bool isDegenerate;
		UInt32 mask = APIMemoMask_CWallFrames | APIMemoMask_CWallPanels | APIMemoMask_CWallJunctions | APIMemoMask_CWallAccessories;
		err = ACAPI_Element_GetMemo (guid, &memo, mask);
		if (err != NoError) {
			ErrorBeep ("ACAPI_Element_GetMemo", err);
			ACAPI_DisposeElemMemoHdls (&memo);
			return;
		}

		// frames
		GSSize nFrames = BMGetPtrSize (reinterpret_cast<GSPtr>(memo.cWallFrames)) / sizeof (API_CWallFrameType);
		for (idx = 0; idx < nFrames; ++idx) {
			err = ACAPI_Database (APIDb_IsCWPanelDegenerateID, &memo.cWallFrames[idx].head.guid, &isDegenerate);
			if (!isDegenerate) {
				elems.Push (memo.cWallFrames[idx].head);
			} else {
				GS::Guid guid = APIGuid2GSGuid (memo.cWallFrames[idx].head.guid);
				WriteReport ("Degenerate Frame: %s", (const char*) guid.ToUniString ().ToCStr ());
			}
		}

		// panels
		GSSize nPanels = BMGetPtrSize (reinterpret_cast<GSPtr>(memo.cWallPanels)) / sizeof (API_CWallPanelType);
		for (idx = 0; idx < nPanels; ++idx) {
			err = ACAPI_Database (APIDb_IsCWPanelDegenerateID, &memo.cWallPanels[idx].head.guid, &isDegenerate);
			if (!isDegenerate) {
				elems.Push (memo.cWallPanels[idx].head);
			} else {
				GS::Guid guid = APIGuid2GSGuid (memo.cWallPanels[idx].head.guid);
				WriteReport ("Degenerate Panel: %s", (const char*) guid.ToUniString ().ToCStr ());
			}
		}

		// junctions
		GSSize nJunctions = BMGetPtrSize (reinterpret_cast<GSPtr>(memo.cWallJunctions)) / sizeof (API_CWallJunctionType);
		for (idx = 0; idx < nJunctions; ++idx)
			elems.Push (memo.cWallJunctions[idx].head);

		// accessories
		GSSize nAccessories = BMGetPtrSize (reinterpret_cast<GSPtr>(memo.cWallAccessories)) / sizeof (API_CWallAccessoryType);
		for (idx = 0; idx < nAccessories; ++idx)
			elems.Push (memo.cWallAccessories[idx].head);

	} else {
		elem.guid = guid;
		if (ACAPI_Element_GetHeader (&elem) == NoError)
			elems.Push (elem);
	}

	for (GS::Array<API_Elem_Head>::ConstIterator it = elems.Enumerate (); it != NULL; ++it) {
		const API_Elem_Head& elemHead = *it;
		err = ACAPI_Element_Get3DInfo (elemHead, &info3D);
		if (err == APIERR_BADID) {
			WriteReport_Alert ("This element does not have 3D information.");
			return;
		}
		if (err != NoError) {
			ErrorBeep ("Error in ACAPI_Element_Get3DInfo", err);
			return;
		}

		WriteReport ("%s GUID:%s (3D model Dump) ", ElemID_To_Name (elemHead.typeID), APIGuidToString (elemHead.guid).ToCStr ().Get ());
		WriteReport ("  number of bodies: %d", info3D.lbody - info3D.fbody + 1);
		WriteReport ("  number of lights: %d", info3D.llight - info3D.flight + 1);
		WriteReport ("  bounding box: %5.2lf, %5.2lf, %5.2lf, %5.2lf", info3D.bounds.xMin, info3D.bounds.xMax,
						info3D.bounds.yMin, info3D.bounds.yMax);

		detailed = true;
		for (ibody = info3D.fbody; ibody <= info3D.lbody; ibody++) {
			// only the first 5 BODYs are written in detailed mode
			if (ibody == info3D.fbody + 5)
				detailed = false;
			DumpBody (ibody, detailed, dbOffset);
		}
	}

	WriteReport_End (err);

	ACAPI_DisposeElemMemoHdls (&memo);
}		// Do_Get3DOfClickedElem

// ----------------------------------------
// Test Code added by Ferry
// ----------------------------------------
#include "File.hpp"          // brings everything needed to use the File (e.g. Location)
#include "FileSystem.hpp"    // to query location of the running application

#include	"DGModule.hpp"
static void __ACENV_CALL OpenFileDialog()
{
	
	DG::FileDialog dlg(DG::FileDialog::OpenFile);
	//dlg.EnablePreview();

	//dlg.AddFilterGroupChildren(FTM::RootGroup);
	//dlg.AddFilter(FTM::UnknownType);               // Separator
	//dlg.AddFilterTypeChildren(FTM::RootGroup);
	//dlg.AddFilter(FTM::UnknownType);               // Separator
	//dlg.AddFilter(FTM::RootGroup);                 // All types

	//dlg.SelectFilter(1);

	dlg.SetTitle("Custom Open File");
	dlg.SetOKButtonText("Open!");
	dlg.SetCancelButtonText("Cancel!");
	IO::Location defaultLoc;
	IO::fileSystem.GetSpecialLocation(IO::FileSystem::ApplicationFolder, &defaultLoc);
	dlg.SetFolder(defaultLoc);
	//dlg.SelectFile(IO::Location("Text1.txt"));

	if (!dlg.Invoke())
		return;
	//int count = dlg.GetSelectionCount();
	//for (int n = 0; n < count; n++) {
	//	IO::File file(dlg.GetSelectedFile(n));
	//}
	IO::File file(dlg.GetSelectedFile(0));
	GSErrCode errorCode;    // holds error codes of operations

							// obtaining location of the file: application folder's location + name of the file

	//IO::Location fileLoc;    // Location instance
	//errorCode = IO::fileSystem.GetSpecialLocation(IO::FileSystem::ApplicationFolder, &fileLoc);
	//errorCode = fileLoc.AppendToLocal("Test.dat");

	// creating a File instance which will be used to operate on the desired OS file

	//IO::File file(fileLoc);

	// using the file

	errorCode = file.Open(IO::File::ReadMode);    // opening the file in read-only mode
	std::string msg1 = "ACAPI open file " + std::to_string((long long) errorCode);
	ACAPI_WriteReport(msg1.c_str(), false);
	WriteReport("open file: ");
	//
	UInt64 result;

	//file.GetAvailable(&result);
	//std::string str;
	//const long x = (long)(result);
	//
	//std::vector<int> z = std::vector<int>();
	//while(result > 256){
	//	char buffer[256];
	//	errorCode = file.ReadBin(buffer, result);
	//	WriteReport("read file: " + errorCode);
	//	// reading 16 bytes from the file into the buffer
	//	//z.
	//	WriteReport(buffer);
	//}
	errorCode = file.Close();
	std::string msg2 = "ACAPI close file " + std::to_string((long long)errorCode);
	ACAPI_WriteReport(msg2.c_str(), false);
	WriteReport("close file: " );

	//ACAPI_WriteReport(string, true) can pop up message in dialog, but false didnt write to session report
	//WriteReport(string) will actually write message into session report
	ACAPI_WriteReport("Hello World", false);
}

// -----------------------------------------------------------------------------
// Handles menu commands
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL MenuCommandHandler (const API_MenuParams *menuParams)
{
	switch (menuParams->menuItemRef.menuResID) {
		case ID_MENU_STRINGS:
			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		Do_Get3DOfClickedElem ();		break;
				case 2:		Do_Dump3DModel (false);			break;
				case 3:		OpenFileDialog();				break;
				case 4:		Do_DumpAllElement();			break;
				case 5:		Do_Save_As_Image();				break;
				case 6:		Do_Save_IfcFile();				break;
				case 7:		Do_Save_PdfFile();				break;
				case 8:		Do_CopyObjects();				break;
				case 9:		Do_CopyWalls();					break;
				case 10:	Do_CopySlabs();					break;
				case 11:	Do_CreatePipesAndWalls();		break;
				case 12:	Do_Call_Web();					break;
			}
			break;
	}

	return NoError;
}		// MenuCommandHandler

// -----------------------------------------------------------------------------
// Dump 3D Model (called from another add-on)
// -----------------------------------------------------------------------------
GSErrCode __ACENV_CALL	DumpCommandHandler (GSHandle parHdl, GSPtr resultData, bool silentMode)
{
UNUSED_PARAMETER (parHdl);
UNUSED_PARAMETER (resultData);
UNUSED_PARAMETER (silentMode);

	GSErrCode	err = Do_Dump3DModel (false);

	return err;
}	// DumpCommandHandler

// -----------------------------------------------------------------------------
// Dump the 3D model
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL IOCommandProc (const API_IOParams * /*ioParams*/)
{
	GSErrCode	err = Do_Dump3DModel (true);
	return err;
}		// IOCommandProc

// =============================================================================
//
// Required functions
//
// =============================================================================


// -----------------------------------------------------------------------------
// Dependency definitions
// -----------------------------------------------------------------------------

API_AddonType	__ACENV_CALL	CheckEnvironment (API_EnvirParams* envir)
{
	if (envir->serverInfo.serverApplication != APIAppl_ArchiCADID)
		return APIAddon_DontRegister;

	ACAPI_Resource_GetLocStr (envir->addOnInfo.name,		ID_ADDON_INFO, 1);
	ACAPI_Resource_GetLocStr (envir->addOnInfo.description,	ID_ADDON_INFO, 2);

	return APIAddon_Normal;
}		// CheckEnvironment


// -----------------------------------------------------------------------------
// Interface definitions
// -----------------------------------------------------------------------------

GSErrCode	__ACENV_CALL	RegisterInterface (void)
{
	GSErrCode err = NoError;
	err = ACAPI_Register_Menu (ID_MENU_STRINGS, 0, MenuCode_UserDef, MenuFlag_SeparatorBefore);
	if (err == NoError)
		err = ACAPI_Register_FileType (1, 'TEXT', 'GSAC', "txt;", 0, ID_FILETYPE_STRINGS, 1, SaveAs3DSupported);
	if (err == NoError)
		err = ACAPI_Register_SupportedService ('DUMP', 1L);
	return err;
}		// RegisterInterface


// -----------------------------------------------------------------------------
// Called when the Add-On has been loaded into memory
// to perform an operation
// -----------------------------------------------------------------------------

GSErrCode	__ACENV_CALL Initialize	(void)
{
	GSErrCode err = ACAPI_Install_MenuHandler (ID_MENU_STRINGS, MenuCommandHandler);
	if (err != NoError)
		DBPrintf ("3D_Test:: Initialize () ACAPI_Install_MenuHandler failed\n");

	err = ACAPI_Install_FileTypeHandler (1, IOCommandProc);
	if (err != NoError)
		DBPrintf ("3D_Test:: Initialize () ACAPI_Install_FileTypeHandler failed\n");

	err = ACAPI_Install_ModulCommandHandler ('DUMP', 1L, DumpCommandHandler);
	if (err != NoError)
		DBPrintf ("3D_Test:: Initialize () ACAPI_Install_ModulCommandHandler failed\n");


	return err;
}		// Initialize


// -----------------------------------------------------------------------------
// FreeData
//		called when the Add-On is going to be unloaded
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	FreeData (void)

{
	return NoError;
}		// FreeData

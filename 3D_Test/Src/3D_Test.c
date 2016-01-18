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

#include	"ACAPinc.h"					// also includes APIdefs.h
#include	"APICommon.h"

#include	"Location.hpp"

#include	"ResourceIDs.hpp"
#include <vector>


// -----------------------------------------------------------------------------
// Get a material with index
// -----------------------------------------------------------------------------

API_MaterialType	GetUMaterial (short iumat)
{
	static GS::HashTable<short, API_MaterialType> umatCache;

	API_Component3D	mat;
	BNZeroMemory (&mat, sizeof (mat));
	mat.header.typeID = API_UmatID;
	mat.header.index = iumat;
	if (umatCache.ContainsKey (iumat))
		return umatCache.Get (iumat);

	ACAPI_3D_GetComponent (&mat);
	delete mat.umat.mater.texture.fileLoc;
	mat.umat.mater.texture.fileLoc = NULL;

	umatCache.Add (iumat, mat.umat.mater);
	return mat.umat.mater;
}


// -----------------------------------------------------------------------------
// Dump a body (BODY, VERT, EDGE)
// -----------------------------------------------------------------------------

static void		DumpBody (Int32 ibody, bool detailed, const API_Coord& dbOffset)
{
	API_Component3D 	component;
	Int32				j, nVert, nEdge, nPgon;
	API_Tranmat			tm;
	API_UVCoord			resultUV;
	Int32				elemIdx, bodyIdx, ivect;
	GSErrCode			err;

	WriteReport ("BODY #%d", ibody);

	BNZeroMemory (&component, sizeof (component));

	component.header.typeID = API_BodyID;
	component.header.index  = ibody;
	err = ACAPI_3D_GetComponent (&component);
	if (err != NoError) {
		WriteReport ("    ERROR");
		return;
	}

	if (!detailed)
		return;

	nVert = component.body.nVert;
	nEdge = component.body.nEdge;
	nPgon = component.body.nPgon;
	tm = component.body.tranmat;
	elemIdx = component.body.head.elemIndex - 1;
	bodyIdx = component.body.head.bodyIndex - 1;

	WriteReport ("  VERTs");
	for (j = 1; j <= nVert; j++) {
		component.header.typeID = API_VertID;
		component.header.index  = j;
		err = ACAPI_3D_GetComponent (&component);
		if (err == NoError) {
			API_Coord3D	trCoord;	// world coordinates
			trCoord.x = tm.tmx[0]*component.vert.x + tm.tmx[1]*component.vert.y + tm.tmx[2]*component.vert.z + tm.tmx[3];
			trCoord.y = tm.tmx[4]*component.vert.x + tm.tmx[5]*component.vert.y + tm.tmx[6]*component.vert.z + tm.tmx[7];
			trCoord.z = tm.tmx[8]*component.vert.x + tm.tmx[9]*component.vert.y + tm.tmx[10]*component.vert.z + tm.tmx[11];
			trCoord.x += dbOffset.x;
			trCoord.y += dbOffset.y;
			WriteReport ("    [%2d]  (%lf, %lf, %lf)",
						 j,
						 trCoord.x, trCoord.y, trCoord.z);
		}
	}

	WriteReport ("  EDGEs");
	for (j = 1; j <= nEdge; j++) {
		component.header.typeID = API_EdgeID;
		component.header.index  = j;
		err = ACAPI_3D_GetComponent (&component);
		if (err == NoError)
			WriteReport ("    [%2d]  (%3d, %3d) - (%3d, %3d)",
						 j,
						 component.edge.vert1, component.edge.vert2,
						 component.edge.pgon1, component.edge.pgon2);
	}

	WriteReport ("  PGONs %ld %ld %ld", nPgon, elemIdx, bodyIdx);
	for (j = 1; j <= nPgon; j++) {
		component.header.typeID = API_PgonID;
		component.header.index  = j;
		err = ACAPI_3D_GetComponent (&component);
		if (err == NoError)
			WriteReport ("    [%2d]  (first edge: %3d, last edge: %3d, ivect: %3d)",
						 j,
						 component.pgon.fpedg, component.pgon.lpedg, component.pgon.ivect);

		API_MaterialType mat = GetUMaterial (component.pgon.iumat);
		WriteReport ("    MATERIAL %s [index: %d], guid: %s", mat.head.name, mat.head.index, APIGuidToString (mat.head.guid).ToCStr ().Get ());

		if (elemIdx < 0 || bodyIdx < 0) {
			WriteReport ("  No TEXTURE info");
			continue;
		}

		ivect = component.pgon.ivect;

		WriteReport ("    TEXTURE");

		API_TexCoordPars	pars = {0};
		pars.elemIdx = elemIdx;
		pars.bodyIdx = bodyIdx;
		pars.pgonIndex = j;
		//pars.filler_1;
		// get a coordinate for surface point
		component.header.typeID = API_PedgID;
		component.header.index  = component.pgon.fpedg;
		err = ACAPI_3D_GetComponent (&component);
		if (err == NoError) {
			component.header.typeID = API_EdgeID;
			component.header.index  = ::abs (component.pedg.pedg);
			err = ACAPI_3D_GetComponent (&component);
			if (err == NoError) {
				component.header.typeID = API_VertID;
				component.header.index  = ::abs (component.edge.vert1);
				err = ACAPI_3D_GetComponent (&component);
				if (err == NoError) {
					pars.surfacePoint.x = component.vert.x;
					pars.surfacePoint.y = component.vert.y;
					pars.surfacePoint.z = component.vert.z;
					err = ACAPI_Goodies (APIAny_GetTextureCoordID, &pars, &resultUV);
					if (err == NoError)
						WriteReport ("          (texture: (u,v) : (%5.2lf, %5.2lf)",
									 resultUV.u, resultUV.v);
				}
			}
		}

		WriteReport ("    NORMALVECTOR");
		component.header.typeID = API_VectID;
		component.header.index  = ::abs (ivect);
		err = ACAPI_3D_GetComponent (&component);
		if (err == NoError) {
			if (ivect < 0) {
				component.vect.x = -component.vect.x;
				component.vect.y = -component.vect.y;
				component.vect.z = -component.vect.z;
			}
			if (err == NoError) {
				API_Coord3D	trVect;	// world coordinates
				trVect.x = tm.tmx[0]*component.vect.x + tm.tmx[1]*component.vect.y + tm.tmx[2]*component.vect.z;
				trVect.y = tm.tmx[4]*component.vect.x + tm.tmx[5]*component.vect.y + tm.tmx[6]*component.vect.z;
				trVect.z = tm.tmx[8]*component.vect.x + tm.tmx[9]*component.vect.y + tm.tmx[10]*component.vect.z;
				WriteReport ("          [%2d]  (%lf, %lf, %lf)",
							 ivect,
							 trVect.x, trVect.y, trVect.z);
			}
		}

	}
	return;
}		// DumpBody

// -----------------------------------------------------------------------------
// Dump a light source
// -----------------------------------------------------------------------------

static void		DumpLight (Int32 ilght, bool detailed)
{
	API_Component3D 	component;
	GSErrCode			err;

	WriteReport ("LIGHT #%d", ilght);

	BNZeroMemory (&component, sizeof (component));

	component.header.typeID = API_LghtID;
	component.header.index  = ilght;
	err = ACAPI_3D_GetComponent (&component);
	if (err != NoError) {
		WriteReport ("    ERROR");
		return;
	}

	if (!detailed)
		return;

	API_LghtType	&lght = component.lght;
	switch (lght.type) {
		case APILght_DistantID:
			WriteReport ("  Distant");
			break;
		case APILght_DirectionID:
			WriteReport ("  Direction");
			break;
		case APILght_SpotID:
			WriteReport ("  Spot");
			break;
		case APILght_PointID:
			WriteReport ("  Point");
			break;
		case APILght_SunID:
			WriteReport ("  Sun");
			break;
		case APILght_EyeID:
			WriteReport ("  Eye");
			break;
	}

	WriteReport ("  status: %d", lght.status);
	WriteReport ("  color : (%5.2f, %5.2f, %5.2f)",	lght.lightRGB.f_red,
													lght.lightRGB.f_green,
													lght.lightRGB.f_blue);
	WriteReport ("  pos   : (%5.2f, %5.2f, %5.2f)",	lght.posx,
													lght.posy,
													lght.posz);
	WriteReport ("  dir   : (%5.2f, %5.2f, %5.2f)",	lght.dirx,
													lght.diry,
													lght.dirz);
	WriteReport ("  radius: %5.2f",	lght.radius);
	WriteReport ("  cosa  : %5.2f",	lght.cosa);
	WriteReport ("  cosb  : %5.2f",	lght.cosb);
	WriteReport ("  afall : %5.2f",	lght.afall);
	WriteReport ("  dists : %5.2f, %5.2f",	lght.dist1,
											lght.dist2);
	WriteReport ("  dfall : %5.2f",	lght.dfall);

	return;
}		// DumpLight


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

// -----------------------------------------------------------------------------
// Get the actual 3D model
// -----------------------------------------------------------------------------

static GSErrCode	Do_Dump3DModel (bool fromSaveAs)
{
	API_Coord	nullOffset = { 0.0, 0.0 };
	Int32		nbody, ibody, nlght, ilght;
	void		*origSight;
	bool		detailed;
	GSErrCode	err;

	WriteReport ("# Get the actual 3D model");
	WriteReport ("#   - the model will be not updated");

	WriteReport ("3D model Dump");
	if (!fromSaveAs) {		// we must switch to the 3D Window's sight
		err = ACAPI_3D_SelectSight (NULL, &origSight);
		if (err != NoError) {
			ErrorBeep ("Cannot switch to the 3D sight", err);
			return err;
		}
	}

	err = ACAPI_3D_GetNum (API_BodyID, &nbody);
	if (err != NoError) {
		ErrorBeep ("Error in ACAPI_3D_GetNum", err);
		if (!fromSaveAs)
			ACAPI_3D_SelectSight (origSight, &origSight);
		return err;
	}

	detailed = true;
	for (ibody = 1; ibody <= nbody; ibody++) {
		// only the first 5 BODY-s are written in detailed mode
		if (ibody == 5 && !fromSaveAs)
			detailed = false;
		DumpBody (ibody, detailed, nullOffset);
	}

	err = ACAPI_3D_GetNum (API_LghtID, &nlght);
	if (err != NoError) {
		ErrorBeep ("Error in ACAPI_3D_GetNum", err);
		if (!fromSaveAs)
			ACAPI_3D_SelectSight (origSight, &origSight);
		return err;
	}

	detailed = true;
	for (ilght = 1; ilght <= nlght; ilght++) {
		// only the first 5 BODY-s are written in detailed mode
		if (ilght == 5)
			detailed = false;
		DumpLight (ilght, detailed);
	}

	if (fromSaveAs) {
		WriteReport_Alert ("The output is written into the report window instead of the selected file.");
		// adjust return code to delete the empty output file
		err = APIERR_CANCEL;
	}

	if (!fromSaveAs)
		ACAPI_3D_SelectSight (origSight, &origSight);

	WriteReport_End (err);

	return err;
}		// Do_Get3DModel

// ----------------------------------------
// Test Code added by Ferry
// ----------------------------------------
#include "File.hpp"          // brings everything needed to use the File (e.g. Location)
#include "FileSystem.hpp"    // to query location of the running application

#include	"DGModule.hpp"
static void __ACENV_CALL Test()
{
	
	DG::FileDialog dlg(DG::FileDialog::OpenMultiFile);
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
	IO::fileSystem.GetSpecialLocation(IO::FileSystem::UserLocalSettings, &defaultLoc);
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
	WriteReport("open file: " + errorCode);
	
	UInt64 result;

	file.GetAvailable(&result);
	std::string str;
	const long x = (long)(result);
	
	std::vector<int> z = std::vector<int>();
	while(result > 256){
		char buffer[256];
		errorCode = file.ReadBin(buffer, result);
		WriteReport("read file: " + errorCode);
		// reading 16 bytes from the file into the buffer
		//z.
		WriteReport(buffer);
	}
	errorCode = file.Close();
	WriteReport("close file: " + errorCode);

	//ACAPI_WriteReport(string, true) can pop up message in dialog, but false didnt write to session report
	//WriteReport(string) will actually write message into session report
	ACAPI_WriteReport(
		"Hello World",
		false
		);
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
				case 3:		Test();							break;
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

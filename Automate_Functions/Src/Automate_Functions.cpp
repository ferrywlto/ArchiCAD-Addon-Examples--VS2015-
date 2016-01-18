// *****************************************************************************
// Description:		Source code for the Automate Functions Add-On
//
// Module:			API Development Kit 19; Mac/Win
// Namespace:		-none-
// Contact person:	devsup@graphisoft.hu
//
// SG compatible
// *****************************************************************************

#define	_AUTOMATE_FUNCTIONS_TRANSL_

// --- Includes ----------------------------------------------------------------

#include	"APIEnvir.h"

#include	<string.h>

#include	"GSRoot.hpp"
#include	"ACAPinc.h"					// also includes APIdefs.h

#include	"Location.hpp"
#include	"DGModule.hpp"

#include	"APICommon.h"


// =============================================================================
//
// Main functions
//
// =============================================================================

// -----------------------------------------------------------------------------
// Execute a New & Reset command
// -----------------------------------------------------------------------------

static void		Do_New (void)
{
	API_NewProjectPars	npp;
	GSErrCode			err;

	BNZeroMemory (&npp, sizeof (API_NewProjectPars));
	npp.newAndReset = true;
	npp.enableSaveAlert = false;

	err = ACAPI_Automate (APIDo_NewProjectID, &npp, NULL);
	if (err != NoError) {
		char msgStr[256];
		sprintf (msgStr, "Error in APIDo_NewPlanID: %d", (int) err);
		ACAPI_WriteReport (msgStr, true);
	}

	return;
}		// Do_New


// -----------------------------------------------------------------------------
// Open a project file
//	 - the file is hardcoded
// -----------------------------------------------------------------------------

static void		Do_Open (void)
{
	API_FileOpenPars	fop;
	GSErrCode			err;

	BNZeroMemory (&fop, sizeof (API_FileOpenPars));

	fop.fileTypeID = APIFType_PlanFile;
	fop.useStoredLib = true;

	IO::Location folderLoc;
	API_SpecFolderID specID = API_ApplicationFolderID;
	ACAPI_Environment (APIEnv_GetSpecFolderID, &specID, &folderLoc);

	fop.file = new IO::Location (folderLoc, IO::Name ("QuickTest.pla"));

	err = ACAPI_Automate (APIDo_OpenID, &fop, NULL);
	if (err != NoError) {
		char msgStr[256];
		sprintf (msgStr, "Error in APIDo_OpenID: %d", (int) err);
		ACAPI_WriteReport (msgStr, true);
	}

	delete fop.file;

	return;
}		// Do_Open


// -----------------------------------------------------------------------------
// Close the current project
// -----------------------------------------------------------------------------

static void		Do_Close (void)
{
	GSErrCode	err;

	err = ACAPI_Automate (APIDo_CloseID, (void *) (Int32) 1234, NULL);
	if (err != NoError) {
		char msgStr[256];
		sprintf (msgStr, "Error in APIDo_CloseID: %d", (int) err);
		ACAPI_WriteReport (msgStr, true);
	}

	return;
}		// Do_Close


// -----------------------------------------------------------------------------
// Save the current project
// -----------------------------------------------------------------------------

static void		Do_Save_Plan (void)
{
	GSErrCode	err;

	err = ACAPI_Automate (APIDo_SaveID, NULL, NULL);
	if (err != NoError) {
		char msgStr[256];
		sprintf (msgStr, "Error in APIDo_SaveID: %d", (int) err);
		ACAPI_WriteReport (msgStr, true);
	}

	return;
}		// Do_Save_Plan


// -----------------------------------------------------------------------------
// Save the current plan into a picture
// -----------------------------------------------------------------------------

static void		Do_Save_TiffFile (void)
{
	API_FileSavePars		fsp;
	API_SavePars_Picture	pars_pict;
	GSErrCode				err;

	BNZeroMemory (&fsp, sizeof (API_FileSavePars));
	fsp.fileTypeID = APIFType_TIFFFile;

	IO::Location folderLoc;
	API_SpecFolderID specID = API_UserDocumentsFolderID;
	ACAPI_Environment (APIEnv_GetSpecFolderID, &specID, &folderLoc);
	fsp.file = new IO::Location (folderLoc, IO::Name ("PictTest.tif"));

	BNZeroMemory (&pars_pict, sizeof (API_SavePars_Picture));
	pars_pict.colorDepth = APIColorDepth_256C;
	pars_pict.dithered					= false;
	pars_pict.view2D					= true;
	pars_pict.crop						= true;
	pars_pict.keepSelectionHighlight	= true;

	err = ACAPI_Automate (APIDo_SaveID, &fsp, &pars_pict);
	if (err != NoError) {
		char msgStr[256];
		sprintf (msgStr, "Error in APIDo_SaveID (pict): %d", (int) err);
		ACAPI_WriteReport (msgStr, true);
	}

	delete fsp.file;

	return;
}		// Do_Save_TiffFile


// -----------------------------------------------------------------------------
// Save the current plan to an IFC file
// -----------------------------------------------------------------------------

static void		Do_Save_IfcFile (void)
{
	API_FileSavePars	fsp;
	API_SavePars_Ifc	pars_ifc;
	GSErrCode			err;

	BNZeroMemory (&fsp, sizeof (API_FileSavePars));
	fsp.fileTypeID = APIFType_IfcFile;

	IO::Location folderLoc;
	API_SpecFolderID specID = API_UserDocumentsFolderID;
	ACAPI_Environment (APIEnv_GetSpecFolderID, &specID, &folderLoc);
	fsp.file = new IO::Location (folderLoc, IO::Name ("IfcTest.ifc"));

	BNZeroMemory (&pars_ifc, sizeof (pars_ifc));
	pars_ifc.subType = APIIfc_2x3;

	err = ACAPI_Automate (APIDo_SaveID, &fsp, &pars_ifc);
	if (err != NoError) {
		char msgStr[256];
		sprintf (msgStr, "Error in APIDo_SaveID (Ifc): %s", ErrID_To_Name (err));
		ACAPI_WriteReport (msgStr, true);
	}

	delete fsp.file;

	return;
}		// Do_Save_IfcFile


// -----------------------------------------------------------------------------
// Save the current plan to an Pdf file
// -----------------------------------------------------------------------------

static void		Do_Save_PdfFile (void)
{
	API_FileSavePars	fsp;
	GSErrCode			err;

	BNZeroMemory (&fsp, sizeof (API_FileSavePars));
	fsp.fileTypeID = APIFType_PdfFile;

	IO::Location folderLoc;
	API_SpecFolderID specID = API_UserDocumentsFolderID;
	ACAPI_Environment (APIEnv_GetSpecFolderID, &specID, &folderLoc);
	fsp.file = new IO::Location (folderLoc, IO::Name ("PdfTest.pdf"));

	err = ACAPI_Automate (APIDo_SaveID, &fsp, NULL);
	if (err != NoError) {
		char msgStr[256];
		sprintf (msgStr, "Error in APIDo_SaveID (Pdf): %s", ErrID_To_Name (err));
		ACAPI_WriteReport (msgStr, true);
	}

	delete fsp.file;

	return;
}		// Do_Save_PdfFile


// -----------------------------------------------------------------------------
// Load libraries
// -----------------------------------------------------------------------------

static void		Do_LoadLibraries (void)
{
	GS::Array<API_LibraryInfo>	lip;
	API_LibraryInfo				libInfo;
	GSErrCode					err;

	IO::Location appFolderLoc;
	API_SpecFolderID specID = API_ApplicationFolderID;
	ACAPI_Environment (APIEnv_GetSpecFolderID, &specID, &appFolderLoc);

	libInfo.location = IO::Location (appFolderLoc, IO::Name ("Library Examples"));
	lip.Push (libInfo);

	err = ACAPI_Automate (APIDo_LoadLibrariesID, &lip);
	if (err != NoError) {
		char msgStr[256];
		sprintf (msgStr, "Error in APIDo_LoadLibrariesID: %d", (int) err);
		ACAPI_WriteReport (msgStr, true);
	}

	return;
}		// Do_LoadLibraries


// -----------------------------------------------------------------------------
// Print the current view (window and zoom)
// -----------------------------------------------------------------------------

static void		Do_Print (void)
{
	API_PrintPars	pi;
	GSErrCode		err;

	BNZeroMemory (&pi, sizeof (API_PrintPars));

	pi.grid			= true;
	pi.printArea	= PrintArea_CurrentView;
	pi.fixText		= false;
	pi.scale		= 20;

	err = ACAPI_Automate (APIDo_PrintID, &pi, NULL);
	if (err != NoError) {
		char msgStr[256];
		sprintf (msgStr, "Error in APIDo_PrintID: %d", (int) err);
		ACAPI_WriteReport (msgStr, true);
	}

	return;
}		// Do_Print


// Show Selection/All In 3D
// -----------------------------------------------------------------------------

static void Do_Show3D (bool onlySelection)
{
	API_AutomateID code = onlySelection ? APIDo_ShowSelectionIn3DID : APIDo_ShowAllIn3DID;

	GSErrCode err = ACAPI_Automate (code, NULL, NULL);
	if (err != NoError) {
		char msgStr[256];
		sprintf (msgStr, "Error in %d: %d", code, (int) err);
		ACAPI_WriteReport (msgStr, true);
	}
}		// Do_Show3D


// -----------------------------------------------------------------------------
// Zoom In/Out
// -----------------------------------------------------------------------------

static void Do_Zoom (bool zoomIn)
{
	API_WindowInfo	currWindowInfo;
	BNZeroMemory (&currWindowInfo, sizeof (API_WindowInfo));
	GSErrCode err = ACAPI_Database (APIDb_GetCurrentWindowID, &currWindowInfo, NULL);
	if (err != NoError)
		return;

	API_Box box;
	BNZeroMemory (&box, sizeof (API_Box));
	API_Rect rect;
	BNZeroMemory (&rect, sizeof (API_Rect));

	if (currWindowInfo.typeID == APIWind_3DModelID) {
		API_3DWindowInfo api3DWindowInfo;
		BNZeroMemory (&api3DWindowInfo, sizeof (API_3DWindowInfo));
		ACAPI_Environment (APIEnv_Get3DWindowSetsID, &api3DWindowInfo, NULL);
		rect.right = api3DWindowInfo.hSize;
		rect.bottom = api3DWindowInfo.vSize;
		short inset = (zoomIn ? 50 : -50);
		rect.left = rect.left + inset;
		rect.top = rect.top + inset;
		rect.right = rect.right - inset;
		rect.bottom = rect.bottom - inset;
		err = ACAPI_Automate (APIDo_ZoomID, NULL, &rect);

	} else {
		API_Tranmat		tr;
		ACAPI_Database (APIDb_GetZoomID, &box, &tr);
		double ratio = (zoomIn ? 0.2 : -0.2);
		double dx = ratio * (box.xMax - box.xMin);
		double dy = ratio * (box.yMax - box.yMin);
		box.xMin += dx;
		box.yMin += dy;
		box.xMax -= dx;
		box.yMax -= dy;

		// rotate grid by 30 degrees
		if (zoomIn) {
			tr.tmx[0] = tr.tmx[5] = sqrt(3.0) / 2.0;
			tr.tmx[1] = -0.5;
			tr.tmx[4] = 0.5;
			err = ACAPI_Database (APIDb_SetZoomID, &box, &tr);
		} else
			err = ACAPI_Database (APIDb_SetZoomID, &box);
	}

	char msgStr[256];
	sprintf (msgStr, "Automate Functions - APIDo_ZoomID box(%.4f,%.4f,%.4f,%.4f), rect(%d,%d,%d,%d)%s",
			box.xMin, box.yMin, box.xMax, box.yMax, rect.left, rect.bottom, rect.right, rect.top,
			(err != NoError) ? " - FAILED" : "");
	ACAPI_WriteReport (msgStr, err != NoError);
}		// Do_Zoom


// -----------------------------------------------------------------------------
// Fit In Window
// -----------------------------------------------------------------------------

static void Do_FitInWindow (void)
{
	ACAPI_Automate (APIDo_ZoomID, NULL, NULL);
}		// Do_FitInWindow

static void Do_ZoomToSelected (void)
{
	ACAPI_Automate (APIDo_ZoomToSelectedID, NULL, NULL);
}


// -----------------------------------------------------------------------------
// Change the front window
// 	 - Section: switch to the plan and scan for a selected cutplane
// 	 - Section/3D windows will be rebuilt automatically if necessary
// -----------------------------------------------------------------------------

static void Do_SwitchToWindow (API_WindowTypeID typeID)
{
	API_WindowInfo	windInfo;
	GSErrCode		err;

	BNZeroMemory (&windInfo, sizeof (API_WindowInfo));
	windInfo.typeID = typeID;

	switch (typeID) {
		case APIWind_SectionID:
			{
				API_DatabaseInfo	origDB, planDB;
				API_Element			element;

				err = ACAPI_Database (APIDb_GetCurrentDatabaseID, &origDB, NULL);
				BNZeroMemory (&planDB, sizeof (API_DatabaseInfo));
				planDB.typeID = APIWind_FloorPlanID;
				err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &planDB, NULL);
				if (err == NoError) {
					API_SelectionInfo selectionInfo;
					API_Neig			**selNeigs;
					err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, false);
					if (err == APIERR_NOSEL || selectionInfo.typeID == API_SelEmpty) {
						ACAPI_WriteReport ("Nothing is selected", true);
						return;
					}

					if ((err != APIERR_NOSEL && err != NoError) || selNeigs == NULL) {
						char msgStr[256];
						sprintf (msgStr, "Error in ACAPI_Selection_GetInfo: %d", (int) err);
						ACAPI_WriteReport (msgStr, true);
						return;
					}

					for (Int32 i = 0; i < selectionInfo.sel_nElem; i++) {
						if ((*selNeigs)[i].neigID == APINeig_CutPlane || (*selNeigs)[i].neigID == APINeig_CutPlOn)
							BNZeroMemory (&element, sizeof (API_Element));
							element.header.guid  = (*selNeigs)[i].guid;
							err = ACAPI_Element_Get (&element);
							if (err == NoError) {
								windInfo.databaseUnId = element.cutPlane.segment.databaseID;
								break;
							}
					}
					if (windInfo.databaseUnId.elemSetId == APINULLGuid) {
						ACAPI_WriteReport ("# No cutplane selected - select one and try again", true);
						return;
					}
				}
				ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &origDB, NULL);
			}
			break;

		case APIWind_DetailID:
			{
				API_Element			element;

				API_SelectionInfo selectionInfo;
				API_Neig		  **selNeigs;
				err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, false);
				if (err == APIERR_NOSEL || selectionInfo.typeID == API_SelEmpty) {
					ACAPI_WriteReport ("Nothing is selected", true);
					return;
				}

				if ((err != APIERR_NOSEL && err != NoError) || selNeigs == NULL) {
					char msgStr[256];
					sprintf (msgStr, "Error in ACAPI_Selection_GetInfo: %d", (int) err);
					ACAPI_WriteReport (msgStr, true);
					return;
				}

				for (Int32 i = 0; i < selectionInfo.sel_nElem; i++) {
					if ((*selNeigs)[i].neigID == APINeig_Detail || (*selNeigs)[i].neigID == APINeig_DetailPoly ||
						(*selNeigs)[i].neigID == APINeig_DetailPolyOn || (*selNeigs)[i].neigID == APINeig_DetailMarker)
						BNZeroMemory (&element, sizeof (API_Element));
						element.header.guid  = (*selNeigs)[i].guid;
						err = ACAPI_Element_Get (&element);
						if (err == NoError) {
							windInfo.databaseUnId = element.detail.databaseID;
							break;
						}
				}
				if (windInfo.databaseUnId.elemSetId == APINULLGuid) {
					ACAPI_WriteReport ("# No detail selected - select one and try again", true);
					return;
				}
			}
			break;

		case APIWind_WorksheetID:
			{
				API_Element			element;

				API_SelectionInfo selectionInfo;
				API_Neig		  **selNeigs;
				err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, false);
				if (err == APIERR_NOSEL || selectionInfo.typeID == API_SelEmpty) {
					ACAPI_WriteReport ("Nothing is selected", true);
					return;
				}

				if ((err != APIERR_NOSEL && err != NoError) || selNeigs == NULL) {
					char msgStr[256];
					sprintf (msgStr, "Error in ACAPI_Selection_GetInfo: %d", (int) err);
					ACAPI_WriteReport (msgStr, true);
					return;
				}

				for (Int32 i = 0; i < selectionInfo.sel_nElem; i++) {
					if ((*selNeigs)[i].neigID == APINeig_Worksheet || (*selNeigs)[i].neigID == APINeig_WorksheetPoly ||
						(*selNeigs)[i].neigID == APINeig_WorksheetPolyOn || (*selNeigs)[i].neigID == APINeig_WorksheetMarker)
						BNZeroMemory (&element, sizeof (API_Element));
						element.header.guid  = (*selNeigs)[i].guid;
						err = ACAPI_Element_Get (&element);
						if (err == NoError) {
							windInfo.databaseUnId = element.worksheet.databaseID;
							break;
						}
				}
				if (windInfo.databaseUnId.elemSetId == APINULLGuid) {
					ACAPI_WriteReport ("# No worksheet selected - select one and try again", true);
					return;
				}
			}
			break;

		default:
			break;
	}

	err = ACAPI_Automate (APIDo_ChangeWindowID, &windInfo, NULL);

	return;
}		// Do_SwitchToWindow


////////////////////////////////////////////////////////////////////////////////
// To show db-list

class ListDbDialog :	public DG::ModalDialog,
						public DG::ButtonItemObserver
{
public:
	ListDbDialog (API_WindowTypeID typeID, API_DatabaseID code);
	~ListDbDialog ();

// Results after running modal dlg
public:
	// NULL=None
	API_DatabaseUnId* GetSelection ()
	{
		if (m_nListSelection < 0)
			return NULL;

		return m_pDatabases + m_nListSelection;
	}

	bool GetChangeDb ()
	{
		return m_bChangeDb;
	}

protected:
	DG::Dialog& GetReference ()
	{
		return *this;
	}

protected:
	virtual void	ButtonClicked (const DG::ButtonClickEvent& ev) override;

private:
	enum {
		OkButtonId = 1,
		CancelButtonId = 2,
		DbListId = 4,
		DbCheckId = 5
	};

	DG::Button				m_ok;
	DG::Button				m_cancel;
	DG::SingleSelListBox	m_list;
	DG::CheckBox			m_check;
	API_DatabaseUnId*		m_pDatabases;

private:
	int						m_nListSelection;
	bool					m_bChangeDb;
};


ListDbDialog::ListDbDialog (API_WindowTypeID typeID, API_DatabaseID code) : ModalDialog (ACAPI_GetOwnResModule (), 32505, InvalidResModule),
	m_ok	(GetReference (), OkButtonId),
	m_cancel(GetReference (), CancelButtonId),
	m_list	(GetReference (), DbListId),
	m_check	(GetReference (), DbCheckId),
	m_pDatabases (NULL),
	m_nListSelection (-1),
	m_bChangeDb (false)
{
	int					index, nDBs;
	GSErrCode           err;

	err = ACAPI_Database (code, (void *) &m_pDatabases, NULL);
	if (err == NoError && m_pDatabases != NULL && (nDBs = BMpGetSize ((GS::GSPtr) m_pDatabases) / Sizeof32(*m_pDatabases)) != 0) {
		for (index = 0; index < nDBs; index++) {
			API_DatabaseInfo	info;

			BNZeroMemory (&info, sizeof(info));
			info.typeID = typeID;
			info.databaseUnId = m_pDatabases[index];
			err = ACAPI_Database (APIDb_GetDatabaseInfoID, (void *) &info, NULL);
			if (DBERROR(err))
				continue;

			m_list.AppendItem ();
			//m_list.SetTabItemText (DG::ListBox::BottomItem, 1, info.title);
			GS::UniString	tmp (info.title);
			m_list.SetTabItemText (DG::ListBox::BottomItem, 1, tmp);
			m_list.SetItemValue (DG::ListBox::BottomItem, index);
		}
	}

	if (m_list.GetItemCount ())
		m_list.SelectItem (1);

	m_list.EnableSeparatorLines (true);

	m_ok.Attach (*this);
	m_cancel.Attach (*this);
}


ListDbDialog::~ListDbDialog ()
{
	BMpFree ((GS::GSPtr) m_pDatabases);

	m_ok.Detach (*this);
	m_cancel.Detach (*this);
}


void ListDbDialog::ButtonClicked (const DG::ButtonClickEvent& ev)
{
	if (ev.GetSource () == &m_ok) {
		// Save results
		short	sel;

		sel = m_list.GetSelectedItem ();
		if (sel >= 1)
			m_nListSelection = TruncateTo32Bit (m_list.GetItemValue (sel));

		m_bChangeDb = m_check.IsChecked ();

		PostCloseRequest (Accept);
	} else if (ev.GetSource () == &m_cancel)
		PostCloseRequest (Cancel);
}


// -----------------------------------------------------------------------------
// Shows list of databases and changes the front window.
// Works with sections, elevations, interior elevations, details, layouts and
//  master layouts
// -----------------------------------------------------------------------------

static void Do_EnumerateAndSwitchToWindow (API_DatabaseTypeID typeID)
{
	API_DatabaseID		code;
	GSErrCode           err;

	switch (typeID) {
	case APIWind_SectionID:
		code = APIDb_GetSectionDatabasesID;
		break;

	case APIWind_ElevationID:
		code = APIDb_GetElevationDatabasesID;
		break;

	case APIWind_InteriorElevationID:
		code = APIDb_GetInteriorElevationDatabasesID;
		break;

	case APIWind_DetailID:
		code = APIDb_GetDetailDatabasesID;
		break;

	case APIWind_WorksheetID:
		code = APIDb_GetWorksheetDatabasesID;
		break;

	case APIWind_LayoutID:
		code = APIDb_GetLayoutDatabasesID;
		break;

	case APIWind_MasterLayoutID:
		code = APIDb_GetMasterLayoutDatabasesID;
		break;

	default:
		DBBREAK ();
		return;
	}

	ListDbDialog	dlg (typeID, code);

	if (!dlg.Invoke ())
		return;

	API_DatabaseUnId*	pDbUnId = dlg.GetSelection ();

	if (pDbUnId == NULL)
		return;

	API_DatabaseInfo	prevDbInfo;

	ACAPI_Database (APIDb_GetCurrentDatabaseID, &prevDbInfo, NULL);

	// Activate window or just db in background
	if (!dlg.GetChangeDb ()) {
		API_WindowInfo	windInfo;

		BNZeroMemory (&windInfo, sizeof (windInfo));
		windInfo.typeID = typeID;
		windInfo.databaseUnId = *pDbUnId;

		err = ACAPI_Automate (APIDo_ChangeWindowID, &windInfo, NULL);
	} else {
		API_DatabaseInfo	dbInfo;

		BNZeroMemory (&dbInfo, sizeof (dbInfo));
		dbInfo.typeID = typeID;
		dbInfo.databaseUnId = *pDbUnId;

		err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &dbInfo, NULL);
	}

	if (DBERROR (err != 0))
		return;

	// Dump number of labels
	char	buf[256];

	GS::Array<API_Guid> labelList;
	ACAPI_Element_GetElemList (API_LabelID, &labelList);
	sprintf (buf, "Number of labels in active db=%d", (int) labelList.GetSize ());
	ACAPI_WriteReport (buf, false);

	// Switch back if we changed here
	if (dlg.GetChangeDb ())
		ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &prevDbInfo, NULL);
}

// To show db-list
////////////////////////////////////////////////////////////////////////////////



// -----------------------------------------------------------------------------
// Menu command handler
// -----------------------------------------------------------------------------

static GSErrCode __ACENV_CALL	MenuCommandProc (const API_MenuParams* menuParams)
{
	GSErrCode	err = NoError;

	DBPrintf ("Automate Functions Add-On User Menu called, ResID: %d, Item No.: %d\n",
				menuParams->menuItemRef.menuResID, menuParams->menuItemRef.itemIndex);

	switch (menuParams->menuItemRef.itemIndex) {
		case  1:	Do_New ();									break;
		case  2:	Do_Open ();									break;
		case  3:	Do_Close ();								break;
		case  4:	Do_Save_Plan ();							break;
		case  5:	Do_Save_TiffFile ();						break;
		case  6:	Do_Save_IfcFile ();							break;
		case  7:	Do_Save_PdfFile ();							break;
		case  8:	Do_LoadLibraries ();						break;
		case  9:	Do_Print ();								break;
		// -----
		case 11:	Do_Zoom (true);								break;
		case 12:	Do_Zoom (false);							break;
		case 13:	Do_ZoomToSelected();						break;
		case 14:	Do_FitInWindow ();							break;
		// ---
		case 16:	Do_Show3D (true);							break;
		case 17:	Do_Show3D (false);							break;
		// -----
		case 19:	Do_SwitchToWindow (APIWind_FloorPlanID);	break;
		case 20:	Do_SwitchToWindow (APIWind_SectionID);		break;
		case 21:	Do_SwitchToWindow (APIWind_DetailID);		break;
		case 22:	Do_SwitchToWindow (APIWind_3DModelID);		break;
		// -----
		case 24:	Do_EnumerateAndSwitchToWindow (APIWind_SectionID);				break;
		case 25:	Do_EnumerateAndSwitchToWindow (APIWind_ElevationID);			break;
		case 26:	Do_EnumerateAndSwitchToWindow (APIWind_InteriorElevationID);	break;
		case 27:	Do_EnumerateAndSwitchToWindow (APIWind_DetailID);				break;
		case 28:	Do_EnumerateAndSwitchToWindow (APIWind_MasterLayoutID);			break;
		case 29:	Do_EnumerateAndSwitchToWindow (APIWind_LayoutID);				break;
	}

	return err;
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
// Called when the Add-On is going to be registered
// -----------------------------------------------------------------------------

API_AddonType	__ACENV_CALL	CheckEnvironment (API_EnvirParams* envirParams)
{
	if (envirParams->serverInfo.serverApplication != APIAppl_ArchiCADID)
		return APIAddon_DontRegister;

	ACAPI_Resource_GetLocStr (envirParams->addOnInfo.name, 32000, 1);
	ACAPI_Resource_GetLocStr (envirParams->addOnInfo.description, 32000, 2);

	return APIAddon_Normal;
}


// -----------------------------------------------------------------------------
// Interface definitions
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	RegisterInterface (void)
{
	GSErrCode	err = NoError;

	// Register menus
	err = ACAPI_Register_Menu (32500, 32501, MenuCode_UserDef, MenuFlag_SeparatorBefore); //or MenuFlag_Default

	return err;
}


// -----------------------------------------------------------------------------
// Initialize
//		called after the Add-On has been loaded into memory
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	Initialize (void)
{
	ACAPI_Install_MenuHandler (32500, MenuCommandProc);

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

/**
 * @file Implements the "Find element by GUID" command.
 */

#include "APIOutputFramework_Test.hpp"
#include "FindByGuid.hpp"
#include "BFSTreeSelector.hpp"
#include "DatabaseGraph.hpp"
#include <math.h>

#if defined (_MSC_VER)
	#define snprintf _snprintf
#else
	// MSL and BSD Has snprintf, and vsnprintf in :: scope
#endif

FindDialog::FindDialog (const GS::Array<API_DatabaseInfo>& inDbList, ULong& inDbIndex, API_Guid& inRequestedGuid) :
	DG::ModalDialog (ACAPI_GetOwnResModule (), dialogResId, InvalidResModule),

	databaseList	(inDbList),
	databaseIndex	(inDbIndex),
	requestedGuid	(inRequestedGuid),

	okButton		(GetReference (), OkButtonId),
	cancelButton	(GetReference (), CancelButtonId),
	databasePopup	(GetReference (), DatabasePopupId),
	guidStaticText	(GetReference (), GuidStaticTextId),
	guidEdit		(GetReference (), GuidEditId),
	separator		(GetReference (), SeparatorId)
{
	Attach (*this);
	okButton.Attach (*this);
	cancelButton.Attach (*this);

	FillDatabasePopup ();
}

FindDialog::~FindDialog ()
{
	okButton.Detach (*this);
	cancelButton.Detach (*this);
	Detach (*this);
}

void FindDialog::FillDatabasePopup ()
{
	short         floorplanIndex = 0;
	UInt32 dbCount        = databaseList.GetSize ();

	databasePopup.DeleteItem (DG::PopUp::AllItems);
	for (unsigned short index = 0; index < dbCount; index++) {
		databasePopup.AppendItem ();

		char buffer[256];
		BNZeroMemory (buffer, sizeof (buffer));
		RSGetIndString (buffer, 32505, databaseList[index].typeID + 1, ACAPI_GetOwnResModule ());

		UInt32 typeLength = Strlen32 (buffer);
#if defined (WINDOWS)
		::_snprintf_s (buffer + typeLength, sizeof (buffer) - 1 - typeLength, _TRUNCATE,
						"  %s  %s",
						APIGuidToString (databaseList[index].databaseUnId.elemSetId).ToCStr ().Get(),
						(const char *) GS::UniString (databaseList[index].title).ToCStr ());
#else
		::snprintf (buffer + typeLength, sizeof (buffer) - 1 - typeLength, "  %s %s",
					APIGuidToString (databaseList[index].databaseUnId.elemSetId).ToCStr ().Get(),
					(const char *) GS::UniString (databaseList[index].title).ToCStr ());
#endif

		databasePopup.SetItemText (DG::PopUp::BottomItem, buffer);
		databasePopup.SetItemValue (DG::PopUp::BottomItem, index);

		if (databaseList[index].typeID == APIWind_FloorPlanID) {
			floorplanIndex = index;
		}
	}
	databasePopup.SelectItem (floorplanIndex + 1);
}

void FindDialog::ButtonClicked (const DG::ButtonClickEvent& ev)
{
	if (ev.GetSource () == &okButton) {
		PostCloseRequest (DG::ModalDialog::Accept);
	} else if (ev.GetSource () == &cancelButton) {
		PostCloseRequest (DG::ModalDialog::Cancel);
	}
}


void FindDialog::PanelClosed (const DG::PanelCloseEvent& ev)
{
	if (ev.IsAccepted ()) {
		databaseIndex = TruncateTo32Bit (databasePopup.GetItemValue (databasePopup.GetSelectedItem ()));

		GS::Guid guid;
		if (guid.ConvertFromString (guidEdit.GetText ()) == NoError) {
			requestedGuid = GSGuid2APIGuid (guid);
		} else {
			requestedGuid = APINULLGuid;
		}
	}
}


/**
* Finds an element by its GUID.
* Switches to the database, selects the element, and zooms to it.
* @return error code if fails.
*/
GSErrCode FindDialog::FindElemByGuid (void)
{
	GSErrCode                   err           = NoError;
	GS::Array<API_DatabaseInfo>	dbList        = GSAPI::DatabaseGraph::Instance ().SelectDatabases (GSAPI::BFSTreeSelector ());
	API_Guid                    requestedGuid = APINULLGuid;
	ULong                       dbIndex       = 0;

	//	get the unique ID from the user
	FindDialog findDialog (dbList, dbIndex, requestedGuid);
	if (findDialog.Invoke ()) {
		// select the element
		err = SelectAndZoom (dbList[dbIndex], requestedGuid);
	}
	return err;
}


/**
* Select and zoom to the element identified by its GUID.
* Selects the element, and zooms to it.
*/
GSErrCode FindDialog::SelectAndZoom (API_DatabaseInfo homeDB, const API_Guid& requestedGuid)
{
	GSErrCode error = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, reinterpret_cast<void *> (&homeDB), NULL);
	if (error != NoError) {
		return error;
	}

	API_Element	selElem;

	BNZeroMemory (&selElem, sizeof (selElem));
	selElem.header.guid = requestedGuid;
	error = ACAPI_Element_Get (&selElem);
	if (error != NoError) {
		return error;
	}

	if (homeDB.typeID == APIWind_FloorPlanID) {
		// switch to the element's story
		API_StoryCmdType	storyCmd;

		BNZeroMemory (&storyCmd, sizeof (storyCmd));
		storyCmd.index  = selElem.header.floorInd;
		storyCmd.action = APIStory_GoTo;
		error = ACAPI_Environment (APIEnv_ChangeStorySettingsID, &storyCmd, NULL);
		if (error != NoError) {
			return error;
		}
	}

	// calculate element bounds for zooming
	API_Box3D	box;

	BNZeroMemory (&box, sizeof (box));
	error = ACAPI_Database (APIDb_CalcBoundsID, &selElem.header, &box);
	if (error != NoError) {
		return error;
	}

	const double MarginMult = 1.05;
	API_Box		 zoomBox;

	BNZeroMemory (&zoomBox, sizeof (zoomBox));
	zoomBox.xMin = box.xMin - MarginMult * (box.xMax - box.xMin);
	zoomBox.xMax = box.xMax + MarginMult * (box.xMax - box.xMin);
	zoomBox.yMin = box.yMin - MarginMult * (box.yMax - box.yMin);
	zoomBox.yMax = box.yMax + MarginMult * (box.yMax - box.yMin);

	// and zoom
	if (fabs (zoomBox.xMax - zoomBox.xMin) > 1E-5 && fabs (zoomBox.yMax - zoomBox.yMin) > 1E-5) {
		error = ACAPI_Automate (APIDo_ZoomID, &zoomBox, NULL);
		if (error != NoError) {
			return error;
		}
	}

	// select the element
	API_Neig **selNeig = reinterpret_cast<API_Neig**> (BMAllocateHandle (sizeof (API_Neig), ALLOCATE_CLEAR, 0));

	ACAPI_Goodies (APIAny_ElemTypeToNeigID, (void *) (GS::IntPtr) selElem.header.typeID, &(*selNeig)->neigID);
	(*selNeig)->guid  = selElem.header.guid;
	error = ACAPI_Element_Select (selNeig, 1, true);
	BMKillHandle (reinterpret_cast<GSHandle*> (&selNeig));

	return error;
}

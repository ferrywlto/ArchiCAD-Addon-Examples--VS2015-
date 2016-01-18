// *****************************************************************************
// Source code for the BuildingMaterial Dialog in DG Test Add-On
// API Development Kit 19; Mac/Win
//
// Namespaces:			Contact person:
//		-None-						LT
//
// [SG compatible] - Yes
// *****************************************************************************

#include	"BuildingMaterialDialog.hpp"
#include	"DrawDialogItems.hpp"


//---------------------------- Class BuildingMaterialDialog -----------------------

BuildingMaterialDialog::BuildingMaterialDialog (GSResModule dialResModule, short resId):
	DG::ModalDialog		(dialResModule, resId, dialResModule),

	buildMatList		(GetReference (), ListBoxId),
	buildingMatControl	(GetReference (), BuildingMatControlId),
	penControl			(GetReference (), PenControlId),
	okButton			(GetReference (), OkButtonId)
{
	API_UCCallbackType	ucb;
 
	// initialize building material user controls
	BNZeroMemory (&ucb, sizeof (ucb));
	ucb.dialogID = resId;
	ucb.type     = APIUserControlType_BuildingMaterial;
	ucb.itemID   = BuildingMatControlId;
	ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
	buildingMatControl.Hide ();

	// initialize pen user controls
	BNZeroMemory (&ucb, sizeof (ucb));
	ucb.dialogID = resId;
	ucb.type     = APIUserControlType_Pen;
	ucb.itemID   = PenControlId;
	ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
	penControl.Hide ();

	const short width = buildMatList.GetItemWidth ();
	const short	PenControlTab_SIZE			= penControl.GetWidth ();
	const short	PenIndexTab_SIZE			= PenControlTab_SIZE;
	const short	BuildingMatControlTab_SIZE	= buildingMatControl.GetWidth ();
	const short	BuildingMatDrawTab_SIZE		= 50;
	const short	BuildingMatNameTab_SIZE		= width - BuildingMatDrawTab_SIZE - BuildingMatControlTab_SIZE - PenIndexTab_SIZE - PenControlTab_SIZE;

	// initialize the listbox
	buildMatList.SetTabFieldCount (NTabFields);
	buildMatList.SetHeaderSynchronState (false);

	buildMatList.SetHeaderItemSize (BuildingMatDrawTab,		0);
	buildMatList.SetHeaderItemSize (BuildingMatNameTab,		buildMatList.GetItemWidth () - penControl.GetWidth () * 2);
	buildMatList.SetHeaderItemSize (BuildingMatControlTab,	0);
	buildMatList.SetHeaderItemSize (PenIndexTab,			penControl.GetWidth () * 2);
	buildMatList.SetHeaderItemSize (PenControlTab,			0);

	short pos = 0;
	buildMatList.SetTabFieldProperties (BuildingMatDrawTab,		pos, pos + BuildingMatDrawTab_SIZE,		DG::ListBox::Center,	DG::ListBox::NoTruncate, false);
	pos += BuildingMatDrawTab_SIZE;
	buildMatList.SetTabFieldProperties (BuildingMatNameTab,		pos, pos + BuildingMatNameTab_SIZE,		DG::ListBox::Left,		DG::ListBox::NoTruncate, false);
	pos += BuildingMatNameTab_SIZE;
	buildMatList.SetTabFieldProperties (BuildingMatControlTab,	pos, pos + BuildingMatControlTab_SIZE,	DG::ListBox::Center,	DG::ListBox::EndTruncate, true);
	pos += BuildingMatControlTab_SIZE;
	buildMatList.SetTabFieldProperties (PenIndexTab,			pos, pos + PenIndexTab_SIZE,			DG::ListBox::Left,		DG::ListBox::NoTruncate, false);
	pos += PenIndexTab_SIZE;
	buildMatList.SetTabFieldProperties (PenControlTab,			pos, width,								DG::ListBox::Center,	DG::ListBox::EndTruncate, false);

	// set texts:
	buildMatList.SetHeaderItemText (BuildingMatNameTab,	"Building Material");
	buildMatList.SetHeaderItemText (PenIndexTab,		"cutFillPen");

	// to catch ListBoxTabFieldUpdate event:
	buildMatList.SetTabFieldOwnerDrawFlag (PenControlTab,		true);
	buildMatList.SetTabFieldOwnerDrawFlag (BuildingMatDrawTab,	true);

	const short maxBuildingMat = 50;
	short nBuildingMat = maxBuildingMat;
	ACAPI_Attribute_GetNum (API_BuildingMaterialID, &nBuildingMat);
	if (nBuildingMat > maxBuildingMat)
		nBuildingMat = maxBuildingMat; // add only the first 50 building materials to the listbox

	for (short i = 1; i <= nBuildingMat; i++) {
		buildMatList.InsertItem (i);

		API_Attribute attr;
		Attribute_Get (&attr, API_BuildingMaterialID, i);
		buildMatIndexList.Push (i);
		penIndexList.Push (attr.buildingMaterial.cutFillPen);

		buildMatList.SetTabItemText (i, BuildingMatNameTab, attr.header.name);
		buildMatList.SetTabItemText (i, PenIndexTab, GS::UniString::Printf ("%d", attr.buildingMaterial.cutFillPen));
	}
}


BuildingMaterialDialog::~BuildingMaterialDialog ()
{
}


//-------------------------- Class BuildingMaterialObserver -----------------------

BuildingMaterialObserver::BuildingMaterialObserver (BuildingMaterialDialog* testDialog):
	dialog 	 (testDialog)
{
	dialog->Attach (*this);
	AttachToAllItems (*dialog);
}


BuildingMaterialObserver::~BuildingMaterialObserver ()
{
	dialog->Detach (*this);
	DetachFromAllItems (*dialog);
}


void BuildingMaterialObserver::PanelOpened (const DG::PanelOpenEvent& /*ev*/)
{
	dialog->SetClientSize (dialog->GetOriginalClientWidth (), dialog->GetOriginalClientHeight ());
}


void BuildingMaterialObserver::ButtonClicked (const DG::ButtonClickEvent& ev)
{
	if (ev.GetSource () == &dialog->okButton) {
		dialog->PostCloseRequest (DG::ModalDialog::Accept);
	}
}


void BuildingMaterialObserver::UserControlChanged (const DG::UserControlChangeEvent& ev)
{
	short listItemIndex = dialog->buildMatList.GetSelectedItem ();

	if (ev.GetSource () == &dialog->buildingMatControl) {
		Int32 fIndex = dialog->buildingMatControl.GetValue ();

		API_Attribute attr;
		Attribute_Get (&attr, API_BuildingMaterialID, (short) fIndex);

		dialog->buildMatIndexList[listItemIndex - 1] = attr.header.index;
		dialog->penIndexList[listItemIndex - 1] = attr.buildingMaterial.cutFillPen;

		dialog->buildMatList.SetTabItemText (listItemIndex, dialog->BuildingMatNameTab, attr.header.name);
		// change pen control
		dialog->penControl.SetValue (dialog->penIndexList[listItemIndex - 1]);
	}
	else if (ev.GetSource () == &dialog->penControl) {
		Int32 fIndex = dialog->penControl.GetValue ();
		dialog->penIndexList[listItemIndex - 1] = (short) fIndex;

		API_Attribute attr;
		Attribute_Get (&attr, API_BuildingMaterialID, dialog->buildMatIndexList[listItemIndex - 1]);
		attr.buildingMaterial.cutFillPen = (short) fIndex;
		ACAPI_Attribute_Modify (&attr, NULL);
	}

	dialog->buildMatList.SetTabItemText (listItemIndex, dialog->PenIndexTab, GS::UniString::Printf ("%d", dialog->penIndexList[listItemIndex - 1]));

	dialog->buildMatList.RedrawTabItem (listItemIndex, BuildingMaterialDialog::BuildingMatDrawTab);
	dialog->buildMatList.RedrawTabItem (listItemIndex, BuildingMaterialDialog::PenControlTab);
}


void BuildingMaterialObserver::ListBoxSelectionChanged (const DG::ListBoxSelectionEvent& ev)
{
	if (ev.GetSource () == &dialog->buildMatList) {
		short listItemIndex = dialog->buildMatList.GetSelectedItem ();

		if (listItemIndex <= dialog->buildMatList.GetItemCount () && listItemIndex > 0) {
			// change building material control
			dialog->buildingMatControl.SetValue (dialog->buildMatIndexList[listItemIndex - 1]);
			// change pen control
			dialog->penControl.SetValue (dialog->penIndexList[listItemIndex - 1]);
			// show controls
			dialog->buildMatList.SetOnTabItem (BuildingMaterialDialog::BuildingMatControlTab,	dialog->buildingMatControl);
			dialog->buildMatList.SetOnTabItem (BuildingMaterialDialog::PenControlTab,			dialog->penControl);
		}
	}
}


void BuildingMaterialObserver::UpdateBuildingMatDrawTab (const DG::ListBoxTabItemUpdateEvent& ev)
{
	short item = ev.GetListItem ();
	if (item < 1)
		return;

	NewDisplay::ListBoxUpdateEventContext context (ev);

	short width   = ev.GetWidth ();
	short height  = ev.GetHeight ();

	DrawBuildingMaterial (context, DG::Rect (1, 1, width - 1, height - 1), dialog->buildMatIndexList[item - 1]);
}


void BuildingMaterialObserver::PanelResized (const DG::PanelResizeEvent& ev)
{
	short vGrow = ev.GetVerticalChange ();
	if (vGrow != 0) {
		dialog->BeginMoveResizeItems ();

		dialog->okButton.Move			(0, vGrow);
		dialog->penControl.Move			(0, vGrow);
		dialog->buildingMatControl.Move	(0, vGrow);
		dialog->buildMatList.Resize		(0, vGrow);

		dialog->EndMoveResizeItems ();
	}
}


void BuildingMaterialObserver::UpdatePenControlTab (const DG::ListBoxTabItemUpdateEvent& ev)
{
	short item = ev.GetListItem ();
	if (item < 1)
		return;

	NewDisplay::ListBoxUpdateEventContext context (ev);

	short width   = ev.GetWidth ();
	short height  = ev.GetHeight ();

	DrawPen (context, DG::Rect (2, 2, width - 2, height - 2), dialog->penIndexList[item - 1]);
}


void BuildingMaterialObserver::ListBoxTabFieldUpdate (const DG::ListBoxTabItemUpdateEvent& ev)
{
	short item = ev.GetListItem ();
	if (item < 1)
		return;

	if (ev.GetSource () == &dialog->buildMatList) {
		switch (ev.GetTabFieldIndex ()) {
			case BuildingMaterialDialog::BuildingMatDrawTab:
				UpdateBuildingMatDrawTab (ev);
				break;
			case BuildingMaterialDialog::PenControlTab:
				UpdatePenControlTab (ev);
				break;
			default:
				break;
		}
	}
}
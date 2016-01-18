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


namespace PanelTest {

// --- Class definitions -------------------------------------------------------

// --- TestSettingsPage --------------------------------------------------------

TestSettingsPage::TestSettingsPage (const DG::TabControl& tabControl, TBUI::IAPIToolUIData* puiData):
	DG::TabPage (tabControl, 1, ACAPI_GetOwnResModule (), SettingsPageId, InvalidResModule),

	nameTextEdit			(GetReference (), NameTextEditId),
	uValueRealEdit			(GetReference (), UValueRealEditId),
	oldHeightLengthEdit		(GetReference (), OldHeightLengthEditId),
	newHeightLengthEdit		(GetReference (), NewHeightLengthEditId),

	uiData (puiData)
{
}


TestSettingsPage::~TestSettingsPage (void)
{
	uiData = NULL;
}


// --- TestSettingsPageObserver ------------------------------------------------

TestSettingsPageObserver::TestSettingsPageObserver (TestSettingsPage* testPage):
	tabPage (testPage)
{
	tabPage->Attach (*this);
	AttachToAllItems (*tabPage);
	tabPage->uiData->AttachObserver (this);

	BNZeroMemory (&previousElem, sizeof(API_Element));
}


TestSettingsPageObserver::~TestSettingsPageObserver (void)
{
	tabPage->Detach (*this);
	DetachFromAllItems (*tabPage);
	tabPage->uiData->DetachObserver (this);
	tabPage = NULL;
}


// ... Utilitiy functions ......................................................

void	TestSettingsPageObserver::GetUserDataFromDefault (WallUserData* data)
{
	API_ElementUserData userData;
	BNZeroMemory (&userData, sizeof (API_ElementUserData));
	tabPage->uiData->GetUserData (&userData);

	// allocating area if needed
	if (userData.dataHdl == NULL) {
		userData.dataHdl = BMAllocateHandle (sizeof(WallUserData), ALLOCATE_CLEAR, 0);
		if (userData.dataHdl == NULL)
			return;			//!!! error!!!
	}

	// gets pointer
	WallUserData* sdata = reinterpret_cast<WallUserData*>(*userData.dataHdl);

	// copies from the data
	CHCopyC (sdata->name, data->name);
	data->uValue = sdata->uValue;

	// kill
	BMKillHandle (&userData.dataHdl);
}


void	TestSettingsPageObserver::SetUserDataToDefault (WallUserData* sdata)
{
	API_ElementUserData userData;
	BNZeroMemory (&userData, sizeof (API_ElementUserData));

	// allocating
	userData.dataHdl = BMAllocateHandle (sizeof(WallUserData), ALLOCATE_CLEAR, 0);
	if (userData.dataHdl == NULL)
		return;			//!!! error!!!

	userData.flags = APIUserDataFlag_FillWith | APIUserDataFlag_Pickup;

	// gets pointer
	WallUserData* data = reinterpret_cast<WallUserData*>(*userData.dataHdl);

	// copies to the data
	CHCopyC (sdata->name, data->name);
	data->uValue = sdata->uValue;

	// sets to the default
	tabPage->uiData->SetUserData (&userData);

	BMKillHandle (&userData.dataHdl);
}


// ... TabPage notifications ...................................................

void	TestSettingsPageObserver::PanelOpened (const DG::PanelOpenEvent& /*ev*/)
{
	// initial values in the controls
	TBUI::APIElemDefaultFieldMask mask;
	mask.SetAll ();
	APIElementChanged (mask);

	// disables the control
	tabPage->oldHeightLengthEdit.SetStatus (false);

	// initial defElem
	tabPage->uiData->GetAPIElement (&previousElem);
}


void	TestSettingsPageObserver::PanelResized (const DG::PanelResizeEvent& /*ev*/)
{
}


// ... Dialog item notifications ...............................................

void	TestSettingsPageObserver::TextEditChanged (const DG::TextEditChangeEvent& ev)
{
	if (ev.GetSource () == &tabPage->nameTextEdit) {
		WallUserData data;
		BNZeroMemory (&data, sizeof (WallUserData));
		GS::UniString buffer = tabPage->nameTextEdit.GetText ();
		CHTruncate (UniStringToConstCString (buffer), data.name, Sizeof32 (data.name));
		data.uValue = tabPage->uValueRealEdit.GetValue ();

		SetUserDataToDefault (&data);
	}
}


void	TestSettingsPageObserver::RealEditChanged (const DG::RealEditChangeEvent& ev)
{
	if (ev.GetSource () == &tabPage->uValueRealEdit) {
		WallUserData data;
		BNZeroMemory (&data, sizeof (WallUserData));

		GS::UniString buffer = tabPage->nameTextEdit.GetText ();
		CHTruncate (UniStringToConstCString (buffer), data.name, Sizeof32 (data.name));
		data.uValue = tabPage->uValueRealEdit.GetValue ();

		SetUserDataToDefault (&data);
	} else if (ev.GetSource () == &tabPage->newHeightLengthEdit) {
		// the default change handling:
		API_Element elem;
		tabPage->uiData->GetAPIElement (&elem);
		// comparing
		if (elem.wall.height != tabPage->newHeightLengthEdit.GetValue ()) {
			API_Element	mask;
			ACAPI_ELEMENT_MASK_CLEAR(mask);
			ACAPI_ELEMENT_MASK_SET(mask, API_WallType, height);
			elem.wall.height = tabPage->newHeightLengthEdit.GetValue ();
			tabPage->uiData->SetAPIElement (&elem, &mask);
		}
	}
}


void	TestSettingsPageObserver::APIElementChanged (const TBUI::APIElemDefaultFieldMask& fieldMask)
{
	if (fieldMask.GetRegDataChanged ()) {
		// the regdata has changed
		WallUserData data;
		BNZeroMemory (&data, sizeof (WallUserData));
		GetUserDataFromDefault (&data);

		tabPage->nameTextEdit.SetText (data.name);
		tabPage->uValueRealEdit.SetValue (data.uValue);

	}
	if (fieldMask.GetOriginalFieldsChanged ()) {
		// the default change handling:
		API_Element elem;
		tabPage->uiData->GetAPIElement (&elem);
		// comparing
		if (elem.wall.height != previousElem.wall.height) {
			tabPage->oldHeightLengthEdit.SetValue (previousElem.wall.height);
			previousElem.wall.height = elem.wall.height;
		}
		if (elem.wall.height != tabPage->newHeightLengthEdit.GetValue ()) {
			tabPage->newHeightLengthEdit.SetValue (elem.wall.height);
		}
	}
}

// --- TestSettingsPanel -------------------------------------------------------

TestSettingsPanel::TestSettingsPanel (Int32 refCon) :
	TBUI::APIToolUIPanel	(refCon),
	observer				(NULL)
{
}


TestSettingsPanel::~TestSettingsPanel (void)
{

}


bool	TestSettingsPanel::CreatePage (const DG::TabControl& tabControl, TBUI::IAPIToolUIData* data, DG::TabPage** tabPage)
{
	this->tabPage = new TestSettingsPage (tabControl, data);
	*tabPage = this->tabPage;

	observer = new TestSettingsPageObserver (dynamic_cast<TestSettingsPage*> (this->tabPage));

	return true;
}


void	TestSettingsPanel::DestroyPage (void)
{
	delete observer;		// this must be first
	observer = NULL;

	delete tabPage;			// this must be after destroying the observers
	tabPage = NULL;
}


// --- TestInfoBoxPage ---------------------------------------------------------

TestInfoBoxPage::TestInfoBoxPage (const DG::TabControl& tabControl, TBUI::IAPIToolUIData* puiData):
	DG::TabPage (tabControl, 1, ACAPI_GetOwnResModule (), InfoBoxPageId, InvalidResModule),

	nameTextEdit			(GetReference (), NameTextEditId),
	uValueRealEdit			(GetReference (), UValueRealEditId),

	uiData (puiData)
{
}


TestInfoBoxPage::~TestInfoBoxPage (void)
{
	uiData = NULL;
}


// --- TestInfoBoxPageObserver -------------------------------------------------

TestInfoBoxPageObserver::TestInfoBoxPageObserver (TestInfoBoxPage* testPage):
	tabPage (testPage)
{
	AttachToAllItems (*tabPage);
	tabPage->uiData->AttachObserver (this);

	TBUI::APIElemDefaultFieldMask mask;
	mask.SetAll ();
	APIElementChanged (mask);
}


TestInfoBoxPageObserver::~TestInfoBoxPageObserver (void)
{
	DetachFromAllItems (*tabPage);
	tabPage->uiData->DetachObserver (this);
	tabPage = NULL;
}


// ... Utilitiy functions ......................................................

void	TestInfoBoxPageObserver::GetUserDataFromDefault (WallUserData* data)
{
	API_ElementUserData userData;
	BNZeroMemory (&userData, sizeof (API_ElementUserData));
	tabPage->uiData->GetUserData (&userData);

	// allocating area if needed
	if (userData.dataHdl == NULL) {
		userData.dataHdl = BMAllocateHandle (sizeof(WallUserData), ALLOCATE_CLEAR, 0);
		if (userData.dataHdl == NULL)
			return;			//!!! error!!!
	}

	// gets pointer
	WallUserData* sdata = reinterpret_cast<WallUserData*>(*userData.dataHdl);

	// copies from the data
	CHCopyC (sdata->name, data->name);
	data->uValue = sdata->uValue;

	// kill
	BMKillHandle (&userData.dataHdl);
}


void	TestInfoBoxPageObserver::SetUserDataToDefault (WallUserData* sdata)
{
	API_ElementUserData userData;
	BNZeroMemory (&userData, sizeof (API_ElementUserData));

	// allocating
	userData.dataHdl = BMAllocateHandle (sizeof(WallUserData), ALLOCATE_CLEAR, 0);
	if (userData.dataHdl == NULL)
		return;			//!!! error!!!

	userData.flags = APIUserDataFlag_FillWith | APIUserDataFlag_Pickup;

	// gets pointer
	WallUserData* data = reinterpret_cast<WallUserData*>(*userData.dataHdl);

	// copies to the data
	CHCopyC (sdata->name, data->name);
	data->uValue = sdata->uValue;

	// sets to the default
	tabPage->uiData->SetUserData (&userData);
	tabPage->uiData->ApplyToGlobalDefault ();

	BMKillHandle (&userData.dataHdl);
}


// ... Dialog item notifications ...............................................

void	TestInfoBoxPageObserver::TextEditChanged (const DG::TextEditChangeEvent& ev)
{
	if (ev.GetSource () == &tabPage->nameTextEdit) {
		WallUserData data;
		BNZeroMemory (&data, sizeof (WallUserData));

		GS::UniString buffer = tabPage->nameTextEdit.GetText ();
		CHTruncate (UniStringToConstCString (buffer), data.name, Sizeof32 (data.name));
		data.uValue = tabPage->uValueRealEdit.GetValue ();

		SetUserDataToDefault (&data);
	}
}


void	TestInfoBoxPageObserver::RealEditChanged (const DG::RealEditChangeEvent& ev)
{
	if (ev.GetSource () == &tabPage->uValueRealEdit) {
		WallUserData data;
		BNZeroMemory (&data, sizeof (WallUserData));

		GS::UniString buffer = tabPage->nameTextEdit.GetText ();
		CHTruncate (UniStringToConstCString (buffer), data.name, Sizeof32 (data.name));
		data.uValue = tabPage->uValueRealEdit.GetValue ();

		SetUserDataToDefault (&data);
	}
}


void	TestInfoBoxPageObserver::APIElementChanged (const TBUI::APIElemDefaultFieldMask& fieldMask)
{
	if (fieldMask.GetRegDataChanged ()) {
		// the regdata has changed
		WallUserData data;
		BNZeroMemory (&data, sizeof (WallUserData));
		GetUserDataFromDefault (&data);

		tabPage->nameTextEdit.SetText (data.name);
		tabPage->uValueRealEdit.SetValue (data.uValue);

	}
}

// --- TestInfoBoxPanel --------------------------------------------------------

TestInfoBoxPanel::TestInfoBoxPanel (Int32 refCon) :
	TBUI::APIToolUIPanel	(refCon),
	observer				(NULL)
{
}


TestInfoBoxPanel::~TestInfoBoxPanel (void)
{

}

bool	TestInfoBoxPanel::CreatePage (const DG::TabControl& tabControl, TBUI::IAPIToolUIData* data, DG::TabPage** tabPage)
{
	this->tabPage = new TestInfoBoxPage (tabControl, data);
	*tabPage = this->tabPage;

	observer = new TestInfoBoxPageObserver (dynamic_cast<TestInfoBoxPage*> (this->tabPage));

	return true;
}


void	TestInfoBoxPanel::DestroyPage (void)
{
	delete observer;		// this must be first
	observer = NULL;

	delete tabPage;			// this must be after destroying the observers
	tabPage = NULL;
}


// --- TestAttributePage --------------------------------------------------------

TestAttributePage::TestAttributePage (const DG::TabControl& tabControl, VBAD::IAPIAttrUIData* puiData):
	DG::TabPage (tabControl, 1, ACAPI_GetOwnResModule (), AttributePageId, InvalidResModule),

	nameTextEdit			(GetReference (), NameTextEditId),
	lambdaValueRealEdit		(GetReference (), LambdaValueRealEditId),

	uiData (puiData)
{
}


TestAttributePage::~TestAttributePage (void)
{
	uiData = NULL;
}


// --- TestAttributePageObserver ------------------------------------------------

TestAttributePageObserver::TestAttributePageObserver (TestAttributePage* testPage):
	tabPage (testPage)
{
	tabPage->Attach (*this);
	AttachToAllItems (*tabPage);
	tabPage->uiData->AttachObserver (this);

	BNZeroMemory (&previousAttr, sizeof(API_Attribute));
}


TestAttributePageObserver::~TestAttributePageObserver (void)
{
	tabPage->Detach (*this);
	DetachFromAllItems (*tabPage);
	tabPage->uiData->DetachObserver (this);
	tabPage = NULL;
}


// ... Utilitiy functions ......................................................

void	TestAttributePageObserver::GetUserDataFromAttribute (MaterialUserData* data)
{
	API_AttributeUserData currentUserData;
	BNZeroMemory (&currentUserData, sizeof (API_AttributeUserData));
	tabPage->uiData->GetUserData (&currentUserData);

	// allocating area if needed
	if (currentUserData.dataHdl == NULL) {
		currentUserData.dataHdl = BMAllocateHandle (sizeof(MaterialUserData), ALLOCATE_CLEAR, 0);
		if (currentUserData.dataHdl == NULL)
			return;			//!!! error!!!
	}

	// gets pointer
	MaterialUserData* sdata = reinterpret_cast<MaterialUserData*>(*currentUserData.dataHdl);

	// copies from the data
	CHCopyC (sdata->name, data->name);
	data->lambdaValue = sdata->lambdaValue;

	// kill
	BMKillHandle (&currentUserData.dataHdl);
	currentUserData.dataHdl = NULL;
}


void	TestAttributePageObserver::SetUserDataToAttribute (MaterialUserData* sdata)
{
	API_AttributeUserData currentUserData;
	BNZeroMemory (&currentUserData, sizeof (API_AttributeUserData));

	// allocating
	currentUserData.dataHdl = BMAllocateHandle (sizeof(MaterialUserData), ALLOCATE_CLEAR, 0);
	if (currentUserData.dataHdl == NULL)
		return;			//!!! error!!!

	currentUserData.platformSign = GS::Act_Platform_Sign;
	// gets pointer
	MaterialUserData* data = reinterpret_cast<MaterialUserData*>(*currentUserData.dataHdl);

	// copies to the data
	CHCopyC (sdata->name, data->name);
	data->lambdaValue = sdata->lambdaValue;

	// sets to the default
	tabPage->uiData->SetUserData (&currentUserData);

	// kill
	BMKillHandle (&currentUserData.dataHdl);
	currentUserData.dataHdl = NULL;
}


// ... TabPage notifications ...................................................

void	TestAttributePageObserver::PanelOpened (const DG::PanelOpenEvent& /*ev*/)
{
	// initial values in the controls
	VBAD::APIAttributeFieldMask mask;
	mask.SetAll ();
	APIAttributeChanged (mask);

	// initial attribute
	tabPage->uiData->GetAPIAttribute (&previousAttr);
	if (previousAttr.material.texture.fileLoc != NULL) {
		delete previousAttr.material.texture.fileLoc;
		previousAttr.material.texture.fileLoc = NULL;
	}
}


void	TestAttributePageObserver::PanelResized (const DG::PanelResizeEvent& /*ev*/)
{
}


// ... Dialog item notifications ...............................................

void	TestAttributePageObserver::TextEditChanged (const DG::TextEditChangeEvent& ev)
{
	if (ev.GetSource () == &tabPage->nameTextEdit) {
		MaterialUserData data;
		BNZeroMemory (&data, sizeof (MaterialUserData));

		GS::UniString buffer = tabPage->nameTextEdit.GetText ();
		CHTruncate (UniStringToConstCString (buffer), data.name, Sizeof32 (data.name));
		data.lambdaValue = tabPage->lambdaValueRealEdit.GetValue ();

		SetUserDataToAttribute (&data);
	}
}


void	TestAttributePageObserver::RealEditChanged (const DG::RealEditChangeEvent& ev)
{
	if (ev.GetSource () == &tabPage->lambdaValueRealEdit) {
		MaterialUserData data;
		BNZeroMemory (&data, sizeof (MaterialUserData));
		GS::UniString buffer = tabPage->nameTextEdit.GetText ();
		CHTruncate (UniStringToConstCString (buffer), data.name, Sizeof32 (data.name));
		data.lambdaValue = tabPage->lambdaValueRealEdit.GetValue ();

		SetUserDataToAttribute (&data);
	}
}


void	TestAttributePageObserver::APIAttributeChanged (const VBAD::APIAttributeFieldMask& fieldMask)
{
	if (fieldMask.GetRegDataChanged ()) {
		// the regdata has changed
		MaterialUserData data;
		BNZeroMemory (&data, sizeof (MaterialUserData));
		GetUserDataFromAttribute (&data);

		tabPage->nameTextEdit.SetText (data.name);
		tabPage->lambdaValueRealEdit.SetValue (data.lambdaValue);

	}

	// the default change handling:
	API_Attribute attr;
	tabPage->uiData->GetAPIAttribute (&attr);
	if (attr.material.texture.fileLoc != NULL) {
		delete attr.material.texture.fileLoc;
		attr.material.texture.fileLoc = NULL;
	}

	// comparing
	if (attr.material.ambientPc != previousAttr.material.ambientPc) {
		previousAttr.material.ambientPc = attr.material.ambientPc;
	}
}

// --- TestAttributePanel -------------------------------------------------------

TestAttributePanel::TestAttributePanel (Int32 refCon) :
	VBAD::APIAttrUIPanel	(refCon),
	observer				(NULL)
{
}


TestAttributePanel::~TestAttributePanel (void)
{

}


bool	TestAttributePanel::CreatePage (const DG::TabControl& tabControl, VBAD::IAPIAttrUIData* data, DG::TabPage** tabPage)
{
	this->tabPage = new TestAttributePage (tabControl, data);
	*tabPage = this->tabPage;

	observer = new TestAttributePageObserver (dynamic_cast<TestAttributePage*> (this->tabPage));

	return true;
}


void	TestAttributePanel::DestroyPage (void)
{
	delete observer;		// this must be first
	observer = NULL;

	delete tabPage;			// this must be after destroying the observers
	tabPage = NULL;
}


}		// namespace PanelTest

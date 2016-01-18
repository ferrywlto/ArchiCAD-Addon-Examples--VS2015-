// *****************************************************************************
// Source code for the AddOnObject Manager Add-On
// API Development Kit 19; Mac/Win
//
// Namespaces:			Contact person:
//		-None-
//
// [SG compatible] - Yes
// *****************************************************************************

#include "APIEnvir.h"
#define	_ADDONOBJECT_MANAGER_TRANSL_

// ---------------------------------- Includes ---------------------------------

#include	"Array.hpp"
#include	"MemoryOChannel.hpp"
#include	"MemoryIChannel.hpp"
#include	"SetPlatformProtocol.hpp"
#include	"DG.h"
#include	"DGDialog.hpp"
#include	"DGUtility.hpp"
#include	"DGPanel.hpp"
#include	"DGButton.hpp"
#include	"DGListBox.hpp"
#include	"DGListView.hpp"
#include	"DGTabControl.hpp"
#include	"DGModule.hpp"

#include	"ACAPinc.h"					// also includes APIdefs.h
#include	"CustomerData.hpp"

// ---------------------------------- Types ------------------------------------

typedef GS::HashTable<short, GS::UniString> UserIdToUserNameTable;
// ---------------------------------- Variables --------------------------------
static const short CustomerManagerDialogResourceId				= 32500;
static const short MenuStringResourceId							= 32500;

static GS::UniString AddOnObjectName ("CustomerInfo");
// ---------------------------------- Prototypes -------------------------------
extern bool ModifyCustomerData				   (CustomerData& customerData, TWCloseReason& closeReason);
extern bool IsModifyButtonReservationDependent (void);

// =============================================================================
//
//
//
// =============================================================================

static bool	IsTeamworkProject (short* currentUserId)
{
	API_ProjectInfo	pi;
	BNZeroMemory (&pi, sizeof (API_ProjectInfo));

	ACAPI_Environment (APIEnv_ProjectID, &pi);

	if (currentUserId != nullptr)
		*currentUserId = pi.teamwork ? pi.userId : 0;

	if (pi.location != NULL)
		delete pi.location;
	if (pi.location_team != NULL)
		delete pi.location_team;

	return pi.teamwork != 0;
}


static GSErrCode	GetUserIdToUserNameTable (UserIdToUserNameTable* userIdToUserNameTable)
{
	userIdToUserNameTable->Clear ();

	API_SharingInfo	sharingInfo;
	BNZeroMemory (&sharingInfo, sizeof (API_SharingInfo));
	GSErrCode err = ACAPI_Environment (APIEnv_ProjectSharingID, &sharingInfo);
	if (err == NoError && sharingInfo.users != NULL) {
		for (Int32 i = 0; i < sharingInfo.nUsers; i++)
			userIdToUserNameTable->Add (((*sharingInfo.users)[i]).userId, GS::UniString (((*sharingInfo.users)[i]).loginName));
	}
	if (sharingInfo.users != NULL)
		BMhKill (reinterpret_cast<GSHandle*>(&sharingInfo.users));

	return err;
}


class CustomerManagerDialog : public DG::ModalDialog,
							  public DG::CompoundItemObserver,
							  public DG::PanelObserver,
							  public DG::SplitButtonObserver,
							  public DG::ListBoxObserver
{
	DG::Button					okButton;
	DG::Button					cancelButton;
	DG::SingleSelListBox		customerListBox;
	DG::Button					newButton;
	DG::Button					modifyButton;
	DG::Button					deleteButton;
	DG::Button					reserveButton;
	DG::Button					releaseButton;
	DG::SplitButton				grantButton;
	DG::Button					requestButton;

	bool						isTeamwork;
	short						currentUserId;

	GS::Array<GS::Guid>			twOperationGuids;

	TWCloseReason				twCloseReason;

	short						grantUserId;

	bool						isUpdateItemsEnabled;

	enum {
		OkButtonId		= 1,
		CancelButtonId	= 2,
		CustomerListId	= 3,
		NewButtonId		= 4,
		ModifyButtonId	= 5,
		DeleteButtonId	= 6,
		ReserveButtonId	= 7,
		ReleaseButtonId	= 8,
		GrantButtonId	= 9,
		RequestButtonId	= 10
	};

public:
	CustomerManagerDialog (GSResModule dialModule, short resId, GSResModule dialIconModule);
	~CustomerManagerDialog ();

	virtual void	ButtonClicked (const DG::ButtonClickEvent& ev);
	virtual void	SplitButtonPopupChanged (const DG::SplitButtonChangeEvent& ev);

	virtual void	PanelClosed (const DG::PanelCloseEvent& ev);

	virtual void	ListBoxSelectionChanged (const DG::ListBoxSelectionEvent& ev);

	TWCloseReason	GetTWCloseReason (void) const;
	short			GetGrantUserId (void) const;
	void			GetTWOperationGuids (GS::Array<GS::Guid>* twOperationGuidArray) const;

	void			UpdateItems ();

private:
	void			SetTabData (short tabFieldCount);
	void			Init (void);
	void			SetListItemText (short index, const UserIdToUserNameTable& userIdToUserNameTable);
};


CustomerManagerDialog::CustomerManagerDialog (GSResModule dialResModule, short resId, GSResModule dialIconResModule):
	DG::ModalDialog (dialResModule, resId, dialIconResModule),
	okButton (GetReference (), OkButtonId),
	cancelButton (GetReference (), CancelButtonId),
	customerListBox (GetReference (), CustomerListId),
	newButton (GetReference (), NewButtonId),
	modifyButton (GetReference (), ModifyButtonId),
	deleteButton (GetReference (), DeleteButtonId),
	reserveButton (GetReference (), ReserveButtonId),
	releaseButton (GetReference (), ReleaseButtonId),
	grantButton (GetReference (), GrantButtonId),
	requestButton (GetReference (), RequestButtonId),
	twCloseReason (TWCloseReason_Nothing),
	grantUserId (-1),
	isUpdateItemsEnabled (true)
{
	isTeamwork = IsTeamworkProject (&currentUserId);

	AttachToAllItems (*this);
	Init ();
	this->Attach (*this);
}


CustomerManagerDialog::~CustomerManagerDialog ()
{
}


void	CustomerManagerDialog::ButtonClicked (const DG::ButtonClickEvent& ev)
{
	if (ev.GetSource () == &okButton) {
		PostCloseRequest (DG::ModalDialog::Accept);
	} else if (ev.GetSource () == &cancelButton) {
		PostCloseRequest (DG::ModalDialog::Cancel);
	} else if (ev.GetSource () == &newButton) {
		CustomerData* customerData = new CustomerData ();
		if (customerData != NULL) {
			customerData->guid.Generate ();
			customerData->GenerateRandomContent ();
			customerData->created = true;
			customerListBox.AppendItem ();
			customerListBox.SetItemValue (DG::ListBox::BottomItem, reinterpret_cast<DGUserData> (customerData));
			UpdateItems ();
		}
	} else if (ev.GetSource () == &modifyButton) {
		short listItem = customerListBox.GetSelectedItem ();
		if (listItem > 0) {
			CustomerData* customerData = reinterpret_cast<CustomerData*> (customerListBox.GetItemValue (listItem));
			if (!customerData->markedAsDeleted && !customerData->created) {
				CustomerData customerDataCopy (*customerData);
				TWCloseReason closeReason = TWCloseReason_Nothing;
				if (ModifyCustomerData (customerDataCopy, closeReason)) {
					*customerData = customerDataCopy;
					customerData->modified = true;
					UpdateItems ();
					customerListBox.SetItemColor (listItem, Gfx::Color::DarkGray);
					if (closeReason != TWCloseReason_Nothing) {
						twOperationGuids.Clear ();
						twOperationGuids.Push (customerData->guid);
						twCloseReason = closeReason;
						if (twCloseReason != TWCloseReason_Nothing)
							PostCloseRequest (DG::ModalDialog::Accept);
					}
				}
			}
		}
	} else if (ev.GetSource () == &deleteButton) {
		short listItem = customerListBox.GetSelectedItem ();
		if (listItem > 0) {
			CustomerData* customerData = reinterpret_cast<CustomerData*> (customerListBox.GetItemValue (listItem));
			if (customerData->created) {
				delete customerData;
				customerListBox.DeleteItem (listItem);
			} else {
				customerData->modified = false;
				customerData->markedAsDeleted = true;
				customerListBox.SetItemColor (listItem, Gfx::Color::Gray);
			}
			UpdateItems ();
		}
	} else if (ev.GetSource () == &reserveButton) {
		short listItem = customerListBox.GetSelectedItem ();
		CustomerData* customerData = reinterpret_cast<CustomerData*> (customerListBox.GetItemValue (listItem));
		if (listItem > 0) {
			twOperationGuids.Clear ();
			twOperationGuids.Push (customerData->guid);
			twCloseReason = TWCloseReason_Reserve;
			customerListBox.SetItemColor (listItem, Gfx::Color::Red);
			PostCloseRequest (DG::ModalDialog::Accept);
		}
	} else if (ev.GetSource () == &releaseButton) {
		short listItem = customerListBox.GetSelectedItem ();
		CustomerData* customerData = reinterpret_cast<CustomerData*> (customerListBox.GetItemValue (listItem));
		if (listItem > 0) {
			twOperationGuids.Clear ();
			twOperationGuids.Push (customerData->guid);
			twCloseReason = TWCloseReason_Release;
			customerListBox.SetItemColor (listItem, Gfx::Color::Blue);
			PostCloseRequest (DG::ModalDialog::Accept);
		}
	} else if (ev.GetSource () == &requestButton) {
		short listItem = customerListBox.GetSelectedItem ();
		CustomerData* customerData = reinterpret_cast<CustomerData*> (customerListBox.GetItemValue (listItem));
		if (listItem > 0) {
			twOperationGuids.Clear ();
			twOperationGuids.Push (customerData->guid);
			twCloseReason = TWCloseReason_Request;
			customerListBox.SetItemColor (listItem, Gfx::Color::Yellow);
			PostCloseRequest (DG::ModalDialog::Accept);
		}
	} else if (ev.GetSource () == &grantButton) {
		short listItem = customerListBox.GetSelectedItem ();
		CustomerData* customerData = reinterpret_cast<CustomerData*> (customerListBox.GetItemValue (listItem));
		if (listItem > 0) {
			twOperationGuids.Clear ();
			twOperationGuids.Push (customerData->guid);
			twCloseReason = TWCloseReason_Grant;
			customerListBox.SetItemColor (listItem, Gfx::Color::Green);
			PostCloseRequest (DG::ModalDialog::Accept);
		}
	}
}


void	CustomerManagerDialog::SplitButtonPopupChanged (const DG::SplitButtonChangeEvent& ev)
{
	if (ev.GetSource () == &grantButton) {
		UIndex grantUserItem = grantButton.GetSelectedItem ();
		GS::UniString grantUserString = grantButton.GetItemText ((short)grantUserItem);

		short listItem = customerListBox.GetSelectedItem ();
		CustomerData* customerData = reinterpret_cast<CustomerData*> (customerListBox.GetItemValue (listItem));
		if (listItem > 0) {
			twOperationGuids.Clear ();
			twOperationGuids.Push (customerData->guid);
			twCloseReason = TWCloseReason_Grant;

			UserIdToUserNameTable userTable;
			GetUserIdToUserNameTable (&userTable);
			for (auto it = userTable.EnumeratePairs (); it != nullptr; ++it) {
				short userId = *it->key;
				GS::UniString userName = *it->value;
				if (userName == grantUserString)
					grantUserId = userId;
			}

			customerListBox.SetItemColor (listItem, Gfx::Color::Green);
			PostCloseRequest (DG::ModalDialog::Accept);
		}
	}
}


void	CustomerManagerDialog::PanelClosed (const DG::PanelCloseEvent& ev)
{
	isUpdateItemsEnabled = false;

	for (short listItem = 1; listItem <= customerListBox.GetItemCount (); listItem++) {
		CustomerData* customerData = reinterpret_cast<CustomerData*> (customerListBox.GetItemValue (listItem));
		if (ev.IsAccepted ()) {
			if (customerData->markedAsDeleted) {
				bool exists = false;
				if (DBVERIFY (ACAPI_AddOnObject_ExistsObject (GSGuid2APIGuid (customerData->guid), &exists) == NoError && exists))
					ACAPI_AddOnObject_DeleteObject (GSGuid2APIGuid (customerData->guid));
			} else if (customerData->created) {
				IO::MemoryOChannel memChannel;
				if (customerData->Write (memChannel) == NoError) {
					GSHandle content = nullptr;
					BMPtrToHandle (memChannel.GetDestination (), &content, memChannel.GetDataSize ());
					API_Guid objectId;
					ACAPI_AddOnObject_CreateObject (AddOnObjectName, content, &objectId);

					BMKillHandle (&content);

					GS::Guid gsObjectId = APIGuid2GSGuid (objectId);
					twOperationGuids.Replace (customerData->guid, gsObjectId);
				}
			} else if (customerData->modified) {
				bool exists = false;
				if (DBVERIFY (ACAPI_AddOnObject_ExistsObject (GSGuid2APIGuid (customerData->guid), &exists) == NoError && exists)) {
					IO::MemoryOChannel memChannel;
					if (customerData->Write (memChannel) == NoError) {
						GSHandle content = nullptr;
						BMPtrToHandle (memChannel.GetDestination (), &content, memChannel.GetDataSize ());
						ACAPI_AddOnObject_ModifyObject (GSGuid2APIGuid (customerData->guid), nullptr, &content);
						BMKillHandle (&content);
					}
				}
			}
		}
		delete customerData;
	}
}


void CustomerManagerDialog::ListBoxSelectionChanged (const DG::ListBoxSelectionEvent& /*ev*/)
{
	UpdateItems ();
}


void CustomerManagerDialog::SetTabData (short tabFieldCount)
{
	if (customerListBox.GetTabFieldCount () == tabFieldCount)
		return;

	DG::Rect lBox = customerListBox.GetRect ();
	lBox.SetRight (lBox.GetRight () - 44);
	short width = (short) (lBox.GetRight () - lBox.GetLeft ());

	short columnWidth = (short)(width / tabFieldCount);

	customerListBox.SetTabFieldCount (tabFieldCount);
	for (short i = 1; i <= tabFieldCount; ++i)
		customerListBox.SetTabFieldProperties (i, (i - 1) * columnWidth, i * columnWidth, DG::ListBox::Left, DG::ListBox::EndTruncate, false, true);
}


void	CustomerManagerDialog::UpdateItems (void)
{
	if (!isUpdateItemsEnabled)
		return;

	SetTabData (isTeamwork ? 4 : 3);

	UserIdToUserNameTable userIdToUserNameTable;
	GetUserIdToUserNameTable (&userIdToUserNameTable);

	short selectedItem = customerListBox.GetSelectedItem ();
	if (selectedItem == 0 && customerListBox.GetItemCount () > 0) {
		customerListBox.SelectItem (1);
		selectedItem = 1;
	}

	for (short listItem = 1; listItem <= customerListBox.GetItemCount (); listItem++) {
		SetListItemText (listItem, userIdToUserNameTable);
		if (selectedItem > 0 && selectedItem == listItem) {
			CustomerData* customerData = reinterpret_cast<CustomerData*> (customerListBox.GetItemValue (listItem));
			if (isTeamwork) {
				GSErrCode err = NoError;
				short ownerId = 0;
				if (customerData->created) {
					ownerId = 2;
				} else {
					err = ACAPI_AddOnObject_GetTeamworkOwnerId (GSGuid2APIGuid (customerData->guid), &ownerId);
				}
				if (err == NoError) {
					reserveButton.SetStatus (ownerId == 0);
					releaseButton.SetStatus (ownerId == currentUserId);
					grantButton.SetStatus (ownerId == currentUserId && userIdToUserNameTable.GetSize () > 1);
					requestButton.SetStatus (ownerId != 0 && ownerId != currentUserId);
					deleteButton.SetStatus (ownerId == currentUserId);
					if (IsModifyButtonReservationDependent ())
						modifyButton.SetStatus (ownerId == currentUserId);
				} else {
					reserveButton.Disable ();
					releaseButton.Disable ();
					grantButton.Disable ();
					requestButton.Disable ();
					deleteButton.Disable ();
					modifyButton.Disable ();
				}
			}
		}
	}

	if (!isTeamwork) {
		reserveButton.Hide ();
		releaseButton.Hide ();
		grantButton.Hide ();
		requestButton.Hide ();
	} else {
		reserveButton.Show ();
		releaseButton.Show ();
		grantButton.Show ();
		requestButton.Show ();
	}

	if (isTeamwork) {
		grantButton.DeleteAllItems ();
		for (auto it = userIdToUserNameTable.EnumeratePairs (); it != nullptr; ++it) {
			if (*it->key == currentUserId)
				continue;

			grantButton.AppendItem (*it->value);
		}
	}
}


void CustomerManagerDialog::SetListItemText (short index, const UserIdToUserNameTable& userIdToUserNameTable)
{
	CustomerData* customerData = reinterpret_cast<CustomerData*> (customerListBox.GetItemValue (index));

	customerListBox.SetTabItemText (index, 1, customerData->name);
	customerListBox.SetTabItemText (index, 2, customerData->city);
	customerListBox.SetTabItemText (index, 3, customerData->country);

	if (isTeamwork) {

		short ownerId = 0;
		ACAPI_AddOnObject_GetTeamworkOwnerId (GSGuid2APIGuid (customerData->guid), &ownerId);
		GS::UniString itemString;
		if (userIdToUserNameTable.ContainsKey (ownerId)) {
			itemString += userIdToUserNameTable[ownerId];
		}
		customerListBox.SetTabItemText (index, 4, itemString);
	}
}


void CustomerManagerDialog::Init (void)
{
	GS::Array<API_Guid> objects;
	ACAPI_AddOnObject_GetObjectList (&objects);

	for (GS::Array<API_Guid>::ConstIterator it = objects.Enumerate (); it != NULL; ++it) {
		API_Guid objectGuid = *it;
		GS::UniString objectName;
		GSHandle content = nullptr;
		if (ACAPI_AddOnObject_GetObjectContent (objectGuid, &objectName, &content) == NoError) {
			IO::MemoryIChannel mic (*content, BMhGetSize (content));

			CustomerData* customerData = new CustomerData;
			customerData->Read (mic);
			customerData->guid = APIGuid2GSGuid (objectGuid);

			customerListBox.AppendItem ();
			customerListBox.SetItemValue (DG::ListBox::BottomItem, reinterpret_cast<DGUserData> (customerData));
		}
		BMKillHandle (&content);
	}

	UpdateItems ();
}


TWCloseReason	CustomerManagerDialog::GetTWCloseReason (void) const
{
	return twCloseReason;
}


short	CustomerManagerDialog::GetGrantUserId (void) const
{
	return grantUserId;
}


void	CustomerManagerDialog::GetTWOperationGuids (GS::Array<GS::Guid>* twOperationGuidArray) const
{
	*twOperationGuidArray = twOperationGuids;
}


GSErrCode __ACENV_CALL ReservationChangedHandler (const GS::HashTable<API_Guid, short>&	reserved,
											     const GS::HashSet<API_Guid>&			released,
												 const GS::HashSet<API_Guid>&			/*deleted*/)
{
	if (!DGIsDialogOpen (CustomerManagerDialogResourceId))
		return NoError;

	bool addOnObjectFound = false;
	for (auto it = reserved.EnumeratePairs (); it != nullptr && !addOnObjectFound; ++it) {
		API_Guid objectGuid = *it->key;
		bool exists = false;
		if (ACAPI_AddOnObject_ExistsObject (objectGuid, &exists) == NoError && exists)
			addOnObjectFound = true;
	}

	for (auto it = released.Enumerate (); it != nullptr && !addOnObjectFound; ++it) {
		API_Guid objectGuid = *it;
		bool exists = false;
		if (ACAPI_AddOnObject_ExistsObject (objectGuid, &exists) == NoError && exists)
			addOnObjectFound = true;
	}

	DG::Dialog* dialog = DG::GetDialogFromId (CustomerManagerDialogResourceId);
	if (dialog != nullptr) {
		CustomerManagerDialog* customerManagerDialog = dynamic_cast<CustomerManagerDialog*> (dialog);
		if (DBVERIFY (customerManagerDialog != nullptr))
			customerManagerDialog->UpdateItems ();
	}

	return NoError;
}

// -----------------------------------------------------------------------------
// DoCommand
//		called to perform the user-asked command
// -----------------------------------------------------------------------------

GSErrCode	__ACENV_CALL	MenuCommandHandler (const API_MenuParams *params)
{
	if (params->menuItemRef.itemIndex == 1) {
		bool invokeDialog = true;
		while (invokeDialog) {
			invokeDialog = false;
			CustomerManagerDialog customerManagerDialog (ACAPI_GetOwnResModule (), CustomerManagerDialogResourceId,  ACAPI_GetOwnResModule ());
			bool result = customerManagerDialog.Invoke ();

			if (result && customerManagerDialog.GetTWCloseReason () != TWCloseReason_Nothing) {
				GS::Array<API_Guid> objectGuids;
				GS::Array<GS::Guid> twOperationGuids;
				customerManagerDialog.GetTWOperationGuids (&twOperationGuids);
				for (auto it = twOperationGuids.Enumerate (); it != nullptr; ++it)
					objectGuids.Push (GSGuid2APIGuid (*it));

				if (customerManagerDialog.GetTWCloseReason () == TWCloseReason_Reserve) {
					ACAPI_AddOnObject_ReserveObjects (objectGuids, nullptr/*&conflicts*/);
					invokeDialog = true;
				} else if (customerManagerDialog.GetTWCloseReason () == TWCloseReason_Release) {
					ACAPI_AddOnObject_ReleaseObjects (objectGuids);
					invokeDialog = true;
				} else if (customerManagerDialog.GetTWCloseReason () == TWCloseReason_Request) {
					ACAPI_AddOnObject_RequestObjects (objectGuids, GS::UniString ());
					invokeDialog = true;
				} else if (customerManagerDialog.GetTWCloseReason () == TWCloseReason_Grant) {
					short currentUserId = 0;
					IsTeamworkProject (&currentUserId);

					short toUserId = customerManagerDialog.GetGrantUserId ();
					if (toUserId == -1) {
						UserIdToUserNameTable userIdToNameTable;
						GetUserIdToUserNameTable (&userIdToNameTable);
						toUserId = -1;
						for (auto it = userIdToNameTable.EnumeratePairs (); it != nullptr && toUserId == -1; ++it) {
							if (*it->key != currentUserId)
								toUserId = *it->key;
						}
					}

					if (toUserId != -1) {
						ACAPI_AddOnObject_GrantObjects (objectGuids, toUserId, nullptr);
					}

					invokeDialog = true;
				}
			}
		}
	}

	return NoError;
}		// MenuCommandHandler


// -----------------------------------------------------------------------------
// AddOnObject Merge event handler
// -----------------------------------------------------------------------------
static void	CopyAPIAddOnObject (API_AddonObject& target, const API_AddonObject& source)
{
	BNZeroMemory (&target, sizeof (API_AddonObject));
	target.id = source.id;
	if (source.name != nullptr)
		target.name = new GS::UniString (*source.name);
	if (source.data != nullptr)
		BMHandleToHandle (source.data, &target.data);
}


static bool EqualAddOnObjects (const GS::UniString& name1, GSConstHandle data1, const GS::UniString& name2, GSConstHandle data2)
{
	// id is ignored here
	if (name1 != name2)
		return false;
	if ((data1 == nullptr) != (data2 == nullptr))
		return false;
	if (data1 != nullptr) {
		IO::MemoryIChannel micA (*data1, BMhGetSize (data1));

		CustomerData customerDataA;
		customerDataA.Read (micA);

		IO::MemoryIChannel micB (*data2, BMhGetSize (data2));

		CustomerData customerDataB;
		customerDataB.Read (micB);

		return customerDataA.name == customerDataB.name &&
				customerDataA.city == customerDataB.city &&
				customerDataA.country == customerDataB.country;
	}
	return true;
}


GSErrCode	__ACENV_CALL	APIAddonObjectMergeHandler (const GS::Array<API_AddonObject>&	sourceObjects)
{
	GS::Array<API_Guid> targetObjects;
	GSErrCode err = ACAPI_AddOnObject_GetObjectList (&targetObjects);

	for (auto it = sourceObjects.Enumerate (); it != nullptr && err == NoError; ++it) {
		const API_AddonObject& sourceObject = *it;
		bool found = false;
		for (auto it2 = targetObjects.Enumerate (); it2 != nullptr && !found && err == NoError; ++it2) {
			GS::UniString objectName;
			GSHandle objectContent = nullptr;
			err = ACAPI_AddOnObject_GetObjectContent (*it2, &objectName, &objectContent);
			if (err == NoError)
				found = EqualAddOnObjects (*sourceObject.name, sourceObject.data, objectName, objectContent);
			BMhKill (&objectContent);
		}
		if (found || err != NoError)
			continue;

		API_Guid guid;
		err = ACAPI_AddOnObject_CreateObject (*sourceObject.name, sourceObject.data, &guid);
	}

	return err;
}		// APIAddonObjectMergeHandler


// -----------------------------------------------------------------------------
// AddOnObject Save into old format project file
// -----------------------------------------------------------------------------

GSErrCode	__ACENV_CALL	APIAddonObjectSaveOldFormatHandler (API_FTypeID							/*planFileType*/,
																const GS::Array<API_AddonObject>&	originalInfo,
																GS::Array<API_AddonObject>&			destInfo)
{
	for (auto it = originalInfo.Enumerate (); it != nullptr; ++it) {
		API_AddonObject tmp;
		CopyAPIAddOnObject (tmp, *it);
		destInfo.Push (tmp);
	}

	return NoError;
}		// APIAddonObjectSaveOldFormatHandler

// -----------------------------------------------------------------------------
// AddOnObject convert into new format project file
// -----------------------------------------------------------------------------

GSErrCode	__ACENV_CALL	APIAddonObjectConvertNewFormatHandler (API_FTypeID						/*planFileType*/,
																  const GS::Array<API_AddonObject>&	originalInfo,
																  GS::Array<API_AddonObject>&		destInfo)

{
	for (auto it = originalInfo.Enumerate (); it != nullptr; ++it) {
		API_AddonObject tmp;
		CopyAPIAddOnObject (tmp, *it);
		destInfo.Push (tmp);
	}

	return NoError;
}		// APIAddonObjectConvertNewFormatHandler

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
}		// CheckEnvironment


//------------------------------------------------------
// Interface definitions
//------------------------------------------------------
GSErrCode	__ACENV_CALL	RegisterInterface (void)
{
	ACAPI_Register_Menu (MenuStringResourceId, 0, MenuCode_UserDef, MenuFlag_Default);
	ACAPI_Register_AddOnObjectHandler ();

	return NoError;
}		// RegisterInterface


// -----------------------------------------------------------------------------
// Initialize
//		called after the Add-On has been loaded into memory
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	Initialize (void)
{
	GSErrCode err = ACAPI_Install_MenuHandler (MenuStringResourceId, MenuCommandHandler);
	if (err == NoError)
		err = ACAPI_Notify_CatchElementReservationChange (ReservationChangedHandler);
	if (err == NoError)
		err = ACAPI_Install_AddOnObjectMergeHandler (APIAddonObjectMergeHandler);
	if (err == NoError)
		err = ACAPI_Install_AddOnObjectSaveOldFormatHandler (APIAddonObjectSaveOldFormatHandler);
	if (err == NoError)
		err = ACAPI_Install_AddOnObjectConvertNewFormatHandler (APIAddonObjectConvertNewFormatHandler);

	char str[3200];
	ACAPI_Resource_GetLocStr (str, 32501, 1);
	GS::UniString localisedName (str);
	ACAPI_AddOnObject_SetUIProperties (nullptr, &AddOnObjectName, localisedName, MenuStringResourceId);

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

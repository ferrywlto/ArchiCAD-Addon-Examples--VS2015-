// *****************************************************************************
// Source code for the ModulData Manager Add-On
// API Development Kit 19; Mac/Win
//
// Namespaces:			Contact person:
//		-None-
//
// [SG compatible] - Yes
// *****************************************************************************

#include "APIEnvir.h"
#define	_MODULDATA_MANAGER_TRANSL_

// ---------------------------------- Includes ---------------------------------

#include	"Array.hpp"
#include	"MemoryOChannel.hpp"
#include	"MemoryIChannel.hpp"
#include	"SetPlatformProtocol.hpp"
#include	"DG.h"

#include	"ACAPinc.h"					// also includes APIdefs.h


// ---------------------------------- Types ------------------------------------


// ---------------------------------- Variables --------------------------------


// ---------------------------------- Prototypes -------------------------------


// =============================================================================
//
//
//
// =============================================================================

class CustomerData {
public:
	GS::Guid			guid;
	GS::UniString		name;
	GS::UniString		city;
	GS::UniString		country;
	bool				modified;
	bool				markedAsDeleted;

	CustomerData () : modified (false), markedAsDeleted (false) { }

	CustomerData (const API_ModulData& modulData, const GS::UniString& modulName) : modified (false), markedAsDeleted (false)
	{
		guid.ConvertFromString (modulName);
		if (modulData.dataHdl != NULL && modulData.dataVersion == 1) {
			IO::MemoryIChannel memChannel (*modulData.dataHdl, BMGetHandleSize (modulData.dataHdl));
			if (modulData.platformSign != GS::Act_Platform_Sign) {
				GS::PlatformSign ps = (modulData.platformSign == GS::Win_Platform_Sign) ? GS::Win_Platform_Sign :
									  ((modulData.platformSign == GS::Mac_Platform_Sign) ? GS::Mac_Platform_Sign : GS::Mactel_Platform_Sign);
				IO::SetPlatformIProtocol (memChannel, ps);
			}
			Read (memChannel);
		}
	}

	GSErrCode	Read (GS::IChannel& ic)
	{
		name.Read (ic);
		city.Read (ic);
		country.Read (ic);
		return ic.GetInputStatus ();
	}

	GSErrCode	Write (GS::OChannel& oc) const
	{
		name.Write (oc);
		city.Write (oc);
		country.Write (oc);
		return oc.GetOutputStatus ();
	}

	void	GenerateRandomContent (void)
	{
		static const char firstNames[10][32] = { "Albert", "Alex", "Alfred", "Amy", "Andrew", "Angelina", "Anne", "Anthony", "Arnold", "Arthur" };
		static const char familyNames[10][32] = { "Smith", "Johnson", "Williams", "Brown", "Jones", "Miller", "Davis", "Garcia", "Rodriguez", "Wilson" };
		static const char cities[10][32] = { "San Francisco", "Budapest", "Amsterdam", "Venezia", "Helsinki", "Limassol", "Tokyo", "Barcelona", "Abuja", "Auckland" };
		static const char countries[10][32] = { "United States", "Hungary", "Netherlands", "Italy", "Finland", "Cyprus", "Japan", "Spain", "Nigeria", "New Zeeland" };

		Int32 firstNameIndex = rand () % 10;
		Int32 familyNameIndex = rand () % 10;
		Int32 locationIndex = rand () % 10;
		name = GS::UniString (firstNames[firstNameIndex]) + " " + GS::UniString (familyNames[familyNameIndex]);
		city = GS::UniString (cities[locationIndex]);
		country = GS::UniString (countries[locationIndex]);
	}
};


// -----------------------------------------------------------------------------
// Set up the list item's tabulator fields
// -----------------------------------------------------------------------------

static void SetTabData (short dialId, short item)
{
	DGListTabData		lTabData[2];
	API_Rect			lBox;

	DGGetItemRect (dialId, item, &lBox.left, &lBox.top, &lBox.right, &lBox.bottom);
	lBox.right -= 44;
	short width = (short) (lBox.right - lBox.left);

	lTabData[0].begPos		= 0;
	lTabData[0].endPos		= (short) (width / 2);
	lTabData[0].justFlag	= DG_IS_LEFT;
	lTabData[0].truncFlag	= DG_IS_TRUNCEND;
	lTabData[0].hasSeparator = false;
	lTabData[0].disabled	= false;

	lTabData[1].begPos		= lTabData[0].endPos;
	lTabData[1].endPos		= (short) (lBox.right);
	lTabData[1].justFlag	= DG_IS_LEFT;
	lTabData[1].truncFlag	= DG_IS_TRUNCEND;
	lTabData[1].hasSeparator = false;
	lTabData[1].disabled	= false;

	DGListSetTabData (dialId, item, 2, &lTabData[0]);
}		// SetTabData


// -----------------------------------------------------------------------------
// Convert Customer list from old format if there is any
// -----------------------------------------------------------------------------

static void ConvertModulesFromOldFormat (void)
{
	GS::Array<GS::UniString> modulNameList;
	ACAPI_ModulData_GetList (&modulNameList);

	for (GS::Array<GS::UniString>::ConstIterator it = modulNameList.Enumerate (); it != NULL; ++it) {
		const GS::UniString& oldModulName = *it;
		API_ModulData oldModulData;
		BNZeroMemory (&oldModulData, sizeof (API_ModulData));
		if (ACAPI_ModulData_GetInfo (&oldModulData, oldModulName) == NoError) {
			if (oldModulData.dataVersion < 1) {						// old version (all customers stored in the same moduldata)
				if (ACAPI_ModulData_Get (&oldModulData, oldModulName) == NoError) {
					IO::MemoryIChannel oldModulMemChannel (*oldModulData.dataHdl, BMGetHandleSize (oldModulData.dataHdl));
					if (oldModulData.platformSign != GS::Act_Platform_Sign) {
						GS::PlatformSign ps = (oldModulData.platformSign == GS::Win_Platform_Sign) ? GS::Win_Platform_Sign :
											  ((oldModulData.platformSign == GS::Mac_Platform_Sign) ? GS::Mac_Platform_Sign : GS::Mactel_Platform_Sign);
						IO::SetPlatformIProtocol (oldModulMemChannel, ps);
					}
					USize nObjects = 0;
					oldModulMemChannel.Read (nObjects);
					for (USize i = 0; i < nObjects; i++) {
						CustomerData customerData;
						customerData.guid.Generate ();
						customerData.Read (oldModulMemChannel);

						API_ModulData modulData;
						BNZeroMemory (&modulData, sizeof (API_ModulData));
						modulData.dataVersion = 1;
						modulData.platformSign = GS::Act_Platform_Sign;
						IO::MemoryOChannel memChannel;
						if (customerData.Write (memChannel) == NoError) {
							BMPtrToHandle (memChannel.GetDestination (), &modulData.dataHdl, memChannel.GetDataSize ());
							ACAPI_ModulData_Store (&modulData, customerData.guid.ToUniString ());	// store converted
							BMKillHandle (&modulData.dataHdl);
						}
					}
				}
				BMKillHandle (&oldModulData.dataHdl);
				ACAPI_ModulData_Delete (oldModulName);												// delete old format
			}
		}
	}
}		// ConvertModulesFromOldFormat


// -----------------------------------------------------------------------------
// Manage customer data in a modal dialog
// -----------------------------------------------------------------------------
#define OKButtonId				1
#define CancelButtonId			2
#define CustomerListBoxId		3
#define NewButtonId				4
#define ModifyButtonId			5
#define DeleteButtonId			6

static short DGCALLBACK ManagerDialogCallback (short message, short dialId, short item, DGUserData, DGMessageData)
{
	switch (message) {
		case DG_MSG_INIT:
			{
				SetTabData (dialId, CustomerListBoxId);

				ConvertModulesFromOldFormat ();

				GS::Array<GS::UniString> modulNameList;
				ACAPI_ModulData_GetList (&modulNameList);

				DGListDeleteItem (dialId, CustomerListBoxId, DG_ALL_ITEMS);
				for (GS::Array<GS::UniString>::ConstIterator it = modulNameList.Enumerate (); it != NULL; ++it) {
					API_ModulData modulData;
					BNZeroMemory (&modulData, sizeof (API_ModulData));
					if (ACAPI_ModulData_Get (&modulData, *it) == NoError) {
						CustomerData* customerData = new CustomerData (modulData, *it);
						DGListInsertItem (dialId, CustomerListBoxId, DG_LIST_BOTTOM);
						GS::UniString itemString = customerData->name + "\t" + customerData->city + ", " + customerData->country;
						DGListSetItemText (dialId, CustomerListBoxId, DG_LIST_BOTTOM, itemString.ToCStr ().Get ());
						DGListSetItemUserData (dialId, CustomerListBoxId, DG_LIST_BOTTOM, reinterpret_cast<DGUserData> (customerData));
					}
					BMKillHandle (&modulData.dataHdl);
				}
			}
			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
				case DG_CANCEL:
					return item;

				case NewButtonId: {
					CustomerData* customerData = new CustomerData ();
					if (customerData != NULL) {
						customerData->guid.Generate ();
						customerData->GenerateRandomContent ();
						customerData->modified = true;
						DGListInsertItem (dialId, CustomerListBoxId, DG_LIST_BOTTOM);
						GS::UniString itemString = customerData->name + "\t" + customerData->city + ", " + customerData->country;
						DGListSetItemText (dialId, CustomerListBoxId, DG_LIST_BOTTOM, itemString.ToCStr ().Get ());
						DGListSetItemUserData (dialId, CustomerListBoxId, DG_LIST_BOTTOM, reinterpret_cast<DGUserData> (customerData));
					}
					break;
				}

				case ModifyButtonId: {
					short listItem = DGListGetSelected (dialId, CustomerListBoxId, 0);
					if (listItem > 0) {
						CustomerData* customerData = reinterpret_cast<CustomerData*> (DGListGetItemUserData (dialId, CustomerListBoxId, listItem));
						if (!customerData->markedAsDeleted) {
							customerData->GenerateRandomContent ();
							customerData->modified = true;
							GS::UniString itemString = customerData->name + "\t" + customerData->city + ", " + customerData->country;
							DGListSetItemText (dialId, CustomerListBoxId, listItem, itemString.ToCStr ().Get ());
							DGListSetItemColor (dialId, CustomerListBoxId, listItem, 60000, 0, 0);
						}
					}
					break;
				}

				case DeleteButtonId: {
					short listItem = DGListGetSelected (dialId, CustomerListBoxId, 0);
					if (listItem > 0) {
						CustomerData* customerData = reinterpret_cast<CustomerData*> (DGListGetItemUserData (dialId, CustomerListBoxId, listItem));
						customerData->markedAsDeleted = true;
						DGListSetItemColor (dialId, CustomerListBoxId, listItem, 50000, 50000, 50000);
					}
					break;
				}
			}
			break;

		case DG_MSG_CLOSE:
			for (short listItem = 1; listItem <= DGListGetItemCount (dialId, CustomerListBoxId); listItem++) {
				CustomerData* customerData = reinterpret_cast<CustomerData*> (DGListGetItemUserData (dialId, CustomerListBoxId, listItem));
				if (item == DG_OK) {
					if (customerData->markedAsDeleted) {
						ACAPI_ModulData_Delete (customerData->guid.ToUniString ());
					} else if (customerData->modified) {
						API_ModulData modulData;
						BNZeroMemory (&modulData, sizeof (API_ModulData));
						modulData.dataVersion = 1;
						modulData.platformSign = GS::Act_Platform_Sign;
						IO::MemoryOChannel memChannel;
						if (customerData->Write (memChannel) == NoError) {
							BMPtrToHandle (memChannel.GetDestination (), &modulData.dataHdl, memChannel.GetDataSize ());
							ACAPI_ModulData_Store (&modulData, customerData->guid.ToUniString ());
							BMKillHandle (&modulData.dataHdl);
						}
					}
				}
				delete customerData;
			}
			break;
	}

	return 0;
}		// ManagerDialogCallback

#undef OKButtonId
#undef CancelButtonId
#undef CustomerListBoxId
#undef NewButtonId
#undef ModifyButtonId
#undef DeleteButtonId

// -----------------------------------------------------------------------------
// DoCommand
//		called to perform the user-asked command
// -----------------------------------------------------------------------------

GSErrCode	__ACENV_CALL	MenuCommandHandler (const API_MenuParams *params)
{
	if (params->menuItemRef.itemIndex == 1)
		DGModalDialog (ACAPI_GetOwnResModule (), 32500, ACAPI_GetOwnResModule (), ManagerDialogCallback, 0);

	return NoError;
}		// MenuCommandHandler


// -----------------------------------------------------------------------------
// ModulData Merge event handler
// -----------------------------------------------------------------------------

GSErrCode	__ACENV_CALL	APIModulDataMergeHandler (API_MDSetCmd			mode,
													  const GS::UniString&	/*modulName*/,
													  const API_ModulData*	originalInfo,
													  const API_ModulData*	toMergeInfo,
													  API_ModulData*		destInfo)
{
	if (toMergeInfo == NULL || destInfo == NULL)
		return APIERR_BADPARS;

	GSErrCode err = NoError;

	const API_ModulData* modulDataToKeep = (mode == APIMDSetMerge_TW || originalInfo == NULL) ? toMergeInfo : originalInfo;

	if (modulDataToKeep->dataHdl != NULL) {
		BNZeroMemory (destInfo, sizeof (API_ModulData));
		destInfo->dataVersion = modulDataToKeep->dataVersion;
		destInfo->platformSign = modulDataToKeep->platformSign;
		BMHandleToHandle (modulDataToKeep->dataHdl, &destInfo->dataHdl);
	} else
		err = APIERR_BADPARS;

	return err;
}		// APIModulDataMergeHandler


// -----------------------------------------------------------------------------
// ModulData Save into old format project file
// -----------------------------------------------------------------------------

GSErrCode	__ACENV_CALL	APIModulDataSaveAsOldFormat (API_FTypeID									planFileType,
														 GS::HashTable<GS::UniString, API_ModulData*>&	modulesToSave)
{
	GSErrCode err = NoError;

	if (planFileType == APIFType_PlanFile1200 || planFileType == APIFType_PlanFile1300) {
		GS::Array<GS::UniString> modulNameList;
		ACAPI_ModulData_GetList (&modulNameList);

		if (!modulNameList.IsEmpty ()) {
			API_ModulData* oldFormatModulData = new API_ModulData;
			if (oldFormatModulData != NULL) {
				BNZeroMemory (oldFormatModulData, sizeof (API_ModulData));
				oldFormatModulData->dataVersion = 0;
				oldFormatModulData->platformSign = GS::Act_Platform_Sign;

				IO::MemoryOChannel memChannel;
				USize nObjects = modulNameList.GetSize ();
				memChannel.Write (nObjects);

				for (GS::Array<GS::UniString>::ConstIterator it = modulNameList.Enumerate (); it != NULL; ++it) {
					API_ModulData modulData;
					BNZeroMemory (&modulData, sizeof (API_ModulData));
					if (ACAPI_ModulData_Get (&modulData, *it) == NoError) {
						CustomerData customerData (modulData, *it);
						customerData.Write (memChannel);
					}
					BMKillHandle (&modulData.dataHdl);
				}

				BMPtrToHandle (memChannel.GetDestination (), &oldFormatModulData->dataHdl, memChannel.GetDataSize ());
				if (oldFormatModulData->dataHdl != NULL)
					modulesToSave.Put ("", oldFormatModulData);		// Save As 12 project - only one modul data is handled per add-on
				else
					err = APIERR_MEMFULL;
			}
		} else {
			err = APIERR_NOMODULEDATA;
		}
	}

	return err;
}		// APIModulDataSaveAsOldFormat



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
	ACAPI_Register_Menu (32500, 0, MenuCode_UserDef, MenuFlag_Default);
	ACAPI_Register_ModulDataHandler ();

	return NoError;
}		// RegisterInterface


// -----------------------------------------------------------------------------
// Initialize
//		called after the Add-On has been loaded into memory
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	Initialize (void)
{
	GSErrCode err = ACAPI_Install_MenuHandler (32500, MenuCommandHandler);
	if (err == NoError)
		err = ACAPI_Install_ModulDataMergeHandler (APIModulDataMergeHandler);
	if (err == NoError)
		err = ACAPI_Install_ModulDataSaveOldFormatHandler (APIModulDataSaveAsOldFormat);

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

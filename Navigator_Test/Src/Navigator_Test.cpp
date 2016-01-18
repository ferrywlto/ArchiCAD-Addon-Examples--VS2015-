// *****************************************************************************
// AddOn to test the API Navigator
//
// Classes:        Contact person:
//
// [SG compatible]
// *****************************************************************************

#define _API_NAVIGATOR_TRANSL_

#if !defined (ACExtension)
#define	ACExtension
#endif

#if defined (_MSC_VER)
	#if !defined (WINDOWS)
		#define WINDOWS
	#endif
#endif

#if defined (WINDOWS)
	#include "Win32Interface.hpp"
	#pragma warning (disable: 4068)
#endif

// ------------------------------ Includes -------------------------------------

#include	<stdio.h>
#include	<math.h>
#include	<string.h>

#if defined (macintosh)
#include <CoreServices/CoreServices.h>
#endif

#include	"HashSet.hpp"

#include	"DG.h"
#include	"DGModule.hpp"

#include	"ACAPinc.h"

#include	"Array.hpp"

#include	"APIdefs_Database.h"


// ------------------------------ Constants ------------------------------------

const short NavCloseButton		= 1;
const short NavUpdateButton		= 2;
const short NavDeleteButton		= 3;
const short NavPublishButton	= 4;
const short NavTreeView			= 5;
const short LayerText			= 6;
const short ScaleText			= 7;
const short ZoomText			= 8;
const short DisplayOpText		= 9;
const short NavPopUp			= 10;
const short NavCustomNameText	= 11;
const short NavRenameButton		= 12;

const short BookIcon			= 32408;
const short ProjectLinkIcon		= 32409;
const short StoryLinkIcon		= 32410;
const short SectionLinkIcon		= 32411;
const short DetailLinkIcon		= 32412;
const short LayoutLinkIcon		= 32413;
const short MasterLinkIcon		= 32414;
const short Axon3DLinkIcon		= 32415;
const short Persp3DLinkIcon		= 32416;
const short ScheduleLinkIcon	= 32417;
const short ListIcon			= 32418;
const short HelpIcon			= 32419;
const short OtherIcon			= 32420;
const short MasterFolderIcon	= 32421;
const short WorksheetLinkIcon	= 32422;
const short SubsetIcon			= 32423;
const short TOCIcon				= 32424;
const short CameraIcon			= 32425;
const short CameraSetIcon		= 32426;
const short InfoIcon			= 32427;
const short ElevationIcon		= 32428;
const short IntElevationIcon	= 32429;
const short DocumentFrom3DIcon	= 32430;

// -------------------------------- Types --------------------------------------

class NavigatorTestGlobals
{
private:
	short						navigatorDialID;
	API_NavigatorMapID			navigatorMapID;
	GS::PagedArray<API_Guid>	navigatorItems;
	GS::HashSet<API_Guid>		openedItems;

private:
	NavigatorTestGlobals () : navigatorDialID (0), navigatorMapID (API_ProjectMap) {}

public:
	~NavigatorTestGlobals () { navigatorDialID = 0; navigatorMapID = API_ProjectMap; navigatorItems.Clear (); }

	static NavigatorTestGlobals& Instance ();

	void				SetNavigatorDialID	(short dialID)				{ navigatorDialID = dialID; }
	short				GetNavigatorDialID	()							{ return navigatorDialID; }
	void				SetNavigatorMapID	(API_NavigatorMapID mapID)	{ navigatorMapID = mapID; }
	API_NavigatorMapID	GetNavigatorMapID	()							{ return navigatorMapID; }
	UIndex				AddNavigatorItem	(API_Guid item)				{ navigatorItems.Push (item); return (navigatorItems.GetSize () - 1); }
	void				DeleteNavigatorItem	(UIndex itemIndex)			{ if (itemIndex < navigatorItems.GetSize ()) { navigatorItems.Delete (itemIndex); } }
	API_Guid			GetNavigatorItem	(UIndex itemIndex)			{ if (itemIndex < navigatorItems.GetSize ()) { return navigatorItems[itemIndex]; } else { return APINULLGuid; } }

	void				SetNavigatorItemOpen (UIndex itemIndex, bool open)
	{
		if (itemIndex < navigatorItems.GetSize ()) {
			if (open)
				openedItems.Add (navigatorItems[itemIndex]);
			else
				openedItems.Delete (navigatorItems[itemIndex]);
		}
	}
	bool				IsNavigatorItemOpen	(UIndex itemIndex)			{ if (itemIndex < navigatorItems.GetSize ()) { return openedItems.Contains (navigatorItems[itemIndex]); } else { return false; } }
	void				ResetOpened (void)								{ openedItems.Clear (); }
};

NavigatorTestGlobals& NavigatorTestGlobals::Instance ()
{
	static NavigatorTestGlobals navigatorTestGlobals;
	return navigatorTestGlobals;
}

// ------------------------------ Variables ------------------------------------


// ------------------------------ Prototypes -----------------------------------


// ============================================================================
// ============================================================================
static GSErrCode __ACENV_CALL	APINavigatorPaletteAPIControlCallBack (Int32 referenceID, API_PaletteMessageID messageID, GS::IntPtr /*param*/)
{
	short navigatorDialID = NavigatorTestGlobals::Instance ().GetNavigatorDialID ();
	if (referenceID == navigatorDialID) {
		switch (messageID) {
			case APIPalMsg_ClosePalette:		DGModelessClose (navigatorDialID);
												break;
			case APIPalMsg_HidePalette_Begin:	DGHideModelessDialog (navigatorDialID);
												break;
			case APIPalMsg_HidePalette_End:		DGShowModelessDialog (navigatorDialID, DG_DF_FIRST);
												break;
			case APIPalMsg_DisableItems_Begin:
			case APIPalMsg_DisableItems_End:	// actually do nothing, because the input finish functionality the buttons have to stay enabled
			case APIPalMsg_IsPaletteVisible:
			case APIPalMsg_OpenPalette:			break;
		}
	}

	return NoError;
}		/* APINavigatorPaletteAPIControlCallBack */


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
static void DelTreeData (short dialogID, short	treeItem, Int32 parent)
{
	Int32 child = DGTreeViewGetItem (dialogID, treeItem, parent, DG_TVG_CHILD);
	while (child != DG_TVI_NONE) {
		DelTreeData (dialogID, treeItem, child);
		child = DGTreeViewGetItem (dialogID, treeItem, child, DG_TVG_NEXT);
	}

	if (parent != DG_TVI_ROOT) {
		UIndex itemIndex = (UIndex) DGTreeViewGetItemUserData (dialogID, NavTreeView, parent);
		NavigatorTestGlobals::Instance ().DeleteNavigatorItem (itemIndex);
	}
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
static Int32 InsertTreeItem (short dialogID, short	treeItem, Int32 parent, char* name, char* uiId, const API_Guid& guid, API_NavigatorItemTypeID itemType)
{
	Int32 newItem = DGTreeViewInsertItem (dialogID, treeItem, parent, DG_TVI_BOTTOM);

	char buffer[1024];
	if (uiId == NULL || uiId[0] == '\0')
		sprintf (buffer, "%s", name);
	else
		sprintf (buffer, "%s %s", uiId, name);
	DGTreeViewSetItemText (dialogID, treeItem, newItem, buffer);

	short  iconId = OtherIcon;
	switch (itemType) {
		case API_ProjectNavItem:			iconId = ProjectLinkIcon;		break;
		case API_StoryNavItem:				iconId = StoryLinkIcon;			break;
		case API_SectionNavItem:			iconId = SectionLinkIcon;		break;
		case API_DetailDrawingNavItem:		iconId = DetailLinkIcon;		break;
		case API_PerspectiveNavItem:		iconId = Persp3DLinkIcon;		break;
		case API_AxonometryNavItem:			iconId = Axon3DLinkIcon;		break;
		case API_ListNavItem:				iconId = ListIcon;				break;
		case API_ScheduleNavItem:			iconId = ScheduleLinkIcon;		break;
		case API_TocNavItem:				iconId = TOCIcon;				break;
		case API_CameraNavItem:				iconId = CameraIcon;			break;
		case API_CameraSetNavItem:			iconId = CameraSetIcon;			break;
		case API_InfoNavItem:				iconId = InfoIcon;				break;
		case API_HelpNavItem:				iconId = HelpIcon;				break;
		case API_LayoutNavItem:				iconId = LayoutLinkIcon;		break;
		case API_MasterLayoutNavItem:		iconId = MasterLinkIcon;		break;
		case API_BookNavItem:				iconId = BookIcon;				break;
		case API_MasterFolderNavItem:		iconId = MasterFolderIcon;		break;
		case API_SubSetNavItem:				iconId = SubsetIcon;			break;
		case API_TextListNavItem:			iconId = ListIcon;				break;
		case API_ElevationNavItem:			iconId = ElevationIcon;			break;
		case API_InteriorElevationNavItem:	iconId = IntElevationIcon;		break;
		case API_WorksheetDrawingNavItem:	iconId = WorksheetLinkIcon;		break;
		case API_DocumentFrom3DNavItem:		iconId = DocumentFrom3DIcon;	break;
		case API_FolderNavItem:				iconId = OtherIcon;				break;
		case API_UndefinedNavItem:			iconId = OtherIcon;				break;
	}

	DGTreeViewSetItemIcon (dialogID, treeItem, newItem, DG_TVIT_NORMAL, DG::Icon (ACAPI_GetOwnResModule (), iconId));
	DGTreeViewSetItemUserData (dialogID, treeItem, newItem, NavigatorTestGlobals::Instance ().AddNavigatorItem (guid));

	return newItem;
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
static void MakeTree (short dialogID, short	treeItem, Int32 parent, const API_Guid& guid, GS::PagedArray<Int32>& itemsArray)
{
	API_NavigatorItem** items = NULL;
	API_NavigatorItem item;

	BNZeroMemory (&item, sizeof (API_NavigatorItem));
	item.guid = guid;
	item.mapId = NavigatorTestGlobals::Instance ().GetNavigatorMapID ();

	GSErrCode err = ACAPI_Environment (APIEnv_GetNavigatorChildrenItemsID, &item, &items);
	if (err != NoError || items == NULL)
		return;

	Int32 n = BMhGetSize ((GSHandle)items) / Sizeof32 (API_NavigatorItem);
	for (Int32 i = 0; i < n; i++) {
		Int32 newItem = InsertTreeItem (dialogID, treeItem, parent, (*items)[i].name, (*items)[i].uiId, (*items)[i].guid, (*items)[i].itemType);
		if (newItem != 0) {
			itemsArray.Push (newItem);
			MakeTree (dialogID, treeItem, newItem, (*items)[i].guid, itemsArray);
		}
	}

	BMhKill ((GSHandle *) &items);
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
static void UpdateTree (short dialogID)
{
	GS::PagedArray<Int32>	items;

	API_NavigatorSet	set;
	API_NavigatorMapID	navigatorMapID = NavigatorTestGlobals::Instance ().GetNavigatorMapID ();

	Int32		nSet = 1;
	Int32		newItem = 0;
	Int32		i = 0;
	GSErrCode	err = NoError;

	DGTreeViewDisableDraw (dialogID, NavTreeView);

	DelTreeData (dialogID, NavTreeView, DG_TVI_ROOT);
	DGTreeViewDeleteItem (dialogID, NavTreeView, DG_ALL_ITEMS);
	BNZeroMemory (&set, sizeof (API_NavigatorSet));

	bool isPublisher = (navigatorMapID == API_PublisherSets);
	bool isItemSelected = (DGTreeViewGetSelected (dialogID, NavTreeView, DG_TVI_ROOT) != DG_TVI_NONE);

	DGSetItemEnable (dialogID, NavPublishButton, isPublisher);
	DGSetItemEnable (dialogID, NavRenameButton, isPublisher && isItemSelected);
	DGSetItemEnable (dialogID, NavCustomNameText, isPublisher && isItemSelected);

	bool deleteButtonEnabled = (navigatorMapID == API_PublicViewMap || navigatorMapID == API_MyViewMap) && isItemSelected;
	DGSetItemEnable (dialogID, NavDeleteButton, deleteButtonEnabled);

	if (navigatorMapID == API_PublisherSets) {
		err = ACAPI_Environment (APIEnv_GetNavigatorSetNumID, &nSet, NULL);
		if (err != NoError) {
			DGTreeViewEnableDraw (dialogID, NavTreeView);
			DGRedrawItem (dialogID, NavTreeView);
			return;
		}
	}

	for (i = 0; i < nSet; i++) {
		set.mapId = navigatorMapID;
		set.wantsExtraInfo = (navigatorMapID == API_PublisherSets) ? 1 : 0;
		err = ACAPI_Environment (APIEnv_GetNavigatorSetID, &set, &i);
		if (err != NoError) {
			DGTreeViewEnableDraw (dialogID, NavTreeView);
			DGRedrawItem (dialogID, NavTreeView);
			return;
		}

		if (navigatorMapID == API_PublisherSets) {
			if (set.path != NULL) {
				GS::UniString outPath;
				set.path->ToDisplayText (&outPath);
				DBPrintf ("Navigator set \"%s\" is published to LAN. Path: \"%s\"\n", set.name, (const char *) outPath.ToCStr ());
				delete set.path;
			} else if (set.host != NULL) {
				DBPrintf ("Navigator set \"%s\" is published to Internet. Host: \"%s\", directory: \"%s\"\n",
					set.name, (const char *) set.host->ToCStr (), (const char *) set.dirName->ToCStr ());
				delete set.host;
				delete set.dirName;
			}
		}

		if (navigatorMapID == API_PublisherSets && nSet > 1) {
			newItem = InsertTreeItem (dialogID, NavTreeView, DG_TVI_ROOT, set.name, NULL, set.rootGuid, API_UndefinedNavItem);
			items.Push (newItem);
		}

		MakeTree (dialogID, NavTreeView, newItem, set.rootGuid, items);
	}

	// Update expanded/collapsed state
	for (GS::PagedArray<Int32>::ConstIterator ii = items.Enumerate (); ii != NULL; ++ii) {
		if (NavigatorTestGlobals::Instance ().IsNavigatorItemOpen (TruncateTo32Bit (DGTreeViewGetItemUserData (dialogID, NavTreeView, *ii))))
			DGTreeViewExpandItem (dialogID, NavTreeView, *ii);
	}

	DGTreeViewEnableDraw (dialogID, NavTreeView);
	DGRedrawItem (dialogID, NavTreeView);
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
static void RenamePublisherItem (short dialogID)
{
	Int32 selected = DGTreeViewGetSelected (dialogID, NavTreeView, DG_TVI_ROOT);
	if (DBERROR (selected == DG_TVI_NONE))
		return;

	GS::Array<API_Guid> toRenameItems;
	API_Guid guid = NavigatorTestGlobals::Instance ().GetNavigatorItem ((UIndex) DGTreeViewGetItemUserData (dialogID, NavTreeView, selected));
	toRenameItems.Push (guid);

	for (;;) {
		selected = DGTreeViewGetItem (dialogID, NavTreeView, selected, DG_TVG_NEXTLISTED);
		selected = DGTreeViewGetSelected (dialogID, NavTreeView, selected);
		if (selected == DG_TVI_NONE)
			break;
		guid = NavigatorTestGlobals::Instance ().GetNavigatorItem ((UIndex) DGTreeViewGetItemUserData (dialogID, NavTreeView, selected));
		toRenameItems.Push (guid);
	}

	GS::UniString customName = DGGetItemText (dialogID, NavCustomNameText);
	for (USize i = 0; i < toRenameItems.GetSize (); ++i) {
		API_NavigatorItem navItem;

		BNZeroMemory (&navItem, sizeof (navItem));
		navItem.mapId = API_PublisherSets;
		GSErrCode err = ACAPI_Environment (APIEnv_GetNavigatorItemID, &toRenameItems[i], &navItem);
		if (err != NoError)
			continue;
		if (customName.IsEmpty ())
			navItem.customName = false;
		else {
			CHCopyC (customName.ToCStr ().Get (), navItem.name);
			navItem.customName = true;
		}
		ACAPI_Environment (APIEnv_ChangeNavigatorItemID, &navItem, NULL);
	}

	UpdateTree (dialogID);
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
static void DeleteView (short dialogID)
{
	Int32 selected = DGTreeViewGetSelected (dialogID, NavTreeView, DG_TVI_ROOT);
	if (DBERROR (selected == DG_TVI_NONE))
		return;

	GS::Array<API_Guid> toDeleteItems;
	API_Guid guid = NavigatorTestGlobals::Instance ().GetNavigatorItem ((UIndex) DGTreeViewGetItemUserData (dialogID, NavTreeView, selected));
	toDeleteItems.Push (guid);

	for (;;) {
		selected = DGTreeViewGetItem (dialogID, NavTreeView, selected, DG_TVG_NEXTLISTED);
		selected = DGTreeViewGetSelected (dialogID, NavTreeView, selected);
		if (selected == DG_TVI_NONE)
			break;
		guid = NavigatorTestGlobals::Instance ().GetNavigatorItem ((UIndex) DGTreeViewGetItemUserData (dialogID, NavTreeView, selected));
		toDeleteItems.Push (guid);
	}

	bool silentMode = true;
	for (USize i = 0; i < toDeleteItems.GetSize (); ++i)
		ACAPI_Environment (APIEnv_DeleteNavigatorViewID, &toDeleteItems[i], &silentMode);

	UpdateTree (dialogID);
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
inline bool Change_Layer (short	layerID, short status)
{
	API_Attribute	layer;

	BNZeroMemory (&layer, sizeof (layer));
	layer.header.typeID = API_LayerID;
	layer.header.index  = layerID;
	GSErrCode err = ACAPI_Attribute_Get (&layer);
	if (err == NoError) {
		if ((status & APILay_Hidden) != (layer.layer.head.flags & APILay_Hidden)) {
			//layer.layer.head.flags &= !APILay_Locked;

			layer.layer.head.flags &= !APILay_Hidden;
			layer.layer.head.flags |= (status & APILay_Hidden);

			err = ACAPI_Attribute_Modify (&layer, NULL);
			return (err == NoError);
		}
	}

	return false;
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
inline bool Change_LayerComb (char* layerCombName)
{
	API_Attribute	layerComb;

	BNZeroMemory (&layerComb, sizeof (layerComb));
	layerComb.header.typeID = API_LayerCombID;
	CHCopyC (layerCombName, layerComb.header.name);
	GSErrCode err = ACAPI_Attribute_Get (&layerComb);
	if (err != NoError)
		return false;

	API_AttributeDef	attrDef;

	BNZeroMemory (&attrDef, sizeof (attrDef));
	err = ACAPI_Attribute_GetDef (API_LayerCombID, layerComb.header.index, &attrDef);
	if (err != NoError)
		return false;

	bool	change = false;
	Int32	nLayer = BMhGetSize ((GSHandle) attrDef.layer_statItems) / Sizeof32 (API_LayerStat);

	for (Int32 i = 0; i < nLayer; i++)
		change = change || Change_Layer ((*attrDef.layer_statItems)[i].lInd, (*attrDef.layer_statItems)[i].lFlags);

	BMhKill ((GSHandle*) &attrDef.layer_statItems);

	return change;
}

//------------------------------------------------------
//------------------------------------------------------
static void PublishTest ()
{
	short dialogID = NavigatorTestGlobals::Instance ().GetNavigatorDialID ();
	ULong selected = DGTreeViewGetSelected (dialogID, NavTreeView, DG_TVI_ROOT);
	if (selected != DG_TVI_NONE) {
		API_Guid guid = NavigatorTestGlobals::Instance ().GetNavigatorItem ((UIndex) DGTreeViewGetItemUserData (dialogID, NavTreeView, selected));
		if (guid != GS::NULLGuid) {
			API_NavigatorItem navItem = {0};
			navItem.mapId = NavigatorTestGlobals::Instance ().GetNavigatorMapID ();
			GSErrCode err = ACAPI_Environment (APIEnv_GetNavigatorItemID, &guid, &navItem);

			if (err == NoError) {
				API_PublishPars	publishPars;
				BNZeroMemory (&publishPars, sizeof (publishPars));
				publishPars.guid = guid;
				// PublishTest #1: publish with default parameters
				err = ACAPI_Automate (APIDo_PublishID, &publishPars, NULL);

				if (err == NoError) {
					GS::Array<API_Guid> toPublishItems;
					for (;;) {
						selected = DGTreeViewGetItem (dialogID, NavTreeView, selected, DG_TVG_NEXTLISTED);
						selected = DGTreeViewGetSelected (dialogID, NavTreeView, selected);
						if (selected == DG_TVI_NONE)
							break;
						API_Guid guid = NavigatorTestGlobals::Instance ().GetNavigatorItem ((UIndex) DGTreeViewGetItemUserData (dialogID, NavTreeView, selected));
						toPublishItems.Push (guid);
					}

					// PublishTest #2: publish to path
					publishPars.path = new IO::Location ("C:\\Work\\PublishTemp");
					err = ACAPI_Automate (APIDo_PublishID, &publishPars, &toPublishItems);
					delete publishPars.path;
				}
			}
		}
	}
}		// PublishTest


// -----------------------------------------------------------------------------
// Navigator Test palette handler (callback)
// -----------------------------------------------------------------------------
static short DGCALLBACK APINavigator_Handler (short				message,
											  short 			dialogID,
											  short				item,
											  DGUserData		/*userData*/,
											  DGMessageData		msgData)
{
	short ret = 0;

	switch (message) {
		case DG_MSG_INIT:
			if (ACAPI_RegisterModelessWindow (dialogID, APINavigatorPaletteAPIControlCallBack,
					API_PalEnabled_FloorPlan + API_PalEnabled_Section + API_PalEnabled_Elevation +
					API_PalEnabled_InteriorElevation + API_PalEnabled_Detail + API_PalEnabled_Worksheet + API_PalEnabled_Layout +
					API_PalEnabled_3D) != NoError) {
				DBPrintf ("Navigator_Test:: ACAPI_RegisterModelessWindow failed\n");
			}

			NavigatorTestGlobals::Instance ().ResetOpened ();
			UpdateTree (dialogID);

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case NavCloseButton:
					ret = item;
					break;

				case NavUpdateButton:
					UpdateTree (dialogID);
					break;

				case NavPublishButton:
					PublishTest ();
					break;

				case NavDeleteButton:
					DeleteView (dialogID);
					break;

				case NavRenameButton:
					RenamePublisherItem (dialogID);
					break;
			}
			break;
		case DG_MSG_DOUBLECLICK:
			if (item == NavTreeView) {
				Int32 selected = DGTreeViewGetSelected (dialogID, NavTreeView, DG_TVI_ROOT);
				API_Guid guid = NavigatorTestGlobals::Instance ().GetNavigatorItem (TruncateTo32Bit (DGTreeViewGetItemUserData (dialogID, NavTreeView, selected)));
				if (guid != GS::NULLGuid) {
					API_NavigatorItem navItem;

					BNZeroMemory (&navItem, sizeof (navItem));
					navItem.mapId = NavigatorTestGlobals::Instance ().GetNavigatorMapID ();
					GSErrCode err = ACAPI_Environment (APIEnv_GetNavigatorItemID, &guid, &navItem);

					if (err == NoError) {
						err = ACAPI_Automate (APIDo_ChangeWindowID, &navItem.db, NULL);
					}

					if (err == NoError && navItem.db.typeID == APIWind_FloorPlanID) {
						API_StoryCmdType cmd;

						BNZeroMemory (&cmd, sizeof (cmd));
						cmd.action = APIStory_GoTo;
						cmd.index = navItem.floorNum;
						err = ACAPI_Environment (APIEnv_ChangeStorySettingsID, &cmd, NULL);
					}

					if (err == NoError) {
						API_NavigatorView	view;
						API_NavigatorItem	item;

						BNZeroMemory (&view, sizeof (view));
						BNZeroMemory (&item, sizeof (item));
						item.guid = guid;
						item.mapId = NavigatorTestGlobals::Instance ().GetNavigatorMapID ();

						GSErrCode	err = ACAPI_Environment (APIEnv_GetNavigatorViewID, &item, &view);
						if (err == NoError) {
							bool needRebuild = false;
							if (view.saveLaySet) {
								if (view.layerCombination[0] != 0) {
									needRebuild = needRebuild || Change_LayerComb (view.layerCombination);
								} else {
									Int32 nLayer = BMhGetSize ((GSHandle)view.layerStats) / Sizeof32 (API_LayerStat);
									for (Int32 layer = 0; layer < nLayer; layer++) {
										needRebuild = needRebuild || Change_Layer ( (*view.layerStats)[layer].lInd,
																					(*view.layerStats)[layer].lFlags);
									}
								}
							}
							if (view.saveDispOpt) {
								API_ViewOptions viewOptions;
								BNZeroMemory (&viewOptions, sizeof (API_ViewOptions));
								err = ACAPI_Environment (APIEnv_GetViewOptionsID, &viewOptions, NULL);
								ACAPI_FreeGDLModelViewOptionsPtr (&viewOptions.modelViewOpt.gdlOptions);
								if (view.modelViewOpt != NULL)
									viewOptions.modelViewOpt = *view.modelViewOpt;
								err = ACAPI_Environment (APIEnv_ChangeViewOptionsID, &viewOptions, NULL);
							}

							if (view.modelViewOpt != NULL) {
								ACAPI_FreeGDLModelViewOptionsPtr (&view.modelViewOpt->gdlOptions);
								BMKillPtr ((GSPtr*) &view.modelViewOpt);
							}

							if (err == NoError && view.saveDScale) {
								double scale = view.drawingScale;
								err = ACAPI_Database (APIDb_ChangeDrawingScaleID, &scale, (void*) ((GS::IntPtr)(!view.saveZoom)));
							}
							if (err == NoError && view.saveZoom)
								err = ACAPI_Automate (APIDo_ZoomID, &view.zoom, NULL);

							BMhKill ((GSHandle*) &view.layerStats);
							BMKillPtr ((GSPtr*) &view.pens);

							if (needRebuild) {
								bool regener = true;
								err = ACAPI_Automate (APIDo_RebuildID, &regener, NULL);
							}
						}
					}
				}

			}
			break;

		case DG_MSG_CHANGE:
			switch (item) {
				case NavTreeView:
					{
						DGTreeViewChangeData* treeChange = (DGTreeViewChangeData*) msgData;
						if (treeChange->actionFlag == DG_TVCF_SELECTION) {
							Int32 selected = DGTreeViewGetSelected (dialogID, NavTreeView, DG_TVI_ROOT);
							if (selected == DG_TVI_NONE)
								break;

							API_NavigatorMapID	navigatorMapID = NavigatorTestGlobals::Instance ().GetNavigatorMapID ();
							DGSetItemEnable (dialogID, NavDeleteButton, navigatorMapID == API_PublicViewMap || navigatorMapID == API_MyViewMap);

							API_Guid guid = NavigatorTestGlobals::Instance ().GetNavigatorItem (TruncateTo32Bit (DGTreeViewGetItemUserData (dialogID, NavTreeView, selected)));
							if (guid != GS::NULLGuid) {
								char msgStr[256];
								API_SubSet subSet;
								BNZeroMemory (&subSet, sizeof (API_SubSet));

								GSErrCode err = ACAPI_Database (APIDb_GetSubSetID, &guid, &subSet);
								if (err == NoError) {
									GS::UniString name (subSet.name);
									GS::UniString customNumber (subSet.customNumber);
									GS::UniString autoNumber (subSet.autoNumber);
									GS::UniString ownPrefix (subSet.ownPrefix);
									sprintf (msgStr, "Name: %s, CustomNumber: %s, AutoNumber: %s, Prefix: %s",
											 name.ToCStr ().Get (), customNumber.ToCStr ().Get (),
											 autoNumber.ToCStr ().Get (), ownPrefix.ToCStr ().Get ());
									ACAPI_WriteReport (msgStr, false);
								}

								API_NavigatorView	view;
								API_NavigatorItem	item;

								BNZeroMemory (&view, sizeof (view));
								BNZeroMemory (&item, sizeof (item));
								item.guid = guid;
								item.mapId = NavigatorTestGlobals::Instance ().GetNavigatorMapID ();

								bool isPublisher = (item.mapId == API_PublisherSets);
								DGSetItemEnable (dialogID, NavRenameButton, isPublisher);
								DGSetItemEnable (dialogID, NavCustomNameText, isPublisher);
								if (isPublisher) {
									err = ACAPI_Environment (APIEnv_GetNavigatorItemID, &guid, &item);
									if (err == NoError)
										DGSetItemText (dialogID, NavCustomNameText, item.autoTextedName);
								} else
									DGSetItemText (dialogID, NavCustomNameText, "");

								err = ACAPI_Environment (APIEnv_GetNavigatorViewID, &item, &view);
								if (err == NoError) {
									if (view.modelViewOpt != NULL) {
										ACAPI_FreeGDLModelViewOptionsPtr (&view.modelViewOpt->gdlOptions);
										BMKillPtr ((GSPtr*) &view.modelViewOpt);
									}
									char str[256];
									if (view.saveLaySet) {
										if (view.layerCombination[0] != 0)
											DGSetItemText (dialogID, LayerText, view.layerCombination);
										else
											DGSetItemText (dialogID, LayerText, "Individual");
									} else {
										DGSetItemText (dialogID, LayerText, "None");
									}

									if (view.saveDScale) {
										sprintf (str, "1 : %ld", (GS::LongForStdio) view.drawingScale);
										DGSetItemText (dialogID, ScaleText, str);
									} else {
										DGSetItemText (dialogID, ScaleText, "None");
									}
									if (!view.ignoreSavedZoom) {
										if (view.saveZoom) {
											DGSetItemText (dialogID, ZoomText, "Zoomed area");
										} else {
											DGSetItemText (dialogID, ZoomText, "Fit in window");
										}
									}
									else {
										DGSetItemText (dialogID, ZoomText, "None");
									}
									if (view.saveDispOpt) {
										if (view.modelViewOptName[0] != 0)
											DGSetItemText (dialogID, DisplayOpText, view.modelViewOptName);
										else
											DGSetItemText (dialogID, DisplayOpText, "Display Options");
									} else {
										DGSetItemText (dialogID, DisplayOpText, "None");
									}
									BMhKill ((GSHandle*) &view.layerStats);
									BMKillPtr ((GSPtr*) &view.pens);
								}
							}
						} else if (treeChange->actionFlag == DG_TVCF_COLLAPSE || treeChange->actionFlag == DG_TVCF_EXPAND) {
							NavigatorTestGlobals::Instance ().SetNavigatorItemOpen (TruncateTo32Bit (DGTreeViewGetItemUserData (dialogID, NavTreeView, treeChange->treeItem)),
																					treeChange->actionFlag == DG_TVCF_EXPAND);
						}
					}
					break;
				case NavPopUp:
					{
						short sel = DGPopUpGetSelected (dialogID, NavPopUp);
						switch (sel) {
							case 1:
								NavigatorTestGlobals::Instance ().SetNavigatorMapID (API_ProjectMap);
								break;
							case 2:
								NavigatorTestGlobals::Instance ().SetNavigatorMapID (API_PublicViewMap);
								break;
							case 3:
								NavigatorTestGlobals::Instance ().SetNavigatorMapID (API_LayoutMap);
								break;
							case 4:
								NavigatorTestGlobals::Instance ().SetNavigatorMapID (API_PublisherSets);
								break;
							default:
								DBBREAK ();
						}
						NavigatorTestGlobals::Instance ().ResetOpened ();
						UpdateTree (dialogID);
					}
					break;

				default:
					break;
			}
			break;

		case DG_MSG_TRACK:
			break;

		case DG_MSG_DRAGDROP:
			{
				Int32 selected = DGTreeViewGetSelected (dialogID, NavTreeView, DG_TVI_ROOT);
				API_Guid selectedGuid = selected >= 0 ? NavigatorTestGlobals::Instance ().GetNavigatorItem (TruncateTo32Bit (DGTreeViewGetItemUserData (dialogID, NavTreeView, selected)))
													  : APINULLGuid;

				DGDragDropMsgData* ddData = reinterpret_cast<DGDragDropMsgData*> (msgData);

				if (ddData->subMessage == DG_DRAG_STARTING) {
					ret = 1;
				} else if (ddData->subMessage == DG_DRAG_START) {
					ddData->rightDragCtxMenu = false;
				} else if (ddData->subMessage == DG_DRAG_END) {
				} else if (ddData->subMessage == DG_DRAG_SETDELAYEDDATA) {
				} else if (ddData->subMessage == DG_DRAG_ENTER) {
					ddData->effect = DG_DRAG_ACCEPT;
				} else if (ddData->subMessage == DG_DRAG_LEAVE) {
				} else if (ddData->subMessage == DG_DRAG_DROP) {
					GS::Guid parentGuid = GS::NULLGuid;
					GS::Guid childGuid = GS::NULLGuid;
					API_Guid insertionGuid = NavigatorTestGlobals::Instance ().GetNavigatorItem (TruncateTo32Bit (DGTreeViewGetItemUserData (dialogID, NavTreeView, ddData->insertPos)));
					switch (ddData->relativePos) {
						case DG_DRAG_INSERT_ON:
							{
								parentGuid = APIGuid2GSGuid (insertionGuid);
								API_NavigatorItem item;
								BNZeroMemory (&item, sizeof (item));
								item.guid = insertionGuid;
								item.mapId = NavigatorTestGlobals::Instance ().GetNavigatorMapID ();
								API_NavigatorItem** items = NULL;
								ACAPI_Environment (APIEnv_GetNavigatorChildrenItemsID, &item, &items);
								if (items != NULL && *items != NULL) {
									Int32 n = BMhGetSize ((GSHandle)items) / Sizeof32 (API_NavigatorItem);
									childGuid = APIGuid2GSGuid ((*items)[n-1].guid);
								}
								BMhKill ((GSHandle *) &items);
							}
							break;
						case DG_DRAG_INSERT_UNDER:
							{
								childGuid = APIGuid2GSGuid (insertionGuid);
								Int32 parent = DGTreeViewGetItem (dialogID, NavTreeView, ddData->insertPos, DG_TVG_PARENT);
								parentGuid =  APIGuid2GSGuid (NavigatorTestGlobals::Instance ().GetNavigatorItem (TruncateTo32Bit (DGTreeViewGetItemUserData (dialogID, NavTreeView, parent))));
							}
							break;

						case DG_DRAG_INSERT_ABOVE:
							{
								Int32 parent = DGTreeViewGetItem (dialogID, NavTreeView, ddData->insertPos, DG_TVG_PARENT);
								parentGuid =  APIGuid2GSGuid (NavigatorTestGlobals::Instance ().GetNavigatorItem (TruncateTo32Bit (DGTreeViewGetItemUserData (dialogID, NavTreeView, parent))));
							}
							break;
					}
					GS::Guid sourceGuid = APIGuid2GSGuid (selectedGuid);
					if (ACAPI_Environment (APIEnv_SetNavigatorItemPositionID, &sourceGuid, &parentGuid, &childGuid) == NoError) {
						UpdateTree (dialogID);
					}
				}
			}
			break;

		case DG_MSG_GROW:
			{
				short	vgrow = DGGetVGrow (msgData);
				short	hgrow = DGGetHGrow (msgData);
				DGBeginMoveGrowItems (dialogID);

				DGMoveItem (dialogID, NavCloseButton, hgrow, vgrow);
				DGMoveItem (dialogID, NavUpdateButton, hgrow, vgrow);
				DGMoveItem (dialogID, NavDeleteButton, hgrow, vgrow);
				DGMoveItem (dialogID, NavPublishButton, hgrow, vgrow);

				DGGrowItem (dialogID, NavPopUp, hgrow, 0);
				DGGrowItem (dialogID, NavTreeView, hgrow, vgrow);
				DGMoveGrowItem (dialogID, LayerText, 0, vgrow, hgrow, 0);
				DGMoveGrowItem (dialogID, ScaleText, 0, vgrow, hgrow, 0);
				DGMoveGrowItem (dialogID, ZoomText, 0, vgrow, hgrow, 0);
				DGMoveGrowItem (dialogID, DisplayOpText, 0, vgrow, hgrow, 0);
				DGMoveGrowItem (dialogID, NavCustomNameText, 0, vgrow, hgrow, 0);
				DGMoveGrowItem (dialogID, NavRenameButton, 0, vgrow, hgrow, 0);

				DGEndMoveGrowItems (dialogID);

			}
			break;

		case DG_MSG_CLOSE:
			{
				DelTreeData (dialogID, NavTreeView, DG_TVI_ROOT);
				ACAPI_UnregisterModelessWindow (dialogID);
			}
			break;
	}

	return ret;
}		// APINavigator_Handler


//------------------------------------------------------
//------------------------------------------------------
static void Do_APINavigator (void)
{
	NavigatorTestGlobals::Instance ().SetNavigatorDialID (DGModelessInit (ACAPI_GetOwnResModule (), 32420, ACAPI_GetOwnResModule (), APINavigator_Handler, 0, 1));
	DGEnableMessage (NavigatorTestGlobals::Instance ().GetNavigatorDialID(), DG_ALL_ITEMS, DG_MSG_DRAGDROP);
}


//------------------------------------------------------
//------------------------------------------------------
GSErrCode __ACENV_CALL APIMenuCommandProc_32500 (const API_MenuParams * /*menuParams*/)
{
	Do_APINavigator ();

	return NoError;
}


// ============================================================================
// Required functions
//
//
// ============================================================================


//------------------------------------------------------
// Dependency definitions
//------------------------------------------------------
API_AddonType __ACENV_CALL	CheckEnvironment (API_EnvirParams* envir)
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
GSErrCode __ACENV_CALL	RegisterInterface (void)
{
	GSErrCode	err;

	// Register menus
	err = ACAPI_Register_Menu (32500, 0, MenuCode_UserDef, MenuFlag_Default);

	return err;
}		/* RegisterInterface */


//------------------------------------------------------
// Called when the Add-On has been loaded into memory
// to perform an operation
//------------------------------------------------------
GSErrCode __ACENV_CALL	Initialize (void)
{
	GSErrCode err = NoError;

	err = ACAPI_Install_MenuHandler (32500, APIMenuCommandProc_32500);

	return err;
}		/* Initialize */


//------------------------------------------------------
// Called when the Add-On is going to be unloaded
//------------------------------------------------------
GSErrCode __ACENV_CALL	FreeData (void)
{
	return NoError;
}		/* FreeData */

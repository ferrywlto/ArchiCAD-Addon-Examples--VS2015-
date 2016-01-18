// *****************************************************************************
// Source code for the Environment Control Add-On
// API Development Kit 19; Mac/Win
//
// Namespaces:			Contact person:
//		-None-
//
// [SG compatible] - Yes
// *****************************************************************************

#include "APIEnvir.h"
#define	_ENVIRONMENT_CONTROL_TRANSL_


// ---------------------------------- Includes ---------------------------------

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<math.h>

#include	"ACAPinc.h"					// also includes APIdefs.h

#include	"APICommon.h"

#include	"Location.hpp"


// ---------------------------------- Types ------------------------------------


// ---------------------------------- Variables --------------------------------


// ---------------------------------- Prototypes -------------------------------



// =============================================================================
//
//
//
// =============================================================================

// -----------------------------------------------------------------------------
// Testing various Environment functions
// -----------------------------------------------------------------------------
static void		Do_TestEnvironmentFunctions (void)
{
	{
		API_SharingInfo sharingInfo;
		BNZeroMemory (&sharingInfo, sizeof (API_SharingInfo));
		ACAPI_Environment (APIEnv_ProjectSharingID, &sharingInfo); 
	}
	{
		API_ProjectNoteInfo projectNoteInfo;
		BNZeroMemory (&projectNoteInfo, sizeof (API_ProjectNoteInfo));
		GSErrCode err = ACAPI_Environment (APIEnv_GetProjectNotesID, &projectNoteInfo); 
		if (err != NoError)
			WriteReport_Alert ("Error");
		
		err = ACAPI_Environment (APIEnv_ChangeProjectNotesID, &projectNoteInfo);
		if (err != NoError)
			WriteReport_Alert ("Error");
	}

	{
		GS::Array<API_LibraryInfo> activeLibs;
		GSErrCode err = ACAPI_Environment (APIEnv_GetLibrariesID, &activeLibs); 
		if (err != NoError)
			WriteReport_Alert ("Error");

		GS::Array<API_LibraryInfo> emptyLibs;
		err = ACAPI_Environment (APIEnv_SetLibrariesID, &emptyLibs); 
		if (err != NoError)
			WriteReport_Alert ("Error");

		err = ACAPI_Environment (APIEnv_SetLibrariesID, &activeLibs); 
		if (err != NoError)
			WriteReport_Alert ("Error");

		err = ACAPI_Environment (APIEnv_ResetLibrariesID, &activeLibs); 
		if (err != NoError)
			WriteReport_Alert ("Error");

		IO::Location libLocation ("D:\\Temp");
		err = ACAPI_Environment (APIEnv_AddLibrariesID, &libLocation);
		if (err != NoError)
			WriteReport_Alert ("Error");

		err = ACAPI_Environment (APIEnv_CheckLibrariesID);
		if (err != NoError)
			WriteReport_Alert ("Error");

		err = ACAPI_Environment (APIEnv_OverwriteLibPartID, (void*)true);
		if (err != NoError)
			WriteReport_Alert ("Error");

		err = ACAPI_Environment (APIEnv_OverwriteLibPartID, (void*)false);
		if (err != NoError)
			WriteReport_Alert ("Error");

		API_Guid guid;
		BNZeroMemory (&guid, sizeof (API_Guid));
		err = ACAPI_Environment (APIEnv_GetLibPartRefGuidID, &libLocation, &guid);
		if (err != NoError)
			WriteReport_Alert ("Error");

		char guidStr[512];
		ACAPI_Environment (APIEnv_GetLibPartRefGuidStrID, &libLocation, &guidStr);
	}
	{
		API_ToolBoxItem toolBoxItem;
		BNZeroMemory (&toolBoxItem, sizeof (API_ToolBoxItem));
		GSErrCode err = ACAPI_Environment (APIEnv_GetToolBoxModeID, &toolBoxItem);
		if (err != NoError)
			WriteReport_Alert ("Error");
	}
	{
		API_GhostStoryType storyInfo;
		BNZeroMemory (&storyInfo, sizeof (API_GhostStoryType));
		GSErrCode err = ACAPI_Environment (APIEnv_GetGhostStorySettingsID, &storyInfo);
		if (err != NoError)
			WriteReport_Alert ("Error");
	}
	{
		API_SunInfo sunInfo;
		BNZeroMemory (&sunInfo, sizeof (API_SunInfo));
		GSErrCode err = ACAPI_Environment (APIEnv_GetSunSetsID, &sunInfo);
		if (err != NoError)
			WriteReport_Alert ("Error");
	}
	{
		GSErrCode err = ACAPI_Environment (APIEnv_CanChangePlaceSetsID);
		if (err != NoError)
			WriteReport_Alert ("Error");
	}
	{
		API_PlaceInfo placeInfo;
		BNZeroMemory (&placeInfo, sizeof (API_PlaceInfo));

		GSErrCode err = ACAPI_Environment (APIEnv_GetPlaceSetsID, &placeInfo); 
		if (err != NoError)
			WriteReport_Alert ("Error");

		err = ACAPI_Environment (APIEnv_ChangePlaceSetsID, &placeInfo); 
		if (err != NoError)
			WriteReport_Alert ("Error");
	}
	{
		Int32 date[6];
		for (Int32 i = 0; i < 6; ++i)
			date[i] = 0;

		GSErrCode err = ACAPI_Environment (APIEnv_GetGDLDateID, &date); 
		if (err != NoError)
			WriteReport_Alert ("Error");

		date[5]++;

		err = ACAPI_Environment (APIEnv_ChangeGDLDateID, &date); 
		if (err != NoError)
			WriteReport_Alert ("Error");
	}
	{
		API_MagicWandInfo magicInfo;
		BNZeroMemory (&magicInfo, sizeof (API_MagicWandInfo));
		GSErrCode err = ACAPI_Environment (APIEnv_GetMagicWandSetsID, &magicInfo);
		if (err != NoError)
			WriteReport_Alert ("Error");
	}
	{
		double tolerance = 0.0;
		ACAPI_Environment (APIEnv_GetExportToleranceID, &tolerance);
	}
	{
		API_3DProjectionInfo projectionInfo;
		BNZeroMemory (&projectionInfo, sizeof (API_3DProjectionInfo));

		GSErrCode err = ACAPI_Environment (APIEnv_Get3DProjectionSetsID, &projectionInfo);
		if (err != NoError)
			WriteReport_Alert ("Error");

		projectionInfo.isPersp = true;

		err = ACAPI_Environment (APIEnv_Change3DProjectionSetsID, &projectionInfo);
		if (err != NoError)
			WriteReport_Alert ("Error");
	}
	{
		API_RendEffects rendEffects;
		BNZeroMemory (&rendEffects, sizeof (API_RendEffects));

		GSErrCode err = ACAPI_Environment (APIEnv_GetRenderingSetsID, &rendEffects, (void*) (Int32)APIRendSet_EffectsID);
		if (err != NoError)
			WriteReport_Alert ("Error");

		rendEffects.method = API_RendMethod_Final;

		err = ACAPI_Environment (APIEnv_ChangeRenderingSetsID, &rendEffects, (void*) (Int32)APIRendSet_EffectsID);
		if (err != NoError)
			WriteReport_Alert ("Error");
	}
	{
		API_3DImageInfo imageInfo;
		BNZeroMemory (&imageInfo, sizeof (API_3DImageInfo));

		GSErrCode err = ACAPI_Environment (APIEnv_Get3DImageSetsID, &imageInfo);
		if (err != NoError)
			WriteReport_Alert ("Error");

		imageInfo.allStories = true;

		err = ACAPI_Environment (APIEnv_Change3DImageSetsID, &imageInfo);
		if (err != NoError)
			WriteReport_Alert ("Error");
	}
	{
		API_3DWindowInfo windowInfo;
		BNZeroMemory (&windowInfo, sizeof (API_3DWindowInfo));

		GSErrCode err = ACAPI_Environment (APIEnv_Get3DWindowSetsID, &windowInfo);
		if (err != NoError)
			WriteReport_Alert ("Error");

		windowInfo.bkgAsInRendering = true;
		err = ACAPI_Environment (APIEnv_Change3DWindowSetsID, &windowInfo);
		if (err != NoError)
			WriteReport_Alert ("Error");
	}
	{
		bool isShown = false;
		GSErrCode err = ACAPI_Environment (APIEnv_GetShowHideStateID, &isShown);
		if (err != NoError)
			WriteReport_Alert ("Error");
	}
	{
		API_MiscPrinterInfo printerInfo;
		BNZeroMemory (&printerInfo, sizeof (API_MiscPrinterInfo));

		GSErrCode err = ACAPI_Environment (APIEnv_GetMiscPrinterInfoID, &printerInfo);
		if (err != NoError)
			WriteReport_Alert ("Error");

	}
	{
		API_MiscPlotterInfo info;
		BNZeroMemory (&info, sizeof (API_MiscPlotterInfo));

		GSErrCode err = ACAPI_Environment (APIEnv_GetMiscPlotterInfoID, &info);
		if (err != NoError)
			WriteReport_Alert ("Error");
	}
	{
		Int32 num = 0;
		GSErrCode err = ACAPI_Environment (APIEnv_GetNavigatorSetNumID, &num);
		if (err != NoError)
			WriteReport_Alert ("Error");
	}
	{
		API_Guid navigatorItemGuid;
		API_NavigatorItem navigatorItem;
		BNZeroMemory (&navigatorItem, sizeof (API_NavigatorItem));
		BNZeroMemory (&navigatorItemGuid, sizeof (API_Guid));
		ACAPI_Environment (APIEnv_GetNavigatorParentItemID, &navigatorItemGuid, &navigatorItem);
	}
	{
		API_NavigatorItem navigatorItem;
		API_NavigatorItem** foundItems = nullptr;
		ACAPI_Environment (APIEnv_SearchNavigatorItemID, &navigatorItem, &foundItems);
	}
	{
		short layerCombInd = 3;
		GSErrCode err = ACAPI_Environment (APIEnv_ChangeCurrLayerCombID, &layerCombInd);
		if (err != NoError)
			WriteReport_Alert ("Error");
	}
	{
		bool isAutoIntersectOn = false;
		GSErrCode err = ACAPI_Environment (APIEnv_IsAutoIntersectOnID, &isAutoIntersectOn);
		if (err != NoError)
			WriteReport_Alert ("Error");

		isAutoIntersectOn = !isAutoIntersectOn;
		err = ACAPI_Environment (APIEnv_ChangeAutoIntersectID, &isAutoIntersectOn);
		if (err != NoError)
			WriteReport_Alert ("Error");

		isAutoIntersectOn = !isAutoIntersectOn;
		err = ACAPI_Environment (APIEnv_ChangeAutoIntersectID, &isAutoIntersectOn);
		if (err != NoError)
			WriteReport_Alert ("Error");
	}
	{
		bool isInCurtainWallEditMode = false;
		GSErrCode err = ACAPI_Environment (APIEnv_IsInCurtainWallEditModeID, &isInCurtainWallEditMode);
		if (err != NoError)
			WriteReport_Alert ("Error");
	}
	{
		bool hasRight = false;
		ACAPI_Environment (APIEnv_GetTWAccessRightID, (void*) APISaveAsIFC, &hasRight);
	}
	{
		bool isSuspendGroupOn = false;
		GSErrCode err = ACAPI_Environment (APIEnv_IsSuspendGroupOnID, &isSuspendGroupOn);
		if (err != NoError)
			WriteReport_Alert ("Error");

		isSuspendGroupOn = !isSuspendGroupOn;
		err = ACAPI_Environment ((API_EnvironmentID)'CSPG', &isSuspendGroupOn);
		if (err != NoError)
			WriteReport_Alert ("Error");

		isSuspendGroupOn = !isSuspendGroupOn;
		err = ACAPI_Environment ((API_EnvironmentID)'CSPG', &isSuspendGroupOn);
		if (err != NoError)
			WriteReport_Alert ("Error");
	}
	{
		{
			API_UnitPrefs				unitPrefs;
			GSErrCode err = ACAPI_Environment (APIEnv_GetPreferencesID, &unitPrefs, (void *) APIPrefs_WorkingUnitsID);
			if (err != NoError)
				WriteReport_Alert ("Error");

			API_UnitTypeID oldLengthUnit = unitPrefs.lengthUnit;
			unitPrefs.lengthUnit = APIUnit_Centimetric;

			err = ACAPI_Environment ((API_EnvironmentID)'SPRF', &unitPrefs, (void *) APIPrefs_WorkingUnitsID);
			if (err != NoError)
				WriteReport_Alert ("Error");

			unitPrefs.lengthUnit = oldLengthUnit;
			err = ACAPI_Environment ((API_EnvironmentID)'SPRF', &unitPrefs, (void *) APIPrefs_WorkingUnitsID);
			if (err != NoError)
				WriteReport_Alert ("Error");
		}
		{
			API_DimensionPrefs			dimPrefs;
			GSErrCode err = ACAPI_Environment (APIEnv_GetPreferencesID, &dimPrefs, (void *) APIPrefs_DimensionsID);
			if (err != NoError)
				WriteReport_Alert ("Error");

			short oldUnit = dimPrefs.linear.unit;
			dimPrefs.linear.unit = (short)APIUnit_Centimetric;

			err = ACAPI_Environment ((API_EnvironmentID)'SPRF', &dimPrefs, (void *) APIPrefs_DimensionsID);
			if (err != NoError)
				WriteReport_Alert ("Error");

			dimPrefs.linear.unit = oldUnit;
			err = ACAPI_Environment ((API_EnvironmentID)'SPRF', &dimPrefs, (void *) APIPrefs_DimensionsID);
			if (err != NoError)
				WriteReport_Alert ("Error");
		}
		{
			API_LegacyPrefs				legacyPrefs;
			BNZeroMemory (&legacyPrefs, sizeof (API_LegacyPrefs));

			GSErrCode err = ACAPI_Environment (APIEnv_GetPreferencesID, &legacyPrefs, (void *) APIPrefs_LegacyID);
			if (err != NoError)
				WriteReport_Alert ("Error");

		}
		{
			API_ZonePrefs				zonePrefs;
			BNZeroMemory (&zonePrefs, sizeof (API_ZonePrefs));

			GSErrCode err = ACAPI_Environment (APIEnv_GetPreferencesID, &zonePrefs, (void *) APIPrefs_ZonesID);
			if (err != NoError)
				WriteReport_Alert ("Error");
		}
		{
			API_SafetyPrefs				safetyPrefs;
			BNZeroMemory (&safetyPrefs, sizeof (API_SafetyPrefs));

			GSErrCode err = ACAPI_Environment (APIEnv_GetPreferencesID, &safetyPrefs, (void *) APIPrefs_DataSafetyID);
			if (err != NoError)
				WriteReport_Alert ("Error");
		}
		{
			API_CalcUnitPrefs			calcUnitPrefs;
			BNZeroMemory (&calcUnitPrefs, sizeof (API_CalcUnitPrefs));

			GSErrCode err = ACAPI_Environment (APIEnv_GetPreferencesID, &calcUnitPrefs, (void *) APIPrefs_CalcUnitsID);
			if (err != NoError)
				WriteReport_Alert ("Error");
		}
		{
			API_RefLevelsPrefs			refLevelsPrefs;
			BNZeroMemory (&refLevelsPrefs, sizeof (API_RefLevelsPrefs));

			GSErrCode err = ACAPI_Environment (APIEnv_GetPreferencesID, &refLevelsPrefs, (void *) APIPrefs_ReferenceLevelsID);
			if (err != NoError)
				WriteReport_Alert ("Error");
		}
		{
			API_ImagingAndCalcPrefs		imagingAndCalcPrefs;
			BNZeroMemory (&imagingAndCalcPrefs, sizeof (API_ImagingAndCalcPrefs));

			GSErrCode err = ACAPI_Environment (APIEnv_GetPreferencesID, &imagingAndCalcPrefs, (void *) APIPrefs_ImagingAndCalcID);
			if (err != NoError)
				WriteReport_Alert ("Error");
		}
		{
			API_FloorPlanCutDefinition	fpCutDefPrefs;
			BNZeroMemory (&fpCutDefPrefs, sizeof (API_FloorPlanCutDefinition));

			GSErrCode err = ACAPI_Environment (APIEnv_GetPreferencesID, &fpCutDefPrefs, (void *) APIPrefs_FloorPlanCutPlaneDef);
			if (err != NoError)
				WriteReport_Alert ("Error");
		}
		{
			API_LayoutsPrefs			layoutsPrefs;
			BNZeroMemory (&layoutsPrefs, sizeof (API_LayoutsPrefs));

			GSErrCode err = ACAPI_Environment (APIEnv_GetPreferencesID, &layoutsPrefs, (void *) APIPrefs_LayoutsID);
			if (err != NoError)
				WriteReport_Alert ("Error");
		}
	}
	{
		Int32 libIndex = 1;
		IO::Location libLocation;
		GSErrCode err = ACAPI_Environment ((API_EnvironmentID)'GLOL', &libIndex, &libLocation);
		if (err != NoError)
			WriteReport_Alert ("Error");
	}
	{
		GSErrCode commandErrorCode = ACAPI_CallCommand (GS::UniString ("ChangeProjectNotesAndCreateGhostRecordCommand"), [&] () ->GSErrCode {
			API_ProjectNoteInfo	noteInfo;
			BNZeroMemory (&noteInfo, sizeof (API_ProjectNoteInfo));
			GSErrCode			err;

			CHCopyC ("client", noteInfo.client);
			CHCopyC ("projectName", noteInfo.projectName);
			CHCopyC ("company", noteInfo.company);
			CHCopyC ("street", noteInfo.street);
			CHCopyC ("city", noteInfo.city);
			CHCopyC ("country", noteInfo.country);
			CHCopyC ("code", noteInfo.code);
			CHCopyC ("architect", noteInfo.architect);
			CHCopyC ("draftsmen", noteInfo.draftsmen);
			CHCopyC ("projectStatus", noteInfo.projectStatus);
			CHCopyC ("dateOfIssue", noteInfo.dateOfIssue);
			CHCopyC ("keywords", noteInfo.keywords);
			CHCopyC ("notes", noteInfo.notes);

			err = ACAPI_Environment (APIEnv_ChangeProjectNotesID, &noteInfo, NULL);
			if (err != NoError)
				return err;

			API_DatabaseUnId	*dbases = NULL;
			GSSize				nDbases = 0;

			err = ACAPI_Database (APIDb_GetElevationDatabasesID, &dbases, NULL);
			if (err == NoError)
				nDbases = BMpGetSize (reinterpret_cast<GSPtr>(dbases)) / Sizeof32 (API_DatabaseUnId);

			if (err != NoError || nDbases == 0) {
				err = ACAPI_Database (APIDb_GetLayoutDatabasesID, &dbases, NULL);
				nDbases = BMpGetSize (reinterpret_cast<GSPtr>(dbases)) / Sizeof32 (API_DatabaseUnId);
			}

			if (err == NoError && nDbases > 0) {
				API_DatabaseUnId floorPlan;
				BNZeroMemory (&floorPlan, sizeof (API_DatabaseUnId));

				{
					short userId = 0;
					UInt32 databaseId = 1;
					ACAPI_Database (APIDb_GetGuidFromDBUnIdID, &userId, &databaseId, &floorPlan.elemSetId);
				}

				API_GhostRecord	apiGhostRecord;
				BNZeroMemory (&apiGhostRecord, sizeof (API_GhostRecord));
				apiGhostRecord.parentDatabase = floorPlan;
				apiGhostRecord.ghostDatabase = dbases[0];

				apiGhostRecord.scale = 1.0;
				apiGhostRecord.rotation = PI / 2;

				apiGhostRecord.floor = 0;
				apiGhostRecord.ghostType = APIGhost_NoFloor;
				apiGhostRecord.show = true;
				apiGhostRecord.activated = true;

				apiGhostRecord.ghostColor.f_red = 0.9;
				apiGhostRecord.ghostColor.f_green = 0.9;
				apiGhostRecord.ghostColor.f_blue = 0.7;
				apiGhostRecord.ghostColor.f_alpha = 1.0;
				apiGhostRecord.ghostOriginalColor = false;
				apiGhostRecord.ghostOriginalTone = false;
				apiGhostRecord.ghostBackgroundFill = true;

				apiGhostRecord.viewColor.f_red = 0.9;
				apiGhostRecord.viewColor.f_green = 0.9;
				apiGhostRecord.viewColor.f_blue = 0.7;
				apiGhostRecord.viewColor.f_alpha = 1.0;
				apiGhostRecord.viewOriginalColor = true;
				apiGhostRecord.viewOriginalTone = false;
				apiGhostRecord.viewBackgroundFill = true;

				apiGhostRecord.ghostOnTop = false;

				memset (&apiGhostRecord.filter, 0xFF, sizeof (API_ElemFilter));
				apiGhostRecord.filter[API_ObjectID] = false;	// don't show objects

				err = ACAPI_Environment (APIEnv_CreateGhostRecordID, &apiGhostRecord, NULL);
			}

			if (dbases != NULL)
				BMpFree (reinterpret_cast<GSPtr>(dbases));

			return err;
		});
		if (commandErrorCode != NoError)
			ErrorBeep ("ChangeProjectNotesAndCreateGhostRecordCommand failed", commandErrorCode);
	}
	{
		// TODO D-141 APIEnv_GetLibPartLockStatus
		// TODO D-141 APIEnv_ChangeStorySettings
	}

}

// -----------------------------------------------------------------------------
// Identify the server application and version
// -----------------------------------------------------------------------------

static void		Do_GetServerApplication (void)
{
	API_ServerApplicationInfo	serverApp;

	ACAPI_Environment (APIEnv_ApplicationID, &serverApp, NULL);

	switch (serverApp.serverApplication) {
		case APIAppl_ArchiCADID:
					if (serverApp.runningInBackground)
						WriteReport ("Background ArchiCAD (API %d R%d)", (short) serverApp.mainVersion, (short) serverApp.releaseVersion);
					else
						WriteReport_Alert ("ArchiCAD (API %d R%d)", (short) serverApp.mainVersion, (short) serverApp.releaseVersion);
					break;

		case APIAppl_3DEngineID:
					WriteReport_Alert ("3D Engine (API %d R%d)", (short) serverApp.mainVersion, (short) serverApp.releaseVersion);
					break;

		default:
					WriteReport_Alert ("Unknown server application");
					break;
	}

	return;
}		// Do_GetServerApplication



// =============================================================================
//
// Make queries on the project installation
//
// =============================================================================


// -----------------------------------------------------------------------------
// Get information on the current project
//	 - project name, file location
//	 - Teamwork settings
// -----------------------------------------------------------------------------

static void		Do_DumpProjectInfo (void)
{
	API_ProjectInfo		projectInfo;
	API_SharingInfo		sharingInfo;
	IO::Path			path;
	char				buffer[256];
	Int32				i;
	GSErrCode			err;

	err = ACAPI_Environment (APIEnv_ProjectID, &projectInfo, NULL);
	if (err != NoError) {
		ErrorBeep ("APIEnv_ProjectID", err);
		return;
	}

	if (projectInfo.untitled)
		WriteReport ("Project file has not been saved yet");
	else {
		if (!projectInfo.teamwork) {
			projectInfo.location->ToDisplayText (&path);
			WriteReport ("Solo Project: %s", (const char *) path);
		}
		delete projectInfo.location;
	}

	if (!projectInfo.teamwork)
		return;

	projectInfo.location_team->ToPath (&path);
	WriteReport ("Shared project: %s", (const char *) path);

	delete projectInfo.location_team;

	WriteReport ("User ID: %d", projectInfo.userId);
	WriteReport ("--------------------");
	WriteReport ("Project Members:");

	err = ACAPI_Environment (APIEnv_ProjectSharingID, &sharingInfo, NULL);
	if (err != NoError) {
		ErrorBeep ("APIEnv_ProjectSharingID", err);
		return;
	}

	for (i = 0; i < sharingInfo.nUsers; i++) {
		GS::UniString fullName ((*sharingInfo.users)[i].loginName);
		sprintf (buffer, "  \"%s\"   id:%d   %s",
				 fullName.ToCStr ().Get (),
				 (*sharingInfo.users)[i].userId,
				 (*sharingInfo.users)[i].connected ? "Connected" : "");
		if ((*sharingInfo.users)[i].userId == projectInfo.userId)
			buffer[0] = '*';
		WriteReport (buffer);
	}

	BMKillHandle ((GSHandle *) &sharingInfo.users);

	return;
}		// Do_DumpProjectInfo


// -----------------------------------------------------------------------------
// Dump preferences
// -----------------------------------------------------------------------------

static void	Do_DumpPreferences (void)
{
	API_UnitPrefs				unitPrefs;
	API_DimensionPrefs			dimPrefs;
	API_LegacyPrefs				legacyPrefs;
	API_ZonePrefs				zonePrefs;
	API_SafetyPrefs				safetyPrefs;
	API_CalcUnitPrefs			calcUnitPrefs;
	API_RefLevelsPrefs			refLevelsPrefs;
	API_ImagingAndCalcPrefs		imagingAndCalcPrefs;
	API_FloorPlanCutDefinition	fpCutDefPrefs;
	API_LayoutsPrefs			layoutsPrefs;

	API_StoryInfo	storyInfo;

	BNZeroMemory (&dimPrefs, sizeof (API_DimensionPrefs));

	IO::Path	path;
	char		colorStr[32] = { '\0' };
	short		i;
	GSErrCode	err;

	err = NoError;
	err |= ACAPI_Environment (APIEnv_GetPreferencesID, &unitPrefs, (void *) APIPrefs_WorkingUnitsID);
	err |= ACAPI_Environment (APIEnv_GetPreferencesID, &dimPrefs, (void *) APIPrefs_DimensionsID);
	err |= ACAPI_Environment (APIEnv_GetPreferencesID, &legacyPrefs, (void *) APIPrefs_LegacyID);
	err |= ACAPI_Environment (APIEnv_GetPreferencesID, &zonePrefs, (void *) APIPrefs_ZonesID);
	err |= ACAPI_Environment (APIEnv_GetPreferencesID, &safetyPrefs, (void *) APIPrefs_DataSafetyID);
	err |= ACAPI_Environment (APIEnv_GetPreferencesID, &calcUnitPrefs, (void *) APIPrefs_CalcUnitsID);
	err |= ACAPI_Environment (APIEnv_GetPreferencesID, &refLevelsPrefs, (void *) APIPrefs_ReferenceLevelsID);
	err |= ACAPI_Environment (APIEnv_GetPreferencesID, &imagingAndCalcPrefs, (void *) APIPrefs_ImagingAndCalcID);
	err |= ACAPI_Environment (APIEnv_GetPreferencesID, &fpCutDefPrefs, (void *) APIPrefs_FloorPlanCutPlaneDef);
	err |= ACAPI_Environment (APIEnv_GetPreferencesID, &layoutsPrefs, (void *) APIPrefs_LayoutsID);
	if (err != NoError) {
		ErrorBeep ("APIEnv_GetPreferencesID", err);
		return;
	}

	WriteReport (">>>>>> Dump Preferences <<<<<<");

	WriteReport ("Dimensions: ");
	WriteReport ("  Index: ");
	WriteReport ("%d\n", dimPrefs.index);

	safetyPrefs.tempFolder->ToDisplayText (&path);
	delete safetyPrefs.tempFolder;
	WriteReport ("Temporary folder: %s", (const char *) path);

	WriteReport ("LengthDec: %d,  AngleDec: %d,  RoundInch: %d,  AngleAccuracy: %d",
				unitPrefs.lenDecimals, unitPrefs.angleDecimals,
				unitPrefs.roundInch, unitPrefs.angleAccuracy);

	WriteReport ("All columns VS beams: %d", (legacyPrefs.coluConnPriority / 10 - 420 + 10) / 20 * 2);
	if (legacyPrefs.useLegacyInteriorElevations)
		WriteReport("Uses legacy method when creating Interioir Elevations with infinite Horizontal Range");
	else
		WriteReport("Doesn't uses legacy method when creating Interioir Elevations with infinite Horizontal Range");

	WriteReport ("Zones: ");
	WriteReport ("  Reduction: ");
	for (i = 0; i < zonePrefs.nLowAreas; i++)
		WriteReport ("  [%3d]:  %3d %%   %4.2lf", i, zonePrefs.roomRedPercents[i], zonePrefs.roomHeightLimits[i]);


	WriteReport ("Calculation units: ");
	WriteReport ("  Length unit: %d,  decimals: %d,  accuracy: %d",
				(int) calcUnitPrefs.length.unit, calcUnitPrefs.length.decimals, calcUnitPrefs.length.accuracy);
	WriteReport ("  Area unit:   %d,  decimals: %d,  accuracy: %d",
				(int) calcUnitPrefs.area.unit, calcUnitPrefs.area.decimals, calcUnitPrefs.area.accuracy);
	WriteReport ("  Volume unit: %d,  decimals: %d,  accuracy: %d",
				(int) calcUnitPrefs.volume.unit, calcUnitPrefs.volume.decimals, calcUnitPrefs.volume.accuracy);


	WriteReport ("Reference levels: ");
	WriteReport ("  Level 1: \"%s\", value: %4f", refLevelsPrefs.level1UStr->ToCStr ().Get (), refLevelsPrefs.level1val);
	WriteReport ("  Level 2: \"%s\", value: %4f", refLevelsPrefs.level2UStr->ToCStr ().Get (), refLevelsPrefs.level2val);


	WriteReport ("Imaging and Calculation preferences: ");
	WriteReport ("  Auto-rebuild: %s,  Bring 3D to front: %s,  Keep zoomed 3D: %s,  Keep zoomed Section: %s",
					imagingAndCalcPrefs.autoRebuild3D ? "ON" : "OFF",
					imagingAndCalcPrefs.activate3DOnChange ? "ON" : "OFF",
					imagingAndCalcPrefs.keepZoomed3D ? "ON" : "OFF",
					imagingAndCalcPrefs.keepZoomedSection ? "ON" : "OFF");
	WriteReport ("  Show Progress Window: %s,  Interrupt with error messages: %s,  3D progress sounds: %s,  Write Report: %s",
					imagingAndCalcPrefs.showProgressWindow ? "ON" : "OFF",
					imagingAndCalcPrefs.interruptOnError ? "ON" : "OFF",
					imagingAndCalcPrefs.progressSounds ? "ON" : "OFF",
					imagingAndCalcPrefs.generate3DReport ? "ON" : "OFF");

	switch (imagingAndCalcPrefs.reportLevel) {
		case APIReportLevel_Brief:			WriteReport ("  Report level: Brief");		break;
		case APIReportLevel_Short:			WriteReport ("  Report level: Short");		break;
		case APIReportLevel_Detailed:		WriteReport ("  Report level: Detailed");	break;
		case APIReportLevel_Full:			WriteReport ("  Report level: Full");		break;
	}
	switch (imagingAndCalcPrefs.selElemsListing) {
		case APICalcSelElems_ShowAlert:		WriteReport ("  Selected elements to be calculated: Show Alert");	break;
		case APICalcSelElems_ListAll:		WriteReport ("  Selected elements to be calculated: List All");		break;
		case APICalcSelElems_UseFilters:	WriteReport ("  Selected elements to be calculated: Use Filters");	break;
	}


	ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo, NULL);
	BMKillHandle (reinterpret_cast<GSHandle *> (&storyInfo.data));

	WriteReport ("Floor plan cut preferences: ");
	WriteReport ("  Current cut level relative to the given story: %5.2f", fpCutDefPrefs.currCutLevel);
	WriteReport ("  Top level relative to the top story: %5.2f", fpCutDefPrefs.topCutLevel);
	WriteReport ("  Bottom level relative to the bottom story: %5.2f", fpCutDefPrefs.bottomCutLevel);
	WriteReport ("  Top story index relative (absolute) to the current story index: %d (%d)", fpCutDefPrefs.topCutBaseStoryRelInd,
						fpCutDefPrefs.topCutBaseStoryRelInd + storyInfo.actStory);
	WriteReport ("  Bottom story index relative (absolute) to the current story index: %d (%d)", fpCutDefPrefs.bottomCutBaseStoryRelInd,
						fpCutDefPrefs.bottomCutBaseStoryRelInd + storyInfo.actStory);
	WriteReport ("  Current cut level relative to project 0: %5.2f", fpCutDefPrefs.fixLevel2Absolute0);


	WriteReport ("Layouts preferences: ");
	if (!layoutsPrefs.useOwnMasterColor)
		sprintf (colorStr, "(%d, %d, %d)", (short) layoutsPrefs.masterColor.f_red * 255,
											(short) layoutsPrefs.masterColor.f_green * 255,
											(short) layoutsPrefs.masterColor.f_blue * 255);
	WriteReport ("  Use %s master color,%s adjust drawing frame to view zoom",
					layoutsPrefs.useOwnMasterColor ? "own" : colorStr,
					layoutsPrefs.adjustDrawingFrameToViewZoom ? "" : " don't");

	return;
}		// Do_DumpPreferences


// -----------------------------------------------------------------------------
// Get city/date/time/sun information
// -----------------------------------------------------------------------------

static void		Do_DumpLocation (void)

{	API_PlaceInfo	placeInfo;
	API_Vector3D	sunDir;
	GSErrCode		err;

	BNZeroMemory (&placeInfo, sizeof (API_PlaceInfo));
	err = ACAPI_Environment (APIEnv_GetPlaceSetsID, &placeInfo, NULL);
	if (err) {
		ErrorBeep ("APIEnv_GetPlaceSetsID", err);
		return;
	}

	WriteReport ("actual day: %d", placeInfo.dayOfYear);
	WriteReport ("actual minute: %d", placeInfo.minOfDay);

	placeInfo.dayOfYear = 45;
	placeInfo.minOfDay = 8 * 60 + 45;
	err = ACAPI_Goodies (APIAny_CalcSunOnPlaceID, &placeInfo, NULL);
	if (err) {
		ErrorBeep ("APIAny_CalcSunOnPlaceID", err);
		return;
	}

	sunDir.x = cos (placeInfo.sunAngXY) * cos (placeInfo.sunAngZ);
	sunDir.y = sin (placeInfo.sunAngXY) * cos (placeInfo.sunAngZ);
	sunDir.z = sin (placeInfo.sunAngZ);

	WriteReport ("Sun data at 8:45 on 14th February");
	WriteReport ("  xy angle: %lf", placeInfo.sunAngXY);
	WriteReport ("  z angle:  %lf", placeInfo.sunAngZ);
	WriteReport ("  dirVect:  (%lf, %lf, %lf) ", sunDir.x, sunDir.y, sunDir.z);

	return;
}		// Do_DumpLocation


// -----------------------------------------------------------------------------
// Dump 3D projection settings
// -----------------------------------------------------------------------------

static void Do_Dump3DProjection (void)
{
	API_3DProjectionInfo	p;
	short					i;
	GSErrCode				err;

	err = ACAPI_Environment (APIEnv_Get3DProjectionSetsID, &p, NULL);
	if (err != NoError) {
		ErrorBeep ("APIEnv_Get3DProjectionSetsID", err);
		return;
	}

	WriteReport ("3D Projection Settings:");
	if (p.isPersp) {
		WriteReport ("  perspective");
	} else {
		WriteReport ("  axonometric");
		WriteReport ("  mode: %d", p.u.axono.projMod);
		WriteReport ("  transformat matrix:");
		for (i = 0; i < 12; i++)
			WriteReport ("    [%.2d] = %.4f", i+1, p.u.axono.tranmat.tmx[i]);
	}

	return;
}		// Do_Dump3DProjection


// -----------------------------------------------------------------------------
// Dump story settings
// -----------------------------------------------------------------------------

static void Do_DumpStorySettings (void)
{
	API_StoryInfo	storyInfo;
	Int32			i;
	GSErrCode		err;

	err = ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo, NULL);
	if (err != NoError) {
		ErrorBeep ("APIEnv_GetStorySettingsID", err);
		return;
	}


	WriteReport ("actual story: %d", storyInfo.actStory);
	WriteReport ("first story: %d", storyInfo.firstStory);
	WriteReport ("last story: %d", storyInfo.lastStory);

	for (i = storyInfo.lastStory - storyInfo.firstStory; i >= 0; i--) {
		WriteReport ("   [%d]  \"%s\"  level: %f  height: %f",
					 (*storyInfo.data)[i].index,
					 (*storyInfo.data)[i].name,
					 (*storyInfo.data)[i].level,
					 (*storyInfo.data)[i+1].level - (*storyInfo.data)[i].level);
	}

	BMKillHandle (reinterpret_cast<GSHandle *> (&storyInfo.data));

	return;
}		// Do_DumpStorySettings


// -----------------------------------------------------------------------------
// Dump 3D DocumentFrom3D Defaults
// -----------------------------------------------------------------------------

static void Do_DumpDocumentFrom3DDefaults (void)
{
	API_DocumentFrom3DDefaults	d3Default;

	GSErrCode err = ACAPI_Environment (APIEnv_GetDocumentFrom3DDefaultsID, &d3Default, NULL);
	if (err != NoError) {
		ErrorBeep ("APIEnv_GetDocumentFrom3DDefaultsID", err);
		return;
	}

	WriteReport ("3D Document Default Settings:");

	WriteReport ("shouldUseUniformCutAttributes: %s", d3Default.attributeOptions.shouldUseUniformCutAttributes ? "true" : "false");
	WriteReport ("uniformLineType: %d", d3Default.attributeOptions.uniformLineType);
	WriteReport ("uniformFillPen: %d", d3Default.attributeOptions.uniformFillPen);
	WriteReport ("uniformFillBkgPen: %d", d3Default.attributeOptions.uniformFillBkgPen);
	WriteReport ("shouldUseUncutSurfaceFill: %s", d3Default.attributeOptions.shouldUseUncutSurfaceFill ? "true" : "false");
	WriteReport ("uncutSurfaceFillType: %d", d3Default.attributeOptions.uncutSurfaceFillType);
	WriteReport ("uncutSurfaceBkgPen: %d", d3Default.attributeOptions.uncutSurfaceBkgPen);
	WriteReport ("shouldUseElemPen: %s", d3Default.attributeOptions.shouldUseElemPen ? "true" : "false");
	WriteReport ("elemPen: %d", d3Default.attributeOptions.elemPen);
	WriteReport ("shadFill: %d", d3Default.attributeOptions.shadFill);
	WriteReport ("shadFillPen: %d", d3Default.attributeOptions.shadFillPen);
	WriteReport ("shadFillBkgPen: %d", d3Default.attributeOptions.shadFillBkgPen);
	WriteReport ("shadFillBkgPen: %s", d3Default.name);
	WriteReport ("shadFillBkgPen: %d", d3Default.refId);
	WriteReport ("vect3DHatching: %s", d3Default.vect3DHatching ? "true" : "false");
	WriteReport ("vectSunShadow: %s", d3Default.vectSunShadow ? "true" : "false");
	WriteReport ("mustBeTransparent: %s", d3Default.mustBeTransparent ? "true" : "false");
	WriteReport ("intPlanConn: %d", d3Default.intPlanConn);
	WriteReport ("materialFrom3D: %s", d3Default.materialFrom3D ? "true" : "false");
	WriteReport ("contureOff: %s", d3Default.contureOff ? "true" : "false");
	WriteReport ("cMater: %d", d3Default.cMater);
	WriteReport ("viewSrc: %d", d3Default.skeletonOptions.viewSrc);
	WriteReport ("floorIdx: %d", d3Default.skeletonOptions.floorIdx);
	WriteReport ("cutHeight: %d", d3Default.skeletonOptions.cutHeight);
	WriteReport ("floorOffset: %d", d3Default.skeletonOptions.floorOffset);
	WriteReport ("cutOffset: %d", d3Default.skeletonOptions.cutOffset);

	return;
}		// Do_Dump3DProjection

// =============================================================================
//
// Modify the project environment
//
// =============================================================================
#ifdef __APPLE__
#pragma mark -
#endif

// -----------------------------------------------------------------------------
// Change Project Notes
// -----------------------------------------------------------------------------

static void		Do_ChangeProjectNotes (void)
{
	API_ProjectNoteInfo	noteInfo;
	GSErrCode			err;

	CHCopyC ("client", noteInfo.client);
	CHCopyC ("projectName", noteInfo.projectName);
	CHCopyC ("company", noteInfo.company);
	CHCopyC ("street", noteInfo.street);
	CHCopyC ("city", noteInfo.city);
	CHCopyC ("country", noteInfo.country);
	CHCopyC ("code", noteInfo.code);
	CHCopyC ("architect", noteInfo.architect);
	CHCopyC ("draftsmen", noteInfo.draftsmen);
	CHCopyC ("projectStatus", noteInfo.projectStatus);
	CHCopyC ("dateOfIssue", noteInfo.dateOfIssue);
	CHCopyC ("keywords", noteInfo.keywords);
	CHCopyC ("notes", noteInfo.notes);

	err = ACAPI_Environment (APIEnv_ChangeProjectNotesID, &noteInfo, NULL);
	if (err != NoError)
		ErrorBeep ("APIEnv_ChangeProjectNotesID", err);

	return;
}		// Do_ProjectNotes


// -----------------------------------------------------------------------------
// Create a new story above the topmost one
// 	- ArchiCAD menus must be updated
// 	- check the "Stories..." dialog after execution
// -----------------------------------------------------------------------------

static void Do_NewStory (void)
{
	API_StoryInfo		storyInfo;
	API_StoryCmdType	storyCmd;
	GSErrCode			err;

	err = ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo, NULL);
	if (err != NoError) {
		ErrorBeep ("APIEnv_GetStorySettingsID", err);
		return;
	}

	BNZeroMemory (&storyCmd, sizeof (API_StoryCmdType));
	storyCmd.action = APIStory_InsAbove;
	storyCmd.index  = (short) (storyInfo.lastStory - storyInfo.firstStory);
	storyCmd.height = 3.21;
	sprintf (storyCmd.name, "Created story #%d", storyCmd.index);

	BMKillHandle (reinterpret_cast<GSHandle *> (&storyInfo.data));

	err = ACAPI_Environment (APIEnv_ChangeStorySettingsID, &storyCmd, NULL);
	if (err != NoError)
		ErrorBeep ("APIEnv_ChangeStorySettingsID (new story)", err);

	return;
}		// Do_NewStory


// -----------------------------------------------------------------------------
// Change the height of the active story
// 	- ArchiCAD menus must be updated
// 	- check the "Stories..." dialog after execution
// -----------------------------------------------------------------------------

static void		Do_ChangeStoryHeight (void)

{	API_StoryInfo		storyInfo;
	API_StoryCmdType	storyCmd;
	Int32				actFloor;
	double				actHeight;
	GSErrCode			err;

	BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	err = ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo, NULL);
	if (err != NoError) {
		ErrorBeep ("APIEnv_GetStorySettingsID", err);
		return;
	}

	actFloor = storyInfo.actStory - storyInfo.firstStory;
	actHeight = (*storyInfo.data)[actFloor + 1].level - (*storyInfo.data)[actFloor].level;

	BNZeroMemory (&storyCmd, sizeof (API_StoryCmdType));
	storyCmd.action = APIStory_SetHeight;
	storyCmd.index  = (*storyInfo.data)[actFloor].index;
	storyCmd.height = actHeight + 1.0;

	BMKillHandle (reinterpret_cast<GSHandle *> (&storyInfo.data));

	err = ACAPI_Environment (APIEnv_ChangeStorySettingsID, &storyCmd, NULL);
	if (err != NoError)
		ErrorBeep ("APIEnv_ChangeStorySettingsID (chg heigth)", err);

	return;
}		// Do_ChangeStoryHeight


// -----------------------------------------------------------------------------
// Create a ghost record
// -----------------------------------------------------------------------------

static void		Do_CreateGhostRecord (void)
{
	GSErrCode			err = NoError;

	API_DatabaseUnId	*dbases = NULL;
	GSSize				nDbases = 0;

	err = ACAPI_Database (APIDb_GetElevationDatabasesID, &dbases, NULL);
	if (err == NoError)
		nDbases = BMpGetSize (reinterpret_cast<GSPtr>(dbases)) / Sizeof32 (API_DatabaseUnId);

	if (err != NoError || nDbases == 0) {
		err = ACAPI_Database (APIDb_GetLayoutDatabasesID, &dbases, NULL);
		nDbases = BMpGetSize (reinterpret_cast<GSPtr>(dbases)) / Sizeof32 (API_DatabaseUnId);
	}

	if (err == NoError && nDbases > 0) {
		API_DatabaseUnId floorPlan;
		BNZeroMemory (&floorPlan, sizeof (API_DatabaseUnId));

		{
			short userId = 0;
			UInt32 databaseId = 1;
			ACAPI_Database (APIDb_GetGuidFromDBUnIdID, &userId, &databaseId, &floorPlan.elemSetId);
		}

		API_GhostRecord	apiGhostRecord;
		BNZeroMemory (&apiGhostRecord, sizeof (API_GhostRecord));
		apiGhostRecord.parentDatabase = floorPlan;
		apiGhostRecord.ghostDatabase = dbases[0];

		apiGhostRecord.scale = 1.0;
		apiGhostRecord.rotation = PI / 2;

		apiGhostRecord.floor = 0;
		apiGhostRecord.ghostType = APIGhost_NoFloor;
		apiGhostRecord.show = true;
		apiGhostRecord.activated = true;

		apiGhostRecord.ghostColor.f_red = 0.9;
		apiGhostRecord.ghostColor.f_green = 0.9;
		apiGhostRecord.ghostColor.f_blue = 0.7;
		apiGhostRecord.ghostColor.f_alpha = 1.0;
		apiGhostRecord.ghostOriginalColor = false;
		apiGhostRecord.ghostOriginalTone = false;
		apiGhostRecord.ghostBackgroundFill = true;

		apiGhostRecord.viewColor.f_red = 0.9;
		apiGhostRecord.viewColor.f_green = 0.9;
		apiGhostRecord.viewColor.f_blue = 0.7;
		apiGhostRecord.viewColor.f_alpha = 1.0;
		apiGhostRecord.viewOriginalColor = true;
		apiGhostRecord.viewOriginalTone = false;
		apiGhostRecord.viewBackgroundFill = true;

		apiGhostRecord.ghostOnTop = false;

		memset (&apiGhostRecord.filter, 0xFF, sizeof (API_ElemFilter));
		apiGhostRecord.filter[API_ObjectID] = false;	// don't show objects

		err = ACAPI_Environment (APIEnv_CreateGhostRecordID, &apiGhostRecord, NULL);
		if (err != NoError)
			ErrorBeep ("APIEnv_CreateGhostRecordID", err);
	}

	if (dbases != NULL)
		BMpFree (reinterpret_cast<GSPtr>(dbases));

	return;
}		// Do_CreateGhostRecord

// -----------------------------------------------------------------------------
// Change a ghost record
// -----------------------------------------------------------------------------

static void		Do_ChangeGhostRecord (void)
{
	API_DatabaseUnId floorPlan;
	BNZeroMemory (&floorPlan, sizeof (API_DatabaseUnId));

	{
		short userId = 0;
		UInt32 databaseId = 1;
		ACAPI_Database (APIDb_GetGuidFromDBUnIdID, &userId, &databaseId, &floorPlan.elemSetId);
	}

	API_GhostRecord	apiGhostRecord;
	BNZeroMemory (&apiGhostRecord, sizeof (API_GhostRecord));
	apiGhostRecord.parentDatabase = floorPlan;

	GSErrCode err = ACAPI_Environment (APIEnv_GetGhostRecordID, &floorPlan, &apiGhostRecord);
	if (err == APIERR_WINDNOTEXIST) // parent database doesn't have any ghost database
		return;

	if (err != NoError) {
		ErrorBeep ("APIEnv_GetGhostRecordID", err);
		return;
	}

	if (apiGhostRecord.viewOriginalColor)
		apiGhostRecord.viewOriginalColor = false;
	else
		apiGhostRecord.viewOriginalColor = true;

	if (apiGhostRecord.ghostOriginalColor)
		apiGhostRecord.ghostOriginalColor = false;
	else
		apiGhostRecord.ghostOriginalColor = true;

	err = ACAPI_Environment (APIEnv_ChangeGhostRecordID, &apiGhostRecord, NULL);
	if (err != NoError) {
		ErrorBeep ("APIEnv_ChangeGhostRecordID", err);
		return;
	}
}

// -----------------------------------------------------------------------------
// Delete a ghost record
// -----------------------------------------------------------------------------

static void		Do_DeleteGhostRecord (void)
{
	API_DatabaseUnId floorPlan;
	BNZeroMemory (&floorPlan, sizeof (API_DatabaseUnId));

	{
		short userId = 0;
		UInt32 databaseId = 1;
		ACAPI_Database (APIDb_GetGuidFromDBUnIdID, &userId, &databaseId, &floorPlan.elemSetId);
	}

	API_GhostRecord	apiGhostRecord;
	BNZeroMemory (&apiGhostRecord, sizeof (API_GhostRecord));
	apiGhostRecord.parentDatabase = floorPlan;

	GSErrCode err = ACAPI_Environment (APIEnv_GetGhostRecordID, &floorPlan, &apiGhostRecord);
	if (err == APIERR_WINDNOTEXIST) // parent database doesn't have any ghost database
		return;

	if (err != NoError) {
		ErrorBeep ("APIEnv_GetGhostRecordID", err);
		return;
	}

	err = ACAPI_Environment (APIEnv_DeleteGhostRecordID, &apiGhostRecord, NULL);
	if (err != NoError) {
		ErrorBeep ("APIEnv_DeleteGhostRecordID", err);
		return;
	}
}

// -----------------------------------------------------------------------------
// Hide ArchiCAD (supported on Windows only, but not recommended)
// -----------------------------------------------------------------------------

static void		Do_Hide (void)
{
	bool	toShow = false;

	GSErrCode err = ACAPI_Environment (APIEnv_ChangeShowHideStateID, &toShow, NULL);
	if (err == APIERR_NOTSUPPORTED)
		ErrorBeep ("APIEnv_ChangeShowHideStateID not supported", err);

	return;
}		// Do_Hide


// -----------------------------------------------------------------------------
// Load a new library into ArchiCAD
// -----------------------------------------------------------------------------

static void		Do_NewLibraryDialog (void)
{
	IO::Location	newLib;
	char			defName[] = "New Library";

	if (ACAPI_Environment (APIEnv_NewLibDialID, defName, &newLib) == NoError) {
		IO::URL		url;
		newLib.ToURL (&url);
		DBPrintf ("Environment Control -- New library dialog:\n\t%s\n", (const char *) url);
	}

	return;
}		// Do_NewLibraryDialog


// -----------------------------------------------------------------------------
// Switch to Arrow mode
// -----------------------------------------------------------------------------

static void		Do_SwitchToArrow (void)
{
	API_ToolBoxItem		tbi;

	BNZeroMemory (&tbi, sizeof (tbi));
	tbi.typeID = API_ZombieElemID;
	if (ACAPI_Environment (APIEnv_SetToolBoxModeID, &tbi, NULL) == NoError) {
		DBPrintf ("Environment Control -- switch to arrow mode was successful\n");
	}

	return;
}		// Do_SwitchToArrow


// -----------------------------------------------------------------------------
// Display the list of loaded libraries
// -----------------------------------------------------------------------------

static void		Do_ListLoadedLibraries (void)
{
	GS::Array<API_LibraryInfo>	libInfo;

	if (ACAPI_Environment (APIEnv_GetLibrariesID, &libInfo, NULL) == NoError) {
		DBPrintf ("Environment Control :: The number of loaded libraries is %lu\n", libInfo.GetSize ());
		for (UInt32 ii = 0; ii < libInfo.GetSize (); ii++) {
			IO::Path	libPath;

			libInfo[ii].location.ToDisplayText (&libPath);
			DBPrintf ("Environment Control :: Library #%-2d is %s\n", ii+1, (const char *) libPath);
		}
	}

	return;
}		// Do_ListLoadedLibraries


// -----------------------------------------------------------------------------
// Change a navigator item
// -----------------------------------------------------------------------------

static void		Do_ChangeNavigatorItem (void)
{
	API_NavigatorSet set;
	GSErrCode err = NoError;

	set.mapId = API_PublicViewMap;
	err = ACAPI_Environment (APIEnv_GetNavigatorSetID, &set, NULL);
	if (err != NoError)
		return;

	API_NavigatorItem** items = NULL;
	API_NavigatorItem item;

	BNZeroMemory (&item, sizeof (API_NavigatorItem));
	item.guid = set.rootGuid;
	item.mapId = API_PublicViewMap;

	err = ACAPI_Environment (APIEnv_GetNavigatorChildrenItemsID, &item, &items);
	if (err != NoError || items == NULL)
		return;

	Int32 n = BMhGetSize ((GSHandle)items) / Sizeof32 (API_NavigatorItem);


	DBASSERT (n == 1);
	item = **items;
	BMhKill ((GSHandle *) &items);
	items = NULL;

	err = ACAPI_Environment (APIEnv_GetNavigatorChildrenItemsID, &item, &items);
	if (err != NoError || items == NULL)
		return;

	n = BMhGetSize ((GSHandle)items) / Sizeof32 (API_NavigatorItem);
	for (Int32 idx = 0; idx < n; idx++) {
		if ((*items)[idx].itemType == API_StoryNavItem) {
			//get settings
			API_NavigatorView	view;
			BNZeroMemory (&view, sizeof (view));

			// change settings
			// name
			if ((*items)[idx].customName) {
				(*items)[idx].customName = false;
			} else {
				(*items)[idx].customName = true;
				CHCopyC ("Name from API", (*items)[idx].name);
			}

			//uiId
			if ((*items)[idx].customUiId) {
				(*items)[idx].customUiId = false;
			} else {
				(*items)[idx].customUiId = true;
				CHCopyC ("ID from API", (*items)[idx].uiId);
			}

			// settings
			// layer combination
			short attributeCount, i;
			API_Attribute attribute;
			err = ACAPI_Attribute_GetNum (API_LayerCombID, &attributeCount);
			if (err != NoError)
				break;
			BNZeroMemory (&attribute, sizeof (API_Attribute));
			attribute.header.typeID = API_LayerCombID;
			for (i = attributeCount; i > 0; --i) {
				attribute.header.index = i;
				err = ACAPI_Attribute_Get (&attribute);
				if (err == NoError) {
					CHTruncate (attribute.header.name, view.layerCombination, sizeof (view.layerCombination));
					break;
				}
			}

			// model view options
			err = ACAPI_Attribute_GetNum (API_ModelViewOptionsID, &attributeCount);
			if (err != NoError)
				break;
			BNZeroMemory (&attribute, sizeof (API_Attribute));
			attribute.header.typeID = API_ModelViewOptionsID;
			for (i = attributeCount; i > 0; --i) {
				attribute.header.index = i;
				err = ACAPI_Attribute_Get (&attribute);
				if (err == NoError) {
					CHTruncate (attribute.header.name, view.modelViewOptName, sizeof (view.modelViewOptName));
					ACAPI_FreeGDLModelViewOptionsPtr (&attribute.modelViewOpt.modelViewOpt.gdlOptions);
					break;
				}
			}

			// dimension styles
			err = ACAPI_Attribute_GetNum (API_DimStandID, &attributeCount);
			if (err != NoError)
				break;
			BNZeroMemory (&attribute, sizeof (API_Attribute));
			attribute.header.typeID = API_DimStandID;
			for (i = attributeCount; i > 0; --i) {
				attribute.header.index = i;
				err = ACAPI_Attribute_Get (&attribute);
				if (err == NoError) {
					CHTruncate (attribute.header.name, view.dimName, sizeof (view.dimName));
					break;
				}
			}

			// scale
			view.drawingScale = 123;

			// renovation filter
			API_NavigatorView	oldView;
			BNZeroMemory (&oldView, sizeof (oldView));
			err = ACAPI_Environment (APIEnv_GetNavigatorViewID, &((*items)[idx]), &oldView);
			if (err == NoError) {
				GS::Array<API_Guid> renFilters;
				err = ACAPI_Database (APIDb_GetRenovationFiltersID, &renFilters);
				if (err == NoError) {
					GS::USize nRenFilters = renFilters.GetSize ();
					for (GS::UIndex i = 0; i < nRenFilters; ++i) {
						if (oldView.renovationFilterGuid == renFilters[i]) {
							view.renovationFilterGuid = renFilters[i + 1 >= nRenFilters ? 0 : i + 1];
							break;
						}
					}
				}
			}

			// transformation
			if ((*items)[idx].customUiId) {
				// rotate grid by 30 degrees
				view.tr.tmx[0] = view.tr.tmx[5] = sqrt(3.0) / 2.0;
				view.tr.tmx[1] = -0.5;
				view.tr.tmx[4] = 0.5;
				view.tr.tmx[10] = 1.0;
			} else {
				// identity
				view.tr.tmx[0] = view.tr.tmx[5] = view.tr.tmx[10] = 1.0;
			}

			err = ACAPI_Environment (APIEnv_ChangeNavigatorViewID, &((*items)[idx]), &view);
			break;
		}
	}

	BMhKill ((GSHandle *) &items);
}


// -----------------------------------------------------------------------------
// Change 3D DocumentFrom3D Default Setting
// -----------------------------------------------------------------------------

static void Do_ChangeDocumentFrom3DDefaults (void)
{
	API_DocumentFrom3DDefaults	d3Default;

	GSErrCode err = ACAPI_Environment (APIEnv_GetDocumentFrom3DDefaultsID, &d3Default, NULL);
	if (err != NoError) {
		ErrorBeep ("APIEnv_GetDocumentFrom3DDefaultsID", err);
		return;
	}

	d3Default.vect3DHatching = !d3Default.vect3DHatching;
	d3Default.vectSunShadow = !d3Default.vectSunShadow;
	d3Default.mustBeTransparent = !d3Default.mustBeTransparent;

	err = ACAPI_Environment (APIEnv_ChangeDocumentFrom3DDefaultsID, &d3Default, NULL);
	if (err != NoError) {
		ErrorBeep ("APIEnv_ChangeDocumentFrom3DDefaultsID", err);
		return;
	}
}


// -----------------------------------------------------------------------------
// Dump Partial Structure Display state of the current database
// -----------------------------------------------------------------------------

static void Do_DumpPartialStructureDisplay (void)
{
	API_UIStructureDisplay structureDisplay;
	BNZeroMemory (&structureDisplay, sizeof (API_UIStructureDisplay));

	GSErrCode err = ACAPI_Environment (APIEnv_GetStructureDisplayID, &structureDisplay, NULL);
	if (err == NoError) {
		char state[256];
		switch (structureDisplay.structureDisplay) {
			case API_EntireStructure:
				CHCopyC ("Entire Structure", state);
				break;
			case API_CoreOnly:
				CHCopyC ("Core Only", state);
				break;
			case API_WithoutFinishes:
				CHCopyC ("Without Finishes", state);
				break;
			case API_StructureOnly:
				CHCopyC ("Structure Only", state);
				break;
			default:
				DBBREAK ();
				CHCopyC ("Unknown", state);
				break;
		}
		WriteReport ("Partial Structure Display of current database: %s", state);
	} else {
		WriteReport ("The current database doesn't contain Partial Structure Display functionality");
	}
}


// -----------------------------------------------------------------------------
// Change Partial Structure Display state of the current database
// -----------------------------------------------------------------------------

static void Do_ChangePartialStructureDisplay (void)
{
	API_UIStructureDisplay structureDisplay;
	BNZeroMemory (&structureDisplay, sizeof (API_UIStructureDisplay));

	GSErrCode err = ACAPI_Environment (APIEnv_GetStructureDisplayID, &structureDisplay, NULL);
	if (err != NoError) {
		ErrorBeep ("APIEnv_GetStructureDisplayID", err);
		return;
	}

	Int32 state = static_cast<Int32>(structureDisplay.structureDisplay);
	state = (state + 1) % 4;
	structureDisplay.structureDisplay = static_cast<API_StructureDisplay>(state);

	err = ACAPI_Environment (APIEnv_ChangeStructureDisplayID, &structureDisplay, NULL);
	if (err != NoError) {
		ErrorBeep ("APIEnv_ChangeStructureDisplayID", err);
	}
}


// -----------------------------------------------------------------------------
// Change Model View Options
// -----------------------------------------------------------------------------

static void Do_ChangeModelViewOptions (void)
{
	GSErrCode err = NoError;

	API_ViewOptions viewOptions;
	BNZeroMemory (&viewOptions, sizeof (API_ViewOptions));
	err = ACAPI_Environment (APIEnv_GetViewOptionsID, &viewOptions, NULL);
	if (err == NoError) {
		Int32 nGDLOptions = BMGetPtrSize (reinterpret_cast<GSPtr>(viewOptions.modelViewOpt.gdlOptions)) / sizeof (API_GDLModelViewOptions);
		for (Int32 idx = 0; idx < nGDLOptions; ++idx) {
			GS::UniString name (viewOptions.modelViewOpt.gdlOptions[idx].name);
			if (name == "LibraryGlobals13") {
				if (viewOptions.modelViewOpt.gdlOptions[idx].params != NULL) {
					Int32 paramsNumber =  BMGetHandleSize (reinterpret_cast<GSHandle>(viewOptions.modelViewOpt.gdlOptions[idx].params)) / sizeof (API_AddParType);
					for (Int32 i = 0; i < paramsNumber; ++i) {
						API_AddParType& param = (*(viewOptions.modelViewOpt.gdlOptions[idx].params))[i];
						if ((param.typeID == APIParT_Boolean) && CHEqualASCII (param.name, "showBOpLine3D", CS_CaseInsensitive)) { // Show opening lines in 3D for doors
							bool showBOpLine3D = (fabs (param.value.real) > EPS);
							param.value.real = showBOpLine3D ? 0.0 : 1.0;
							break;
						}
					}
				}
			}
		}
	} else {
		ErrorBeep ("APIEnv_GetViewOptionsID", err);
	}

	err = ACAPI_Environment (APIEnv_ChangeViewOptionsID, &viewOptions, NULL);
	if (err != NoError) {
		ErrorBeep ("APIEnv_ChangeViewOptionsID", err);
	}

	ACAPI_FreeGDLModelViewOptionsPtr (&viewOptions.modelViewOpt.gdlOptions);
}


// -----------------------------------------------------------------------------
// Change 3D Cutting Planes
// -----------------------------------------------------------------------------

static void Do_Change3DCuttingPlanes (void)
{
	API_3DCutPlanesInfo cutInfo;
	BNZeroMemory (&cutInfo, sizeof (API_3DCutPlanesInfo));

	GSErrCode err = ACAPI_Environment (APIEnv_Get3DCuttingPlanesID, &cutInfo, NULL);
	if (err == NoError) {
		if (cutInfo.shapes != NULL)
			BMKillHandle ((GSHandle *) &(cutInfo.shapes));

		cutInfo.isCutPlanes = true;
		cutInfo.nShapes = 2;
		cutInfo.shapes = reinterpret_cast<API_3DCutShapeType**> (BMAllocateHandle (cutInfo.nShapes * sizeof (API_3DCutShapeType), ALLOCATE_CLEAR, 0));
		if (cutInfo.shapes != NULL) {
			(*cutInfo.shapes)[0].cutStatus = 2;
			(*cutInfo.shapes)[0].cutPen = 3;
			(*cutInfo.shapes)[0].cutMater = 11;
			(*cutInfo.shapes)[0].pa = -3.0499805934954503;
			(*cutInfo.shapes)[0].pb = 0.43107875694662845;
			(*cutInfo.shapes)[0].pc = 3.5670423669734248;
			(*cutInfo.shapes)[0].pd = 2.4161856450872907;
			(*cutInfo.shapes)[1].cutStatus = 3;
			(*cutInfo.shapes)[1].cutPen = 4;
			(*cutInfo.shapes)[1].cutMater = 12;
			(*cutInfo.shapes)[1].pa = -2.9081872443425456;
			(*cutInfo.shapes)[1].pb = 0.37912781320386035;
			(*cutInfo.shapes)[1].pc = 3.4016167929617027;
			(*cutInfo.shapes)[1].pd = -1.1569668026192714;
		}

		err = ACAPI_Environment (APIEnv_Change3DCuttingPlanesID, &cutInfo, NULL);
		if (err == NoError) {
			API_WindowInfo windowInfo;
			BNZeroMemory (&windowInfo, sizeof (API_WindowInfo));
			windowInfo.typeID = APIWind_3DModelID;
			err = ACAPI_Automate (APIDo_ChangeWindowID, &windowInfo, NULL);
		}

		BMKillHandle ((GSHandle *) &(cutInfo.shapes));
	}
}


// Dump the settings of the most recently used named Rendering Scene. If no named Rendering Scenes are saved yet, dump the current Scene (which can only be the Custom Scene).
static void	Do_DumpRenderingSettings (void)
{
	GS::Array<GS::UniString> recentSceneNames;
	ACAPI_Environment (APIEnv_GetRenderingSceneNamesID, &recentSceneNames);

	GS::UniString* sceneNameToDump = recentSceneNames.IsEmpty () ? NULL : &recentSceneNames[0];

	WriteReport ("Dumping some settings of Rendering Scene \"%s\":", sceneNameToDump == NULL ? "the current Scene" : sceneNameToDump->ToCStr ().Get ());
	WriteReport ("-------------------------------------------------------------");

	API_RendEffects		sceneEffects;
	API_RendImage		sceneImage;
	API_RendBrightness	sceneBrightness;
	BNZeroMemory (&sceneEffects,	sizeof (API_RendEffects));
	BNZeroMemory (&sceneImage,		sizeof (API_RendImage));
	BNZeroMemory (&sceneBrightness,	sizeof (API_RendBrightness));

	GSErrCode err = NoError;
	if (err == NoError)
		err = ACAPI_Environment (APIEnv_GetRenderingSetsID, &sceneEffects,	  (void*) (Int32) APIRendSet_EffectsID,	   sceneNameToDump);
	if (err == NoError)
		err = ACAPI_Environment (APIEnv_GetRenderingSetsID, &sceneImage,	  (void*) (Int32) APIRendSet_ImageID,	   sceneNameToDump);
	if (err == NoError)
		err = ACAPI_Environment (APIEnv_GetRenderingSetsID, &sceneBrightness, (void*) (Int32) APIRendSet_BrightnessID, sceneNameToDump);

	switch (err) {
		case NoError:
			WriteReport ("- Effects    / Sun shadow : %s",			sceneEffects.sunShadowOn ? "on" : "off");
			WriteReport ("- Image      / size       : %dpx x %dpx",	sceneImage.hSize, sceneImage.vSize);
			WriteReport ("- Brightness / brightness : %d",			sceneBrightness.brightness);
			break;

		case APIERR_BADNAME:
			if (DBVERIFY (sceneNameToDump != NULL))
				WriteReport ("- No such named Rendering Scene exists: \"%s\".", sceneNameToDump->ToCStr ().Get ());
			break;

		case APIERR_NOPLAN:
			WriteReport ("- No plan file is loaded.");
			break;
	}

	delete sceneImage.bkgPictFile;
}


// Dump the names of all named Rendering Scenes in the order of recent usage.
static void	Do_DumpAllRenderingSceneNames (void)
{
	GS::Array<GS::UniString> sceneNames;
	const GSErrCode err = ACAPI_Environment (APIEnv_GetRenderingSceneNamesID, &sceneNames);

	WriteReport ("Rendering Scene Names, most recent first");
	WriteReport ("----------------------------------------");
	switch (err) {
		case NoError: {
			USize i = 0;
			for (GS::Array<GS::UniString>::ConstIterator it = sceneNames.Enumerate (); it != NULL; ++it)
				WriteReport ("- Scene %2u: %s", GS::UIntForStdio (++i), it->ToCStr ().Get ());
			break;
		}

		case APIERR_NOPLAN: {
			WriteReport ("- No plan file is loaded.");
			break;
		}
	}
}


// Set the current Rendering Scene to the second most recently used named Rendering Scene.
static void	Do_SetCurrentRenderingScene (void)
{
	GS::Array<GS::UniString> recentSceneNames;
	ACAPI_Environment (APIEnv_GetRenderingSceneNamesID, &recentSceneNames);

	if (recentSceneNames.GetSize () < 2) {
		WriteReport_Alert ("At least two named Rendering Scenes must exist.");
		return;
	}

	GS::UniString newCurrentSceneName = recentSceneNames[1];

	const GSErrCode err = ACAPI_Environment (APIEnv_SetCurrentRenderingSceneID, &newCurrentSceneName);
	switch (err) {
		case NoError:		 /* successful */																								   break;
		case APIERR_NOPLAN:	 WriteReport_Alert ("No plan file is loaded.");																	   break;
		case APIERR_BADNAME: WriteReport_Alert ("No such named Rendering Scene exists: \"%s\".",		newCurrentSceneName.ToCStr ().Get ()); break;
		case APIERR_GENERAL: WriteReport_Alert ("Could not set the current Rendering Scene to \"%s\".",	newCurrentSceneName.ToCStr ().Get ()); break;
	}
}


#ifdef __APPLE__
#pragma mark -
#endif

// -----------------------------------------------------------------------------
// Handles menu commands
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL MenuCommandHandler (const API_MenuParams *menuParams)
{
	switch (menuParams->menuItemRef.menuResID) {
		case 32500:
			switch (menuParams->menuItemRef.itemIndex) {
					case 1:		Do_GetServerApplication ();			break;
					/* ------ */
					case 3:		Do_DumpProjectInfo ();				break;
					case 4:		Do_DumpPreferences ();				break;
					case 5:		Do_DumpLocation ();					break;
					case 6:		Do_Dump3DProjection ();				break;
					case 7:		Do_DumpStorySettings ();			break;
					case 8:		Do_DumpDocumentFrom3DDefaults ();	break;
					/* ------ */
					case 10:	Do_ChangeProjectNotes ();			break;
					case 11:	Do_NewStory ();						break;
					case 12:	Do_ChangeStoryHeight ();			break;
					case 13:	Do_CreateGhostRecord ();			break;
					case 14:	Do_ChangeGhostRecord ();			break;
					case 15:	Do_DeleteGhostRecord ();			break;
					/* ------ */
					case 17:	Do_Hide ();							break;
					/* ------ */
					case 19:	Do_NewLibraryDialog ();				break;
					/* ------ */
					case 21:	Do_SwitchToArrow ();				break;
					/* ------ */
					case 23:	Do_ListLoadedLibraries ();			break;
					/* ------ */
					case 25:	Do_ChangeNavigatorItem ();			break;
					case 26:	Do_ChangeDocumentFrom3DDefaults ();	break;
					/* ------ */
					case 28:	Do_DumpPartialStructureDisplay ();	break;
					case 29:	Do_ChangePartialStructureDisplay ();break;
					/* ------ */
					case 31:	Do_ChangeModelViewOptions ();		break;
					case 32:	Do_Change3DCuttingPlanes ();		break;
					/* ------ */
					case 34:	Do_DumpRenderingSettings ();		break;
					case 35:	Do_DumpAllRenderingSceneNames ();	break;
					case 36:	Do_SetCurrentRenderingScene ();		break;
					case 37:	Do_TestEnvironmentFunctions ();		break;
			}
			break;
	}

	return NoError;
}		// MenuCommandHandler


// =============================================================================
//
// Required functions
//
// =============================================================================

#ifdef __APPLE__
#pragma mark -
#endif

// -----------------------------------------------------------------------------
// Dependency definitions
// -----------------------------------------------------------------------------

API_AddonType	__ACENV_CALL	CheckEnvironment (API_EnvirParams* envir)
{
	if (envir->serverInfo.serverApplication != APIAppl_ArchiCADID)
		return APIAddon_DontRegister;

	ACAPI_Resource_GetLocStr (envir->addOnInfo.name, 32000, 1);
	ACAPI_Resource_GetLocStr (envir->addOnInfo.description, 32000, 2);

	return APIAddon_Normal;
}		// CheckEnvironment


// -----------------------------------------------------------------------------
// Interface definitions
// -----------------------------------------------------------------------------

GSErrCode	__ACENV_CALL	RegisterInterface (void)
{
	GSErrCode err;

	err = ACAPI_Register_Menu (32500, 32520, MenuCode_UserDef, MenuFlag_Default);
	if (err != NoError)
		DBPrintf ("Environment Control:: RegisterInterface() ACAPI_Register_Menu failed\n");

	return err;
}		// RegisterInterface


// -----------------------------------------------------------------------------
// Called when the Add-On has been loaded into memory
// to perform an operation
// -----------------------------------------------------------------------------

GSErrCode	__ACENV_CALL Initialize	(void)
{
	GSErrCode err = ACAPI_Install_MenuHandler (32500, MenuCommandHandler);
	if (err != NoError)
		DBPrintf ("Environment Control:: Initialize() ACAPI_Install_MenuHandler failed\n");

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

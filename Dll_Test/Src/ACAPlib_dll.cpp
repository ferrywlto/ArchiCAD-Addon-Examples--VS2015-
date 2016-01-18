// *****************************************************************************
// Common support for ACAP_DLL.apx based addons
// API Development Kit 19
//
// Namespaces:		 Contact person:
//		 -None-
//
// [SG compatible] - Yes
// *****************************************************************************

// ---------------------------------- Includes ---------------------------------

#define DLL_TEST_EXPORTS
#include "ACAPlib_dll.h"

#if defined (macintosh)
#include "AutoCFTypeRef.hpp"
#endif

#if defined (WINDOWS)
	#include "Win32UxInterface.hpp"
#endif
// ---------------------------------- Types ------------------------------------

struct ExpList_ACAP {
	Int32					version;

	CheckEnvironmentProc	*checkenvironment;
	RegisterInterfaceProc	*registerinterface;
	InitializeProc			*initialize;
	FreeDataProc			*freedata;

	Int32					devkitNumber;

	GSPtr					filler[9];
};


#define	API_MAIN_VERSION		19 // APIVersion::V1900
#define	API_RELEASE_VERSION		1  // APIVersion::Dev

#define	API_DEVKIT_NUMBER		19000

// =============================================================================
//
// 							The communication protocol
//
// =============================================================================

static	GSErrCode		__ACDLL_CALL	My_Initialize (void* clientPtr);
static	GSErrCode		__ACDLL_CALL	My_FreeData (void);

ClientID		gOwnClientID = NULL;		// pointer to the client in the API database (valid only between Initialize and FreeData)
API_functions*	gFunctionTable = NULL;
GSResModule		gACAPIDLL = 0;

// ---------------------------------------------------------------
// Dll entry
// ---------------------------------------------------------------

#define 	DLLMAINIMP_PLUGIN
#if !defined (COMP_API_DLL)
	#define		CALL_DLL_INIT			1
	#define 	CALL_DLL_TERM			1
	#define		IMPLEMENT_DLL_MAINENTRY	1
#endif

#include "DllMainImp.hpp"

static APIDllTermHookProc*	gDllTermHookProc = NULL;

#ifdef WINDOWS
static HINSTANCE		gExtensionInstance = NULL;
#endif

GSResModule		gOwnResModule = InvalidResModule;			// resource module of own resource fork

static DllMainImp_LocationSpec			gOwnLocationData;

static void		OpenOwnResFile (const DllMainImp_InitContext* ctx);
static void		CloseOwnResFile (void);

static bool		DllInit (const DllMainImp_InitContext* ctx)
{
#if defined (WINDOWS)
	RegisterWindowMessage (FILEOKSTRING);
	gExtensionInstance = DllMainImp_GetWindowsHInstance (ctx);
#endif
	gOwnLocationData = DllMainImp_GetLocation (ctx);
	OpenOwnResFile (ctx);

	return true;
}


static bool		DllTerm ()
{
	CloseOwnResFile ();

	if (gOwnClientID != 0) {
		DBPrintf ("Abnormal add-on termination (\"%s\")", gOwnLocationData.mFullPath);
		if (gDllTermHookProc != NULL) {
			DBPrintf (", calling custom DllTermHookProc\n");
			(*gDllTermHookProc) ();
		} else {
			DBPrintf ("\n");
		}

		gOwnClientID = 0;
	}

	return true;
}


// ---------------------------------------------------------------
// Open the addon's resource fork
// Setup 'gOwnResModule' for later references
// ---------------------------------------------------------------
static void		OpenOwnResFile (const DllMainImp_InitContext* ctx)
{
	gOwnResModule = DllMainImp_OpenResModule (ctx);

	return;
}		/* OpenOwnResFile */


// ---------------------------------------------------------------
// Close the addon's resource fork
// ---------------------------------------------------------------
static void		CloseOwnResFile (void)
{
	if (gOwnResModule != InvalidResModule) {
		DllMainImp_CloseResModule (gOwnResModule);
	}
	gOwnResModule = InvalidResModule;

	return;
}		/* CloseOwnResFile */


// ---------------------------------------------------------------
// Pass the required entry points to the server application
// ---------------------------------------------------------------
ACAP_DLL_CLIENT void __ACDLL_CALL	GetExportedFuncAddrs (GSPtr expList)
{
	ExpList_ACAP	*expPtr;
	memset (expList, 0, sizeof(ExpList_ACAP));

	expPtr = (ExpList_ACAP *) expList;

	expPtr->version = (API_MAIN_VERSION << 16) | (API_RELEASE_VERSION & 0xFFFF);

	expPtr->checkenvironment	= CheckEnvironment;
	expPtr->registerinterface	= RegisterInterface;
	expPtr->initialize			= My_Initialize;
	expPtr->freedata			= My_FreeData;

	expPtr->devkitNumber		= API_DEVKIT_NUMBER;

	return;
}		/* GetExportedFuncAddrs */


// ---------------------------------------------------------------
// Receive the callbacks from the server application
// ---------------------------------------------------------------
ACAP_DLL_CLIENT GSErrCode __ACDLL_CALL	SetImportedFuncAddrs (GSPtr /*impList*/)
{
	return NoError;
}		/* SetImportedFuncAddrs */


// ---------------------------------------------------------------
// Receive the callbacks from the server application
// ---------------------------------------------------------------
static void* GetProcAddress (GSResModule moduleID, const char* procName)
{
	if (moduleID == 0)
		return  NULL;

#ifdef macintosh
		CFBundleRef bundle = reinterpret_cast<CFBundleRef> (moduleID);
		DBASSERT (bundle != 0);
		GS::AutoCFTypeRef <CFStringRef> functionName (::CFStringCreateWithCString (kCFAllocatorDefault, procName, kCFStringEncodingASCII));
		void* result = ::CFBundleGetFunctionPointerForName (bundle, functionName.Get ());
		return result;
#endif

#if defined (WINDOWS)
	HINSTANCE hLibInstance = reinterpret_cast<HINSTANCE> (moduleID);
	return  ::GetProcAddress (hLibInstance, procName);
#endif
}

// ---------------------------------------------------------------
// Receive the callbacks from the server application
// ---------------------------------------------------------------
ACAP_DLL_CLIENT GSErrCode __ACDLL_CALL	SetACAPDll (GSResModule ACAPIHandle)
{
	if (gFunctionTable != NULL) {
		delete gFunctionTable;
		gFunctionTable = NULL;
	}

	gFunctionTable = new API_functions;
	if (gFunctionTable == NULL)
		return Error;

	BNZeroMemory (gFunctionTable, sizeof(API_functions));

	gACAPIDLL = ACAPIHandle;
	if (gACAPIDLL == 0)
		return Error;

	gFunctionTable->acapi_3d_createsight                                      = (acapi_3d_createsightProc)                                    GetProcAddress (gACAPIDLL, "ACAPI_3D_CreateSight");
	gFunctionTable->acapi_3d_decomposepgon                                    = (acapi_3d_decomposepgonProc)                                  GetProcAddress (gACAPIDLL, "ACAPI_3D_DecomposePgon");
	gFunctionTable->acapi_3d_deletesight                                      = (acapi_3d_deletesightProc)                                    GetProcAddress (gACAPIDLL, "ACAPI_3D_DeleteSight");
	gFunctionTable->acapi_3d_getcomponent                                     = (acapi_3d_getcomponentProc)                                   GetProcAddress (gACAPIDLL, "ACAPI_3D_GetComponent");
	gFunctionTable->acapi_3d_getcurrentwindowsight                            = (acapi_3d_getcurrentwindowsightProc)                          GetProcAddress (gACAPIDLL, "ACAPI_3D_GetCurrentWindowSight");
	gFunctionTable->acapi_3d_getcutpolygoninfo                                = (acapi_3d_getcutpolygoninfoProc)                              GetProcAddress (gACAPIDLL, "ACAPI_3D_GetCutPolygonInfo");
	gFunctionTable->acapi_3d_getnum                                           = (acapi_3d_getnumProc)                                         GetProcAddress (gACAPIDLL, "ACAPI_3D_GetNum");
	gFunctionTable->acapi_3d_selectsight                                      = (acapi_3d_selectsightProc)                                    GetProcAddress (gACAPIDLL, "ACAPI_3D_SelectSight");
	gFunctionTable->acapi_activatepropertyhandler                             = (acapi_activatepropertyhandlerProc)                           GetProcAddress (gACAPIDLL, "ACAPI_ActivatePropertyHandler");
	gFunctionTable->acapi_attribute_create                                    = (acapi_attribute_createProc)                                  GetProcAddress (gACAPIDLL, "ACAPI_Attribute_Create");
	gFunctionTable->acapi_attribute_createext                                 = (acapi_attribute_createextProc)                               GetProcAddress (gACAPIDLL, "ACAPI_Attribute_CreateExt");
	gFunctionTable->acapi_attribute_delete                                    = (acapi_attribute_deleteProc)                                  GetProcAddress (gACAPIDLL, "ACAPI_Attribute_Delete");
	gFunctionTable->acapi_attribute_deletemore                                = (acapi_attribute_deletemoreProc)                              GetProcAddress (gACAPIDLL, "ACAPI_Attribute_DeleteMore");
	gFunctionTable->acapi_attribute_get                                       = (acapi_attribute_getProc)                                     GetProcAddress (gACAPIDLL, "ACAPI_Attribute_Get");
	gFunctionTable->acapi_attribute_getdef                                    = (acapi_attribute_getdefProc)                                  GetProcAddress (gACAPIDLL, "ACAPI_Attribute_GetDef");
	gFunctionTable->acapi_attribute_getdefext                                 = (acapi_attribute_getdefextProc)                               GetProcAddress (gACAPIDLL, "ACAPI_Attribute_GetDefExt");
	gFunctionTable->acapi_attribute_getnum                                    = (acapi_attribute_getnumProc)                                  GetProcAddress (gACAPIDLL, "ACAPI_Attribute_GetNum");
	gFunctionTable->acapi_attribute_getuserdata                               = (acapi_attribute_getuserdataProc)                             GetProcAddress (gACAPIDLL, "ACAPI_Attribute_GetUserData");
	gFunctionTable->acapi_attribute_modify                                    = (acapi_attribute_modifyProc)                                  GetProcAddress (gACAPIDLL, "ACAPI_Attribute_Modify");
	gFunctionTable->acapi_attribute_modifyext                                 = (acapi_attribute_modifyextProc)                               GetProcAddress (gACAPIDLL, "ACAPI_Attribute_ModifyExt");
	gFunctionTable->acapi_attribute_search                                    = (acapi_attribute_searchProc)                                  GetProcAddress (gACAPIDLL, "ACAPI_Attribute_Search");
	gFunctionTable->acapi_attribute_setuserdata                               = (acapi_attribute_setuserdataProc)                             GetProcAddress (gACAPIDLL, "ACAPI_Attribute_SetUserData");
	gFunctionTable->acapi_automate                                            = (acapi_automateProc)                                          GetProcAddress (gACAPIDLL, "ACAPI_Automate");
	gFunctionTable->acapi_body_addedge                                        = (acapi_body_addedgeProc)                                      GetProcAddress (gACAPIDLL, "ACAPI_Body_AddEdge");
	gFunctionTable->acapi_body_addpolynormal                                  = (acapi_body_addpolynormalProc)                                GetProcAddress (gACAPIDLL, "ACAPI_Body_AddPolyNormal");
	gFunctionTable->acapi_body_addpolygon                                     = (acapi_body_addpolygonProc)                                   GetProcAddress (gACAPIDLL, "ACAPI_Body_AddPolygon");
	gFunctionTable->acapi_body_addvertex                                      = (acapi_body_addvertexProc)                                    GetProcAddress (gACAPIDLL, "ACAPI_Body_AddVertex");
	gFunctionTable->acapi_body_create                                         = (acapi_body_createProc)                                       GetProcAddress (gACAPIDLL, "ACAPI_Body_Create");
	gFunctionTable->acapi_body_dispose                                        = (acapi_body_disposeProc)                                      GetProcAddress (gACAPIDLL, "ACAPI_Body_Dispose");
	gFunctionTable->acapi_body_finish                                         = (acapi_body_finishProc)                                       GetProcAddress (gACAPIDLL, "ACAPI_Body_Finish");
	gFunctionTable->acapi_callundoablecommand                                 = (acapi_callundoablecommandProc)                               GetProcAddress (gACAPIDLL, "ACAPI_CallUndoableCommand");
	gFunctionTable->acapi_command_call                                        = (acapi_command_callProc)                                      GetProcAddress (gACAPIDLL, "ACAPI_Command_Call");
	gFunctionTable->acapi_command_callfromeventloop                           = (acapi_command_callfromeventloopProc)                         GetProcAddress (gACAPIDLL, "ACAPI_Command_CallFromEventLoop");
	gFunctionTable->acapi_command_externalcall                                = (acapi_command_externalcallProc)                              GetProcAddress (gACAPIDLL, "ACAPI_Command_ExternalCall");
	gFunctionTable->acapi_command_test                                        = (acapi_command_testProc)                                      GetProcAddress (gACAPIDLL, "ACAPI_Command_Test");
	gFunctionTable->acapi_database                                            = (acapi_databaseProc)                                          GetProcAddress (gACAPIDLL, "ACAPI_Database");
	gFunctionTable->acapi_disposeaddparhdl                                    = (acapi_disposeaddparhdlProc)                                  GetProcAddress (gACAPIDLL, "ACAPI_DisposeAddParHdl");
	gFunctionTable->acapi_disposeattrdefshdls                                 = (acapi_disposeattrdefshdlsProc)                               GetProcAddress (gACAPIDLL, "ACAPI_DisposeAttrDefsHdls");
	gFunctionTable->acapi_disposeattrdefshdlsext                              = (acapi_disposeattrdefshdlsextProc)                            GetProcAddress (gACAPIDLL, "ACAPI_DisposeAttrDefsHdlsExt");
	gFunctionTable->acapi_disposebeamrelationhdls                             = (acapi_disposebeamrelationhdlsProc)                           GetProcAddress (gACAPIDLL, "ACAPI_DisposeBeamRelationHdls");
	gFunctionTable->acapi_disposeelemmemohdls                                 = (acapi_disposeelemmemohdlsProc)                               GetProcAddress (gACAPIDLL, "ACAPI_DisposeElemMemoHdls");
	gFunctionTable->acapi_disposeparagraphshdl                                = (acapi_disposeparagraphshdlProc)                              GetProcAddress (gACAPIDLL, "ACAPI_DisposeParagraphsHdl");
	gFunctionTable->acapi_disposeroomrelationhdls                             = (acapi_disposeroomrelationhdlsProc)                           GetProcAddress (gACAPIDLL, "ACAPI_DisposeRoomRelationHdls");
	gFunctionTable->acapi_disposewallrelationhdls                             = (acapi_disposewallrelationhdlsProc)                           GetProcAddress (gACAPIDLL, "ACAPI_DisposeWallRelationHdls");
	gFunctionTable->acapi_disposeworkspaceinfohdls                            = (acapi_disposeworkspaceinfohdlsProc)                          GetProcAddress (gACAPIDLL, "ACAPI_DisposeWorkspaceInfoHdls");
	gFunctionTable->acapi_elementgroup_create                                 = (acapi_elementgroup_createProc)                               GetProcAddress (gACAPIDLL, "ACAPI_ElementGroup_Create");
	gFunctionTable->acapi_elementgroup_getallgroupedelems                     = (acapi_elementgroup_getallgroupedelemsProc)                   GetProcAddress (gACAPIDLL, "ACAPI_ElementGroup_GetAllGroupedElems");
	gFunctionTable->acapi_elementgroup_getgroup                               = (acapi_elementgroup_getgroupProc)                             GetProcAddress (gACAPIDLL, "ACAPI_ElementGroup_GetGroup");
	gFunctionTable->acapi_elementgroup_getgroupedelems                        = (acapi_elementgroup_getgroupedelemsProc)                      GetProcAddress (gACAPIDLL, "ACAPI_ElementGroup_GetGroupedElems");
	gFunctionTable->acapi_elementgroup_getrootgroup                           = (acapi_elementgroup_getrootgroupProc)                         GetProcAddress (gACAPIDLL, "ACAPI_ElementGroup_GetRootGroup");
	gFunctionTable->acapi_elementgroup_getuserdata                            = (acapi_elementgroup_getuserdataProc)                          GetProcAddress (gACAPIDLL, "ACAPI_ElementGroup_GetUserData");
	gFunctionTable->acapi_elementgroup_setuserdata                            = (acapi_elementgroup_setuserdataProc)                          GetProcAddress (gACAPIDLL, "ACAPI_ElementGroup_SetUserData");
	gFunctionTable->acapi_elementset_create                                   = (acapi_elementset_createProc)                                 GetProcAddress (gACAPIDLL, "ACAPI_ElementSet_Create");
	gFunctionTable->acapi_elementset_delete                                   = (acapi_elementset_deleteProc)                                 GetProcAddress (gACAPIDLL, "ACAPI_ElementSet_Delete");
	gFunctionTable->acapi_elementset_getdata                                  = (acapi_elementset_getdataProc)                                GetProcAddress (gACAPIDLL, "ACAPI_ElementSet_GetData");
	gFunctionTable->acapi_elementset_identify                                 = (acapi_elementset_identifyProc)                               GetProcAddress (gACAPIDLL, "ACAPI_ElementSet_Identify");
	gFunctionTable->acapi_element_attachobserver                              = (acapi_element_attachobserverProc)                            GetProcAddress (gACAPIDLL, "ACAPI_Element_AttachObserver");
	gFunctionTable->acapi_element_change                                      = (acapi_element_changeProc)                                    GetProcAddress (gACAPIDLL, "ACAPI_Element_Change");
	gFunctionTable->acapi_element_changedefaults                              = (acapi_element_changedefaultsProc)                            GetProcAddress (gACAPIDLL, "ACAPI_Element_ChangeDefaults");
	gFunctionTable->acapi_element_changedefaultsext                           = (acapi_element_changedefaultsextProc)                         GetProcAddress (gACAPIDLL, "ACAPI_Element_ChangeDefaultsExt");
	gFunctionTable->acapi_element_changeext                                   = (acapi_element_changeextProc)                                 GetProcAddress (gACAPIDLL, "ACAPI_Element_ChangeExt");
	gFunctionTable->acapi_element_changememo                                  = (acapi_element_changememoProc)                                GetProcAddress (gACAPIDLL, "ACAPI_Element_ChangeMemo");
	gFunctionTable->acapi_element_changemore                                  = (acapi_element_changemoreProc)                                GetProcAddress (gACAPIDLL, "ACAPI_Element_ChangeMore");
	gFunctionTable->acapi_element_changeparameters                            = (acapi_element_changeparametersProc)                          GetProcAddress (gACAPIDLL, "ACAPI_Element_ChangeParameters");
	gFunctionTable->acapi_element_create                                      = (acapi_element_createProc)                                    GetProcAddress (gACAPIDLL, "ACAPI_Element_Create");
	gFunctionTable->acapi_element_createext                                   = (acapi_element_createextProc)                                 GetProcAddress (gACAPIDLL, "ACAPI_Element_CreateExt");
	gFunctionTable->acapi_element_decompose                                   = (acapi_element_decomposeProc)                                 GetProcAddress (gACAPIDLL, "ACAPI_Element_Decompose");
	gFunctionTable->acapi_element_delete                                      = (acapi_element_deleteProc)                                    GetProcAddress (gACAPIDLL, "ACAPI_Element_Delete");
	gFunctionTable->acapi_element_deleteuserdata                              = (acapi_element_deleteuserdataProc)                            GetProcAddress (gACAPIDLL, "ACAPI_Element_DeleteUserData");
	gFunctionTable->acapi_element_detachobserver                              = (acapi_element_detachobserverProc)                            GetProcAddress (gACAPIDLL, "ACAPI_Element_DetachObserver");
	gFunctionTable->acapi_element_drwguidtolinkid                             = (acapi_element_drwguidtolinkidProc)                           GetProcAddress (gACAPIDLL, "ACAPI_Element_DrwGuidToLinkId");
	gFunctionTable->acapi_element_edit                                        = (acapi_element_editProc)                                      GetProcAddress (gACAPIDLL, "ACAPI_Element_Edit");
	gFunctionTable->acapi_element_filter                                      = (acapi_element_filterProc)                                    GetProcAddress (gACAPIDLL, "ACAPI_Element_Filter");
	gFunctionTable->acapi_element_get                                         = (acapi_element_getProc)                                       GetProcAddress (gACAPIDLL, "ACAPI_Element_Get");
	gFunctionTable->acapi_element_get3dinfo                                   = (acapi_element_get3dinfoProc)                                 GetProcAddress (gACAPIDLL, "ACAPI_Element_Get3DInfo");
	gFunctionTable->acapi_element_getcategoryvalue                            = (acapi_element_getcategoryvalueProc)                          GetProcAddress (gACAPIDLL, "ACAPI_Element_GetCategoryValue");
	gFunctionTable->acapi_element_getcategoryvaluedefault                     = (acapi_element_getcategoryvaluedefaultProc)                   GetProcAddress (gACAPIDLL, "ACAPI_Element_GetCategoryValueDefault");
	gFunctionTable->acapi_element_getcomponents                               = (acapi_element_getcomponentsProc)                             GetProcAddress (gACAPIDLL, "ACAPI_Element_GetComponents");
	gFunctionTable->acapi_element_getconnectedelements                        = (acapi_element_getconnectedelementsProc)                      GetProcAddress (gACAPIDLL, "ACAPI_Element_GetConnectedElements");
	gFunctionTable->acapi_element_getdefaults                                 = (acapi_element_getdefaultsProc)                               GetProcAddress (gACAPIDLL, "ACAPI_Element_GetDefaults");
	gFunctionTable->acapi_element_getdefaultsext                              = (acapi_element_getdefaultsextProc)                            GetProcAddress (gACAPIDLL, "ACAPI_Element_GetDefaultsExt");
	gFunctionTable->acapi_element_getdescriptors                              = (acapi_element_getdescriptorsProc)                            GetProcAddress (gACAPIDLL, "ACAPI_Element_GetDescriptors");
	gFunctionTable->acapi_element_getelemlist                                 = (acapi_element_getelemlistProc)                               GetProcAddress (gACAPIDLL, "ACAPI_Element_GetElemList");
	gFunctionTable->acapi_element_getelemlistbyifcidentifier                  = (acapi_element_getelemlistbyifcidentifierProc)                GetProcAddress (gACAPIDLL, "ACAPI_Element_GetElemListByIFCIdentifier");
	gFunctionTable->acapi_element_getheader                                   = (acapi_element_getheaderProc)                                 GetProcAddress (gACAPIDLL, "ACAPI_Element_GetHeader");
	gFunctionTable->acapi_element_gethotspots                                 = (acapi_element_gethotspotsProc)                               GetProcAddress (gACAPIDLL, "ACAPI_Element_GetHotspots");
	gFunctionTable->acapi_element_getifcattributes                            = (acapi_element_getifcattributesProc)                          GetProcAddress (gACAPIDLL, "ACAPI_Element_GetIFCAttributes");
	gFunctionTable->acapi_element_getifcclassificationreferences              = (acapi_element_getifcclassificationreferencesProc)            GetProcAddress (gACAPIDLL, "ACAPI_Element_GetIFCClassificationReferences");
	gFunctionTable->acapi_element_getifcidentifier                            = (acapi_element_getifcidentifierProc)                          GetProcAddress (gACAPIDLL, "ACAPI_Element_GetIFCIdentifier");
	gFunctionTable->acapi_element_getifcproperties                            = (acapi_element_getifcpropertiesProc)                          GetProcAddress (gACAPIDLL, "ACAPI_Element_GetIFCProperties");
	gFunctionTable->acapi_element_getifcpropertyvalueprimitivetype            = (acapi_element_getifcpropertyvalueprimitivetypeProc)          GetProcAddress (gACAPIDLL, "ACAPI_Element_GetIFCPropertyValuePrimitiveType");
	gFunctionTable->acapi_element_getlinkflags                                = (acapi_element_getlinkflagsProc)                              GetProcAddress (gACAPIDLL, "ACAPI_Element_GetLinkFlags");
	gFunctionTable->acapi_element_getlinkedproperties                         = (acapi_element_getlinkedpropertiesProc)                       GetProcAddress (gACAPIDLL, "ACAPI_Element_GetLinkedProperties");
	gFunctionTable->acapi_element_getlinks                                    = (acapi_element_getlinksProc)                                  GetProcAddress (gACAPIDLL, "ACAPI_Element_GetLinks");
	gFunctionTable->acapi_element_getmemo                                     = (acapi_element_getmemoProc)                                   GetProcAddress (gACAPIDLL, "ACAPI_Element_GetMemo");
	gFunctionTable->acapi_element_getmorequantities                           = (acapi_element_getmorequantitiesProc)                         GetProcAddress (gACAPIDLL, "ACAPI_Element_GetMoreQuantities");
	gFunctionTable->acapi_element_getobservedelements                         = (acapi_element_getobservedelementsProc)                       GetProcAddress (gACAPIDLL, "ACAPI_Element_GetObservedElements");
	gFunctionTable->acapi_element_getproperties                               = (acapi_element_getpropertiesProc)                             GetProcAddress (gACAPIDLL, "ACAPI_Element_GetProperties");
	gFunctionTable->acapi_element_getquantities                               = (acapi_element_getquantitiesProc)                             GetProcAddress (gACAPIDLL, "ACAPI_Element_GetQuantities");
	gFunctionTable->acapi_element_getrelations                                = (acapi_element_getrelationsProc)                              GetProcAddress (gACAPIDLL, "ACAPI_Element_GetRelations");
	gFunctionTable->acapi_element_getsytran                                   = (acapi_element_getsytranProc)                                 GetProcAddress (gACAPIDLL, "ACAPI_Element_GetSyTran");
	gFunctionTable->acapi_element_getuserdata                                 = (acapi_element_getuserdataProc)                               GetProcAddress (gACAPIDLL, "ACAPI_Element_GetUserData");
	gFunctionTable->acapi_element_guidtounid                                  = (acapi_element_guidtounidProc)                                GetProcAddress (gACAPIDLL, "ACAPI_Element_GuidToUnId");
	gFunctionTable->acapi_element_link                                        = (acapi_element_linkProc)                                      GetProcAddress (gACAPIDLL, "ACAPI_Element_Link");
	gFunctionTable->acapi_element_linkidtodrwguid                             = (acapi_element_linkidtodrwguidProc)                           GetProcAddress (gACAPIDLL, "ACAPI_Element_LinkIdToDrwGuid");
	gFunctionTable->acapi_element_merge_elements                              = (acapi_element_merge_elementsProc)                            GetProcAddress (gACAPIDLL, "ACAPI_Element_Merge_Elements");
	gFunctionTable->acapi_element_merge_getmergedelements                     = (acapi_element_merge_getmergedelementsProc)                   GetProcAddress (gACAPIDLL, "ACAPI_Element_Merge_GetMergedElements");
	gFunctionTable->acapi_element_merge_remove                                = (acapi_element_merge_removeProc)                              GetProcAddress (gACAPIDLL, "ACAPI_Element_Merge_Remove");
	gFunctionTable->acapi_element_removeifcclassificationreference            = (acapi_element_removeifcclassificationreferenceProc)          GetProcAddress (gACAPIDLL, "ACAPI_Element_RemoveIFCClassificationReference");
	gFunctionTable->acapi_element_removeifcproperty                           = (acapi_element_removeifcpropertyProc)                         GetProcAddress (gACAPIDLL, "ACAPI_Element_RemoveIFCProperty");
	gFunctionTable->acapi_element_select                                      = (acapi_element_selectProc)                                    GetProcAddress (gACAPIDLL, "ACAPI_Element_Select");
	gFunctionTable->acapi_element_setcategoryvalue                            = (acapi_element_setcategoryvalueProc)                          GetProcAddress (gACAPIDLL, "ACAPI_Element_SetCategoryValue");
	gFunctionTable->acapi_element_setcategoryvaluedefault                     = (acapi_element_setcategoryvaluedefaultProc)                   GetProcAddress (gACAPIDLL, "ACAPI_Element_SetCategoryValueDefault");
	gFunctionTable->acapi_element_setifcattribute                             = (acapi_element_setifcattributeProc)                           GetProcAddress (gACAPIDLL, "ACAPI_Element_SetIFCAttribute");
	gFunctionTable->acapi_element_setifcclassificationreference               = (acapi_element_setifcclassificationreferenceProc)             GetProcAddress (gACAPIDLL, "ACAPI_Element_SetIFCClassificationReference");
	gFunctionTable->acapi_element_setifcproperty                              = (acapi_element_setifcpropertyProc)                            GetProcAddress (gACAPIDLL, "ACAPI_Element_SetIFCProperty");
	gFunctionTable->acapi_element_setlinkedproperties                         = (acapi_element_setlinkedpropertiesProc)                       GetProcAddress (gACAPIDLL, "ACAPI_Element_SetLinkedProperties");
	gFunctionTable->acapi_element_setuserdata                                 = (acapi_element_setuserdataProc)                               GetProcAddress (gACAPIDLL, "ACAPI_Element_SetUserData");
	gFunctionTable->acapi_element_shapeprims                                  = (acapi_element_shapeprimsProc)                                GetProcAddress (gACAPIDLL, "ACAPI_Element_ShapePrims");
	gFunctionTable->acapi_element_shapeprimsext                               = (acapi_element_shapeprimsextProc)                             GetProcAddress (gACAPIDLL, "ACAPI_Element_ShapePrimsExt");
	gFunctionTable->acapi_element_solidlink_create                            = (acapi_element_solidlink_createProc)                          GetProcAddress (gACAPIDLL, "ACAPI_Element_SolidLink_Create");
	gFunctionTable->acapi_element_solidlink_getflags                          = (acapi_element_solidlink_getflagsProc)                        GetProcAddress (gACAPIDLL, "ACAPI_Element_SolidLink_GetFlags");
	gFunctionTable->acapi_element_solidlink_getoperation                      = (acapi_element_solidlink_getoperationProc)                    GetProcAddress (gACAPIDLL, "ACAPI_Element_SolidLink_GetOperation");
	gFunctionTable->acapi_element_solidlink_getoperators                      = (acapi_element_solidlink_getoperatorsProc)                    GetProcAddress (gACAPIDLL, "ACAPI_Element_SolidLink_GetOperators");
	gFunctionTable->acapi_element_solidlink_gettargets                        = (acapi_element_solidlink_gettargetsProc)                      GetProcAddress (gACAPIDLL, "ACAPI_Element_SolidLink_GetTargets");
	gFunctionTable->acapi_element_solidlink_gettime                           = (acapi_element_solidlink_gettimeProc)                         GetProcAddress (gACAPIDLL, "ACAPI_Element_SolidLink_GetTime");
	gFunctionTable->acapi_element_solidlink_remove                            = (acapi_element_solidlink_removeProc)                          GetProcAddress (gACAPIDLL, "ACAPI_Element_SolidLink_Remove");
	gFunctionTable->acapi_element_tool                                        = (acapi_element_toolProc)                                      GetProcAddress (gACAPIDLL, "ACAPI_Element_Tool");
	gFunctionTable->acapi_element_trim_elements                               = (acapi_element_trim_elementsProc)                             GetProcAddress (gACAPIDLL, "ACAPI_Element_Trim_Elements");
	gFunctionTable->acapi_element_trim_elementswith                           = (acapi_element_trim_elementswithProc)                         GetProcAddress (gACAPIDLL, "ACAPI_Element_Trim_ElementsWith");
	gFunctionTable->acapi_element_trim_gettrimtype                            = (acapi_element_trim_gettrimtypeProc)                          GetProcAddress (gACAPIDLL, "ACAPI_Element_Trim_GetTrimType");
	gFunctionTable->acapi_element_trim_gettrimmedelements                     = (acapi_element_trim_gettrimmedelementsProc)                   GetProcAddress (gACAPIDLL, "ACAPI_Element_Trim_GetTrimmedElements");
	gFunctionTable->acapi_element_trim_gettrimmingelements                    = (acapi_element_trim_gettrimmingelementsProc)                  GetProcAddress (gACAPIDLL, "ACAPI_Element_Trim_GetTrimmingElements");
	gFunctionTable->acapi_element_trim_remove                                 = (acapi_element_trim_removeProc)                               GetProcAddress (gACAPIDLL, "ACAPI_Element_Trim_Remove");
	gFunctionTable->acapi_element_unidtoguid                                  = (acapi_element_unidtoguidProc)                                GetProcAddress (gACAPIDLL, "ACAPI_Element_UnIdToGuid");
	gFunctionTable->acapi_element_unlink                                      = (acapi_element_unlinkProc)                                    GetProcAddress (gACAPIDLL, "ACAPI_Element_Unlink");
	gFunctionTable->acapi_environment                                         = (acapi_environmentProc)                                       GetProcAddress (gACAPIDLL, "ACAPI_Environment");
	gFunctionTable->acapi_favorite_create                                     = (acapi_favorite_createProc)                                   GetProcAddress (gACAPIDLL, "ACAPI_Favorite_Create");
	gFunctionTable->acapi_favorite_delete                                     = (acapi_favorite_deleteProc)                                   GetProcAddress (gACAPIDLL, "ACAPI_Favorite_Delete");
	gFunctionTable->acapi_favorite_getdefaults                                = (acapi_favorite_getdefaultsProc)                              GetProcAddress (gACAPIDLL, "ACAPI_Favorite_GetDefaults");
	gFunctionTable->acapi_favorite_getnum                                     = (acapi_favorite_getnumProc)                                   GetProcAddress (gACAPIDLL, "ACAPI_Favorite_GetNum");
	gFunctionTable->acapi_freecwcontourptr                                    = (acapi_freecwcontourptrProc)                                  GetProcAddress (gACAPIDLL, "ACAPI_FreeCWContourPtr");
	gFunctionTable->acapi_freegdlmodelviewoptionsptr                          = (acapi_freegdlmodelviewoptionsptrProc)                        GetProcAddress (gACAPIDLL, "ACAPI_FreeGDLModelViewOptionsPtr");
	gFunctionTable->acapi_freegridmeshptr                                     = (acapi_freegridmeshptrProc)                                   GetProcAddress (gACAPIDLL, "ACAPI_FreeGridMeshPtr");
#if defined(WINDOWS)
	gFunctionTable->acapi_getextensioninstance                                = (acapi_getextensioninstanceProc)                              GetProcAddress (gACAPIDLL, "ACAPI_GetExtensionInstance");
#endif
#if defined(WINDOWS)
	gFunctionTable->acapi_getmainwindow                                       = (acapi_getmainwindowProc)                                     GetProcAddress (gACAPIDLL, "ACAPI_GetMainWindow");
#endif
	gFunctionTable->acapi_getownlocation                                      = (acapi_getownlocationProc)                                    GetProcAddress (gACAPIDLL, "ACAPI_GetOwnLocation");
	gFunctionTable->acapi_getownresmodule                                     = (acapi_getownresmoduleProc)                                   GetProcAddress (gACAPIDLL, "ACAPI_GetOwnResModule");
	gFunctionTable->acapi_getpreferences                                      = (acapi_getpreferencesProc)                                    GetProcAddress (gACAPIDLL, "ACAPI_GetPreferences");
	gFunctionTable->acapi_getpreferences_platform                             = (acapi_getpreferences_platformProc)                           GetProcAddress (gACAPIDLL, "ACAPI_GetPreferences_Platform");
	gFunctionTable->acapi_getreleasenumber                                    = (acapi_getreleasenumberProc)                                  GetProcAddress (gACAPIDLL, "ACAPI_GetReleaseNumber");
	gFunctionTable->acapi_goodies                                             = (acapi_goodiesProc)                                           GetProcAddress (gACAPIDLL, "ACAPI_Goodies");
	gFunctionTable->acapi_install_esymhandler                                 = (acapi_install_esymhandlerProc)                               GetProcAddress (gACAPIDLL, "ACAPI_Install_ESYMHandler");
	gFunctionTable->acapi_install_filetypehandler                             = (acapi_install_filetypehandlerProc)                           GetProcAddress (gACAPIDLL, "ACAPI_Install_FileTypeHandler");
	gFunctionTable->acapi_install_filetypehandler3d                           = (acapi_install_filetypehandler3dProc)                         GetProcAddress (gACAPIDLL, "ACAPI_Install_FileTypeHandler3D");
	gFunctionTable->acapi_install_menuhandler                                 = (acapi_install_menuhandlerProc)                               GetProcAddress (gACAPIDLL, "ACAPI_Install_MenuHandler");
	gFunctionTable->acapi_install_modulcommandhandler                         = (acapi_install_modulcommandhandlerProc)                       GetProcAddress (gACAPIDLL, "ACAPI_Install_ModulCommandHandler");
	gFunctionTable->acapi_install_moduldatamergehandler                       = (acapi_install_moduldatamergehandlerProc)                     GetProcAddress (gACAPIDLL, "ACAPI_Install_ModulDataMergeHandler");
	gFunctionTable->acapi_install_moduldatasaveoldformathandler               = (acapi_install_moduldatasaveoldformathandlerProc)             GetProcAddress (gACAPIDLL, "ACAPI_Install_ModulDataSaveOldFormatHandler");
	gFunctionTable->acapi_install_panelhandler                                = (acapi_install_panelhandlerProc)                              GetProcAddress (gACAPIDLL, "ACAPI_Install_PanelHandler");
	gFunctionTable->acapi_install_propertyhandler                             = (acapi_install_propertyhandlerProc)                           GetProcAddress (gACAPIDLL, "ACAPI_Install_PropertyHandler");
	gFunctionTable->acapi_interface                                           = (acapi_interfaceProc)                                         GetProcAddress (gACAPIDLL, "ACAPI_Interface");
	gFunctionTable->acapi_keepinmemory                                        = (acapi_keepinmemoryProc)                                      GetProcAddress (gACAPIDLL, "ACAPI_KeepInMemory");
	gFunctionTable->acapi_libpart_addsection                                  = (acapi_libpart_addsectionProc)                                GetProcAddress (gACAPIDLL, "ACAPI_LibPart_AddSection");
	gFunctionTable->acapi_libpart_create                                      = (acapi_libpart_createProc)                                    GetProcAddress (gACAPIDLL, "ACAPI_LibPart_Create");
	gFunctionTable->acapi_libpart_endsection                                  = (acapi_libpart_endsectionProc)                                GetProcAddress (gACAPIDLL, "ACAPI_LibPart_EndSection");
	gFunctionTable->acapi_libpart_get                                         = (acapi_libpart_getProc)                                       GetProcAddress (gACAPIDLL, "ACAPI_LibPart_Get");
	gFunctionTable->acapi_libpart_getdetails                                  = (acapi_libpart_getdetailsProc)                                GetProcAddress (gACAPIDLL, "ACAPI_LibPart_GetDetails");
	gFunctionTable->acapi_libpart_gethotspots                                 = (acapi_libpart_gethotspotsProc)                               GetProcAddress (gACAPIDLL, "ACAPI_LibPart_GetHotspots");
	gFunctionTable->acapi_libpart_getnum                                      = (acapi_libpart_getnumProc)                                    GetProcAddress (gACAPIDLL, "ACAPI_LibPart_GetNum");
	gFunctionTable->acapi_libpart_getparams                                   = (acapi_libpart_getparamsProc)                                 GetProcAddress (gACAPIDLL, "ACAPI_LibPart_GetParams");
	gFunctionTable->acapi_libpart_getsect_2ddrawhdl                           = (acapi_libpart_getsect_2ddrawhdlProc)                         GetProcAddress (gACAPIDLL, "ACAPI_LibPart_GetSect_2DDrawHdl");
	gFunctionTable->acapi_libpart_getsect_paramdef                            = (acapi_libpart_getsect_paramdefProc)                          GetProcAddress (gACAPIDLL, "ACAPI_LibPart_GetSect_ParamDef");
	gFunctionTable->acapi_libpart_getsection                                  = (acapi_libpart_getsectionProc)                                GetProcAddress (gACAPIDLL, "ACAPI_LibPart_GetSection");
	gFunctionTable->acapi_libpart_getsectionlist                              = (acapi_libpart_getsectionlistProc)                            GetProcAddress (gACAPIDLL, "ACAPI_LibPart_GetSectionList");
	gFunctionTable->acapi_libpart_newsection                                  = (acapi_libpart_newsectionProc)                                GetProcAddress (gACAPIDLL, "ACAPI_LibPart_NewSection");
	gFunctionTable->acapi_libpart_patternsearch                               = (acapi_libpart_patternsearchProc)                             GetProcAddress (gACAPIDLL, "ACAPI_LibPart_PatternSearch");
	gFunctionTable->acapi_libpart_register                                    = (acapi_libpart_registerProc)                                  GetProcAddress (gACAPIDLL, "ACAPI_LibPart_Register");
	gFunctionTable->acapi_libpart_registerall                                 = (acapi_libpart_registerallProc)                               GetProcAddress (gACAPIDLL, "ACAPI_LibPart_RegisterAll");
	gFunctionTable->acapi_libpart_save                                        = (acapi_libpart_saveProc)                                      GetProcAddress (gACAPIDLL, "ACAPI_LibPart_Save");
	gFunctionTable->acapi_libpart_search                                      = (acapi_libpart_searchProc)                                    GetProcAddress (gACAPIDLL, "ACAPI_LibPart_Search");
	gFunctionTable->acapi_libpart_setdetails_paramdef                         = (acapi_libpart_setdetails_paramdefProc)                       GetProcAddress (gACAPIDLL, "ACAPI_LibPart_SetDetails_ParamDef");
	gFunctionTable->acapi_libpart_setupsect_2ddrawhdl                         = (acapi_libpart_setupsect_2ddrawhdlProc)                       GetProcAddress (gACAPIDLL, "ACAPI_LibPart_SetUpSect_2DDrawHdl");
	gFunctionTable->acapi_libpart_shapeprims                                  = (acapi_libpart_shapeprimsProc)                                GetProcAddress (gACAPIDLL, "ACAPI_LibPart_ShapePrims");
	gFunctionTable->acapi_libpart_updatesection                               = (acapi_libpart_updatesectionProc)                             GetProcAddress (gACAPIDLL, "ACAPI_LibPart_UpdateSection");
	gFunctionTable->acapi_libpart_writesection                                = (acapi_libpart_writesectionProc)                              GetProcAddress (gACAPIDLL, "ACAPI_LibPart_WriteSection");
	gFunctionTable->acapi_listdata_get                                        = (acapi_listdata_getProc)                                      GetProcAddress (gACAPIDLL, "ACAPI_ListData_Get");
	gFunctionTable->acapi_listdata_getlocal                                   = (acapi_listdata_getlocalProc)                                 GetProcAddress (gACAPIDLL, "ACAPI_ListData_GetLocal");
	gFunctionTable->acapi_listdata_getnum                                     = (acapi_listdata_getnumProc)                                   GetProcAddress (gACAPIDLL, "ACAPI_ListData_GetNum");
	gFunctionTable->acapi_listdata_getset                                     = (acapi_listdata_getsetProc)                                   GetProcAddress (gACAPIDLL, "ACAPI_ListData_GetSet");
	gFunctionTable->acapi_listdata_getsetnum                                  = (acapi_listdata_getsetnumProc)                                GetProcAddress (gACAPIDLL, "ACAPI_ListData_GetSetNum");
	gFunctionTable->acapi_listdata_search                                     = (acapi_listdata_searchProc)                                   GetProcAddress (gACAPIDLL, "ACAPI_ListData_Search");
	gFunctionTable->acapi_moduldata_delete                                    = (acapi_moduldata_deleteProc)                                  GetProcAddress (gACAPIDLL, "ACAPI_ModulData_Delete");
	gFunctionTable->acapi_moduldata_get                                       = (acapi_moduldata_getProc)                                     GetProcAddress (gACAPIDLL, "ACAPI_ModulData_Get");
	gFunctionTable->acapi_moduldata_getinfo                                   = (acapi_moduldata_getinfoProc)                                 GetProcAddress (gACAPIDLL, "ACAPI_ModulData_GetInfo");
	gFunctionTable->acapi_moduldata_getlist                                   = (acapi_moduldata_getlistProc)                                 GetProcAddress (gACAPIDLL, "ACAPI_ModulData_GetList");
	gFunctionTable->acapi_moduldata_store                                     = (acapi_moduldata_storeProc)                                   GetProcAddress (gACAPIDLL, "ACAPI_ModulData_Store");
	gFunctionTable->acapi_notify_catchchangedefaults                          = (acapi_notify_catchchangedefaultsProc)                        GetProcAddress (gACAPIDLL, "ACAPI_Notify_CatchChangeDefaults");
	gFunctionTable->acapi_notify_catchelementreservationchange                = (acapi_notify_catchelementreservationchangeProc)              GetProcAddress (gACAPIDLL, "ACAPI_Notify_CatchElementReservationChange");
	gFunctionTable->acapi_notify_catchlockablereservationchange               = (acapi_notify_catchlockablereservationchangeProc)             GetProcAddress (gACAPIDLL, "ACAPI_Notify_CatchLockableReservationChange");
	gFunctionTable->acapi_notify_catchnewelement                              = (acapi_notify_catchnewelementProc)                            GetProcAddress (gACAPIDLL, "ACAPI_Notify_CatchNewElement");
	gFunctionTable->acapi_notify_catchprojectevent                            = (acapi_notify_catchprojecteventProc)                          GetProcAddress (gACAPIDLL, "ACAPI_Notify_CatchProjectEvent");
	gFunctionTable->acapi_notify_catchselectionchange                         = (acapi_notify_catchselectionchangeProc)                       GetProcAddress (gACAPIDLL, "ACAPI_Notify_CatchSelectionChange");
	gFunctionTable->acapi_notify_catchtoolchange                              = (acapi_notify_catchtoolchangeProc)                            GetProcAddress (gACAPIDLL, "ACAPI_Notify_CatchToolChange");
	gFunctionTable->acapi_notify_catchviewevent                               = (acapi_notify_catchvieweventProc)                             GetProcAddress (gACAPIDLL, "ACAPI_Notify_CatchViewEvent");
	gFunctionTable->acapi_notify_getparentelement                             = (acapi_notify_getparentelementProc)                           GetProcAddress (gACAPIDLL, "ACAPI_Notify_GetParentElement");
	gFunctionTable->acapi_notify_gettranparams                                = (acapi_notify_gettranparamsProc)                              GetProcAddress (gACAPIDLL, "ACAPI_Notify_GetTranParams");
	gFunctionTable->acapi_notify_installelementobserver                       = (acapi_notify_installelementobserverProc)                     GetProcAddress (gACAPIDLL, "ACAPI_Notify_InstallElementObserver");
	gFunctionTable->acapi_registermodelesswindow                              = (acapi_registermodelesswindowProc)                            GetProcAddress (gACAPIDLL, "ACAPI_RegisterModelessWindow");
	gFunctionTable->acapi_register_attributepanel                             = (acapi_register_attributepanelProc)                           GetProcAddress (gACAPIDLL, "ACAPI_Register_AttributePanel");
	gFunctionTable->acapi_register_builtinlibrary                             = (acapi_register_builtinlibraryProc)                           GetProcAddress (gACAPIDLL, "ACAPI_Register_BuiltInLibrary");
	gFunctionTable->acapi_register_esym                                       = (acapi_register_esymProc)                                     GetProcAddress (gACAPIDLL, "ACAPI_Register_ESYM");
	gFunctionTable->acapi_register_filetype                                   = (acapi_register_filetypeProc)                                 GetProcAddress (gACAPIDLL, "ACAPI_Register_FileType");
	gFunctionTable->acapi_register_infoboxpanel                               = (acapi_register_infoboxpanelProc)                             GetProcAddress (gACAPIDLL, "ACAPI_Register_InfoBoxPanel");
	gFunctionTable->acapi_register_menu                                       = (acapi_register_menuProc)                                     GetProcAddress (gACAPIDLL, "ACAPI_Register_Menu");
	gFunctionTable->acapi_register_moduldatahandler                           = (acapi_register_moduldatahandlerProc)                         GetProcAddress (gACAPIDLL, "ACAPI_Register_ModulDataHandler");
	gFunctionTable->acapi_register_propertyhandler                            = (acapi_register_propertyhandlerProc)                          GetProcAddress (gACAPIDLL, "ACAPI_Register_PropertyHandler");
	gFunctionTable->acapi_register_requiredservice                            = (acapi_register_requiredserviceProc)                          GetProcAddress (gACAPIDLL, "ACAPI_Register_RequiredService");
	gFunctionTable->acapi_register_settingspanel                              = (acapi_register_settingspanelProc)                            GetProcAddress (gACAPIDLL, "ACAPI_Register_SettingsPanel");
	gFunctionTable->acapi_register_subtype                                    = (acapi_register_subtypeProc)                                  GetProcAddress (gACAPIDLL, "ACAPI_Register_Subtype");
	gFunctionTable->acapi_register_supportedservice                           = (acapi_register_supportedserviceProc)                         GetProcAddress (gACAPIDLL, "ACAPI_Register_SupportedService");
	gFunctionTable->acapi_resource_getfixstr                                  = (acapi_resource_getfixstrProc)                                GetProcAddress (gACAPIDLL, "ACAPI_Resource_GetFixStr");
	gFunctionTable->acapi_resource_getlocstr                                  = (acapi_resource_getlocstrProc)                                GetProcAddress (gACAPIDLL, "ACAPI_Resource_GetLocStr");
	gFunctionTable->acapi_resource_getlocustr                                 = (acapi_resource_getlocustrProc)                               GetProcAddress (gACAPIDLL, "ACAPI_Resource_GetLocUStr");
	gFunctionTable->acapi_selection_get                                       = (acapi_selection_getProc)                                     GetProcAddress (gACAPIDLL, "ACAPI_Selection_Get");
	gFunctionTable->acapi_selection_setmarquee                                = (acapi_selection_setmarqueeProc)                              GetProcAddress (gACAPIDLL, "ACAPI_Selection_SetMarquee");
	gFunctionTable->acapi_setdlltermhookproc                                  = (acapi_setdlltermhookprocProc)                                GetProcAddress (gACAPIDLL, "ACAPI_SetDllTermHookProc");
	gFunctionTable->acapi_setpreferences                                      = (acapi_setpreferencesProc)                                    GetProcAddress (gACAPIDLL, "ACAPI_SetPreferences");
	gFunctionTable->acapi_setpreferences_oldversion                           = (acapi_setpreferences_oldversionProc)                         GetProcAddress (gACAPIDLL, "ACAPI_SetPreferences_OldVersion");
	gFunctionTable->acapi_teamworkcontrol_findlockableobjectset               = (acapi_teamworkcontrol_findlockableobjectsetProc)             GetProcAddress (gACAPIDLL, "ACAPI_TeamworkControl_FindLockableObjectSet");
	gFunctionTable->acapi_teamworkcontrol_gethotlinkcachemanagementowner      = (acapi_teamworkcontrol_gethotlinkcachemanagementownerProc)    GetProcAddress (gACAPIDLL, "ACAPI_TeamworkControl_GetHotlinkCacheManagementOwner");
	gFunctionTable->acapi_teamworkcontrol_getlockablestatus                   = (acapi_teamworkcontrol_getlockablestatusProc)                 GetProcAddress (gACAPIDLL, "ACAPI_TeamworkControl_GetLockableStatus");
	gFunctionTable->acapi_teamworkcontrol_hasconnection                       = (acapi_teamworkcontrol_hasconnectionProc)                     GetProcAddress (gACAPIDLL, "ACAPI_TeamworkControl_HasConnection");
	gFunctionTable->acapi_teamworkcontrol_hascreateright                      = (acapi_teamworkcontrol_hascreaterightProc)                    GetProcAddress (gACAPIDLL, "ACAPI_TeamworkControl_HasCreateRight");
	gFunctionTable->acapi_teamworkcontrol_hasdeletemodifyright                = (acapi_teamworkcontrol_hasdeletemodifyrightProc)              GetProcAddress (gACAPIDLL, "ACAPI_TeamworkControl_HasDeleteModifyRight");
	gFunctionTable->acapi_teamworkcontrol_isonline                            = (acapi_teamworkcontrol_isonlineProc)                          GetProcAddress (gACAPIDLL, "ACAPI_TeamworkControl_IsOnline");
	gFunctionTable->acapi_teamworkcontrol_isserverlibpart                     = (acapi_teamworkcontrol_isserverlibpartProc)                   GetProcAddress (gACAPIDLL, "ACAPI_TeamworkControl_IsServerLibPart");
	gFunctionTable->acapi_teamworkcontrol_releaseelements                     = (acapi_teamworkcontrol_releaseelementsProc)                   GetProcAddress (gACAPIDLL, "ACAPI_TeamworkControl_ReleaseElements");
	gFunctionTable->acapi_teamworkcontrol_releasehotlinkcachemanagement       = (acapi_teamworkcontrol_releasehotlinkcachemanagementProc)     GetProcAddress (gACAPIDLL, "ACAPI_TeamworkControl_ReleaseHotlinkCacheManagement");
	gFunctionTable->acapi_teamworkcontrol_releaselockable                     = (acapi_teamworkcontrol_releaselockableProc)                   GetProcAddress (gACAPIDLL, "ACAPI_TeamworkControl_ReleaseLockable");
	gFunctionTable->acapi_teamworkcontrol_reserveelements                     = (acapi_teamworkcontrol_reserveelementsProc)                   GetProcAddress (gACAPIDLL, "ACAPI_TeamworkControl_ReserveElements");
	gFunctionTable->acapi_teamworkcontrol_reservehotlinkcachemanagement       = (acapi_teamworkcontrol_reservehotlinkcachemanagementProc)     GetProcAddress (gACAPIDLL, "ACAPI_TeamworkControl_ReserveHotlinkCacheManagement");
	gFunctionTable->acapi_teamworkcontrol_reservelockable                     = (acapi_teamworkcontrol_reservelockableProc)                   GetProcAddress (gACAPIDLL, "ACAPI_TeamworkControl_ReserveLockable");
	gFunctionTable->acapi_unregistermodelesswindow                            = (acapi_unregistermodelesswindowProc)                          GetProcAddress (gACAPIDLL, "ACAPI_UnregisterModelessWindow");
	gFunctionTable->acapi_writereport                                         = (acapi_writereportProc)                                       GetProcAddress (gACAPIDLL, "ACAPI_WriteReport");
	gFunctionTable->closeownresfile                                           = (closeownresfileProc)                                         GetProcAddress (gACAPIDLL, "CloseOwnResFile");
	gFunctionTable->openownresfile                                            = (openownresfileProc)                                          GetProcAddress (gACAPIDLL, "OpenOwnResFile");

	// -----------------------
	// resource initializing
#ifdef WINDOWS
	if (gOwnResModule != 0) {
		gFunctionTable->openownresfile (gOwnResModule);
	}
#endif

	return NoError;
}

// ---------------------------------------------------------------
// Called every time when the addon is loaded into the memory
// to execute a service
// ---------------------------------------------------------------
static GSErrCode __ACDLL_CALL	My_Initialize (void* clientId)
{
	gOwnClientID = clientId;
	GSErrCode err = Initialize ();

	return err;
}		/* My_Initialize */


// ---------------------------------------------------------------
// Called every time before the addon is unloaded from the memory
// ---------------------------------------------------------------
static GSErrCode __ACDLL_CALL	My_FreeData (void)
{
	GSErrCode err = FreeData ();

	return err;
}		/* My_FreeData */

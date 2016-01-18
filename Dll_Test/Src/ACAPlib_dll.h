// *****************************************************************************
// Common support for ACAP_DLL.apx based addons
// API Development Kit 19
//
// Namespaces:		Contact person:
//	-None-
//
// [SG compatible] - Yes
// *****************************************************************************

#define USE_API_DLL

#include "PlatformDLLExport.hpp"

#include "APIEnvir.h"
#include "ACAPinc_dll.h"

#ifdef DLL_TEST_EXPORTS
	#define ACAP_DLL_CLIENT PLATFORM_DLL_EXPORT
#else
	#define ACAP_DLL_CLIENT	PLATFORM_DLL_IMPORT
#endif

#if defined (WINDOWS)
	#if !defined (__ACENV_CALL)
		#define	__ACENV_CALL	__cdecl
	#endif
#endif

#if defined (__cplusplus)
extern "C" {
#endif

// -----------------------------------------------------------------------------
// Addon interface
// -----------------------------------------------------------------------------
typedef API_AddonType	__ACENV_CALL	CheckEnvironmentProc (API_EnvirParams* envirParams);
typedef GSErrCode		__ACENV_CALL	RegisterInterfaceProc (void);
typedef GSErrCode		__ACENV_CALL	InitializeProc (void* clientID);
typedef GSErrCode		__ACENV_CALL	FreeDataProc (void);

// -----------------------------------------------------------------------------
// Dll interface
// -----------------------------------------------------------------------------
ACAP_DLL_CLIENT void		__ACENV_CALL	GetExportedFuncAddrs (GSPtr expList);
ACAP_DLL_CLIENT GSErrCode	__ACENV_CALL	SetImportedFuncAddrs (GSPtr impList);
ACAP_DLL_CLIENT GSErrCode	__ACENV_CALL	SetACAPDll (GSResModule ACAPIHandle);


#if defined (__cplusplus)
}	/* extern "C"	*/
#endif

// --- Predeclarations -------------------------------------------------------------------------------------------

namespace ModelerAPI {
	class Model;
	class Camera;
}



typedef		void	(__ACENV_CALL	*openownresfileProc) (GSResModule ownResModule);
typedef		void	(__ACENV_CALL	*closeownresfileProc) (GSResModule ownResModule);


typedef		void	(__ACENV_CALL	*acapi_getreleasenumberProc) (ClientID clientPtr, API_ServerApplicationInfo	*serverApplicationInfo);

/* -- System ------------------------------ */
#if defined (WINDOWS)
typedef		HINSTANCE	(__ACENV_CALL	*acapi_getextensioninstanceProc) (void);

typedef		HWND	(__ACENV_CALL	*acapi_getmainwindowProc) (ClientID clientPtr);
#endif
typedef		GSResModule	(__ACENV_CALL	*acapi_getownresmoduleProc) (void);

typedef		GSErrCode	(__ACENV_CALL	*acapi_getownlocationProc) (IO::Location *ownFileLoc);

typedef		void	(__ACENV_CALL	*acapi_setdlltermhookprocProc) (APIDllTermHookProc* dllTermHookProc);

/* -- Registration ------------------ */

	// CheckEnvironment
typedef		GSErrCode	(__ACENV_CALL	*acapi_register_supportedserviceProc) (GSType cmdID, Int32 cmdVersion);

typedef		GSErrCode	(__ACENV_CALL	*acapi_register_requiredserviceProc) (const API_ModulID	*modulID,
																		GSType				cmdID,
																		Int32				cmdVersion);

	// RegisterInterface
typedef		GSErrCode	(__ACENV_CALL	*acapi_register_menuProc)		(short			menuStrResID,
																 short			promptStrResID,
																 APIMenuCodeID	menuPosCode,
																 GSFlags		menuFlags);

typedef		GSErrCode	(__ACENV_CALL	*acapi_register_filetypeProc) (Int32			refCon,
																 GSType			ftype,
																 GSType			fcreator,
																 const char		*extname,
																 short			iconResID,
																 short			descStrResID,
																 short			descStrResItemID,
																 API_IOMethod	methodFlags);

typedef		GSErrCode	(__ACENV_CALL	*acapi_register_subtypeProc)	(GSType				signature,				// identifier
																 short				templateFileResID,		// subtype template (FILE resource ID)
																 bool				transparentNode,		// allow ancestor tools to place object instances
																 short				stringResID,			// tool descriptions (STR# resource ID)
																 short				smallIconResID,			// toolbar icon
																 short				largeIconResID,			// toolbox icon
																 API_TBoxGroup		toolGroupId,			// toolbar group
																 API_TBoxEnabled	enableFlags,			// toolbar enable flags
																 short				defaultsResID);			// factory defaults (ACP0 resource ID)

typedef		GSErrCode	(__ACENV_CALL	*acapi_register_settingspanelProc)	(Int32					refCon,
																		 API_ElemTypeID			tool,
																		 API_ElemVariationID	variation,
																		 short					iconId,
																		 short					stringId,
																		 short					pageId,
																		 short					growType,	// means NoGrow
																		 bool					visibleByDefault);

typedef		GSErrCode	(__ACENV_CALL	*acapi_register_attributepanelProc)	(Int32					refCon,
																		 API_AttrTypeID			attr,
																		 short					iconId,
																		 short					stringId,
																		 short					pageId,
																		 short					growType);	// means NoGrow

typedef		GSErrCode	(__ACENV_CALL	*acapi_register_infoboxpanelProc)		(Int32					refCon,
																		 API_ElemTypeID			tool,
																		 API_ElemVariationID	variation,
																		 short					stringId,
																		 short					pageId,
																		 bool					visibleByDefault);

typedef		GSErrCode	(__ACENV_CALL	*acapi_register_esymProc)		(GSType			signature,				// LibPart owner ID
																 API_LibTypeID	typeID,					// LibPart base type (Window, Door, Object, Lamp)
																 short			stringResID);			// description

typedef		GSErrCode	(__ACENV_CALL	*acapi_register_propertyhandlerProc) (short assignStrResID);

typedef		GSErrCode	(__ACENV_CALL	*acapi_register_builtinlibraryProc) (void);

typedef		GSErrCode	(__ACENV_CALL	*acapi_register_moduldatahandlerProc) (void);


	// Initialize
typedef		GSErrCode	(__ACENV_CALL	*acapi_install_modulcommandhandlerProc)  (ClientID				clientPtr,
																			GSType					cmdID,
																			Int32					cmdVersion,
																			APIModulCommandProc*	handlerProc);

typedef		GSErrCode	(__ACENV_CALL	*acapi_install_menuhandlerProc)		   (ClientID				clientPtr,
																			short					menuStrResID,
																			APIMenuCommandProc*		handlerProc);

typedef		GSErrCode	(__ACENV_CALL	*acapi_install_filetypehandlerProc)	   (ClientID				clientPtr,
																			GSType					cmdID,
																			APIIOCommandProc*		handlerProc);

typedef		GSErrCode	(__ACENV_CALL	*acapi_install_filetypehandler3dProc)	   (ClientID				clientPtr,
																			GSType					cmdID,
																			APIIO3DCommandProc*		handlerProc);

typedef		GSErrCode	(__ACENV_CALL	*acapi_install_panelhandlerProc) (ClientID				clientPtr,
																	Int32					refCon,
																	APIPanelCreateProc*		handlerCreateProc,
																	APIPanelDestroyProc*	handlerDestroyProc);

typedef		GSErrCode	(__ACENV_CALL	*acapi_install_esymhandlerProc)		   (ClientID				clientPtr,
																			GSType					signature,
																			APIESYMCommandProc*		handlerProc);

typedef		GSErrCode	(__ACENV_CALL	*acapi_install_propertyhandlerProc)		(ClientID				clientPtr,
																			 short					strResID,
																			 APIPropertyCommandProc* propertyCommandProc);

typedef		GSErrCode	(__ACENV_CALL	*acapi_install_moduldatamergehandlerProc)	(ClientID						clientPtr,
																			 APIModulDataMergeHandlerProc*	mergeHandlerProc);

typedef		GSErrCode	(__ACENV_CALL	*acapi_install_moduldatasaveoldformathandlerProc) (ClientID								clientPtr,
																					 APIModulDataSaveOldFormatHandlerProc*	modulDataSaveOldFormatProc);


/* -- Memory Manager ---------------------- */
/**
 * @defgroup MemMgr Memory Manager
 */
/*@{*/

typedef		void	(__ACENV_CALL	*acapi_disposeaddparhdlProc) (API_AddParType ***addPars);

typedef		void	(__ACENV_CALL	*acapi_disposeparagraphshdlProc) (API_ParagraphType ***paragraphs);

typedef		void	(__ACENV_CALL	*acapi_disposeelemmemohdlsProc) (API_ElementMemo *memo);

typedef		void	(__ACENV_CALL	*acapi_disposeattrdefshdlsProc) (API_AttributeDef *defs);

typedef		void	(__ACENV_CALL	*acapi_disposeattrdefshdlsextProc) (API_AttributeDefExt *defs);

typedef		void	(__ACENV_CALL	*acapi_disposeroomrelationhdlsProc) (API_RoomRelation *roomInfo);

typedef		void	(__ACENV_CALL	*acapi_disposewallrelationhdlsProc) (API_WallRelation *wallInfo);

typedef		void	(__ACENV_CALL	*acapi_disposebeamrelationhdlsProc) (API_BeamRelation *beamInfo);

typedef		void	(__ACENV_CALL	*acapi_disposeworkspaceinfohdlsProc) (API_WorkspaceInfo *workspaceInfo);

typedef		void	(__ACENV_CALL	*acapi_freegridmeshptrProc) (API_GridMesh **gridMeshes);

typedef		void	(__ACENV_CALL	*acapi_freecwcontourptrProc) (API_CWContourType **contours);

typedef		void	(__ACENV_CALL	*acapi_freegdlmodelviewoptionsptrProc) (API_GDLModelViewOptions **gdlModelViewOptions);

/*@}*/

/* -- Resource Manager -------------------- */
/**
 * @defgroup ResMgr Resource Manager
 */
/*@{*/

typedef		bool	(__ACENV_CALL	*acapi_resource_getlocstrProc) (char *str, short resID, short index, GSResModule resModule);

typedef		bool	(__ACENV_CALL	*acapi_resource_getlocustrProc) (GS::uchar_t *str, short resID, short index, GSResModule resModule);

typedef		bool	(__ACENV_CALL	*acapi_resource_getfixstrProc) (char *str, short resID, short index, GSResModule resModule);

/*@}*/

/* -- Client Manager ---------------------- */
typedef		void	(__ACENV_CALL	*acapi_keepinmemoryProc) (ClientID clientPtr, bool keepIn);

typedef		GSErrCode	(__ACENV_CALL	*acapi_setpreferencesProc) (ClientID clientPtr, Int32 version, GSSize nByte, const void *data);

typedef		GSErrCode	(__ACENV_CALL	*acapi_getpreferencesProc) (ClientID clientPtr, Int32 *version, GSSize *nByte, void *data);

typedef		GSErrCode	(__ACENV_CALL	*acapi_getpreferences_platformProc) (Int32			*version,
																	   GSSize			*nByte,
																	   void				*data,
																	   unsigned short	*platformSign);

typedef		GSErrCode	(__ACENV_CALL	*acapi_setpreferences_oldversionProc) (ClientID			clientPtr,
																		 Int32				version,
																		 GSSize				nByte,
																		 const void*		data,
																		 unsigned short		platformSign,
																		 API_FTypeID		oldPlanFileID);

typedef		GSErrCode	(__ACENV_CALL	*acapi_registermodelesswindowProc) (ClientID						clientPtr,
																	  Int32							referenceID,
																	  APIPaletteControlCallBackProc	*callBackProc,
																	  GSFlags						controlFlags,
																	  const API_Guid&				paletteGuid);

typedef		GSErrCode	(__ACENV_CALL	*acapi_unregistermodelesswindowProc) (ClientID clientPtr, Int32 referenceID);

typedef		GSErrCode	(__ACENV_CALL	*acapi_callundoablecommandProc) (ClientID clientPtr, const GS::UniString& undoString, const std::function<GSErrCode ()>& command);

typedef		void	(__ACENV_CALL	*acapi_writereportProc) (ClientID clientPtr, const char *msg, bool withDial);


/* -- General Manager --------------------- */
/**
 * @defgroup GenFunc General Functions
 */
/*@{*/

typedef		GSErrCode	(__ACENV_CALL	*acapi_databaseProc)		(ClientID clientPtr, API_DatabaseID code, void* par1, void* par2, void* par3);

typedef		GSErrCode	(__ACENV_CALL	*acapi_environmentProc)	(ClientID clientPtr, API_EnvironmentID code, void* par1, void* par2, void* par3);

typedef		GSErrCode	(__ACENV_CALL	*acapi_interfaceProc)		(ClientID clientPtr, API_InterfaceID code, void* par1, void* par2, void* par3, void* par4);

typedef		GSErrCode	(__ACENV_CALL	*acapi_goodiesProc)		(ClientID clientPtr, API_GoodiesID code, void* par1, void* par2, void* par3);

typedef		GSErrCode	(__ACENV_CALL	*acapi_automateProc)		(ClientID clientPtr, API_AutomateID code, void* par1, void* par2, void* par3);

/*@}*/

/* -- Attribute Manager ------------------- */

/**
 * @defgroup AttrMan Attribute Manager
 */
/*@{*/

typedef		GSErrCode	(__ACENV_CALL	*acapi_attribute_getnumProc) (ClientID clientPtr, API_AttrTypeID typeID, short *count);

typedef		GSErrCode	(__ACENV_CALL	*acapi_attribute_getProc) (ClientID clientPtr, API_Attribute *attribute);

typedef		GSErrCode	(__ACENV_CALL	*acapi_attribute_getdefProc) (ClientID clientPtr, API_AttrTypeID typeID, short index, API_AttributeDef *defs);

typedef		GSErrCode	(__ACENV_CALL	*acapi_attribute_getdefextProc) (ClientID				clientPtr,
																   API_AttrTypeID		typeID,
																   short				index,
																   API_AttributeDefExt	*defs);

typedef		GSErrCode	(__ACENV_CALL	*acapi_attribute_createProc) (ClientID clientPtr, API_Attribute *attribute, API_AttributeDef *defs);

typedef		GSErrCode	(__ACENV_CALL	*acapi_attribute_createextProc) (ClientID clientPtr, API_Attribute *attribute, API_AttributeDefExt *defs);

typedef		GSErrCode	(__ACENV_CALL	*acapi_attribute_modifyProc) (ClientID clientPtr, API_Attribute *attribute, API_AttributeDef *defs);

typedef		GSErrCode	(__ACENV_CALL	*acapi_attribute_modifyextProc) (ClientID clientPtr, API_Attribute *attribute, API_AttributeDefExt *defs);

typedef		GSErrCode	(__ACENV_CALL	*acapi_attribute_searchProc) (ClientID clientPtr, API_Attr_Head *attrHead);

typedef		GSErrCode	(__ACENV_CALL	*acapi_attribute_deleteProc) (ClientID clientPtr, const API_Attr_Head *attrHead);

typedef		GSErrCode	(__ACENV_CALL	*acapi_attribute_deletemoreProc) (ClientID clientPtr, const API_Attr_Head *attrHead, short* attrIndexListPtr);

typedef		GSErrCode	(__ACENV_CALL	*acapi_attribute_getuserdataProc) (ClientID				clientPtr,
																	 API_Attr_Head			*attrHead,
																	 API_AttributeUserData	*userData);

typedef		GSErrCode	(__ACENV_CALL	*acapi_attribute_setuserdataProc) (ClientID						clientPtr,
																	 API_Attr_Head					*attrHead,
																	 const API_AttributeUserData	*userData);

/*@}*/

/* -- Library Part Manager ---------------- */

/**
 * \defgroup LPMan Library Part Manager
 */
/*@{*/

typedef		GSErrCode	(__ACENV_CALL	*acapi_libpart_getnumProc) (ClientID clientPtr, Int32 *count);

typedef		GSErrCode	(__ACENV_CALL	*acapi_libpart_searchProc) (ClientID clientPtr, API_LibPart *ancestor, bool createIfMissing, bool onlyPlaceable);

typedef		GSErrCode	(__ACENV_CALL	*acapi_libpart_patternsearchProc) (ClientID clientPtr, const API_LibPart *ancestor, const GS::UniString& pattern, API_LibPart result[50], Int32* numFound);

typedef		GSErrCode	(__ACENV_CALL	*acapi_libpart_getProc) (ClientID clientPtr, API_LibPart *libPart);

typedef		GSErrCode	(__ACENV_CALL	*acapi_libpart_getparamsProc) (ClientID		clientPtr,
																 Int32			libInd,
																 double			*a,
																 double			*b,
																 Int32			*addParNum,
																 API_AddParType	***addPars);

typedef		GSErrCode	(__ACENV_CALL	*acapi_libpart_getsectionProc) (ClientID				clientPtr,
																  Int32					libInd,
																  API_LibPartSection	*section,
																  GSHandle				*sectionHdl,
																  GS::UniString			*sectionStr,
																  GS::UniString			*password);

typedef		GSErrCode	(__ACENV_CALL	*acapi_libpart_updatesectionProc) (ClientID					clientPtr,
																	 Int32						libInd,
																	 const API_LibPartSection	*section,
																	 GSHandle					sectionHdl,
																	 GS::UniString				*sectionStr,
																	 GS::UniString				*password);

typedef		GSErrCode	(__ACENV_CALL	*acapi_libpart_setupsect_2ddrawhdlProc) (ClientID clientPtr);

typedef		GSErrCode	(__ACENV_CALL	*acapi_libpart_getsect_2ddrawhdlProc) (ClientID clientPtr, GSHandle *sectionHdl);

typedef		GSErrCode	(__ACENV_CALL	*acapi_libpart_getsect_paramdefProc) (ClientID			clientPtr,
																		const API_LibPart	*libPart,
																		API_AddParType		**addPars,
																		double				*a,
																		double				*b,
																		GSHandle			sect2DDrawHdl,
																		GSHandle			*sectionHdl,
																		GS::UniString		*password);

typedef		GSErrCode	(__ACENV_CALL	*acapi_libpart_createProc) (ClientID clientPtr, const API_LibPart *libPart);

typedef		GSErrCode	(__ACENV_CALL	*acapi_libpart_addsectionProc) (ClientID					clientPtr,
																  const API_LibPartSection	*section,
																  GSHandle					sectionHdl,
																  GS::UniString				*sectionStr);

typedef		GSErrCode	(__ACENV_CALL	*acapi_libpart_newsectionProc) (ClientID clientPtr, const API_LibPartSection *section);

typedef		GSErrCode	(__ACENV_CALL	*acapi_libpart_writesectionProc) (ClientID clientPtr, Int32 nByte, GSPtr data);

typedef		GSErrCode	(__ACENV_CALL	*acapi_libpart_endsectionProc) (ClientID clientPtr);

typedef		GSErrCode	(__ACENV_CALL	*acapi_libpart_saveProc) (ClientID clientPtr, API_LibPart *libPart);

typedef		GSErrCode	(__ACENV_CALL	*acapi_libpart_shapeprimsProc) (ClientID			clientPtr,
																  Int32				libInd,
																  const API_Guid&	instanceElemGuid,
																  short				gdlContext,
																  ShapePrimsProc	*shapePrimsProc);

typedef		GSErrCode	(__ACENV_CALL	*acapi_libpart_gethotspotsProc) (ClientID				clientPtr,
																   Int32				libInd,
																   const API_Guid&		instanceElemGuid,
																   Int32*				nHotspots,
																   API_PrimHotspot***	hotspots);

typedef		GSErrCode	(__ACENV_CALL	*acapi_libpart_registerProc) (ClientID clientPtr, API_LibPart *libPart);

typedef		GSErrCode	(__ACENV_CALL	*acapi_libpart_registerallProc) (ClientID clientPtr, GS::Array<API_LibPart>* libParts);

typedef		GSErrCode	(__ACENV_CALL	*acapi_libpart_getsectionlistProc) (ClientID				clientPtr,
																	  Int32					libInd,
																	  Int32					*nSection,
																	  API_LibPartSection	***sections);

typedef		GSErrCode	(__ACENV_CALL	*acapi_libpart_getdetailsProc) (ClientID				clientPtr,
																  Int32					libInd,
																  API_LibPartDetails	*details);

typedef		GSErrCode	(__ACENV_CALL	*acapi_libpart_setdetails_paramdefProc) (ClientID				clientPtr,
																		   const API_LibPart		*libPart,
																		   GSHandle					paramHdl,
																		   const API_LibPartDetails	*details);

/*@}*/

/* -- Element Manager --------------------- */

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_getdefaultsProc) (ClientID clientPtr, API_Element *element, API_ElementMemo *memo);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_getdefaultsextProc) (ClientID				clientPtr,
																	  API_Element			*element,
																	  API_ElementMemo		*memo,
																	  UInt32				nSubElems,
																	  API_SubElemMemoMask	*subElems);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_changedefaultsProc) (ClientID			clientPtr,
																	  API_Element		*element,
																	  API_ElementMemo	*memo,
																	  const API_Element	*mask);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_changedefaultsextProc) (ClientID				clientPtr,
																		 API_Element			*element,
																		 API_ElementMemo		*memo,
																		 const API_Element		*mask,
																		 UInt32					nSubElems,
																		 API_SubElemMemoMask	*subElems);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_getelemlistProc) (ClientID				clientPtr,
																   API_ElemTypeID		typeID,
																   GS::Array<API_Guid>*	elemList,
																   API_ElemFilterFlags	filterBits,
																   API_ElemVariationID	variationID,
																   const API_Guid&		renovationFilterGuid);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_getconnectedelementsProc) (ClientID				clientPtr,
																			const API_Guid&			guid,
																			API_ElemTypeID			connectedElemTypeID,
																			GS::Array<API_Guid>*	connectedElements,
																			API_ElemFilterFlags		filterBits,
																			API_ElemVariationID		variationID,
																			const API_Guid&		renovationFilterGuid);

typedef		bool	(__ACENV_CALL	*acapi_element_filterProc) (ClientID				clientPtr,
															  const API_Guid&		guid,
															  API_ElemFilterFlags	filterBits,
															  API_ElemVariationID	variationID,
															  const API_Guid&		renovationFilterGuid);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_getProc) (ClientID		clientPtr,
														   API_Element	*element,
														   UInt32		mask);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_getheaderProc) (ClientID		clientPtr,
														  		 API_Elem_Head	*elementHead,
																 UInt32			mask);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_getmemoProc)	(ClientID					clientPtr,
																 const API_Guid&			guid,
																 API_ElementMemo*			memo,
																 UInt64						mask);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_changememoProc) (ClientID				clientPtr,
																  API_Guid&				guid,
																  UInt64				mask,
											   					  const API_ElementMemo	*memo);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_getrelationsProc) (ClientID			clientPtr,
																	const API_Guid&		guid,
																	API_ElemTypeID		otherID,
																	void				*relData);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_getsytranProc) (ClientID clientPtr, const API_Elem_Head *elemHead, API_SyTran *syTran);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_gethotspotsProc) (ClientID							clientPtr,
																   const API_Guid&					guid,
																   GS::Array<API_ElementHotspot>*	hotspotArray);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_createProc) (ClientID clientPtr, API_Element *element, API_ElementMemo *memo);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_createextProc) (ClientID				clientPtr,
																 API_Element			*element,
																 API_ElementMemo		*memo,
																 UInt32					nSubElems,
																 API_SubElemMemoMask	*subElems);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_changeparametersProc) (ClientID				clientPtr,
																		API_Elem_Head			**elemHead,
																		Int32					nItem,
							   											const API_Element		*defPars,
																		const API_ElementMemo	*defMemo,
												   						const API_Element		*mask);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_changeProc) (ClientID				clientPtr,
															  API_Element			*element,
															  const API_Element		*mask,
															  const API_ElementMemo	*memo,
															  UInt64				memoMask,
															  bool					withdel);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_changemoreProc) (ClientID				clientPtr,
																  API_Elem_Head			*elemHead,
																  Int32					nItem,
							   									  const API_Element		*defPars,
																  const API_ElementMemo	*defMemo,
												   				  const API_Element		*mask,
																  UInt64				memoMask,
																  bool					withdel);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_changeextProc) (ClientID				clientPtr,
															  	 API_Element			*element,
																 const API_Element		*mask,
																 API_ElementMemo		*memo,
																 UInt64					memoMask,
																 UInt32					nSubElems,
																 API_SubElemMemoMask	*subElems,
																 bool					withdel,
																 Int32					subIndex);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_deleteProc) (ClientID clientPtr, API_Elem_Head **elemHead, Int32 nItem);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_editProc) (ClientID clientPtr, API_Neig **items, Int32 nItem, const API_EditPars *pars);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_toolProc) (ClientID		clientPtr,
															API_Elem_Head	**elemHead,
															Int32			nItem,
															API_ToolCmdID	typeID,
															void			*pars);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_selectProc) (ClientID clientPtr, API_Neig **selNeig, Int32 nItem, bool add);


typedef		GSErrCode	(__ACENV_CALL	*acapi_element_getuserdataProc)	(ClientID					clientPtr,
																	 API_Elem_Head				*elemHead,
																	 API_ElementUserData		*userData,
																	 UInt32						mask);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_setuserdataProc)	(ClientID					clientPtr,
																	 API_Elem_Head				*elemHead,
																	 const API_ElementUserData	*userData);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_deleteuserdataProc) (ClientID clientPtr, API_Elem_Head *elemHead);

typedef		API_Guid	(__ACENV_CALL	*acapi_element_unidtoguidProc) (ClientID clientPtr, API_DatabaseUnId databaseUnId, UInt32 elemUnId);

typedef		UInt32	(__ACENV_CALL	*acapi_element_guidtounidProc) (ClientID clientPtr, API_Guid elemGuid, API_DatabaseUnId* databaseUnId);

typedef		API_Guid	(__ACENV_CALL	*acapi_element_linkidtodrwguidProc) (ClientID clientPtr, Int32 linkId);

typedef		Int32	(__ACENV_CALL	*acapi_element_drwguidtolinkidProc) (ClientID clientPtr, API_Guid drwGuid);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_linkProc) (ClientID	clientPtr,
															API_Guid	guid_linkFrom,
															API_Guid	guid_linkTo,
															GSFlags		linkFlags);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_unlinkProc) (ClientID clientPtr, API_Guid guid_linkFrom, API_Guid guid_linkTo);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_getlinksProc) (ClientID	clientPtr,
																API_Guid	guid_linkFrom,
																API_Guid	***guid_linkTo,
																Int32 		*nLinks);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_getlinkflagsProc) (ClientID	clientPtr,
																	API_Guid	guid_linkFrom,
																	API_Guid	guid_linkTo,
																	GSFlags		*linkFlags);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_attachobserverProc) (ClientID clientPtr, API_Elem_Head *elemHead, GSFlags notifyFlags);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_detachobserverProc) (ClientID clientPtr, API_Elem_Head *elemHead);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_getobservedelementsProc) (ClientID clientPtr, API_Elem_Head ***elemHead, Int32 *nElems);


typedef		GSErrCode	(__ACENV_CALL	*acapi_element_solidlink_createProc) (ClientID				clientPtr,
																		API_Guid				guid_Target,
																		API_Guid				guid_Operator,
																		API_SolidOperationID	operation,
																		GSFlags					linkFlags);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_solidlink_removeProc) (ClientID	clientPtr,
																		API_Guid	guid_Target,
																		API_Guid	guid_Operator);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_solidlink_gettimeProc) (ClientID		clientPtr,
																		 API_Guid		guid_Target,
																		 API_Guid		guid_Operator,
																		 GSTime			*linkTime,
																		 UInt32			*linkSubTime);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_solidlink_getoperationProc) (ClientID				clientPtr,
																			  API_Guid				guid_Target,
																			  API_Guid				guid_Operator,
																			  API_SolidOperationID	*operation);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_solidlink_getflagsProc) (ClientID	clientPtr,
																		  API_Guid	guid_Target,
																		  API_Guid	guid_Operator,
																		  GSFlags	*linkFlags);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_solidlink_gettargetsProc) (ClientID	clientPtr,
																			API_Guid	guid_Operator,
																			API_Guid	***guid_Targets,
																			Int32		*nLinks);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_solidlink_getoperatorsProc) (ClientID	clientPtr,
																			  API_Guid	guid_Target,
																			  API_Guid	***guid_Operators,
																			  Int32		*nLinks);


typedef		GSErrCode	(__ACENV_CALL	*acapi_element_get3dinfoProc)			(ClientID					clientPtr,
																		 const API_Elem_Head&		elemHead,
																		 API_ElemInfo3D				*info3D);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_getquantitiesProc)		(ClientID							clientPtr,
																		 API_Guid							elemGuid,
								  										 const API_QuantityPar				*params,
																		 API_Quantities						*quantities,
																		 const API_QuantitiesMask			*mask);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_getmorequantitiesProc)	(ClientID							clientPtr,
																		 const GS::Array<API_Guid>			*elemGuids,
								  										 const API_QuantityPar				*params,
																		 GS::Array<API_Quantities>			*quantities,
																		 const API_QuantitiesMask			*mask);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_getpropertiesProc)		(ClientID					clientPtr,
																		 const API_Elem_Head*		elemHead,
																		 API_PropertyRefType		***propRefs,
																		 Int32						*nProp);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_getcomponentsProc)		(ClientID					clientPtr,
																		 const API_Elem_Head*		elemHead,
																		 API_ComponentRefType		***compRefs,
																		 Int32						*nComp);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_getdescriptorsProc)	(ClientID					clientPtr,
																		 const API_Elem_Head*		elemHead,
																		 API_DescriptorRefType		***descRef,
																		 Int32						*nDesc);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_shapeprimsProc)		(ClientID				clientPtr,
																		 const API_Elem_Head&	elemHead,
																		 ShapePrimsProc			*shapePrimsProc);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_shapeprimsextProc) (ClientID				clientPtr,
																	 const API_Elem_Head&	elemHead,
																	 ShapePrimsProc			*shapePrimsProc,
																	 API_ShapePrimsParams*	shapePrimsParams);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_getlinkedpropertiesProc) (ClientID				clientPtr,
																		   const API_Elem_Head	*head,
																		   bool					*criteria,
																		   Int32				*inviduallyLibInd,
																		   Int32				**critLibInds,
																		   Int32				*nCrits);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_setlinkedpropertiesProc) (ClientID				clientPtr,
																		   API_Elem_Head		*head,
																		   bool					criteria,
																		   Int32				inviduallyLibInd);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_getcategoryvalueProc) (ClientID					clientPtr,
																		const API_Guid&				elemGuid,
																		const API_ElemCategory&		elemCategory,
																		API_ElemCategoryValue*		elemCategoryValue);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_getcategoryvaluedefaultProc) (ClientID						clientPtr,
																			   const API_ElemTypeID&		typeID,
																			   const API_ElemVariationID&	variationID,
																			   const API_ElemCategory&		elemCategory,
																			   API_ElemCategoryValue*		elemCategoryValue);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_setcategoryvalueProc) (ClientID						clientPtr,
																		const API_Guid&					elemGuid,
																		const API_ElemCategory&			elemCategory,
																		const API_ElemCategoryValue&	elemCategoryValue);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_setcategoryvaluedefaultProc) (ClientID							clientPtr,
																			   const API_ElemTypeID&			typeID,
																			   const API_ElemVariationID&		variationID,
																			   const API_ElemCategory&			elemCategory,
																			   const API_ElemCategoryValue&		elemCategoryValue);

/* -- Element Composition ----------------- */

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_trim_elementsProc)			(ClientID					clientPtr,
																			 const GS::Array<API_Guid>&	guid_ElementsToTrim);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_trim_elementswithProc)		(ClientID					clientPtr,
																			 const GS::Array<API_Guid>&	guid_ElementsToTrim,
																			 const API_Guid&			guid_Element,
																			 API_TrimTypeID				trimType);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_trim_removeProc)			(ClientID					clientPtr,
																			 const API_Guid&			guid_Element1,
																			 const API_Guid&			guid_Element2);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_trim_gettrimtypeProc)		(ClientID					clientPtr,
																			 const API_Guid&			guid_Element1,
																			 const API_Guid&			guid_Element2,
																			 API_TrimTypeID*			trimType);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_trim_gettrimmedelementsProc)	(ClientID				clientPtr,
																				 const API_Guid&		guid_Element,
																				 GS::Array<API_Guid>*	guid_TrimmedElements);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_trim_gettrimmingelementsProc)	(ClientID				clientPtr,
																				 const API_Guid&		guid_Element,
																				 GS::Array<API_Guid>*	guid_TrimmingElements);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_merge_elementsProc)			(ClientID					clientPtr,
																				 const GS::Array<API_Guid>&	guid_ElementsToMerge);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_merge_getmergedelementsProc)	(ClientID				clientPtr,
																				 const API_Guid&		guid_Element,
																				 GS::Array<API_Guid>*	guid_MergedElements);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_merge_removeProc)				(ClientID			clientPtr,
																				 const API_Guid&	guid_Element1,
																				 const API_Guid&	guid_Element2);

/* -- IFC --------------------------------- */

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_getifcidentifierProc)		(const API_Guid&				elementID,
																			 API_Guid&						archicadIFCID,
																			 API_Guid&						externalIFCID);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_getelemlistbyifcidentifierProc)	(const API_Guid*				archicadIFCID,
																					 const API_Guid*				externalIFCID,
																					 GS::Array<API_Guid>&			elements);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_getifcpropertiesProc)		(ClientID						clientPtr,
																			 const API_Guid&				guid_Element,
																			 GS::Array<API_IFCProperty>*	properties);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_setifcpropertyProc)		(ClientID						clientPtr,
																			 const API_Guid&				guid_Element,
																			 const API_IFCProperty&			property);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_removeifcpropertyProc)		(ClientID						clientPtr,
																			 const API_Guid&				guid_Element,
																			 const API_IFCProperty&			property);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_getifcattributesProc)		(ClientID						clientPtr,
																			 const API_Guid&				guid_Element,
																			 GS::Array<API_IFCAttribute>*	attributes);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_setifcattributeProc)		(ClientID						clientPtr,
																			 const API_Guid&				guid_Element,
																			 const API_IFCAttribute&		attribute);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_getifcclassificationreferencesProc)	(ClientID									clientPtr,
																						 const API_Guid&							guid_Element,
																						 GS::Array<API_IFCClassificationReference>*	classificationReferences);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_setifcclassificationreferenceProc)		(ClientID								clientPtr,
																						 const API_Guid&						guid_Element,
																						 const API_IFCClassificationReference&	classificationReference);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_removeifcclassificationreferenceProc)	(ClientID								clientPtr,
																						 const API_Guid&						guid_Element,
																						 const API_IFCClassificationReference&	classificationReference);

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_getifcpropertyvalueprimitivetypeProc)	(ClientID								clientPtr,
																						 const GS::UniString&					valueType,
																						 API_IFCPropertyValuePrimitiveType*		primitiveType);


/* -- Body Editing ------------------------ */

typedef		GSErrCode	(__ACENV_CALL	*acapi_body_createProc)			(ClientID							clientPtr,
																	 const Modeler::Body*				body,
																	 const API_MaterialOverrideType*	bodyMaterialMapTable,
																	 void**								bodyData);

typedef		GSErrCode	(__ACENV_CALL	*acapi_body_finishProc)			(ClientID						clientPtr,
																	 void*							bodyData,
																	 Modeler::Body**				body,
																	 API_MaterialOverrideType**		bodyMaterialMapTable);

typedef		GSErrCode	(__ACENV_CALL	*acapi_body_disposeProc)			(ClientID				clientPtr,
																	 void**					bodyData);

typedef		GSErrCode	(__ACENV_CALL	*acapi_body_addvertexProc)		(ClientID				clientPtr,
																	 void*					bodyData,
																	 const API_Coord3D&		coord,
																	 UInt32&				index);

typedef		GSErrCode	(__ACENV_CALL	*acapi_body_addedgeProc)			(ClientID				clientPtr,
																	 void*					bodyData,
																	 const UInt32&			vertex1,
																	 const UInt32&			vertex2,
																	 Int32&					index);

typedef		GSErrCode	(__ACENV_CALL	*acapi_body_addpolynormalProc)	(ClientID				clientPtr,
																	 void*					bodyData,
																	 const API_Vector3D&	normal,
																	 Int32&					index);

typedef		GSErrCode	(__ACENV_CALL	*acapi_body_addpolygonProc)		(ClientID					clientPtr,
																	 void*						bodyData,
																	 Int32*						edges,
																	 Int32						nEdges,
																	 Int32						polyNormal,
																	 API_MaterialOverrideType	material,
																	 UInt32&					index);


/* -- PolyRoof Decomposer ----------------- */

typedef		GSErrCode	(__ACENV_CALL	*acapi_element_decomposeProc)	(ClientID							clientPtr,
																 const API_Guid&					elemGuid,
																 API_ElementDecomposerInterface*	elementDecomposer);

/* -- ElementSet Manager ------------------ */

typedef		GSErrCode	(__ACENV_CALL	*acapi_elementset_createProc) (ClientID				clientPtr,
																 GS::Array<API_Guid>	*guids,
																 const API_UserData		*userData,
																 API_Guid				*setGuid);

typedef		GSErrCode	(__ACENV_CALL	*acapi_elementset_deleteProc) (ClientID clientPtr, API_Guid setGuid);

typedef		GSErrCode	(__ACENV_CALL	*acapi_elementset_getdataProc) (ClientID				clientPtr,
																  API_Guid				setGuid,
																  GS::Array<API_Guid>	*elemGuids,
																  API_UserData			*data);

typedef		GSErrCode	(__ACENV_CALL	*acapi_elementset_identifyProc) (ClientID clientPtr, API_Guid elemGuid, GS::Array<API_Guid> *setGuids);


/* -- Group Manager ------------------ */

typedef		GSErrCode	(__ACENV_CALL	*acapi_elementgroup_createProc) (ClientID						clientPtr,
																   const GS::Array<API_Guid>&	elemGuids,
																   API_Guid*					groupGuid,
																   const API_Guid*				parentGroupGuid);

typedef		GSErrCode	(__ACENV_CALL	*acapi_elementgroup_getgroupProc) (ClientID				clientPtr,
																	 const API_Guid&		elemGuid,
																	 API_Guid*				groupGuid);

typedef		GSErrCode	(__ACENV_CALL	*acapi_elementgroup_getrootgroupProc) (ClientID				clientPtr,
																		 const API_Guid&		elemGuid,
																		 API_Guid*				rootGroupGuid);

typedef		GSErrCode	(__ACENV_CALL	*acapi_elementgroup_getgroupedelemsProc) (ClientID				clientPtr,
																			const API_Guid&			groupGuid,
																			GS::Array<API_Guid>*	elemGuids);

typedef		GSErrCode	(__ACENV_CALL	*acapi_elementgroup_getallgroupedelemsProc) (ClientID				clientPtr,
																			   const API_Guid&		groupGuid,
																			   GS::Array<API_Guid>*	elemGuids);

typedef		GSErrCode	(__ACENV_CALL	*acapi_elementgroup_getuserdataProc) (ClientID			clientPtr,
																		const API_Guid&		groupGuid,
																		API_UserData*		userData);

typedef		GSErrCode	(__ACENV_CALL	*acapi_elementgroup_setuserdataProc) (ClientID			clientPtr,
																		const API_Guid&		groupGuid,
																		const API_UserData*	userData);

/* -- Selection Manager ------------------- */

typedef		GSErrCode	(__ACENV_CALL	*acapi_selection_getProc) (ClientID				clientPtr,
															 API_SelectionInfo*		selectionInfo,
															 API_Neig***			selNeigs,
															 bool					onlyEditable,
															 bool					ignorePartialSelection,
															 API_SelRelativePosID	relativePosToMarquee);

typedef		GSErrCode	(__ACENV_CALL	*acapi_selection_setmarqueeProc) (ClientID clientPtr, API_SelectionInfo *selectionInfo);


/* -- 3D Manager -------------------------- */

typedef		GSErrCode	(__ACENV_CALL	*acapi_3d_getnumProc) (ClientID clientPtr, API_3DTypeID typeID, Int32 *count);

typedef		GSErrCode	(__ACENV_CALL	*acapi_3d_getcomponentProc) (ClientID clientPtr, API_Component3D *component);

typedef		GSErrCode	(__ACENV_CALL	*acapi_3d_decomposepgonProc) (ClientID clientPtr, Int32 ipgon, Int32 ***cpoly);

typedef		GSErrCode	(__ACENV_CALL	*acapi_3d_createsightProc) (ClientID clientPtr, void** newSightPtr);

typedef		GSErrCode	(__ACENV_CALL	*acapi_3d_selectsightProc) (ClientID clientPtr, void* sight, void** oldSightPtr);

typedef		GSErrCode	(__ACENV_CALL	*acapi_3d_deletesightProc) (ClientID clientPtr, void* sight);

typedef		GSErrCode	(__ACENV_CALL	*acapi_3d_getcurrentwindowsightProc) (ClientID clientPtr, void** sightPtr);

typedef		GSErrCode	(__ACENV_CALL	*acapi_3d_getcutpolygoninfoProc) (ClientID 								clientPtr,
																			Int32 									bodyIndex,
																			const API_Plane3D& 						cutPlane,
																			GS::Array<Geometry::MultiPolygon2D>* 	resPolygons,
																			double* 								area);

/* -- ListData Manager -------------------- */

typedef		Int32	(__ACENV_CALL	*acapi_listdata_getsetnumProc) (ClientID clientPtr);

typedef		GSErrCode	(__ACENV_CALL	*acapi_listdata_getsetProc) (ClientID clientPtr, API_ListDataSetType *listdataSet);

typedef		GSErrCode	(__ACENV_CALL	*acapi_listdata_getnumProc) (ClientID clientPtr, Int32 setIndex, API_ListDataID typeID, Int32 *count);

typedef		GSErrCode	(__ACENV_CALL	*acapi_listdata_getProc) (ClientID clientPtr, API_ListData *listdata);

typedef		GSErrCode	(__ACENV_CALL	*acapi_listdata_searchProc) (ClientID clientPtr, API_ListData *listdata);

typedef		GSErrCode	(__ACENV_CALL	*acapi_listdata_getlocalProc) (ClientID clientPtr, Int32 libIndex, const API_Elem_Head *elemHead, API_ListData *listdata);


/* -- Communication Manager ------- */

typedef		GSErrCode	(__ACENV_CALL	*acapi_command_testProc)	(ClientID			clientPtr,
															 const API_ModulID*	mdid,
															 GSType				cmdID,
															 Int32				cmdVersion);

typedef		GSErrCode	(__ACENV_CALL	*acapi_command_callProc)	(ClientID			clientPtr,
															 const API_ModulID*	mdid,
															 GSType				cmdID,
															 Int32				cmdVersion,
															 GSHandle			params,
															 GSPtr				resultData,
															 bool				silentMode);

typedef		GSErrCode	(__ACENV_CALL	*acapi_command_externalcallProc)	(ClientID					clientPtr,
																	 const IO::Location*		projectFileLoc,
																	 const API_ModulID*			mdid,
																	 GSType						cmdID,
																	 Int32						cmdVersion,
																	 GSHandle					params,
																	 bool						silentMode,
																	 APIExternalCallBackProc*	externalCallbackProc);

typedef		GSErrCode	(__ACENV_CALL	*acapi_command_callfromeventloopProc) (ClientID					clientPtr,
																		 const API_ModulID*			mdid,
																		 GSType						cmdID,
																		 Int32						cmdVersion,
																		 GSHandle					paramsHandle,
																		 bool						silentMode,
																		 APICommandCallBackProc*	callbackProc);

/* -- ModulData Manager ---------------- */

typedef		GSErrCode	(__ACENV_CALL	*acapi_moduldata_storeProc) (ClientID clientPtr, const API_ModulData* info, const GS::UniString& modulName);

typedef		GSErrCode	(__ACENV_CALL	*acapi_moduldata_getinfoProc) (ClientID clientPtr, API_ModulData* info, const GS::UniString& modulName);

typedef		GSErrCode	(__ACENV_CALL	*acapi_moduldata_getProc) (ClientID clientPtr, API_ModulData* info, const GS::UniString& modulName);

typedef		GSErrCode	(__ACENV_CALL	*acapi_moduldata_getlistProc) (ClientID clientPtr, GS::Array<GS::UniString>* modulNameList);

typedef		GSErrCode	(__ACENV_CALL	*acapi_moduldata_deleteProc) (ClientID clientPtr, const GS::UniString& modulName);


/* -- Notification Manager ---------------- */

typedef		GSErrCode	(__ACENV_CALL	*acapi_notify_catchprojecteventProc)		(ClientID						clientPtr,
																			 GSFlags						eventTypes,
																			 APIProjectEventHandlerProc		*handlerProc);

typedef		GSErrCode	(__ACENV_CALL	*acapi_notify_catchvieweventProc)			(ClientID						clientPtr,
																			 GSFlags						eventTypes,
																			 API_NavigatorMapID				mapId,
																			 APIViewEventHandlerProc		*handlerProc);

typedef		GSErrCode	(__ACENV_CALL	*acapi_notify_catchtoolchangeProc)		(ClientID clientPtr, APIToolChangeHandlerProc *handlerProc);

typedef		GSErrCode	(__ACENV_CALL	*acapi_notify_catchselectionchangeProc)	(ClientID clientPtr, APISelectionChangeHandlerProc *handlerProc);

typedef		GSErrCode	(__ACENV_CALL	*acapi_notify_catchchangedefaultsProc)	(ClientID						clientPtr,
																			 const API_ToolBoxItem			*elemType,
																			 APIDefaultsChangeHandlerProc	*handlerProc);

typedef		GSErrCode	(__ACENV_CALL	*acapi_notify_catchnewelementProc)		(ClientID						clientPtr,
																			 const API_ToolBoxItem			*elemType,
																			 APIElementEventHandlerProc		*handlerProc);

typedef		GSErrCode	(__ACENV_CALL	*acapi_notify_installelementobserverProc)	(ClientID clientPtr, APIElementEventHandlerProc *handlerProc);

typedef		GSErrCode	(__ACENV_CALL	*acapi_notify_gettranparamsProc)			(ClientID clientPtr, API_ActTranPars *actTranPars);

typedef		GSErrCode	(__ACENV_CALL	*acapi_notify_getparentelementProc)		(ClientID			clientPtr,
																			 API_Element		*element,
																			 API_ElementMemo	*memo,
																			 UInt64				mask);

typedef		GSErrCode	(__ACENV_CALL	*acapi_notify_catchelementreservationchangeProc)	(ClientID							clientPtr,
																					 APIReservationChangeHandlerProc*	handlerProc,
																					 const GS::HashSet<API_Guid>*		filterElementsInterestedOnly);

typedef		GSErrCode	(__ACENV_CALL	*acapi_notify_catchlockablereservationchangeProc)	(ClientID							clientPtr,
																					 APILockChangeHandlerProc*			handlerProc,
																					 const GS::HashSet<API_Guid>*		filterLockablesInterestedOnly);


/* -- Miscellaneous ----------------------- */

typedef		GSErrCode	(__ACENV_CALL	*acapi_activatepropertyhandlerProc) (ClientID clientPtr, short strResID, bool active);

typedef		GSErrCode	(__ACENV_CALL	*acapi_favorite_getdefaultsProc) (ClientID			clientPtr,
																	API_Element			*element,
																	API_ElementMemo		*memo,
																	API_Element			*elementMarker,
																	API_ElementMemo		*memoMarker,
																	GS::UniString		*name,
																	short				*index);

typedef		GSErrCode	(__ACENV_CALL	*acapi_favorite_createProc) (ClientID				clientPtr,
															   API_Element			*element,
															   API_ElementMemo		*memo,
															   API_Element			*elementMarker,
															   API_ElementMemo		*memoMarker,
															   const GS::UniString	*name);

typedef		GSErrCode	(__ACENV_CALL	*acapi_favorite_deleteProc) (ClientID clientPtr, const GS::UniString* name, short *index);

typedef		GSErrCode	(__ACENV_CALL	*acapi_favorite_getnumProc) (ClientID					clientPtr,
															   API_ElemTypeID			typeID,
															   API_ElemVariationID		variationID,
															   short					*count,
															   GS::Array<short>			*indexes,
															   GS::Array<GS::UniString>	*names);


/* -- Teamwork Control -------------------- */

typedef		bool	(__ACENV_CALL	*acapi_teamworkcontrol_hasconnectionProc) (ClientID clientPtr);

typedef		bool	(__ACENV_CALL	*acapi_teamworkcontrol_isonlineProc) (ClientID clientPtr);

typedef		bool	(__ACENV_CALL	*acapi_teamworkcontrol_isserverlibpartProc) (ClientID clientPtr, const IO::Location& location);

typedef		bool	(__ACENV_CALL	*acapi_teamworkcontrol_hascreaterightProc) (ClientID clientPtr, const API_Guid& objectId);

typedef		bool	(__ACENV_CALL	*acapi_teamworkcontrol_hasdeletemodifyrightProc) (ClientID clientPtr, const API_Guid& objectId);

typedef		API_LockableStatus	(__ACENV_CALL	*acapi_teamworkcontrol_getlockablestatusProc) (ClientID clientPtr, const API_Guid& objectId, GS::PagedArray<GS::UniString>* conflicts);

typedef		API_Guid	(__ACENV_CALL	*acapi_teamworkcontrol_findlockableobjectsetProc) (ClientID clientPtr, const GS::UniString& objectSetName);

typedef		GSErrCode	(__ACENV_CALL	*acapi_teamworkcontrol_reservelockableProc) (ClientID clientPtr, const API_Guid& objectId, GS::PagedArray<GS::UniString>* conflicts);

typedef		GSErrCode	(__ACENV_CALL	*acapi_teamworkcontrol_releaselockableProc) (ClientID clientPtr, const API_Guid& objectId);

typedef		GSErrCode	(__ACENV_CALL	*acapi_teamworkcontrol_reserveelementsProc) (ClientID clientPtr, const GS::PagedArray<API_Guid>& objectIdArray, GS::HashTable<API_Guid, short>* conflicts);

typedef		GSErrCode	(__ACENV_CALL	*acapi_teamworkcontrol_releaseelementsProc) (ClientID clientPtr, const GS::PagedArray<API_Guid>& objectIdArray);

typedef		GSErrCode	(__ACENV_CALL	*acapi_teamworkcontrol_reservehotlinkcachemanagementProc) (ClientID clientPtr, short* conflict);

typedef		GSErrCode	(__ACENV_CALL	*acapi_teamworkcontrol_releasehotlinkcachemanagementProc) (ClientID clientPtr);

typedef		GSErrCode	(__ACENV_CALL	*acapi_teamworkcontrol_gethotlinkcachemanagementownerProc) (ClientID clientPtr, short* owner);

// -----------------------------------------------------------------------------
// API function table
// -----------------------------------------------------------------------------

struct API_functions {
	acapi_3d_createsightProc                                  		acapi_3d_createsight;
	acapi_3d_decomposepgonProc                                		acapi_3d_decomposepgon;
	acapi_3d_deletesightProc                                  		acapi_3d_deletesight;
	acapi_3d_getcomponentProc                                 		acapi_3d_getcomponent;
	acapi_3d_getcurrentwindowsightProc                        		acapi_3d_getcurrentwindowsight;
	acapi_3d_getcutpolygoninfoProc                            		acapi_3d_getcutpolygoninfo;
	acapi_3d_getnumProc                                       		acapi_3d_getnum;
	acapi_3d_selectsightProc                                  		acapi_3d_selectsight;
	acapi_activatepropertyhandlerProc                         		acapi_activatepropertyhandler;
	acapi_attribute_createProc                                		acapi_attribute_create;
	acapi_attribute_createextProc                             		acapi_attribute_createext;
	acapi_attribute_deleteProc                                		acapi_attribute_delete;
	acapi_attribute_deletemoreProc                            		acapi_attribute_deletemore;
	acapi_attribute_getProc                                   		acapi_attribute_get;
	acapi_attribute_getdefProc                                		acapi_attribute_getdef;
	acapi_attribute_getdefextProc                             		acapi_attribute_getdefext;
	acapi_attribute_getnumProc                                		acapi_attribute_getnum;
	acapi_attribute_getuserdataProc                           		acapi_attribute_getuserdata;
	acapi_attribute_modifyProc                                		acapi_attribute_modify;
	acapi_attribute_modifyextProc                             		acapi_attribute_modifyext;
	acapi_attribute_searchProc                                		acapi_attribute_search;
	acapi_attribute_setuserdataProc                           		acapi_attribute_setuserdata;
	acapi_automateProc                                        		acapi_automate;
	acapi_body_addedgeProc                                    		acapi_body_addedge;
	acapi_body_addpolynormalProc                              		acapi_body_addpolynormal;
	acapi_body_addpolygonProc                                 		acapi_body_addpolygon;
	acapi_body_addvertexProc                                  		acapi_body_addvertex;
	acapi_body_createProc                                     		acapi_body_create;
	acapi_body_disposeProc                                    		acapi_body_dispose;
	acapi_body_finishProc                                     		acapi_body_finish;
	acapi_callundoablecommandProc                             		acapi_callundoablecommand;
	acapi_command_callProc                                    		acapi_command_call;
	acapi_command_callfromeventloopProc                     		acapi_command_callfromeventloop;
	acapi_command_externalcallProc                            		acapi_command_externalcall;
	acapi_command_testProc                                    		acapi_command_test;
	acapi_databaseProc                                        		acapi_database;
	acapi_disposeaddparhdlProc                                		acapi_disposeaddparhdl;
	acapi_disposeattrdefshdlsProc                             		acapi_disposeattrdefshdls;
	acapi_disposeattrdefshdlsextProc                          		acapi_disposeattrdefshdlsext;
	acapi_disposebeamrelationhdlsProc                         		acapi_disposebeamrelationhdls;
	acapi_disposeelemmemohdlsProc                             		acapi_disposeelemmemohdls;
	acapi_disposeparagraphshdlProc                            		acapi_disposeparagraphshdl;
	acapi_disposeroomrelationhdlsProc                         		acapi_disposeroomrelationhdls;
	acapi_disposewallrelationhdlsProc                         		acapi_disposewallrelationhdls;
	acapi_disposeworkspaceinfohdlsProc                        		acapi_disposeworkspaceinfohdls;
	acapi_elementgroup_createProc                             		acapi_elementgroup_create;
	acapi_elementgroup_getallgroupedelemsProc                 		acapi_elementgroup_getallgroupedelems;
	acapi_elementgroup_getgroupProc                           		acapi_elementgroup_getgroup;
	acapi_elementgroup_getgroupedelemsProc                    		acapi_elementgroup_getgroupedelems;
	acapi_elementgroup_getrootgroupProc                       		acapi_elementgroup_getrootgroup;
	acapi_elementgroup_getuserdataProc                        		acapi_elementgroup_getuserdata;
	acapi_elementgroup_setuserdataProc                        		acapi_elementgroup_setuserdata;
	acapi_elementset_createProc                               		acapi_elementset_create;
	acapi_elementset_deleteProc                               		acapi_elementset_delete;
	acapi_elementset_getdataProc                              		acapi_elementset_getdata;
	acapi_elementset_identifyProc                             		acapi_elementset_identify;
	acapi_element_attachobserverProc                          		acapi_element_attachobserver;
	acapi_element_changeProc                                  		acapi_element_change;
	acapi_element_changedefaultsProc                          		acapi_element_changedefaults;
	acapi_element_changedefaultsextProc                       		acapi_element_changedefaultsext;
	acapi_element_changeextProc                               		acapi_element_changeext;
	acapi_element_changememoProc                              		acapi_element_changememo;
	acapi_element_changemoreProc                              		acapi_element_changemore;
	acapi_element_changeparametersProc                        		acapi_element_changeparameters;
	acapi_element_createProc                                  		acapi_element_create;
	acapi_element_createextProc                               		acapi_element_createext;
	acapi_element_decomposeProc                               		acapi_element_decompose;
	acapi_element_deleteProc                                  		acapi_element_delete;
	acapi_element_deleteuserdataProc                          		acapi_element_deleteuserdata;
	acapi_element_detachobserverProc                          		acapi_element_detachobserver;
	acapi_element_drwguidtolinkidProc                         		acapi_element_drwguidtolinkid;
	acapi_element_editProc                                    		acapi_element_edit;
	acapi_element_filterProc                                  		acapi_element_filter;
	acapi_element_getProc                                     		acapi_element_get;
	acapi_element_get3dinfoProc                               		acapi_element_get3dinfo;
	acapi_element_getcategoryvalueProc                        		acapi_element_getcategoryvalue;
	acapi_element_getcategoryvaluedefaultProc                 		acapi_element_getcategoryvaluedefault;
	acapi_element_getcomponentsProc                           		acapi_element_getcomponents;
	acapi_element_getconnectedelementsProc                    		acapi_element_getconnectedelements;
	acapi_element_getdefaultsProc                             		acapi_element_getdefaults;
	acapi_element_getdefaultsextProc                          		acapi_element_getdefaultsext;
	acapi_element_getdescriptorsProc                          		acapi_element_getdescriptors;
	acapi_element_getelemlistProc                             		acapi_element_getelemlist;
	acapi_element_getelemlistbyifcidentifierProc              		acapi_element_getelemlistbyifcidentifier;
	acapi_element_getheaderProc                               		acapi_element_getheader;
	acapi_element_gethotspotsProc                             		acapi_element_gethotspots;
	acapi_element_getifcattributesProc                        		acapi_element_getifcattributes;
	acapi_element_getifcclassificationreferencesProc          		acapi_element_getifcclassificationreferences;
	acapi_element_getifcidentifierProc                        		acapi_element_getifcidentifier;
	acapi_element_getifcpropertiesProc                        		acapi_element_getifcproperties;
	acapi_element_getifcpropertyvalueprimitivetypeProc        		acapi_element_getifcpropertyvalueprimitivetype;
	acapi_element_getlinkflagsProc                            		acapi_element_getlinkflags;
	acapi_element_getlinkedpropertiesProc                     		acapi_element_getlinkedproperties;
	acapi_element_getlinksProc                                		acapi_element_getlinks;
	acapi_element_getmemoProc                                 		acapi_element_getmemo;
	acapi_element_getmorequantitiesProc                       		acapi_element_getmorequantities;
	acapi_element_getobservedelementsProc                     		acapi_element_getobservedelements;
	acapi_element_getpropertiesProc                           		acapi_element_getproperties;
	acapi_element_getquantitiesProc                           		acapi_element_getquantities;
	acapi_element_getrelationsProc                            		acapi_element_getrelations;
	acapi_element_getsytranProc                               		acapi_element_getsytran;
	acapi_element_getuserdataProc                             		acapi_element_getuserdata;
	acapi_element_guidtounidProc                              		acapi_element_guidtounid;
	acapi_element_linkProc                                    		acapi_element_link;
	acapi_element_linkidtodrwguidProc                         		acapi_element_linkidtodrwguid;
	acapi_element_merge_elementsProc                          		acapi_element_merge_elements;
	acapi_element_merge_getmergedelementsProc                 		acapi_element_merge_getmergedelements;
	acapi_element_merge_removeProc                            		acapi_element_merge_remove;
	acapi_element_removeifcclassificationreferenceProc        		acapi_element_removeifcclassificationreference;
	acapi_element_removeifcpropertyProc                       		acapi_element_removeifcproperty;
	acapi_element_selectProc                                  		acapi_element_select;
	acapi_element_setcategoryvalueProc                        		acapi_element_setcategoryvalue;
	acapi_element_setcategoryvaluedefaultProc                 		acapi_element_setcategoryvaluedefault;
	acapi_element_setifcattributeProc                         		acapi_element_setifcattribute;
	acapi_element_setifcclassificationreferenceProc           		acapi_element_setifcclassificationreference;
	acapi_element_setifcpropertyProc                          		acapi_element_setifcproperty;
	acapi_element_setlinkedpropertiesProc                     		acapi_element_setlinkedproperties;
	acapi_element_setuserdataProc                             		acapi_element_setuserdata;
	acapi_element_shapeprimsProc                              		acapi_element_shapeprims;
	acapi_element_shapeprimsextProc                           		acapi_element_shapeprimsext;
	acapi_element_solidlink_createProc                        		acapi_element_solidlink_create;
	acapi_element_solidlink_getflagsProc                      		acapi_element_solidlink_getflags;
	acapi_element_solidlink_getoperationProc                  		acapi_element_solidlink_getoperation;
	acapi_element_solidlink_getoperatorsProc                  		acapi_element_solidlink_getoperators;
	acapi_element_solidlink_gettargetsProc                    		acapi_element_solidlink_gettargets;
	acapi_element_solidlink_gettimeProc                       		acapi_element_solidlink_gettime;
	acapi_element_solidlink_removeProc                        		acapi_element_solidlink_remove;
	acapi_element_toolProc                                    		acapi_element_tool;
	acapi_element_trim_elementsProc                           		acapi_element_trim_elements;
	acapi_element_trim_elementswithProc                       		acapi_element_trim_elementswith;
	acapi_element_trim_gettrimtypeProc                        		acapi_element_trim_gettrimtype;
	acapi_element_trim_gettrimmedelementsProc                 		acapi_element_trim_gettrimmedelements;
	acapi_element_trim_gettrimmingelementsProc                		acapi_element_trim_gettrimmingelements;
	acapi_element_trim_removeProc                             		acapi_element_trim_remove;
	acapi_element_unidtoguidProc                              		acapi_element_unidtoguid;
	acapi_element_unlinkProc                                  		acapi_element_unlink;
	acapi_environmentProc                                     		acapi_environment;
	acapi_favorite_createProc                                 		acapi_favorite_create;
	acapi_favorite_deleteProc                                 		acapi_favorite_delete;
	acapi_favorite_getdefaultsProc                            		acapi_favorite_getdefaults;
	acapi_favorite_getnumProc                                 		acapi_favorite_getnum;
	acapi_freecwcontourptrProc                                		acapi_freecwcontourptr;
	acapi_freegdlmodelviewoptionsptrProc                      		acapi_freegdlmodelviewoptionsptr;
	acapi_freegridmeshptrProc                                 		acapi_freegridmeshptr;
#if defined(WINDOWS)
	acapi_getextensioninstanceProc                            		acapi_getextensioninstance;
#endif
#if defined(WINDOWS)
	acapi_getmainwindowProc                                   		acapi_getmainwindow;
#endif
	acapi_getownlocationProc                                  		acapi_getownlocation;
	acapi_getownresmoduleProc                                 		acapi_getownresmodule;
	acapi_getpreferencesProc                                  		acapi_getpreferences;
	acapi_getpreferences_platformProc                         		acapi_getpreferences_platform;
	acapi_getreleasenumberProc                                		acapi_getreleasenumber;
	acapi_goodiesProc                                         		acapi_goodies;
	acapi_install_esymhandlerProc                             		acapi_install_esymhandler;
	acapi_install_filetypehandlerProc                         		acapi_install_filetypehandler;
	acapi_install_filetypehandler3dProc                       		acapi_install_filetypehandler3d;
	acapi_install_menuhandlerProc                             		acapi_install_menuhandler;
	acapi_install_modulcommandhandlerProc                     		acapi_install_modulcommandhandler;
	acapi_install_moduldatamergehandlerProc                   		acapi_install_moduldatamergehandler;
	acapi_install_moduldatasaveoldformathandlerProc           		acapi_install_moduldatasaveoldformathandler;
	acapi_install_panelhandlerProc                            		acapi_install_panelhandler;
	acapi_install_propertyhandlerProc                         		acapi_install_propertyhandler;
	acapi_interfaceProc                                       		acapi_interface;
	acapi_keepinmemoryProc                                    		acapi_keepinmemory;
	acapi_libpart_addsectionProc                              		acapi_libpart_addsection;
	acapi_libpart_createProc                                  		acapi_libpart_create;
	acapi_libpart_endsectionProc                              		acapi_libpart_endsection;
	acapi_libpart_getProc                                     		acapi_libpart_get;
	acapi_libpart_getdetailsProc                              		acapi_libpart_getdetails;
	acapi_libpart_gethotspotsProc                             		acapi_libpart_gethotspots;
	acapi_libpart_getnumProc                                  		acapi_libpart_getnum;
	acapi_libpart_getparamsProc                               		acapi_libpart_getparams;
	acapi_libpart_getsect_2ddrawhdlProc                       		acapi_libpart_getsect_2ddrawhdl;
	acapi_libpart_getsect_paramdefProc                        		acapi_libpart_getsect_paramdef;
	acapi_libpart_getsectionProc                              		acapi_libpart_getsection;
	acapi_libpart_getsectionlistProc                          		acapi_libpart_getsectionlist;
	acapi_libpart_newsectionProc                              		acapi_libpart_newsection;
	acapi_libpart_patternsearchProc                           		acapi_libpart_patternsearch;
	acapi_libpart_registerProc                                		acapi_libpart_register;
	acapi_libpart_registerallProc                             		acapi_libpart_registerall;
	acapi_libpart_saveProc                                    		acapi_libpart_save;
	acapi_libpart_searchProc                                  		acapi_libpart_search;
	acapi_libpart_setdetails_paramdefProc                     		acapi_libpart_setdetails_paramdef;
	acapi_libpart_setupsect_2ddrawhdlProc                     		acapi_libpart_setupsect_2ddrawhdl;
	acapi_libpart_shapeprimsProc                              		acapi_libpart_shapeprims;
	acapi_libpart_updatesectionProc                           		acapi_libpart_updatesection;
	acapi_libpart_writesectionProc                            		acapi_libpart_writesection;
	acapi_listdata_getProc                                    		acapi_listdata_get;
	acapi_listdata_getlocalProc                               		acapi_listdata_getlocal;
	acapi_listdata_getnumProc                                 		acapi_listdata_getnum;
	acapi_listdata_getsetProc                                 		acapi_listdata_getset;
	acapi_listdata_getsetnumProc                              		acapi_listdata_getsetnum;
	acapi_listdata_searchProc                                 		acapi_listdata_search;
	acapi_moduldata_deleteProc                                		acapi_moduldata_delete;
	acapi_moduldata_getProc                                   		acapi_moduldata_get;
	acapi_moduldata_getinfoProc                               		acapi_moduldata_getinfo;
	acapi_moduldata_getlistProc                               		acapi_moduldata_getlist;
	acapi_moduldata_storeProc                                 		acapi_moduldata_store;
	acapi_notify_catchchangedefaultsProc                      		acapi_notify_catchchangedefaults;
	acapi_notify_catchelementreservationchangeProc            		acapi_notify_catchelementreservationchange;
	acapi_notify_catchlockablereservationchangeProc           		acapi_notify_catchlockablereservationchange;
	acapi_notify_catchnewelementProc                          		acapi_notify_catchnewelement;
	acapi_notify_catchprojecteventProc                        		acapi_notify_catchprojectevent;
	acapi_notify_catchselectionchangeProc                     		acapi_notify_catchselectionchange;
	acapi_notify_catchtoolchangeProc                          		acapi_notify_catchtoolchange;
	acapi_notify_catchvieweventProc                           		acapi_notify_catchviewevent;
	acapi_notify_getparentelementProc                         		acapi_notify_getparentelement;
	acapi_notify_gettranparamsProc                            		acapi_notify_gettranparams;
	acapi_notify_installelementobserverProc                   		acapi_notify_installelementobserver;
	acapi_registermodelesswindowProc                          		acapi_registermodelesswindow;
	acapi_register_attributepanelProc                         		acapi_register_attributepanel;
	acapi_register_builtinlibraryProc                         		acapi_register_builtinlibrary;
	acapi_register_esymProc                                   		acapi_register_esym;
	acapi_register_filetypeProc                               		acapi_register_filetype;
	acapi_register_infoboxpanelProc                           		acapi_register_infoboxpanel;
	acapi_register_menuProc                                   		acapi_register_menu;
	acapi_register_moduldatahandlerProc                       		acapi_register_moduldatahandler;
	acapi_register_propertyhandlerProc                        		acapi_register_propertyhandler;
	acapi_register_requiredserviceProc                        		acapi_register_requiredservice;
	acapi_register_settingspanelProc                          		acapi_register_settingspanel;
	acapi_register_subtypeProc                                		acapi_register_subtype;
	acapi_register_supportedserviceProc                       		acapi_register_supportedservice;
	acapi_resource_getfixstrProc                              		acapi_resource_getfixstr;
	acapi_resource_getlocstrProc                              		acapi_resource_getlocstr;
	acapi_resource_getlocustrProc                             		acapi_resource_getlocustr;
	acapi_selection_getProc                                   		acapi_selection_get;
	acapi_selection_setmarqueeProc                            		acapi_selection_setmarquee;
	acapi_setdlltermhookprocProc                              		acapi_setdlltermhookproc;
	acapi_setpreferencesProc                                  		acapi_setpreferences;
	acapi_setpreferences_oldversionProc                       		acapi_setpreferences_oldversion;
	acapi_teamworkcontrol_findlockableobjectsetProc           		acapi_teamworkcontrol_findlockableobjectset;
	acapi_teamworkcontrol_gethotlinkcachemanagementownerProc  		acapi_teamworkcontrol_gethotlinkcachemanagementowner;
	acapi_teamworkcontrol_getlockablestatusProc               		acapi_teamworkcontrol_getlockablestatus;
	acapi_teamworkcontrol_hasconnectionProc                   		acapi_teamworkcontrol_hasconnection;
	acapi_teamworkcontrol_hascreaterightProc                  		acapi_teamworkcontrol_hascreateright;
	acapi_teamworkcontrol_hasdeletemodifyrightProc            		acapi_teamworkcontrol_hasdeletemodifyright;
	acapi_teamworkcontrol_isonlineProc                        		acapi_teamworkcontrol_isonline;
	acapi_teamworkcontrol_isserverlibpartProc                 		acapi_teamworkcontrol_isserverlibpart;
	acapi_teamworkcontrol_releaseelementsProc                 		acapi_teamworkcontrol_releaseelements;
	acapi_teamworkcontrol_releasehotlinkcachemanagementProc   		acapi_teamworkcontrol_releasehotlinkcachemanagement;
	acapi_teamworkcontrol_releaselockableProc                 		acapi_teamworkcontrol_releaselockable;
	acapi_teamworkcontrol_reserveelementsProc                 		acapi_teamworkcontrol_reserveelements;
	acapi_teamworkcontrol_reservehotlinkcachemanagementProc   		acapi_teamworkcontrol_reservehotlinkcachemanagement;
	acapi_teamworkcontrol_reservelockableProc                 		acapi_teamworkcontrol_reservelockable;
	acapi_unregistermodelesswindowProc                        		acapi_unregistermodelesswindow;
	acapi_writereportProc                                     		acapi_writereport;
	closeownresfileProc                                      		closeownresfile;
	openownresfileProc                                       		openownresfile;
};

extern	GSResModule		gACAPIDLL;
extern	API_functions*	gFunctionTable;
extern	ClientID		gOwnClientID;
extern	GSResModule		gOwnResModule;

// helper defines

#define	ACAPI_3D_CreateSight(a)                                   	gFunctionTable->acapi_3d_createsight                                       (gOwnClientID,a)
#define	ACAPI_3D_DecomposePgon(a,b)                               	gFunctionTable->acapi_3d_decomposepgon                                     (gOwnClientID,a,b)
#define	ACAPI_3D_DeleteSight(a)                                   	gFunctionTable->acapi_3d_deletesight                                       (gOwnClientID,a)
#define	ACAPI_3D_GetComponent(a)                                  	gFunctionTable->acapi_3d_getcomponent                                      (gOwnClientID,a)
#define	ACAPI_3D_GetCurrentWindowSight(a)                         	gFunctionTable->acapi_3d_getcurrentwindowsight                             (gOwnClientID,a)
#define	ACAPI_3D_GetCutPolygonInfo(a,b,c,d)                       	gFunctionTable->acapi_3d_getcutpolygoninfo                                 (gOwnClientID,a,b,c,d)
#define	ACAPI_3D_GetNum(a,b)                                      	gFunctionTable->acapi_3d_getnum                                            (gOwnClientID,a,b)
#define	ACAPI_3D_SelectSight(a,b)                                 	gFunctionTable->acapi_3d_selectsight                                       (gOwnClientID,a,b)
#define	ACAPI_ActivatePropertyHandler(a,b)                        	gFunctionTable->acapi_activatepropertyhandler                              (gOwnClientID,a,b)
#define	ACAPI_Attribute_Create(a,b)                               	gFunctionTable->acapi_attribute_create                                     (gOwnClientID,a,b)
#define	ACAPI_Attribute_CreateExt(a,b)                            	gFunctionTable->acapi_attribute_createext                                  (gOwnClientID,a,b)
#define	ACAPI_Attribute_Delete(a)                                 	gFunctionTable->acapi_attribute_delete                                     (gOwnClientID,a)
#define	ACAPI_Attribute_DeleteMore(a,b)                           	gFunctionTable->acapi_attribute_deletemore                                 (gOwnClientID,a,b)
#define	ACAPI_Attribute_Get(a)                                    	gFunctionTable->acapi_attribute_get                                        (gOwnClientID,a)
#define	ACAPI_Attribute_GetDef(a,b,c)                             	gFunctionTable->acapi_attribute_getdef                                     (gOwnClientID,a,b,c)
#define	ACAPI_Attribute_GetDefExt(a,b,c)                          	gFunctionTable->acapi_attribute_getdefext                                  (gOwnClientID,a,b,c)
#define	ACAPI_Attribute_GetNum(a,b)                               	gFunctionTable->acapi_attribute_getnum                                     (gOwnClientID,a,b)
#define	ACAPI_Attribute_GetUserData(a,b)                          	gFunctionTable->acapi_attribute_getuserdata                                (gOwnClientID,a,b)
#define	ACAPI_Attribute_Modify(a,b)                               	gFunctionTable->acapi_attribute_modify                                     (gOwnClientID,a,b)
#define	ACAPI_Attribute_ModifyExt(a,b)                            	gFunctionTable->acapi_attribute_modifyext                                  (gOwnClientID,a,b)
#define	ACAPI_Attribute_Search(a)                                 	gFunctionTable->acapi_attribute_search                                     (gOwnClientID,a)
#define	ACAPI_Attribute_SetUserData(a,b)                          	gFunctionTable->acapi_attribute_setuserdata                                (gOwnClientID,a,b)
#define	ACAPI_Automate(a,b,c,d)                                   	gFunctionTable->acapi_automate                                             (gOwnClientID,a,b,c,d)
#define	ACAPI_Body_AddEdge(a,b,c,d)                               	gFunctionTable->acapi_body_addedge                                         (gOwnClientID,a,b,c,d)
#define	ACAPI_Body_AddPolyNormal(a,b,c)                           	gFunctionTable->acapi_body_addpolynormal                                   (gOwnClientID,a,b,c)
#define	ACAPI_Body_AddPolygon(a,b,c,d,e,f)                        	gFunctionTable->acapi_body_addpolygon                                      (gOwnClientID,a,b,c,d,e,f)
#define	ACAPI_Body_AddVertex(a,b,c)                               	gFunctionTable->acapi_body_addvertex                                       (gOwnClientID,a,b,c)
#define	ACAPI_Body_Create(a,b,c)                                  	gFunctionTable->acapi_body_create                                          (gOwnClientID,a,b,c)
#define	ACAPI_Body_Dispose(a)                                     	gFunctionTable->acapi_body_dispose                                         (gOwnClientID,a)
#define	ACAPI_Body_Finish(a,b,c)                                  	gFunctionTable->acapi_body_finish                                          (gOwnClientID,a,b,c)
#define	ACAPI_CallUndoableCommand(a,b)                            	gFunctionTable->acapi_callundoablecommand                                  (gOwnClientID,a,b)
#define	ACAPI_Command_CallFromEventLoop(a,b,c,d,e,f)     			gFunctionTable->acapi_command_callfromeventloop							   (gOwnClientID,a,b,c,d,e,f)
#define	ACAPI_Command_Call(a,b,c,d,e,f)                           	gFunctionTable->acapi_command_call                                         (gOwnClientID,a,b,c,d,e,f)
#define	ACAPI_Command_ExternalCall(a,b,c,d,e,f,g)                 	gFunctionTable->acapi_command_externalcall                                 (gOwnClientID,a,b,c,d,e,f,g)
#define	ACAPI_Command_Test(a,b,c)                                 	gFunctionTable->acapi_command_test                                         (gOwnClientID,a,b,c)
#define	ACAPI_Database(a,b,c,d)                                   	gFunctionTable->acapi_database                                             (gOwnClientID,a,b,c,d)
#define	ACAPI_DisposeAddParHdl(a)                                 	gFunctionTable->acapi_disposeaddparhdl                                     (a)
#define	ACAPI_DisposeAttrDefsHdls(a)                              	gFunctionTable->acapi_disposeattrdefshdls                                  (a)
#define	ACAPI_DisposeAttrDefsHdlsExt(a)                           	gFunctionTable->acapi_disposeattrdefshdlsext                               (a)
#define	ACAPI_DisposeBeamRelationHdls(a)                          	gFunctionTable->acapi_disposebeamrelationhdls                              (a)
#define	ACAPI_DisposeElemMemoHdls(a)                              	gFunctionTable->acapi_disposeelemmemohdls                                  (a)
#define	ACAPI_DisposeParagraphsHdl(a)                             	gFunctionTable->acapi_disposeparagraphshdl                                 (a)
#define	ACAPI_DisposeRoomRelationHdls(a)                          	gFunctionTable->acapi_disposeroomrelationhdls                              (a)
#define	ACAPI_DisposeWallRelationHdls(a)                          	gFunctionTable->acapi_disposewallrelationhdls                              (a)
#define	ACAPI_DisposeWorkspaceInfoHdls(a)                         	gFunctionTable->acapi_disposeworkspaceinfohdls                             (a)
#define	ACAPI_ElementGroup_Create(a,b,c)                          	gFunctionTable->acapi_elementgroup_create                                  (gOwnClientID,a,b,c)
#define	ACAPI_ElementGroup_GetAllGroupedElems(a,b)                	gFunctionTable->acapi_elementgroup_getallgroupedelems                      (gOwnClientID,a,b)
#define	ACAPI_ElementGroup_GetGroup(a,b)                          	gFunctionTable->acapi_elementgroup_getgroup                                (gOwnClientID,a,b)
#define	ACAPI_ElementGroup_GetGroupedElems(a,b)                   	gFunctionTable->acapi_elementgroup_getgroupedelems                         (gOwnClientID,a,b)
#define	ACAPI_ElementGroup_GetRootGroup(a,b)                      	gFunctionTable->acapi_elementgroup_getrootgroup                            (gOwnClientID,a,b)
#define	ACAPI_ElementGroup_GetUserData(a,b)                       	gFunctionTable->acapi_elementgroup_getuserdata                             (gOwnClientID,a,b)
#define	ACAPI_ElementGroup_SetUserData(a,b)                       	gFunctionTable->acapi_elementgroup_setuserdata                             (gOwnClientID,a,b)
#define	ACAPI_ElementSet_Create(a,b,c)                            	gFunctionTable->acapi_elementset_create                                    (gOwnClientID,a,b,c)
#define	ACAPI_ElementSet_Delete(a)                                	gFunctionTable->acapi_elementset_delete                                    (gOwnClientID,a)
#define	ACAPI_ElementSet_GetData(a,b,c)                           	gFunctionTable->acapi_elementset_getdata                                   (gOwnClientID,a,b,c)
#define	ACAPI_ElementSet_Identify(a,b)                            	gFunctionTable->acapi_elementset_identify                                  (gOwnClientID,a,b)
#define	ACAPI_Element_AttachObserver(a,b)                         	gFunctionTable->acapi_element_attachobserver                               (gOwnClientID,a,b)
#define	ACAPI_Element_Change(a,b,c,d,e)                           	gFunctionTable->acapi_element_change                                       (gOwnClientID,a,b,c,d,e)
#define	ACAPI_Element_ChangeDefaults(a,b,c)                       	gFunctionTable->acapi_element_changedefaults                               (gOwnClientID,a,b,c)
#define	ACAPI_Element_ChangeDefaultsExt(a,b,c,d,e)                	gFunctionTable->acapi_element_changedefaultsext                            (gOwnClientID,a,b,c,d,e)
#define	ACAPI_Element_ChangeExt(a,b,c,d,e,f,g,h)                  	gFunctionTable->acapi_element_changeext                                    (gOwnClientID,a,b,c,d,e,f,g,h)
#define	ACAPI_Element_ChangeMemo(a,b,c)                           	gFunctionTable->acapi_element_changememo                                   (gOwnClientID,a,b,c)
#define	ACAPI_Element_ChangeMore(a,b,c,d,e,f,g)                   	gFunctionTable->acapi_element_changemore                                   (gOwnClientID,a,b,c,d,e,f,g)
#define	ACAPI_Element_ChangeParameters(a,b,c,d,e)                 	gFunctionTable->acapi_element_changeparameters                             (gOwnClientID,a,b,c,d,e)
#define	ACAPI_Element_Create(a,b)                                 	gFunctionTable->acapi_element_create                                       (gOwnClientID,a,b)
#define	ACAPI_Element_CreateExt(a,b,c,d)                          	gFunctionTable->acapi_element_createext                                    (gOwnClientID,a,b,c,d)
#define	ACAPI_Element_Decompose(a,b)                              	gFunctionTable->acapi_element_decompose                                    (gOwnClientID,a,b)
#define	ACAPI_Element_Delete(a,b)                                 	gFunctionTable->acapi_element_delete                                       (gOwnClientID,a,b)
#define	ACAPI_Element_DeleteUserData(a)                           	gFunctionTable->acapi_element_deleteuserdata                               (gOwnClientID,a)
#define	ACAPI_Element_DetachObserver(a)                           	gFunctionTable->acapi_element_detachobserver                               (gOwnClientID,a)
#define	ACAPI_Element_DrwGuidToLinkId(a)                          	gFunctionTable->acapi_element_drwguidtolinkid                              (gOwnClientID,a)
#define	ACAPI_Element_Edit(a,b,c)                                 	gFunctionTable->acapi_element_edit                                         (gOwnClientID,a,b,c)
#define	ACAPI_Element_Filter(a,b,c,d)                             	gFunctionTable->acapi_element_filter                                       (gOwnClientID,a,b,c,d)
#define	ACAPI_Element_Get(a,b)                                    	gFunctionTable->acapi_element_get                                          (gOwnClientID,a,b)
#define	ACAPI_Element_Get3DInfo(a,b)                              	gFunctionTable->acapi_element_get3dinfo                                    (gOwnClientID,a,b)
#define	ACAPI_Element_GetCategoryValue(a,b,c)                     	gFunctionTable->acapi_element_getcategoryvalue                             (gOwnClientID,a,b,c)
#define	ACAPI_Element_GetCategoryValueDefault(a,b,c,d)            	gFunctionTable->acapi_element_getcategoryvaluedefault                      (gOwnClientID,a,b,c,d)
#define	ACAPI_Element_GetComponents(a,b,c)                        	gFunctionTable->acapi_element_getcomponents                                (gOwnClientID,a,b,c)
#define	ACAPI_Element_GetConnectedElements(a,b,c,d,e,f)           	gFunctionTable->acapi_element_getconnectedelements                         (gOwnClientID,a,b,c,d,e,f)
#define	ACAPI_Element_GetDefaults(a,b)                            	gFunctionTable->acapi_element_getdefaults                                  (gOwnClientID,a,b)
#define	ACAPI_Element_GetDefaultsExt(a,b,c,d)                     	gFunctionTable->acapi_element_getdefaultsext                               (gOwnClientID,a,b,c,d)
#define	ACAPI_Element_GetDescriptors(a,b,c)                       	gFunctionTable->acapi_element_getdescriptors                               (gOwnClientID,a,b,c)
#define	ACAPI_Element_GetElemList(a,b,c,d,e)                      	gFunctionTable->acapi_element_getelemlist                                  (gOwnClientID,a,b,c,d,e)
#define	ACAPI_Element_GetElemListByIFCIdentifier(a,b,c)           	gFunctionTable->acapi_element_getelemlistbyifcidentifier                   (a,b,c)
#define	ACAPI_Element_GetHeader(a,b)                              	gFunctionTable->acapi_element_getheader                                    (gOwnClientID,a,b)
#define	ACAPI_Element_GetHotspots(a,b)                            	gFunctionTable->acapi_element_gethotspots                                  (gOwnClientID,a,b)
#define	ACAPI_Element_GetIFCAttributes(a,b)                       	gFunctionTable->acapi_element_getifcattributes                             (gOwnClientID,a,b)
#define	ACAPI_Element_GetIFCClassificationReferences(a,b)         	gFunctionTable->acapi_element_getifcclassificationreferences               (gOwnClientID,a,b)
#define	ACAPI_Element_GetIFCIdentifier(a,b,c)                     	gFunctionTable->acapi_element_getifcidentifier                             (a,b,c)
#define	ACAPI_Element_GetIFCProperties(a,b)                       	gFunctionTable->acapi_element_getifcproperties                             (gOwnClientID,a,b)
#define	ACAPI_Element_GetIFCPropertyValuePrimitiveType(a,b)       	gFunctionTable->acapi_element_getifcpropertyvalueprimitivetype             (gOwnClientID,a,b)
#define	ACAPI_Element_GetLinkFlags(a,b,c)                         	gFunctionTable->acapi_element_getlinkflags                                 (gOwnClientID,a,b,c)
#define	ACAPI_Element_GetLinkedProperties(a,b,c,d,e)              	gFunctionTable->acapi_element_getlinkedproperties                          (gOwnClientID,a,b,c,d,e)
#define	ACAPI_Element_GetLinks(a,b,c)                             	gFunctionTable->acapi_element_getlinks                                     (gOwnClientID,a,b,c)
#define	ACAPI_Element_GetMemo(a,b,c)                              	gFunctionTable->acapi_element_getmemo                                      (gOwnClientID,a,b,c)
#define	ACAPI_Element_GetMoreQuantities(a,b,c,d)                  	gFunctionTable->acapi_element_getmorequantities                            (gOwnClientID,a,b,c,d)
#define	ACAPI_Element_GetObservedElements(a,b)                    	gFunctionTable->acapi_element_getobservedelements                          (gOwnClientID,a,b)
#define	ACAPI_Element_GetProperties(a,b,c)                        	gFunctionTable->acapi_element_getproperties                                (gOwnClientID,a,b,c)
#define	ACAPI_Element_GetQuantities(a,b,c,d)                      	gFunctionTable->acapi_element_getquantities                                (gOwnClientID,a,b,c,d)
#define	ACAPI_Element_GetRelations(a,b,c)                         	gFunctionTable->acapi_element_getrelations                                 (gOwnClientID,a,b,c)
#define	ACAPI_Element_GetSyTran(a,b)                              	gFunctionTable->acapi_element_getsytran                                    (gOwnClientID,a,b)
#define	ACAPI_Element_GetUserData(a,b,c)                          	gFunctionTable->acapi_element_getuserdata                                  (gOwnClientID,a,b,c)
#define	ACAPI_Element_GuidToUnId(a,b)                             	gFunctionTable->acapi_element_guidtounid                                   (gOwnClientID,a,b)
#define	ACAPI_Element_Link(a,b,c)                                 	gFunctionTable->acapi_element_link                                         (gOwnClientID,a,b,c)
#define	ACAPI_Element_LinkIdToDrwGuid(a)                          	gFunctionTable->acapi_element_linkidtodrwguid                              (gOwnClientID,a)
#define	ACAPI_Element_Merge_Elements(a)                           	gFunctionTable->acapi_element_merge_elements                               (gOwnClientID,a)
#define	ACAPI_Element_Merge_GetMergedElements(a,b)                	gFunctionTable->acapi_element_merge_getmergedelements                      (gOwnClientID,a,b)
#define	ACAPI_Element_Merge_Remove(a,b)                           	gFunctionTable->acapi_element_merge_remove                                 (gOwnClientID,a,b)
#define	ACAPI_Element_RemoveIFCClassificationReference(a,b)       	gFunctionTable->acapi_element_removeifcclassificationreference             (gOwnClientID,a,b)
#define	ACAPI_Element_RemoveIFCProperty(a,b)                      	gFunctionTable->acapi_element_removeifcproperty                            (gOwnClientID,a,b)
#define	ACAPI_Element_Select(a,b,c)                               	gFunctionTable->acapi_element_select                                       (gOwnClientID,a,b,c)
#define	ACAPI_Element_SetCategoryValue(a,b,c)                     	gFunctionTable->acapi_element_setcategoryvalue                             (gOwnClientID,a,b,c)
#define	ACAPI_Element_SetCategoryValueDefault(a,b,c,d)            	gFunctionTable->acapi_element_setcategoryvaluedefault                      (gOwnClientID,a,b,c,d)
#define	ACAPI_Element_SetIFCAttribute(a,b)                        	gFunctionTable->acapi_element_setifcattribute                              (gOwnClientID,a,b)
#define	ACAPI_Element_SetIFCClassificationReference(a,b)          	gFunctionTable->acapi_element_setifcclassificationreference                (gOwnClientID,a,b)
#define	ACAPI_Element_SetIFCProperty(a,b)                         	gFunctionTable->acapi_element_setifcproperty                               (gOwnClientID,a,b)
#define	ACAPI_Element_SetLinkedProperties(a,b,c)                  	gFunctionTable->acapi_element_setlinkedproperties                          (gOwnClientID,a,b,c)
#define	ACAPI_Element_SetUserData(a,b)                            	gFunctionTable->acapi_element_setuserdata                                  (gOwnClientID,a,b)
#define	ACAPI_Element_ShapePrims(a,b)                             	gFunctionTable->acapi_element_shapeprims                                   (gOwnClientID,a,b)
#define	ACAPI_Element_ShapePrimsExt(a,b,c)                        	gFunctionTable->acapi_element_shapeprimsext                                (gOwnClientID,a,b,c)
#define	ACAPI_Element_SolidLink_Create(a,b,c,d)                   	gFunctionTable->acapi_element_solidlink_create                             (gOwnClientID,a,b,c,d)
#define	ACAPI_Element_SolidLink_GetFlags(a,b,c)                   	gFunctionTable->acapi_element_solidlink_getflags                           (gOwnClientID,a,b,c)
#define	ACAPI_Element_SolidLink_GetOperation(a,b,c)               	gFunctionTable->acapi_element_solidlink_getoperation                       (gOwnClientID,a,b,c)
#define	ACAPI_Element_SolidLink_GetOperators(a,b,c)               	gFunctionTable->acapi_element_solidlink_getoperators                       (gOwnClientID,a,b,c)
#define	ACAPI_Element_SolidLink_GetTargets(a,b,c)                 	gFunctionTable->acapi_element_solidlink_gettargets                         (gOwnClientID,a,b,c)
#define	ACAPI_Element_SolidLink_GetTime(a,b,c,d)                  	gFunctionTable->acapi_element_solidlink_gettime                            (gOwnClientID,a,b,c,d)
#define	ACAPI_Element_SolidLink_Remove(a,b)                       	gFunctionTable->acapi_element_solidlink_remove                             (gOwnClientID,a,b)
#define	ACAPI_Element_Tool(a,b,c,d)                               	gFunctionTable->acapi_element_tool                                         (gOwnClientID,a,b,c,d)
#define	ACAPI_Element_Trim_Elements(a)                            	gFunctionTable->acapi_element_trim_elements                                (gOwnClientID,a)
#define	ACAPI_Element_Trim_ElementsWith(a,b,c)                    	gFunctionTable->acapi_element_trim_elementswith                            (gOwnClientID,a,b,c)
#define	ACAPI_Element_Trim_GetTrimType(a,b,c)                     	gFunctionTable->acapi_element_trim_gettrimtype                             (gOwnClientID,a,b,c)
#define	ACAPI_Element_Trim_GetTrimmedElements(a,b)                	gFunctionTable->acapi_element_trim_gettrimmedelements                      (gOwnClientID,a,b)
#define	ACAPI_Element_Trim_GetTrimmingElements(a,b)               	gFunctionTable->acapi_element_trim_gettrimmingelements                     (gOwnClientID,a,b)
#define	ACAPI_Element_Trim_Remove(a,b)                            	gFunctionTable->acapi_element_trim_remove                                  (gOwnClientID,a,b)
#define	ACAPI_Element_UnIdToGuid(a,b)                             	gFunctionTable->acapi_element_unidtoguid                                   (gOwnClientID,a,b)
#define	ACAPI_Element_Unlink(a,b)                                 	gFunctionTable->acapi_element_unlink                                       (gOwnClientID,a,b)
#define	ACAPI_Environment(a,b,c,d)                                	gFunctionTable->acapi_environment                                          (gOwnClientID,a,b,c,d)
#define	ACAPI_Favorite_Create(a,b,c,d,e)                          	gFunctionTable->acapi_favorite_create                                      (gOwnClientID,a,b,c,d,e)
#define	ACAPI_Favorite_Delete(a,b)                                	gFunctionTable->acapi_favorite_delete                                      (gOwnClientID,a,b)
#define	ACAPI_Favorite_GetDefaults(a,b,c,d,e,f)                   	gFunctionTable->acapi_favorite_getdefaults                                 (gOwnClientID,a,b,c,d,e,f)
#define	ACAPI_Favorite_GetNum(a,b,c,d,e)                          	gFunctionTable->acapi_favorite_getnum                                      (gOwnClientID,a,b,c,d,e)
#define	ACAPI_FreeCWContourPtr(a)                                 	gFunctionTable->acapi_freecwcontourptr                                     (a)
#define	ACAPI_FreeGDLModelViewOptionsPtr(a)                       	gFunctionTable->acapi_freegdlmodelviewoptionsptr                           (a)
#define	ACAPI_FreeGridMeshPtr(a)                                  	gFunctionTable->acapi_freegridmeshptr                                      (a)
#if defined(WINDOWS)
#define	ACAPI_GetExtensionInstance()                              	gFunctionTable->acapi_getextensioninstance                                 ()
#endif
#if defined(WINDOWS)
#define	ACAPI_GetMainWindow()                                     	gFunctionTable->acapi_getmainwindow                                        ()
#endif
#define	ACAPI_GetOwnLocation(a)                                   	gFunctionTable->acapi_getownlocation                                       (a)
#define	ACAPI_GetOwnResModule()                                   	gFunctionTable->acapi_getownresmodule                                      ()
#define	ACAPI_GetPreferences(a,b,c)                               	gFunctionTable->acapi_getpreferences                                       (gOwnClientID,a,b,c)
#define	ACAPI_GetPreferences_Platform(a,b,c,d)                    	gFunctionTable->acapi_getpreferences_platform                              (a,b,c,d)
#define	ACAPI_GetReleaseNumber(a)                                 	gFunctionTable->acapi_getreleasenumber                                     (gOwnClientID,a)
#define	ACAPI_Goodies(a,b,c,d)                                    	gFunctionTable->acapi_goodies                                              (gOwnClientID,a,b,c,d)
#define	ACAPI_Install_ESYMHandler(a,b)                            	gFunctionTable->acapi_install_esymhandler                                  (gOwnClientID,a,b)
#define	ACAPI_Install_FileTypeHandler(a,b)                        	gFunctionTable->acapi_install_filetypehandler                              (gOwnClientID,a,b)
#define	ACAPI_Install_FileTypeHandler3D(a,b)                      	gFunctionTable->acapi_install_filetypehandler3d                            (gOwnClientID,a,b)
#define	ACAPI_Install_MenuHandler(a,b)                            	gFunctionTable->acapi_install_menuhandler                                  (gOwnClientID,a,b)
#define	ACAPI_Install_ModulCommandHandler(a,b,c)                  	gFunctionTable->acapi_install_modulcommandhandler                          (gOwnClientID,a,b,c)
#define	ACAPI_Install_ModulDataMergeHandler(a)                    	gFunctionTable->acapi_install_moduldatamergehandler                        (gOwnClientID,a)
#define	ACAPI_Install_ModulDataSaveOldFormatHandler(a)            	gFunctionTable->acapi_install_moduldatasaveoldformathandler                (gOwnClientID,a)
#define	ACAPI_Install_PanelHandler(a,b,c)                         	gFunctionTable->acapi_install_panelhandler                                 (gOwnClientID,a,b,c)
#define	ACAPI_Install_PropertyHandler(a,b)                        	gFunctionTable->acapi_install_propertyhandler                              (gOwnClientID,a,b)
#define	ACAPI_Interface(a,b,c,d,e)                                	gFunctionTable->acapi_interface                                            (gOwnClientID,a,b,c,d,e)
#define	ACAPI_KeepInMemory(a)                                     	gFunctionTable->acapi_keepinmemory                                         (gOwnClientID,a)
#define	ACAPI_LibPart_AddSection(a,b,c)                           	gFunctionTable->acapi_libpart_addsection                                   (gOwnClientID,a,b,c)
#define	ACAPI_LibPart_Create(a)                                   	gFunctionTable->acapi_libpart_create                                       (gOwnClientID,a)
#define	ACAPI_LibPart_EndSection()                                	gFunctionTable->acapi_libpart_endsection                                   ()
#define	ACAPI_LibPart_Get(a)                                      	gFunctionTable->acapi_libpart_get                                          (gOwnClientID,a)
#define	ACAPI_LibPart_GetDetails(a,b)                             	gFunctionTable->acapi_libpart_getdetails                                   (gOwnClientID,a,b)
#define	ACAPI_LibPart_GetHotspots(a,b,c,d)                        	gFunctionTable->acapi_libpart_gethotspots                                  (gOwnClientID,a,b,c,d)
#define	ACAPI_LibPart_GetNum(a)                                   	gFunctionTable->acapi_libpart_getnum                                       (gOwnClientID,a)
#define	ACAPI_LibPart_GetParams(a,b,c,d,e)                        	gFunctionTable->acapi_libpart_getparams                                    (gOwnClientID,a,b,c,d,e)
#define	ACAPI_LibPart_GetSect_2DDrawHdl(a)                        	gFunctionTable->acapi_libpart_getsect_2ddrawhdl                            (gOwnClientID,a)
#define	ACAPI_LibPart_GetSect_ParamDef(a,b,c,d,e,f,g)             	gFunctionTable->acapi_libpart_getsect_paramdef                             (gOwnClientID,a,b,c,d,e,f,g)
#define	ACAPI_LibPart_GetSection(a,b,c,d,e)                       	gFunctionTable->acapi_libpart_getsection                                   (gOwnClientID,a,b,c,d,e)
#define	ACAPI_LibPart_GetSectionList(a,b,c)                       	gFunctionTable->acapi_libpart_getsectionlist                               (gOwnClientID,a,b,c)
#define	ACAPI_LibPart_NewSection(a)                               	gFunctionTable->acapi_libpart_newsection                                   (gOwnClientID,a)
#define	ACAPI_LibPart_PatternSearch(a,b,c,d)                      	gFunctionTable->acapi_libpart_patternsearch                                (gOwnClientID,a,b,c,d)
#define	ACAPI_LibPart_Register(a)                                 	gFunctionTable->acapi_libpart_register                                     (gOwnClientID,a)
#define	ACAPI_LibPart_RegisterAll(a)                              	gFunctionTable->acapi_libpart_registerall                                  (gOwnClientID,a)
#define	ACAPI_LibPart_Save(a)                                     	gFunctionTable->acapi_libpart_save                                         (gOwnClientID,a)
#define	ACAPI_LibPart_Search(a,b,c)                               	gFunctionTable->acapi_libpart_search                                       (gOwnClientID,a,b,c)
#define	ACAPI_LibPart_SetDetails_ParamDef(a,b,c)                  	gFunctionTable->acapi_libpart_setdetails_paramdef                          (gOwnClientID,a,b,c)
#define	ACAPI_LibPart_SetUpSect_2DDrawHdl()                       	gFunctionTable->acapi_libpart_setupsect_2ddrawhdl                          ()
#define	ACAPI_LibPart_ShapePrims(a,b,c,d)                         	gFunctionTable->acapi_libpart_shapeprims                                   (gOwnClientID,a,b,c,d)
#define	ACAPI_LibPart_UpdateSection(a,b,c,d,e)                    	gFunctionTable->acapi_libpart_updatesection                                (gOwnClientID,a,b,c,d,e)
#define	ACAPI_LibPart_WriteSection(a,b)                           	gFunctionTable->acapi_libpart_writesection                                 (gOwnClientID,a,b)
#define	ACAPI_ListData_Get(a)                                     	gFunctionTable->acapi_listdata_get                                         (gOwnClientID,a)
#define	ACAPI_ListData_GetLocal(a,b,c)                            	gFunctionTable->acapi_listdata_getlocal                                    (gOwnClientID,a,b,c)
#define	ACAPI_ListData_GetNum(a,b,c)                              	gFunctionTable->acapi_listdata_getnum                                      (gOwnClientID,a,b,c)
#define	ACAPI_ListData_GetSet(a)                                  	gFunctionTable->acapi_listdata_getset                                      (gOwnClientID,a)
#define	ACAPI_ListData_GetSetNum()                                	gFunctionTable->acapi_listdata_getsetnum                                   ()
#define	ACAPI_ListData_Search(a)                                  	gFunctionTable->acapi_listdata_search                                      (gOwnClientID,a)
#define	ACAPI_ModulData_Delete(a)                                 	gFunctionTable->acapi_moduldata_delete                                     (gOwnClientID,a)
#define	ACAPI_ModulData_Get(a,b)                                  	gFunctionTable->acapi_moduldata_get                                        (gOwnClientID,a,b)
#define	ACAPI_ModulData_GetInfo(a,b)                              	gFunctionTable->acapi_moduldata_getinfo                                    (gOwnClientID,a,b)
#define	ACAPI_ModulData_GetList(a)                                	gFunctionTable->acapi_moduldata_getlist                                    (gOwnClientID,a)
#define	ACAPI_ModulData_Store(a,b)                                	gFunctionTable->acapi_moduldata_store                                      (gOwnClientID,a,b)
#define	ACAPI_Notify_CatchChangeDefaults(a,b)                     	gFunctionTable->acapi_notify_catchchangedefaults                           (gOwnClientID,a,b)
#define	ACAPI_Notify_CatchElementReservationChange(a,b)           	gFunctionTable->acapi_notify_catchelementreservationchange                 (gOwnClientID,a,b)
#define	ACAPI_Notify_CatchLockableReservationChange(a,b)          	gFunctionTable->acapi_notify_catchlockablereservationchange                (gOwnClientID,a,b)
#define	ACAPI_Notify_CatchNewElement(a,b)                         	gFunctionTable->acapi_notify_catchnewelement                               (gOwnClientID,a,b)
#define	ACAPI_Notify_CatchProjectEvent(a,b)                       	gFunctionTable->acapi_notify_catchprojectevent                             (gOwnClientID,a,b)
#define	ACAPI_Notify_CatchSelectionChange(a)                      	gFunctionTable->acapi_notify_catchselectionchange                          (gOwnClientID,a)
#define	ACAPI_Notify_CatchToolChange(a)                           	gFunctionTable->acapi_notify_catchtoolchange                               (gOwnClientID,a)
#define	ACAPI_Notify_CatchViewEvent(a,b,c)                        	gFunctionTable->acapi_notify_catchviewevent                                (gOwnClientID,a,b,c)
#define	ACAPI_Notify_GetParentElement(a,b,c,d)                     	gFunctionTable->acapi_notify_getparentelement                              (gOwnClientID,a,b,c,d)
#define	ACAPI_Notify_GetTranParams(a)                             	gFunctionTable->acapi_notify_gettranparams                                 (gOwnClientID,a)
#define	ACAPI_Notify_InstallElementObserver(a)                    	gFunctionTable->acapi_notify_installelementobserver                        (gOwnClientID,a)
#define	ACAPI_RegisterModelessWindow(a,b,c,d)                     	gFunctionTable->acapi_registermodelesswindow                               (gOwnClientID,a,b,c,d)
#define	ACAPI_Register_AttributePanel(a,b,c,d,e,f)                	gFunctionTable->acapi_register_attributepanel                              (a,b,c,d,e,f)
#define	ACAPI_Register_BuiltInLibrary()                           	gFunctionTable->acapi_register_builtinlibrary                              ()
#define	ACAPI_Register_ESYM(a,b,c)                                	gFunctionTable->acapi_register_esym                                        (a,b,c)
#define	ACAPI_Register_FileType(a,b,c,d,e,f,g,h)                  	gFunctionTable->acapi_register_filetype                                    (a,b,c,d,e,f,g,h)
#define	ACAPI_Register_InfoBoxPanel(a,b,c,d,e,f)                  	gFunctionTable->acapi_register_infoboxpanel                                (a,b,c,d,e,f)
#define	ACAPI_Register_Menu(a,b,c,d)                              	gFunctionTable->acapi_register_menu                                        (a,b,c,d)
#define	ACAPI_Register_ModulDataHandler()                         	gFunctionTable->acapi_register_moduldatahandler                            ()
#define	ACAPI_Register_PropertyHandler(a)                         	gFunctionTable->acapi_register_propertyhandler                             (a)
#define	ACAPI_Register_RequiredService(a,b,c)                     	gFunctionTable->acapi_register_requiredservice                             (a,b,c)
#define	ACAPI_Register_SettingsPanel(a,b,c,d,e,f,g,h)             	gFunctionTable->acapi_register_settingspanel                               (a,b,c,d,e,f,g,h)
#define	ACAPI_Register_Subtype(a,b,c,d,e,f,g,h,i)                 	gFunctionTable->acapi_register_subtype                                     (a,b,c,d,e,f,g,h,i)
#define	ACAPI_Register_SupportedService(a,b)                      	gFunctionTable->acapi_register_supportedservice                            (a,b)
#define	ACAPI_Resource_GetFixStr(a,b,c,d)                         	gFunctionTable->acapi_resource_getfixstr                                   (a,b,c,d)
#define	ACAPI_Resource_GetLocStr(a,b,c,d)                         	gFunctionTable->acapi_resource_getlocstr                                   (a,b,c,d)
#define	ACAPI_Resource_GetLocUStr(a,b,c,d)                        	gFunctionTable->acapi_resource_getlocustr                                  (a,b,c,d)
#define	ACAPI_Selection_Get(a,b,c,d,e)                            	gFunctionTable->acapi_selection_get                                        (gOwnClientID,a,b,c,d,e)
#define	ACAPI_Selection_SetMarquee(a)                             	gFunctionTable->acapi_selection_setmarquee                                 (gOwnClientID,a)
#define	ACAPI_SetDllTermHookProc(a)                               	gFunctionTable->acapi_setdlltermhookproc                                   (a)
#define	ACAPI_SetPreferences(a,b,c)                               	gFunctionTable->acapi_setpreferences                                       (gOwnClientID,a,b,c)
#define	ACAPI_SetPreferences_OldVersion(a,b,c,d,e)                	gFunctionTable->acapi_setpreferences_oldversion                            (gOwnClientID,a,b,c,d,e)
#define	ACAPI_TeamworkControl_FindLockableObjectSet(a)            	gFunctionTable->acapi_teamworkcontrol_findlockableobjectset                (gOwnClientID,a)
#define	ACAPI_TeamworkControl_GetHotlinkCacheManagementOwner(a)   	gFunctionTable->acapi_teamworkcontrol_gethotlinkcachemanagementowner       (gOwnClientID,a)
#define	ACAPI_TeamworkControl_GetLockableStatus(a,b)              	gFunctionTable->acapi_teamworkcontrol_getlockablestatus                    (gOwnClientID,a,b)
#define	ACAPI_TeamworkControl_HasConnection()                     	gFunctionTable->acapi_teamworkcontrol_hasconnection                        ()
#define	ACAPI_TeamworkControl_HasCreateRight(a)                   	gFunctionTable->acapi_teamworkcontrol_hascreateright                       (gOwnClientID,a)
#define	ACAPI_TeamworkControl_HasDeleteModifyRight(a)             	gFunctionTable->acapi_teamworkcontrol_hasdeletemodifyright                 (gOwnClientID,a)
#define	ACAPI_TeamworkControl_IsOnline()                          	gFunctionTable->acapi_teamworkcontrol_isonline                             ()
#define	ACAPI_TeamworkControl_IsServerLibPart(a)                  	gFunctionTable->acapi_teamworkcontrol_isserverlibpart                      (gOwnClientID,a)
#define	ACAPI_TeamworkControl_ReleaseElements(a)                  	gFunctionTable->acapi_teamworkcontrol_releaseelements                      (gOwnClientID,a)
#define	ACAPI_TeamworkControl_ReleaseHotlinkCacheManagement()     	gFunctionTable->acapi_teamworkcontrol_releasehotlinkcachemanagement        ()
#define	ACAPI_TeamworkControl_ReleaseLockable(a)                  	gFunctionTable->acapi_teamworkcontrol_releaselockable                      (gOwnClientID,a)
#define	ACAPI_TeamworkControl_ReserveElements(a,b,c)              	gFunctionTable->acapi_teamworkcontrol_reserveelements                      (gOwnClientID,a,b,c)
#define	ACAPI_TeamworkControl_ReserveHotlinkCacheManagement(a)    	gFunctionTable->acapi_teamworkcontrol_reservehotlinkcachemanagement        (gOwnClientID,a)
#define	ACAPI_TeamworkControl_ReserveLockable(a,b)                	gFunctionTable->acapi_teamworkcontrol_reservelockable                      (gOwnClientID,a,b)
#define	ACAPI_UnregisterModelessWindow(a)                         	gFunctionTable->acapi_unregistermodelesswindow                             (gOwnClientID,a)
#define	ACAPI_WriteReport(a,b)                                    	gFunctionTable->acapi_writereport                                          (gOwnClientID,a,b)

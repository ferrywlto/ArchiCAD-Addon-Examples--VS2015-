// *****************************************************************************
// Header file for the Element Test Add-On
// API Development Kit 19; Mac/Win
//
// Namespaces:        Contact person:
//     -None-
//
// [SG compatible] - Yes
// *****************************************************************************

#if !defined _ELEMENT_TEST_H
#define _ELEMENT_TEST_H


// =============================================================================
//
// General element functions
//
// =============================================================================

void	Do_CountLines (void);
void	Do_CreateLine (API_Guid& guid);
void	Do_GetLineByGuid (const API_Guid& guid);
void	Do_CreateDetail (void);
void	Do_CreateLabel (void);
void	Do_CreateLabel_Associative (void);
void	Do_CreateZone (short geometryId);
void	Do_CreateCamset (void);
void	Do_CreatePerspCam (void);
void	Do_CreatePicture (void);
void	Do_CreateGroupedLines (void);
void	Do_CreateCutPlane (void);
void	Do_CreateInteriorElevation (void);
void	Do_CreateWindow (void);
void	Do_CreateSkylight (void);
void	Do_CreatePolyRoof (void);
void	Do_CreateExtrudedShell (void);
void	Do_CreateRevolvedShell (void);
void	Do_CreateRuledShell (void);
void	Do_CreateMorph (void);
void	Do_CreateSlab (void);
void	Do_CreateCurvedWall (void);
void	Do_CreateCurvedBeam (void);
void	Do_CreateCurtainWall (void);
void	Do_CreateWord (const API_Guid& renFiltGuid);
void	Do_CreateDrawing (const API_Guid& elemGuid, API_Coord* pos = NULL);
void	Do_CreateDrawingFromPlanView (void);
void	Do_CreateDrawingFromSelection (void);
void	Do_SelectElems (void);
void	Do_DeleteElems (void);
void	Do_PickupElem (void);
void	Do_ChangeElem (void);
void	Do_ExplodeElem (void);
void	Do_CopyElem (void);
void	Do_PickupProperties (void);
void	Do_FillProperties (void);
void	Do_DumpElem (API_Guid& renFiltGuid);
void	Do_SetElemCategories (bool changeDefaults);
void	Do_CreateIrregularMesh (void);
void	Do_CreateIrregularSlab (void);
void	Do_CreateIrregularExtShell (void);
void	Do_CreateChangeMarker (void);
void	Do_CreateStaticDimension (void);
void	Do_CreateStaticAngleDimension (void);


// =============================================================================
//
// Edit elements
//
// =============================================================================

void	Do_DragElems (bool withInput);
void	Do_StretchElems (bool withInput);
void	Do_ResizeElems (bool withInput);
void	Do_RotateElems (bool withInput);
void	Do_MirrorElems (bool withInput);
void	Do_ElevElems (bool withInput);


// =============================================================================
//
// Apply tools on elements
//
// =============================================================================

void	Do_GroupElems (bool withInput);
void	Do_UngroupElems (bool withInput);
void	Do_LockElems (bool withInput);
void	Do_UnlockElems (bool withInput);
void	Do_ForwardElems (bool withInput);
void	Do_BackwardElems (bool withInput);
void	Do_SuspendGroups (void);


// =============================================================================
//
// Modify elements
//
// =============================================================================

void	Do_Change_ElemInfo (void);
void	Do_Change_ElemParameters (bool defaults);
void	Do_Poly_AdjustNode (void);
void	Do_Poly_InsertNode (void);
void	Do_Poly_DeleteNode (void);
void	Do_Poly_DeleteHole (void);
void	Do_Poly_NewHole (void);

void	Do_Wall_Edit (void);
void	Do_Column_Edit (void);
void	Do_Beam_Edit (void);
void	Do_Window_Edit (void);
void	Do_Skylight_Edit (void);
void	Do_Object_Edit (void);
void	Do_Ceil_Edit (void);
void	Do_Roof_Edit (void);
void	Do_Shell_Edit (void);
void	Do_Shell_Edit2 (void);
void	Do_Morph_Edit (void);
void	Do_Mesh_Edit (void);
void	Do_Zone_Edit (short mode);
void	Do_Word_Edit (short mode);
void	Do_2D_Edit (void);
void	Do_Hotspot_Edit (void);
void	Do_Spline_Edit (void);
void	Do_CutPlane_Edit (void);
void	Do_Hatch_Edit (void);
void	Do_PolyLine_Edit (void);
void	Do_Label_Edit (void);
void	Do_Dimension_Edit (void);
void	Do_LevelDimension_Edit (void);
void	Do_RadialDimension_Edit (void);
void	Do_AngleDimension_Edit (void);
void	Do_Detail_Edit (void);
void	Do_ChangeMarker_Edit (void);
void	Do_Dimensions_Test (void);
void	Do_Drawing_Edit (void);
void	Do_ChangeInteriorElevation (void);
void	Do_RotateRenovationStatus (void);

// =============================================================================
//
// Group elements
//
// =============================================================================

void	Do_CreateCustomGroup (void);
void	Do_ListMyGroups (void);
void	Do_ChangeMyGroup (void);
void	Do_UnbindMyGroup (void);

// =============================================================================
//
// Observe elements
//
// =============================================================================

void	Do_CatchElementCreation (void);


// =============================================================================
//
// Manage element sets
//
// =============================================================================

void	Do_ElemSet_Create (void);
void	Do_ElemSet_Delete (void);


// =============================================================================
//
// Manage solid operation links
//
// =============================================================================

void	Do_SolidLink_Create (void);
void	Do_SolidLink_Remove (void);
void	Do_SolidLink_Targets (void);
void	Do_SolidLink_Operators (void);


// =============================================================================
//
// Manage trim/merge
//
// =============================================================================

void	Do_Trim_Elements (void);
void	Do_Trim_ElementsWith (void);
void	Do_Trim_Remove (void);
void	Do_Trim_GetTrimType (void);
void	Do_Trim_GetTrimmedElements (void);
void	Do_Trim_GetTrimmingElements (void);
void	Do_Merge_Elements (void);
void	Do_Merge_GetMergedElements (void);
void	Do_Merge_Remove (void);


// =============================================================================
//
// Manage hotlinks
//
// =============================================================================

void	Do_CreateHotlink (void);
void	Do_UpdateHotlink (void);
void	Do_DeleteHotlink (void);
void	Do_BreakHotlink (void);
void	Do_ListHotlinks (void);


// =============================================================================
//
// Tips & Tricks
//
// =============================================================================

void	Do_AvoidFlashing (short mode);


// =============================================================================
//
// Code snippets
//
// =============================================================================

void	Do_DeleteCamset (bool active);
void	Do_DumpZone (void);
void	Do_DumpWall (void);
void	Do_DumpBeam (void);
void	Do_DumpWindow (void);
void	Do_DumpDoor (void);
void	Do_DumpShell (void);
void	Do_NeigToCoord (void);
void	Do_GetBounds (void);
void	Do_ShowSymbolHotspots (void);
void	Do_CalcQuantities (void);
void	Do_GetComponents (void);
void	Do_SetToolBoxMode (void);
void	Do_ChangeSubtypeSettings (void);
void	Do_SplitPolyRoof (void);

GSErrCode __ACENV_CALL Do_ExportPrimitives (const API_IOParams *ioParams);
GSErrCode __ACENV_CALL Do_Import (const API_IOParams *ioParams);

// =============================================================================
//
// Utility functions
//
// =============================================================================


bool	GetEditVector (API_Coord3D	*begC,
					   API_Coord3D	*endC,
					   const char	*prompt,
					   bool		begGiven);

bool	GetEditArc (API_Coord3D	*begC,
					API_Coord3D *endC,
					API_Coord3D *origC,
					const char	*prompt);

bool	DumpPolygon (const API_Element	*element,
					 short				lineInd,
					 const double		offset,
					 Int32				nCoords,
					 Int32				nSubPolys,
					 Int32				nArcs,
					 API_Coord			**coords,
					 Int32				**subPolys,
					 API_PolyArc		**arcs,
					 bool				createShadow,
				 	 bool				writeReport);

void	SearchActiveCamset (API_Guid*	actGuid,
							API_Guid*	perspGuid);


#endif

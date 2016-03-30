#pragma once
#include "ACAPinc.h"					// also includes APIdefs.h
API_MaterialType GetUMaterial(short iumat);
void	DumpOwner(const char *info, const API_ProjectInfo *projectInfo, const API_SharingInfo	*sharingInfo, short userId);
void	DumpBody(Int32 ibody, bool detailed, const API_Coord& dbOffset);
void	DumpLight(Int32 ilght, bool detailed);
GSErrCode Do_Dump3DModel(bool fromSaveAs);


void Do_DumpElemCategoryValue(const API_ElemCategoryValue& elemCategoryValue);
void Do_DumpElemCategories(const API_Guid& elemGuid, const API_ElemTypeID& typeID, const API_ElemVariationID& variationID, bool dumpDefaults);
void Do_DumpAllElement();
void Do_Save_As_Image();
void Do_Save_IfcFile();
void Do_Save_PdfFile();
void Do_CopySlabs();
void Do_CopyWalls();
void Do_CopyObjects();
void Do_CreatePipesAndWalls();
void Do_Call_Web();

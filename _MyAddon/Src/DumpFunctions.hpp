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
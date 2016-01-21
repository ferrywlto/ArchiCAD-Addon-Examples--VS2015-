
// -----------------------------------------------------------------------------
// Get the actual 3D model
// -----------------------------------------------------------------------------
#include "APIEnvir.h"
#include "ACAPinc.h"					// also includes APIdefs.h
#include "APICommon.h"
#include <string>

// -----------------------------------------------------------------------------
// Get a material with index
// -----------------------------------------------------------------------------

API_MaterialType	GetUMaterial(short iumat)
{
	static GS::HashTable<short, API_MaterialType> umatCache;

	API_Component3D	mat;
	BNZeroMemory(&mat, sizeof(mat));
	mat.header.typeID = API_UmatID;
	mat.header.index = iumat;
	if (umatCache.ContainsKey(iumat))
		return umatCache.Get(iumat);

	ACAPI_3D_GetComponent(&mat);
	delete mat.umat.mater.texture.fileLoc;
	mat.umat.mater.texture.fileLoc = NULL;

	umatCache.Add(iumat, mat.umat.mater);
	return mat.umat.mater;
}

// -----------------------------------------------------------------------------
// Dump owner information
// -----------------------------------------------------------------------------
void		DumpOwner(const char 				*info,
	const API_ProjectInfo	*projectInfo,
	const API_SharingInfo	*sharingInfo,
	short 					userId)
{
	char		ownerStr[256];
	bool		found;
	Int32		i;

	if (userId == 0) {
		strcpy(ownerStr, "none");
	}
	else if (userId == projectInfo->userId) {
		strcpy(ownerStr, "me");
	}
	else {
		found = false;
		for (i = 0; i < sharingInfo->nUsers; i++) {
			if ((*sharingInfo->users)[i].userId == userId) {
				found = true;
				break;
			}
		}
		if (found) {
			GS::UniString fullName((*sharingInfo->users)[i].fullName);
			sprintf(ownerStr, "\"%s\" connected: %s", fullName.ToCStr().Get(), (*sharingInfo->users)[i].connected ? "YES" : "NO");
		}
		else {
			sprintf(ownerStr, "not found");
		}
	}
	WriteReport("%s %s", info, ownerStr);

	return;
}		// DumpOwner


void	DumpBody(Int32 ibody, bool detailed, const API_Coord& dbOffset)
{
	API_Component3D 	component;
	Int32				j, nVert, nEdge, nPgon;
	API_Tranmat			tm;
	API_UVCoord			resultUV;
	Int32				elemIdx, bodyIdx, ivect;
	GSErrCode			err;

	WriteReport("BODY #%d", ibody);

	BNZeroMemory(&component, sizeof(component));

	component.header.typeID = API_BodyID;
	component.header.index = ibody;
	err = ACAPI_3D_GetComponent(&component);
	if (err != NoError) {
		WriteReport("    ERROR");
		return;
	}

	if (!detailed)
		return;

	nVert = component.body.nVert;
	nEdge = component.body.nEdge;
	nPgon = component.body.nPgon;
	tm = component.body.tranmat;
	elemIdx = component.body.head.elemIndex - 1;
	bodyIdx = component.body.head.bodyIndex - 1;

	WriteReport("  VERTs");
	for (j = 1; j <= nVert; j++) {
		component.header.typeID = API_VertID;
		component.header.index = j;
		err = ACAPI_3D_GetComponent(&component);
		if (err == NoError) {
			API_Coord3D	trCoord;	// world coordinates
			trCoord.x = tm.tmx[0] * component.vert.x + tm.tmx[1] * component.vert.y + tm.tmx[2] * component.vert.z + tm.tmx[3];
			trCoord.y = tm.tmx[4] * component.vert.x + tm.tmx[5] * component.vert.y + tm.tmx[6] * component.vert.z + tm.tmx[7];
			trCoord.z = tm.tmx[8] * component.vert.x + tm.tmx[9] * component.vert.y + tm.tmx[10] * component.vert.z + tm.tmx[11];
			trCoord.x += dbOffset.x;
			trCoord.y += dbOffset.y;
			WriteReport("    [%2d]  (%lf, %lf, %lf)",
				j,
				trCoord.x, trCoord.y, trCoord.z);
		}
	}

	WriteReport("  EDGEs");
	for (j = 1; j <= nEdge; j++) {
		component.header.typeID = API_EdgeID;
		component.header.index = j;
		err = ACAPI_3D_GetComponent(&component);
		if (err == NoError)
			WriteReport("    [%2d]  (%3d, %3d) - (%3d, %3d)",
				j,
				component.edge.vert1, component.edge.vert2,
				component.edge.pgon1, component.edge.pgon2);
	}

	WriteReport("  PGONs %ld %ld %ld", nPgon, elemIdx, bodyIdx);
	for (j = 1; j <= nPgon; j++) {
		component.header.typeID = API_PgonID;
		component.header.index = j;
		err = ACAPI_3D_GetComponent(&component);
		if (err == NoError)
			WriteReport("    [%2d]  (first edge: %3d, last edge: %3d, ivect: %3d)",
				j,
				component.pgon.fpedg, component.pgon.lpedg, component.pgon.ivect);

		API_MaterialType mat = GetUMaterial(component.pgon.iumat);
		WriteReport("    MATERIAL %s [index: %d], guid: %s", mat.head.name, mat.head.index, APIGuidToString(mat.head.guid).ToCStr().Get());

		if (elemIdx < 0 || bodyIdx < 0) {
			WriteReport("  No TEXTURE info");
			continue;
		}

		ivect = component.pgon.ivect;

		WriteReport("    TEXTURE");

		API_TexCoordPars	pars = { 0 };
		pars.elemIdx = elemIdx;
		pars.bodyIdx = bodyIdx;
		pars.pgonIndex = j;
		//pars.filler_1;
		// get a coordinate for surface point
		component.header.typeID = API_PedgID;
		component.header.index = component.pgon.fpedg;
		err = ACAPI_3D_GetComponent(&component);
		if (err == NoError) {
			component.header.typeID = API_EdgeID;
			component.header.index = ::abs(component.pedg.pedg);
			err = ACAPI_3D_GetComponent(&component);
			if (err == NoError) {
				component.header.typeID = API_VertID;
				component.header.index = ::abs(component.edge.vert1);
				err = ACAPI_3D_GetComponent(&component);
				if (err == NoError) {
					pars.surfacePoint.x = component.vert.x;
					pars.surfacePoint.y = component.vert.y;
					pars.surfacePoint.z = component.vert.z;
					err = ACAPI_Goodies(APIAny_GetTextureCoordID, &pars, &resultUV);
					if (err == NoError)
						WriteReport("          (texture: (u,v) : (%5.2lf, %5.2lf)",
							resultUV.u, resultUV.v);
				}
			}
		}

		WriteReport("    NORMALVECTOR");
		component.header.typeID = API_VectID;
		component.header.index = ::abs(ivect);
		err = ACAPI_3D_GetComponent(&component);
		if (err == NoError) {
			if (ivect < 0) {
				component.vect.x = -component.vect.x;
				component.vect.y = -component.vect.y;
				component.vect.z = -component.vect.z;
			}
			if (err == NoError) {
				API_Coord3D	trVect;	// world coordinates
				trVect.x = tm.tmx[0] * component.vect.x + tm.tmx[1] * component.vect.y + tm.tmx[2] * component.vect.z;
				trVect.y = tm.tmx[4] * component.vect.x + tm.tmx[5] * component.vect.y + tm.tmx[6] * component.vect.z;
				trVect.z = tm.tmx[8] * component.vect.x + tm.tmx[9] * component.vect.y + tm.tmx[10] * component.vect.z;
				WriteReport("          [%2d]  (%lf, %lf, %lf)",
					ivect,
					trVect.x, trVect.y, trVect.z);
			}
		}

	}
	return;
}		// DumpBody

// -----------------------------------------------------------------------------
// Dump a light source
// -----------------------------------------------------------------------------

void	DumpLight(Int32 ilght, bool detailed)
{
	API_Component3D 	component;
	GSErrCode			err;

	WriteReport("LIGHT #%d", ilght);

	BNZeroMemory(&component, sizeof(component));

	component.header.typeID = API_LghtID;
	component.header.index = ilght;
	err = ACAPI_3D_GetComponent(&component);
	if (err != NoError) {
		WriteReport("    ERROR");
		return;
	}

	if (!detailed)
		return;

	API_LghtType	&lght = component.lght;
	switch (lght.type) {
	case APILght_DistantID:
		WriteReport("  Distant");
		break;
	case APILght_DirectionID:
		WriteReport("  Direction");
		break;
	case APILght_SpotID:
		WriteReport("  Spot");
		break;
	case APILght_PointID:
		WriteReport("  Point");
		break;
	case APILght_SunID:
		WriteReport("  Sun");
		break;
	case APILght_EyeID:
		WriteReport("  Eye");
		break;
	}

	WriteReport("  status: %d", lght.status);
	WriteReport("  color : (%5.2f, %5.2f, %5.2f)", lght.lightRGB.f_red,
		lght.lightRGB.f_green,
		lght.lightRGB.f_blue);
	WriteReport("  pos   : (%5.2f, %5.2f, %5.2f)", lght.posx,
		lght.posy,
		lght.posz);
	WriteReport("  dir   : (%5.2f, %5.2f, %5.2f)", lght.dirx,
		lght.diry,
		lght.dirz);
	WriteReport("  radius: %5.2f", lght.radius);
	WriteReport("  cosa  : %5.2f", lght.cosa);
	WriteReport("  cosb  : %5.2f", lght.cosb);
	WriteReport("  afall : %5.2f", lght.afall);
	WriteReport("  dists : %5.2f, %5.2f", lght.dist1,
		lght.dist2);
	WriteReport("  dfall : %5.2f", lght.dfall);

	return;
}		// DumpLight


GSErrCode	Do_Dump3DModel(bool fromSaveAs)
{
	API_Coord	nullOffset = { 0.0, 0.0 };
	Int32		nbody, ibody, nlght, ilght;
	void		*origSight;
	bool		detailed;
	GSErrCode	err;

	WriteReport("# Get the actual 3D model");
	WriteReport("#   - the model will be not updated");

	WriteReport("3D model Dump");
	if (!fromSaveAs) {		// we must switch to the 3D Window's sight
		err = ACAPI_3D_SelectSight(NULL, &origSight);
		if (err != NoError) {
			ErrorBeep("Cannot switch to the 3D sight", err);
			return err;
		}
	}

	err = ACAPI_3D_GetNum(API_BodyID, &nbody);
	if (err != NoError) {
		ErrorBeep("Error in ACAPI_3D_GetNum", err);
		if (!fromSaveAs)
			ACAPI_3D_SelectSight(origSight, &origSight);
		return err;
	}

	detailed = true;
	for (ibody = 1; ibody <= nbody; ibody++) {
		// only the first 5 BODY-s are written in detailed mode
		if (ibody == 5 && !fromSaveAs)
			detailed = false;
		DumpBody(ibody, detailed, nullOffset);
	}

	err = ACAPI_3D_GetNum(API_LghtID, &nlght);
	if (err != NoError) {
		ErrorBeep("Error in ACAPI_3D_GetNum", err);
		if (!fromSaveAs)
			ACAPI_3D_SelectSight(origSight, &origSight);
		return err;
	}

	detailed = true;
	for (ilght = 1; ilght <= nlght; ilght++) {
		// only the first 5 BODY-s are written in detailed mode
		if (ilght == 5)
			detailed = false;
		DumpLight(ilght, detailed);
	}

	if (fromSaveAs) {
		WriteReport_Alert("The output is written into the report window instead of the selected file.");
		// adjust return code to delete the empty output file
		err = APIERR_CANCEL;
	}

	if (!fromSaveAs)
		ACAPI_3D_SelectSight(origSight, &origSight);

	WriteReport_End(err);

	return err;
}		// Do_Get3DModel
// -----------------------------------------------------------------------------
// Dump a body (BODY, VERT, EDGE)
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Dump element category value
// -----------------------------------------------------------------------------
void	Do_DumpElemCategoryValue(const API_ElemCategoryValue& elemCategoryValue)
{
	WriteReport("   %s   : %s (%s)", GS::UniString(elemCategoryValue.category.name).ToCStr().Get(), GS::UniString(elemCategoryValue.name).ToCStr().Get(), APIGuidToString(elemCategoryValue.guid).ToCStr().Get());
}

// -----------------------------------------------------------------------------
// Dump element's categories
// -----------------------------------------------------------------------------
void	Do_DumpElemCategories(const API_Guid& elemGuid, const API_ElemTypeID& typeID, const API_ElemVariationID& variationID, bool dumpDefaults)
{
	GSErrCode			err = NoError;

	GS::Array<API_ElemCategory> categoryList;
	ACAPI_Database(APIDb_GetElementCategoriesID, &categoryList);

	categoryList.Enumerate([&](const API_ElemCategory& category) {
		if (category.categoryID != API_ElemCategory_BRI) {
			API_ElemCategoryValue	elemCategoryValue;

			if (dumpDefaults) {
				err = ACAPI_Element_GetCategoryValueDefault(typeID, variationID, category, &elemCategoryValue);
				if (err == NoError)
					Do_DumpElemCategoryValue(elemCategoryValue);
				else
					ErrorBeep("ACAPI_Element_GetCategoryValueDefault ()", err);
			}
			else {
				err = ACAPI_Element_GetCategoryValue(elemGuid, category, &elemCategoryValue);
				if (err == NoError)
					Do_DumpElemCategoryValue(elemCategoryValue);
				else
					ErrorBeep("ACAPI_Element_GetCategoryValue ()", err);
			}
		}
	});
}

// -----------------------------------------------------------------------------
// Dump basic information of the clicked element into the Report Window
//   - check in Teamwork mode also
// -----------------------------------------------------------------------------
void	Do_DumpAllElement()
{
	//API_ElemTypeID	typeID;
	//API_Guid		guid;
	//API_Element		element;
	//API_ProjectInfo	projectInfo;
	//API_SharingInfo	sharingInfo;
	GSErrCode		err;

	//if (!ClickAnElem("Click an elem to get info for", API_ZombieElemID, NULL, &typeID, &guid)) {
	//	WriteReport_Alert("No element was clicked");
	//	return;
	//}

	WriteReport("### Start dumping all element data... ###");
	//BNZeroMemory(&element, sizeof(API_Element));
	//element.header.guid = guid;

	GS::Array<API_Guid> elemList;
	API_Element tempElement;
	ACAPI_Element_GetElemList(API_ZombieElemID, &elemList);
	for (GS::Array<API_Guid>::ConstIterator it = elemList.Enumerate(); it != NULL; ++it) {
		BNZeroMemory(&tempElement, sizeof(API_Element));
		tempElement.header.guid = *it;
		err = ACAPI_Element_Get(&tempElement);

		if (err != NoError) { //Cannot find element
			ErrorBeep("ACAPI_Element_Get", err);
			return;
		}
		GS::UniString	typeName;
		ACAPI_Goodies(APIAny_GetElemTypeNameID, (void *)(GS::IntPtr) tempElement.header.typeID, &typeName);

		double detailAngle = tempElement.detail.angle;
		API_Coord detailPos = tempElement.detail.pos;
		
		API_Coord3D coord3d = tempElement.dimension.refC3D;
		API_Coord dimenRef = tempElement.dimension.refC;
		API_Vector3D dimenDir3D = tempElement.dimension.direction3D;
		API_Vector  dimenDir = tempElement.dimension.direction;
		
		API_Coord anDimBaseLoc = tempElement.angleDimension.base->loc;
		API_Coord3D anDimBaseLoc3D = tempElement.angleDimension.base->loc3D;

		API_Coord radDimBaseLoc = tempElement.radialDimension.base.loc;
		API_Coord3D radDimBaseLoc3D = tempElement.radialDimension.base.loc3D;
		
		WriteReport("----------------------------------------------------------");
		WriteReport("Floor:%d GUID:%s(%s) Type:%s(%03d)", 
			tempElement.header.floorInd, 
			APIGuidToString(tempElement.header.guid).ToCStr().Get(),
			ElemID_To_Name(tempElement.header.typeID),
			static_cast<const char *> (typeName.ToCStr()), tempElement.header.typeID);
		if(tempElement.header.typeID == 6)
		{
			API_Coord objPos = tempElement.object.pos;
			double objAngle = tempElement.object.angle;
			WriteReport("Object: Angle:%10.7f Pos: x:%10.7f y:%10.7f", objAngle, objPos.x, objPos.y);
		}
		WriteReport("Detail: Angle:%10.7f Pos: x:%10.7f y:%10.7f", detailAngle, detailPos.x, detailPos.y);

		WriteReport("Dimension->RefC3D x: %10.7f y: %10.7f z: %10.7f", coord3d.x, coord3d.y, coord3d.z);
		WriteReport("Dimension->RefC   x: %10.7f y: %10.7f", dimenRef.x, dimenRef.y);
		WriteReport("Dimension->Direction3D   x: %10.7f y: %10.7f z: %10.7f", dimenDir3D.x, dimenDir3D.y, dimenDir3D.z);
		WriteReport("Dimension->Direction     x: %10.7f y: %10.7f", dimenDir.x, dimenDir.y);

		WriteReport("angleDimension.base->Loc   x: %10.7f y: %10.7f", anDimBaseLoc.x, anDimBaseLoc.y);
		WriteReport("angleDimension.base->Loc3D x: %10.7f y: %10.7f z: %10.7f", anDimBaseLoc3D.x, anDimBaseLoc3D.y, anDimBaseLoc3D.z);

		WriteReport("radDimBaseLoc.base->Loc   x: %10.7f y: %10.7f", radDimBaseLoc.x, radDimBaseLoc.y);
		WriteReport("radDimBaseLoc3D.base->Loc3D x: %10.7f y: %10.7f z: %10.7f", radDimBaseLoc3D.x, radDimBaseLoc3D.y, radDimBaseLoc3D.z);
	}
	WriteReport("### Dumping all element data finished ###");
/*
	err = ACAPI_Element_Get(&element);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_Get", err);
		return;
	}

	WriteReport("Dump: %s GUID:%s", ElemID_To_Name(typeID), APIGuidToString(element.header.guid).ToCStr().Get());

	BNZeroMemory(&projectInfo, sizeof(API_ProjectInfo));
	ACAPI_Environment(APIEnv_ProjectID, &projectInfo, NULL);
	if (projectInfo.location != NULL)
		delete projectInfo.location;
	if (projectInfo.location_team != NULL)
		delete projectInfo.location_team;

	BNZeroMemory(&sharingInfo, sizeof(API_SharingInfo));
	if (projectInfo.userId != element.header.userId) {
		WriteReport("  out of my workspace");
		err = ACAPI_Environment(APIEnv_ProjectSharingID, &sharingInfo, NULL);
		if (err != NoError) {
			ErrorBeep("APIEnv_ProjectSharingID", err);
			return;
		}
	}

	GS::UniString	typeName, renovationStatusName, renovationFilterName;
	ACAPI_Goodies(APIAny_GetElemTypeNameID, (void *)(GS::IntPtr) element.header.typeID, &typeName);
	ACAPI_Goodies(APIAny_GetRenovationStatusNameID, (void *)(GS::IntPtr) element.header.renovationStatus, &renovationStatusName);
	ACAPI_Goodies(APIAny_GetRenovationFilterNameID, &element.header.renovationFilterGuid, &renovationFilterName);

	WriteReport("  typeID     : (%03d) %s", element.header.typeID, static_cast<const char *> (typeName.ToCStr()));
	WriteReport("  guid       : %s", APIGuidToString(element.header.guid).ToCStr().Get());
	WriteReport("  modiStamp  : %d", element.header.modiStamp);
	WriteReport("  groupGuid  : %s", APIGuidToString(element.header.groupGuid).ToCStr().Get());
	WriteReport("  floorInd   : %d", element.header.floorInd);
	WriteReport("  layer      : %d", element.header.layer);
	WriteReport("  hasMemo    : %d", element.header.hasMemo);
	WriteReport("  drwIndex   : %d", element.header.drwIndex);
	WriteReport("  userId     : %d", element.header.userId);
	WriteReport("  lockId     : %d", element.header.lockId);
	WriteReport("  renovation status name : %s", static_cast<const char *> (renovationStatusName.ToCStr()));
	WriteReport("  renovation filter name : %s", static_cast<const char *> (renovationFilterName.ToCStr()));
	WriteReport("  Categories :");
	Do_DumpElemCategories(element.header.guid, element.header.typeID, element.header.variationID, false);
	WriteReport("  Default categories :");
	Do_DumpElemCategories(element.header.guid, element.header.typeID, element.header.variationID, true);

	if (projectInfo.teamwork) {
		DumpOwner("owned by  :", &projectInfo, &sharingInfo, element.header.userId);
		DumpOwner("locked by :", &projectInfo, &sharingInfo, element.header.lockId);
	}

	if (sharingInfo.users != NULL)
		BMhKill(reinterpret_cast<GSHandle*>(&sharingInfo.users));

	renFiltGuid = element.header.renovationFilterGuid;

	ACAPI_KeepInMemory(true);*/

	return;
}		// Do_DumpElem

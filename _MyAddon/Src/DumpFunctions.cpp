
// -----------------------------------------------------------------------------
// Get the actual 3D model
// -----------------------------------------------------------------------------

#include <string>
#include <iostream>
#include "APIEnvir.h"
#include "ACAPinc.h"					// also includes APIdefs.h
#include "APICommon.h"
#include <curl/curl.h>

struct MemoryStruct {
	char *memory;
	size_t size;
};
static std::string DownloadedResponse;
static size_t WriteMemoryCallback(char *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userp;

	mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
	if (mem->memory == NULL) {
		/* out of memory! */
		printf("not enough memory (realloc returned NULL)\n");
		return 0;
	}

	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	DownloadedResponse = std::string(contents);
	return realsize;
}

void webTest()
{
	CURL *curl;
	CURLcode res;
	curl = curl_easy_init();
	if (curl) {
		struct curl_slist *headers = NULL; /* init to NULL is important */
		headers = curl_slist_append(headers, "Accept: application/json");
		headers = curl_slist_append(headers, "Content-Type: application/json");
		headers = curl_slist_append(headers, "charsets: utf-8");

		curl_easy_setopt(curl, CURLOPT_VERBOSE, headers);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:5000/api/values/newWeb");

		res = curl_easy_perform(curl);
		if (CURLE_OK == res)
		{
			char *ct;
			res = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &ct);
			if ((CURLE_OK == res) && ct)
				WriteReport_Alert(DownloadedResponse.c_str());
				//std::cout << DownloadedResponse;
		}
		curl_slist_free_all(headers); /* free the header list */
		curl_easy_cleanup(curl);
	}
	return;
}
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
std::string d2s(double input)
{
	return std::to_string(long double(input));
}

std::string GetObjectInfoStr(API_ObjectType input)
{
	API_Coord pos = input.pos;
	API_Coord offset = input.offset;
	double angle = input.angle * 180 / PI;
	double level = input.level; //height from floor
	int modelId = input.libInd; //3D model index in 3D model object library
	std::string str = "Object [Model:" + d2s(modelId) + "]\n" +
		"Angle: " + d2s(angle) + "\n" +
		"Height from floorlevel: " + d2s(level) + "\n" +
		"Origin Position [x:" + d2s(pos.x) + " y:" + d2s(pos.y) + "]\n" +
		"Offset from Origin [x:" + d2s(offset.x) + " y:" + d2s(offset.y) + "]";
	return str;
}

std::string GetWallInfoStr(API_WallType input)
{
	API_Coord begin_coord = input.begC;
	API_Coord end_coord = input.endC;
	double height = input.height;
	double angle = input.angle * 180 / PI;

	std::string str = "Wall: Angle: " + d2s(angle) + "\n" +
		"Height: " +d2s(height) + " Above: " + d2s(input.bottomOffset) + "\n" +
		"Being Position [x:" + d2s(begin_coord.x) + " y:" + d2s(begin_coord.y) + " thickness: "+ d2s(input.thickness) +"]\n" +
		"End Position [x:" + d2s(end_coord.x) + " y:" + d2s(end_coord.y) + " thickness: " + d2s(input.thickness1) + "]";
	return str;
}

std::string GetSlabInfoStr(API_SlabType input)
{
	double thickness = input.thickness;
	double level = input.level; //height from floor
	
	std::string str = "Slab - Height from floorlevel: " + d2s(level) + "\n" +
		"Thinkness:" + d2s(thickness);
	return str;
}

std::string GetBeamInfoStr(API_BeamType input)
{
	API_Coord begin_coord = input.begC;
	API_Coord end_coord = input.endC;
	double offset = input.offset;
	double width = input.width;
	double height = input.height;
	double level = input.level; //height from floor
	std::string str = "Beam From:[x:" + d2s(begin_coord.x) + " y:" + d2s(begin_coord.y) + "]" + 
		"To:[x:" + d2s(end_coord.x) + " y:" + d2s(end_coord.y) + "]\n" +
		"Height from floorlevel: " + d2s(level) + "\n" +
		"Width: " + d2s(width) + " Height:" + d2s(height) + " Offset:" + d2s(offset);
	return str;
}
GS::UniString AppendValue(API_AddParID typeID, GS::uchar_t* uStr, double real) {
	switch (typeID) {
	case APIParT_CString:
	case APIParT_Title:
		return GS::UniString(uStr) + GS::UniString("\n");
	default:
		return GS::UniString::Printf("%f", real) + GS::UniString("\n");
	}
}
void DuplicateSlab(API_Element& old_element)
{
	ACAPI_CallUndoableCommand("Duplicate Wall",
		[&]() -> GSErrCode {

		API_Element newElement;
		BNZeroMemory(&newElement, sizeof(API_Element));
		API_ElementMemo newMemo;
		BNZeroMemory(&newMemo, sizeof(API_ElementMemo));

		//API_ElemURLRef oldURLRef;
		//BNZeroMemory(&oldURLRef, sizeof(API_ElemURLRef));
		//CHTruncate("http://www.google.com", oldURLRef.urlLink, sizeof(oldURLRef.urlLink));
		//CHTruncate("Google", oldURLRef.description, sizeof(oldURLRef.description));
		//GSErrCode err = ACAPI_Database(APIDb_SetElementURLRefID, &old_element.header, &oldURLRef);
		//if (err == NoError)
		//	WriteReport("Set Url Success");
		//else
		//	WriteReport("Set Url Failed");

		newElement.header.typeID = old_element.header.typeID;
		newElement.header.variationID = old_element.header.variationID;
		ACAPI_Element_GetDefaults(&newElement, &newMemo);

		newElement.header.floorInd = old_element.header.floorInd + 1;
		newElement.header.layer = old_element.header.layer;
		newElement.header.drwIndex = old_element.header.drwIndex;

		newElement.detail.pos.x = old_element.detail.pos.x;
		newElement.detail.pos.y = old_element.detail.pos.y;
		newElement.detail.angle = old_element.detail.angle;
		newElement.detail.pen = old_element.detail.pen;

		API_SlabType& oo = old_element.slab;
		API_SlabType& no = newElement.slab;
		no.thickness = oo.thickness;
		//no.level = oo.level;
		no.offsetFromTop = oo.offsetFromTop;
		no.referencePlaneLocation = oo.referencePlaneLocation;
		no.poly = oo.poly;
		//newElement.slab = old_element.slab;

		if (old_element.header.hasMemo) {
			API_ElementMemo oldMemo;
			BNZeroMemory(&oldMemo, sizeof(API_ElementMemo));
			                 
			GSErrCode err1 = ACAPI_Element_GetMemo(old_element.header.guid, &oldMemo, APIMemoMask_AddPars);
			if (err1 != NoError) {
				WriteReport_Alert("API_ElementMemo couldn't be retrieved!");
				return err1;
			}

			//auto& addPars = oldMemo.params;
			//auto& newPars = newMemo.params;

			//newMemo.params = newPars;
			//newMemo.additionalPolyContourIDs = oldMemo.additionalPolyContourIDs;
			//newMemo.additionalPolyCoords = oldMemo.additionalPolyCoords;
			//newMemo.additionalPolyEdgeIDs = oldMemo.additionalPolyEdgeIDs;
			//newMemo.additionalPolyParcs = oldMemo.additionalPolyParcs;
			//newMemo.additionalPolyPends = oldMemo.additionalPolyPends;
			//newMemo.additionalPolyVertexIDs = 0;
			//newMemo.coords = oldMemo.coords;

			GSErrCode err2 = ACAPI_Element_Create(&newElement, &oldMemo);
			if (err2 != NoError)
			{
				WriteReport("Error code when create %d", err2);
			}
			                                       
			ACAPI_DisposeElemMemoHdls(&oldMemo); //important
			return err2;
		}
		ACAPI_DisposeElemMemoHdls(&newMemo); //important
	});
}
void DuplicateWall(API_Element& old_element)
{
	ACAPI_CallUndoableCommand("Duplicate Wall",
		[&]() -> GSErrCode {

		API_Element newElement;
		BNZeroMemory(&newElement, sizeof(API_Element));
		API_ElementMemo newMemo;
		BNZeroMemory(&newMemo, sizeof(API_ElementMemo));

		GSErrCode err;
		newElement.header.typeID = old_element.header.typeID;			WriteReport("header.typeID: %d", old_element.header.typeID);
		newElement.header.variationID = old_element.header.variationID; WriteReport("header.variationID: %d", old_element.header.variationID);
		ACAPI_Element_GetDefaults(&newElement, &newMemo);				
		newElement.header.floorInd = old_element.header.floorInd + 1;	WriteReport("header.floorInd: %d", old_element.header.floorInd);
		newElement.header.layer = old_element.header.layer;				WriteReport("header.layer: %d", old_element.header.layer);
		newElement.header.drwIndex = old_element.header.drwIndex;		WriteReport("header.drwIndex: %d", old_element.header.drwIndex);

		newElement.detail.pos.x = old_element.detail.pos.x;				WriteReport("detail.pos.x: %f", old_element.detail.pos.x);
		newElement.detail.pos.y = old_element.detail.pos.y;				WriteReport("detail.pos.y: %f", old_element.detail.pos.y);
		newElement.detail.angle = old_element.detail.angle;				WriteReport("detail.angle: %f", old_element.detail.angle);
		newElement.detail.pen = old_element.detail.pen;					WriteReport("detail.pen: %d", old_element.detail.pen);

		API_WallType& oo = old_element.wall;

		// Here was a mistake in your code also, I fixed it:
		API_WallType& no = newElement.wall;
		no.begC = oo.begC;					WriteReport("begC: %f %f", oo.begC.x, oo.begC.y);
		no.thickness = oo.thickness;		WriteReport("thickness: %f", oo.thickness);
		no.endC = oo.endC;					WriteReport("endC: %f %f", oo.endC.x, oo.endC.y);
		no.thickness1 = oo.thickness1;		WriteReport("thickness1: %f", oo.thickness1);
		no.angle = oo.angle;				WriteReport("angle: %f", oo.angle);
		no.offset = oo.offset;				WriteReport("offset: %f", oo.offset);
		no.bottomOffset = oo.bottomOffset;	WriteReport("bottomOffset: %f", oo.bottomOffset);
		no.height = oo.height;				WriteReport("height: %f", oo.height);
		no.buildingMaterial = oo.buildingMaterial; WriteReport("Material: %d", oo.buildingMaterial);
		API_ElementMemo oldMemo;
		BNZeroMemory(&oldMemo, sizeof(API_ElementMemo));

		err = ACAPI_Element_GetMemo(old_element.header.guid, &oldMemo, APIMemoMask_AddPars);
		if (err != NoError) {
			WriteReport_Alert("API_ElementMemo couldn't be retrieved!");
			return err;
		}

		//auto& addPars = oldMemo.params;
		//auto& newPars = newMemo.params;
		//if (addPars != nullptr && *addPars != nullptr) {
		//	auto addParNum = BMGetHandleSize((GSHandle)addPars) / sizeof(API_AddParType);
		//	GS::UniString outVal = "";
		//	for (auto i = 0; i < addParNum; ++i)
		//	{
		//		API_AddParType oldParam = (*addPars)[i];
		//		API_AddParType newParam = (*newPars)[i];
		//		outVal += GS::UniString(oldParam.name) + " -> ";
		//		if (oldParam.typeMod == API_ParSimple)
		//		{
		//			outVal += AppendValue(oldParam.typeID, oldParam.value.uStr, oldParam.value.real);
		//		}
		//		else {
		//			for (auto i1 = 1; i1 <= (*addPars)[i].dim1; ++i1) {
		//				for (auto i2 = 1; i2 <= (*addPars)[i].dim2; ++i2) {
		//					AppendValue((*addPars)[i].typeID, (*addPars)[i].value.uStr, (*addPars)[i].value.real);
		//				}
		//			}
		//		}
		//		newParam.value = oldParam.value;
		//		DBPrintf(outVal.ToCStr());
		//	}
		//}
		//newMemo.params = newPars;
		err = ACAPI_Element_Create(&newElement, &oldMemo);
		if (err != NoError)
		{
			WriteReport("Error code when create %d", err);
		}

		ACAPI_DisposeElemMemoHdls(&oldMemo); //important
		ACAPI_DisposeElemMemoHdls(&newMemo); //important
		return err;
	});
}
void Do_Get_IFC(API_Element& element) {
	GS::Array<API_IFCProperty>     properties;
	ACAPI_Element_GetIFCProperties(element.header.guid, false, &properties);
	for (unsigned int i = 0; i < properties.GetSize(); i++)
	{
		API_IFCProperty prop = properties.Get(i);
		WriteReport(GS::UniString("IFC Prop: Name: " + prop.head.propertyName).ToCStr());
		//std::string str = GS::UniString("IFC Prop: Name: " + prop.head.propertyName);
	}
}
void Do_Set_IFC(API_Element& element)
{
	ACAPI_CallUndoableCommand(GS::UniString("IFC Testing"), [&element](void)->GSErrCode {
		API_IFCPropertyAnyValue anyValue;
		BNZeroMemory(&anyValue, sizeof(API_IFCPropertyAnyValue));
		anyValue.primitiveType = API_IFCPropertyAnyValueStringType;
		anyValue.stringValue = GS::UniString("http://www.google.com");

		API_IFCPropertyValue propertyValue;
		BNZeroMemory(&propertyValue, sizeof(API_IFCPropertyValue));
		propertyValue.value = anyValue;
		propertyValue.valueType = GS::UniString("IfcLabel");

		API_IFCPropertySingleValue singleValue;
		BNZeroMemory(&singleValue, sizeof(API_IFCPropertySingleValue));
		singleValue.nominalValue = propertyValue;

		API_IFCPropertyHead ifcHead;
		BNZeroMemory(&ifcHead, sizeof(API_IFCPropertyHead));

		ifcHead.propertySetName = GS::UniString("Pset_VH_Extra");
		ifcHead.propertyName = GS::UniString("url");
		ifcHead.propertyDescription = GS::UniString("test description");
		ifcHead.propertyType = API_IFCPropertySingleValueType;

		API_IFCProperty property;
		BNZeroMemory(&property, sizeof(API_IFCProperty));
		property.head = ifcHead;
		property.singleValue = singleValue;

		GSErrCode err = ACAPI_Element_SetIFCProperty(element.header.guid, property);
		if (err != NoError)
		{
			WriteReport("Error code when IFC %d", err);
		}
		return err;
	});
}

void DuplicateElement(API_Element& old_element)
{
	ACAPI_CallUndoableCommand("Duplicate Object",
		[&]() -> GSErrCode {

		API_Element newElement;
		BNZeroMemory(&newElement, sizeof(API_Element));
		API_ElementMemo newMemo;
		BNZeroMemory(&newMemo, sizeof(API_ElementMemo));

		API_ElemURLRef oldURLRef;
		BNZeroMemory(&oldURLRef, sizeof(API_ElemURLRef));
		CHTruncate("http://www.google.com", oldURLRef.urlLink, sizeof(oldURLRef.urlLink));
		CHTruncate("Google", oldURLRef.description, sizeof(oldURLRef.description));

		GSErrCode err = ACAPI_Database(APIDb_SetElementURLRefID, &old_element.header, &oldURLRef);
		if (err == NoError)
			WriteReport("Set Url Success");
		else
			WriteReport("Set Url Failed");

		newElement.header.typeID = old_element.header.typeID;
		WriteReport("header.typeID: %d", old_element.header.typeID);
		newElement.header.variationID = old_element.header.variationID;
		WriteReport("header.variationID: %d", old_element.header.variationID);

		ACAPI_Element_GetDefaults(&newElement, &newMemo);
		newElement.header.floorInd = old_element.header.floorInd+1;
		WriteReport("header.floorInd: %d", old_element.header.floorInd+1);
		newElement.header.layer = old_element.header.layer;
		WriteReport("header.layer: %d", old_element.header.layer);
		newElement.header.drwIndex = old_element.header.drwIndex;
		WriteReport("header.drwIndex: %d", old_element.header.drwIndex);

		newElement.detail.pos.x = old_element.detail.pos.x;
		WriteReport("detail.pos.x: %f", old_element.detail.pos.x);
		newElement.detail.pos.y = old_element.detail.pos.y;
		WriteReport("detail.pos.y: %f", old_element.detail.pos.y);
		newElement.detail.angle = old_element.detail.angle;
		WriteReport("detail.angle: %f", old_element.detail.angle);
		newElement.detail.pen = old_element.detail.pen;
		WriteReport("detail.pen: %d", old_element.detail.pen);

		API_ObjectType& oo = old_element.object;

		// Here was a mistake in your code also, I fixed it:
		API_ObjectType& no = newElement.object;
		no.pos = oo.pos;							WriteReport("pos: %f %f", oo.pos.x, oo.pos.y);
		no.pen = oo.pen;							WriteReport("pen: %d", oo.pen);
		no.angle = oo.angle;						WriteReport("angle: %f", oo.angle);
		no.level = oo.level;						WriteReport("level: %f", oo.level);
		no.libInd = oo.libInd;						WriteReport("libInd: %d", oo.libInd);
		no.offset = oo.offset;						WriteReport("offset: %f", oo.offset);
		no.xRatio = oo.xRatio;						WriteReport("xRatio: %f", oo.xRatio);
		no.yRatio = oo.yRatio;						WriteReport("yRatio: %f", oo.yRatio);
		no.ltypeInd = oo.ltypeInd;					WriteReport("ltypeInd: %d", oo.ltypeInd);
		no.sectFill = oo.sectFill;					WriteReport("sectFill: %d", oo.sectFill);
		no.reflected = oo.reflected;				WriteReport("reflected: %d", oo.reflected);
		no.sectBGPen = oo.sectBGPen;				WriteReport("sectBGPen: %d", oo.sectBGPen);
		no.sectFillPen = oo.sectFillPen;			WriteReport("sectFillPen: %d", oo.sectFillPen);
		no.sectContPen = oo.sectContPen;			WriteReport("sectContPen: %d", oo.sectContPen);
		no.useObjSectAttrs = oo.useObjSectAttrs;	WriteReport("useObjSectAttrs: %d", oo.useObjSectAttrs);
		no.useObjMaterials = oo.useObjMaterials;	WriteReport("useObjMaterials: %d", oo.useObjMaterials);

		//WriteReport("", newMemo.)
		API_ElementMemo oldMemo;
		BNZeroMemory(&oldMemo, sizeof(API_ElementMemo));
		WriteReport("additionalPolyContourIDs: %d", oldMemo.additionalPolyContourIDs);
		WriteReport("additionalPolyCoords: %d", oldMemo.additionalPolyCoords);
		WriteReport("additionalPolyEdgeIDs: %d", oldMemo.additionalPolyEdgeIDs);
		WriteReport("additionalPolyParcs: %d", oldMemo.additionalPolyParcs);
		WriteReport("additionalPolyPends: %d", oldMemo.additionalPolyPends);
		WriteReport("additionalPolyVertexIDs: %d", oldMemo.additionalPolyVertexIDs);
		if(oldMemo.beamHoles!=nullptr)
			WriteReport("beamHoles:", oldMemo.beamHoles);
		if (oldMemo.bezierDirs != nullptr)
			WriteReport("bezierDirs:", oldMemo.bezierDirs);
		WriteReport("contourIDs: %d", oldMemo.contourIDs);
		if (oldMemo.coords != nullptr)
			WriteReport("coords:", oldMemo.coords);
		if (oldMemo.customOrigProfile != nullptr)
			WriteReport("customOrigProfile:", oldMemo.customOrigProfile);
		if (oldMemo.customParams != nullptr)
			WriteReport("customParams:", oldMemo.customParams);
		if (oldMemo.customParams != nullptr)
			WriteReport("cWallAccessories:", oldMemo.cWallAccessories);
		if (oldMemo.customParams != nullptr)
			WriteReport("cWallFrames:", oldMemo.cWallFrames);
		if (oldMemo.customParams != nullptr)
			WriteReport("cWallJunctions:", oldMemo.cWallJunctions);
		if (oldMemo.customParams != nullptr)
			WriteReport("cWallPanels:", oldMemo.cWallPanels);
		if (oldMemo.customParams != nullptr)
			WriteReport("cWallSegments:", oldMemo.cWallSegments);
		if (oldMemo.customParams != nullptr)
			WriteReport("cWSegContour:", oldMemo.cWSegContour);
		if (oldMemo.customParams != nullptr)
			WriteReport("cWSegGridMesh:", oldMemo.cWSegGridMesh);

		WriteReport("cWSegPanelPattern: %s", oldMemo.cWSegPanelPattern);
		WriteReport("cWSegPrimaryPattern: %f", oldMemo.cWSegPrimaryPattern);
		WriteReport("cWSegSecondaryPattern: %f", oldMemo.cWSegSecondaryPattern);
		if (oldMemo.customParams != nullptr)
		WriteReport("dimElems:", oldMemo.dimElems);
		if (oldMemo.drawingData != nullptr)
			WriteReport("drawingData:", oldMemo.drawingData);
		WriteReport("edgeIDs: %d", oldMemo.edgeIDs);
		if (oldMemo.edgeTrims != nullptr)
			WriteReport("edgeTrims:", oldMemo.edgeTrims);
		WriteReport("elemInfoString: %s", oldMemo.elemInfoString);
		if (oldMemo.gables != nullptr)
			WriteReport("gables:", oldMemo.gables);
		if (oldMemo.linkList != nullptr)
			WriteReport("linkList:", oldMemo.linkList);
		if (oldMemo.meshLevelCoords != nullptr)
			WriteReport("meshLevelCoords:", oldMemo.meshLevelCoords);
		WriteReport("meshLevelEnds: %d", oldMemo.meshLevelEnds);
		WriteReport("meshPolyZ: %f", oldMemo.meshPolyZ);
		if (oldMemo.morphBody != nullptr)
			WriteReport("morphBody:", oldMemo.morphBody);
		if (oldMemo.morphMaterialMapTable != nullptr)
			WriteReport("morphMaterialMapTable:", oldMemo.morphMaterialMapTable);
		if (oldMemo.paragraphs != nullptr)
			WriteReport("paragraphs:", oldMemo.paragraphs);
		if (oldMemo.parcs != nullptr)
			WriteReport("parcs:", oldMemo.parcs);
		WriteReport("pends: %d", oldMemo.pends);
		WriteReport("pictHdl: %s", oldMemo.pictHdl);
		if (oldMemo.pivotPolyEdges != nullptr)
			WriteReport("pivotPolyEdges:", oldMemo.pivotPolyEdges);
		if (oldMemo.roofEdgeTypes != nullptr)
			WriteReport("roofEdgeTypes:", oldMemo.roofEdgeTypes);
		if (oldMemo.sectionLineCoords != nullptr)
			WriteReport("sectionLineCoords:", oldMemo.sectionLineCoords);
		if (oldMemo.segments != nullptr)
			WriteReport("segments:", oldMemo.segments);
		if (oldMemo.shellContours != nullptr)
			WriteReport("shellContours:", oldMemo.shellContours);
		if (oldMemo.shellShapes != nullptr)
			WriteReport("shellShapes:", oldMemo.shellShapes);
		if (oldMemo.sideMaterials != nullptr)
			WriteReport("sideMaterials:", oldMemo.sideMaterials);
		if (oldMemo.stretchedProfile != nullptr)
			WriteReport("stretchedProfile:", oldMemo.stretchedProfile);
		WriteReport("textContent: %s", oldMemo.textContent);
		WriteReport("textLineStarts: %d", oldMemo.textLineStarts);
		WriteReport("vertexIDs: %d", oldMemo.vertexIDs);
		WriteReport("wallDoors: %s", oldMemo.wallDoors);
		WriteReport("wallWindows: %s", oldMemo.wallWindows);

		err = ACAPI_Element_GetMemo(old_element.header.guid, &oldMemo, APIMemoMask_AddPars);
		if (err != NoError) {
			WriteReport_Alert("API_ElementMemo couldn't be retrieved!");
			return err;
		}
		//
		auto& addPars = oldMemo.params;
		//auto& newPars = newMemo.params;
		if (addPars != nullptr && *addPars != nullptr) {
			auto addParNum = BMGetHandleSize((GSHandle)addPars) / sizeof(API_AddParType);
			GS::UniString outVal = "";
			for (auto i = 0; i < addParNum; ++i)
			{
				API_AddParType oldParam = (*addPars)[i];
				//API_AddParType newParam = (*newPars)[i];
				outVal += GS::UniString(oldParam.name) + " -> ";
				if (oldParam.typeMod == API_ParSimple) 
				{
					outVal += AppendValue(oldParam.typeID, oldParam.value.uStr, oldParam.value.real);
				}
				//else {
				//	for (auto i1 = 1; i1 <= (*addPars)[i].dim1; ++i1) {
				//		for (auto i2 = 1; i2 <= (*addPars)[i].dim2; ++i2) {
				//			AppendValue((*addPars)[i].typeID, (*addPars)[i].value.uStr, (*addPars)[i].value.real);
				//		}
				//	}
				//}
				//newParam.value = oldParam.value;
				DBPrintf(outVal.ToCStr());
			}
		}
		//newMemo.params = newPars;
		err = ACAPI_Element_Create(&newElement, &oldMemo); 
		if (err != NoError)
		{
			WriteReport("Error code when create %d", err);
		}
		Do_Set_IFC(newElement);
		Do_Get_IFC(newElement);
		ACAPI_DisposeElemMemoHdls(&oldMemo); //important
		//ACAPI_DisposeElemMemoHdls(&newMemo); //important
		return err;
	});
}


void GetLibPartData(void) {
	API_Element object;
	BNZeroMemory(&object, sizeof(API_Element));
	if (!ClickAnElem("Click the Element", API_ZombieElemID, nullptr, &object.header.typeID, &object.header.guid)) {
		WriteReport_Alert("Clicked object is not an element!");
		return;
	}

	if (ACAPI_Element_Get(&object) != NoError) {
		WriteReport_Alert("API_Element couldn't be retrieved!");
		return;
	}

	API_ElementMemo memo;
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	if (ACAPI_Element_GetMemo(object.header.guid, &memo, APIMemoMask_AddPars) != NoError) {
		WriteReport_Alert("API_ElementMemo couldn't be retrieved!");
		return;
	}

	auto& addPars = memo.params;
	if (addPars != nullptr && *addPars != nullptr) {
		auto addParNum = BMGetHandleSize((GSHandle)addPars) / sizeof(API_AddParType);
		for (auto i = 0; i < addParNum; ++i) {
			auto outVal = GS::UniString((*addPars)[i].name);
			outVal += " -> ";

			auto AppendValue = [&outVal](API_AddParID typeID, GS::uchar_t* uStr, double real) {
				switch (typeID) {
				case APIParT_CString:
				case APIParT_Title:
					outVal += GS::UniString(uStr);
					outVal += GS::UniString("\n");
					break;
				default:
					outVal += GS::UniString::Printf("%f", real);
					outVal += GS::UniString("\n");
					break;
				}
			};

			if ((*addPars)[i].typeMod == API_ParSimple) {
				AppendValue((*addPars)[i].typeID, (*addPars)[i].value.uStr, (*addPars)[i].value.real);
			}
			else {
				for (auto i1 = 1; i1 <= (*addPars)[i].dim1; ++i1) {
					for (auto i2 = 1; i2 <= (*addPars)[i].dim2; ++i2) {
						AppendValue((*addPars)[i].typeID, (*addPars)[i].value.uStr, (*addPars)[i].value.real);
					}
				}
			}
			DBPrintf(outVal.ToCStr());
		}
	}

	ACAPI_DisposeElemMemoHdls(&memo);
}

void Do_CreateWall(double x1, double y1, double t1,
	double x2, double y2, double t2, double o1, double o2, double angle, double height)
{
	WriteReport("### Create wall start... ###");
	ACAPI_CallUndoableCommand("Create Wall",
		[&]() -> GSErrCode {

		API_Element wallElement;	BNZeroMemory(&wallElement, sizeof(API_Element));
		API_ElementMemo wallMemo;	BNZeroMemory(&wallMemo, sizeof(API_ElementMemo));
		API_Elem_Head header;		BNZeroMemory(&header, sizeof(API_Elem_Head));
		GSErrCode err;

		header.typeID = API_WallID;
		header.variationID = APIVarId_Generic;
		wallElement.header = header;
		err = ACAPI_Element_GetDefaults(&wallElement, &wallMemo);
		if (err != NoError) {
			WriteReport("Error when getting defaults");
			return err;
		}
		header.floorInd = -1;
		header.layer = 33;
		header.drwIndex = 7;

		API_WallType& wall = wallElement.wall;
		API_Coord begC;				BNZeroMemory(&begC, sizeof(API_Coord));
		API_Coord endC;				BNZeroMemory(&endC, sizeof(API_Coord));

		begC.x = x1;
		begC.y = y1;

		endC.x = x2;
		endC.y = y2;

		wall.begC = begC;
		wall.endC = endC;
		wall.thickness = t1;
		wall.thickness1 = t2;
		wall.angle = angle;
		wall.offset = o1;
		wall.bottomOffset = o2;
		wall.height = height;
		wall.buildingMaterial = 59;

		err = ACAPI_Element_Create(&wallElement, &wallMemo);
		if (err != NoError)
			WriteReport("Error when create wall %d", err);
		Do_Set_IFC(wallElement);
		Do_Get_IFC(wallElement);
		ACAPI_DisposeElemMemoHdls(&wallMemo); //important
		return err;
	});
	WriteReport("### Create wall finished... ###");
}

void Do_CreatePipesAndWalls()
{
	GSErrCode		err;
	WriteReport("### Trying to create pipe... ###");
	ACAPI_CallUndoableCommand("Create Pipe",
	[&]() -> GSErrCode {
		API_Element pipeElement;		BNZeroMemory(&pipeElement, sizeof(API_Element));
		API_ElementMemo pipeMemo;		BNZeroMemory(&pipeMemo, sizeof(API_ElementMemo));
		API_Elem_Head header;			BNZeroMemory(&header, sizeof(API_Elem_Head));
		
		header.typeID = API_ObjectID;
		//header.variationID = APIVarId_Generic;
		header.variationID = API_ElemVariationID('PRS ');
		pipeElement.header = header;
		err = ACAPI_Element_GetDefaults(&pipeElement, &pipeMemo);
		if (err != NoError) {
			WriteReport("Error getting defaults");
			ACAPI_DisposeElemMemoHdls(&pipeMemo); //important
			return err;
		}

		header.floorInd = 0;
		header.layer = 19;
		header.drwIndex = 8;
		
		API_DetailType& detail = pipeElement.detail;
		//BNZeroMemory(&detail, sizeof(API_DetailType));
		detail.pen = 3;
		detail.angle = 0;

		API_ObjectType& object = pipeElement.object;
		//BNZeroMemory(&object, sizeof(API_ObjectType));
		object.pos.x = 2.372636;
		object.pos.y = 6.669568;
		object.pen = 3;
		object.angle = 0;
		object.level = 1.5;
		object.libInd = 4407;
		object.offset.x = 0;
		object.offset.y = 0;
		object.ltypeInd = 1;
		object.sectFill = 65;
		object.sectBGPen = 3;
		object.sectFillPen = 2;
		object. sectContPen = 1;
		
		err = ACAPI_Element_Create(&pipeElement, &pipeMemo);
		if (err != NoError)
		{
			WriteReport("Error code when create pipe %d", err);
			ACAPI_DisposeElemMemoHdls(&pipeMemo); //important
			return err;
		}
		Do_Set_IFC(pipeElement);
		Do_Get_IFC(pipeElement);
	});
	
	WriteReport("### Create pipe finished... ###");

	WriteReport("### Trying to create walls... ###");
	Do_CreateWall(2.472636, 7.369568,	0.1f, 1.472636, 7.369568, 0.1f, 0, 0, 0, 2.1);
	Do_CreateWall(1.472636, 6.369568,	0.1f, 2.472636, 6.369568, 0.1f, 0, 0, 0, 2.1);
	Do_CreateWall(2.472636, 6.369568,	0.1f, 2.472636, 7.369568, 0.1f, 0, 0, 0, 2.1);
	Do_CreateWall(1.472636, 7.369568,	0.1f, 1.472636, 6.369568, 0.1f, 0, 0, 0, 2.1);
	WriteReport("### Create walls finished... ###");
}

void Do_CopyObjects()
{
	GSErrCode		err;
	WriteReport("### Start dumping all object data... ###");

	GS::Array<API_Guid> elemList;
	API_Element tempElement;
	ACAPI_Element_GetElemList(API_ObjectID, &elemList);
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

		WriteReport("----------------------------------------------------------");
		WriteReport("Floor:%d GUID:%s(%s) Type:%s(%03d)",
			tempElement.header.floorInd,
			APIGuidToString(tempElement.header.guid).ToCStr().Get(),
			ElemID_To_Name(tempElement.header.typeID),
			static_cast<const char *> (typeName.ToCStr()), tempElement.header.typeID);

		WriteReport(GetObjectInfoStr(tempElement.object).c_str());
		DuplicateElement(tempElement);
		Do_Set_IFC(tempElement);
		Do_Get_IFC(tempElement);
	}
	WriteReport("### Dumping all object data finished ###");
}
void Do_CopyWalls()
{
	GSErrCode		err;
	WriteReport("### Start dumping all wall data... ###");

	GS::Array<API_Guid> elemList;
	API_Element tempElement;
	ACAPI_Element_GetElemList(API_WallID, &elemList);
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

		WriteReport("----------------------------------------------------------");
		WriteReport("Floor:%d GUID:%s(%s) Type:%s(%03d)",
			tempElement.header.floorInd,
			APIGuidToString(tempElement.header.guid).ToCStr().Get(),
			ElemID_To_Name(tempElement.header.typeID),
			static_cast<const char *> (typeName.ToCStr()), tempElement.header.typeID);

		WriteReport(GetWallInfoStr(tempElement.wall).c_str());
		DuplicateWall(tempElement);
	}
	WriteReport("### Dumping all wall data finished ###");
}
void Do_CopySlabs()
{
	GSErrCode		err;
	WriteReport("### Start dumping all slab data... ###");

	GS::Array<API_Guid> elemList;
	API_Element tempElement;
	ACAPI_Element_GetElemList(API_SlabID, &elemList);
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

		WriteReport("----------------------------------------------------------");
		WriteReport("Floor:%d GUID:%s(%s) Type:%s(%03d)",
			tempElement.header.floorInd,
			APIGuidToString(tempElement.header.guid).ToCStr().Get(),
			ElemID_To_Name(tempElement.header.typeID),
			static_cast<const char *> (typeName.ToCStr()), tempElement.header.typeID);

		WriteReport(GetWallInfoStr(tempElement.wall).c_str());
		DuplicateSlab(tempElement);
	}
	WriteReport("### Dumping all slab data finished ###");
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

		//double detailAngle = tempElement.detail.angle;
		//API_Coord detailPos = tempElement.detail.pos;
		//
		//API_Coord3D coord3d = tempElement.dimension.refC3D;
		//API_Coord dimenRef = tempElement.dimension.refC;
		//API_Vector3D dimenDir3D = tempElement.dimension.direction3D;
		//API_Vector  dimenDir = tempElement.dimension.direction;
		//
		//API_Coord anDimBaseLoc = tempElement.angleDimension.base->loc;
		//API_Coord3D anDimBaseLoc3D = tempElement.angleDimension.base->loc3D;
		//API_Coord radDimBaseLoc = tempElement.radialDimension.base.loc;
		//API_Coord3D radDimBaseLoc3D = tempElement.radialDimension.base.loc3D;
		
		WriteReport("----------------------------------------------------------");
		WriteReport("Floor:%d GUID:%s(%s) Type:%s(%03d)", 
			tempElement.header.floorInd, 
			APIGuidToString(tempElement.header.guid).ToCStr().Get(),
			ElemID_To_Name(tempElement.header.typeID),
			static_cast<const char *> (typeName.ToCStr()), tempElement.header.typeID);

		switch(tempElement.header.typeID)
		{
			case API_WallID:
				WriteReport(GetWallInfoStr(tempElement.wall).c_str());
				DuplicateWall(tempElement);
				break;
			case API_SlabID:
				WriteReport(GetSlabInfoStr(tempElement.slab).c_str());
				DuplicateSlab(tempElement);
				break;
			case API_ObjectID: 
				WriteReport(GetObjectInfoStr(tempElement.object).c_str());
				DuplicateElement(tempElement);
				Do_Set_IFC(tempElement);
				Do_Get_IFC(tempElement);
				break;
			case API_BeamID: WriteReport(GetBeamInfoStr(tempElement.beam).c_str()); break;
			default: break;
		}
		
		//WriteReport("Detail: Angle:%10.7f Pos: x:%10.7f y:%10.7f", detailAngle, detailPos.x, detailPos.y);
		//WriteReport("Dimension->RefC3D x: %10.7f y: %10.7f z: %10.7f", coord3d.x, coord3d.y, coord3d.z);
		//WriteReport("Dimension->RefC   x: %10.7f y: %10.7f", dimenRef.x, dimenRef.y);
		//WriteReport("Dimension->Direction3D   x: %10.7f y: %10.7f z: %10.7f", dimenDir3D.x, dimenDir3D.y, dimenDir3D.z);
		//WriteReport("Dimension->Direction     x: %10.7f y: %10.7f", dimenDir.x, dimenDir.y);
		//WriteReport("angleDimension.base->Loc   x: %10.7f y: %10.7f", anDimBaseLoc.x, anDimBaseLoc.y);
		//WriteReport("angleDimension.base->Loc3D x: %10.7f y: %10.7f z: %10.7f", anDimBaseLoc3D.x, anDimBaseLoc3D.y, anDimBaseLoc3D.z);
		//WriteReport("radDimBaseLoc.base->Loc   x: %10.7f y: %10.7f", radDimBaseLoc.x, radDimBaseLoc.y);
		//WriteReport("radDimBaseLoc3D.base->Loc3D x: %10.7f y: %10.7f z: %10.7f", radDimBaseLoc3D.x, radDimBaseLoc3D.y, radDimBaseLoc3D.z);
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

// -----------------------------------------------------------------------------
// Save the current plan into a picture
// -----------------------------------------------------------------------------

void		Do_Save_As_Image(void)
{
	API_FileSavePars		fsp;
	API_SavePars_Picture	pars_pict;
	GSErrCode				err;

	BNZeroMemory(&fsp, sizeof(API_FileSavePars));
	fsp.fileTypeID = APIFType_TIFFFile;
	//fsp.fileTypeID = APIFType_JPEGFile;

	IO::Location folderLoc;
	API_SpecFolderID specID = API_UserDocumentsFolderID;
	ACAPI_Environment(APIEnv_GetSpecFolderID, &specID, &folderLoc);
	fsp.file = new IO::Location(folderLoc, IO::Name("PictTest.tif"));
	//fsp.file = new IO::Location(folderLoc, IO::Name("PictTest.jpg"));

	BNZeroMemory(&pars_pict, sizeof(API_SavePars_Picture));
	pars_pict.colorDepth = APIColorDepth_256C;
	pars_pict.dithered = false;
	pars_pict.view2D = false;
	pars_pict.crop = true;
	pars_pict.keepSelectionHighlight = true;

	err = ACAPI_Automate(APIDo_SaveID, &fsp, &pars_pict);
	if (err != NoError) {
		char msgStr[256];
		sprintf(msgStr, "Error in APIDo_SaveID (pict): %d", (int)err);
		ACAPI_WriteReport(msgStr, true);
	}

	delete fsp.file;

	return;
}		// Do_Save_TiffFile

// -----------------------------------------------------------------------------
// Save the current plan to an IFC file
// -----------------------------------------------------------------------------

void		Do_Save_IfcFile(void)
{
	API_FileSavePars	fsp;
	API_SavePars_Ifc	pars_ifc;
	GSErrCode			err;

	BNZeroMemory(&fsp, sizeof(API_FileSavePars));
	fsp.fileTypeID = APIFType_IfcFile;

	IO::Location folderLoc;
	API_SpecFolderID specID = API_UserDocumentsFolderID;
	ACAPI_Environment(APIEnv_GetSpecFolderID, &specID, &folderLoc);
	fsp.file = new IO::Location(folderLoc, IO::Name("IfcTest.ifc"));

	BNZeroMemory(&pars_ifc, sizeof(pars_ifc));
	pars_ifc.subType = APIIfc_2x3;

	err = ACAPI_Automate(APIDo_SaveID, &fsp, &pars_ifc);
	if (err != NoError) {
		char msgStr[256];
		sprintf(msgStr, "Error in APIDo_SaveID (Ifc): %s", ErrID_To_Name(err));
		ACAPI_WriteReport(msgStr, true);
	}

	delete fsp.file;

	return;
}		// Do_Save_IfcFile

// -----------------------------------------------------------------------------
// Save the current plan to an Pdf file
// -----------------------------------------------------------------------------

void		Do_Save_PdfFile(void)
{
	API_FileSavePars	fsp;
	GSErrCode			err;

	BNZeroMemory(&fsp, sizeof(API_FileSavePars));
	fsp.fileTypeID = APIFType_PdfFile;

	IO::Location folderLoc;
	API_SpecFolderID specID = API_UserDocumentsFolderID;
	ACAPI_Environment(APIEnv_GetSpecFolderID, &specID, &folderLoc);
	fsp.file = new IO::Location(folderLoc, IO::Name("PdfTest.pdf"));

	err = ACAPI_Automate(APIDo_SaveID, &fsp, NULL);
	if (err != NoError) {
		char msgStr[256];
		sprintf(msgStr, "Error in APIDo_SaveID (Pdf): %s", ErrID_To_Name(err));
		ACAPI_WriteReport(msgStr, true);
	}

	delete fsp.file;

	return;
	//Do_SaveFile
}
void Do_Call_Web()
{
	webTest();
}

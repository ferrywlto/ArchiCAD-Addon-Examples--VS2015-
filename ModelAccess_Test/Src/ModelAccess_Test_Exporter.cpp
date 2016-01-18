// *****************************************************************************
// Export ModelAccess Test files
// API Development Kit 19; Mac/Win
//
// Namespaces:		Contact person:
//	   WRL
//
// [SG compatible] - Yes
// *****************************************************************************

//----------------------------------- Includes ---------------------------------

#include "RealNumber.h"
#include "AngleData.h"
#include "TRANMAT.h"
//#include "CGraphics2DIO.h"

#include <math.h>

#include "BaseElement.hpp"
#include "ModelBody.hpp"
#include "ConvexPolygon.hpp"
#include "Light.hpp"
#include "Model.hpp"
#include "ModelElement.hpp"
#include "ModelMaterial.hpp"
#include "Polygon.hpp"
#include "Texture.hpp"
#include "TextureCoordinate.hpp"
#include "Vertex.hpp"
#include "Transformation.hpp"
#include "Coord3D.h"
//#include "Graphics2D.h"
#include "GSRoot.hpp"
#include "FileSystem.hpp"
#include "Folder.hpp"
//#include "IOUtilities.hpp"

#include "ModelAccess_Test_Exporter.hpp"
#include "ModelAccess_Test_Common.hpp"
#include "ModelAccess_Test_Resources.hpp"


//------------------------------- Class definitions ----------------------------

//------------------------------- Class MAExporter ---------------------------

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
WRL::MAExporter::MAExporter (ModelerAPI::Model* newModel, const API_IOParams* ioParams) :
	model (NULL),
	writer (NULL)
{
	if (DBERROR (newModel == NULL))
		throw GS::NullPointerException ();
	model = newModel;

	if (DBERROR (ioParams == NULL))
		throw GS::NullPointerException ();
	ioParam_fileLoc			= *ioParams->fileLoc;
	ioParam_saveFileIOName  = *ioParams->saveFileIOName;

	LoadLocalizedStringResources ();

	try {
		writer = new MAWriter (ioParam_fileLoc, model);
	}

	catch (...) {
		throw GS::OutOfMemoryException ();
	}
}


//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
WRL::MAExporter::~MAExporter ()
{
	if (writer) {
		delete writer;
		writer = NULL;
	}
}


//------------------------------------------------------------------------------
// Generate and copy texture files next to the file
//------------------------------------------------------------------------------
void		WRL::MAExporter::CreateTextures (void)
{
	Int32	nTexture = model->GetTextureNum ();
	if (nTexture <= 0)
		return;

	IO::Name	fName = ioParam_saveFileIOName;
	fName.DeleteExtension ();		// if any
	fName.Append ("_");
	fName.Append (localizedStrings[WRL::TexturesName]);

	textureLoc = ioParam_fileLoc;
	GSErrCode	errCode = textureLoc.SetLastLocalName (fName);	// replace scratch file name
	TEST_AND_THROW (errCode);

	IO::Folder	textureFolder (textureLoc, IO::Folder::Create);
	errCode = textureFolder.GetStatus ();
	TEST_AND_THROW (errCode);

	char	fileName[WRL::MaxStringLength + 256];
	for (Int32 i = 2; i <= nTexture; i++) {	// the first texture is the background picture
		CreateTextureFile (i, fileName);
		textureNames.Add (fileName);
	}
}


//------------------------------------------------------------------------------
// Remove the extension from the file name
//------------------------------------------------------------------------------
void		WRL::MAExporter::RemoveExtension (char* fileName)
{
	char*	pointPosInFileName = CHSearchCharRight ('.', fileName);
	if (pointPosInFileName != NULL) {
		for (char* p = fileName + strlen (fileName); p >= pointPosInFileName; p--)
			*p = 0;
	}
}

#ifdef WINDOWS
const char		Delimiter = '\\';
#endif

#ifdef macintosh
const char		Delimiter = '/';
#endif


//------------------------------------------------------------------------------
// Create a texture file
//------------------------------------------------------------------------------
void		WRL::MAExporter::CreateTextureFile (Int32 textureIndex, char* fullPathName)
{
	ModelerAPI::Texture	texture;
	model->GetTexture (textureIndex, &texture);

	char	name[256];
	try {
		CHTruncate (texture.GetName ().ToCStr(), name, 256);
		RemoveExtension (name);
	} catch (...) {
		name[0] = '\0';
	}

#if 0
	ModelerAPI::Texture::Pixel* pixelMap = new ModelerAPI::Texture::Pixel[texture.GetPixelMapSize ()];
	texture.GetPixelMap (pixelMap);

	GSPixMapHandle	pixMapHdl = GXCreateGSPixMap (texture.GetPixelMapXSize (), texture.GetPixelMapYSize ());
	if (pixMapHdl == NULL) {
		delete[] pixelMap;
		throw GS::OutOfMemoryException ();
	}

	GSPtr pixMapPtr = GXGetGSPixMapBaseAddr (pixMapHdl);
	if (DBERROR (pixMapPtr == NULL)) {
		GXDeleteGSPixMap (pixMapHdl);
		delete[] pixelMap;
		throw GS::NullPointerException ();
	}

	BNCopyMemory (pixMapPtr, pixelMap, texture.GetPixelMapSize () * sizeof (ModelerAPI::Texture::Pixel));
#endif

	char	pathBuffer[WRL::MaxStringLength];
	char	fileName[WRL::MaxStringLength];

	if (name[0] == '\0')
		GS::csnprintf (fileName, sizeof (fileName), "%s%ld.jpg", localizedStrings[WRL::TextureName].ToCStr ().Get (), (GS::LongForStdio) textureIndex);
	else
		GS::csnprintf (fileName, sizeof (fileName), "%s.jpg", name);

	IO::Location	fileLoc (textureLoc);
	IO::Name		fileIoName (fileName);
	fileLoc.AppendToLocal (fileIoName);

#if 0
	GX::Image aimage(pixMapHdl);
	GSErrCode err = aimage.WriteToFile (loc, FTM::FileTypeManager::SearchForMime("image/jpeg", NULL), 24);


	GXDeleteGSPixMap (pixMapHdl);		//jg050606 - #40824
	delete[] pixelMap;

	if (err != NoError)
		throw GS::GeneralException ();
#endif

	IO::Name	folderName;
	fileLoc.DeleteLastLocalName ();
	fileLoc.GetLastLocalName (&folderName);
	CHTruncate (folderName.ToString ().ToCStr(), pathBuffer, WRL::MaxStringLength);

	sprintf (fullPathName, "%s%c%s", pathBuffer, Delimiter, fileName);
}


//------------------------------------------------------------------------------
// Write the default header
//------------------------------------------------------------------------------
void		WRL::MAExporter::WriteHeader (void)
{
	writer->WriteHeader (const_cast<char*> (localizedStrings[WRL::CreatedText].ToCStr ().Get ()));
	writer->WriteNavigationInfo ();
}


//------------------------------------------------------------------------------
// Calculate the orientation of the coordinate system
//------------------------------------------------------------------------------
void		WRL::MAExporter::CalcOrientation (const Coord3D& viewPoint, const Coord3D& targetPoint,
												double rollAngle, Coord3D* axis, double* angle)
{
	if (DBERROR (axis == NULL || angle == NULL))
		throw GS::NullPointerException ();

		// calculation of the direction unit vector
	Coord3D	dir;
	dir.x = targetPoint.x - viewPoint.x;
	dir.y = targetPoint.y - viewPoint.y;
	dir.z = targetPoint.z - viewPoint.z;

	double	distance = sqrt (dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
	if (distance > EPS) {
		dir.x /= distance;
		dir.y /= distance;
		dir.z /= distance;
	} else {
		dir.x = 0.0;
		dir.y = 1.0;
		dir.z = 0.0;
	}

		// calculation of the right and the top vector
	Coord3D	right0;
	right0.x = dir.y;
	right0.y = -dir.x;
	right0.z = 0.0;
	distance = sqrt (right0.x*right0.x + right0.y*right0.y + right0.z*right0.z);
	if (distance > EPS) {
		right0.x /= distance;
		right0.y /= distance;
	} else {
		right0.x = 1.0;
		right0.y = 0.0;
		right0.z = 0.0;
	}

	Coord3D	top0;
	top0.x = right0.y * dir.z - right0.z * dir.y;
	top0.y = right0.z * dir.x - right0.x * dir.z;
	top0.z = right0.x * dir.y - right0.y * dir.x;

	double	sinangle = sin (DEGRAD * rollAngle);
	double	cosangle = cos (DEGRAD * rollAngle);

	Coord3D	right, top;
	right.x = right0.x * cosangle + top0.x *   sinangle;
	right.y = right0.y * cosangle + top0.y *   sinangle;
	right.z = right0.z * cosangle + top0.z *   sinangle;
	top.x =   top0.x *   cosangle - right0.x * sinangle;
	top.y =   top0.y *   cosangle - right0.y * sinangle;
	top.z =   top0.z *   cosangle - right0.z * sinangle;

		// the transformation matrix
	TRANMAT	atran;
	atran.tmx[0] = right.x;			atran.tmx[1] = top.x;			atran.tmx[2] = -dir.x;
	atran.tmx[4] = right.y;			atran.tmx[5] = top.y;			atran.tmx[6] = -dir.y;
	atran.tmx[8] = right.z;			atran.tmx[9] = top.z;			atran.tmx[10] = -dir.z;

		// calculation of the angle and the axis
	sinangle = sqrt ((atran.tmx[4] - atran.tmx[1]) * (atran.tmx[4] - atran.tmx[1]) / 4 +
					 (atran.tmx[2] - atran.tmx[8]) * (atran.tmx[2] - atran.tmx[8]) / 4 +
					 (atran.tmx[9] - atran.tmx[6]) * (atran.tmx[9] - atran.tmx[6]) / 4);
	cosangle = (atran.tmx[0] + atran.tmx[5] + atran.tmx[10] - 1) / 2;
	*angle = 0.0;
	if (fabs (cosangle) >= EPS || fabs (sinangle) >= EPS)
		*angle = atan2 (sinangle, cosangle);
	if (*angle > EPS) {
		double	q, p;
		p = 1.0 - cosangle;

		q = (atran.tmx[0] - cosangle) / p;	// theoretically q >= 0
		axis->x = (q >= 0.0) ? sqrt (q) : 0.0;
		q = (atran.tmx[5] - cosangle) / p;
		axis->y = (q >= 0.0) ? sqrt (q) : 0.0;
		q = (atran.tmx[10] - cosangle) / p;
		axis->z = (q >= 0.0) ? sqrt (q) : 0.0;

		if (atran.tmx[9] - atran.tmx[6] < 0)
			axis->x = -axis->x;
		if (atran.tmx[2] - atran.tmx[8] < 0)
			axis->y = -axis->y;
		if (atran.tmx[4] - atran.tmx[1] < 0)
			axis->z = -axis->z;
	} else {
		axis->x = 0.0;
		axis->y = 1.0;
		axis->z = 0.0;
	}
}


//------------------------------------------------------------------------------
// Calculate the scene limits
//------------------------------------------------------------------------------
void		WRL::MAExporter::CalcSceneLimits (Coord3D* minCoord, Coord3D* maxCoord)
{
	if (DBERROR (minCoord == NULL || maxCoord == NULL))
		throw GS::NullPointerException ();

	minCoord->x = 10E10;
	minCoord->y = 10E10;
	minCoord->z = 10E10;
	maxCoord->x = -10E10;
	maxCoord->y = -10E10;
	maxCoord->z = -10E10;

	Int32						iElement = 0, iBody = 0, iPgon = 0, iConvexPolygons = 0, iPedge = 0;
	Int32						nBody = 0, nPoly = 0, nConvexPolygons = 0, nPedge = 0;
	Int32						nElements = model->GetElementNum ();
	ModelerAPI::Element			element;
	ModelerAPI::Body				body;
	ModelerAPI::Polygon			polygon;
	ModelerAPI::ConvexPolygon	convexPolygon;
	ModelerAPI::Vertex			vertex;

	for (iElement = 1; iElement <= nElements; iElement++) {
		model->GetElement (iElement, &element);
		nBody = element.GetBodyNum ();

		for (iBody = 1; iBody <= nBody; iBody++) {
			element.GetBody (iBody, &body);
			nPoly = body.GetPolygonCount ();

			for (iPgon = 1; iPgon <= nPoly; iPgon++) {
				try {
					body.GetPolygon (iPgon, &polygon);
					nConvexPolygons = polygon.GetConvexPolygonNum ();

					for (iConvexPolygons = 1; iConvexPolygons <= nConvexPolygons; iConvexPolygons++) {
						polygon.GetConvexPolygon (iConvexPolygons, &convexPolygon);
						nPedge = convexPolygon.GetVertexNum ();

						for (iPedge = 1; iPedge <= nPedge; iPedge++) {
							body.GetVertex (convexPolygon.GetVertexIndex (iPedge), &vertex);
							if (vertex.x < minCoord->x)
								minCoord->x = vertex.x;
							if (vertex.y < minCoord->y)
								minCoord->y = vertex.y;
							if (vertex.z < minCoord->z)
								minCoord->z = vertex.z;
							if (vertex.x > maxCoord->x)
								maxCoord->x = vertex.x;
							if (vertex.y > maxCoord->y)
								maxCoord->y = vertex.y;
							if (vertex.z > maxCoord->z)
								maxCoord->z = vertex.z;
						}
					}
				}
				catch (const GS::Exception& e) {
					e.Print (dbChannel);
					// skip wrong (self-intersecting?) polygon as earlier
				}
			}
		}
	}
}


//------------------------------------------------------------------------------
// Export the camera in the 3D Window
//------------------------------------------------------------------------------
void		WRL::MAExporter::ExportCameraFromSaveAs3D  (void)
{
	API_3DProjectionInfo	projSets;
	ACAPI_Environment (APIEnv_Get3DProjectionSetsID, &projSets, NULL);

	if (projSets.isPersp) {
		Coord3D	viewPoint;
		viewPoint.x = projSets.u.persp.pos.x;
		viewPoint.y = projSets.u.persp.pos.y;
		viewPoint.z = projSets.u.persp.cameraZ;

		Coord3D	targetPoint;
		targetPoint.x = projSets.u.persp.target.x;
		targetPoint.y = projSets.u.persp.target.y;
		targetPoint.z = projSets.u.persp.targetZ;

		Coord3D	axis;
		double	amount;
		CalcOrientation (viewPoint, targetPoint, projSets.u.persp.rollAngle, &axis, &amount);
		writer->WriteViewpoint (0, viewPoint.x, viewPoint.y, viewPoint.z, axis.x, axis.y, axis.z, amount);
	} else {
		Coord3D	minCoord, maxCoord;
		CalcSceneLimits (&minCoord, &maxCoord);
		writer->WriteViewpoint (0, 0.0, 0.0, maxCoord.z + 1.0, 1.0, 0.0, 0.0, PI / 2);
	}
}


//------------------------------------------------------------------------------
// Export the lightsources
//------------------------------------------------------------------------------
void		WRL::MAExporter::ExportLights (void)
{
	double					intensity;
	Int32					elemNum = model->GetElementNum ();

	for (Int32 elemIndex = 0; elemIndex <= elemNum; elemIndex++) {
		ModelerAPI::Light		light;
		ModelerAPI::Element		elem;
		ModelerAPI::BaseElem	baseElem;
		Int32					nLights = 0;

		if (elemIndex == 0) // 3 special lights
			nLights = 3;
		else {
			model->GetElement (elemIndex, &elem);
			elem.GetBaseElem (&baseElem);
			nLights = baseElem.GetLightNum ();
		}
		for (Int32 i = 1; i <= nLights; i++) {
			if (elemIndex == 0) // 3 special lights
				model->GetLight (i, &light);
			else
				baseElem.GetLight (i, &light);

			switch (light.GetType ()) {
				case ModelerAPI::Light::DirectionLight:
				case ModelerAPI::Light::SunLight:
					writer->WriteDirectionalLight (light.GetColor ().red, light.GetColor ().green, light.GetColor ().blue,
													   light.GetDirection ().x, light.GetDirection ().y, light.GetDirection ().z);
					break;

				case ModelerAPI::Light::SpotLight:
					writer->WriteSpotLight (light.GetColor ().red, light.GetColor ().green, light.GetColor ().blue,
												light.GetPosition ().x, light.GetPosition ().y, light.GetPosition ().z,
												light.GetDirection ().x, light.GetDirection ().y, light.GetDirection ().z,
												2 * light.GetFalloffAngle1 ());
					break;

				case ModelerAPI::Light::PointLight:
					intensity = 1.0 / (1.0 + ((light.GetMaxDistance () - light.GetMinDistance ()) /
										2 - light.GetMinDistance ()) * light.GetDistanceFalloff ());
					writer->WritePointLight (intensity, light.GetColor ().red, light.GetColor ().green, light.GetColor ().blue,
												 light.GetPosition ().x, light.GetPosition ().y, light.GetPosition ().z);
					break;

				case ModelerAPI::Light::EyeLight:
				case ModelerAPI::Light::AmbientLight:
				case ModelerAPI::Light::CameraLight:
				case ModelerAPI::Light::UndefinedLight:
				break;
			}
		}
	}
}


//------------------------------------------------------------------------------
// Export a body
//------------------------------------------------------------------------------
void		WRL::MAExporter::ExportBody (ModelerAPI::Body& body)
{
	Int32	nPgon = body.GetPolygonCount ();
	if (nPgon <= 0)
		return;

	Int32	nVertex = body.GetVertexNum ();
	if (nVertex <= 0)
		return;

	bool	smooth = body.IsCurved ();
	Int32	nConvexPgon = 0, nPedge = 0;
	Int32	iPgon = 0, iConvexPgon = 0, iPedge = 0, j = 0, index = 0;
	char	textureName[WRL::MaxStringLength];

	ModelerAPI::ConvexPolygon		convexPolygon;
	ModelerAPI::Material				material;
	ModelerAPI::Polygon				polygon;
	ModelerAPI::TextureCoordinate	uvCoord;
	ModelerAPI::Vector				polygonNormal;
	ModelerAPI::Vector				vertexNormal;
	ModelerAPI::Vertex				vertex;

	writer->WriteGroupBegin ();

	for (iPgon = 1; iPgon <= nPgon; iPgon++) {
		body.GetPolygon (iPgon, &polygon);

		writer->WriteShapeBegin ();

		// IndexedFaceSet
		writer->WriteIndexedFaceSetBegin (body.IsSolidBody ());

		// Coordinate
		writer->WriteCoordinateBegin (iPgon == 1);
		if (iPgon == 1) {
			for (j = 1; j <= nVertex; j++) {
				body.GetVertex (j, &vertex);
				writer->WritePoint (j == nVertex, vertex.x, vertex.y, vertex.z, j - 1);
			}
		}
		writer->WriteCoordinateEnd (iPgon == 1);

		// coordIndex
		writer->WriteCoordIndexBegin ();
		try {
			nConvexPgon = polygon.GetConvexPolygonNum ();
			for (iConvexPgon = 1; iConvexPgon <= nConvexPgon; iConvexPgon++) {
				polygon.GetConvexPolygon (iConvexPgon, &convexPolygon);
				nPedge = convexPolygon.GetVertexNum ();

				for (iPedge = 1; iPedge <= nPedge; iPedge++)
					writer->WriteCoordIndexListAdd (false, convexPolygon.GetVertexIndex (iPedge) - 1);

				writer->WriteCoordIndexListAdd (iPgon == nPgon && iConvexPgon == nConvexPgon, -1);
			}
		}
		catch (const GS::Exception& e) {
			e.Print (dbChannel);
			// skip wrong (self-intersecting?) polygon as earlier
		}
		writer->WriteCoordIndexEnd ();

		// Normals
		if (smooth) {
			writer->WriteNormalBegin (iPgon == 1);
			if (iPgon == 1) {
				try {
					for (iConvexPgon = 1; iConvexPgon <= nConvexPgon; iConvexPgon++) {
						polygon.GetConvexPolygon (iConvexPgon, &convexPolygon);
						nPedge = convexPolygon.GetVertexNum ();

						for (iPedge = 1; iPedge <= nPedge; iPedge++){
							vertexNormal = convexPolygon.GetNormalVectorByVertex (iPedge);	// ConvexPolygon ==> abs(iPedge) != 0, but in the case of Polygon
																							// it has to be checked!
							writer->WriteVectorListAdd (j == nVertex, j - 1, vertexNormal.x, vertexNormal.y, vertexNormal.z);
						}
					}
				}
				catch (const GS::Exception& e) {
					e.Print (dbChannel);
					// skip wrong (self-intersecting?) polygon as earlier
				}
			}
			writer->WriteNormalEnd (iPgon == 1);

			// normalIndex
			writer->WriteNormalIndexBegin ();
			try {
				for (iConvexPgon = 1; iConvexPgon <= nConvexPgon; iConvexPgon++) {
					polygon.GetConvexPolygon (iConvexPgon, &convexPolygon);
					nPedge = convexPolygon.GetVertexNum ();

					for (iPedge = 1; iPedge <= nPedge; iPedge++)
						writer->WriteNormalIndexListAdd (false, convexPolygon.GetVertexIndex (iPedge) - 1);

					writer->WriteNormalIndexListAdd (iPgon == nPgon && iConvexPgon == nConvexPgon, -1);
				}
			}
			catch (const GS::Exception& e) {
				e.Print (dbChannel);
				// skip wrong (self-intersecting?) polygon as earlier
			}
			writer->WriteNormalIndexEnd ();
		}

		// texCoord
		body.GetVector (polygon.GetNormalVectorIndex (), &polygonNormal);
		writer->WriteTexCoordBegin ();
		try {
			for (iConvexPgon = 1; iConvexPgon <= nConvexPgon; iConvexPgon++) {
				polygon.GetConvexPolygon (iConvexPgon, &convexPolygon);
				nPedge = convexPolygon.GetVertexNum ();

				for (iPedge = 1; iPedge <= nPedge; iPedge++) {
					body.GetVertex (convexPolygon.GetVertexIndex (iPedge), &vertex);
					try {
						polygon.GetTextureCoordinate (&vertex, &uvCoord);
					}
					catch (...) {
						uvCoord.u = uvCoord.v = 0.0;
					}
					writer->WriteTexCoordAdd ((iPedge == nPedge && iConvexPgon == nConvexPgon), &uvCoord);
				}
			}
		}
		catch (const GS::Exception& e) {
			e.Print (dbChannel);
			// skip wrong (self-intersecting?) polygon as earlier
		}
		writer->WriteTexCoordEnd ();

		// texCoordIndex
		index = 0;
		writer->WriteTexCoordIndexBegin ();
		try {
			nConvexPgon = polygon.GetConvexPolygonNum ();
			for (iConvexPgon = 1; iConvexPgon <= nConvexPgon; iConvexPgon++) {
				polygon.GetConvexPolygon (iConvexPgon, &convexPolygon);
				nPedge = convexPolygon.GetVertexNum ();

				for (iPedge = 1; iPedge <= nPedge; iPedge++)
					writer->WriteTexCoordIndexListAdd (false, index++);

				writer->WriteTexCoordIndexListAdd (iPgon == nPgon && iConvexPgon == nConvexPgon, -1);
			}
		}
		catch (const GS::Exception& e) {
			e.Print (dbChannel);
			// skip wrong (self-intersecting?) polygon as earlier
		}
		writer->WriteTexCoordIndexEnd ();

		writer->WriteIndexedFaceSetEnd ();

		// appearance
		strcpy (textureName, "");
		model->GetMaterial (polygon.GetMaterialIndex (), &material);
		if (material.GetTextureIndex () > 1)
			CHTruncate (textureNames[material.GetTextureIndex () - 2].ToCStr ().Get (), textureName, WRL::MaxStringLength);

		writer->WriteAppearance (polygon.GetMaterialIndex (), textureName);

		writer->WriteShapeEnd ();
	}

	writer->WriteGroupEnd ();
}


//------------------------------------------------------------------------------
// Export the bodies
//------------------------------------------------------------------------------
void		WRL::MAExporter::ExportBodies (void)
{
	Int32	nElements = model->GetElementNum ();
	Int32	nBodies = 0;

	ModelerAPI::Body		body;
	ModelerAPI::Element	element;

	short	phaseNum = 1;
	ACAPI_Interface (APIIo_InitProcessWindowID, &localizedStrings[WRL::SavingFile], &phaseNum);
	ACAPI_Interface (APIIo_SetNextProcessPhaseID, NULL, &nElements);

	try {
		for (Int32 iElement = 1; iElement <= nElements; iElement++) {
			model->GetElement (iElement, &element);
			nBodies = element.GetBodyNum ();

			for (Int32 iBody = 1; iBody <= nBodies; iBody++) {
				element.GetBody (iBody, &body);
				ExportBody (body);
			}

			ACAPI_Interface (APIIo_SetProcessValueID, &iElement, NULL);
			if (iElement % 1000 == 0) {
				// the inquire of the cancel button every 1000th step
				if (ACAPI_Interface (APIIo_IsProcessCanceledID, NULL, NULL))
					throw WRL::Cancel ();
			}
		}
	}

	catch (WRL::Cancel&) {
		ACAPI_Interface (APIIo_CloseProcessWindowID, NULL, NULL);
		throw WRL::Cancel ();
	}

	catch (...) {
		ACAPI_Interface (APIIo_CloseProcessWindowID, NULL, NULL);
		throw GS::GeneralException ();
	}

	ACAPI_Interface (APIIo_CloseProcessWindowID, NULL, NULL);
}


//------------------------------------------------------------------------------
// Load the localized string resources
//------------------------------------------------------------------------------
void		WRL::MAExporter::LoadLocalizedStringResources (void)
{
	GS::UniString	name;

	localizedStrings.Add (name);	// dummy string

	RSGetIndString (&name, WRL::LocalizedStringTable, WRL::CreatedText, ACAPI_GetOwnResModule ());
	localizedStrings.Add (name);

	RSGetIndString (&name, WRL::LocalizedStringTable, WRL::TextureName, ACAPI_GetOwnResModule ());
	localizedStrings.Add (name);

	RSGetIndString (&name, WRL::LocalizedStringTable, WRL::TexturesName, ACAPI_GetOwnResModule ());
	localizedStrings.Add (name);

	RSGetIndString (&name, WRL::LocalizedStringTable, WRL::SavingFile, ACAPI_GetOwnResModule ());
	localizedStrings.Add (name);
}


//------------------------------------------------------------------------------
// Export the current 3D model
//------------------------------------------------------------------------------
void		WRL::MAExporter::ExportScene (void)
{
	CreateTextures ();

	WriteHeader ();
	ExportCameraFromSaveAs3D ();
	ExportLights ();
	ExportBodies ();
}

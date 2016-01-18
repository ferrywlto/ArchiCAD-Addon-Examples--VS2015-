// *****************************************************************************
// Write ModelAccess Test files
// API Development Kit 19; Mac/Win
//
// Namespaces:		Contact person:
//	   WRL
//
// [SG compatible] - Yes
// *****************************************************************************

//----------------------------------- Includes ---------------------------------

#include "ModelAccess_Test_Writer.hpp"

#include "File.hpp"
#include "GSRoot.hpp"

#include "Model.hpp"
#include "ModelMaterial.hpp"
#include "Parameter.hpp"
#include "ParameterList.hpp"
#include "Texture.hpp"
#include "TextureCoordinate.hpp"

#if defined (_MSC_VER)
#pragma warning (disable: 4702)		// unreachable code
#endif

//------------------------------- Class definitions ----------------------------

//------------------------------- Class MAWriter -----------------------------

//------------------------------------------------------------------------------
WRL::MAWriter::MAWriter (const IO::Location& ioParam_fileLoc, ModelerAPI::Model* maModel):
	outputFile (NULL),
	model (NULL),
	definedMaterials (NULL),
	nMaterials (0)
{
	if (DBERROR (maModel == NULL))
		throw GS::NullPointerException ();

	model = maModel;
	IO::Location			fileLoc;
	fileLoc = ioParam_fileLoc;
	nMaterials = model->GetMaterialNum ();

	try {
		definedMaterials = new bool[nMaterials];
		for (Int32 i = 0; i < nMaterials; i++)
			definedMaterials[i] = false;

		outputFile = new IO::File (fileLoc, IO::File::Create);
		if (outputFile->GetStatus () != NoError)
			throw GS::OutOfMemoryException ();
		outputFile->Open (IO::File::WriteEmptyMode);
		if (outputFile->GetStatus () != NoError)
			throw GS::OutOfMemoryException ();
	}

	catch (...) {
		throw GS::OutOfMemoryException ();
	}
}


//------------------------------------------------------------------------------
WRL::MAWriter::~MAWriter ()
{
	if (definedMaterials) {
		delete [] definedMaterials;
		definedMaterials = NULL;
	}

	if (outputFile != NULL) {
		if (outputFile->IsOpen ())
			outputFile->Close ();
		delete outputFile;
	}
}


//------------------------------------------------------------------------------
void		WRL::MAWriter::WriteHeader (char* str)
{
	char	buffer[256];

	strcpy (buffer, "#ModelAccess Test\n");
	WriteBuffer (buffer);

	sprintf (buffer, "#%s\n\n", str);
	WriteBuffer (buffer);
}


//------------------------------------------------------------------------------
void		WRL::MAWriter::WriteNavigationInfo (void)
{
	char	buffer[256];

	sprintf (buffer, "NavigationInfo {\n");
	WriteBuffer (buffer);
	sprintf (buffer, "\ttype\t\"EXAMINE\"\n");
	WriteBuffer (buffer);

	sprintf (buffer, "\tspeed\t1.0\n");
	WriteBuffer (buffer);

	sprintf (buffer, "\theadlight\tTRUE\n");
	WriteBuffer (buffer);

	sprintf (buffer, "}\n");
	WriteBuffer (buffer);
}


//------------------------------------------------------------------------------
void		WRL::MAWriter::WriteDefMaterial (Int32 iumat)
{
	char	buffer[256];

	sprintf (buffer, "DEF Mat_%ld Material {\n", (GS::LongForStdio) iumat);
	WriteBuffer (buffer);

	ModelerAPI::Material	material;
	model->GetMaterial (iumat, &material);

	// ambientIntensity

	// diffuseColor
	sprintf (buffer, "\t\t\t\t\tdiffuseColor\t\t%12g %12g %12g\n", material.GetSurfaceColor ().red, material.GetSurfaceColor ().green, material.GetSurfaceColor ().blue);
	WriteBuffer (buffer);

	// emissiveColor
	sprintf (buffer, "\t\t\t\t\temissiveColor\t\t%12g %12g %12g\n", material.GetEmissionColor ().red, material.GetEmissionColor ().green, material.GetEmissionColor ().blue);
	WriteBuffer (buffer);

	// shininess
	sprintf (buffer, "\t\t\t\t\tshininess\t\t%12g\n", material.GetShining () / 100.0);
	WriteBuffer (buffer);

	// specularColor
	sprintf (buffer, "\t\t\t\t\tspecularColor\t\t%12g %12g %12g\n", material.GetSpecularColor ().red, material.GetSpecularColor ().green, material.GetSpecularColor ().blue);
	WriteBuffer (buffer);

	// transparency
	sprintf (buffer, "\t\t\t\t\ttransparency\t\t%12g\n", material.GetTransparency ());
	WriteBuffer (buffer);

	sprintf (buffer, "\t\t\t\t}\n");
	WriteBuffer (buffer);
}


//------------------------------------------------------------------------------
void		WRL::MAWriter::WriteTexCoordBegin (void)
{
	char	buffer[256];

	sprintf (buffer, "\t\t\t\ttexCoord TextureCoordinate {\n");
	WriteBuffer (buffer);

	sprintf (buffer, "\t\t\t\t\tpoint[\n");
	WriteBuffer (buffer);
}


//------------------------------------------------------------------------------
void		WRL::MAWriter::WriteTexCoordEnd (void)
{
	char	buffer[256];

	sprintf (buffer, "\t\t\t\t\t]\n");
	WriteBuffer (buffer);

	sprintf (buffer, "\t\t\t\t}\n");
	WriteBuffer (buffer);
}


//------------------------------------------------------------------------------
void		WRL::MAWriter::WriteTexCoordAdd (bool isLast, ModelerAPI::TextureCoordinate* uvCoord)
{
	char	buffer[256];

	sprintf (buffer, "\t\t\t\t\t\t%12g %12g", uvCoord->u, uvCoord->v);

	if (!isLast)
		strcat (buffer, ",\n");
	else
		strcat (buffer, "\n");

	WriteBuffer (buffer);
}


//------------------------------------------------------------------------------
void		WRL::MAWriter::WriteTexCoordIndexBegin (void)
{
	char	buffer[256];

	sprintf (buffer, "\t\t\t\ttexCoordIndex[ ");
	WriteBuffer (buffer);
}


//------------------------------------------------------------------------------
void		WRL::MAWriter::WriteTexCoordIndexEnd (void)
{
	char	buffer[256];

	sprintf (buffer, "\n\t\t\t\t]\n");
	WriteBuffer (buffer);
}


//------------------------------------------------------------------------------
void		WRL::MAWriter::WriteTexCoordIndexListAdd (bool isLast, Int32 coordInd)
{
	char	buffer[256];

	sprintf (buffer, "%ld", (GS::LongForStdio) coordInd);
	if (!isLast) {
		strcat (buffer, ", ");
		if (coordInd == -1)
			strcat (buffer, "\n\t\t\t\t\t");
	}

	WriteBuffer (buffer);
}


//------------------------------------------------------------------------------
void		DumpMaterialRenderingSettings (ModelerAPI::Material&	material)
{
	ModelerAPI::ParameterList parameters;
	material.GetExtraParameters (&parameters);
	for (Int32 ii = 1; ii <= parameters.GetParameterNum (); ++ii) {
		ModelerAPI::Parameter	param;
		if (parameters.GetParameter (ii, &param)) {
			Int32 			dim1 = 0;
			Int32 			dim2 = 0;
			double			numericValue;
			GS::UniString	strValue;

			switch (param.GetType ()) {
				case ModelerAPI::Parameter::NumericType:
					numericValue = (double)param;
					DBPrintf ("Parameter %-3d: %s\n\ttype: numeric\n\tvalue: %f\n", ii, param.GetName ().ToCStr ().Get (), numericValue);
					break;
				case ModelerAPI::Parameter::StringType:
					param.GetStringValue (strValue);
					DBPrintf ("Parameter %-3d: %s\n\ttype: string\n\tvalue: %s\n", ii, param.GetName ().ToCStr ().Get (), strValue.ToCStr ().Get ());
					break;
				case ModelerAPI::Parameter::NumericArrayType:
					param.GetArrayDimensions (&dim1, &dim2);
					DBPrintf ("Parameter %-3d: %s\n\ttype: numeric array\n\tdims: %dx%d\n", ii, param.GetName ().ToCStr ().Get (), dim1, dim2);
					for (Int32 jj = 1; jj <= dim1; ++jj) {
						if (dim2 == 0) {
							numericValue = (double)param[jj];
							DBPrintf ("\t[%d] value: %f\n", jj, numericValue);
						} else {
							for (Int32 ll = 1; ll <= dim2; ++ll) {
								ModelerAPI::ArrayParameter	arrayItem;
								param.GetArrayItem (jj, ll, &arrayItem);
								numericValue = (double)param;
								DBPrintf ("\t[%d][%d] value: %f\n", jj, ll, numericValue);
							}
						}
					}
					break;
				case ModelerAPI::Parameter::StringArrayType:
					param.GetArrayDimensions (&dim1, &dim2);
					DBPrintf ("Parameter %-3d: %s\n\ttype: numeric array\n\tdims: %dx%d\n", ii, param.GetName ().ToCStr ().Get (), dim1, dim2);
					for (Int32 jj = 1; jj <= dim1; ++jj) {
						if (dim2 == 0) {
							param[jj].GetStringValue (strValue);
							DBPrintf ("\t[%d] value: %s\n", jj, strValue.ToCStr ().Get ());
						} else {
							for (Int32 ll = 1; ll <= dim2; ++ll) {
								ModelerAPI::ArrayParameter	arrayItem;
								param.GetArrayItem (jj, ll, &arrayItem);
								arrayItem.GetStringValue (strValue);
								DBPrintf ("\t[%d][%d] value: %s\n", jj, ll, strValue.ToCStr ().Get ());
							}
						}
					}
					break;
				default:
					DBPrintf ("Parameter %-3d: %s\n\ttype: undefined\n", ii, param.GetName ().ToCStr ().Get ());
					break;
			}
		}
	}
}


//------------------------------------------------------------------------------
void		WRL::MAWriter::WriteAppearance (Int32 iumat, char* textureName)
{
	char	buffer[256];

	sprintf (buffer, "\t\t\tappearance Appearance {\n");
	WriteBuffer (buffer);

	if (definedMaterials[iumat - 1]) {
		sprintf (buffer, "\t\t\t\tmaterial USE Mat_%ld\n", (GS::LongForStdio) iumat);
		WriteBuffer (buffer);
	} else {
		sprintf (buffer, "\t\t\t\tmaterial %ld", (GS::LongForStdio) iumat);
		WriteBuffer (buffer);

		WriteDefMaterial (iumat);
		definedMaterials[iumat - 1] = true;
	}

	ModelerAPI::Material	material;
	model->GetMaterial (iumat, &material);

	if (textureName[0] != '\0') {
		ModelerAPI::Texture	texture;
		model->GetTexture (material.GetTextureIndex (), &texture);

		sprintf (buffer, "\t\t\t\ttexture ImageTexture { url \"%s\" }\n", textureName);
		WriteBuffer (buffer);

		sprintf (buffer, "\t\t\t\ttextureTransform TextureTransform {\n");
		WriteBuffer (buffer);

		sprintf (buffer, "\t\t\t\t\trotation %12g\n", material.GetTextureRotationAngle ());
		WriteBuffer (buffer);

		sprintf (buffer, "\t\t\t\t\tscale %12g %12g\n", texture.GetXSize (), texture.GetYSize ());
		WriteBuffer (buffer);

		sprintf (buffer, "\t\t\t\t}\n");
		WriteBuffer (buffer);
	}
	sprintf (buffer, "\t\t\t}\n");
	WriteBuffer (buffer);

	DumpMaterialRenderingSettings (material);
}


//------------------------------------------------------------------------------
void		WRL::MAWriter::WriteDirectionalLight (double rc, double gc, double bc, double dirx, double diry, double dirz)
{
	char	buffer[256];

	sprintf (buffer, "DirectionalLight {\n");
	WriteBuffer (buffer);

	sprintf (buffer, "\ton\t\t\tTRUE\n");
	WriteBuffer (buffer);

	sprintf (buffer, "\tintensity\t\t%d\n", 1);
	WriteBuffer (buffer);

	sprintf (buffer, "\tcolor\t\t\t%12g %12g %12g\n", rc, gc, bc);
	WriteBuffer (buffer);

	sprintf (buffer, "\tdirection\t\t%12g %12g %12g\n", dirx, diry, dirz);
	WriteBuffer (buffer);

	sprintf (buffer, "}\n");
	WriteBuffer (buffer);
}


//------------------------------------------------------------------------------
void		WRL::MAWriter::WriteSpotLight (double rc, double gc, double bc, double posx, double posy, double posz,
										  double dirx, double diry, double dirz, double cutOffAngle)
{
	char	buffer[256];

	sprintf (buffer, "SpotLight {\n");
	WriteBuffer (buffer);

	sprintf (buffer, "\ton\t\t\tTRUE\n");
	WriteBuffer (buffer);

	sprintf (buffer, "\tintensity\t\t%d\n", 1);
	WriteBuffer (buffer);

	sprintf (buffer, "\tcolor\t\t\t%12g %12g %12g\n", rc, gc, bc);
	WriteBuffer (buffer);

	sprintf (buffer, "\tlocation\t\t%12g %12g %12g\n", posx, posy, posz);
	WriteBuffer (buffer);

	sprintf (buffer, "\tdirection\t\t%12g %12g %12g\n", dirx, diry, dirz);
	WriteBuffer (buffer);

	sprintf (buffer, "\tcutOffAngle\t\t%12g\n", cutOffAngle);
	WriteBuffer (buffer);

	sprintf (buffer, "}\n");
	WriteBuffer (buffer);
}


//------------------------------------------------------------------------------
void		WRL::MAWriter::WritePointLight (double intensity, double rc, double gc, double bc, double posx, double posy, double posz)
{
	char	buffer[256];

	sprintf (buffer, "PointLight {\n");
	WriteBuffer (buffer);

	sprintf (buffer, "\ton\t\t\tTRUE\n");
	WriteBuffer (buffer);

	sprintf (buffer, "\tintensity\t\t%12g\n", intensity);
	WriteBuffer (buffer);

	sprintf (buffer, "\tcolor\t\t\t%12g %12g %12g\n", rc, gc, bc);
	WriteBuffer (buffer);

	sprintf (buffer, "\tlocation\t\t%12g %12g %12g\n", posx, posy, posz);
	WriteBuffer (buffer);

	sprintf (buffer, "}\n");
	WriteBuffer (buffer);
}


//------------------------------------------------------------------------------
void		WRL::MAWriter::WriteGroupBegin (void)
{
	char	buffer[256];

	sprintf (buffer, "Group {\n");
	WriteBuffer (buffer);

	sprintf (buffer, "\tchildren[\n");
	WriteBuffer (buffer);
}


//------------------------------------------------------------------------------
void		WRL::MAWriter::WriteGroupEnd (void)
{
	char	buffer[256];

	sprintf (buffer, "\t]\n");
	WriteBuffer (buffer);

	sprintf (buffer, "}\n");
	WriteBuffer (buffer);
}


//------------------------------------------------------------------------------
void		WRL::MAWriter::WriteShapeBegin (void)
{
	char	buffer[256];

	sprintf (buffer, "\t\tShape {\n");
	WriteBuffer (buffer);
}


//------------------------------------------------------------------------------
void		WRL::MAWriter::WriteShapeEnd (void)
{
	char	buffer[256];

	sprintf (buffer, "\t\t}\n");
	WriteBuffer (buffer);
}


//------------------------------------------------------------------------------
void		WRL::MAWriter::WriteCoordinateBegin (bool first)
{
	char	buffer[256];

	if (first) {
		sprintf (buffer, "\t\t\t\tcoord DEF Coords Coordinate {\n");
		WriteBuffer (buffer);

		sprintf (buffer, "\t\t\t\t\tpoint[\n");
		WriteBuffer (buffer);
	} else {
		sprintf (buffer, "\t\t\t\tcoord USE Coords\n");
		WriteBuffer (buffer);
	}
}


//------------------------------------------------------------------------------
void		WRL::MAWriter::WriteCoordinateEnd (bool first)
{
	char	buffer[256];

	if (first) {
		sprintf (buffer, "\t\t\t\t\t]\n");
		WriteBuffer (buffer);

		sprintf (buffer, "\t\t\t\t}\n");
		WriteBuffer (buffer);
	}
}


//------------------------------------------------------------------------------
void		WRL::MAWriter::WritePoint (bool isLast, double x, double y, double z, Int32 index)
{
	char	buffer[256];
	char	buffer2[256];

	sprintf (buffer, "\t\t\t\t\t\t%12g %12g %12g", x, y, z);
	if (!isLast)
		strcat (buffer, ",");
	sprintf (buffer2, "		#%3ld\n", (GS::LongForStdio) index);
	strcat (buffer, buffer2);

	WriteBuffer (buffer);
}


//------------------------------------------------------------------------------
void		WRL::MAWriter::WriteIndexedFaceSetBegin (bool solid)
{
	char	buffer[256];

	sprintf (buffer, "\t\t\tgeometry IndexedFaceSet {\n");
	WriteBuffer (buffer);

	if (solid == false) {
		sprintf (buffer, "\t\t\t\tsolid FALSE\n");
		WriteBuffer (buffer);
	}
}

//------------------------------------------------------------------------------
void		WRL::MAWriter::WriteIndexedFaceSetEnd (void)
{
	char	buffer[256];

	sprintf (buffer, "\t\t\t}\n");
	WriteBuffer (buffer);
}


//------------------------------------------------------------------------------
void		WRL::MAWriter::WriteCoordIndexBegin (void)
{
	char	buffer[256];

	sprintf (buffer, "\t\t\t\tcoordIndex[ ");
	WriteBuffer (buffer);
}


//------------------------------------------------------------------------------
void		WRL::MAWriter::WriteCoordIndexEnd (void)
{
	char	buffer[256];

	sprintf (buffer, "\n\t\t\t\t]\n");
	WriteBuffer (buffer);
}


//------------------------------------------------------------------------------
void		WRL::MAWriter::WriteCoordIndexListAdd (bool isLast, Int32 coordInd)
{
	char	buffer[256];

	sprintf (buffer, "%ld", (GS::LongForStdio) coordInd);
	if (!isLast) {
		strcat (buffer, ", ");
		if (coordInd == -1)		// face end index
			strcat (buffer, "\n\t\t\t\t\t");
	}

	WriteBuffer (buffer);
}


//------------------------------------------------------------------------------
void		WRL::MAWriter::WriteNormalBegin (bool first)
{
	char	buffer[256];

	if (first) {
		sprintf (buffer, "\t\t\t\tnormal DEF Normals Normal {\n\t\t\t\t\tvector[\n");
		WriteBuffer (buffer);
	} else {
		sprintf (buffer, "\t\t\t\tnormal USE Normals\n");
		WriteBuffer (buffer);
	}
}


//------------------------------------------------------------------------------
void		WRL::MAWriter::WriteNormalEnd (bool first)
{
	char	buffer[256];

	if (first) {
		sprintf (buffer, "\t\t\t\t\t]\n\t\t\t\t}\n");
		WriteBuffer (buffer);
	}
}


//------------------------------------------------------------------------------
void		WRL::MAWriter::WriteVectorListAdd (bool isLast, Int32 index, double nx, double ny, double nz)
{
	char	buffer[256];
	char	buffer2[256];

	sprintf (buffer, "\t\t\t\t\t\t%12g %12g %12g", nx, ny, nz);
	if (!isLast)
		strcat (buffer, ",");

	sprintf (buffer2, "		#%3ld\n", (GS::LongForStdio) index);
	strcat (buffer, buffer2);

	WriteBuffer (buffer);
}


//------------------------------------------------------------------------------
void		WRL::MAWriter::WriteNormalIndexBegin (void)
{
	char	buffer[256];

	sprintf (buffer, "\t\t\t\tnormalIndex[ ");
	WriteBuffer (buffer);
}


//------------------------------------------------------------------------------
void		WRL::MAWriter::WriteNormalIndexEnd (void)
{
	char	buffer[256];

	sprintf (buffer, "\n\t\t\t\t]\n");
	WriteBuffer (buffer);
}


//------------------------------------------------------------------------------
void		WRL::MAWriter::WriteNormalIndexListAdd (bool isLast, Int32 normalInd)
{
	char	buffer[256];

	sprintf (buffer, "%ld", (GS::LongForStdio) normalInd);
	if (!isLast) {
		strcat (buffer, ", ");
		if (normalInd == -1)		// face end index
			strcat (buffer, "\n\t\t\t\t\t");
	}

	WriteBuffer (buffer);
}


//------------------------------------------------------------------------------
void		WRL::MAWriter::WriteViewpoint (Int32 i, double posx, double posy, double posz, double orx, double ory, double orz, double amount)
{
	char	buffer[256];

	sprintf (buffer, "\tDEF\tAC_Camera%ld\tViewpoint {\n", (GS::LongForStdio) i);
	WriteBuffer (buffer);

	sprintf (buffer, "\t\tposition\t%12g %12g %12g\n", posx, posy, posz);
	WriteBuffer (buffer);

	sprintf (buffer, "\t\torientation\t%12g %12g %12g %12g\n", orx, ory, orz, amount);
	WriteBuffer (buffer);

	sprintf (buffer, "\t\tdescription\t\"AC_Camera%ld\"\n", (GS::LongForStdio) i);
	WriteBuffer (buffer);

	sprintf (buffer, "\t}\n");
	WriteBuffer (buffer);
}


//------------------------------------------------------------------------------
void		WRL::MAWriter::WriteBuffer (char* buffer)
{
	Int32	size = Strlen32 (buffer);

#ifdef WINDOWS
	// convert "\n"s to "CharCR CharLF"
	char	buffer2[512];
	Int32	i, j;

	for (i = 0, j = 0; i < size; i++) {
		if (buffer[i] != '\n') {
			buffer2[j++] = buffer[i];
		} else {
			buffer2[j++] = CharCR;
			buffer2[j++] = CharLF;
		}
	}
	buffer2[j] = '\0';
	outputFile->WriteBin (buffer2, j);
#endif

#ifdef macintosh
	outputFile->WriteBin (buffer, size);
#endif
}

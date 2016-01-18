// *****************************************************************************
// Description:		Source code for the Plan Dump Add-On
//
// Module:			API Development Kit 19; Mac/Win
// Namespace:		-none-
// Contact person:	devsup@graphisoft.hu
//
// SG compatible
// *****************************************************************************

#define	_PLAN_DUMP_TRANSL_

// --- Includes ----------------------------------------------------------------

#include	"APIEnvir.h"
#include	"Resource.h"
#include	"DumpWriter.hpp"
#include	"DumpUtils.hpp"

#include	<time.h>

#include	"GSRoot.hpp"
#include	"BM.hpp"

#include	"DG.h"

#include	"GenArc2DData.h"

#include	"ACAPinc.h"					// also includes APIdefs.h

// --- Variable declarations ---------------------------------------------------
static DumpWriter*		writer;

static double			dScale;


// =============================================================================
//
// Helper functions
//
// =============================================================================



// -----------------------------------------------------------------------------
//	Return the length unit as text
// -----------------------------------------------------------------------------

static void		SetLengthUnit (API_UnitPrefs prefs, char *s)
{
	switch (prefs.lengthUnit) {
		case APIUnit_Metric:		strcpy (s, "M");
									break;

		case APIUnit_Centimetric:	strcpy (s, "CM");
									break;

		case APIUnit_Millimetric:	strcpy (s, "MM");
									break;

		case APIUnit_FootInch:		strcpy (s, "FFI");
									break;

		case APIUnit_FootDecInch:	strcpy (s, "FDI");
									break;

		case APIUnit_DecFoot:		strcpy (s, "DF");
									break;

		case APIUnit_Inch:			strcpy (s, "FI");
									break;

		case APIUnit_DecInch:		strcpy (s, "DI");
									break;

		default:					strcpy (s, "XXX");
									break;
	}

	return;
}		// SetLengthUnit


// -----------------------------------------------------------------------------
//	Return the angle unit as text
// -----------------------------------------------------------------------------

static	void	SetAngleUnit (API_UnitPrefs prefs, char *s)
{
	switch (prefs.angleUnit) {
		case APIAngle_DecimalDegree:	strcpy (s, "DD");
										break;

		case APIAngle_DegreeMinSec:		strcpy (s, "DMS");
										break;

		case APIAngle_Grads:			strcpy (s, "GR");
										break;

		case APIAngle_Radians:			strcpy (s, "RAD");
										break;

		case APIAngle_SurveyorsUnit:	strcpy (s, "SURV");
										break;

		default:						strcpy (s, "XXX");
	}

	return;
}		// SetAngleUnit


// =============================================================================
//
// 								I/O functions
//
// =============================================================================
#ifdef __APPLE__
#pragma mark -
#endif

// -----------------------------------------------------------------------------
//	Save all pens into a file
// -----------------------------------------------------------------------------

static GSErrCode	Do_SaveAs_Pen (void)
{
	API_Attribute		attrib;
	short				nAttr;
	GSErrCode			err;

	BNZeroMemory (&attrib, sizeof (API_Attribute));
	err = ACAPI_Attribute_GetNum (API_PenID, &nAttr);

	for (short i = 1; i <= nAttr && err == NoError; i++) {
		attrib.header.typeID = API_PenID;
		attrib.header.index = i;
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			writer->WriteBlock ("PEN", i);
			writer->WriteRGB (&attrib.pen.rgb);
			err = writer->WrEndblk ();
		} else {
			if (err == APIERR_DELETED)
				err = NoError;
		}
	}

	return err;
}		// Do_SaveAs_Pen


// -----------------------------------------------------------------------------
//	Save all linetypes into a file
// -----------------------------------------------------------------------------

static GSErrCode	Do_SaveAs_Linetype (void)
{
	API_Attribute		attrib;
	API_AttributeDef	defs;
	short				nAttr;
	GSErrCode			err;

	BNZeroMemory (&attrib, sizeof (API_Attribute));
	err = ACAPI_Attribute_GetNum (API_LinetypeID, &nAttr);

	for (short i = 1; i <= nAttr && err == NoError; i++) {
		attrib.header.typeID = API_LinetypeID;
		attrib.header.index = i;
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			err = ACAPI_Attribute_GetDef (API_LinetypeID, i, &defs);
			if (err == NoError) {
				writer->WriteBlock ("LINE_TYPE", i);
				writer->WriteName (attrib.header.name, DumpWriter::WithNewLine);
				switch (attrib.linetype.type) {
					case APILine_SolidLine:
							writer->WriteStr ("SOLID_LINE");
							break;
					case APILine_DashedLine:
						{
							writer->WriteStr ("DASHED_LINE", DumpWriter::WithNewLine);
							writer->WriteInt (2 * attrib.linetype.nItems, DumpWriter::WithNewLine);
							for (short j = 0; j < attrib.linetype.nItems; j++) {
								writer->WriteFloat ((*defs.ltype_dashItems)[j].dash / 1000.0);
								writer->WrNewLine ();
								writer->WriteFloat ((*defs.ltype_dashItems)[j].gap / 1000.0);
								if (j != attrib.linetype.nItems)
									writer->WrNewLine ();
							}
							break;
						}
					case APILine_SymbolLine:
							writer->WriteStr ("SYMBOL_LINE");
							break;
				}
				err = writer->WrEndblk ();
				ACAPI_DisposeAttrDefsHdls (&defs);
			}
		} else {
			if (err == APIERR_DELETED)
				err = NoError;
		}
	}

	return err;
}		// Do_SaveAs_Linetype


// -----------------------------------------------------------------------------
//	Save all fill types
// -----------------------------------------------------------------------------

static GSErrCode	Do_SaveAs_Filltype (void)
{
	API_Attribute		attrib;
	API_AttributeDef	defs,
						defs2;
	short				nAttr;
	short				j, k;
	GSErrCode			err;

	BNZeroMemory (&attrib, sizeof (API_Attribute));
	err = ACAPI_Attribute_GetNum (API_FilltypeID, &nAttr);

	for (short i = 1; i <= nAttr && err == NoError; i++) {
		attrib.header.typeID = API_FilltypeID;
		attrib.header.index = i;
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			err = ACAPI_Attribute_GetDef (API_FilltypeID, i, &defs);
			if (err == NoError) {
				writer->WriteBlock ("FILL", i);
				writer->WriteName (attrib.header.name, DumpWriter::WithNewLine);
				switch (attrib.filltype.subType) {
					case APIFill_Vector:
							writer->WriteStr ("VECTOR_FILL", DumpWriter::WithNewLine);

							for (j = 0; j <= 7; j++)
								writer->WriteInt (attrib.filltype.bitPat[j]);

							writer->WrNewLine ();
							if (attrib.header.flags & APIFill_ScaleWithPlan) {
								attrib.filltype.hXSpac /= (Int32) dScale;
								attrib.filltype.hYSpac /= (Int32) dScale;
							}

							writer->WriteFloat (attrib.filltype.hXSpac);
							writer->WriteFloat (attrib.filltype.hYSpac);
							writer->WriteAngle (attrib.filltype.hAngle);
							writer->WriteInt (attrib.filltype.linNumb);

							err = ACAPI_Attribute_GetDef (API_FilltypeID, i, &defs2 );

							if (err == NoError) {

								for (j = 0; j < attrib.filltype.linNumb; j++) {
									writer->WrNewLine ();
									writer->WriteFloat ((*(defs2.fill_lineItems))[j].lFreq);
									writer->WriteFloat ((*(defs2.fill_lineItems))[j].lDir);
									writer->WriteFloat ((*(defs2.fill_lineItems))[j].lOffsetLine);
									writer->WriteFloat ((*(defs2.fill_lineItems))[j].lOffset.x);
									writer->WriteFloat ((*(defs2.fill_lineItems))[j].lOffset.y);
									writer->WriteInt ((*(defs2.fill_lineItems))[j].lPartNumb);

									if ((*(defs2.fill_lineItems))[j].lPartNumb > 0 ) {

										writer->WrNewLine ();
										for (k = 0; k < (*(defs2.fill_lineItems))[j].lPartNumb; k++) {
											writer->WriteFloat ((*(defs2.fill_lineLength))[k +
															(*(defs2.fill_lineItems))[j].lPartOffs]);
										}

									}
								}
							}
							ACAPI_DisposeAttrDefsHdls (&defs2);
							break;

					case APIFill_Symbol:
							writer->WriteStr ("SYMBOL_FILL");
							break;

					case APIFill_Solid:
							writer->WriteStr ("SOLID_FILL");
							writer->WriteFloat (attrib.filltype.percent);
							break;

					case APIFill_Empty:
							writer->WriteStr ("EMPTY_FILL");
							break;

					case APIFill_LinearGradient:
							writer->WriteStr ("LINEAR_GRADIENT");
							break;

					case APIFill_RadialGradient:
							writer->WriteStr ("RADIAL_GRADIENT");
							break;

					case APIFill_Image:
							{
								writer->WriteStr ("IMAGE_FILL");

								char				buffer[512];
								sprintf (buffer, "%s", (const char *) GS::UniString (attrib.filltype.textureName).ToCStr ());
								writer->WriteStr (buffer);
								writer->WriteInt (attrib.filltype.textureStatus);
								writer->WriteFloat (attrib.filltype.textureXSize);
								writer->WriteFloat (attrib.filltype.textureYSize);
								writer->WriteFloat (attrib.filltype.textureRotAng);
							}
							break;
				}
				err = writer->WrEndblk ();
				ACAPI_DisposeAttrDefsHdls (&defs);
			}
		} else {
			if (err == APIERR_DELETED)
				err = NoError;
		}
	}

	return err;
}		// Do_SaveAs_Filltype


// -----------------------------------------------------------------------------
//	Save all composite structures
// -----------------------------------------------------------------------------

static GSErrCode	Do_SaveAs_CompStruct (void)
{
	API_Attribute		attrib;
	API_AttributeDef	defs;
	short				nAttr;
	char				name[API_NameLen];
	GSErrCode			err;

	BNZeroMemory (&attrib, sizeof (API_Attribute));
	err = ACAPI_Attribute_GetNum (API_CompWallID, &nAttr);

	for (short i = 1; i <= nAttr && err == NoError; i++) {
		attrib.header.typeID = API_CompWallID;
		attrib.header.index = i;
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			err = ACAPI_Attribute_GetDef (API_CompWallID, i, &defs);
			if (err == NoError) {
				writer->WriteBlock ("COMPOSITE_STRUCTURE", i);
				writer->WriteName (attrib.header.name, DumpWriter::WithNewLine);
				writer->WriteFloat (attrib.compWall.totalThick);
				writer->WrNewLine ();
				writer->WriteInt (attrib.compWall.nComps);
				for (short j = 0; j < attrib.compWall.nComps; j++) {

					writer->WrNewLine ();
					DumpUtils::GetAttrName (name, API_BuildingMaterialID, (*defs.cwall_compItems)[j].buildingMaterial);
					writer->WriteInt ((Int32) (*defs.cwall_compItems)[j].framePen);
					writer->WriteName (name);
					writer->WriteFloat ((*defs.cwall_compItems)[j].fillThick);
				}
				err = writer->WrEndblk ();
				ACAPI_DisposeAttrDefsHdls (&defs);
			}
		} else {
			if (err == APIERR_DELETED)
				err = NoError;
		}
	}

	return err;
}		// Do_SaveAs_CompStruct


// -----------------------------------------------------------------------------
//	Save all materials
// -----------------------------------------------------------------------------

static GSErrCode	Do_SaveAs_Material (void)
{
	API_Attribute		attrib;
	char				buffer[512];
	char				name[API_NameLen];
	short				nAttr;
	GSErrCode			err;

	BNZeroMemory (&attrib, sizeof (API_Attribute));
	err = ACAPI_Attribute_GetNum (API_MaterialID, &nAttr);

	for (short i = 1; i <= nAttr && err == NoError; i++) {
		attrib.header.typeID = API_MaterialID;
		attrib.header.index = i;
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			writer->WriteBlock ("MATERIAL", i);
			writer->WriteName (attrib.header.name, DumpWriter::WithNewLine);
			writer->WriteRGB (&attrib.material.surfaceRGB);
			writer->WrNewLine ();
			sprintf (buffer, "%.6lf %.6lf %.6lf %.6lf",
					attrib.material.ambientPc / 100.0,  attrib.material.diffusePc / 100.0,
					attrib.material.specularPc / 100.0, attrib.material.transpPc / 100.0);
			writer->WriteStr (buffer, DumpWriter::WithNewLine);
			writer->WriteInt (attrib.material.shine / 100);
			writer->WriteInt (attrib.material.transpAtt / 100, DumpWriter::WithNewLine);

			writer->WriteRGB (&attrib.material.specularRGB);
			writer->WrNewLine ();
			writer->WriteRGB (&attrib.material.emissionRGB);
			writer->WrNewLine ();
			sprintf (buffer, "%.6lf", attrib.material.emissionAtt / 100.0);
			writer->WriteStr (buffer, DumpWriter::WithNewLine);

			DumpUtils::GetAttrName (name, API_FilltypeID, attrib.material.ifill);
			writer->WriteName (name, DumpWriter::WithNewLine);
			writer->WriteInt (attrib.material.fillCol, DumpWriter::WithNewLine);

			if (attrib.material.texture.status != 0 && attrib.material.texture.fileLoc != NULL && attrib.material.texture.fileLoc->GetLocalLength () != 0) {
				IO::Name	name;
				err = attrib.material.texture.fileLoc->GetLastLocalName (&name);
				writer->WriteName ((const char*)name.ToString ().ToCStr ());
				writer->WriteFloat (attrib.material.texture.xSize);
				writer->WriteFloat (attrib.material.texture.ySize);
				writer->WriteAngle (attrib.material.texture.rotAng);
				writer->WrNewLine ();
			} else {
				writer->WriteName ("???");
			}
			err = writer->WrEndblk ();
		} else {
			if (err == APIERR_DELETED)
				err = NoError;
		}

		if (attrib.material.texture.fileLoc != NULL) {
			delete attrib.material.texture.fileLoc;
			attrib.material.texture.fileLoc = NULL;
		}
	}
	return err;
}		// Do_SaveAs_Material


// -----------------------------------------------------------------------------
//	Save all zone categories
// -----------------------------------------------------------------------------

static GSErrCode	Do_SaveAs_ZoneCategories (void)
{
	API_Attribute		attrib;
	API_AttributeDefExt	defs;
	short				nAttr;
	GSErrCode			err;

	BNZeroMemory (&attrib, sizeof (API_Attribute));
	err = ACAPI_Attribute_GetNum (API_ZoneCatID, &nAttr);

	for (short i = 1; i <= nAttr && err == NoError; i++) {
		attrib.header.typeID = API_ZoneCatID;
		attrib.header.index = i;
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			writer->WriteBlock ("ZONE_CATEGORY", i);
			writer->WriteName (attrib.header.name, DumpWriter::WithNewLine);
			writer->WriteStr (GS::UniString (attrib.zoneCat.catCode).ToCStr ().Get (), DumpWriter::WithNewLine);
			writer->WriteStr (GS::UniString (attrib.zoneCat.stampName).ToCStr ().Get (), DumpWriter::WithNewLine);
			writer->WriteRGB (&attrib.zoneCat.rgb);
			writer->WrNewLine ();

			err = ACAPI_Attribute_GetDefExt (API_ZoneCatID, i, &defs);
			if (err == NoError) {
				UInt32 addParNum = BMGetHandleSize (reinterpret_cast<GSHandle> (defs.zone_addParItems)) / sizeof (API_AddParType);
				writer->WriteBlock ("ZC_PARAMETERS", (short)addParNum);
				writer->WriteParams (defs.zone_addParItems);
				err = writer->WrEndblk ();
				ACAPI_DisposeAttrDefsHdlsExt (&defs);
			}

			err = writer->WrEndblk ();
		} else {
			if (err == APIERR_DELETED)
				err = NoError;
		}
	}
	return err;
}		// Do_SaveAs_ZoneCategories


// -----------------------------------------------------------------------------
//	Save all MEP Systems
// -----------------------------------------------------------------------------

static GSErrCode	Do_SaveAs_MEPSystems (void)
{
	//MEP systems don't have extended data.
	API_Attribute		attrib;
	short				nAttr;
	GSErrCode			err;

	BNZeroMemory (&attrib, sizeof (API_Attribute));
	err = ACAPI_Attribute_GetNum (API_MEPSystemID, &nAttr);

	for (short i = 1; i <= nAttr && err == NoError; i++) {
		attrib.header.typeID = API_MEPSystemID;
		attrib.header.index = i;
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			writer->WriteBlock ("MEP_SYSTEM", i);
			writer->WriteName (attrib.header.name, DumpWriter::WithNewLine);

			writer->WriteStr (attrib.mepSystem.isForDuctwork ? "true" : "false");
			writer->WriteStr (attrib.mepSystem.isForPipework ? "true" : "false");
			writer->WriteStr (attrib.mepSystem.isForCabling ? "true" : "false");
			writer->WrNewLine ();
			writer->WriteInt (attrib.mepSystem.contourPen);
			writer->WriteInt (attrib.mepSystem.fillPen);
			writer->WriteInt (attrib.mepSystem.fillBgPen);
			writer->WriteInt (attrib.mepSystem.fillInd);
			writer->WriteInt (attrib.mepSystem.centerLTypeInd);
			writer->WriteInt (attrib.mepSystem.centerLinePen);
			writer->WriteInt (attrib.mepSystem.materialInd);
			writer->WriteInt (attrib.mepSystem.insulationMatInd);
			err = writer->WrEndblk ();
		} else {
			if (err == APIERR_DELETED)
				err = NoError;
		}
	}
	return err;
}		// Do_SaveAs_MEPSystems


// -----------------------------------------------------------------------------
//	Save all building materials
// -----------------------------------------------------------------------------

static GSErrCode	Do_SaveAs_BuildingMaterial (void)
{
	API_Attribute		attrib;
	short				nAttr;
	GSErrCode			err;
	char				name[API_NameLen];

	BNZeroMemory (&attrib, sizeof (API_Attribute));
	err = ACAPI_Attribute_GetNum (API_BuildingMaterialID, &nAttr);

	for (short i = 1; i <= nAttr && err == NoError; i++) {
		attrib.header.typeID = API_BuildingMaterialID;
		attrib.header.index = i;
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			writer->WriteBlock ("BUILDING_MATERIAL", i);
			writer->WriteName (attrib.header.name, DumpWriter::WithNewLine);
			Int32 uiPriority = 0;
			ACAPI_Goodies (APIAny_Elem2UIPriorityID, &attrib.buildingMaterial.connPriority, &uiPriority);
			writer->WriteInt (uiPriority, DumpWriter::WithNewLine);
			DumpUtils::GetAttrName (name, API_FilltypeID, attrib.buildingMaterial.cutFill);
			writer->WriteInt (attrib.buildingMaterial.cutFillPen);
			writer->WriteInt (attrib.buildingMaterial.cutFillBackgroundPen);
			writer->WriteName (name, DumpWriter::WithNewLine);
			DumpUtils::GetAttrName (name, API_MaterialID, attrib.buildingMaterial.cutMaterial);
			writer->WriteName (name, DumpWriter::WithNewLine);
			writer->WriteInt (attrib.buildingMaterial.cutFillOrientation, DumpWriter::WithNewLine);
			writer->WriteFloat (attrib.buildingMaterial.thermalConductivity);
			writer->WriteFloat (attrib.buildingMaterial.density);
			writer->WriteFloat (attrib.buildingMaterial.heatCapacity);
			err = writer->WrEndblk ();
		} else {
			if (err == APIERR_DELETED)
				err = NoError;
		}
	}

	return err;
}		// Do_SaveAs_CompStruct


// -----------------------------------------------------------------------------
// Save the attributes of the actual project
// -----------------------------------------------------------------------------

static GSErrCode	Do_SaveAs_Attribute (void)
{
	GSErrCode	err;

	err = Do_SaveAs_Pen ();

	if (err == NoError)
		err = Do_SaveAs_Linetype ();

	if (err == NoError)
		err = Do_SaveAs_Filltype ();

	if (err == NoError)
		err = Do_SaveAs_CompStruct ();

	if (err == NoError)
		err = Do_SaveAs_Material ();

	if (err == NoError)
		err = Do_SaveAs_ZoneCategories ();

	if (err == NoError)
		err = Do_SaveAs_MEPSystems ();

	if (err == NoError)
		err = Do_SaveAs_BuildingMaterial ();

	return err;

}		// Do_SaveAs_Attribute


// -----------------------------------------------------------------------------
//	Save some parameters of an object based element
// -----------------------------------------------------------------------------

static GSErrCode	Do_SaveAs_Object (API_ElemTypeID elemsTypeID)
{
	API_Element			element;
	API_LibPart 		libPart;
	GSErrCode			err;

	GS::Array<API_Guid> elemList;
	err = ACAPI_Element_GetElemList (elemsTypeID, &elemList);

	for (GS::Array<API_Guid>::ConstIterator it = elemList.Enumerate (); it != NULL && err == NoError; ++it) {
		BNZeroMemory (&element, sizeof (API_Element));
		element.header.guid = *it;
		err = ACAPI_Element_Get (&element);

		if (err == NoError) {
			BNZeroMemory (&libPart, sizeof (API_LibPart));

			GS::UniString infoString;
			switch (elemsTypeID) {
				case API_ObjectID:	writer->WriteBlock ("SYMBOL", element.header.guid);
									ACAPI_Database (APIDb_GetCompoundInfoStringID, &element.header.guid, &infoString);
									writer->WriteElemHead (&element, infoString.ToCStr ().Get (), NULL);
									libPart.index = element.object.libInd;
									break;

				case API_LampID:	writer->WriteBlock ("LIGHT", element.header.guid);
									ACAPI_Database (APIDb_GetCompoundInfoStringID, &element.header.guid, &infoString);
									writer->WriteElemHead (&element, infoString.ToCStr ().Get (), NULL);
									libPart.index = element.lamp.libInd;
									break;

				case API_ZoneID:	writer->WriteBlock ("ROOM", element.header.guid);
									ACAPI_Database (APIDb_GetCompoundInfoStringID, &element.header.guid, &infoString);
									writer->WriteElemHead (&element, infoString.ToCStr ().Get (), NULL);
									libPart.index = element.zone.libInd;
									break;

				default:			writer->WriteBlock ("???", element.header.guid);
			}

			err = ACAPI_LibPart_Get (&libPart);
			if (libPart.location != NULL) {
				delete libPart.location;
				libPart.location = NULL;
			}

			if (err == NoError) {
				char docuname [256];
				CHTruncate (GS::UniString (libPart.docu_UName).ToCStr (), docuname, sizeof (docuname));
				writer->WriteName (docuname, DumpWriter::WithNewLine);

				if (elemsTypeID == API_ZoneID) {
					writer->WriteFloat (element.zone.pos.x);
					writer->WriteFloat (element.zone.pos.y);
					writer->WriteFloat (0.0);
					writer->WriteAngle (0.0);
					writer->WrNewLine ();
					writer->WriteFloat (1.0);
					writer->WriteFloat (1.0);
					writer->WrNewLine ();
					writer->WriteInt (0);
					writer->WriteInt (0, DumpWriter::WithNewLine);
					writer->WriteInt (0, DumpWriter::WithNewLine);
				} else {
					writer->WriteFloat (element.object.pos.x);
					writer->WriteFloat (element.object.pos.y);
					writer->WriteFloat (element.object.level);
					writer->WriteAngle (element.object.angle);
					writer->WrNewLine ();
					writer->WriteFloat (element.object.xRatio);
					writer->WriteFloat (element.object.yRatio);
					writer->WrNewLine ();
					writer->WriteInt (element.object.reflected);
					writer->WriteInt (element.object.lightIsOn, DumpWriter::WithNewLine);
					writer->WriteInt (element.object.mat, DumpWriter::WithNewLine);
				}

				writer->WriteParams (&element);
				err = writer->WriteSurfVol (element.header.typeID, element.header.guid);

				if (err == NoError)
					err = writer->WrProperties (element.header.guid);
			}
			if (err == NoError)
				err = writer->WrEndblk ();
		} else {
			if (err == APIERR_DELETED)
				err = NoError;
		}
	}

	return err;
}		// Do_SaveAs_Object


// -----------------------------------------------------------------------------
//	Save some parameters of walls
// -----------------------------------------------------------------------------

static GSErrCode	Do_SaveAs_Wall (void)
{
	API_Element			element;
	double				dirangle;
	double				wradius;
	char				name[API_NameLen];
	GSErrCode			err;

	GS::Array<API_Guid> wallList;
	err = ACAPI_Element_GetElemList (API_WallID, &wallList);

	for (GS::Array<API_Guid>::ConstIterator it = wallList.Enumerate (); it != NULL && err == NoError; ++it) {
		BNZeroMemory (&element, sizeof (API_Element));
		element.header.guid = *it;
		err = ACAPI_Element_Get (&element);

		if (err == NoError) {
			writer->WriteBlock ("WALL", element.header.guid);
			GS::UniString infoString;
			ACAPI_Database (APIDb_GetCompoundInfoStringID, &element.header.guid, &infoString);
			writer->WriteElemHead (&element, infoString.ToCStr ().Get (), NULL);

			err = writer->Wr2D_Poly (&element, &element.wall.poly);

			DumpUtils::GetWallData (&element.wall, &dirangle, &wradius);

			writer->WriteAngle (dirangle);
			writer->WrNewLine ();
			writer->WriteAngle (element.wall.angle);
			writer->WrNewLine ();

			writer->WriteFloat (element.wall.height);
			writer->WriteFloat (element.wall.bottomOffset);
			writer->WriteFloat (element.wall.topOffset);
			writer->WriteFloat (element.wall.thickness);
			writer->WrNewLine ();

			if (element.wall.modelElemStructureType == API_BasicStructure)
				DumpUtils::GetAttrName (name, API_BuildingMaterialID, element.wall.buildingMaterial);

			writer->WriteName (name, DumpWriter::WithNewLine);

			DumpUtils::GetAttrName ( name, API_MaterialID, element.wall.refMat.material);
			writer->WriteName (name);
			DumpUtils::GetAttrName ( name, API_MaterialID, element.wall.oppMat.material);
			writer->WriteName (name);
			DumpUtils::GetAttrName ( name, API_MaterialID, element.wall.sidMat.material);
			writer->WriteName (name, DumpWriter::WithNewLine);

			API_ElementMemo memo;
			BNZeroMemory (&memo, sizeof (API_ElementMemo));
			ACAPI_Element_GetMemo (element.header.guid, &memo, APIMemoMask_WallWindows | APIMemoMask_WallDoors);
			if (memo.wallWindows != NULL) {
				GSSize	nWinds = BMGetPtrSize (reinterpret_cast<GSPtr>(memo.wallWindows)) / sizeof (API_Guid);
				for (GSIndex i = 0; i < nWinds; i++)
					writer->WrAnOpening (API_WindowID, memo.wallWindows[i], &element, dirangle, wradius);
			}
			if (memo.wallDoors != NULL) {
				GSSize	nDoors = BMGetPtrSize (reinterpret_cast<GSPtr>(memo.wallDoors)) / sizeof (API_Guid);
				for (GSIndex i = 0; i < nDoors; i++)
					writer->WrAnOpening (API_DoorID, memo.wallDoors[i], &element, dirangle, wradius);
			}
			ACAPI_DisposeElemMemoHdls (&memo);

			if (fabs (dirangle) > EPS) {
				writer->WriteFloat (wradius);
				writer->WrNewLine ();
			}
			err = writer->WrCutPlane ( &element);

			if (err == NoError)
				err = writer->WriteSurfVol (element.header.typeID, element.header.guid);

			if (err == NoError)
				err = writer->WrProperties (element.header.guid);

			if (err == NoError)
				err = writer->WrEndblk ();

		} else {
			if (err == APIERR_DELETED)
				err = NoError;
		}
	}

	return err;
}		// Do_SaveAs_Wall


// -----------------------------------------------------------------------------
//	Save parameters of columns
// -----------------------------------------------------------------------------

static GSErrCode	Do_SaveAs_Column (void)
{
	API_Element			element;
	char				buffer[256];
	GSErrCode			err;

	GS::Array<API_Guid> columnList;
	err = ACAPI_Element_GetElemList (API_ColumnID, &columnList);

	for (GS::Array<API_Guid>::ConstIterator it = columnList.Enumerate (); it != NULL && err == NoError; ++it) {
		BNZeroMemory (&element, sizeof (API_Element));
		element.header.guid = *it;
		err = ACAPI_Element_Get (&element);
		if (err == NoError) {
			writer->WriteBlock ("COLUMN", element.header.guid);
			GS::UniString infoString;
			ACAPI_Database (APIDb_GetCompoundInfoStringID, &element.header.guid, &infoString);
			writer->WriteElemHead (&element, infoString.ToCStr ().Get (), NULL);

			writer->WriteFloat (element.column.origoPos.x);
			writer->WriteFloat (element.column.origoPos.y);
			writer->WriteAngle (element.column.angle);
			writer->WrNewLine ();

			writer->WriteFloat (element.column.coreWidth);
			writer->WriteFloat (element.column.coreDepth);
			writer->WriteFloat (static_cast<Int32> (element.column.venType));
			writer->WriteFloat (element.column.venThick);
			writer->WrNewLine ();

			writer->WriteFloat (element.column.height);
			writer->WriteFloat (element.column.bottomOffset);
			writer->WriteFloat (element.column.topOffset);
			writer->WrNewLine ();

			DumpUtils::GetAttrName (buffer, API_MaterialID, element.column.mater.material);
			writer->WriteName (buffer, DumpWriter::WithNewLine);

			if (element.column.modelElemStructureType == API_BasicStructure) {
				DumpUtils::GetAttrName (buffer, API_BuildingMaterialID, element.column.buildingMaterial);
				writer->WriteName (buffer, DumpWriter::WithNewLine);

				DumpUtils::GetAttrName (buffer, API_BuildingMaterialID, element.column.venBuildingMaterial);
				writer->WriteName (buffer, DumpWriter::WithNewLine);
			}

			err = writer->WrCutPlane ( &element);

			if (err == NoError)
				err = writer->WriteSurfVol (element.header.typeID, element.header.guid);
			if (err == NoError)
				err = writer->WrProperties (element.header.guid);
			if (err == NoError)
				err = writer->WrEndblk ();
		} else {
			if (err == APIERR_DELETED)
				err = NoError;
		}
	}

	return err;
}		// Do_SaveAs_Column


// -----------------------------------------------------------------------------
//	Save parameters of slabs
// -----------------------------------------------------------------------------

static GSErrCode	Do_SaveAs_Slab (void)
{
	API_Element			element;
	char				name[API_NameLen];
	GSErrCode			err;

	GS::Array<API_Guid> slabList;
	err = ACAPI_Element_GetElemList (API_SlabID, &slabList);

	for (GS::Array<API_Guid>::ConstIterator it = slabList.Enumerate (); it != NULL && err == NoError; ++it) {
		BNZeroMemory (&element, sizeof (API_Element));
		element.header.guid = *it;
		err = ACAPI_Element_Get (&element);
		if (err == NoError) {
			writer->WriteBlock ("SLAB", element.header.guid);
			GS::UniString infoString;
			ACAPI_Database (APIDb_GetCompoundInfoStringID, &element.header.guid, &infoString);
			writer->WriteElemHead (&element, infoString.ToCStr ().Get (), NULL);

			if (element.slab.modelElemStructureType == API_BasicStructure) {
				DumpUtils::GetAttrName (name, API_BuildingMaterialID, element.slab.buildingMaterial);
				writer->WrNewLine ();
				writer->WriteName (name, DumpWriter::WithNewLine);
			}

			err = writer->Wr2D_Poly (&element, &element.slab.poly);

			if (err == NoError) {
				writer->WriteFloat (element.slab.level);
				writer->WrNewLine ();
				writer->WriteFloat (element.slab.thickness);
				writer->WrNewLine ();

				DumpUtils::GetAttrName (name, API_MaterialID, element.slab.topMat.material);
				writer->WriteName (name);
				DumpUtils::GetAttrName (name, API_MaterialID, element.slab.botMat.material);
				writer->WriteName (name);
				DumpUtils::GetAttrName (name, API_MaterialID, element.slab.sideMat.material);
				writer->WriteName (name, DumpWriter::WithNewLine);

				err = writer->WriteSurfVol (element.header.typeID, element.header.guid);
				if (err == NoError)
					err = writer->WrProperties (element.header.guid);

				if (err == NoError)
					err = writer->WrEndblk ();
			}
		} else {
			if (err == APIERR_DELETED)
				err = NoError;
		}
	}

	return err;
}		// Do_SaveAs_Slab


// -----------------------------------------------------------------------------
//	Save parameters of roofs
// -----------------------------------------------------------------------------

static GSErrCode Do_SaveAs_Roof (void)
{
	API_Element			element;
	char				name[API_NameLen];
	GSErrCode			err;

	GS::Array<API_Guid> roofList;
	err = ACAPI_Element_GetElemList (API_RoofID, &roofList);

	for (GS::Array<API_Guid>::ConstIterator it = roofList.Enumerate (); it != NULL && err == NoError; ++it) {
		BNZeroMemory (&element, sizeof (API_Element));
		element.header.guid = *it;
		err = ACAPI_Element_Get (&element);
		if (err == NoError) {
			writer->WriteBlock ("ROOF", element.header.guid);
			GS::UniString infoString;
			ACAPI_Database (APIDb_GetCompoundInfoStringID, &element.header.guid, &infoString);
			writer->WriteElemHead (&element, infoString.ToCStr ().Get (), NULL);

			writer->WriteFloat (element.roof.shellBase.level);
			writer->WriteFloat (element.roof.shellBase.thickness);
			writer->WrNewLine ();

			DumpUtils::GetAttrName (name, API_MaterialID, element.roof.shellBase.topMat.material);
			writer->WriteName (name);
			DumpUtils::GetAttrName (name, API_MaterialID, element.roof.shellBase.botMat.material);
			writer->WriteName (name);
			DumpUtils::GetAttrName (name, API_MaterialID, element.roof.shellBase.sidMat.material);
			writer->WriteName (name, DumpWriter::WithNewLine);

			if (element.roof.roofClass == API_PlaneRoofID) {
				writer->WriteFloat (element.roof.u.planeRoof.baseLine.c1.x);
				writer->WriteFloat (element.roof.u.planeRoof.baseLine.c1.y);
				writer->WriteFloat (element.roof.u.planeRoof.baseLine.c2.x);
				writer->WriteFloat (element.roof.u.planeRoof.baseLine.c2.y);
				writer->WrNewLine ();
				writer->WriteAngle (element.roof.u.planeRoof.angle);
				writer->WrNewLine ();
				err = writer->Wr2D_Poly (&element, &(element.roof.u.planeRoof.poly));
			} else {
				err = writer->Wr2D_PivotPoly (&element, &(element.roof.u.polyRoof.pivotPolygon));
				if (err == NoError)
					err = writer->Wr2D_Poly (&element, &(element.roof.u.polyRoof.contourPolygon));
			}

			if (err == NoError)
				err = writer->WriteSurfVol (element.header.typeID, element.header.guid, element.roof.roofClass == API_PlaneRoofID);

			if (err == NoError)
				err = writer->WrProperties (element.header.guid);

			if (err == NoError)
				err = writer->WrEndblk ();

		} else {
			if (err == APIERR_DELETED)
				err = NoError;
		}
	}

	return err;
}		// Do_SaveAs_Roof


// -----------------------------------------------------------------------------
//	Save parameters of shells
// -----------------------------------------------------------------------------

static GSErrCode Do_SaveAs_Shell (void)
{
	API_Element			element;
	char				name[API_NameLen];
	GSErrCode			err;

	GS::Array<API_Guid> shellList;
	err = ACAPI_Element_GetElemList (API_ShellID, &shellList);

	for (GS::Array<API_Guid>::ConstIterator it = shellList.Enumerate (); it != NULL && err == NoError; ++it) {
		BNZeroMemory (&element, sizeof (API_Element));
		element.header.guid = *it;
		err = ACAPI_Element_Get (&element);
		if (err == NoError) {
			writer->WriteBlock ("SHELL", element.header.guid);
			GS::UniString infoString;
			ACAPI_Database (APIDb_GetCompoundInfoStringID, &element.header.guid, &infoString);
			writer->WriteElemHead (&element, infoString.ToCStr ().Get (), NULL);

			writer->WrNewLine ();
			writer->WriteFloat (element.shell.shellBase.level);
			writer->WriteFloat (element.shell.shellBase.thickness);
			writer->WrNewLine ();

			DumpUtils::GetAttrName (name, API_MaterialID, element.shell.shellBase.topMat.material);
			writer->WriteName (name);
			DumpUtils::GetAttrName (name, API_MaterialID, element.shell.shellBase.botMat.material);
			writer->WriteName (name);
			DumpUtils::GetAttrName (name, API_MaterialID, element.shell.shellBase.sidMat.material);
			writer->WriteName (name, DumpWriter::WithNewLine);

			err = writer->Wr2D_ShellShape (&element);

			if (err == NoError)
				err = writer->WriteSurfVol (element.header.typeID, element.header.guid);

			if (err == NoError)
				err = writer->WrProperties (element.header.guid);

			if (err == NoError)
				err = writer->WrEndblk ();

		} else {
			if (err == APIERR_DELETED)
				err = NoError;
		}
	}

	return err;
}		// Do_SaveAs_Shell


// -----------------------------------------------------------------------------
//	Save parameters of morph
// -----------------------------------------------------------------------------

static GSErrCode Do_SaveAs_Morph (void)
{
	API_Element			element;
	GSErrCode			err;

	GS::Array<API_Guid> morphList;
	err = ACAPI_Element_GetElemList (API_MorphID, &morphList);

	for (GS::Array<API_Guid>::ConstIterator it = morphList.Enumerate (); it != NULL && err == NoError; ++it) {
		BNZeroMemory (&element, sizeof (API_Element));
		element.header.guid = *it;
		err = ACAPI_Element_Get (&element);
		if (err == NoError) {
			writer->WriteBlock ("MORPH", element.header.guid);
			GS::UniString infoString;
			ACAPI_Database (APIDb_GetCompoundInfoStringID, &element.header.guid, &infoString);
			writer->WriteElemHead (&element, infoString.ToCStr ().Get (), NULL);

			writer->WrNewLine ();

			err = writer->Wr_Morph (&element);

			if (err == NoError)
				err = writer->WriteSurfVol (element.header.typeID, element.header.guid);

			if (err == NoError)
				err = writer->WrProperties (element.header.guid);

			if (err == NoError)
				err = writer->WrEndblk ();

		} else {
			if (err == APIERR_DELETED)
				err = NoError;
		}
	}

	return err;
}		// Do_SaveAs_Morph


// -----------------------------------------------------------------------------
//	Save some element types into a file
// -----------------------------------------------------------------------------

static GSErrCode	Do_SaveAs_Elements (void)
{
	GSErrCode err = Do_SaveAs_Wall ();

	if (err == NoError)
		err = Do_SaveAs_Column ();

	if (err == NoError)
		err = Do_SaveAs_Object (API_ObjectID);

	if (err == NoError)
		err = Do_SaveAs_Object (API_LampID);

	if (err == NoError)
		err = Do_SaveAs_Slab ();

	if (err == NoError)
		err = Do_SaveAs_Roof ();

	if (err == NoError)
		err = Do_SaveAs_Shell ();

	if (err == NoError)
		err = Do_SaveAs_Morph ();

	if (err == NoError)
		err = Do_SaveAs_Object (API_ZoneID);

	return err;
}		// Do_SaveAs_Elements


// -----------------------------------------------------------------------------
//	Dump parts of the plan
// -----------------------------------------------------------------------------

static void		Do_SaveAs (void)
{
	API_StoryInfo		storyInfo;
	time_t				t;
	char				buffer[256];
	char				dateStr[128];
	short 				i;
	GSErrCode			err = NoError;;

	IO::Location		temporaryFolderLoc;
	API_SpecFolderID	temporaryFolderID = API_TemporaryFolderID;
	err = ACAPI_Environment (APIEnv_GetSpecFolderID, &temporaryFolderID, &temporaryFolderLoc);
	if (err == NoError)
		err = writer->Open (IO::Location (temporaryFolderLoc, IO::Name ("PlanDump.txt")));
	if (err == NoError) {
		GS::UniString	path;
		IO::Location (temporaryFolderLoc, IO::Name ("PlanDump.txt")).ToPath (&path);
	    sprintf (buffer, "Plan Dump path is \"%s\"", path.ToCStr ().Get ());
		ACAPI_WriteReport (buffer, false);
	}

	// ========== header ==========
	if (err == NoError) {
		time (&t);
		strcpy (dateStr, ctime (&t));
		dateStr[24] = 0;													// end of string: 0

		sprintf (buffer, "# ArchiCAD PlanDump with API,   %s", dateStr);
		writer->WriteStr (buffer, DumpWriter::WithNewLine);
	}

	// ======= environment ========
	if (err == NoError) {

		writer->WriteBlock ("ENVIR", 0);
		API_UnitPrefs	prefs;
		err = ACAPI_Environment (APIEnv_GetPreferencesID, &prefs, (void *) APIPrefs_WorkingUnitsID);
		if (err == NoError) {
			SetLengthUnit (prefs, buffer);
			writer->WriteStr (buffer);
			writer->WriteInt (prefs.lenDecimals, DumpWriter::WithNewLine);

			SetAngleUnit (prefs, buffer);
			writer->WriteStr (buffer);
			writer->WriteInt (prefs.angleDecimals, DumpWriter::WithNewLine);

			err = ACAPI_Database (APIDb_GetDrawingScaleID, &dScale, NULL);
			if (err == NoError) {
				writer->WriteInt ((Int32) dScale);
				writer->WrEndblk ();
				err = writer->WrNewLine ();
			}
		}
	}

    // ======== attributes ========
    if (err == NoError)
		err = Do_SaveAs_Attribute ();

	// ======== stories ========
	if (err == NoError) {
		err = ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo, NULL);
		for (i = 0; (i <= storyInfo.lastStory - storyInfo.firstStory) && (err == NoError); i++) {
			writer->WriteBlock ("STORY", i + storyInfo.firstStory);

			if (strlen ((*storyInfo.data)[i].name) == 0) {
				sprintf (buffer, "%d. Story", i + storyInfo.firstStory);
				writer->WriteName (buffer, DumpWriter::WithNewLine);
			} else
				writer->WriteName ((*(storyInfo.data))[i].name, DumpWriter::WithNewLine);

			if (i == 0)
				sprintf (buffer, "%.6lf", (*storyInfo.data)[i].level);
			else
				sprintf (buffer, "%.6lf", (*storyInfo.data)[i].level - (*storyInfo.data)[i-1].level);


			writer->WriteStr (buffer);
			err = writer->WrEndblk ();
		}
		BMKillHandle ((GSHandle*) &storyInfo.data);
	}

	// ========= elements =========
	if (err == NoError)
		err = Do_SaveAs_Elements ();

	if (err != NoError) {
	    sprintf (buffer, "Error code:  %d", (int) err);
	    ACAPI_WriteReport (buffer, true);
		writer->WriteStr ("ErrorCode: ");
		writer->WriteInt (err);
	}

	writer->Close ();
}		// Do_SaveAs


// -----------------------------------------------------------------------------
// Parse the menu events
// -----------------------------------------------------------------------------

static GSErrCode __ACENV_CALL	MenuCommandProc (const API_MenuParams* menuParams)
{
	try {
		if (menuParams->menuItemRef.itemIndex == 1)
			Do_SaveAs ();
	}
	catch (...) {
		return APIERR_CANCEL;
	}

	return NoError;
}		// MenuCommandProc


// =============================================================================
//
// Required functions
//
// =============================================================================
#ifdef __APPLE__
#pragma mark -
#endif

// -----------------------------------------------------------------------------
// Called when the Add-On is going to be registered
// -----------------------------------------------------------------------------

API_AddonType	__ACENV_CALL	CheckEnvironment (API_EnvirParams* envirParams)
{
	if (envirParams->serverInfo.serverApplication != APIAppl_ArchiCADID)
		return APIAddon_DontRegister;

	ACAPI_Resource_GetLocStr (envirParams->addOnInfo.name, IDR_AddOnDescStrings, IDS_AddOnName);
	ACAPI_Resource_GetLocStr (envirParams->addOnInfo.description, IDR_AddOnDescStrings, IDS_AddOnDesc);

	return APIAddon_Normal;
}


// -----------------------------------------------------------------------------
// Interface definitions
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	RegisterInterface (void)
{
	GSErrCode	err = NoError;

	// Register menus
	err = ACAPI_Register_Menu (IDR_OwnMenu, 0, MenuCode_UserDef, MenuFlag_SeparatorBefore); //or MenuFlag_Default

	return err;
}


// -----------------------------------------------------------------------------
// Initialize
//		called after the Add-On has been loaded into memory
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	Initialize (void)
{
	ACAPI_Install_MenuHandler (IDR_OwnMenu, MenuCommandProc);

	writer = new DumpWriter ();

	return NoError;
}		// Initialize


// -----------------------------------------------------------------------------
// FreeData
//		called when the Add-On is going to be unloaded
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	FreeData (void)
{
	delete writer;

	return NoError;
}		// FreeData

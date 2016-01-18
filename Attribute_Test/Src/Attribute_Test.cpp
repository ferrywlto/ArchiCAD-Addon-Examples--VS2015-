// *****************************************************************************
// Source code for the Attribute Test Add-On
// API Development Kit 19; Mac/Win
//
// Namespaces:		 Contact person:
//		 -None-
//
// [SG compatible] - Yes
// *****************************************************************************

#include "APIEnvir.h"

// =============================================================================
//
// System Includes
//
// =============================================================================

#include	<math.h>
#include	<stdio.h>

// =============================================================================
//
// API Includes
//
// =============================================================================

#include	"Location.hpp"
#include	"UniString.hpp"
#include	"VectorImage.hpp"
#include	"ProfileAdditionalInfo.hpp"
#include	"ACAPinc.h"		// also includes APIdefs.h
#include	"APICommon.h"	// also includes GSRoot.hpp


static void DisposeAttribute (API_Attribute* attrib)
{
	switch (attrib->header.typeID) {
		case API_MaterialID:			delete attrib->material.texture.fileLoc;											break;
		case API_ModelViewOptionsID:	ACAPI_FreeGDLModelViewOptionsPtr (&attrib->modelViewOpt.modelViewOpt.gdlOptions);	break;
		default:						/* nothing has to be disposed */													break;
	}
}


// =============================================================================
//
// Attribute functions
//
// =============================================================================

// -----------------------------------------------------------------------------
//  Count the attribute instances
// -----------------------------------------------------------------------------

static void		Do_CountAttributes (void)
{
	API_AttrTypeID	typeID;
	short			count;
	GSErrCode		err = NoError;

	WriteReport ("# Count attributes:");
	WriteReport ("#   - report the maximum index for each attribute type");

	for (typeID = API_FirstAttributeID; typeID <= API_LastAttributeID; ((Int32 &) typeID)++) {
		err = ACAPI_Attribute_GetNum (typeID, &count);
		if (err != NoError) {
			WriteReport_Err ("Error in ACAPI_Attribute_GetNum", err);
			continue;
		}

		WriteReport ("%-10s: %d", AttrID_To_Name (typeID), count);
	}

	WriteReport_End (err);

	return;
}		// Do_CountAttributes


// -----------------------------------------------------------------------------
//  List all the attributes
// -----------------------------------------------------------------------------

static void		Do_ListAttributes (void)
{
	API_AttrTypeID	typeID;
	API_Attribute	attrib;
	short			count, i;
	GSErrCode		err = NoError;

	WriteReport ("# List attributes:");
	WriteReport ("#   - scan all attribute types and show the number of each type");
	WriteReport ("#   - deleted instances will be called \"DEL\"");

	for (typeID = API_FirstAttributeID; typeID <= API_LastAttributeID; ((Int32 &) typeID)++) {
		err = ACAPI_Attribute_GetNum (typeID, &count);
		if (err != NoError) {
			WriteReport_Err ("Error in ACAPI_Attribute_GetNum", err);
			continue;
		}

		WriteReport ("%s: %d", AttrID_To_Name (typeID), count);

		for (i = 1; i <= count; i++) {
			BNZeroMemory (&attrib, sizeof (API_Attribute));
			attrib.header.typeID = typeID;
			attrib.header.index = i;

			GS::UniString uniStringName;
			attrib.header.uniStringNamePtr = &uniStringName;

			err = ACAPI_Attribute_Get (&attrib);

			if (err == APIERR_DELETED) {
				WriteReport ("  [%3d]   DEL", i);
				err = NoError;
			} else if (err != NoError) {
				WriteReport ("  [%3d]   err:%d", i, err);
			} else {
				char guidStr[64];
				APIGuid2GSGuid (attrib.header.guid).ConvertToString (guidStr);
				char nameStr[256];
				CHTruncate (uniStringName.ToCStr (), nameStr, sizeof (nameStr));
				WriteReport ("  [%3d]   {%s}  \"%s\"", i, guidStr, nameStr);
			}

			DisposeAttribute (&attrib);
		}
	}

	WriteReport_End (err);

	return;
}		// Do_ListAttributes


// -----------------------------------------------------------------------------
// Rename all attributes	(append index)
// -----------------------------------------------------------------------------

static GSErrCode	Do_RenameAttributes (bool giveInfo)
{
	API_AttrTypeID		typeID;
	API_Attribute		attrib;
	API_AttributeDefExt	defs;
	short				count, i;
	char				postfix[32], buffer[256];
	GSErrCode			err = NoError;

	if (giveInfo) {
		WriteReport ("# Rename all attributes");
		WriteReport ("# (new name: append the attribute index");
		WriteReport ("#   - pens, fonts and ArchiCAD Layer cannot be edited");
		WriteReport ("# ArchiCAD menus must be updated");
		WriteReport ("# Elements must keep the references to the renamed attributes");
	}

	for (typeID = API_FirstAttributeID; typeID <= API_LastAttributeID; ((Int32 &) typeID)++) {
		if (giveInfo) {
			sprintf (buffer, "Renaming %ss...", AttrID_To_Name (typeID));
			WriteReport (buffer);
		}
		err = ACAPI_Attribute_GetNum (typeID, &count);
		for (i = 1; i <= count; i++) {
			BNZeroMemory (&attrib, sizeof (API_Attribute));
			attrib.header.typeID = typeID;
			attrib.header.index  = i;
			err = ACAPI_Attribute_Get (&attrib);
			if (err == NoError) {
				err = ACAPI_Attribute_GetDefExt (typeID, attrib.header.index, &defs);
				if (err == APIERR_BADID) {
					BNZeroMemory (&defs, sizeof (API_AttributeDefExt));
					err = NoError;
				}
				if (err == NoError) {
					if (attrib.header.typeID == API_LayerID && attrib.header.index == 1)
						strcpy (attrib.header.name, "ArchiCAD Layer");
					sprintf (postfix, " %d", i);
					attrib.header.name [API_AttrNameLen-6] = '\0';
					strcat (attrib.header.name, postfix);
					err = ACAPI_Attribute_ModifyExt (&attrib, &defs);
				}
				ACAPI_DisposeAttrDefsHdlsExt (&defs);
			}
			DisposeAttribute (&attrib);
		}
	}

	if (giveInfo)
		WriteReport_End (err);

	return err;
}		// Do_RenameAttributes


// -----------------------------------------------------------------------------
//  Copy all attributes
// -----------------------------------------------------------------------------

static void		Do_CopyAttributes (void)
{
	API_AttrTypeID		typeID;
	API_Attribute		attrib;
	API_AttributeDefExt	defs;
	short				count, i;
	bool				**attrMap;
	GSErrCode			err = NoError;

	WriteReport ("# Copy all attributes");
	WriteReport ("#   - font:   not editable");
	WriteReport ("#   - fill:   Solid, Empty, ind=1 not editable");
	WriteReport ("# Important:");
	WriteReport ("#   - copied instances may be created in place of deleted indices");
	WriteReport ("#   - so avoid recursion");
	WriteReport ("# ArchiCAD menus must be updated");
	WriteReport ("# Try this at different drawing scales");

	for (typeID = API_FirstAttributeID; typeID <= API_LastAttributeID; ((Int32 &) typeID)++) {
		err = ACAPI_Attribute_GetNum (typeID, &count);
		if (err != NoError)
			continue;

		attrMap = (bool **) BMAllocateHandle (count + 1, ALLOCATE_CLEAR, 0);
		if (attrMap != NULL) {
			for (i = 1; i <= count; i++) {
				BNZeroMemory (&attrib, sizeof (API_Attribute));
				attrib.header.typeID = typeID;
				attrib.header.index  = i;
				err = ACAPI_Attribute_Get (&attrib);
				if (err == NoError)
					(*attrMap) [i] = true;
				DisposeAttribute (&attrib);
			}
			err = NoError;
		} else
			err = APIERR_MEMFULL;

		for (i = 1; i <= count; i++) {
			if (!(*attrMap) [i])
				continue;

			BNZeroMemory (&attrib, sizeof (API_Attribute));
			attrib.header.typeID = typeID;
			attrib.header.index  = i;
			err = ACAPI_Attribute_Get (&attrib);
			if (err == NoError) {
				err = ACAPI_Attribute_GetDefExt (typeID, attrib.header.index, &defs);
				if (err == APIERR_BADID) {
					BNZeroMemory (&defs, sizeof (API_AttributeDefExt));
					err = NoError;
				}
				if (err == NoError) {
					if (attrib.header.typeID == API_LayerID && attrib.header.index == 1)
						strcpy (attrib.header.name, "ArchiCAD Layer");
					attrib.header.name [API_AttrNameLen-6] = '\0';
					strcat (attrib.header.name, " COPY");
					err = ACAPI_Attribute_CreateExt (&attrib, &defs);
				}
				ACAPI_DisposeAttrDefsHdlsExt (&defs);
			}
			DisposeAttribute (&attrib);
		}

		BMKillHandle ((GSHandle *) &attrMap);
	}

	WriteReport_End (err);

	return;
}		// Do_CopyAttributes


// -----------------------------------------------------------------------------
//  Delete all attributes
// -----------------------------------------------------------------------------

static GSErrCode	Do_DeleteAttributes (bool giveInfo)
{
	API_AttrTypeID	typeID;
	API_Attribute	attrib;
	short			count, i;
	GSErrCode		err = NoError;

	if (giveInfo) {
		WriteReport ("# Delete all attributes");
		WriteReport ("#   - ltype, layer:		ind=1 not deletable");
		WriteReport ("#   - pen, font:			not deletable");
		WriteReport ("#   - fill:				Solid, Empty not deletable");
		WriteReport ("#   - comp, mat, zcat, mepsys:	at least one attrib must remain");
		WriteReport ("#   - lcomb, city:		all deletable");
		WriteReport ("# ArchiCAD menus must be updated");
	}

	for (typeID = API_FirstAttributeID; typeID <= API_LastAttributeID; ((Int32 &) typeID)++) {
		err = ACAPI_Attribute_GetNum (typeID, &count);
		if (err != NoError)
			continue;
		for (i = 1; i <= count; i++) {
			BNZeroMemory (&attrib, sizeof (API_Attribute));
			attrib.header.typeID = typeID;
			attrib.header.index  = i;
			err = ACAPI_Attribute_Delete (&attrib.header);
		}
	}

	if (giveInfo)
		WriteReport_End (err);

	return err;
}		// Do_DeleteAttributes


// -----------------------------------------------------------------------------
//  Create a layer
// -----------------------------------------------------------------------------

static void		Do_CreateLayer_Training (void)
{
	API_Attribute	attrib;
	GSErrCode		err;

	WriteReport ("# Create a layer \"Training\"");
	WriteReport ("#   - lock status specified");
	WriteReport ("# ArchiCAD menus must be updated");
	WriteReport ("# Check the \"Layer Settings...\" dialog after execution");

	BNZeroMemory (&attrib, sizeof (API_Attribute));
	attrib.header.typeID = API_LayerID;
	attrib.header.flags  = APILay_Locked;
	strcpy (attrib.header.name, "Training_0123456789012345678901234567890123456789");

	err = ACAPI_Attribute_Create (&attrib, NULL);
	if (err == APIERR_ATTREXIST) {
		WriteReport_Alert ("The layer exists. Its index is: #%d", attrib.header.index);
		return;
	}
	if (err != NoError) {
		WriteReport_Err ("Unable to create layer \"Training\"", err);
		return;
	}

	WriteReport_Alert ("The layer index is: #%d", attrib.header.index);

	WriteReport_End (err);

	return;
}		// Do_CreateLayer_Training


// -----------------------------------------------------------------------------
// Create linetype attributes
//		- Dashed
//		- Symbol
// -----------------------------------------------------------------------------

static void		Do_CreateLinetypes_Training (void)
{
	API_DashItems		*dash;
	API_LineItems		*line;
	API_Attribute		line_atr;
	API_AttributeDef	line_atrdef;
	double				sq2;
	short				nItems;
	GSErrCode			err = NoError;

	WriteReport ("# Create linetype attributes:");
	WriteReport ("#   - dashed: \"Training Dashed\"");
	WriteReport ("#   - symbol: \"Training Symbol\"");
	WriteReport ("# ArchiCAD menus must be updated");
	WriteReport ("# Check the \"Line Types...\" dialog after execution");

	// Dashed linetype
	BNZeroMemory ((GSPtr) &line_atr, sizeof (API_Attribute));
	BNZeroMemory ((GSPtr) &line_atrdef, sizeof (API_AttributeDef));

	nItems = 3;
	line_atrdef.ltype_dashItems = (API_DashItems **) BMAllocateHandle (nItems * sizeof (API_DashItems), ALLOCATE_CLEAR, 0);
	if (line_atrdef.ltype_dashItems == NULL) {
		WriteReport_Err ("Memory full", 0);
		return;
	}
	dash = *line_atrdef.ltype_dashItems;
	dash[0].dash = 3.0;
	dash[0].gap  = 1.5;
	dash[1].dash = 0.0;
	dash[1].gap  = 1.5;
	dash[2].dash = 0.0;
	dash[2].gap  = 1.5;

	line_atr.linetype.head.typeID = API_LinetypeID;
	line_atr.linetype.head.flags  = APILine_ScaleWithPlan;
	line_atr.linetype.defineScale = 10.0;
	strcpy (line_atr.linetype.head.name, "Training Dashed");
	line_atr.linetype.period = 7.5;
	line_atr.linetype.height = 0.0;
	line_atr.linetype.type   = APILine_DashedLine;
	line_atr.linetype.nItems = nItems;

	err = ACAPI_Attribute_Create (&line_atr, &line_atrdef);
	if (err == APIERR_ATTREXIST) {
		err = ACAPI_Attribute_Modify (&line_atr, &line_atrdef);
	}
	ACAPI_DisposeAttrDefsHdls (&line_atrdef);
	if (err != NoError) {
		WriteReport_Err ("Error while creating the dashed linetype", err);
	} else {
		WriteReport ("Dashed linetype \"Training Dashed\" was created successfully.");
	}


	// Symbol linetype
	BNZeroMemory (&line_atr, sizeof (API_Attribute));
	BNZeroMemory (&line_atrdef, sizeof (API_AttributeDef));

	nItems = 3;
	line_atrdef.ltype_lineItems = (API_LineItems **) BMAllocateHandle (nItems * sizeof(API_LineItems), ALLOCATE_CLEAR, 0);
	if (line_atrdef.ltype_lineItems == NULL) {
		WriteReport_Err ("Memory full", 0);
		return;
	}
	line = *line_atrdef.ltype_lineItems;

	sq2 = sqrt (2.0);

	line[0].itemType = APILine_LineItemType;
	line[0].itemBegPos.x = 0.0;
	line[0].itemBegPos.y = 0.0;
	line[0].itemEndPos.x = 1.0;
	line[0].itemEndPos.y = 1.0;
	line[1].itemType = APILine_ArcItemType;
	line[1].itemBegPos.x = 0.0;
	line[1].itemBegPos.y = 2.0;
	line[1].itemRadius   = sq2;
	line[1].itemBegAngle = -45 * PI/180;
	line[1].itemEndAngle = 45 * PI/180;
	line[2].itemType = APILine_CircItemType;
	line[2].itemBegPos.x = 1.0 - 0.5/sq2;
	line[2].itemBegPos.y = 3.0 + 0.5/sq2;
	line[2].itemRadius   = 0.5;

	line_atr.linetype.head.typeID = API_LinetypeID;
	line_atr.linetype.head.flags  = APILine_FixScale;
	line_atr.linetype.defineScale = 0.0;
	strcpy (line_atr.linetype.head.name, "Training Symbol");
	line_atr.linetype.period = 4.5;
	line_atr.linetype.height = 2.0 * (3.5 + 0.5/sq2);
	line_atr.linetype.type   = APILine_SymbolLine;
	line_atr.linetype.nItems = nItems;

	err = ACAPI_Attribute_Create (&line_atr, &line_atrdef);
	if (err == APIERR_ATTREXIST) {
		err = ACAPI_Attribute_Modify (&line_atr, &line_atrdef);
	}
	ACAPI_DisposeAttrDefsHdls (&line_atrdef);
	if (err != NoError) {
		WriteReport_Err ("Error while creating the symbol linetype", err);
	} else {
		WriteReport ("Symbol linetype \"Training Symbol\" was created successfully.");
	}

	WriteReport_End (err);

	return;
}		// Do_CreateLinetypes_Training


// -----------------------------------------------------------------------------
// Create fill attributes
//		- Solid
//		- Empty
//		- Vector
//		- Symbol
// -----------------------------------------------------------------------------

static void		Do_CreateFills_Training (void)
{
	API_Attribute		fill_atr;
	API_AttributeDef	fill_atrdef;
	API_FillLine		*fillLine;
	API_Polygon			*poly;
	API_Coord			*coords;
	Int32				*subpoly;
	double				*lineLength;
	GSErrCode				err;

	WriteReport ("# Create the following fill attributes:");
	WriteReport ("#   - solid:  \"Training Solid\"");
	WriteReport ("#   - empty:  \"Training Empty\"");
	WriteReport ("#   - vector: \"Training Vector\"");
	WriteReport ("#   - symbol: \"Training Symbol\"");
	WriteReport ("# ArchiCAD menus must be updated");
	WriteReport ("# Check the \"Fill Types...\" dialog after execution");

	// APIFill_Solid
	BNZeroMemory (&fill_atr, sizeof (API_Attribute));
	BNZeroMemory (&fill_atrdef, sizeof (API_AttributeDef));

	fill_atr.header.typeID = API_FilltypeID;
	fill_atr.header.flags  = APIFill_ScaleWithPlan;
	strcpy (fill_atr.header.name, "Training Solid");
	fill_atr.filltype.subType = APIFill_Solid;

	BNZeroMemory (fill_atr.filltype.bitPat, sizeof (API_Pattern));

	err = ACAPI_Attribute_Create (&fill_atr, &fill_atrdef);
	if (err == APIERR_ATTREXIST) {	// This can not be modified
		err = NoError;
	}
	ACAPI_DisposeAttrDefsHdls (&fill_atrdef);
	if (err != NoError) {
		WriteReport_Err ("Error while creating the solid fill", err);
	} else {
		WriteReport ("The solid fill \"Training Solid\" was created successfully!");
	}

	// APIFill_Empty
	BNZeroMemory (&fill_atr, sizeof (API_Attribute));
	fill_atr.header.typeID = API_FilltypeID;
	fill_atr.header.flags  = APIFill_ScaleWithPlan;
	strcpy (fill_atr.header.name, "Training Empty");
	fill_atr.filltype.subType = APIFill_Empty;

	err = ACAPI_Attribute_Create (&fill_atr, &fill_atrdef);
	if (err == APIERR_ATTREXIST) {	// This can not be modified
		err = NoError;
	}
	ACAPI_DisposeAttrDefsHdls (&fill_atrdef);
	if (err != NoError) {
		WriteReport_Err ("Error while creating the empty fill", err);
	} else {
		WriteReport ("The empty fill \"Training Empty\" was created successfully!");
	}

	// APIFill_Vector
	BNZeroMemory (&fill_atr, sizeof (API_Attribute));
	BNZeroMemory (&fill_atrdef, sizeof (API_AttributeDef));
	fill_atr.header.typeID = API_FilltypeID;
	fill_atr.header.flags  = APIFill_ScaleWithPlan | APIFill_ForPoly;
	strcpy (fill_atr.header.name, "Training Vector");
	fill_atr.filltype.subType = APIFill_Vector;

	fill_atr.filltype.hXSpac  = 0.1;
	fill_atr.filltype.hYSpac  = 0.1;
	fill_atr.filltype.hAngle  = 0.0;
	fill_atr.filltype.linNumb = 4;
	fill_atr.filltype.arcNumb = 0;

												/*	32103210  */
	fill_atr.filltype.bitPat[0] = 0x80;			/*	*         */
	fill_atr.filltype.bitPat[1] = 0x80;			/*	*         */
	fill_atr.filltype.bitPat[2] = 0x80;			/*	*         */
	fill_atr.filltype.bitPat[3] = 0x8F;			/*	*   ****  */
	fill_atr.filltype.bitPat[4] = 0x88;			/*	*   *     */
	fill_atr.filltype.bitPat[5] = 0x88;			/*	*   *     */
	fill_atr.filltype.bitPat[6] = 0x88;			/*	*   *     */
	fill_atr.filltype.bitPat[7] = 0xF8;			/*	*****     */

	fill_atrdef.fill_lineItems = (API_FillLine **) BMAllocateHandle (fill_atr.filltype.linNumb*sizeof (API_FillLine), ALLOCATE_CLEAR, 0);
	if (fill_atrdef.fill_lineItems == NULL) {
		WriteReport_Err ("Memory full", 0);
	}
	fill_atrdef.fill_lineLength = (double **) BMAllocateHandle (2*fill_atr.filltype.linNumb*sizeof (double), ALLOCATE_CLEAR, 0);
	if (fill_atrdef.fill_lineLength == NULL) {
		WriteReport_Err ("Memory full", 0);
	}
	fillLine = *fill_atrdef.fill_lineItems;
	lineLength = *fill_atrdef.fill_lineLength;
	BNZeroMemory (fillLine, fill_atr.filltype.linNumb * sizeof (API_FillLine));
	BNZeroMemory (lineLength, 2 * fill_atr.filltype.linNumb * sizeof (double));

	fillLine[0].lFreq = 2.0;
	fillLine[0].lDir  = 90.0 * DEGRAD;
	fillLine[0].lOffsetLine = 0.0;
	fillLine[0].lOffset.x = 0.0;
	fillLine[0].lOffset.y = 0.0;
	fillLine[0].lPartNumb = 2;
	fillLine[0].lPartOffs = 0;
	lineLength[0] = 2.0;
	lineLength[1] = 0.0;

	fillLine[1].lFreq = 2.0;
	fillLine[1].lDir  = 0.0;
	fillLine[1].lOffsetLine = 0.0;
	fillLine[1].lOffset.x = 0.0;
	fillLine[1].lOffset.y = 0.0;
	fillLine[1].lPartNumb = 2;
	fillLine[1].lPartOffs = 2;
	lineLength[2] = 1.0;
	lineLength[3] = 1.0;

	fillLine[2].lFreq = 2.0;
	fillLine[2].lDir  = 90.0 * DEGRAD;
	fillLine[2].lOffsetLine = 0.0;
	fillLine[2].lOffset.x = 1.0;
	fillLine[2].lOffset.y = 0.0;
	fillLine[2].lPartNumb = 2;
	fillLine[2].lPartOffs = 4;
	lineLength[4] = 1.0;
	lineLength[5] = 1.0;

	fillLine[3].lFreq = 2.0;
	fillLine[3].lDir  = 0.0;
	fillLine[3].lOffsetLine = 0.0;
	fillLine[3].lOffset.x = 1.0;
	fillLine[3].lOffset.y = 1.0;
	fillLine[3].lPartNumb = 2;
	fillLine[3].lPartOffs = 6;
	lineLength[6] = 1.0;
	lineLength[7] = 1.0;


	err = ACAPI_Attribute_Create (&fill_atr, &fill_atrdef);
	if (err == APIERR_ATTREXIST) {
		err = ACAPI_Attribute_Modify (&fill_atr, &fill_atrdef);
	}
	ACAPI_DisposeAttrDefsHdls (&fill_atrdef);
	if (err != NoError) {
		WriteReport_Err ("Error while creating the vectorial fill", err);
	} else {
		WriteReport ("The vectorial fill \"Training Vector\" was created successfully!");
	}

	// APIFill_Symbol
	BNZeroMemory (&fill_atr, sizeof (API_Attribute));
	BNZeroMemory (&fill_atrdef, sizeof (API_AttributeDef));
	fill_atr.header.typeID = API_FilltypeID;
	fill_atr.header.flags  = APIFill_ScaleWithPlan | APIFill_ForPoly;
	strcpy (fill_atr.header.name, "Training Symbol");
	fill_atr.filltype.subType = APIFill_Symbol;

	fill_atr.filltype.hXSpac  = 1.0;
	fill_atr.filltype.hYSpac  = 1.0;
	fill_atr.filltype.hAngle  = 0.0;
	fill_atr.filltype.linNumb = 0;
	fill_atr.filltype.arcNumb = 0;
	fill_atr.filltype.filNumb = 1;
	fill_atr.filltype.c1.x = 6.0;
	fill_atr.filltype.c1.y = 1.0;
	fill_atr.filltype.c2.x = 0.0;
	fill_atr.filltype.c2.y = 8.0;

												/*	32103210  */
	fill_atr.filltype.bitPat[0] = 0x80;			/*	*         */
	fill_atr.filltype.bitPat[1] = 0x80;			/*	*         */
	fill_atr.filltype.bitPat[2] = 0x80;			/*	*         */
	fill_atr.filltype.bitPat[3] = 0x8F;			/*	*   ****  */
	fill_atr.filltype.bitPat[4] = 0x88;			/*	*   *     */
	fill_atr.filltype.bitPat[5] = 0x88;			/*	*   *     */
	fill_atr.filltype.bitPat[6] = 0x88;			/*	*   *     */
	fill_atr.filltype.bitPat[7] = 0xF8;			/*	*****     */

	fill_atrdef.sfill_Items.sfill_SolidFills = (API_Polygon **) BMAllocateHandle (sizeof (API_Polygon), ALLOCATE_CLEAR, 0);
	if (fill_atrdef.sfill_Items.sfill_SolidFills) {
		poly = *fill_atrdef.sfill_Items.sfill_SolidFills;
		BNZeroMemory (poly, sizeof (API_Polygon));
		poly->nCoords   = 10;
		poly->nSubPolys = 1;
		fill_atrdef.sfill_Items.sfill_FillCoords = (API_Coord **) BMAllocateHandle (poly->nCoords * sizeof (API_Coord), ALLOCATE_CLEAR, 0);
		if (fill_atrdef.sfill_Items.sfill_FillCoords) {
			coords = *fill_atrdef.sfill_Items.sfill_FillCoords;
			BNZeroMemory (coords, poly->nCoords * sizeof (API_Coord));
			coords[0].x = coords[0].y = 0.0;
			coords[1].x = 0.0;	coords[1].y = 0.0;
			coords[2].x = 2.0;	coords[2].y = 0.0;
			coords[3].x = 2.0;	coords[3].y = 2.0;
			coords[4].x = 4.0;	coords[4].y = 2.0;
			coords[5].x = 4.0;	coords[5].y = 6.0;
			coords[6].x = 2.0;	coords[6].y = 6.0;
			coords[7].x = 2.0;	coords[7].y = 4.0;
			coords[8].x = 0.0;	coords[8].y = 4.0;
			coords[9] = coords[1];

			fill_atrdef.sfill_Items.sfill_SubPolys = (Int32 **) BMAllocateHandle (2 * sizeof (Int32), ALLOCATE_CLEAR, 0);
			if (fill_atrdef.sfill_Items.sfill_SubPolys) {
				subpoly = *fill_atrdef.sfill_Items.sfill_SubPolys;
				BNZeroMemory (subpoly, 2 * sizeof (Int32));
				subpoly[0] = 0;
				subpoly[1] = 9;			/* poly->nCoords - 1 */
			}
		}
	}

	err = ACAPI_Attribute_Create (&fill_atr, &fill_atrdef);
	if (err == APIERR_ATTREXIST) {
		err = ACAPI_Attribute_Modify (&fill_atr, &fill_atrdef);
	}
	ACAPI_DisposeAttrDefsHdls (&fill_atrdef);
	if (err != NoError) {
		WriteReport_Err ("Error while creating the symbol fill", err);
	} else {
		WriteReport ("The symbol fill \"Training Symbol\" was created succesfully!");
	}

	WriteReport_End (err);

	return;
}		// Do_CreateFills_Training


// -----------------------------------------------------------------------------
//  List one profile
// -----------------------------------------------------------------------------

static const char* AttributeName (API_AttrTypeID inType, short inIndex)
{
	API_Attribute	attr;
	static char buffer[256] = {0};

	BNZeroMemory (buffer, sizeof (buffer));
	BNZeroMemory (&attr, sizeof (attr));
	attr.header.typeID = inType;
	attr.header.index  = inIndex;
	if (ACAPI_Attribute_Get (&attr) == NoError) {
		CHTruncate (attr.header.name, buffer, sizeof (buffer));
	} else
		buffer[0] = ' ';

	return buffer;
}

static void	ListProfileDescription (const VectorImage& profileDescription)
{
	ConstVectorImageIterator profileDescriptionIt (profileDescription);

	while (!profileDescriptionIt.IsEOI ()) {
		switch (profileDescriptionIt->item_Typ) {
			case SyHots: {
					const Sy_HotType* pSyHot = ((const Sy_HotType*) profileDescriptionIt);
					WriteReport ("\tHotspot");
					WriteReport ("\t\tCoordinate: %.3f, %.3f", pSyHot->c.x, pSyHot->c.y);
				}
				break;

			case SyLine: {
					const Sy_LinType* pSyLine = static_cast <const Sy_LinType*> (profileDescriptionIt);
					WriteReport ("\tLine");
					WriteReport ("\t\tLayer: %d; SpecForProfile: %d", pSyLine->sy_layer, pSyLine->specForProfile);
				}
				break;

			case SyPolyLine: {
					const Sy_PolyLinType* pSyPolyLine = static_cast <const Sy_PolyLinType*> (profileDescriptionIt);
					WriteReport ("\tLine");
					WriteReport ("\t\tLayer: %d", pSyPolyLine->sy_layer);
				}
				break;

			case SyHatch: {
					const Sy_HatchType* pSyHatch = static_cast <const Sy_HatchType*> (profileDescriptionIt);
					GSConstPtr			hatchCharPtr	= reinterpret_cast<GSConstPtr> (pSyHatch);
					const ProfileItem*	profileItemInfo	= reinterpret_cast<const ProfileItem*> (hatchCharPtr + pSyHatch->addInfoOff);

					WriteReport ("\tHatch");
					Int32 uiPriority = 0;
					API_BuildingMaterialType	buildMat;
					BNZeroMemory (&buildMat, sizeof (API_BuildingMaterialType));
					buildMat.head.typeID = API_BuildingMaterialID;
					buildMat.head.index = pSyHatch->buildMatIdx;
					ACAPI_Attribute_Get ((API_Attribute*)&buildMat);
					ACAPI_Goodies (APIAny_Elem2UIPriorityID, (void*) &buildMat.connPriority, &uiPriority);
					WriteReport ("\t\tPriority: %d%s", uiPriority,
								 profileItemInfo->IsCore () ? ", Core component" : "");
					WriteReport ("\t\tCoordinates: %ld", pSyHatch->nCoords);
					const API_Coord*	coords = reinterpret_cast<const API_Coord*> (hatchCharPtr + pSyHatch->coorOff);
					UInt32				nCoords = pSyHatch->coorLen / sizeof (API_Coord), ii;
					for (ii = 0; ii < nCoords; ii++, coords++) {
						WriteReport ("\t\t\tCoord #%-2d (%5.2f, %5.2f)", ii, coords->x, coords->y);
					}
					WriteReport ("\t\tAdditional info: len = %ld, offset = %ld", pSyHatch->addInfoLen, pSyHatch->addInfoOff);
//					const ProfileItem*		profileItem GCC_UNUSED = reinterpret_cast<const ProfileItem*> (hatchCharPtr + pSyHatch->addInfoOff);
					const ProfileEdgeData*	profileEdgeData = reinterpret_cast<const ProfileEdgeData*> (hatchCharPtr + pSyHatch->addInfoOff + sizeof (ProfileItem));
					for (ii = 0; ii < nCoords; ii++, profileEdgeData++) {
						WriteReport ("\t\t\tEdge data #%-2d surface = %32s (%3ld) pen = (%3ld)", ii, AttributeName (API_MaterialID, profileEdgeData->GetMaterial ()),
									 profileEdgeData->GetMaterial (), profileEdgeData->GetPen ());
					}
				}
				break;

			case SyArc:
			case SyText:
			case SySpline:
			case SyPicture:
			case SyPixMap:
			case SyPointCloud:
				break;
		}
		++profileDescriptionIt;
	}
}		// ListProfileDescription


// -----------------------------------------------------------------------------
//  List all details of all profiles
// -----------------------------------------------------------------------------

static void		Do_ListAllProfileDetails (void)
{
	API_Attribute		attrib;
	API_AttributeDefExt	defs;
	short				count, i;
	GSErrCode			err = NoError;

	WriteReport ("Detailed listing of Profiles");

	err = ACAPI_Attribute_GetNum (API_ProfileID, &count);
	if (err != NoError)
		return;

	for (i = 1; i <= count; i++) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.header.typeID = API_ProfileID;
		attrib.header.index  = i;
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			err = ACAPI_Attribute_GetDefExt (API_ProfileID, attrib.header.index, &defs);
			if (err == APIERR_BADID) {
				BNZeroMemory (&defs, sizeof (API_AttributeDefExt));
				err = NoError;
			}
			if (err == NoError) {
				WriteReport ("\n\n****Profile name: %s", attrib.header.name);
				VectorImage profileDescription;
				try { profileDescription.Import (defs.profile_vectorImageItems); } catch (const GS::Exception&) {}
				ListProfileDescription (profileDescription);
			}
			ACAPI_DisposeAttrDefsHdlsExt (&defs);
		}
	}

	WriteReport_End (err);

	return;
}		// Do_ListAllProfileDetails


// -----------------------------------------------------------------------------
//  List all details of all Operation Profiles
// -----------------------------------------------------------------------------

static void		Do_ListAllOpProfileDetails (void)
{
	API_Attribute		attrib;
	API_AttributeDefExt	defs;
	short				count, i;
	GSErrCode			err = NoError;

	WriteReport ("Detailed listing of OperationProfiles");

	err = ACAPI_Attribute_GetNum (API_OperationProfileID, &count);
	if (err != NoError)
		return;

	for (i = 1; i <= count; i++) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.header.typeID = API_OperationProfileID;
		attrib.header.index  = i;
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			WriteReport ("\n\n****Profile name: %s", attrib.header.name);
			WriteReport ("\tOccupancyType: %d", attrib.operationProfile.occupancyType);
			WriteReport ("\tHot water: %.3f", attrib.operationProfile.hotWaterLoad);
			WriteReport ("\tHuman heat gain: %.3f", attrib.operationProfile.humanHeatGain);
			WriteReport ("\tHumidity: %.3f", attrib.operationProfile.humidity);
			err = ACAPI_Attribute_GetDefExt (API_OperationProfileID, attrib.header.index, &defs);
			if (err == APIERR_BADID) {
				BNZeroMemory (&defs, sizeof (API_AttributeDefExt));
				err = NoError;
			}
			if (err == NoError) {
			}
			Int32 maxHours = BMGetHandleSize ((GSHandle)defs.op_hourlyProfiles) / sizeof (API_HourlyProfile);

			for (Int32 p = 0; p < maxHours; p += 6) {
				WriteReport ("\n\n\t****DailyProfile name: %s for hour: %d", (*defs.op_hourlyProfiles)[p].name, p);
				WriteReport ("\t\tminTemp: %.3f ", (*defs.op_hourlyProfiles)[p].minTemp);
				WriteReport ("\t\tmaxTemp: %.3f ", (*defs.op_hourlyProfiles)[p].maxTemp);
				WriteReport ("\t\toccupancyCount: %.3f ", (*defs.op_hourlyProfiles)[p].occupancyCount);
				WriteReport ("\t\tlightGain: %.3f ", (*defs.op_hourlyProfiles)[p].lightGain);
				WriteReport ("\t\tequipmentGain: %.3f", (*defs.op_hourlyProfiles)[p].equipmentGain);
			}
			ACAPI_DisposeAttrDefsHdlsExt (&defs);
		}
	}

	WriteReport_End (err);
}		// Do_ListAllOpProfileDetails


// -----------------------------------------------------------------------------
//  List all details of all Building Materials
// -----------------------------------------------------------------------------

static void		Do_ListAllBuildingMaterials (void)
{
	API_Attribute		attrib;
	short				count, i;
	GSErrCode			err = NoError;

	WriteReport ("Detailed listing of Building Materials");

	err = ACAPI_Attribute_GetNum (API_BuildingMaterialID, &count);
	if (err != NoError)
		return;

	for (i = 1; i <= count; i++) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.header.typeID = API_BuildingMaterialID;
		attrib.header.index  = i;
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			GS::UniString cutFillOrientationString;
			if (attrib.buildingMaterial.cutFillOrientation == APIFillOrientation_ProjectOrigin)
				cutFillOrientationString = "ProjectOrigin";
			else if (attrib.buildingMaterial.cutFillOrientation == APIFillOrientation_ElementOrigin)
				cutFillOrientationString = "ElementOrigin";
			else if (attrib.buildingMaterial.cutFillOrientation == APIFillOrientation_FitToSkin)
				cutFillOrientationString = "FitToSkin";

			WriteReport ("\n\n****Building Material #%d name: \"%s\"", attrib.header.index, attrib.header.name);
			WriteReport ("\tConnection priority: %d", attrib.buildingMaterial.connPriority);
			WriteReport ("\tCut fill index: %d", attrib.buildingMaterial.cutFill);
			WriteReport ("\tCut fill pen index: %d", attrib.buildingMaterial.cutFillPen);
			WriteReport ("\tCut fill background pen index: %d", attrib.buildingMaterial.cutFillBackgroundPen);
			WriteReport ("\tSurface index: %d", attrib.buildingMaterial.cutMaterial);
			WriteReport ("\tCut fill orientation: %s", cutFillOrientationString.ToCStr ().Get ());
			WriteReport ("\tThermal conductivity: %.3f", attrib.buildingMaterial.thermalConductivity);
			WriteReport ("\tDensity: %.3f", attrib.buildingMaterial.density);
			WriteReport ("\tHeat capacity: %.3f", attrib.buildingMaterial.heatCapacity);
		}
	}

	WriteReport_End (err);
}		// Do_ListAllBuildingMaterials


// -----------------------------------------------------------------------------
//  Creates the internal structure of a profile attribute
// -----------------------------------------------------------------------------

static void	BuildProfileDescription (VectorImage* image)
{
	const short		nCoords = 5;
	Coord			coords[nCoords + 1] = {Coord(0.0, 0.0), Coord(0.0, 0.0), Coord(1.0, 0.0), Coord(1.0, 1.0), Coord(0.0, 1.0), Coord(0.0, 0.0)};
	Int32			size = sizeof (ProfileItem) + (nCoords + 1) * sizeof (ProfileEdgeData);
	GSHandle		addInfo = BMAllocateHandle (size, ALLOCATE_CLEAR, 0);
	Int32			boends[2] = { 0, nCoords };

	if (!DBERROR (addInfo == NULL)) {
		BNZeroMemory (*addInfo, size);

		ProfileItem*	profileItem = reinterpret_cast<ProfileItem*>(*addInfo);
		profileItem->obsoletePriorityValue = 0;		// not used
		profileItem->profileItemVersion = ProfileItemVersion;
		profileItem->SetCutEndLinePen (5);
		profileItem->SetCutEndLineType (5);
		profileItem->SetVisibleCutEndLines (true);
		profileItem->SetCore (true);

		ProfileEdgeData*	profileEdgeData = reinterpret_cast<ProfileEdgeData*>(reinterpret_cast<char*>(*addInfo) + sizeof (ProfileItem));

		profileEdgeData[0].SetPen (0);
		profileEdgeData[0].SetLineType (0);
		profileEdgeData[0].SetMaterial (0);
		profileEdgeData[0].SetFlags (0);

		for (short i = 1; i <= nCoords; i++) {
			profileEdgeData[i].SetPen (1);
			profileEdgeData[i].SetLineType (1);
			profileEdgeData[i].SetMaterial (i);		// set different material attribute for each edges
			profileEdgeData[i].SetFlags (ProfileEdgeData::IsVisibleLineFlag);
		}
	}

	PlaneEq pe;

	GX::Pattern::HatchTran	hatchTrafo;
	hatchTrafo.SetGlobal ();

	try {
		image->AddHatchWhole (true,								// hatch contour visible
							  5,								// drawing pen index of the hatch contour
							  5,								// line type attribute index of the hatch contour
							  7,								// building material
							  7,								// fill attribute index of the hatch
							  VBUtil::OverriddenPen (VBUtil::UseThisAttribute, 4),	// override pen index of the fill (false means no override, pen coming from building material)
							  VBUtil::OverriddenPen (VBUtil::UseThisAttribute, 12),	// override pen index of the fill background (zero means transparent; false means no override, pen coming from building material)
							  1,								// layer index of the hatch [1..5]
							  DrwIndexForHatches,				// drawing index of the hatch, determining the drawing order of the item
							  GS::NULLGuid,						// reserved for internal use, should be NULLGuid
							  hatchTrafo,
							  1,								// number of the polygon contours
							  boends,							// ending vertex indices of contours
							  nCoords,							// number of vertices of the polygon
							  coords,							// vertex coordinates (array of pairs of double values)
							  NULL,								// view angles of the edges, applicable if the polygon contains curved edges
							  pe,
							  DefaultDisplayOrder,
							  0,								// fill category: 0 - DraftingFills, 1 - CutFills, 2 - CoverFills
							  addInfo);							// profile related additional parameters
	} catch (const GS::Exception&) {}

	if (addInfo != NULL)
		BMKillHandle (&addInfo);

}		// BuildProfileDescription


// -----------------------------------------------------------------------------
// Create a profile attribute
//		- Symbol
// -----------------------------------------------------------------------------

static void		Do_CreateProfile (void)
{
	API_Attribute		profile_attr;
	API_AttributeDefExt	profile_attrdef;
	GSErrCode			err = NoError;

	WriteReport ("# Create a profile attribute:");
	WriteReport ("# ArchiCAD menus must be updated");
	WriteReport ("# Check the \"Profiles...\" dialog after execution");

	BNZeroMemory ((GSPtr) &profile_attr, sizeof (API_Attribute));
	BNZeroMemory ((GSPtr) &profile_attrdef, sizeof (API_AttributeDefExt));

	profile_attr.header.typeID = API_ProfileID;
	profile_attr.header.flags  = 0;
	strcpy (profile_attr.header.name, "Profile from API");
	profile_attr.profile.wallType = true;
	profile_attr.profile.beamType = false;
	profile_attr.profile.coluType = true;

	VectorImage profileDescription;
	BuildProfileDescription (&profileDescription);
	profile_attrdef.profile_vectorImageItems = BMhAll (0);
	try { profileDescription.Export (profile_attrdef.profile_vectorImageItems); } catch (const GS::Exception&) {}

	err = ACAPI_Attribute_CreateExt (&profile_attr, &profile_attrdef);
	if (err == APIERR_ATTREXIST) {
		err = ACAPI_Attribute_ModifyExt (&profile_attr, &profile_attrdef);
	}
	ACAPI_DisposeAttrDefsHdlsExt (&profile_attrdef);
	if (err != NoError) {
		WriteReport_Err ("Error while creating the profile", err);
	} else {
		WriteReport ("Profile \"Profile from API\" was created successfully.");
	}

	WriteReport_End (err);

	return;
}		// Do_CreateProfile


// -----------------------------------------------------------------------------
// Create a pen table attribute
// -----------------------------------------------------------------------------

static void		Do_CreatePenTable (void)
{
	API_Attribute		attrib;
	API_AttributeDefExt	attrdef;
	GSErrCode			err = NoError;

	WriteReport ("# Create a pen table attribute:");
	WriteReport ("# ArchiCAD menus must be updated");
	WriteReport ("# Check the \"Pens and Colors...\" dialog after execution");

	BNZeroMemory ((GSPtr) &attrib, sizeof (API_Attribute));
	BNZeroMemory ((GSPtr) &attrdef, sizeof (API_AttributeDefExt));

	attrib.header.typeID = API_PenTableID;
	attrib.header.flags  = 0;
	strcpy (attrib.header.name, "Gray pen table from API");

	short numOfPens = 255;

	attrdef.penTable_Items = (API_PenType **) BMhAllClear (numOfPens * sizeof (API_PenType));
	if (attrdef.penTable_Items == NULL) {
		WriteReport ("!!! Not enough memory for pen table pens");
		return;
	}

	API_PenType		*pen = *attrdef.penTable_Items;
	for (short ii = 1; ii <= numOfPens; ii++, pen++) {
		pen->head.typeID = API_PenID;
		pen->head.index  = ii;
		pen->rgb.f_red   = ii / 255.0;
		pen->rgb.f_green = ii / 255.0;
		pen->rgb.f_blue  = ii / 255.0;
		pen->width       = 0.0;
	}

	err = ACAPI_Attribute_CreateExt (&attrib, &attrdef);
	if (err == APIERR_ATTREXIST) {
		err = ACAPI_Attribute_ModifyExt (&attrib, &attrdef);
	}
	ACAPI_DisposeAttrDefsHdlsExt (&attrdef);

	if (err != NoError) {
		WriteReport_Err ("Error while creating the pen table", err);
	} else {
		WriteReport ("Pen table \"Gray pen table from API\" was created successfully.");
	}

	WriteReport_End (err);

	return;
}		// Do_CreatePenTable


// -----------------------------------------------------------------------------
// Create a MEP System attribute
// -----------------------------------------------------------------------------

static void		Do_CreateMEPSystem (void)
{
	API_Attribute		mepSystem_atr;
	GSErrCode			err;

	WriteReport ("# Create a new MEPSystem attribute:");
	WriteReport ("# Check the \"MEP Systems...\" dialog after execution");

	BNZeroMemory (&mepSystem_atr, sizeof (API_Attribute));

	mepSystem_atr.header.typeID = API_MEPSystemID;
	mepSystem_atr.header.flags  = 0;
	strcpy (mepSystem_atr.header.name, "Test System");

	mepSystem_atr.mepSystem.isForDuctwork = true;
	mepSystem_atr.mepSystem.isForPipework = false;
	mepSystem_atr.mepSystem.isForCabling = true;

	mepSystem_atr.mepSystem.contourPen = 5;
	mepSystem_atr.mepSystem.fillPen = 4;
	mepSystem_atr.mepSystem.fillBgPen = -1;
	mepSystem_atr.mepSystem.fillInd = 1;
	mepSystem_atr.mepSystem.centerLTypeInd = 4;
	mepSystem_atr.mepSystem.centerLinePen = 6;
	mepSystem_atr.mepSystem.materialInd = 55;
	mepSystem_atr.mepSystem.insulationMatInd = 45;

	err = ACAPI_Attribute_Create (&mepSystem_atr, NULL);
	if (err == APIERR_ATTREXIST) {	// This can not be modified
		err = NoError;
	}

	if (err != NoError) {
		WriteReport_Err ("Error while creating the new MEPSystem", err);
	} else {
		WriteReport ("The MEPSystem \"Test System\" was created successfully!");
	}

	WriteReport_End (err);

	return;
}		// Do_CreateMEPSystem


// -----------------------------------------------------------------------------
// Create a Building Material attribute
// -----------------------------------------------------------------------------

static void		Do_CreateBuildingMaterial (void)
{
	API_Attribute attrib;
	BNZeroMemory (&attrib, sizeof (API_Attribute));

	attrib.header.typeID = API_BuildingMaterialID;
	strcpy (attrib.header.name, "Test Building Material");

	Int32 uiPriority = 8;
	Int32 elemPriority = 0;
	ACAPI_Goodies (APIAny_UI2ElemPriorityID, &uiPriority, &elemPriority);
	attrib.buildingMaterial.connPriority = elemPriority;
	attrib.buildingMaterial.cutFill = 1;
	attrib.buildingMaterial.cutFillPen = 4;
	attrib.buildingMaterial.cutFillBackgroundPen = 0;
	attrib.buildingMaterial.cutMaterial = 75;
	attrib.buildingMaterial.cutFillOrientation = APIFillOrientation_ProjectOrigin;
	attrib.buildingMaterial.thermalConductivity = 0.4;
	attrib.buildingMaterial.density = 1500.0;
	attrib.buildingMaterial.heatCapacity = 700.0;
	attrib.buildingMaterial.showUncutLines = true;

	GSErrCode err = ACAPI_Attribute_Create (&attrib, NULL);
	if (err != NoError) {
		WriteReport_Err ("Error while creating the new Building Material", err);
	} else {
		WriteReport ("\"Test Building Material\" attribute was created successfully!");
	}

	WriteReport_End (err);

	return;
}		// Do_CreateBuildingMaterial


// -----------------------------------------------------------------------------
// Hide the layer of lines : "2D Drafting - General"
// -----------------------------------------------------------------------------

static void		Do_HideLayerOfLines (void)
{
	API_Attribute	attrib;
	GSErrCode			err;

	WriteReport ("# Hide the layer: \"2D Drafting - General\"");
	WriteReport ("#   - should work in each Teamwork state");
	WriteReport ("# Check the \"Layer Settings...\" dialog after execution");

	BNZeroMemory (&attrib, sizeof (API_Attribute));
	attrib.header.typeID = API_LayerID;
	strcpy (attrib.header.name, "2D Drafting - General");

	err = ACAPI_Attribute_Search (&attrib.header);		/* search by name: index returned in header */
	if (err != NoError) {
		WriteReport_Err ("Layer \"2D Drafting - General\" was not found", err);
		return;
	}
	err = ACAPI_Attribute_Get (&attrib);
	if (err != NoError) {
		WriteReport_Err ("Unable to get the layer definition", err);
		return;
	}

	attrib.layer.head.flags |= APILay_Hidden;
	err = ACAPI_Attribute_Modify (&attrib, NULL);
	if (err != NoError) {
		WriteReport_Err ("Unable to modify the layer \"2D Drafting - General\"", err);
		return;
	}

	WriteReport_End (err);

	return;
}		// Do_HideLayerOfLines


// -----------------------------------------------------------------------------
// Modify the linetype: Wave
// -----------------------------------------------------------------------------

static void		Do_ChangeLinetype_Wave (void)
{
	API_Attribute		attrib;
	API_AttributeDef	def;
	GSErrCode				err;

	WriteReport ("# Change the linetype: \"Wave\"");
	WriteReport ("#   - add a centerline to the linetype");
	WriteReport ("# Check the \"Line Types...\" dialog after execution");

	BNZeroMemory (&attrib, sizeof (API_Attribute));
	BNZeroMemory (&def, sizeof (API_AttributeDef));

	attrib.header.guid = GSGuid2APIGuid (GS::Guid ("35DE8210-A51E-43AE-9837-8CF3A01E8F63"));	// GUID of "Wave" linetype attribute
	err = ACAPI_Attribute_Search (&attrib.header);			/* search by guid: type and index returned */
	if (err != NoError) {
		GS::UniString uniStringName ("Wave");
		attrib.header.uniStringNamePtr = &uniStringName;
		attrib.header.typeID = API_LinetypeID;
		attrib.header.guid = APINULLGuid;
		err = ACAPI_Attribute_Search (&attrib.header);		/* search by name and type: index returned */
		attrib.header.uniStringNamePtr = NULL;
	}
	if (err != NoError) {
		WriteReport_Err ("Linetype \"Wave\" was not found", err);
		return;
	}
	err = ACAPI_Attribute_Get (&attrib);
	if (err == NoError)
		err = ACAPI_Attribute_GetDef (attrib.header.typeID, attrib.header.index, &def);
	if (err != NoError) {
		WriteReport_Err ("Unable to get the linetype definition", err);
		return;
	}

	def.ltype_lineItems = (API_LineItems **) BMReallocHandle ((GSHandle) def.ltype_lineItems,
																 (attrib.linetype.nItems + 1) * sizeof (API_LineItems), REALLOC_CLEAR, 0);
	if (def.ltype_lineItems == NULL)
		return;

	/* add a centerline to the linetype */
	BNZeroMemory (&(*def.ltype_lineItems) [attrib.linetype.nItems], sizeof (API_LineItems));
	(*def.ltype_lineItems) [attrib.linetype.nItems].itemType   = APILine_CenterLineItemType;
	(*def.ltype_lineItems) [attrib.linetype.nItems].itemLength = attrib.linetype.period;
	attrib.linetype.nItems ++;

	err = ACAPI_Attribute_Modify (&attrib, &def);
	ACAPI_DisposeAttrDefsHdls (&def);
	if (err != NoError) {
		WriteReport_Err ("Unable to modify the linetype \"Wave\"", err);
		return;
	}

	WriteReport_End (err);

	return;
}		// Do_ChangeLinetype_Wave


#ifdef __APPLE__
#pragma mark -
#endif

// =============================================================================
//
// TeamWork environment
//
// =============================================================================

// -----------------------------------------------------------------------------
// Create an attribute in TeamWork project
// -----------------------------------------------------------------------------

static void		Do_CreateAttributes_TW (void)
{
	API_AttrTypeID		typeID;
	API_Attribute		attrib;
	API_AttributeDefExt	defs;
	short				count, i;
	GSErrCode			err = NoError;

	WriteReport ("# Create an attribute in TeamWork project: \"My TW Attribute\"");
	WriteReport ("# Copy the first attribute instance of each type");
	WriteReport ("#   - FullAccess:  should work on each");
	WriteReport ("#   - ViewOnly:    refused");
	WriteReport ("#   - Client:      the ClientBit should be set");
	WriteReport ("#   				 for layers: the reservation must be extended");
	WriteReport ("# Check all attribute setting dialogs after execution");
	WriteReport ("# Try it in all Teamwork modes");

	for (typeID = API_FirstAttributeID; typeID <= API_LastAttributeID; ((Int32 &) typeID)++) {
		err = ACAPI_Attribute_GetNum (typeID, &count);
		for (i = 1; i <= count; i++) {
			BNZeroMemory (&attrib, sizeof (API_Attribute));
			attrib.header.typeID = typeID;
			attrib.header.index  = i;
			err = ACAPI_Attribute_Get (&attrib);
			if (err == NoError) {
				err = ACAPI_Attribute_GetDefExt (typeID, attrib.header.index, &defs);
				if (err == APIERR_BADID) {
					err = NoError;
				}
				if (err == NoError) {
					strcpy (attrib.header.name, "\"My TW Attribute\"");
					err = ACAPI_Attribute_CreateExt (&attrib, &defs);
					ACAPI_DisposeAttrDefsHdlsExt (&defs);
					DisposeAttribute (&attrib);
					break;
				}
				ACAPI_DisposeAttrDefsHdlsExt (&defs);
				DisposeAttribute (&attrib);
			}
		}
	}

	WriteReport_End (err);

	return;
}		// Do_CreateAttributes_TW


// -----------------------------------------------------------------------------
// Rename all attributes in a TeamWork project
// -----------------------------------------------------------------------------

static void		Do_RenameMyAttributes_TW (void)
{
	GSErrCode	err;

	WriteReport ("# Rename all attributes in a TeamWork project");
	WriteReport ("#   - ViewOnly:    disabled");
	WriteReport ("#   - Client:      should work only on: \"My TW Attribute\"");
	WriteReport ("#   - FullAccess:  should work on each");
	WriteReport ("# Check all attribute setting dialogs after execution");
	WriteReport ("# Execute it in each TeamWork mode");

	err = Do_RenameAttributes (false);

	WriteReport_End (err);

	return;
}		// Do_RenameMyAttributes_TW


// -----------------------------------------------------------------------------
// Delete all attributes in TeamWork mode
// -----------------------------------------------------------------------------

static void		Do_DeleteMyAttributes_TW (void)
{
	GSErrCode	err;

	WriteReport ("# Delete all attributes in TeamWork mode (each type)");
	WriteReport ("#   - ViewOnly:    disabled");
	WriteReport ("#   - Client:      should work only on: \"My TW Attribute\"");
	WriteReport ("#   - FullAccess:  should work on each");
	WriteReport ("# Check all attribute setting dialogs after execution");
	WriteReport ("# Execute it in each TeamWork mode");

	err = Do_DeleteAttributes (false);

	WriteReport_End (err);

	return;
}		// Do_DeleteMyAttributes_TW


// -----------------------------------------------------------------------------
// Show/Hide and Lock/Unlock the layer of lines : "2D Drafting - General"
// -----------------------------------------------------------------------------

static void		Do_InvertLayerOfLines (void)
{
	API_Attribute	attrib;
	GSErrCode			err;

	WriteReport ("# Show/Hide and Lock/Unlock the layer named \"2D Drafting - General\"");
	WriteReport ("#   - ViewOnly:    disabled");
	WriteReport ("#   - Client:");
	WriteReport ("#       not in workspace:  only Show/Hide status is allowed to be changed");
	WriteReport ("#       in workspace:      must work, all enabled");
	WriteReport ("#   - FullAccess:  must work, all enabled");
	WriteReport ("# Execute it in each TeamWork mode");
	WriteReport ("# Check the \"Layer Settings...\" dialog after execution");

	BNZeroMemory (&attrib, sizeof (API_Attribute));
	attrib.header.typeID = API_LayerID;
	strcpy (attrib.header.name, "2D Drafting - General");

	err = ACAPI_Attribute_Search (&attrib.header);
	if (err != NoError) {
		WriteReport_Err ("ACAPI_Attribute_Search", err);
		return;
	}

	WriteReport ("layer index of \"2D Drafting - General\": #%d", attrib.header.index);

	err = ACAPI_Attribute_Get (&attrib);
	if (err != NoError) {
		WriteReport_Err ("ACAPI_Attribute_Get", err);
		return;
	}

	attrib.header.flags ^= APILay_Hidden;
	attrib.header.flags ^= APILay_Locked;
	err = ACAPI_Attribute_Modify (&attrib, NULL);
	if (err != NoError)
		WriteReport_Err ("ACAPI_Attribute_Modify", err);

	WriteReport_End (err);

	return;
}		// Do_InvertLayerOfLines


// -----------------------------------------------------------------------------
// Create a Layer Combination named "My TW Layercomb"
// -----------------------------------------------------------------------------

static void		Do_CreateLayerComb (void)
{
	WriteReport ("# Create a LayerCombination named \"My TW Layercomb\"");
	WriteReport ("#   - ViewOnly:    disabled");
	WriteReport ("#   - Client:      notMine layers must be locked");
	WriteReport ("#   - FullAccess:  must work");
	WriteReport ("# ArchiCAD menus must be updated");
	WriteReport ("# Check the \"Layers menu\" and the \"Layer Settings...\" dialog after execution");

	const short startIndex = 10;
	const short endIndex = 40;

	API_AttributeDef defs;
	BNZeroMemory (&defs, sizeof (API_AttributeDef));
	defs.layer_statItems = (API_LayerStat **) BMAllocateHandle ((endIndex - startIndex + 1) * sizeof (API_LayerStat), ALLOCATE_CLEAR, 0);
	if (defs.layer_statItems == NULL) {
		WriteReport_Err ("BMAllocateHandle", ErrMemoryFull);
		return;
	}

	short count = 0;
	for (short i = startIndex; i <= endIndex; i++) {	// fill an incomplete list of layer status
		(*defs.layer_statItems) [count].lInd = i;		// unreferenced layers (except the ArchiCAD Layer) will be hidden in this layer combination
		(*defs.layer_statItems) [count].lFlags = APILay_Locked | APILay_ForceToWire;
		(*defs.layer_statItems) [count].conClassId = count;
		count++;
	}

	API_Attribute attrib;
	BNZeroMemory (&attrib, sizeof (API_Attribute));
	attrib.header.typeID = API_LayerCombID;

	strcpy (attrib.layerComb.head.name, "My TW Layercomb");
	attrib.layerComb.lNumb = endIndex - startIndex + 1;

	GSErrCode err = ACAPI_Attribute_Create (&attrib, &defs);

	ACAPI_DisposeAttrDefsHdls (&defs);

	if (err != NoError) {
		WriteReport_Err ("ACAPI_Attribute_Create", err);
		return;
	}

	WriteReport ("layerComb index of \"My TW Layercomb\": #%d", attrib.header.index);

	WriteReport_End (err);

	return;
}		// Do_CreateLayerComb


#ifdef __APPLE__
#pragma mark -
#endif

// =============================================================================
//
// Layers / Layer Combinations
//
// =============================================================================

// -----------------------------------------------------------------------------
//  List all layers
// -----------------------------------------------------------------------------

static void		Do_ListLayers (void)
{
	API_Attribute	attrib;
	short			count, i;
	GSErrCode		err;

	WriteReport ("# List of layers:");

	err = ACAPI_Attribute_GetNum (API_LayerID, &count);
	if (err != NoError) {
		WriteReport_Err ("ACAPI_Attribute_GetNum", err);
		return;
	}

	WriteReport ("Number of layers: #%d", count);

	for (i = 1; i <= count; i++) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.header.typeID = API_LayerID;
		attrib.header.index  = i;

		err = ACAPI_Attribute_Get (&attrib);

		if (err == APIERR_DELETED)
			WriteReport ("  [%2d]   DEL", i);
		else if (err != NoError)
			WriteReport ("  [%2d]   err:%d", i, err);
		else {
			WriteReport ("  [%2d]   %s  %s  %s", i,
							attrib.header.flags & APILay_Hidden ? "HID " : "    ",
							attrib.header.flags & APILay_Locked ? "LOC " : "    ",
							(attrib.header.index == 1) ? "ArchiCAD Layer" : attrib.header.name);
		}
	}

	WriteReport_End (err);

	return;
}		// Do_ListLayers


// -----------------------------------------------------------------------------
//  List all layer combinations
// -----------------------------------------------------------------------------

static void		Do_ListLayerCombinations (void)
{
	API_Attribute		attrib;
	API_AttributeDef	defs;
	short				count, count2, i, j;
	GSErrCode				err;

	WriteReport ("# List of layer combinations:");

	err = ACAPI_Attribute_GetNum (API_LayerCombID, &count);
	if (err != NoError) {
		WriteReport_Err ("ACAPI_Attribute_GetNum", err);
		return;
	}

	WriteReport ("# Num: %d", count);
	for (i = 1; i <= count; i++) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		BNZeroMemory (&defs, sizeof (API_AttributeDef));
		attrib.header.typeID = API_LayerCombID;
		attrib.header.index  = i;

		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError)
			err = ACAPI_Attribute_GetDef (API_LayerCombID, i, &defs);

		if (err == APIERR_DELETED) {
			WriteReport ("[%2d]   DEL", i);
		} else if (err != NoError) {
			WriteReport ("[%2d]   err:%d", i, err);
		} else {
			count2 = attrib.layerComb.lNumb;
			WriteReport ("-------------------------");
			WriteReport ("[%2d]   %s  nLay: %d", i, attrib.header.name, count2);
			for (j = 0; j < count2; j++) {
				BNZeroMemory (&attrib, sizeof (API_Attribute));
				attrib.header.typeID = API_LayerID;
				attrib.header.index  = (*defs.layer_statItems)[j].lInd;
				err = ACAPI_Attribute_Get (&attrib);
				if (err == APIERR_DELETED)
					WriteReport ("  [%2d]   ***", j + 1);
				else if (err != NoError)
					WriteReport ("  [%2d]   err:%d", j + 1, err);
				else
					WriteReport ("  [%2d]   %s  %s   %3d   %s", j + 1,
									(*defs.layer_statItems)[j].lFlags & APILay_Hidden ? "HID " : "    ",
									(*defs.layer_statItems)[j].lFlags & APILay_Locked ? "LOC " : "    ",
									(*defs.layer_statItems)[j].conClassId,
									(attrib.header.index == 1) ? "ArchiCAD Layer" : attrib.header.name);
			}
		}

		ACAPI_DisposeAttrDefsHdls (&defs);
	}

	WriteReport_End (err);

	return;
}		// Do_ListLayerCombinations


// -----------------------------------------------------------------------------
// Create a layer named "AAAA"
// -----------------------------------------------------------------------------

static void		Do_CreateLayer_AAAA (void)
{
	API_Attribute	attrib;
	GSErrCode			err;

	WriteReport ("# Create a layer named \"AAAA\"");
	WriteReport ("#   - not hidden, not locked");
	WriteReport ("#   - must be put into each layerComb as Hidden");
	WriteReport ("# ArchiCAD menus must be updated");
	WriteReport ("# Check the \"Layer Settings...\" dialog after execution");

	BNZeroMemory (&attrib, sizeof (API_Attribute));
	attrib.header.typeID = API_LayerID;

	strcpy (attrib.layer.head.name, "AAAA");
	err = ACAPI_Attribute_Create (&attrib, NULL);

	if (err != NoError) {
		WriteReport_Err ("ACAPI_Attribute_Create", err);
		return;
	}

	WriteReport ("layer index of \"AAAA\": #%d", attrib.header.index);

	WriteReport_End (err);

	return;
}		// Do_CreateLayer_AAAA


// -----------------------------------------------------------------------------
// Delete the layer named "AAAA"
// -----------------------------------------------------------------------------

static void		Do_DeleteLayer_AAAA (void)
{
	API_Attribute	attrib;
	GSErrCode			err;

	WriteReport ("# Delete the layer named \"AAAA\"");
	WriteReport ("#   - must be removed from all layer combinations");
	WriteReport ("#     (call the \"List layer combinations\" command)");
	WriteReport ("# ArchiCAD menus must be updated");
	WriteReport ("# Check the \"Layer Settings...\" dialog after execution");

	BNZeroMemory (&attrib, sizeof (API_Attribute));
	attrib.header.typeID = API_LayerID;
	strcpy (attrib.header.name, "AAAA");

	err = ACAPI_Attribute_Search (&attrib.header);
	if (err != NoError) {
		WriteReport_Err ("ACAPI_Attribute_Search", err);
		return;
	}

	WriteReport ("layer index of \"AAAA\": #%d", attrib.header.index);

	err = ACAPI_Attribute_Delete (&attrib.header);
	if (err != NoError)
		WriteReport_Err ("ACAPI_Attribute_Delete", err);

	WriteReport_End (err);

	return;
}		// Do_DeleteLayer_AAAA


// -----------------------------------------------------------------------------
// Lock/Unlock the layer named "AAAA"
// -----------------------------------------------------------------------------

static void		Do_InvLockLayer_AAAA (void)
{
	API_Attribute	attrib;
	GSErrCode			err;

	WriteReport ("# Lock/Unlock the layer named \"AAAA\"");
	WriteReport ("#   - no effect on layer combinations");
	WriteReport ("#     (call the \"List layer combinations\" command)");
	WriteReport ("# Check the \"Layer Settings...\" dialog after execution");

	BNZeroMemory (&attrib, sizeof (API_Attribute));
	attrib.header.typeID = API_LayerID;
	strcpy (attrib.header.name, "AAAA");

	err = ACAPI_Attribute_Search (&attrib.header);
	if (err != NoError) {
		WriteReport_Err ("ACAPI_Attribute_Search", err);
		return;
	}

	WriteReport ("layer index of \"AAAA\": #%d", attrib.header.index);

	err = ACAPI_Attribute_Get (&attrib);
	if (err != NoError) {
		WriteReport_Err ("ACAPI_Attribute_Get", err);
		return;
	}

	attrib.header.flags ^= APILay_Locked;
	err = ACAPI_Attribute_Modify (&attrib, NULL);
	if (err != NoError)
		WriteReport_Err ("ACAPI_Attribute_Modify", err);

	WriteReport_End (err);

	return;
}		// Do_InvLockLayer_AAAA


// -----------------------------------------------------------------------------
// Lock/Hide the layer named "ArchiCAD"
// -----------------------------------------------------------------------------

static void		Do_LockHideLayer_1 (void)
{
	API_Attribute	attrib;
	GSErrCode			err;

	WriteReport ("# Lock/Hide the layer named \"ArchiCAD\"");
	WriteReport ("#   - not allowed operation");
	WriteReport ("#     (call the \"List layers\" command)");
	WriteReport ("# Check the \"Layer Settings...\" dialog after execution");

	BNZeroMemory (&attrib, sizeof (API_Attribute));
	attrib.header.typeID = API_LayerID;
	attrib.header.index  = 1;

	err = ACAPI_Attribute_Get (&attrib);
	if (err != NoError) {
		WriteReport_Err ("ACAPI_Attribute_Get", err);
		return;
	}

	attrib.header.flags |= APILay_Locked | APILay_Hidden;
	err = ACAPI_Attribute_Modify (&attrib, NULL);		// should return an error
	if (err != NoError)
		WriteReport_Err ("ACAPI_Attribute_Modify", err);

	WriteReport_End (err);

	return;
}		// Do_LockHideLayer_1


// -----------------------------------------------------------------------------
// Create a layer combination named "BBBB"
// -----------------------------------------------------------------------------

static void		Do_CreateLayerComb_BBBB (void)
{
	API_Attribute		attrib;
	API_AttributeDef	defs;
	short				nLayer, count, i;
	GSErrCode			err;

	WriteReport ("# Create a layer combination named \"BBBB\"");
	WriteReport ("#   - active set should remain the same");
	WriteReport ("#   - every second layer must be locked");
	WriteReport ("#   - \"ArchiCAD\" layer cannot be LOCKED or HIDDEN");
	WriteReport ("# (call the \"List layer combinations\" command)");
	WriteReport ("# ArchiCAD menus must be updated");
	WriteReport ("# Check the \"Layers menu\" and the \"Layer Settings...\" dialog after execution");

	ACAPI_Attribute_GetNum (API_LayerID, &nLayer);

	BNZeroMemory (&defs, sizeof (API_AttributeDef));
	defs.layer_statItems = (API_LayerStat **) BMAllocateHandle (nLayer * sizeof (API_LayerStat), ALLOCATE_CLEAR, 0);
	if (defs.layer_statItems == NULL) {
		WriteReport_Err ("BMAllocateHandle", ErrMemoryFull);
		return;
	}
	count = 0;
	for (i = 1; i <= nLayer; i++) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.header.typeID = API_LayerID;
		attrib.header.index  = i;
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			(*defs.layer_statItems) [count].lInd = i;
			if (count % 2 == 0)
				(*defs.layer_statItems) [count].lFlags = APILay_Locked;
			(*defs.layer_statItems) [count].conClassId = 1;
			count ++;
		}
	}
	defs.layer_statItems = (API_LayerStat **) BMReallocHandle ((GSHandle) defs.layer_statItems, count * sizeof (API_LayerStat), REALLOC_CLEAR, 0);

	BNZeroMemory (&attrib, sizeof (API_Attribute));
	attrib.header.typeID = API_LayerCombID;

	strcpy (attrib.layerComb.head.name, "BBBB");
	attrib.layerComb.lNumb = count;

	err = ACAPI_Attribute_Create (&attrib, &defs);

	ACAPI_DisposeAttrDefsHdls (&defs);

	if (err != NoError) {
		WriteReport_Err ("ACAPI_Attribute_Create", err);
		return;
	}

	WriteReport ("layerComb index of \"BBBB\": #%d", attrib.header.index);

	WriteReport_End (err);

	return;
}		// Do_CreateLayerComb_BBBB


// -----------------------------------------------------------------------------
// Delete the layer combination named "BBBB"
// -----------------------------------------------------------------------------

static void		Do_DeleteLayerComb_BBBB (void)
{
	API_Attribute	attrib;
	GSErrCode			err;

	WriteReport ("# Delete the layer combination named \"BBBB\"");
	WriteReport ("#   - must disappear from ArchiCAD menus");
	WriteReport ("#   - active set should be removed if it is");
	WriteReport ("# ArchiCAD menus must be updated");
	WriteReport ("# Check the \"Layers...\" menu after execution");


	BNZeroMemory (&attrib, sizeof (API_Attribute));
	attrib.header.typeID = API_LayerCombID;
	strcpy (attrib.header.name, "BBBB");

	err = ACAPI_Attribute_Search (&attrib.header);
	if (err != NoError) {
		WriteReport_Err ("ACAPI_Attribute_Search", err);
		return;
	}

	WriteReport ("layerComb index of \"BBBB\": #%d", attrib.header.index);

	err = ACAPI_Attribute_Delete (&attrib.header);
	if (err != NoError)
		WriteReport_Err ("ACAPI_Attribute_Delete", err);

	WriteReport_End (err);

	return;
}		// Do_DeleteLayerComb_BBBB


// -----------------------------------------------------------------------------
// Show/Hide the layers in the layer combination named "BBBB"
// - active set should remain the same
// -----------------------------------------------------------------------------

static void		Do_InvShowLayerComb_BBBB (void)
{
	API_Attribute		attrib;
	API_AttributeDef	defs;
	short				j;
	GSErrCode				err;

	WriteReport ("# Show/Hide the layers in the layer combination named \"BBBB\"");
	WriteReport ("#   - active set should remain the same", false);
	WriteReport ("#   - \"ArchiCAD\" layer cannot be LOCKED or HIDDEN");
	WriteReport ("# ArchiCAD menus must be updated");
	WriteReport ("# Check the \"Layer Settings...\" dialog after execution");

	BNZeroMemory (&attrib, sizeof (API_Attribute));
	attrib.header.typeID = API_LayerCombID;
	strcpy (attrib.header.name, "BBBB");

	err = ACAPI_Attribute_Search (&attrib.header);
	if (err != NoError) {
		WriteReport_Err ("ACAPI_Attribute_Search", err);
		return;
	}

	err = ACAPI_Attribute_Get (&attrib);
	BNZeroMemory (&defs, sizeof (defs));
	if (err == NoError)
		err = ACAPI_Attribute_GetDef (API_LayerCombID, attrib.header.index, &defs);
	if (err != NoError) {
		WriteReport_Err ("ACAPI_Attribute_Get", err);
		return;
	}

	WriteReport ("layerComb index of \"BBBB\": #%d", attrib.header.index);

	for (j = 0; j < attrib.layerComb.lNumb; j++)
		(*defs.layer_statItems)[j].lFlags ^= APILay_Hidden;

	err = ACAPI_Attribute_Modify (&attrib, &defs);
	if (err != NoError)
		WriteReport_Err ("ACAPI_Attribute_Modify", err);

	ACAPI_DisposeAttrDefsHdls (&defs);

	WriteReport_End (err);

	return;
}		// Do_InvShowLayerComb_BBBB


// -----------------------------------------------------------------------------
// Create a Composite Wall named "Extended CompWall (API)"
// -----------------------------------------------------------------------------

static void		Do_CreateCompWall (void)
{
	API_Attribute		attrib;
	API_AttributeDefExt	defs;
	short				nComps = 4, i;
	double				totalThick = 0.6;
	GSErrCode			err;

	WriteReport ("# Create a Composite Wall named \"Extended CompWall (API)\"");

	BNZeroMemory (&attrib, sizeof (API_Attribute));
	BNZeroMemory (&defs, sizeof (API_AttributeDefExt));
	defs.cwall_compItems = (API_CWallComponent **) BMAllocateHandle (nComps * sizeof (API_CWallComponent), ALLOCATE_CLEAR, 0);
	if (defs.cwall_compItems == NULL) {
		WriteReport_Err ("BMAllocateHandle", ErrMemoryFull);
		return;
	}
	for (i = 0; i < nComps; i++) {
		(*defs.cwall_compItems)[i].buildingMaterial = (short) (i + 1);
		(*defs.cwall_compItems)[i].framePen = (short) (i + 1);
		(*defs.cwall_compItems)[i].flagBits = 1;
		(*defs.cwall_compItems)[i].fillThick = totalThick / nComps;
	}

	defs.cwall_compLItems = (API_CWallLineComponent **) BMAllocateHandle ((nComps + 1) * sizeof (API_CWallLineComponent), ALLOCATE_CLEAR, 0);
	if (defs.cwall_compLItems == NULL) {
		WriteReport_Err ("BMAllocateHandle", ErrMemoryFull);
		return;
	}

	for (i = 0; i < nComps + 1; i++) {
		(*defs.cwall_compLItems)[i].ltypeInd = (short) (i + 1);
		(*defs.cwall_compLItems)[i].linePen = (short) (i + 1);
	}

	attrib.header.typeID = API_CompWallID;

	strcpy (attrib.layerComb.head.name, "Extended CompWall (API)");
	attrib.compWall.totalThick = totalThick;
	attrib.compWall.nComps = nComps;

	err = ACAPI_Attribute_CreateExt (&attrib, &defs);

	ACAPI_DisposeAttrDefsHdlsExt (&defs);

	if (err != NoError) {
		WriteReport_Err ("ACAPI_Attribute_Create", err);
		return;
	}

	WriteReport ("CompWall index of \"Extended CompWall (API)\": #%d", attrib.header.index);

	WriteReport_End (err);

	return;
}		// Do_CreateCompWall


// -----------------------------------------------------------------------------
// Modify the CompWall: Extended CompWall
// -----------------------------------------------------------------------------

static void		Do_ChangeCompWall (void)
{
	API_Attribute		attrib;
	API_AttributeDefExt	defs;
	GSErrCode			err;
	double				extra = 0.2;

	WriteReport ("# Change the CompWall: \"Extended CompWall (API)\"");
	WriteReport ("# Check the \"Composite...\" dialog after execution");

	BNZeroMemory (&attrib, sizeof (API_Attribute));
	BNZeroMemory (&defs, sizeof (API_AttributeDefExt));

	attrib.header.typeID = API_CompWallID;
	strcpy (attrib.header.name, "Extended CompWall (API)");

	err = ACAPI_Attribute_Search (&attrib.header);		// search by name: index returned in header
	if (err != NoError) {
		WriteReport_Err ("CompWall \"Extended CompWall (API)\" was not found", err);
		return;
	}
	err = ACAPI_Attribute_Get (&attrib);
	if (err == NoError)
		err = ACAPI_Attribute_GetDefExt (attrib.header.typeID, attrib.header.index, &defs);
	if (err != NoError) {
		WriteReport_Err ("Unable to get the compWall definition", err);
		return;
	}

	attrib.compWall.totalThick += extra;
	attrib.compWall.nComps++;

	defs.cwall_compItems = (API_CWallComponent **) BMReallocHandle ((GSHandle) defs.cwall_compItems,
																	(attrib.compWall.nComps) * sizeof (API_CWallComponent), REALLOC_CLEAR, 0);
	if (defs.cwall_compItems == NULL)
		return;

	defs.cwall_compLItems = (API_CWallLineComponent **) BMReallocHandle ((GSHandle) defs.cwall_compLItems,
																		 (attrib.compWall.nComps + 1) * sizeof (API_CWallLineComponent), REALLOC_CLEAR, 0);
	if (defs.cwall_compLItems == NULL)
		return;

	(*defs.cwall_compItems)[attrib.compWall.nComps - 1].buildingMaterial = 1;
	(*defs.cwall_compItems)[attrib.compWall.nComps - 1].framePen = 4;
	(*defs.cwall_compItems)[attrib.compWall.nComps - 1].flagBits = (*defs.cwall_compItems)[0].flagBits;
	(*defs.cwall_compItems)[attrib.compWall.nComps - 1].fillThick = extra;

	(*defs.cwall_compLItems)[attrib.compWall.nComps].ltypeInd = 6;
	(*defs.cwall_compLItems)[attrib.compWall.nComps].linePen = 6;

	err = ACAPI_Attribute_ModifyExt (&attrib, &defs);

	ACAPI_DisposeAttrDefsHdlsExt (&defs);
	if (err != NoError) {
		WriteReport_Err ("Unable to modify the compWall \"Extended CompWall\"", err);
		return;
	}

	WriteReport_End (err);

	return;
}		// Do_ChangeCompWall


#ifdef __APPLE__
#pragma mark -
#endif

// -----------------------------------------------------------------------------
// MenuCommandHandler
//		called to perform the user-asked command
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL MenuCommandHandler (const API_MenuParams *menuParams)
{
	switch (menuParams->menuItemRef.menuResID) {
		case 32500:		/* Attributes */
				switch (menuParams->menuItemRef.itemIndex) {
					case 1:		Do_CountAttributes ();				break;
					case 2:		Do_ListAttributes ();				break;
					case 3:		Do_RenameAttributes (true);			break;
					case 4:		Do_CopyAttributes ();				break;
					case 5:		Do_DeleteAttributes (true);			break;
					/* ---- */
					case 7:		Do_CreateLayer_Training ();			break;
					case 8:		Do_CreateLinetypes_Training ();		break;
					case 9:		Do_CreateFills_Training ();			break;
					case 10:	Do_CreateCompWall ();				break;
					case 11:	Do_CreateProfile ();				break;
					case 12:	Do_CreatePenTable ();				break;
					case 13:	Do_CreateMEPSystem ();				break;
					case 14:	Do_CreateBuildingMaterial ();		break;
				}
				break;

		case 32501:		/* Attributes: Modify */
				switch (menuParams->menuItemRef.itemIndex) {
					case 1:		Do_HideLayerOfLines ();				break;
					case 2:		Do_ChangeLinetype_Wave ();			break;
					case 3:		Do_ChangeCompWall ();				break;
				}
				break;

		case 32502:		/* Attributes: Teamwork mode */
				switch (menuParams->menuItemRef.itemIndex) {
					case 1:		Do_CreateAttributes_TW ();			break;
					case 2:		Do_RenameMyAttributes_TW ();		break;
					case 3:		Do_DeleteMyAttributes_TW ();		break;
					/* ---- */
					case 5:		Do_InvertLayerOfLines ();			break;
					case 6:		Do_CreateLayerComb ();				break;
				}
				break;

		case 32503:		/* Attributes: Snippets */
				switch (menuParams->menuItemRef.itemIndex) {
					case 1:		Do_ListLayers ();					break;
					case 2:		Do_ListLayerCombinations ();		break;
					/* ---- */
					case 4:		Do_CreateLayer_AAAA ();				break;
					case 5:		Do_DeleteLayer_AAAA ();				break;
					case 6:		Do_InvLockLayer_AAAA ();			break;
					case 7:		Do_LockHideLayer_1 ();				break;
					/* ---- */
					case 9:		Do_CreateLayerComb_BBBB ();			break;
					case 10:	Do_DeleteLayerComb_BBBB ();			break;
					case 11:	Do_InvShowLayerComb_BBBB ();		break;
					/* ---- */
					case 13:	Do_ListAllProfileDetails ();		break;
					case 14:	Do_ListAllOpProfileDetails ();		break;
					case 15:	Do_ListAllBuildingMaterials ();		break;
				}
				break;
	}


	return NoError;
}		// MenuCommandHandler


// =============================================================================
//
// Required functions
//
// =============================================================================

// -----------------------------------------------------------------------------
// Dependency definitions
// -----------------------------------------------------------------------------

API_AddonType	__ACDLL_CALL	CheckEnvironment (API_EnvirParams* envir)
{
	if (envir->serverInfo.serverApplication != APIAppl_ArchiCADID)
		return APIAddon_DontRegister;

	ACAPI_Resource_GetLocStr (envir->addOnInfo.name, 32000, 1);
	ACAPI_Resource_GetLocStr (envir->addOnInfo.description, 32000, 2);

	return APIAddon_Normal;
}		// CheckEnvironment


// -----------------------------------------------------------------------------
// Interface definitions
// -----------------------------------------------------------------------------

GSErrCode	__ACDLL_CALL	RegisterInterface (void)
{
	GSErrCode err = NoError;
	err = ACAPI_Register_Menu (32500, 0, MenuCode_UserDef, MenuFlag_SeparatorBefore);
	if (err == NoError)
		err = ACAPI_Register_Menu (32501, 0, MenuCode_UserDef, MenuFlag_Default);
	if (err == NoError)
		err = ACAPI_Register_Menu (32502, 0, MenuCode_UserDef, MenuFlag_Default);
	if (err == NoError)
		err = ACAPI_Register_Menu (32503, 0, MenuCode_UserDef, MenuFlag_SeparatorAfter);
	return err;
}		// RegisterInterface


// -----------------------------------------------------------------------------
// Initialize
//		called after the Add-On has been loaded into memory
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	Initialize (void)
{
	GSErrCode err = ACAPI_Install_MenuHandler (32500, MenuCommandHandler);
	if (err != NoError)
		DBPrintf ("Attribute Test:: Initialize() ACAPI_Install_MenuHandler failed\n");

	err = ACAPI_Install_MenuHandler (32501, MenuCommandHandler);
	if (err != NoError)
		DBPrintf ("Attribute Test:: Initialize() ACAPI_Install_MenuHandler failed\n");

	err = ACAPI_Install_MenuHandler (32502, MenuCommandHandler);
	if (err != NoError)
		DBPrintf ("Attribute Test:: Initialize() ACAPI_Install_MenuHandler failed\n");

	err = ACAPI_Install_MenuHandler (32503, MenuCommandHandler);
	if (err != NoError)
		DBPrintf ("Attribute Test:: Initialize() ACAPI_Install_MenuHandler failed\n");


	return NoError;
}		// Initialize


// -----------------------------------------------------------------------------
// FreeData
//		called when the Add-On is going to be unloaded
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	FreeData (void)
{
	return NoError;
}		// FreeData

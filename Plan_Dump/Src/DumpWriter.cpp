// *********************************************************************************************************************
// Description:		Implementation of the DumpWriter class
//
// Module:			API Development Kit 19; Mac/Win
// Namespace:		-none-
// Contact person:	devsup@graphisoft.hu
//
// SG compatible
// *********************************************************************************************************************

// --- Includes	----------------------------------------------------------------

#include	"DumpWriter.hpp"
#include	"DumpUtils.hpp"

#include	<math.h>

#include	"Definitions.hpp"

#include	"GenArc2DData.h"
#include	"Coord3d.h"

#include	"Model3D/model.h"
#include	"Model3D/Body.hpp"

// --- Constant definitions ----------------------------------------------------

#define		EOLInFileWIN		"\xD\xA"
#define		EOLInFileWIN_LEN	2
#define		EOLInFileMAC		"\xA"
#define		EOLInFileMAC_LEN	1

#ifdef WINDOWS
#define		EOLInFile			EOLInFileWIN
#define		EOLInFile_LEN		EOLInFileWIN_LEN
#endif
#ifdef macintosh
#define		EOLInFile			EOLInFileMAC
#define		EOLInFile_LEN		EOLInFileMAC_LEN
#endif


// =====================================================================================================================
//
// Constuction / Destruction
//
// =====================================================================================================================

DumpWriter::DumpWriter () :
	outputFile	(NULL),
	fileLoc		(NULL),

	lastErr		(NoError),
	isNewLine	(true),
	tabcount	(0)
{
	BNZeroMemory (&unit, sizeof (DGUnitData));
	DGGetUnit (&unit);
}


DumpWriter::~DumpWriter ()
{
	if (outputFile != NULL) {
		delete outputFile;
		outputFile = NULL;
	}

	if (fileLoc != NULL) {
		delete fileLoc;
		fileLoc = NULL;
	}
}


// =====================================================================================================================
//
// File I/O
//
// =====================================================================================================================


// -----------------------------------------------------------------------------
//	Write into the open file
// -----------------------------------------------------------------------------

GSErrCode DumpWriter::Write (Int32 nBytes, GSPtr data)
{
	if (isNewLine) {
		for (short i = 0; i < tabcount && lastErr == NoError; i++)
			lastErr = outputFile->WriteBin ("\t", 1);
		isNewLine = false;
	}

	if (lastErr != NoError)
		return lastErr;

	lastErr = outputFile->WriteBin (data, nBytes);

	return lastErr;
}		// Write


// -----------------------------------------------------------------------------
//	Write a unicode string into the open file
// -----------------------------------------------------------------------------

GSErrCode DumpWriter::Write (const GS::UniString& val)
{
	if (isNewLine) {
		for (short i = 0; i < tabcount && lastErr == NoError; i++)
			lastErr = outputFile->WriteBin ("\t", 1);
		isNewLine = false;
	}

	if (lastErr != NoError)
		return lastErr;

	Int32 length  = Strlen32 (val.ToCStr (0, MaxUSize, CC_UTF8).Get ());
	char *cBuffer = NULL;
	try {
		cBuffer = new char[length + 1];
		if (cBuffer != NULL) {
			CHCopyC (val.ToCStr (0, MaxUSize, CC_UTF8).Get (), cBuffer);
			lastErr = outputFile->WriteBin (cBuffer, length);
		} else {
			lastErr = ErrMemoryFull;
		}
	} catch (...) {
		lastErr = Error;
	}
	if (cBuffer != NULL)
		delete [] cBuffer;

	return lastErr;
}		// Write


// -----------------------------------------------------------------------------
//	Open the file
// -----------------------------------------------------------------------------

GSErrCode DumpWriter::Open (const IO::Location& inLoc)
{
	GSErrCode	errCode = NoError;

	if (outputFile != NULL)
		return Error;

	fileLoc = new IO::Location (inLoc);
	if (fileLoc == NULL)
		return Error;

	errCode = fileLoc->GetStatus ();
	if (errCode != NoError) {
		delete fileLoc;
		fileLoc = NULL;
		return errCode;
	}

	try {
		outputFile = new IO::File (*fileLoc, IO::File::Create);
		if (outputFile == NULL)
			throw GS::GeneralException ();
	}
	catch (...) {
		if (outputFile != NULL) {
			delete outputFile;
			outputFile = NULL;
		}

		throw GS::GeneralException ();
	}

	errCode = outputFile->GetStatus ();
	if (errCode == NoError)
		errCode = outputFile->Open (IO::File::WriteEmptyMode);

	if (errCode != NoError) {
		outputFile->Close ();

		delete outputFile;
		outputFile = NULL;
		IO::fileSystem.Delete (*fileLoc);

		throw GS::GeneralException ();
	}

	if (outputFile != NULL)
		outputFile->WriteBin ("\xEF\xBB\xBF", 3);	// make it UTF-8

	return NoError;
}


// -----------------------------------------------------------------------------
//	Close the file
// -----------------------------------------------------------------------------

GSErrCode DumpWriter::Close (void)
{
	if (outputFile != NULL)
		lastErr = outputFile->Close ();

	return lastErr;
}		// Close


// -----------------------------------------------------------------------------
//	Write a newline into the file
// -----------------------------------------------------------------------------

GSErrCode DumpWriter::WrNewLine (void)
{
	outputFile->WriteBin (EOLInFile, EOLInFile_LEN);
	isNewLine = true;

	return lastErr;
}		// WrNewLine


// -----------------------------------------------------------------------------
//	Write an RGB value into the file
// -----------------------------------------------------------------------------

GSErrCode DumpWriter::WriteRGB (const API_RGBColor* rgb)
{
	char	buffer[256];

	sprintf (buffer, "%.6f %.6f %.6f", rgb->f_red, rgb->f_green, rgb->f_blue);
	Write (Strlen32 (buffer), buffer);

	return lastErr;
}		// WriteRGB


// -----------------------------------------------------------------------------
//	Write a string value into the file
// -----------------------------------------------------------------------------

GSErrCode	DumpWriter::WriteStr (const char* val, NewLineFlag newLine /* = NoNewLine*/)
{
	char	buffer[512];

	sprintf (buffer, "%s ", val);
	Write (Strlen32 (buffer), buffer);

	if (newLine == WithNewLine)
		WrNewLine ();

	return lastErr;
}		// WriteStr


// -----------------------------------------------------------------------------
//	Write a unicode string value into the file
// -----------------------------------------------------------------------------

GSErrCode	DumpWriter::WriteStr (const GS::UniString& val, NewLineFlag newLine /* = NoNewLine*/)
{
	Write (val);

	if (newLine == WithNewLine)
		WrNewLine ();

	return lastErr;
}		// WriteStr


// -----------------------------------------------------------------------------
//	Write a name string into the file
// -----------------------------------------------------------------------------

GSErrCode	DumpWriter::WriteName (const char* val, NewLineFlag newLine /* = NoNewLine*/)
{
	char	buffer[512];

	sprintf (buffer, "\"%s\" ", val);
	Write (Strlen32 (buffer), buffer);

	if (newLine == WithNewLine)
		WrNewLine ();

	return lastErr;
}		// WriteName


// -----------------------------------------------------------------------------
//	Write a name unicode string into the file
// -----------------------------------------------------------------------------

GSErrCode	DumpWriter::WriteName (const GS::UniString& val, NewLineFlag newLine /* = NoNewLine*/)
{
	Write (val);

	if (newLine == WithNewLine)
		WrNewLine ();

	return lastErr;
}		// WriteName


// -----------------------------------------------------------------------------
//	Write an integer value into the file
// -----------------------------------------------------------------------------

GSErrCode	DumpWriter::WriteInt (const Int32 val, NewLineFlag newLine /* = NoNewLine*/)
{
	char	buffer[64];

	sprintf (buffer, "%ld ", (GS::LongForStdio) val);
	Write (Strlen32 (buffer), buffer);

	if (newLine == WithNewLine)
		WrNewLine ();

	return lastErr;
}		// WriteInt


// -----------------------------------------------------------------------------
//	Write a float value into the file
// -----------------------------------------------------------------------------

GSErrCode	DumpWriter::WriteFloat (const double value)
{
	if (lastErr == NoError) {
		GS::UniString	buffer = DGDoubleToStringUnit (&unit, value, DG_ET_LENGTH);
		buffer.Append (" ");

		lastErr = Write (buffer);
	}

	return lastErr;
}		// WriteFloat


// -----------------------------------------------------------------------------
//	Write an angle value into the file
// -----------------------------------------------------------------------------

GSErrCode	DumpWriter::WriteAngle (const double value)
{
	if (lastErr == NoError) {
		GS::UniString	buffer = DGDoubleToStringUnit (&unit, value * RADDEG, DG_ET_POLARANGLE);
		buffer.Append (" ");

		lastErr = Write (buffer);
	}

	return lastErr;
}		// WriteAngle


// -----------------------------------------------------------------------------
//	Write an indented string into the file
// -----------------------------------------------------------------------------

GSErrCode	DumpWriter::WriteBlock (const char *name)
{
	char	buffer[512];

	WrNewLine ();
	sprintf (buffer, "%s {", name);
	Write (Strlen32 (buffer), buffer);
	tabcount++;
	WrNewLine ();

	return lastErr;
}		// WriteBlock


GSErrCode	DumpWriter::WriteBlock (const char *name, const API_Guid& guid)
{
	char	buffer[512];

	WrNewLine ();
	sprintf (buffer, "%s %s {", name, APIGuidToString (guid).ToCStr ().Get ());
	Write (Strlen32 (buffer), buffer);
	tabcount++;
	WrNewLine ();

	return lastErr;
}		// WriteBlock


GSErrCode	DumpWriter::WriteBlock (const char *name, short index)
{
	char	buffer[512];

	WrNewLine ();
	sprintf (buffer, "%s %d {", name, index);
	Write (Strlen32 (buffer), buffer);
	tabcount++;
	WrNewLine ();

	return lastErr;
}		// WriteBlock


// -----------------------------------------------------------------------------
//	Close the indentation block
// -----------------------------------------------------------------------------

GSErrCode	DumpWriter::WrEndblk (void)
{
	tabcount--;
	if (!isNewLine)
		WrNewLine ();
	WriteStr ("}", WithNewLine);

	return lastErr;
}		// WrEndblk


// -----------------------------------------------------------------------------
//	Write an indented description string into the file
// -----------------------------------------------------------------------------

GSErrCode	DumpWriter::WriteDescText (const char* text)
{
	char	line[256];

	tabcount++;
	WrNewLine ();
	BNZeroMemory (line, sizeof(line));
	Int32	len = Strlen32 (text);
	short	index = 0;

	for (Int32 i = 0; i <= len && lastErr == NoError; i++) {
		if (text[i] >= ' ' && index < sizeof(line)-1) {
			line[index] = text[i];
			index++;
			line[index] = '\0';
		} else {
			if (strlen (line) > 0) {
				WriteStr (line, WithNewLine);
				strcpy (line, "");
				index = 0;
			}
		}
	}
	tabcount--;

	return lastErr;
}		// WriteDescText


// -----------------------------------------------------------------------------
//	Write an element header into the file
// -----------------------------------------------------------------------------

GSErrCode	DumpWriter::WriteElemHead (const API_Element*	elem,
									   const char*			info,
									   const API_Element*	wallelem)
{
	if (lastErr != NoError)
		return lastErr;

	const API_Elem_Head*	elemHead = &elem->header;
	short					pen;

	switch (elemHead->typeID) {
		case API_WallID:		pen = elem->wall.contPen;
								break;

		case API_ColumnID:		pen = elem->column.corePen;
								break;

		case API_WindowID:
		case API_DoorID:		pen = elem->door.openingBase.pen;
								break;

		case API_ObjectID:
		case API_LampID:		pen = elem->lamp.pen;
								break;

		case API_SlabID:		pen = elem->slab.pen;
								break;
		case API_RoofID:		pen = elem->roof.shellBase.pen;
								break;
		case API_ShellID:		pen = elem->shell.shellBase.pen;
								break;
		case API_MorphID:	pen = elem->morph.coverFillPen;
								break;
		case API_ZoneID:		pen = elem->zone.pen;
								break;

		default:				pen = elem->wall.contPen;
	}

	API_StoryInfo		storyInfo;
	char				name[API_NameLen];
	GS::UniString		layerName;
	GSErrCode			err;

	if ((elemHead->typeID == API_WindowID || elemHead->typeID == API_DoorID) && wallelem != NULL)
		DumpUtils::GetAttrName (NULL, API_LayerID, wallelem->header.layer, &layerName);				/* only door or window */
	else
		DumpUtils::GetAttrName	(NULL, API_LayerID, elemHead->layer, &layerName);

	err = ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo, NULL);

	if (err == NoError) {
		if (strlen ((*storyInfo.data)[elemHead->floorInd - storyInfo.firstStory].name) == 0)
			sprintf (name,"%d. Story", elemHead->floorInd);
		else
			strcpy (name, (*(storyInfo.data))[elemHead->floorInd - storyInfo.firstStory].name );

		GS::UniString reportString = GS::UniString::Printf ("\"%s\" %d %d \"%T\" %d \"%s\" {%T}",
		                                                    info, pen, elemHead->layer, layerName.ToPrintf (),
		                                                    elemHead->floorInd, name,
		                                                    APIGuid2GSGuid (elemHead->groupGuid).ToUniString ().ToPrintf ());
 		WriteStr (reportString, WithNewLine);
	}

	BMKillHandle ((GSHandle *) &storyInfo.data);

	return err;
}		// WriteElemHead


// -----------------------------------------------------------------------------
//	Write an additional library part parameter into the file
// -----------------------------------------------------------------------------

void DumpWriter::WriteParameter (API_AddParID		typeID,
								 const char*		varname,
								 double 			value,
								 const char*		valueStr,
								 Int32 				dim1,
								 Int32 				dim2)
{
	if (dim1 < 0)
		return;

	char	head[256];

	if (dim1 == 0 && dim2 == 0)
		sprintf (head, "\"%s\"",  varname);
	else
		sprintf (head, "[%ld][%ld]", (GS::LongForStdio) dim1, (GS::LongForStdio) dim2);

	switch (typeID) {
		case APIParT_FillPat:	strcat ( head, " \"");
								DumpUtils::GetAttrName ( &head[ strlen (head)], API_FilltypeID, (short)value);
								strcat (head, "\"");
								WriteStr (head, WithNewLine);
								break;

		case APIParT_Mater:		strcat (head, " \"");
								DumpUtils::GetAttrName (&head[ strlen (head)], API_MaterialID, (short)value);
								strcat (head, "\"");
								WriteStr (head, WithNewLine);
								break;

		case APIParT_LineTyp:	strcat (head, " \"");
								DumpUtils::GetAttrName (&head[ strlen (head)], API_LinetypeID, (short)value);
								strcat (head, "\"");
								WriteStr (head, WithNewLine);
								break;

		case APIParT_Boolean:
		case APIParT_LightSw:	WriteStr (head);
								WriteStr (((Int32) value) != 0 ? "on" : "off");
								WrNewLine ();
								break;

		case APIParT_Integer:
		case APIParT_PenCol:	WriteStr (head);
								WriteInt ((Int32) value);
								WrNewLine ();
								break;

		case APIParT_Angle:		WriteStr (head);
								WriteAngle (value);
								WrNewLine ();
								break;
		case APIParT_ColRGB:
		case APIParT_Intens:
		case APIParT_RealNum:
		case APIParT_Length:	WriteStr (head);
								WriteFloat (value);
								WrNewLine ();
								break;

		case APIParT_CString:	strcat (head, " \"");
								strcat (head, valueStr);
								strcat (head, "\"");
								WriteStr (head, WithNewLine);
								break;

		case APIParT_Separator:	strcat (head, " \"");
								WriteStr (head, WithNewLine);
								break;

		case APIParT_Title:		strcat (head, " (title)");
								WriteStr (head, WithNewLine);
								break;

		default:				strcat (head, " ???");
								WriteStr (head, WithNewLine);

	}

	return;
}		// WriteParameter


// -----------------------------------------------------------------------------
//	Write the additional parameters of an element into the file
// -----------------------------------------------------------------------------

void DumpWriter::WriteParams (API_AddParType **params)
{
	double	value;
	Int32 	i1, i2, ind;

	if (params == NULL || *params == NULL)
		return;

	UInt32 addParNum = BMGetHandleSize (reinterpret_cast<GSHandle> (params)) / sizeof (API_AddParType);

	for (UInt32 i = 0; i < addParNum; i++) {
		if ((*params)[i].typeMod == API_ParSimple) {
				GS::UniString tmpUStr ((*params)[i].value.uStr);
				WriteParameter ((*params)[i].typeID, (*params)[i].name,
									(*params)[i].value.real, tmpUStr.ToCStr(),0,0);
		} else {
			ind = 0;
			WriteParameter ((*params)[i].typeID, (*params)[i].name,
								0.0, NULL,
								-(*params)[i].dim1, -(*params)[i].dim2);

			for (i1 = 1; i1 <= (*params)[i].dim1; i1++) {

				for (i2 = 1; i2 <= (*params)[i].dim2; i2++) {
					char*	valueStr;
					GS::uchar_t *uValueStr;
					if ((*params)[i].typeID != APIParT_CString) {
						value = (Int32) ((double *)
									*(*params)[i].value.array) [ind];
						valueStr = NULL;
						ind ++;
						WriteParameter ((*params)[i].typeID, NULL,
														value, valueStr, i1, i2);
					} else {
						value = 0.0;
						uValueStr = (GS::uchar_t *) *(*params)[i].value.array + ind;
						GS::UniString tmpUStr (uValueStr);
						ind += GS::ucslen32 (uValueStr) + 1;
						WriteParameter ((*params)[i].typeID, NULL,
														value, tmpUStr.ToCStr(), i1, i2);
					}
				}
			}
		}
	}
}		// WriteParams


// -----------------------------------------------------------------------------
//	Write the additional parameters of an element into the file
// -----------------------------------------------------------------------------

void DumpWriter::WriteParams (const API_Element* elem)
{
	GSErrCode			err;
	API_ElementMemo		memo;
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	err = ACAPI_Element_GetMemo (elem->header.guid, &memo);

	if (err == NoError)  {
		Int32				libInd = DumpUtils::Element_To_LibPartIndex (elem);
		API_AddParType**	addPars;
		double				a, b;
		Int32 				addParNum;

		err = ACAPI_LibPart_GetParams (libInd, &a, &b, &addParNum, &addPars);

		if (err == NoError) {
			WriteInt (addParNum, WithNewLine);
			WriteParams (memo.params);
		}
		ACAPI_DisposeAddParHdl (&addPars);
	}
	ACAPI_DisposeElemMemoHdls (&memo);

	return;
}		// WriteParams


// -----------------------------------------------------------------------------
//	Write the surface and volume of an element into the file
// -----------------------------------------------------------------------------

GSErrCode DumpWriter::WriteSurfVol (API_ElemTypeID typeID, const API_Guid& guid, bool isPlaneRoof)
{
	API_ElementQuantity	quantity;
	API_QuantitiesMask	mask;
	API_Quantities		quantities;

	API_QuantityPar		par;
	char				buffer[256];
	GSErrCode			err = NoError;

	quantities.elements = &quantity;

	switch (typeID) {
		case API_WallID:
			BNZeroMemory (&par, sizeof (API_QuantityPar));
			BNZeroMemory (&quantity, sizeof (API_ElementQuantity));
			par.minOpeningSize = 1.0;

			ACAPI_ELEMENT_QUANTITY_MASK_CLEAR (mask);
			ACAPI_ELEMENT_QUANTITY_MASK_SET (mask, wall, surface1);
			ACAPI_ELEMENT_QUANTITY_MASK_SET (mask, wall, surface2);
			ACAPI_ELEMENT_QUANTITY_MASK_SET (mask, wall, volume);

			err = ACAPI_Element_GetQuantities (guid, &par, &quantities, &mask);
			if (err == NoError) {
				sprintf (buffer, "%.6lf %.6lf %.6lf", quantity.wall.surface1, quantity.wall.surface2, quantity.wall.volume);
			}
			break;
		case API_ColumnID:
			BNZeroMemory (&quantity, sizeof (API_ElementQuantity));
			ACAPI_ELEMENT_QUANTITY_MASK_SETFULL (mask);
			err = ACAPI_Element_GetQuantities (guid, NULL, &quantities, &mask);
			if (err == NoError) {
				sprintf (buffer, "%.6lf %.6lf", quantity.column.veneSurface, quantity.column.veneVolume);
				WriteStr (buffer, WithNewLine);
				sprintf (buffer, "%.6lf %.6lf", quantity.column.coreSurface, quantity.column.coreVolume);
			}
			break;
		case API_DoorID:
		case API_WindowID:
		case API_SkylightID:
			BNZeroMemory (&quantity, sizeof (API_ElementQuantity));
			ACAPI_ELEMENT_QUANTITY_MASK_SETFULL (mask);
			err = ACAPI_Element_GetQuantities (guid, NULL, &quantities, &mask);
			if (err == NoError) {
				sprintf (buffer, "%.6lf %.6lf", quantity.door.surface, quantity.door.volume);
			}
			break;
		case API_ObjectID:
		case API_LampID:
			BNZeroMemory (&quantity, sizeof (API_ElementQuantity));
			ACAPI_ELEMENT_QUANTITY_MASK_SETFULL (mask);
			err = ACAPI_Element_GetQuantities (guid, NULL, &quantities, &mask);
			if (err == NoError) {
				sprintf (buffer, "%.6lf    %.6lf", quantity.symb.surface, quantity.symb.volume);
			}
			break;
		case API_ZoneID:
			BNZeroMemory (&quantity, sizeof (API_ElementQuantity));
			ACAPI_ELEMENT_QUANTITY_MASK_SETFULL (mask);
			err = ACAPI_Element_GetQuantities (guid, NULL, &quantities, &mask);
			if (err == NoError) {
				sprintf (buffer, "%.6lf %.6lf", quantity.zone.area, 0.0);
			}
			break;
		case API_RoofID:
			{
				GS::Array <API_ElemPartQuantity>	elemPartQ;
				BNZeroMemory (&quantity, sizeof (API_ElementQuantity));
				quantities.elemPartQuantities = &elemPartQ;
				ACAPI_ELEMENT_QUANTITY_MASK_SETFULL (mask);
				err = ACAPI_Element_GetQuantities (guid, NULL, &quantities, &mask);
				if (err == NoError) {
					if (isPlaneRoof)
						sprintf (buffer, "%.6lf %.6lf", quantity.roof.topSurface, quantity.roof.volume);
					else {
						for (UInt32 ii = 0; ii < elemPartQ.GetSize (); ++ii) {
							sprintf (buffer, "partID = (%u, %u) : %.6lf %.6lf",
							         elemPartQ[ii].partId.main, elemPartQ[ii].partId.sub,
							         elemPartQ[ii].quantity.roof.bottomSurface_cond, elemPartQ[ii].quantity.roof.topSurface_cond);
							if (ii < elemPartQ.GetSize () - 1)
								WriteStr (buffer, WithNewLine);
						}
					}
				}
				break;
			}
		case API_ShellID:
			BNZeroMemory (&quantity, sizeof (API_ElementQuantity));
			ACAPI_ELEMENT_QUANTITY_MASK_SETFULL (mask);
			err = ACAPI_Element_GetQuantities (guid, NULL, &quantities, &mask);
			if (err == NoError) {
				sprintf (buffer, "%.6lf %.6lf", quantity.shell.referenceSurface, quantity.shell.volume);
			}
			break;
		case API_MorphID:
			BNZeroMemory (&quantity, sizeof (API_ElementQuantity));
			ACAPI_ELEMENT_QUANTITY_MASK_SETFULL (mask);
			err = ACAPI_Element_GetQuantities (guid, NULL, &quantities, &mask);
			if (err == NoError) {
				sprintf (buffer, "%.6lf %.6lf", quantity.morph.surface, quantity.morph.volume);
			}
			break;
		case API_SlabID:
			BNZeroMemory (&quantity, sizeof (API_ElementQuantity));
			ACAPI_ELEMENT_QUANTITY_MASK_SETFULL (mask);
			err = ACAPI_Element_GetQuantities (guid, NULL, &quantities, &mask);
			if (err == NoError) {
				sprintf (buffer, "%.6lf %.6lf", quantity.slab.topSurface, quantity.slab.volume);
			}
			break;
		default:
			strcpy (buffer, "?SurfVol?");
	}

	WriteStr (buffer, WithNewLine);

	return err;
}		// WriteSurfVol


// -----------------------------------------------------------------------------
//	Write the properties of an element into the file
// -----------------------------------------------------------------------------

GSErrCode DumpWriter::WrProperties (const API_Guid& guid)
{
	API_Elem_Head	elemHead;
	BNZeroMemory (&elemHead, sizeof (API_Elem_Head));
	elemHead.guid = guid;

	API_ListData	listdata;
	BNZeroMemory (&listdata, sizeof (API_ListData));

	WriteBlock ("PROPERTIES", guid);

	API_DescriptorRefType**	deschandle = NULL;
	Int32	num;
	GSErrCode err = ACAPI_Element_GetDescriptors (&elemHead, &deschandle, &num);
	if (err == NoError && num > 0) {
		WriteBlock ("DESCRIPTOR", guid);
		WriteInt (num, WithNewLine);
		listdata.header.typeID = API_DescriptorID;
		for (Int32 i = 0; i < num; i++) {
			listdata.header.index = (*deschandle)[i].index;
			listdata.descriptor.name = NULL;
			switch ((*deschandle)[i].status) {
				case APIDBRef_Normal:
							listdata.header.setIndex = (*deschandle)[i].setIndex;
							err = ACAPI_ListData_Get (&listdata);
							break;
				case APIDBRef_Local:
							err = ACAPI_ListData_GetLocal ((*deschandle)[i].libIndex, &elemHead, &listdata);
							break;
				default:	err = APIERR_DELETED;
							break;
			}
			if (err == NoError) {
				WriteName (listdata.descriptor.code);
				WriteName (listdata.descriptor.keycode);
				WriteDescText ((char *) *(listdata.descriptor.name));
			}

			BMKillHandle ((GSHandle *) &listdata.descriptor.name);
			WrNewLine ();
		}
		err = WrEndblk ();
	}

	BMKillHandle ((GSHandle *) &deschandle);

	API_ComponentRefType**	comphandle = NULL;
	if (err == NoError) {
		err = ACAPI_Element_GetComponents (&elemHead, &comphandle, &num);
	}

	if (err == NoError && num > 0) {
		WriteBlock ("COMPONENTS", guid);
		WriteInt (num, WithNewLine);
		listdata.header.typeID = API_ComponentID;
		for (Int32 i = 0; i < num; i++) {
			listdata.header.index = (*comphandle)[i].index;
			switch ((*comphandle)[i].status) {
				case APIDBRef_Normal:
							listdata.header.setIndex = (*comphandle)[i].setIndex;
							err = ACAPI_ListData_Get (&listdata);
							break;
				case APIDBRef_Local:
							err = ACAPI_ListData_GetLocal ((*comphandle)[i].libIndex, &elemHead, &listdata);
							break;
				default:	err = APIERR_DELETED;
							break;
			}
			if (err == NoError) {
				WriteName (listdata.component.code);
				WriteName (listdata.component.keycode);
				WriteName (listdata.component.name);
				WriteFloat (listdata.component.quantity);
				WriteName (listdata.component.unitcode);
				WriteInt (listdata.component.unitRef);
			}
			WrNewLine ();
		}
		err = WrEndblk ();
	}

	BMKillHandle ((GSHandle *) &comphandle);

	if (err == NoError)
		err = WrEndblk ();

	return err;
}		// WrProperties


// -----------------------------------------------------------------------------
//	Write an opening into the file
// -----------------------------------------------------------------------------

void DumpWriter::WrAnOpening (API_ElemTypeID		typeID,
							  const API_Guid&		guid,
							  const API_Element*	wallelem,
							  double				dirangle,
							  double				wradius)
{
	API_Element		element;
	GSErrCode		err;

	BNZeroMemory (&element, sizeof (API_Element));
	element.header.typeID = typeID;
	element.header.guid = guid;
	err = ACAPI_Element_Get (&element);

	if (err == NoError) {
		char			str[32];

		if (typeID == API_WindowID) {
			strcpy (str, "Empty Window Opening");
			WriteBlock ("WINDOW", element.header.guid);
		} else {
			strcpy (str, "Empty Door Opening");
			WriteBlock ("DOOR", element.header.guid);
		}
		GS::UniString infoString;
		ACAPI_Database (APIDb_GetCompoundInfoStringID, &element.header.guid, &infoString);
		WriteElemHead (&element, infoString.ToCStr ().Get (), wallelem);

		if (element.window.openingBase.libInd == 0) {
			WriteName (str, WithNewLine);

		} else {
			API_LibPart		libPart;
			double			loc_x, loc_y, angle;

			BNZeroMemory (&libPart, sizeof (API_LibPart));
			libPart.index = element.window.openingBase.libInd;
			err = ACAPI_LibPart_Get (&libPart);

			if (err == NoError) {
				if (libPart.missingDef)
					WriteStr ("MISSING", WithNewLine);
				else {
					char docuname [256];
					CHTruncate (GS::UniString (libPart.docu_UName).ToCStr (), docuname, sizeof (docuname));
					WriteName (docuname, WithNewLine);
				}
			}

			DumpUtils::Get_DoorVector (&element.door, wradius, dirangle, &loc_x, &loc_y, &angle);

			WriteFloat (loc_x);
			WriteFloat (loc_y);
			WriteFloat (element.door.lower);
			WriteAngle (angle);
			WrNewLine ();

			WriteFloat (element.door.openingBase.width);
			WriteFloat (element.door.openingBase.height);
			WrNewLine ();
			WriteInt (element.door.openingBase.oSide);
			WriteInt (element.door.openingBase.reflected, WithNewLine);

			WriteFloat (element.door.revealDepthFromSide);
			WrNewLine ();
			WriteFloat (element.door.jambDepthHead);
			WriteFloat (element.door.jambDepthSill);
			WriteFloat (element.door.jambDepth);
			WrNewLine ();
			WriteInt (element.door.openingBase.mat, WithNewLine);

			WriteParams  (&element);
		}
		err = WriteSurfVol (typeID, element.header.guid);
		if (err == NoError)
			err = WrProperties (element.header.guid);
		if (err == NoError)
			err = WrEndblk ();
	}

	return;
}		// WrAnOpening


// -----------------------------------------------------------------------------
//	Write a cut plane into the file
// -----------------------------------------------------------------------------

GSErrCode DumpWriter::WrCutPlane (const API_Element* elem)
{
	API_ElementMemo		memo;
	GSErrCode			err;

	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	err = ACAPI_Element_GetMemo (elem->header.guid, &memo);

	if (err == NoError) {
		if (memo.gables != NULL) {
			char	buffer[256];
			Int32 	maxi = maxi = BMhGetSize ((GSHandle) memo.gables) / Sizeof32 (API_Gable);

			WriteInt (maxi, WithNewLine);
			for (Int32 i = 0; i < maxi; i++) {
				sprintf (buffer, "%.6lf %.6lf %.6lf %.6lf",
						(*memo.gables)[i].a, (*memo.gables)[i].b, (*memo.gables)[i].c, (*memo.gables)[i].d );
				WriteStr (buffer, WithNewLine);
			}
		} else
			WriteInt (0, WithNewLine);
	}

	ACAPI_DisposeElemMemoHdls (&memo);

	return err;
}		// WrCutPlane


// -----------------------------------------------------------------------------
//	Write the 2D polygon of an element into the file
// -----------------------------------------------------------------------------

GSErrCode DumpWriter::Wr2D_Poly (const API_Element* elem, const API_Polygon* poly)
{
	API_ElementMemo memo;
	GSErrCode		err;

	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	err = ACAPI_Element_GetMemo (elem->header.guid, &memo, APIMemoMask_Polygon | APIMemoMask_EdgeTrims);

	if (err == NoError) {
		const API_Coord* coords = *memo.coords;
		const API_PolyArc* arcs = *memo.parcs;
		const Int32* pends = *memo.pends;
		Int32 nArcs = poly->nArcs;
		Int32 nSubPolys = poly->nSubPolys;

		if (pends != NULL && coords != NULL) {
			WriteBlock ("POLY");
			if (elem->header.typeID != API_WallID)
				WriteInt (nSubPolys, WithNewLine);

			Int32 j = 1;
			for (Int32 i = 1; i <= nSubPolys; i++) {
				if (i == 1)
					WriteInt (pends[1], WithNewLine);
				else
					WriteInt (pends[i-1] - pends[i], WithNewLine);

				for ( ; j <= pends[i]; j++ ) {
					Int32 arcIndex = -1;
					if (arcs != NULL) {
						for (Int32 k = 0; k < nArcs && arcIndex < 0; k++) {
							if (arcs[k].begIndex == j && arcs[k].endIndex == j + 1)
								arcIndex = k;
						}
					}

					if (arcs != NULL && arcIndex >= 0) {
						Coord	beg, end, origo;
						beg.x = coords[arcs[arcIndex].begIndex].x;
						beg.y = coords[arcs[arcIndex].begIndex].y;
						end.x = coords[arcs[arcIndex].endIndex].x;
						end.y = coords[arcs[arcIndex].endIndex].y;
						Geometry::ArcGetOrigo (&beg, &end, arcs[arcIndex].arcAngle, &origo);
						WriteFloat (coords[j].x);
						WriteFloat (coords[j].y);
						WriteFloat (origo.x);
						WriteFloat (origo.y);
						WriteAngle (arcs[arcIndex].arcAngle);
					} else {
						WriteFloat (coords[j].x);
						WriteFloat (coords[j].y);
					}

					if (memo.edgeTrims != NULL) {
						WriteStr ("[");
						switch ((*memo.edgeTrims)[j].sideType) {
							case APIEdgeTrim_Vertical:		WriteStr ("Vertical");							break;
							case APIEdgeTrim_Perpendicular:	WriteStr ("Perpendicular");						break;
							case APIEdgeTrim_Horizontal:	WriteStr ("Horizontal");						break;
							case APIEdgeTrim_AlignWithCut:	WriteStr ("Align with cut");					break;
							case APIEdgeTrim_CustomAngle:	WriteAngle ((*memo.edgeTrims)[j].sideAngle);	break;
						}
						WriteStr ("]");
					}
					WrNewLine ();
				}
			}
			WrEndblk ();
		}
	}
	ACAPI_DisposeElemMemoHdls (&memo);

	return err;
}		// Wr2D_Poly


// -----------------------------------------------------------------------------
//	Write the 2D pivot polygon of a poly roof element into the file
// -----------------------------------------------------------------------------

GSErrCode DumpWriter::Wr2D_PivotPoly (const API_Element* elem, const API_Polygon* poly)
{
	API_ElementMemo memo;
	GSErrCode		err;

	if (elem->header.typeID != API_RoofID)
		return APIERR_BADPARS;

	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	err = ACAPI_Element_GetMemo (elem->header.guid, &memo, APIMemoMask_AdditionalPolygon);

	if (err == NoError) {
		const API_Coord* coords = *memo.additionalPolyCoords;
		const API_PolyArc* arcs = *memo.additionalPolyParcs;
		const Int32* pends = *memo.additionalPolyPends;
		Int32 nArcs = poly->nArcs;
		Int32 nSubPolys = poly->nSubPolys;

		if (pends != NULL && coords != NULL) {
			WriteBlock ("PIVOTPOLY");
			WriteInt (nSubPolys, WithNewLine);

			Int32 j = 1;
			for (Int32 i = 1; i <= nSubPolys; i++) {
				if (i == 1)
					WriteInt (pends[1], WithNewLine);
				else
					WriteInt (pends[i-1] - pends[i], WithNewLine);

				for ( ; j <= pends[i]; j++ ) {
					Int32 arcIndex = -1;
					if (arcs != NULL) {
						for (Int32 k = 0; k < nArcs && arcIndex < 0; k++) {
							if (arcs[k].begIndex == j && arcs[k].endIndex == j + 1)
								arcIndex = k;
						}
					}

					if (arcs != NULL && arcIndex >= 0) {
						Coord	beg, end, origo;
						beg.x = coords[arcs[arcIndex].begIndex].x;
						beg.y = coords[arcs[arcIndex].begIndex].y;
						end.x = coords[arcs[arcIndex].endIndex].x;
						end.y = coords[arcs[arcIndex].endIndex].y;
						Geometry::ArcGetOrigo (&beg, &end, arcs[arcIndex].arcAngle, &origo);
						WriteFloat (coords[j].x);
						WriteFloat (coords[j].y);
						WriteFloat (origo.x);
						WriteFloat (origo.y);
						WriteAngle (arcs[arcIndex].arcAngle);
					} else {
						WriteFloat (coords[j].x);
						WriteFloat (coords[j].y);
					}

					if (memo.pivotPolyEdges != NULL && j < pends[i]) {
						WriteStr ("[");
						WriteInt (memo.pivotPolyEdges[j].pivotEdgeUnId);
						for (Int32 k = 0; k < memo.pivotPolyEdges[j].nLevelEdgeData; k++)
							WriteAngle (memo.pivotPolyEdges[j].levelEdgeData[k].angle);
						WriteStr ("]");
					}
					WrNewLine ();
				}
			}
			WrEndblk ();
		}
	}
	ACAPI_DisposeElemMemoHdls (&memo);

	return err;
}		// Wr2D_PivotPoly


// -----------------------------------------------------------------------------
//	Write the profile polygon of a shell element
// -----------------------------------------------------------------------------

GSErrCode DumpWriter::Wr2D_ShellShape (const API_Element* elem)
{
	API_ElementMemo memo;
	GSErrCode		err;

	if (elem->header.typeID != API_RoofID)
		return APIERR_BADPARS;

	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	err = ACAPI_Element_GetMemo (elem->header.guid, &memo, APIMemoMask_Polygon);

	for (Int32 i = 0; i < (elem->shell.shellClass == API_RuledShellID ? 2 : 1) && err == NoError; i++) {
		API_ShellShapeData& shellShape = memo.shellShapes[i];
		const API_Coord* coords = *shellShape.coords;
		const API_PolyArc* arcs = *shellShape.parcs;
		const Int32* pends = *shellShape.pends;
		Int32 nArcs, nSubPolys;
		if (elem->shell.shellClass == API_ExtrudedShellID) {
			nArcs = elem->shell.u.extrudedShell.shellShape.nArcs;
			nSubPolys = elem->shell.u.extrudedShell.shellShape.nSubPolys;
		} else if (elem->shell.shellClass == API_RevolvedShellID) {
			nArcs = elem->shell.u.revolvedShell.shellShape.nArcs;
			nSubPolys = elem->shell.u.revolvedShell.shellShape.nSubPolys;
		} else {
			if (i == 0) {
				nArcs = elem->shell.u.ruledShell.shellShape1.nArcs;
				nSubPolys = elem->shell.u.ruledShell.shellShape1.nSubPolys;
			} else {
				nArcs = elem->shell.u.ruledShell.shellShape2.nArcs;
				nSubPolys = elem->shell.u.ruledShell.shellShape2.nSubPolys;
			}
		}

		if (pends != NULL && coords != NULL) {
			WriteBlock ("SHELLSHAPE");
			WriteInt (nSubPolys, WithNewLine);

			Int32 j = 1;
			for (Int32 i = 1; i <= nSubPolys; i++) {
				if (i == 1)
					WriteInt (pends[1], WithNewLine);
				else
					WriteInt (pends[i-1] - pends[i], WithNewLine);

				for ( ; j <= pends[i]; j++ ) {
					Int32 arcIndex = -1;
					if (arcs != NULL) {
						for (Int32 k = 0; k < nArcs && arcIndex < 0; k++) {
							if (arcs[k].begIndex == j && arcs[k].endIndex == j + 1)
								arcIndex = k;
						}
					}

					if (arcs != NULL && arcIndex >= 0) {
						Coord	beg, end, origo;
						beg.x = coords[arcs[arcIndex].begIndex].x;
						beg.y = coords[arcs[arcIndex].begIndex].y;
						end.x = coords[arcs[arcIndex].endIndex].x;
						end.y = coords[arcs[arcIndex].endIndex].y;
						Geometry::ArcGetOrigo (&beg, &end, arcs[arcIndex].arcAngle, &origo);
						WriteFloat (coords[j].x);
						WriteFloat (coords[j].y);
						WriteFloat (origo.x);
						WriteFloat (origo.y);
						WriteAngle (arcs[arcIndex].arcAngle);
					} else {
						WriteFloat (coords[j].x);
						WriteFloat (coords[j].y);
					}

					if (memo.pivotPolyEdges != NULL && j < pends[i]) {
						WriteStr ("[");
						WriteInt (memo.pivotPolyEdges[j].pivotEdgeUnId);
						for (Int32 k = 0; k < memo.pivotPolyEdges[j].nLevelEdgeData; k++)
							WriteAngle (memo.pivotPolyEdges[j].levelEdgeData[k].angle);
						WriteStr ("]");
					}
					WrNewLine ();
				}
			}
			WrEndblk ();
		}
	}
	ACAPI_DisposeElemMemoHdls (&memo);

	return err;
}		// Wr2D_ShellShape


// -----------------------------------------------------------------------------
//	Write the body of a morph element
// -----------------------------------------------------------------------------

GSErrCode DumpWriter::Wr_Morph (const API_Element* elem)
{
	API_ElementMemo memo;
	GSErrCode		err;

	if (elem->header.typeID != API_MorphID)
		return APIERR_BADPARS;

	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	err = ACAPI_Element_GetMemo (elem->header.guid, &memo);

	if (err != NoError || memo.morphBody == NULL)
		return APIERR_GENERAL;

	WriteBlock ("MORPH Body");
	WrNewLine ();
	WriteStr ("VERTs");

	for (ULong j = 0; j < memo.morphBody->GetVertexCount (); ++j) {
		const VERT& vertex = memo.morphBody->GetConstVertex (j);

		WriteInt	(j);
		WriteFloat	(vertex.x);
		WriteFloat	(vertex.y);
		WriteFloat	(vertex.z);
	}

	WriteStr ("EDGEs");
	for (ULong j = 0; j < memo.morphBody->GetEdgeCount (); ++j) {
		const EDGE& edge = memo.morphBody->GetConstEdge (j);

		WriteInt (j);
		WriteInt (edge.vert1);
		WriteInt (edge.vert2);
		WriteInt (edge.pgon1);
		WriteInt (edge.pgon2);
	}

	WriteStr ("PGONs ");
	WriteInt (memo.morphBody->GetPolygonCount ());
	for (ULong j = 0; j < memo.morphBody->GetPolygonCount (); ++j) {
		const Modeler::PGON& pgon = memo.morphBody->GetConstPolygon (j);

		WriteInt (j);
		WriteStr ("first edge: ");
		WriteInt (pgon.fpedg);
		WriteStr (", last edge: ");
		WriteInt (pgon.lpedg);
		WriteStr (", ivect: ");
		WriteInt (pgon.ivect);

		if ((BMpGetSize (reinterpret_cast<GSPtr> (memo.morphMaterialMapTable)) / sizeof (API_MaterialOverrideType)) >= j) {
			WriteStr ("Material ");
			WriteInt (memo.morphMaterialMapTable[j].material);
		}

		WriteStr ("NORMALVECTOR");

		Vector3D vector3D = pgon.GetNormalVector (*memo.morphBody);

		WriteFloat (vector3D.x);
		WriteFloat (vector3D.y);
		WriteFloat (vector3D.z);

	}

	return WrEndblk ();
}		// Wr_Morph

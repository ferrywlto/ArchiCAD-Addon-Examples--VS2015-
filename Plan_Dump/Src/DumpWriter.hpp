// *********************************************************************************************************************
// Description:		Interface of the DumpWriter class
//
// Module:			API Development Kit 19; Mac/Win
// Namespace:		-none-
// Contact person:	devsup@graphisoft.hu
//
// SG compatible
// *********************************************************************************************************************

#if !defined (DUMPWRITER_HPP)
#define DUMPWRITER_HPP

#pragma once

// --- Includes	----------------------------------------------------------------

#include	"APIEnvir.h"

#include	"ACAPinc.h"					// also includes APIdefs.h

#include	"File.hpp"
#include	"FileSystem.hpp"

#include	"DG.h"


// --- class DumpWriter --------------------------------------------------------

class DumpWriter {
private:
	IO::File*		outputFile;
	IO::Location*	fileLoc;

	GSErrCode		lastErr;						// cummulated file i/o error
	bool			isNewLine;
	short 			tabcount;            			// tabulator counter
	DGUnitData		unit;

public:
	enum NewLineFlag {
		NoNewLine,
		WithNewLine
	};

		 DumpWriter ();
		~DumpWriter ();

	GSErrCode	Open		(const IO::Location& inLoc);
	GSErrCode	Close		(void);

	GSErrCode	Write			(Int32 nBytes, GSPtr data);
	GSErrCode	Write			(const GS::UniString& val);
	GSErrCode	WrNewLine		(void);
	GSErrCode	WriteRGB		(const API_RGBColor* rgb);
	GSErrCode	WriteStr		(const char* val, NewLineFlag newLine = NoNewLine);
	GSErrCode	WriteStr		(const GS::UniString& val, NewLineFlag newLine = NoNewLine);
	GSErrCode	WriteName		(const char* val, NewLineFlag newLine = NoNewLine);
	GSErrCode	WriteName		(const GS::UniString& val, NewLineFlag newLine = NoNewLine);
	GSErrCode	WriteInt		(const Int32 val, NewLineFlag newLine = NoNewLine);
	GSErrCode	WriteFloat		(const double value);
	GSErrCode	WriteAngle		(const double value);
	GSErrCode	WriteBlock		(const char *name);
	GSErrCode	WriteBlock		(const char *name, const API_Guid& guid);
	GSErrCode	WriteBlock		(const char *name, short index);
	GSErrCode	WrEndblk		(void);
	GSErrCode	WriteDescText	(const char *text);
	GSErrCode	WriteElemHead	(const API_Element* elem, const char* info, const API_Element* wallelem);
	void		WriteParameter	(API_AddParID	typeID,
								 const char		*varname,
								 double 		value,
								 const char		*valueStr,
								 Int32 			dim1,
								 Int32 			dim2);
	void 		WriteParams		(API_AddParType **params);
	void		WriteParams		(const API_Element* elem);
	GSErrCode	WriteSurfVol	(API_ElemTypeID typeID, const API_Guid& guid, bool isPlaneRoof = true);
	GSErrCode	WrProperties	(const API_Guid& guid);
	void		WrAnOpening		(API_ElemTypeID		typeID,
								 const API_Guid&	guid,
								 const API_Element	*wallelem,
								 double				dirangle,
								 double				wradius);
	GSErrCode	WrCutPlane		(const API_Element* elem);
	GSErrCode	Wr2D_Poly		(const API_Element* elem, const API_Polygon* poly);
	GSErrCode	Wr2D_PivotPoly	(const API_Element* elem, const API_Polygon* poly);
	GSErrCode	Wr2D_ShellShape	(const API_Element* elem);
	GSErrCode	Wr_Morph		(const API_Element* elem);
};

#endif

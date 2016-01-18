// *****************************************************************************
// Source code for the primitive dumper
// Mac/Win
//
// Namespaces:        Contact person:
//     -None-			Somorjai
//
// [SG compatible] - Yes
// *****************************************************************************

#if defined (macintosh)
	#include <CoreServices/CoreServices.h>
#endif

#if defined (_MSC_VER)
	#ifndef WINDOWS
		#define		WINDOWS
	#endif
	#include "Win32Interface.hpp"

	#pragma warning (disable: 4068 4244 4505 4701)
	#pragma warning (push, 3)
#endif

#include	<stddef.h>
#include	<sstream>
using namespace std;

#if defined (WINDOWS)
	#pragma warning (pop)
#endif

#if !defined (ACExtension)
	#define	ACExtension
#endif

// --- INCLUDES ------------------------------------------------------------------

#include	"ACAPinc.h"
#include	"Dumper.h"


// -----------------------------------------------------------------------------
// Dump primitives
// -----------------------------------------------------------------------------

class SSC_API_Attribute {
	API_Attribute	attr;
public:
	SSC_API_Attribute (API_AttrTypeID typeID, short index);

	friend ostream& operator<< (ostream &outs, const SSC_API_Attribute &v);
	ostream& print (ostream &outs) const;
};

SSC_API_Attribute::SSC_API_Attribute (API_AttrTypeID typeID, short index)
{
	BNZeroMemory (&attr, sizeof (attr));
	attr.header.typeID = typeID;
	attr.header.index  = index;
	ACAPI_Attribute_Get (&attr);
}	// print()

ostream& SSC_API_Attribute::print (ostream &outs) const
{
	if (attr.header.typeID == API_FilltypeID &&
		(attr.filltype.subType == APIFill_Solid || attr.filltype.subType == APIFill_Empty)) {
		outs << " = " << attr.header.name << " (#" << attr.header.index << ", " << (attr.filltype.subType == APIFill_Solid ? "solid" : "empty") << ")" << endl;
	} else
		outs << " = " << attr.header.name << " (#" << attr.header.index << ")" << endl;

	return (outs);
}	// print()

ostream& operator<< (ostream &outs, const SSC_API_Attribute &v)
{
	v.print (outs);
	return (outs);
}	// operator<<()


class SSC_API_Prim_Head : public API_Prim_Head {
public:
	SSC_API_Prim_Head (const API_Prim_Head& ph) : API_Prim_Head (ph) {}
	friend ostream& operator<< (ostream &outs, const SSC_API_Prim_Head &v);
	ostream& print (ostream &outs) const;
};

ostream& SSC_API_Prim_Head::print (ostream &outs) const
{
	outs << "\tpen = " << pen << endl;
	outs << "\tlayer" << SSC_API_Attribute (API_LayerID, layer);
	outs << "\tdrwIndex = " << drwIndex << endl;
	outs << "\trenovationStatus = ";
	switch (renovationStatus) {
		case API_UndefinedStatus:
			outs << "undefined";
			break;

		case API_DefaultStatus:
			outs << "default";
			break;

		case API_ExistingStatus:
			outs << "existing";
			break;

		case API_NewStatus:
			outs << "new";
			break;

		case API_DemolishedStatus:
			outs << "demolished";
			break;
	}
	outs << endl;
	return (outs);
}	// print()

ostream& operator<< (ostream &outs, const SSC_API_Prim_Head &v)
{
	v.print (outs);
	return (outs);
}	// operator<<()


class SSC_API_Coord : public API_Coord {
public:
	SSC_API_Coord (const API_Coord& c) : API_Coord (c) {}
	friend ostream& operator<< (ostream &outs, const SSC_API_Coord &v);
	ostream& print (ostream &outs) const;
};

ostream& SSC_API_Coord::print (ostream &outs) const
{
	outs << "(" << x << ", " << y << ")" << endl;
	return (outs);
}	// print()

ostream& operator<< (ostream &outs, const SSC_API_Coord &v)
{
	v.print (outs);
	return (outs);
}	// operator<<()


class SSC_API_Coords {
	const API_Coord * coords;
	const UInt32 nco;
	const bool skipFirst;
public:
	SSC_API_Coords (const API_Coord * coo, UInt32 ncoo, bool skipF = true) : coords (coo), nco (ncoo), skipFirst (skipF) {}
	friend ostream& operator<< (ostream &outs, const SSC_API_Coord &v);
	ostream& print (ostream &outs) const;
};

ostream& SSC_API_Coords::print (ostream &outs) const
{
	for (UInt32 ii = (skipFirst ? 1 : 0); ii < nco; ii++) {
		outs << "\t\t" << SSC_API_Coord (coords[ii]);
	}
	return (outs);
}	// print()

ostream& operator<< (ostream &outs, const SSC_API_Coords &v)
{
	v.print (outs);
	return (outs);
}	// operator<<()


class SSC_API_PrimHatchBorder : public API_PrimHatchBorder {
public:
	SSC_API_PrimHatchBorder (const API_PrimHatchBorder& hb) : API_PrimHatchBorder (hb) {}
	friend ostream& operator<< (ostream &outs, const SSC_API_PrimHatchBorder &v);
	ostream& print (ostream &outs) const;
};

ostream& SSC_API_PrimHatchBorder::print (ostream &outs) const
{
	outs << "\t" << (doFrame ? "frame" : "no frame") << ", " << (fillbkgPen == 0 ? "transparent" : "opaque") << endl;
	outs << "\tfill" << SSC_API_Attribute (API_FilltypeID, fillInd);
	outs << "\tpen = " << fillPen << ", bgPen = " << fillbkgPen << endl;
	outs << "\ttype = " << hatchOrientation.type << ", flags = " << (GS::LongForStdio) hatchOrientation.flags << endl;
	outs << "\t" << ((hatchOrientation.flags & APIHatchOrinFlag_GlobalOrigo) == APIHatchOrinFlag_GlobalOrigo ? "global" : "local") << endl;
	outs << "\toffset = " << SSC_API_Coord (hatchOrientation.origo);
	outs << "\tmatrix = " << hatchOrientation.matrix00 << ", " << hatchOrientation.matrix10 << ", " << hatchOrientation.matrix01 << ", " << hatchOrientation.matrix11 << endl;
	return (outs);
}	// print()

ostream& operator<< (ostream &outs, const SSC_API_PrimHatchBorder &v)
{
	v.print (outs);
	return (outs);
}	// operator<<()


class SSC_API_PrimText : public API_PrimText {
public:
	SSC_API_PrimText (const API_PrimText& pt) : API_PrimText (pt) {}
	friend ostream& operator<< (ostream &outs, const SSC_API_PrimText &v);
	ostream& print (ostream &outs) const;
};

ostream& SSC_API_PrimText::print (ostream &outs) const
{
	outs << SSC_API_Prim_Head (head);
	outs << "\tind1 = " << ind1 << ", ind2 = " << ind2 << ", paramInd = " << paramInd << endl;
	outs << "\tfont" << SSC_API_Attribute (API_FontID, font);

	outs << "\tface = ";
	if ((faceBits & APIFace_Bold) != 0)			outs << "B";
	if ((faceBits & APIFace_Italic) != 0)		outs << "I";
	if ((faceBits & APIFace_Underline) != 0)	outs << "U";
	if (faceBits == 0)							outs << "normal";
	outs << ", anchor =";
	switch (anchor) {
		case APIAnc_LT:
			outs << "LT";
			break;

		case APIAnc_MT:
			outs << "MT";
			break;

		case APIAnc_RT:
			outs << "RT";
			break;

		case APIAnc_LM:
			outs << "LM";
			break;

		case APIAnc_MM:
			outs << "MM";
			break;

		case APIAnc_RM:
			outs << "RM";
			break;

		case APIAnc_LB:
			outs << "LB";
			break;

		case APIAnc_MB:
			outs << "APIAnc_MB";
			break;

		case APIAnc_RB:
			outs << "APIAnc_RB";
			break;
	}

	outs << ", just = ";
	switch (just) {
		case APIJust_Left:
			outs << "left";
			break;

		case APIJust_Center:
			outs << "center";
			break;

		case APIJust_Right:
			outs << "right";
			break;

		case APIJust_Full:
			outs << "full";
			break;
	}
	outs << endl;

	outs << "\tloc = " << SSC_API_Coord (loc);
	outs << "\theight = " << heightMM << " mm, width = " << widthMM << " mm, angle = " << angle << endl;
	return (outs);
}	// print()

ostream& operator<< (ostream &outs, const SSC_API_PrimText &v)
{
	v.print (outs);
	return (outs);
}	// operator<<()


class SSC_API_PrimPict : public API_PrimPict {
public:
	SSC_API_PrimPict (const API_PrimPict& pic) : API_PrimPict (pic) {}
	friend ostream& operator<< (ostream &outs, const SSC_API_PrimPict &v);
	ostream& print (ostream &outs) const;
};

ostream& SSC_API_PrimPict::print (ostream &outs) const
{
	outs << "\tname = " << pictName;
	if (mime != NULL && strlen (mime) != 0)
		outs << " (" << mime << ")";
	outs << endl;
	outs << "\tpixelSizeX = " << pixelSizeX << ", pixelSizeY = " << pixelSizeY << endl;
	outs << "\trotAngle = " << rotAngle << endl;
	if (transparent)
		outs << "\ttransparent" << endl;
	if (mirrored)
		outs << "\tmirrored" << endl;
	return (outs);
}	// print()

ostream& operator<< (ostream &outs, const SSC_API_PrimPict &v)
{
	v.print (outs);
	return (outs);
}	// operator<<()


void DumpPrimitive (const API_PrimElement* primElem,
					const void* par1, const void* /*par2*/, const void* /*par3*/)
{
	std::ostringstream	vlist;

	switch (primElem->header.typeID) {
		case API_ZombiePrimID:
			vlist << "  API_ZombiePrimID" << endl;
			break;

		case API_PrimPointID:
			vlist << "  API_PrimPointID" << endl;
			vlist << SSC_API_Prim_Head (primElem->header);
			vlist << "\tloc = " << SSC_API_Coord (primElem->point.loc);
			break;

		case API_PrimLineID:
			vlist << "  API_PrimLineID" << endl;
			vlist << SSC_API_Prim_Head (primElem->header);
			vlist << "\tpenWeight = " << primElem->line.penWeight << endl;
			vlist << "\tltype = " << SSC_API_Attribute (API_LinetypeID, primElem->line.ltypeInd);
			vlist << "\tc1 = " << SSC_API_Coord (primElem->line.c1);
			vlist << "\tc2 = " << SSC_API_Coord (primElem->line.c2);
			break;

		case API_PrimArcID:
			vlist << "  API_PrimArcID" << endl;
			vlist << SSC_API_Prim_Head (primElem->header);
			vlist << "\tpenWeight = " << primElem->arc.penWeight << endl;
			vlist << "\tltype = " << SSC_API_Attribute (API_LinetypeID, primElem->arc.ltypeInd);
			vlist << "\t" << (primElem->arc.solid ? "solid" : "non-solid") << ", " << (primElem->arc.whole ? "whole" : "part") << endl;
			vlist << "\torigin = " << SSC_API_Coord (primElem->arc.orig);
			vlist << "\tr = " << primElem->arc.r << ", begAng = " << primElem->arc.begAng << ", endAng = " << primElem->arc.endAng <<
				  ", angle = " << primElem->arc.angle << ", ratio = " << primElem->arc.ratio << endl;
			break;

		case API_PrimTextID:
			vlist << "  API_PrimTextID" << endl;
			vlist << SSC_API_PrimText (primElem->text);
			if (par1 != NULL && strlen ((const char *) par1) > 0)
				vlist << "\ttext = \"" << (const char *) par1 << "\"";
			break;

		case API_PrimPLineID:
			vlist << "  API_PrimPLineID" << endl;
			vlist << SSC_API_Prim_Head (primElem->header);
			vlist << "\tpenWeight = " << primElem->pline.penWeight << endl;
			vlist << "\tltype = " << SSC_API_Attribute (API_LinetypeID, primElem->pline.ltypeInd);
			vlist << "\tcoords: nCo = " << primElem->pline.nCoords << ", nArcs = " << primElem->pline.nArcs << endl;
			vlist << SSC_API_Coords ((API_Coord *) par1, primElem->pline.nCoords);
			break;

		case API_PrimTriID:
			vlist << "  API_PrimTriID" << endl;
			vlist << SSC_API_Prim_Head (primElem->header);
			vlist << "\tpenWeight = " << primElem->tri.penWeight << endl;
			vlist << "\tltype = " << SSC_API_Attribute (API_LinetypeID, primElem->tri.ltypeInd);
			vlist << "\t" << (primElem->tri.solid ? "solid" : "non-solid") << endl;
			vlist << "\tcoords = " << SSC_API_Coords (primElem->tri.c, 3, false);
			break;

		case API_PrimPolyID:
			vlist << "  API_PrimPolyID" << endl;
			vlist << SSC_API_Prim_Head (primElem->header);
			vlist << "\tpenWeight = " << primElem->poly.penWeight << endl;
			vlist << "\tltype = " << SSC_API_Attribute (API_LinetypeID, primElem->poly.ltypeInd);
			vlist << "\t" << (primElem->poly.solid ? "solid" : "non-solid") << endl;
			vlist << "\tcoords: nCo = " << primElem->poly.nCoords << ", m_subPolys = " << primElem->poly.nSubPolys << ", nArcs = " << primElem->poly.nArcs << endl;
			vlist << SSC_API_Coords ((API_Coord *) par1, primElem->poly.nCoords);
			break;

		case API_PrimPictID:
			vlist << "  API_PrimPictID" << endl;
			vlist << SSC_API_PrimPict (primElem->pict);
			break;

		case API_PrimCtrl_BegID:
			vlist << "+ API_PrimCtrl_BegID" << endl;
			{
				short	layNum = 0;
				if (ACAPI_Attribute_GetNum (API_LayerID, &layNum) == NoError)
					vlist << " no. of layers: " << layNum << endl;
			}
			break;

		case API_PrimCtrl_HatchBorderBegID:
			vlist << "+ API_PrimCtrl_HatchBorderBegID" << endl;
			if (par1 != NULL)
				vlist << SSC_API_PrimHatchBorder (*(API_PrimHatchBorder *) par1);
			break;

		case API_PrimCtrl_HatchBorderEndID:
			vlist << "- API_PrimCtrl_HatchBorderEndID" << endl;
			break;

		case API_PrimCtrl_HatchLinesBegID:
			vlist << "+ API_PrimCtrl_HatchLinesBegID" << endl;
			if (par1 != NULL)
				vlist << SSC_API_PrimHatchBorder (*(API_PrimHatchBorder *) par1);
			break;

		case API_PrimCtrl_HatchLinesEndID:
			vlist << "- API_PrimCtrl_HatchLinesEndID" << endl;
			break;

		case API_PrimCtrl_HoledimLinesBegID:
			vlist << "+ API_PrimCtrl_HoledimLinesBegID" << endl;
			break;

		case API_PrimCtrl_HoledimLinesEndID:
			vlist << "- API_PrimCtrl_HoledimLinesEndID" << endl;
			break;

		case API_PrimCtrl_ArrowBegID:
			vlist << "+ API_PrimCtrl_ArrowBegID" << endl;
			break;

		case API_PrimCtrl_ArrowEndID:
			vlist << "- API_PrimCtrl_ArrowEndID" << endl;
			break;

		case API_PrimCtrl_ElementRefID:
			vlist << "* API_PrimCtrl_ElementRefID" << endl;
			vlist << "\ttypeID = " << (int) ((API_PrimElemRef *) par1)->typeID << ", guid = " << APIGuidToString (((API_PrimElemRef *) par1)->guid).ToCStr ().Get () << endl;
			break;

		case API_PrimCtrl_EndID:
			vlist << "- API_PrimCtrl_EndID" << endl;
			break;

		case API_PrimCtrl_PlacedBorderBegID:
			vlist << "+ API_PrimCtrl_PlacedBorderBegID" << endl;
			if (par1 != NULL)
				vlist << SSC_API_PrimHatchBorder (*(API_PrimHatchBorder *) par1);
			break;

		case API_PrimCtrl_PlacedBorderEndID:
			vlist << "- API_PrimCtrl_PlacedBorderEndID" << endl;
			break;

		case API_PrimCtrl_TextBegID:
			vlist << "+ API_PrimCtrl_TextBegID" << endl;
			vlist << SSC_API_PrimText (primElem->text);
			if (par1 != NULL && strlen ((const char *) par1) > 0)
				vlist << "\text = \"" << (const char *) par1 << "\"";
			break;

		case API_PrimCtrl_TextEndID:
			vlist << "- API_PrimCtrl_TextEndID" << endl;
			break;

		case API_PrimCtrl_CWallPanelBegID:
			vlist << "+ API_PrimCtrl_CWallPanelBegID" << endl;
			if (primElem->panelKind.isWindow)		vlist << "\tkind = window" << endl;
			if (primElem->panelKind.isDoor)			vlist << "\tkind = door" << endl;
			break;

		case API_PrimCtrl_CWallPanelEndID:
			vlist << "- API_PrimCtrl_CWallPanelEndID" << endl;
			break;
	}

	GS::UniString vlistStr (vlist.str ().c_str ());
	vlistStr.ReplaceAll ("%", "%%");
	DBPrintf (vlistStr.ToCStr ().Get ());
}

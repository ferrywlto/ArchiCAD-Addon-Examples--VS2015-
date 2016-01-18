// *********************************************************************************************************************
// File:			APISpeedTest.cpp
//
// Description:		Speed test for API functions.
//
// Namespace:		-
//
// Contact person:	BIM
//
// *********************************************************************************************************************


#include "APISpeedTest.hpp"

#define ACExtension
#include "ACAPinc.h"


namespace {


static API_Coord	operator+ (const API_Coord& a, const API_Coord& b)
{
	API_Coord result;
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	return result;
}


static void		WriteReportLine (const GS::UniString& text)
{
	const GS::UniString& textH = (text.IsEmpty () ? GS::UniString () : "API Speed Test: " + text);
	ACAPI_WriteReport (textH.ToCStr ().Get (), false);
}


class MeasureTime {
public:
	MeasureTime (USize numberOfOperations, const GS::UniString& measurementText) :
		mNumberOfOperations	(numberOfOperations),
		mStartTime			(TIGetFineTicks ())
	{
		GS::UniString text = "*** " + measurementText + " ";
		while (text.GetLength () < 100)
			text += '*';

		WriteReportLine (text);
	}

	~MeasureTime ()
	{
		const double elapsedSeconds = double (TIGetFineTicks () - mStartTime) / TIGetFineTicksPerSec ();

		WriteReportLine (GS::UniString::Printf ("%6u operations: %12.6f seconds.", mNumberOfOperations, elapsedSeconds));
		WriteReportLine (GS::UniString::Printf ("%6u operation:  %12.6f seconds.", 1, (mNumberOfOperations == 0) ? elapsedSeconds : elapsedSeconds / mNumberOfOperations));
		WriteReportLine ("");
	}

private:
	const USize			mNumberOfOperations;
	const GSFineTicks	mStartTime;
};


typedef void	(*CreateAtReferencePointProc) (const API_Element& defaultElem, const API_Coord& referencePoint);


static void		CreateWallAtReferencePoint (const API_Element& wallDefault, const API_Coord& referencePoint)
{
	API_Coord dx;
	dx.x = 0.5;
	dx.y = 0.0;

	API_Element element = wallDefault;
	element.wall.begC = referencePoint;
	element.wall.endC = referencePoint + dx;

	ACAPI_Element_Create (&element, NULL);
}


static void		CreateColumnAtReferencePoint (const API_Element& columnDefault, const API_Coord& referencePoint)
{
	API_Element element = columnDefault;
	element.column.origoPos = referencePoint;

	ACAPI_Element_Create (&element, NULL);
}


static void		CreateLabelAtReferencePoint (const API_Element& labelDefault, const API_Coord& referencePoint)
{
	API_Coord d1;
	d1.x = 0.371;
	d1.y = 0.25;

	API_Coord d2;
	d2.x = 0.5;
	d2.y = 0.25;

	API_Element element = labelDefault;
	element.label.begC = referencePoint;
	element.label.midC = referencePoint + d1;
	element.label.endC = referencePoint + d2;

	ACAPI_Element_Create (&element, NULL);
}


static void		CreateHotspotAtReferencePoint (const API_Element& hotspotDefault, const API_Coord& referencePoint)
{
	API_Element element = hotspotDefault;
	element.hotspot.pos = referencePoint;

	ACAPI_Element_Create (&element, NULL);
}


static void		CreateSomeElements (const GS::UniString& measurementText, API_ElemTypeID elemType, CreateAtReferencePointProc createProc)
{
	const USize	 SomeNumber		= 500;
	const USize	 BlockWidth		= 20;
	const double GridCellSize	= 1.0;	// meter

	API_Element defaultElem;
	BNZeroMemory (&defaultElem, sizeof (API_Element));
	defaultElem.header.typeID = elemType;
	ACAPI_Element_GetDefaults (&defaultElem, NULL);

	{
		MeasureTime measurement (SomeNumber, measurementText);

		for (UIndex i = 0; i < SomeNumber; i++) {
			API_Coord referencePoint;
			referencePoint.x = (i % BlockWidth) * GridCellSize;
			referencePoint.y = (i / BlockWidth) * GridCellSize;

			createProc (defaultElem, referencePoint);
		}
	}
}


}	// namespace


void	Do_Speed_CreateSomeWalls (void)
{
	CreateSomeElements ("Create some Walls", API_WallID, CreateWallAtReferencePoint);
}


void	Do_Speed_CreateSomeColumns (void)
{
	CreateSomeElements ("Create some Columns", API_ColumnID, CreateColumnAtReferencePoint);
}


void	Do_Speed_CreateSomeLabels (void)
{
	CreateSomeElements ("Create some Labels", API_LabelID, CreateLabelAtReferencePoint);
}


void	Do_Speed_CreateSomeHotspots (void)
{
	CreateSomeElements ("Create some Hotspots", API_HotspotID, CreateHotspotAtReferencePoint);
}


void	Do_Speed_DeleteAllOneByOne (void)
{
	GS::Array<API_Guid> allElements;
	ACAPI_Element_GetElemList (API_ZombieElemID, &allElements);

	API_Elem_Head head;
	BNZeroMemory (&head, sizeof (API_Elem_Head));
	API_Elem_Head* pHead = &head;

	{
		MeasureTime measurement (allElements.GetSize (), "Delete All - one by one");

		for (auto it = allElements.Enumerate (); it != NULL; ++it) {
			pHead->guid = *it;
			ACAPI_Element_Delete (&pHead, 1);
		}
	}
}


void	Do_Speed_DeleteAllWithOneCall (void)
{
	GS::Array<API_Guid> allElements;
	ACAPI_Element_GetElemList (API_ZombieElemID, &allElements);

	API_Elem_Head** heads = (API_Elem_Head**) BMhAllClear (allElements.GetSize () * sizeof (API_Elem_Head));

	for (UIndex i = 0; i < allElements.GetSize (); i++)
		(*heads)[i].guid = allElements[i];

	{
		MeasureTime measurement (allElements.GetSize (), "Delete All - with one call");

		ACAPI_Element_Delete (heads, allElements.GetSize ());
	}

	BMhKill ((GSHandle*) &heads);
}

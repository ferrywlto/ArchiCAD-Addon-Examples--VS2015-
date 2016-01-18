// *****************************************************************************
// Source code for the AddOnObject Manager Add-On
// API Development Kit 19; Mac/Win
//
// Namespaces:			Contact person:
//		-None-
//
// [SG compatible] - Yes
// *****************************************************************************

#include "APIEnvir.h"
#define	_CUSTOMERDATA_TRANSL_

// ---------------------------------- Includes ---------------------------------

#include	"Array.hpp"
#include	"MemoryOChannel.hpp"
#include	"MemoryIChannel.hpp"
#include	"SetPlatformProtocol.hpp"
#include	"CustomerData.hpp"
#include	"ACAPinc.h"					// also includes APIdefs.h


CustomerData::CustomerData () :
	modified (false),
	created (false),
	markedAsDeleted (false)
{
}


GSErrCode	CustomerData::Read (GS::IChannel& ic)
{
	name.Read (ic);
	city.Read (ic);
	country.Read (ic);
	return ic.GetInputStatus ();
}

GSErrCode	CustomerData::Write (GS::OChannel& oc) const
{
	name.Write (oc);
	city.Write (oc);
	country.Write (oc);
	return oc.GetOutputStatus ();
}

void	CustomerData::GenerateRandomContent (void)
{
	static const char firstNames[10][32] = { "Albert", "Alex", "Alfred", "Amy", "Andrew", "Angelina", "Anne", "Anthony", "Arnold", "Arthur" };
	static const char familyNames[10][32] = { "Smith", "Johnson", "Williams", "Brown", "Jones", "Miller", "Davis", "Garcia", "Rodriguez", "Wilson" };
	static const char cities[10][32] = { "San Francisco", "Budapest", "Amsterdam", "Venezia", "Helsinki", "Limassol", "Tokyo", "Barcelona", "Abuja", "Auckland" };
	static const char countries[10][32] = { "United States", "Hungary", "Netherlands", "Italy", "Finland", "Cyprus", "Japan", "Spain", "Nigeria", "New Zeeland" };

	Int32 firstNameIndex = rand () % 10;
	Int32 familyNameIndex = rand () % 10;
	Int32 locationIndex = rand () % 10;
	name = GS::UniString (firstNames[firstNameIndex]) + " " + GS::UniString (familyNames[familyNameIndex]);
	city = GS::UniString (cities[locationIndex]);
	country = GS::UniString (countries[locationIndex]);
}

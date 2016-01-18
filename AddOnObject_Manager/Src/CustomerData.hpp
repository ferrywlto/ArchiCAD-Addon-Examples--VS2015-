// *****************************************************************************
// Source code for the AddOnObject Manager Add-On
// API Development Kit 19; Mac/Win
//
// Namespaces:			Contact person:
//		-None-
//
// [SG compatible] - Yes
// *****************************************************************************

#if !defined (CUSTOMERDATA_HPP)
#define CUSTOMERDATA_HPP
#include "APIEnvir.h"
#include "UniString.hpp"

// ---------------------------------- Includes ---------------------------------
enum TWCloseReason {
	TWCloseReason_Nothing,
	TWCloseReason_Reserve,
	TWCloseReason_Release,
	TWCloseReason_Grant,
	TWCloseReason_Request
};

class CustomerData {
public:
	GS::Guid			guid;
	GS::UniString		name;
	GS::UniString		city;
	GS::UniString		country;
	bool				modified;
	bool				created;
	bool				markedAsDeleted;

	CustomerData ();

	GSErrCode	Read (GS::IChannel& ic);
	GSErrCode	Write (GS::OChannel& oc) const;
	void		GenerateRandomContent (void);
};
#endif
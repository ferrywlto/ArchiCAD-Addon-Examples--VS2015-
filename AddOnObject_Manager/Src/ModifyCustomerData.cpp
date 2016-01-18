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
#define	_MODIFYCUSTOMERDATA_TRANSL_

// ---------------------------------- Includes ---------------------------------

#include	"CustomerData.hpp"
#include	"ACAPinc.h"					// also includes APIdefs.h

bool ModifyCustomerData (CustomerData& customerData, TWCloseReason& closeReason)
{
	customerData.GenerateRandomContent ();
	closeReason = TWCloseReason_Nothing;
	return true;
}


bool IsModifyButtonReservationDependent (void)
{
	return true;
}


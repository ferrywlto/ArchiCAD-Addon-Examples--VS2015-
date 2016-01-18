// *****************************************************************************
// Description:		Source code for the Notification Manager Add-On
//
// Module:			API Development Kit 19; Mac/Win
// Namespace:		-none-
// Contact person:	devsup@graphisoft.hu
//
// SG compatible
// *****************************************************************************
#include "APIEnvir.h"
#define _DEFAULT_OBSERVER_TRANSL_

// ------------------------------ Includes -------------------------------------

#include	<stdio.h>			/* sprintf */

#include	"ACAPinc.h"
#include	"Notification_Manager.h"


// ------------------------------ Constants ------------------------------------

// -------------------------------- Types --------------------------------------

// ------------------------------ Variables ------------------------------------

// ------------------------------ Prototypes -----------------------------------

// -----------------------------------------------------------------------------
// Defaults change handler function
// -----------------------------------------------------------------------------
GSErrCode __ACENV_CALL DefaultChangeHandlerProc (const API_ToolBoxItem *defElemType)

{
    char    msgStr[256];
    char    elemStr[32];

    if (GetElementTypeString (defElemType->typeID, elemStr))
        sprintf (msgStr, "%s element type defaults changed", elemStr);
    else
        sprintf (msgStr, "Unknown element type defaults changed");

    ACAPI_WriteReport (msgStr, false);

    return NoError;
}   // DefaultChangeHandlerProc


// ============================================================================
// Install Notification Handlers
//
//
// ============================================================================
void	Do_DefaultMonitor (bool switchOn)
{
	if (switchOn)
		ACAPI_Notify_CatchChangeDefaults (NULL, DefaultChangeHandlerProc);  // monitor all tools
	else
		ACAPI_Notify_CatchChangeDefaults (NULL, NULL);

	return;
}		/* Do_DefaultMonitor */

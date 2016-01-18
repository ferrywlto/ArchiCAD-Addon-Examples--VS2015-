// *****************************************************************************
// Source code for the Geometry Test Add-On
// API Development Kit 19; Mac/Win
//
// Namespaces:		Contact person:
//	-None-
//
// [SG compatible] - Yes
// *****************************************************************************

#include "APIEnvir.h"
#define	_Geometry_Test_TRANSL_


// ---------------------------------- Includes ---------------------------------

#include	<stdio.h>
#include	<string.h>

#include	"ACAPinc.h"					// also includes APIdefs.h

#include	"basicgeometry.h"

// ---------------------------------- Types ------------------------------------


// ---------------------------------- Variables --------------------------------


// ---------------------------------- Prototypes -------------------------------



// =============================================================================
//
// Main functions
//
// =============================================================================


static void		Do_Test (void)
{

	ACAPI_CallUndoableCommand ("Geometry Test -- Create elements",
		[&] () -> GSErrCode {
			API_Element		quadrangle [4];
			API_ElementMemo memo;
			Int32			i;
			GSErrCode		err = NoError;

			BNZeroMemory (&quadrangle[0], sizeof (API_Element));
			quadrangle[0].line.begC.x = -1.0; quadrangle[0].line.begC.y = 1.0;
			BNZeroMemory (&quadrangle[1], sizeof (API_Element));
			quadrangle[1].line.begC.x = -1.0; quadrangle[1].line.begC.y = -1.0;
			BNZeroMemory (&quadrangle[2], sizeof (API_Element));
			quadrangle[2].line.begC.x = 1.0; quadrangle[2].line.begC.y = -1.0;
			BNZeroMemory (&quadrangle[3], sizeof (API_Element));
			quadrangle[3].line.begC.x = 1.0; quadrangle[3].line.begC.y = 1.0;

			quadrangle[0].line.endC = quadrangle[1].line.begC;
			quadrangle[1].line.endC = quadrangle[2].line.begC;
			quadrangle[2].line.endC = quadrangle[3].line.begC;
			quadrangle[3].line.endC = quadrangle[0].line.begC;

			BNZeroMemory (&memo, sizeof (API_ElementMemo));

			//create lines
			for (i = 0; i < 4; i++) {

				quadrangle[i].header.typeID = API_LineID;
				quadrangle[i].header.layer = 1;
				quadrangle[i].header.floorInd = 1;

				err = ACAPI_Element_Create (&quadrangle[i], &memo);
				if (err != NoError)
					break;
			}


			API_Element tempLine;
			tempLine.header.typeID = API_LineID;
			ACAPI_Element_GetDefaults (&tempLine, NULL);
			tempLine.header.floorInd = 1;


			TRANMAT transformation;
			BNZeroMemory (&transformation, sizeof (TRANMAT));
			Geometry::TMIdentity (&transformation);
			Geometry::TMTranslation (&transformation, 2.0, 2.0, 0.0);

			for (i = 0; i < 4; i++) {
				tempLine.line.begC = quadrangle[i].line.begC;
				tempLine.line.endC = quadrangle[i].line.endC;

				double dummy = 0.0;
				Geometry::TMVector(&transformation, &tempLine.line.begC.x, &tempLine.line.begC.y, &dummy);
				Geometry::TMVector(&transformation, &tempLine.line.endC.x, &tempLine.line.endC.y, &dummy);

				err = ACAPI_Element_Create (&tempLine, &memo);
				if (err != NoError)
					break;
			}

			return err;
		});

	return;
}		/* Do_Test */

// -----------------------------------------------------------------------------
// Entry points to handle ArchiCAD events
//
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	MenuCommandHandler (const API_MenuParams *params)
{
	switch (params->menuItemRef.itemIndex) {
		case 1:		Do_Test ();				break;
	}

	return NoError;
}		// DoCommand


// =============================================================================
//
// Required functions
//
// =============================================================================


//------------------------------------------------------
// Dependency definitions
//------------------------------------------------------
API_AddonType	__ACENV_CALL	CheckEnvironment (API_EnvirParams* envir)
{
	if (envir->serverInfo.serverApplication != APIAppl_ArchiCADID)
		return APIAddon_DontRegister;

	ACAPI_Resource_GetLocStr (envir->addOnInfo.name, 32000, 1);
	ACAPI_Resource_GetLocStr (envir->addOnInfo.description, 32000, 2);

	return APIAddon_Normal;
}		/* CheckEnvironment */


//------------------------------------------------------
// Interface definitions
//------------------------------------------------------
GSErrCode	__ACENV_CALL	RegisterInterface (void)
{
	ACAPI_Register_Menu (32500, 0, MenuCode_UserDef, MenuFlag_Default);

	return NoError;
}		/* RegisterInterface */


//------------------------------------------------------
// Called when the Add-On has been loaded into memory
// to perform an operation
//------------------------------------------------------
GSErrCode	__ACENV_CALL Initialize	(void)
{
	GSErrCode err = ACAPI_Install_MenuHandler (32500, MenuCommandHandler);
	if (err != NoError)
		DBPrintf ("Geometry_Test:: Initialize() ACAPI_Install_MenuHandler failed\n");

	return err;
}		/* Initialize */


// -----------------------------------------------------------------------------
// FreeData
//		called when the Add-On is going to be unloaded
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	FreeData (void)
{
	return NoError;
}		// FreeData

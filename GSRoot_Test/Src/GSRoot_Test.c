// *****************************************************************************
// Source code for the GSRoot Test Add-On
// API Development Kit 19; Mac/Win
//
// Namespaces:			Contact person:
//		-None-
//
// [SG compatible] - Yes
// *****************************************************************************

#include "APIEnvir.h"
#define	_GSBASE_TEST_TRANSL_


// ---------------------------------- Includes ---------------------------------

#include	<stdio.h>
#include	<string.h>
#include	<time.h>

#include	"ACAPinc.h"					// also includes APIdefs.h
#include	"VA.hpp"

#include	"APICommon.h"


// ---------------------------------- Types ------------------------------------


// ---------------------------------- Variables --------------------------------


// ---------------------------------- Prototypes -------------------------------



// =============================================================================
//
// Main functions
//
// =============================================================================

// -----------------------------------------------------------------------------
// Test the Memory Manager
// -----------------------------------------------------------------------------

static void		MemoryTest (void)
{
	Int32	ret = BMCheckHeap () ;

	WriteReport_Alert ("Heap check returned: %d", ret);

	return;
}		// MemoryTest


// -----------------------------------------------------------------------------
// Test the Virtual Array Manager
//   - semi-infinite array inited for selected lines
//   - store the lines and do some art with them
// -----------------------------------------------------------------------------

static GSErrCode  	ArrayTest (bool timerOn)
{
	API_SelectionInfo	selectionInfo;
	API_Element			element;
	API_Neig			**selNeigs;
	DESC				array;
	Int32				ind, i, j, nElems;
	GSErrCode			err;

	err = VAInit (&array, 10, sizeof (API_Element));
	if (err != NoError)
		return err;

	if (timerOn)
		TIStart (1);

	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err != NoError) {		/* including APIERR_NOSEL */
		BMKillHandle ((GSHandle *) &selNeigs);
		return err;
	}

	BNZeroMemory (&selectionInfo, sizeof (API_SelectionInfo));
	nElems = BMGetHandleSize ((GSHandle) selNeigs) / sizeof (API_Neig);
	for (i = 0; i < nElems; i++) {
		BNZeroMemory (&element, sizeof (API_Element));
		element.header.guid  = (*selNeigs)[i].guid;

		err = ACAPI_Element_Get (&element);
		if (err != NoError) {
			ErrorBeep ("ACAPI_Element_Get", err);
			continue;
		}
		if (element.header.typeID != API_LineID)
			continue;


		ind = VASpac (&array);
		if (ind < 0) {
			ErrorBeep ("VASpac", APIERR_MEMFULL);
			break;
		}

		BNCopyMemory (*array.arrhdl + sizeof (API_Element) * ind, &element, sizeof (API_Element));
	}

	BMKillHandle ((GSHandle *) &selNeigs);

	if (timerOn) {
		TIStop (1);
		TIStart (2);
	}

	if (err == NoError) {
		if (array.lastind == 0) {
			WriteReport ("No lines were selected");
		} else {
			err = ACAPI_CallUndoableCommand ("Create array",
				[&] () -> GSErrCode {
					GSErrCode err1 = NoError;
					for (i = 0; i < 20 && err1 == NoError; i++) {
						for (j = 1; j <= array.lastind && err1 == NoError; j++) {		// first record is not used
							if (err == NoError) {
								((API_Element *) *array.arrhdl)[j].line.begC.y -= 1;
								((API_Element *) *array.arrhdl)[j].line.endC.x += 1;
								((API_Element *) *array.arrhdl)[j].line.linePen += 1;
								err1 = ACAPI_Element_Create (((API_Element *) *array.arrhdl) + j, NULL);
								if (err1 != NoError)
									ErrorBeep ("ACAPI_Element_Create", err1);
							}
						}
					}

					return err1;
				});
		}
	}

	if (timerOn)
		TIStop (2);

	VAFree (&array);

	return err;
}		// ArrayTest


// -----------------------------------------------------------------------------
// Test the DateTime Manager
//   - dump the modification date of the layer attribute of the clicked element
// -----------------------------------------------------------------------------

static void		DateTimeTest (void)
{
	API_Element		element;
	char			str[128];
	GSErrCode		err;

	BNZeroMemory (&element, sizeof (API_Element));

	if (!ClickAnElem ("Click an element to get its modification date", API_ZombieElemID, NULL, &element.header.typeID, &element.header.guid)) {
		WriteReport ("No element was clicked");
		return;
	}

	err = ACAPI_Element_Get (&element);
	if (err == NoError) {
		API_Attribute attribute;
		BNZeroMemory (&attribute, sizeof (API_Attribute));
		attribute.header.typeID = API_LayerID;
		attribute.header.index = element.header.layer;
		err = ACAPI_Attribute_Get (&attribute);
		if (err == NoError) {
			TIGetTimeString (attribute.header.modiTime, str, 0);
			WriteReport ("Modification date of the layer of this element: %s", str);
		}
	}

	return;
}		// DateTimeTest


// -----------------------------------------------------------------------------
// Test the Timer Manager
//  - use this manager for optimizing your code
// Launch DebugMonitor, output is written there
// -----------------------------------------------------------------------------

static void		TimerTest (void)
{
	TIReset (0, "Have some fun");
	TIReset (1, "Scan selection");
	TIReset (2, "Create lines");

	TIStart (0);
	ArrayTest (true);
	TIStop (0);

	TIPrintTimers ();

	return;
}		// TimerTest


// -----------------------------------------------------------------------------
// Test DebugManager functions
//   - shows the usage of Debug Manager functions
// Launch DebugMonitor, output is written there
// -----------------------------------------------------------------------------

#if defined (_MSC_VER)
#pragma optimize ("", off)
#endif

static void		Level4 (Int32 what)
{
	switch (what) {
		case 4: {				// Craete ACCES_VIOLATION
			char*	p = 0;
			*p = 'X';	//lint !e413 [Likely use of null pointer]
			DBPrintf (p);
		}	break;

		case 5: 				// Generate Debug Break (ASSERTION)
			DBBreak (__FILE__, __LINE__, "Test ASSERT from GSRoot_Test.c");
			break;

		case 6:	{				// Generate memory leak
			char*	pChar = new char [256];
			strcpy (pChar, "Leaked memory from GSRoot_Test.c");
		}	break;
	}
}

static void		Level3 (Int32 what)
{
	Level4 (what);
}

static void		Level2 (Int32 what)
{
	Level3 (what);
}

static void		Level1 (Int32 what)
{
	Level2 (what);
}

static void		DebugTest (Int32 what)
{
	GSFlags flags;

	switch (what) {
		case 1: 			// printing formatted text
			DBPrintf ("This is a message is from GSRoot Test %.2f\n", 10.0);
			break;

		case 2: 			// get flags
			flags = DBGetFlags (DBGeneral);

			DBPrintf ("The flags are:\n");
			if ((flags & DBEnableCallTrace) == 0)
				DBPrintf ("DBEnableCallTrace: OFF\n");
			else
				DBPrintf ("DBEnableCallTrace: ON\n");

			if ((flags & DBEnableBMTrace) == 0)
				DBPrintf ("DBEnableBMTrace: OFF\n");
			else
				DBPrintf ("DBEnableBMTrace: ON\n");

			if ((flags & DBEnableLogFile) == 0)
				DBPrintf ("DBEnableLogFile: OFF\n");
			else
				DBPrintf ("DBEnableLogFile: ON\n");

			if ((flags & DBEnableTrace) == 0)
				DBPrintf ("DBEnableTrace: OFF\n");
			else
				DBPrintf ("DBEnableTrace: ON\n");

			if ((flags & DBEnableLeakDetection) == 0)
				DBPrintf ("DBEnableLeakDetection: OFF\n");
			else
				DBPrintf ("DBEnableLeakDetection: ON\n");

			break;

		case 3:				// invert the BMTrace flag
			flags = DBGetFlags (DBGeneral);
			flags ^= DBEnableBMTrace;
			DBSetFlags (DBGeneral, flags);
			DebugTest (2);
			break;

		case 4:					// Create ACCESS_VIOLATION
		case 5: 				// Generate Debug Break (ASSERTION)
		case 6:					// Generate memory leak
			Level1 (what);
			break;
	}

	return;
}		// DebugTest

#if defined (_MSC_VER)
#pragma optimize ("", on)
#endif

#ifdef __APPLE__
#pragma mark -
#endif

// -----------------------------------------------------------------------------
// Handles menu commands
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL MenuCommandHandler (const API_MenuParams *menuParams)
{
	switch (menuParams->menuItemRef.menuResID) {
		case 32500:
			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		MemoryTest ();		break;
				case 2:		ArrayTest (false);	break;
				case 3:		DateTimeTest ();	break;
				case 4:		TimerTest ();		break;
				// -----
				case 6:		DebugTest (1);		break;  // dump message
				case 7:		DebugTest (2);		break;	// get flags
				case 8:		DebugTest (3);		break;	// toggle flag
				case 9:		DebugTest (4);		break;	// ACCESS_VIOLATION
				case 10:	DebugTest (5);		break;	// DBBreak
				case 11:	DebugTest (6);		break;	// GenerateLeak
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

#ifdef __APPLE__
#pragma mark -
#endif

// -----------------------------------------------------------------------------
// Dependency definitions
// -----------------------------------------------------------------------------

API_AddonType	__ACENV_CALL	CheckEnvironment (API_EnvirParams* envir)
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

GSErrCode	__ACENV_CALL	RegisterInterface (void)
{
	GSErrCode err;

	err = ACAPI_Register_Menu (32500, 32520, MenuCode_UserDef, MenuFlag_Default);
	if (err != NoError)
		DBPrintf ("DG_Test:: RegisterInterface() ACAPI_Register_Menu failed\n");

	return err;
}		// RegisterInterface


// -----------------------------------------------------------------------------
// Called when the Add-On has been loaded into memory
// to perform an operation
// -----------------------------------------------------------------------------

GSErrCode	__ACENV_CALL Initialize	(void)
{
	GSErrCode err = ACAPI_Install_MenuHandler (32500, MenuCommandHandler);
	if (err != NoError)
		DBPrintf ("DG_Test:: Initialize() ACAPI_Install_MenuHandler failed\n");

	return err;
}		// Initialize


// -----------------------------------------------------------------------------
// FreeData
//		called when the Add-On is going to be unloaded
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	FreeData (void)

{
	return NoError;
}		// FreeData

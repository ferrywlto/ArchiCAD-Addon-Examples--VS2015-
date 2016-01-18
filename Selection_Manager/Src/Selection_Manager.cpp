// *****************************************************************************
// Source code for the Selection Manager Add-On
// API Development Kit 19; Mac/Win
//
// Namespaces:		 Contact person:
//		 -None-
//
// [SG compatible] - Yes
// *****************************************************************************

#include "APIEnvir.h"
#define	_SELECTION_MANAGER_TRANSL_


// ---------------------------------- Includes ---------------------------------

#include	<stdio.h>
#include	<string.h>

#include	"ACAPinc.h"					// also includes APIdefs.h

#include	"APICommon.h"
#include	"UniString.hpp"


// ---------------------------------- Types ------------------------------------


// ---------------------------------- Variables --------------------------------


// ---------------------------------- Prototypes -------------------------------



// =============================================================================
//
// Main functions
//
// =============================================================================

// -----------------------------------------------------------------------------
// Find and select elements inside a given polygon
// -----------------------------------------------------------------------------

static void		Do_SelectElementsRelativeToPolygon (API_SelRelativePosID relativePos)
{
	API_SelectionInfo 	selectionInfo;
	API_Neig			**selNeigs = NULL;
	API_ElemTypeID		typeID;
	GSErrCode			err;

	// ----- Deselect all selected elements -----
	err = ACAPI_Element_Select (NULL, 0, false);
	if (err != NoError)
		ErrorBeep ("ACAPI_Element_Select", err);

	// -------- Modify selection marquee --------
	BNZeroMemory (&selectionInfo, sizeof (API_SelectionInfo));
	selectionInfo.typeID			= API_MarqueePoly;
	selectionInfo.marquee.nCoords	= 10;
	selectionInfo.marquee.coords	= (API_Coord **) BMAllocateHandle (selectionInfo.marquee.nCoords * sizeof (API_Coord), ALLOCATE_CLEAR, 0);
	if (selectionInfo.marquee.coords) {
		API_Coord *coords = *selectionInfo.marquee.coords;
		BNZeroMemory (coords, selectionInfo.marquee.nCoords * sizeof (API_Coord));
		coords[0].x = coords[0].y = 0.0;
		coords[1].x = 2.0;	coords[1].y =-2.0;
		coords[2].x = 4.0;	coords[2].y = 0.0;
		coords[3].x = 4.0;	coords[3].y = 2.0;
		coords[4].x = 8.0;	coords[4].y = 2.0;
		coords[5].x = 8.0;	coords[5].y = 6.0;
		coords[6].x = 4.0;	coords[6].y = 6.0;
		coords[7].x = 4.0;	coords[7].y = 4.0;
		coords[8].x = 0.0;	coords[8].y = 4.0;
		coords[9] = coords[0];

		selectionInfo.multiStory = true;

		err = ACAPI_Selection_SetMarquee (&selectionInfo);

		BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);

		if (err != NoError) {
			ErrorBeep ("ACAPI_Selection_SetMarquee", err);
			return;
		}
	}

	// ----------- Check selection -----------
	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, false, false, relativePos);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Selection_Get", err);
		return;
	}

	// ------- Enumerate the selection -------
	Int32 nElem = BMGetHandleSize ((GSHandle) selNeigs) / sizeof (API_Neig);
	for (Int32 i = 0; i < nElem; i++) {
		typeID = Neig_To_ElemID ((*selNeigs)[i].neigID);
		if (typeID != API_ZombieElemID) {
			GS::UniString reportString = GS::UniString::Printf ("%d. %s #%T",
																i + 1, ElemID_To_Name (typeID), APIGuid2GSGuid ((*selNeigs)[i].guid).ToUniString ().ToPrintf ());
			WriteReport (reportString.ToCStr ().Get ());
		}
	}

	// ---------- Add to selection ----------
	err = ACAPI_Element_Select (selNeigs, nElem, true);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Element_Select", err);
	}

	if (err == APIERR_NOSEL)
		err = NoError;

	BMKillHandle ((GSHandle *) &selNeigs);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);

	return;
}		/* Do_SelectElementsRelativeToPolygon */


// -----------------------------------------------------------------------------
// List the Selection
//   - dump marquee polygon (not yet)
// -----------------------------------------------------------------------------

static void		Do_ListSelection (void)
{
	API_SelectionInfo 	selectionInfo;
	API_Neig			**selNeigs = NULL;
	API_ElemTypeID		typeID;
	GSErrCode			err;

	// ------- Check selection -------
	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, false, false);
	if (err != NoError) {
		ErrorBeep ("ACAPI_Selection_Get", err);
		return;
	}

	switch (selectionInfo.typeID) {
		case API_SelEmpty:
					WriteReport ("No selection");
					break;

		case API_SelElems:
					WriteReport ("Selected elements:");
					break;

		default:
					WriteReport ("Marquee data:");
					if (selectionInfo.multiStory)
						WriteReport ("  - multiple stories");
					else
						WriteReport ("  - single story");
					break;
	}

	// ------- Enumerate the selection -------

	Int32 nElem = BMGetHandleSize ((GSHandle) selNeigs) / sizeof (API_Neig);
	for (Int32 i = 0; i < nElem; i++) {
		typeID = Neig_To_ElemID ((*selNeigs)[i].neigID);
		if (typeID != API_ZombieElemID) {
			GS::UniString reportString = ElemID_To_Name (typeID);
			reportString.Append (" #");
			reportString.Append (APIGuid2GSGuid ((*selNeigs)[i].guid).ToUniString ());

			if ((*selNeigs)[i].inIndex != 0)
				reportString.Append (GS::UniString::Printf ("  inIndex:%d", (*selNeigs)[i].inIndex));

			if ((*selNeigs)[i].elemPartType != APINeigElemPart_None) {
				switch ((*selNeigs)[i].elemPartType) {
					case APINeigElemPart_Edge:		reportString.Append (GS::UniString::Printf ("  Edge index:%d", (*selNeigs)[i].elemPartIndex));		break;
					case APINeigElemPart_Vertex:	reportString.Append (GS::UniString::Printf ("  Vertex index:%d", (*selNeigs)[i].elemPartIndex));	break;
					case APINeigElemPart_Face:		reportString.Append (GS::UniString::Printf ("  Face index:%d", (*selNeigs)[i].elemPartIndex));		break;
					default:						break;
				}
			}

			WriteReport (reportString.ToCStr ().Get ());
		}
	}

	if (err == APIERR_NOSEL)
		err = NoError;

	BMKillHandle ((GSHandle *) &selNeigs);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);

	return;
}		/* Do_ListSelection */


// -----------------------------------------------------------------------------
// Select/Deselect the clicked element
//   - also select other elements in the same group
//   - check whether the layers are locked
//   - check whether the layers are hidden
// -----------------------------------------------------------------------------

static void		Do_SelectClickedElement (bool select)
{
	API_Neig** selNeigs = (API_Neig**) BMhAllClear (sizeof (API_Neig));
	if (selNeigs == NULL)
		return;

	if (ClickAnElem ("Click an elem to select/deselect", API_ZombieElemID, *selNeigs)) {
		GSErrCode err = ACAPI_Element_Select (selNeigs, 1, select);
		if (err != NoError)
			ErrorBeep ("ACAPI_Element_Select", err);
	} else {
		WriteReport_Alert ("No element was clicked");
	}

	BMKillHandle ((GSHandle *) &selNeigs);

	return;
}		// Do_SelectClickedElement


// -----------------------------------------------------------------------------
// Deselect all selected elements
// -----------------------------------------------------------------------------

static void		Do_DeselectAll (void)
{
 	GSErrCode		err;

	err = ACAPI_Element_Select (NULL, 0, false);
	if (err != NoError)
		ErrorBeep ("ACAPI_Element_Select", err);

	return;
}		// Do_DeselectAll


// -----------------------------------------------------------------------------
// Delete the currently selected elements
//   - also delete other elements in the same group
//   - check whether the layers are locked
//   - check whether the layers are hidden
// -----------------------------------------------------------------------------

static void		Do_DeleteSelectedElements (void)
{
	ACAPI_CallUndoableCommand ("Delete selected elements",
		[&] () -> GSErrCode {
			GSErrCode	err = ACAPI_Element_Delete (NULL, 0);
			if (err != NoError)
				ErrorBeep ("ACAPI_Element_Delete", err);

			return err;
		});

	return;
}		// Do_DeleteSelectedElements


// -----------------------------------------------------------------------------
// Duplicate each selected element to one story above
//   - every parameter must be the same
// -----------------------------------------------------------------------------

static void		Do_DuplicateSelectedElements (void)
{
	ACAPI_CallUndoableCommand ("Duplicate selected elements",
		[&] () -> GSErrCode {
			API_SelectionInfo 	selectionInfo;
			API_Neig			**selNeigs = NULL;
			API_Element			element;
			API_ElementMemo		memo;
			API_ElemTypeID		typeID;
			GSErrCode			err;

			err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, false);
			if (err != NoError) {
				ErrorBeep ("ACAPI_Selection_Get", err);
				return err;
			}

			if (selectionInfo.typeID == API_SelEmpty) {
				WriteReport ("Nothing is selected");
				return APIERR_NOSEL;
			}

			for (Int32 i = 0; i < selectionInfo.sel_nElem; i++) {
				typeID = Neig_To_ElemID ((*selNeigs)[i].neigID);
				if (typeID != API_ZombieElemID) {
					BNZeroMemory (&element, sizeof (API_Element));
					BNZeroMemory (&memo, sizeof (API_ElementMemo));
					element.header.guid  = (*selNeigs)[i].guid;
					err = ACAPI_Element_Get (&element);
					if (err == NoError && element.header.hasMemo)
						err = ACAPI_Element_GetMemo (element.header.guid, &memo);
					if (err != NoError)
						ACAPI_WriteReport ("Unable to get the selected element", false);

					if (err == NoError) {
						element.header.floorInd ++;
						err = ACAPI_Element_Create (&element, &memo);
						DBASSERT_STR (err != APIERR_IRREGULARPOLY, "Existing element can't have irregular polygon");
						if (err != NoError) {
							char	buffer[256];
							sprintf (buffer, "Unable to duplicate: {%s}", APIGuidToString (element.header.guid).ToCStr ().Get ());
							ACAPI_WriteReport (buffer, false);
							err = NoError;
						}
					}
					ACAPI_DisposeElemMemoHdls (&memo);
				}
			}

			if (err == APIERR_NOSEL)
				err = NoError;

			BMKillHandle ((GSHandle *) &selNeigs);
			BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);

			return err;
		});

	return;
}		// Do_DuplicateSelectedElements


// -----------------------------------------------------------------------------
// Select the connecting edges of the clicked Morph face
// -----------------------------------------------------------------------------

static void		Do_SelectMorphFaceEdges (void)
{
	API_Elem_Head	elemHead;
	API_Neig		theNeig;

	BNZeroMemory (&elemHead, sizeof (API_Elem_Head));
	BNZeroMemory (&theNeig, sizeof (API_Neig));

	if (ClickAnElem ("Click a Morph Face", API_MorphID, &theNeig, NULL, &elemHead.guid, NULL, false) && theNeig.elemPartType == APINeigElemPart_Face) {
		API_ElemInfo3D info3D;
		BNZeroMemory (&info3D, sizeof (API_ElemInfo3D));
		GSErrCode err = ACAPI_Element_Get3DInfo (elemHead, &info3D);
		if (err == NoError) {
			API_Component3D component;
			BNZeroMemory (&component, sizeof (component));
			component.header.typeID = API_BodyID;
			component.header.index  = info3D.fbody;
			err = ACAPI_3D_GetComponent (&component);
			if (err == NoError) {
				component.header.typeID = API_PgonID;
				component.header.index  = theNeig.elemPartIndex + 1;
				err = ACAPI_3D_GetComponent (&component);
				if (err == NoError) {
					Int32 nItem = component.pgon.lpedg - component.pgon.fpedg + 1;
					if (nItem > 0) {
						API_Neig** selNeigs = (API_Neig**) BMhAllClear (nItem * sizeof (API_Neig));
						if (selNeigs != NULL) {
							Int32 pedg = component.pgon.fpedg;
							theNeig.elemPartType = APINeigElemPart_Edge;
							for (Int32 ind = 0; ind < nItem; ++ind, ++pedg) {
								component.header.typeID = API_PedgID;
								component.header.index  = pedg;
								err = ACAPI_3D_GetComponent (&component);
								if (err == NoError) {
									theNeig.elemPartIndex = GS::Abs (component.pedg.pedg) - 1;
									(*selNeigs)[ind] = theNeig;
								}
							}
							err = ACAPI_Element_Select (selNeigs, nItem, true);
							if (err != NoError)
								ErrorBeep ("ACAPI_Element_Select", err);
						}

						BMKillHandle ((GSHandle *) &selNeigs);
					}
				}
			}
		}
	} else {
		WriteReport_Alert ("No Morph Face was clicked");
	}

	return;
}		// Do_SelectMorphFaceEdges


// -----------------------------------------------------------------------------
// Entry points to handle ArchiCAD events
//
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	MenuCommandHandler (const API_MenuParams *params)
{
	switch (params->menuItemRef.itemIndex) {
		case 1:		Do_ListSelection ();				break;
		case 2:		Do_SelectClickedElement (true);		break;
		case 3:		Do_SelectClickedElement (false);	break;
		case 4:		Do_DeselectAll ();					break;
		/* ----- */
		case 6:		Do_DeleteSelectedElements ();		break;
		case 7:		Do_DuplicateSelectedElements ();	break;
		case 8:		Do_SelectMorphFaceEdges ();			break;
		/* ----- */
		case 10:	Do_SelectElementsRelativeToPolygon (API_InsidePartially);		break;
		case 11:	Do_SelectElementsRelativeToPolygon (API_InsideEntirely);		break;
		case 12:	Do_SelectElementsRelativeToPolygon (API_OutsidePartially);		break;
		case 13:	Do_SelectElementsRelativeToPolygon (API_OutsideEntirely);		break;
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
		DBPrintf ("Selection_Manager:: Initialize() ACAPI_Install_MenuHandler failed\n");

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

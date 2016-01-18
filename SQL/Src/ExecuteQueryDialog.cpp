// *****************************************************************************
// File:			ExecuteQueryDialog.cpp
//
// Description:		Executing an sql query to the database
//
// Project:			APITools/SQL
//
// Namespace:		SQL
//
// Contact person:	BLA
//
// SG compatible
// *****************************************************************************

// --- Includes ----------------------------------------------------------------

#include "SQL.hpp"
#include "APIDefs.h"

#include "ExecuteQueryDialog.hpp"

#include "SQLResource.h"
#include "FileSystem.hpp"

#if defined (WINDOWS)
	#include "Win32ShellInterface.hpp"
#else
	#include <ApplicationServices/ApplicationServices.h>
#endif

namespace SQL {

// -----------------------------------------------------------------------------
// Construction / destruction
// -----------------------------------------------------------------------------

CExecuteQueryDialog::CExecuteQueryDialog () :
	ModalDialog (ACAPI_GetOwnResModule (), IDD_EXECUTEQUERY, InvalidResModule),
	m_static (GetReference (), IDC_QUERY),
	m_queryEdit (GetReference (), IDC_QUERYEDIT),
	m_executeButton (GetReference (), IDB_EXECUTE),
	m_closeButton (GetReference (), IDB_CLOSE)
{
	m_executeButton.Attach (*this);
	m_closeButton.Attach (*this);
}


CExecuteQueryDialog::~CExecuteQueryDialog ()
{
	m_executeButton.Detach (*this);
	m_closeButton.Detach (*this);
}


// -----------------------------------------------------------------------------
// ButtonClicked
// -----------------------------------------------------------------------------

void	CExecuteQueryDialog::ButtonClicked (const DG::ButtonClickEvent& ev)
{
	if (ev.GetSource () == &m_executeButton) {

		GS::UniString queryUniStr;
		queryUniStr = m_queryEdit.GetText (DG::EOL::CRLF);

		if (queryUniStr.GetLength () > 0) {

			IO::Location	io_appFolderLoc;
			IO::fileSystem.GetSpecialLocation (IO::FileSystem::ApplicationFolder, &io_appFolderLoc);

			IO::Location appFolderLoc;
			IO::fileSystem.GetSpecialLocation (IO::FileSystem::ApplicationFolder, &appFolderLoc);
			IO::Location xmlLoc (appFolderLoc, IO::Name ("QueryResult.xml"));

			API_SQLQuery queryStruct;
			
			Int32 nQueryCStrLength = Strlen32 (UniStringToConstCString (queryUniStr));
			char* queryCStr = new char[nQueryCStrLength + 1];
			if (queryCStr == NULL)
				return;

			CHCopyC (UniStringToConstCString (queryUniStr), queryCStr);
			
			queryStruct.query = queryCStr;
			queryStruct.xmlLoc = &xmlLoc;
			queryStruct.useDefaultXSL = true;
			queryStruct.xslLoc = NULL;

			GSErrCode err = ACAPI_Goodies (APIAny_ExecuteSQLQueryID, (void*)&queryStruct, NULL);

			if (err == APIERR_SQLPARSE) {
				::DGResAlert (ACAPI_GetOwnResModule (), LISTING_ALERT_PARSEERROR);

			} else if (err == APIERR_SQLEXECUTE) {
				::DGResAlert (ACAPI_GetOwnResModule (), LISTING_ALERT_EXECUTIONERROR);

			} else if (err == APIERR_SQLANY) {
				DBBREAK ();
			} else {
				// OK:
#if defined (WINDOWS)
				GS::UniString	path;

				IO::Location (appFolderLoc, IO::Name ("QueryResult.xml")).ToPath (&path);

				::ShellExecuteW (NULL, L"open", path.ToUStr (), NULL, NULL, SW_SHOWNORMAL);
#else
				IO::Location	loc (io_appFolderLoc, IO::Name ("QueryResult.xml"));
				IO::File        file (loc);

				CFURLRef cfurl;
				if (loc.ToCFURL (&cfurl) == NoError) {
					LSOpenCFURLRef (cfurl, NULL);
					CFRelease (cfurl);
				}
#endif
			}

			delete[] queryCStr;
		}
	}
	else if (ev.GetSource () == &m_closeButton) {
		PostCloseRequest (Cancel);
	}
}

}	// namespace SQL

// *****************************************************************************
// File:			SQL.hpp
//
// Description:		SQL addon required add-on functions and callbacks
//
// Project:			APITools/SQL
//
// Namespace:		-
//
// Contact person:	BLA
//
// SG compatible
// *****************************************************************************

#define	MY_OWN_CODE			'SQL1'								// Add-on OWND recordja a .grc file-ban

// --- Includes ----------------------------------------------------------------

#if !defined (ACExtension)
	#define	ACExtension
#endif

#if defined (_MSC_VER)
	#if !defined (WINDOWS)
		#define		WINDOWS
	#endif
#endif

#if defined (WINDOWS)
	#pragma warning (disable: 4068 4800 4505)
#endif

#if defined (macintosh)
#endif

#include	"ACAPinc.h"

// -----------------------------------------------------------------------------
//							Prototypes
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL APIMenuCommandProc_Main (const API_MenuParams *menuParams);
API_AddonType	__ACENV_CALL	CheckEnvironment (API_EnvirParams* envir);
GSErrCode	__ACENV_CALL	RegisterInterface (void);
GSErrCode	__ACENV_CALL Initialize	(void);
GSErrCode	__ACENV_CALL FreeData	(void);

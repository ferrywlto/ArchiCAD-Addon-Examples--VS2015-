// *****************************************************************************
// Header file for Inter-AddOn communication
// API Development Kit 19; Mac/Win
//
// Namespaces:			Contact person:
//		-None-
//
// [SG compatible] - Yes
// *****************************************************************************

#ifndef _COMMUNICATIONCLIENT
#define _COMMUNICATIONCLIENT


// =============================================================================
//
// Constants
//
// =============================================================================

#define CmdID_PutText			'CMD1'
#define CmdID_BegBinaryLink		'BEGB'
#define CmdID_EndBinaryLink		'ENDB'


// =============================================================================
//
// Functions exported to the server Add-On (Communication Manager)
//
// =============================================================================

typedef void	CounterProc (void);

typedef struct {
	CounterProc			*counterProc;
} CallBackTbl_Client;


#endif	// _COMMUNICATIONCLIENT

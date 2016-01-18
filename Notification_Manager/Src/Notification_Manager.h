// *****************************************************************************
// Description:		Header file for the Notification Manager Add-On
//
// Module:			API Development Kit 19; Mac/Win
// Namespace:		-none-
// Contact person:	devsup@graphisoft.hu
//
// SG compatible
// *****************************************************************************

#if !defined _NOTIFICATION_MANAGER_H
#define _NOTIFICATION_MANAGER_H

#define ELEMSTR_LEN				256

bool	GetElementTypeString (API_ElemTypeID typeID, char *elemStr);

void	Do_CatchProjectEvent (bool switchOn);
void	Do_DefaultMonitor (bool switchOn);
void	Do_ClickedElementMonitor (bool switchOn);
void	Do_ListMonitoredElements ();
void	Do_ElementMonitor (bool switchOn);
void	Do_ToolMonitor (bool switchOn);
void	Do_SelectionMonitor (bool switchOn);
void	Do_ReservationMonitor (bool switchOn);
void	Do_MarkSelElems (void);
void	Do_ListOwnedElements (void);
void	Do_AttachElementURLRef (void);
void	Do_ListElementURLRef (void);
void	Do_LinkElements (void);

GSErrCode __ACENV_CALL	ElementEventHandlerProc (const API_NotifyElementType *elemType);

#endif	// _NOTIFICATION_MANAGER_H

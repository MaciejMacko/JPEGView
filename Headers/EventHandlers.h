/*********************************************************//* This source code copyright (c) 1991-2001, Aaron Giles *//* See the Read Me file for licensing information.       *//* Contact email: mac@aarongiles.com                     *//*********************************************************/#ifndef __EVENTHANDLERS__#define __EVENTHANDLERS__extern void MainEventLoop(void);extern void DispatchMainEvent(EventRecord *theEvent, Boolean isDialogEvent);extern void HandleNullEvent(void);extern void DispatchBalloonHelp(void);extern void EnsureFrontmost(void);extern void HandleMouseDownDialog(EventRecord *theEvent);extern void HandleMouseDownEvent(EventRecord *theEvent);extern void HandleMenuClick(EventRecord *theEvent);extern void HandleContentClick(EventRecord *theEvent, WindowPtr theWindow);extern void HandleDragClick(EventRecord *theEvent, WindowPtr theWindow);extern void HandleCloseClick(EventRecord *theEvent, WindowPtr theWindow);extern void HandleGrowClick(EventRecord *theEvent, WindowPtr theWindow);extern void HandleZoomClick(EventRecord *theEvent, short thePart, WindowPtr theWindow);extern void HandleKeyDownDialog(EventRecord *theEvent);extern void HandleKeyDownEvent(EventRecord *theEvent);extern void HandleActivateDialog(EventRecord *theEvent);extern void HandleActivateEvent(EventRecord *theEvent);extern void HandleUpdateDialog(EventRecord *theEvent);extern void HandleUpdateEvent(EventRecord *theEvent);extern void HandleOSEvent(EventRecord *theEvent);extern void HandleDiskEvent(EventRecord *theEvent);extern void DispatchDialogEvent(DialogPtr theDialog, short theItem);extern void MyInvalMenuBar(void);#endif
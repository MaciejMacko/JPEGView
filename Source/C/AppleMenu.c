/*********************************************************//* This source code copyright (c) 1991-2001, Aaron Giles *//* See the Read Me file for licensing information.       *//* Contact email: mac@aarongiles.com                     *//*********************************************************/#if THINK_C#include "THINK.Header"#elif applec#pragma load ":Headers:MPW.Header"#elif __MWERKS__//#include "MW.Header"#else#include "JPEGView.h"#endifstatic DialogPtr gAboutHelpDialog = nil;static void CloseAboutHelp(void);/* * HandleAppleChoice(theItem) * *     Purpose: Accepts menu events from the Apple menu *      Inputs: theItem = the menu item number *     Returns: nothing * */ void HandleAppleChoice(short theItem) {	short accNumber;	Str255 accName;		switch (theItem) {		case appleAboutItem:			SendCreateWindow(kAboutWindowID);			break;		case appleAboutHelpItem:			SendCreateWindow(kAboutHelpDialogID);			break;		default:			GetItem(gAppleMenu, theItem, accName);			accNumber = OpenDeskAcc(accName);			break;	}}/* * OpenAbout() * *     Purpose: Handles the display of the about box *      Inputs: none *     Returns: nothing * */void OpenAbout(void){	DescType openScreen, openFull, openPalette, drawQuality;	ImageHandle theImage;	OSErr theErr = noErr;	FSSpec theFile;		for (theImage = gImageRoot; theImage; theImage = (*theImage)->next)		if (IsAboutBox(theImage)) {			ChangeActive((*theImage)->window);			return;		}	GetOpenDefaults(&openScreen, &openFull, &openPalette, &drawQuality);	gGetAbout = true;	theFile.vRefNum = -LMGetSFSaveDisk();	theFile.parID = LMGetCurDirStore();	BlockMove(gString[strAboutJPEGView], theFile.name, *gString[strAboutJPEGView] + 1);	StartSpinning();	theErr = DoOpenDocument(&theFile, true, openScreen, openFull, false, true, 				openPalette, drawQuality, false, false, false, false);	StopSpinning(&qd.arrow);}void OpenAboutHelp(void){	if (gAboutHelpDialog) {		if (!WindowVisible(gAboutHelpDialog)) FWShowWindow(gAboutHelpDialog);		return;	}	if (!(gAboutHelpDialog = AllocateDialog())) return;	PushPort();	MySetPort(nil);	if (gAboutHelpDialog = FWGetNewDialog(rAboutHelpDialog, (Ptr)gAboutHelpDialog, (WindowPtr)-1)) {		PlaceWindow(gAboutHelpDialog, &gThePrefs.aboutHelpBounds, CenterWindow(gAboutHelpDialog));	} else DeallocateDialog(gAboutHelpDialog);	PopPort();}void HandleAboutHelpEvent(short theItem){	switch (theItem) {		case kDialogClose:			CloseAboutHelp();			return;	}}WindowPtr GetAboutHelpWindow(void){	return gAboutHelpDialog;}static void CloseAboutHelp(void) {	PushPort();	MySetPort(nil);	if (gAboutHelpDialog) {		SaveWindowPosition(gAboutHelpDialog, &gThePrefs.aboutHelpBounds);		FWCloseDialog(gAboutHelpDialog);		DisposeHandle(((DialogPeek)gAboutHelpDialog)->items);		DeallocateDialog(gAboutHelpDialog);	}	gAboutHelpDialog = nil;	PopPort();	ChangeActive(FWFrontWindow());}
/*********************************************************//* This source code copyright (c) 1991-2001, Aaron Giles *//* See the Read Me file for licensing information.       *//* Contact email: mac@aarongiles.com                     *//*********************************************************/#ifndef __REQUIREDEVENTHANDLERS__#define __REQUIREDEVENTHANDLERS__pascal OSErr HandleOpenApplication(AppleEvent *theEvent, AppleEvent *reply, long refCon);void GetOpenDefaults(DescType *openScreen, DescType *openFull, DescType *openPalette, 	DescType *drawQuality);pascal OSErr HandleOpenDocuments(AppleEvent *theEvent, AppleEvent *reply, long refCon);pascal OSErr HandlePrintDocuments(AppleEvent *theEvent, AppleEvent *reply, long refCon);extern OSErr DoPrintDocument(ImageHandle theImage, Boolean first);pascal OSErr HandleQuitApplication(AppleEvent *theEvent, AppleEvent *reply, long refCon);#endif
/*********************************************************//* This source code copyright (c) 1991-2001, Aaron Giles *//* See the Read Me file for licensing information.       *//* Contact email: mac@aarongiles.com                     *//*********************************************************/#if THINK_C#include "THINK.Header"#elif applec#pragma load ":Headers:MPW.Header"#elif __MWERKS__//#include "MW.Header"#else#include "JPEGView.h"#endif/* * AppPropertyAccessor(classWanted, container, containerClass, keyForm, keyData, *					    resultToken, theRefCon) * *     Purpose: Finds a file property and returns the corresponding property token *      Inputs: classWanted = the element class desired (typeProperty) *				container = token for this element's container *				containerClass = class of this element's container (cFile) *				keyForm = the search key type *				keyData = descriptor containing search key data *				resultToken = descriptor containing the resulting token *				theRefCon = reference constant *     Returns: an OSErr describing the result * */pascal OSErr AppPropertyAccessor(DescType classWanted, AEDesc *container, 	DescType containerClass, DescType keyForm, AEDesc *keyData, 	AEDesc *resultToken, long theRefCon){#if applec#pragma unused(classWanted, container, containerClass, theRefCon)#endif	ObjectTokenHandle theList;	DescType propertyType;	OSErr theErr;		if ((keyForm != formPropertyID) || (keyData->descriptorType != typeType))		return errAECantSupplyType;	propertyType = *(DescType *)*keyData->dataHandle;	switch (propertyType) {		// Non-modifiable properties		case pBestType:		case pClass:		case pClipboard:		case pDefaultType:		case pIsFrontProcess:		case pName:		case pUserSelection:		case pVersion:			break;		default:			return errAEEventNotHandled;	}	if (!(theList = MakeSingleObjectToken(nil))) return memFullErr;	(*theList)->property = propertyType;	(*theList)->objclass = typeNull;	theErr = AECreateHandleDesc(typeProperty, (Handle)theList, resultToken);	DisposeHandle((Handle)theList);	return theErr;}/* * GetApplicationData(theWindow, typeWanted, theData) * *     Purpose: Extracts the data from the application *      Inputs: theWindow = the window associated with this object *				typeWanted = the type we're asking for *				theData = the data we save *     Returns: an OSErr describing the result * */OSErr GetApplicationData(DescType typeWanted, AEDesc *theData){	if (typeWanted == typeWildCard) typeWanted = typeObjectSpecifier;	if (typeWanted == typeBest) typeWanted = typeObjectSpecifier;	if (typeWanted != typeObjectSpecifier) return errAECantSupplyType;	theData->descriptorType = typeNull;	theData->dataHandle = nil;	return noErr;}/* * GetAppPropertyData(theWindow, theProperty, typeWanted, theData) * *     Purpose: Extracts the data from the application's properties *      Inputs: theProperty = the property wanted *				typeWanted = the type we're asking for *				theData = the data we save *     Returns: an OSErr describing the result * */OSErr GetAppPropertyData(DescType theProperty, DescType typeWanted, AEDesc *theData){	ProcessSerialNumber thePSN, selfPSN = { 0, kCurrentProcess };	WindowPtr theWindow = FWFrontWindow();	Boolean theBoolean;	DescType theType;	AEDesc bestData;	OSErr theErr;	long theLong;		switch (theProperty) {		case pBestType:		case pDefaultType:			theType = typeObjectSpecifier;			theErr = AECreateDesc(typeType, (void *)&theType, sizeof(DescType), &bestData);			break;		case pClass:			theType = cApplication;			theErr = AECreateDesc(typeType, (void *)&theType, sizeof(DescType), &bestData);			break;		case pClipboard:			theErr = AECreateList(nil, 0, false, &bestData);			break;		case pIsFrontProcess:			GetFrontProcess(&thePSN);			SameProcess(&selfPSN, &thePSN, &theBoolean);			theErr = AECreateDesc(typeBoolean, (void *)&theBoolean, sizeof(DescType), &bestData);			break;		case pName:			theErr = AEMakeIntlDesc(gString[strJPEGView], &bestData);			break;		case pUserSelection:			if (theWindow) theErr = MakeSelectionObject(theWindow, &bestData);			else theErr = errAENoSuchObject;			break;		case pVersion:			theLong = kVersion;			theErr = AECreateDesc(typeLongInteger, (void *)&theLong, sizeof(long), &bestData);			break;		default:			theErr = errAEReadDenied;			break;	}	if (theErr != noErr) return theErr;	if ((bestData.descriptorType == typeWanted) ||		(typeWanted == typeWildCard) || (typeWanted == typeBest))		theErr = AEDuplicateDesc(&bestData, theData);	else theErr = AECoerceDesc(&bestData, typeWanted, theData);	AEDisposeDesc(&bestData);	return theErr;}/* * SetAppPropertyData(theProperty, theData) * *     Purpose: Sets the data for the application's properties *      Inputs: theProperty = the property wanted *				theData = the data to set it to *     Returns: an OSErr describing the result * */OSErr SetAppPropertyData(DescType theProperty, AEDesc *theData){#if applec#pragma unused(theData)#endif	OSErr theErr;	switch (theProperty) {		default:			theErr = errAEWriteDenied;			break;	}	return theErr;}/* * MakeAppPropertyObject(theProperty, theObject) * *     Purpose: Creates an object descriptor for the given application property *      Inputs: theWindow = pointer to the window, or nil for all windows *				theObject = pointer to an AEDesc to store the result *     Returns: an OSErr describing what went wrong * */OSErr MakeAppPropertyObject(DescType theProperty, AEDesc *theObject){	AEDesc theKey, theContainer;	OSErr theErr = noErr;		theErr = AECreateDesc(typeType, (void *)&theProperty, sizeof(DescType), &theKey);	if (theErr == noErr) {		theContainer.descriptorType = typeNull;		theContainer.dataHandle = nil;		theErr = CreateObjSpecifier(typeProperty, &theContainer, formPropertyID, 				 &theKey, false, theObject);		AEDisposeDesc(&theKey);	}	return theErr;}
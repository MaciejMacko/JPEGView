/*********************************************************//* This source code copyright (c) 1991-2001, Aaron Giles *//* See the Read Me file for licensing information.       *//* Contact email: mac@aarongiles.com                     *//*********************************************************/#if THINK_C#include "THINK.Header"#elif applec#pragma load ":Headers:MPW.Header"#elif __MWERKS__//#include "MW.Header"#else#include "JPEGView.h"#endif/* * DocumentAccessor(classWanted, container, containerClass, keyForm, keyData, *					resultToken, theRefCon) * *     Purpose: Finds a document according to the specified keyForm and returns the *					corresponding document token *      Inputs: classWanted = the element class desired (cDocument) *				container = token for this element's container *				containerClass = class of this element's container (typeNull/cWindow) *				keyForm = the search key type *				keyData = descriptor containing search key data *				resultToken = descriptor containing the resulting token *				theRefCon = reference constant *     Returns: an OSErr describing the result * */pascal OSErr DocumentAccessor(DescType classWanted, AEDesc *container, 	DescType containerClass, DescType keyForm, AEDesc *keyData, 	AEDesc *resultToken, long theRefCon){#if applec#pragma unused (classWanted, theRefCon)#endif	ObjectTokenHandle theList;	OSErr theErr = noErr;	if (containerClass == cWindow) {		if (IsValidForOne(keyForm, keyData)) {			if ((theErr = HandToHand((Handle *)&theList)) != noErr) return theErr;			PurgeNonDocuments(theList);		} else return errAENoSuchObject;	} else {		switch (containerClass) {			case cDocument:				theList = (ObjectTokenHandle)container->dataHandle;				if ((theErr = HandToHand((Handle *)&theList)) != noErr) return theErr;				break;			case typeNull:				if (!(theList = MakeAllWindowsToken())) return memFullErr;				PurgeNonDocuments(theList);				break;			default:				return errAENoSuchObject;		}		switch (keyForm) {			case formAbsolutePosition:				theErr = GetAbsoluteWindow(keyData, &theList);				break;			case formName:				theErr = GetNamedDocument(keyData, &theList);				break;			case formRelativePosition:			case formTest:			case formRange:			case formPropertyID:			default:				return errAEBadKeyForm;				break;		}	}	if (!theList || !(*theList)->count || (theErr != noErr)) {		DisposeHandle((Handle)theList);		if (theErr == noErr) theErr = errAENoSuchObject;	} else {		theErr = AECreateHandleDesc(cDocument, (Handle)theList, resultToken);		DisposeHandle((Handle)theList);	}	return theErr;}/* * DocPropertyAccessor(classWanted, container, containerClass, keyForm, keyData, *					   resultToken, theRefCon) * *     Purpose: Finds a document property and returns the corresponding property token *      Inputs: classWanted = the element class desired (typeProperty) *				container = token for this element's container *				containerClass = class of this element's container (cDocument) *				keyForm = the search key type *				keyData = descriptor containing search key data *				resultToken = descriptor containing the resulting token *				theRefCon = reference constant *     Returns: an OSErr describing the result * */pascal OSErr DocPropertyAccessor(DescType classWanted, AEDesc *container, 	DescType containerClass, DescType keyForm, AEDesc *keyData, 	AEDesc *resultToken, long theRefCon){#if applec#pragma unused (classWanted, containerClass, theRefCon)#endif	ObjectTokenHandle theList = (ObjectTokenHandle)container->dataHandle;	DescType propertyType;	OSErr theErr;		if ((keyForm != formPropertyID) || (keyData->descriptorType != typeType))		return errAECantSupplyType;	propertyType = *(DescType *)*keyData->dataHandle;	switch (propertyType) {		// Non-modifiable properties		case pBestType:		case pClass:		case pDefaultType:		case pIsModified:		case pSelection:		// Modifiable properties		case pName:			break;		default:			return errAEEventNotHandled;	}	theErr = HandToHand((Handle *)&theList);	if (theErr != noErr) return theErr;	(*theList)->property = propertyType;	(*theList)->objclass = cDocument;	theErr = AECreateHandleDesc(typeProperty, (Handle)theList, resultToken);	DisposeHandle((Handle)theList);	return theErr;}/* * GetDocumentData(theWindow, typeWanted, theData) * *     Purpose: Extracts the data from the document *      Inputs: theWindow = the window associated with this object *				typeWanted = the type we're asking for *				theData = the data we save *     Returns: an OSErr describing the result * */OSErr GetDocumentData(WindowPtr theWindow, DescType typeWanted, AEDesc *theData){	if (typeWanted == typeWildCard) typeWanted = typeObjectSpecifier;	if (typeWanted == typeBest) typeWanted = typeObjectSpecifier;	if (typeWanted != typeObjectSpecifier) return errAECantSupplyType;	return MakeDocumentObject(theWindow, theData);}/* * GetDocPropertyData(theWindow, theProperty, typeWanted, theData) * *     Purpose: Extracts the data from the document's properties *      Inputs: theWindow = the window associated with this object *				theProperty = the property wanted *				typeWanted = the type we're asking for *				theData = the data we save *     Returns: an OSErr describing the result * */OSErr GetDocPropertyData(WindowPtr theWindow, DescType theProperty, DescType typeWanted, 	AEDesc *theData){	ImageHandle theImage = FindImage(theWindow);	Boolean theBoolean = false;	DescType theType;	Str255 theString;	AEDesc bestData;	OSErr theErr;		if (!theImage) return errAEReadDenied;	switch (theProperty) {		case pBestType:		case pDefaultType:			theType = typeObjectSpecifier;			theErr = AECreateDesc(typeType, (void *)&theType, sizeof(DescType), &bestData);			break;		case pClass:			theType = cDocument;			theErr = AECreateDesc(typeType, (void *)&theType, sizeof(DescType), &bestData);			break;		case pIsModified:			theErr = AECreateDesc(typeBoolean, (void *)&theBoolean, sizeof(Boolean), &bestData);			break;		case pName:			if (theImage) {				BlockMove((*theImage)->file.name, theString, 64);				if ((*theImage)->num) {					AddChar(theString, ' ');					AddNumber(theString, (*theImage)->num);				}				theErr = AEMakeIntlDesc(theString, &bestData);			}			break;		case pSelection:			theErr = MakeSelectionObject(theWindow, &bestData);			break;		default:			return errAETypeError;	}	if (theErr != noErr) return theErr;	if ((bestData.descriptorType == typeWanted) ||		(typeWanted == typeWildCard) || (typeWanted == typeBest))		theErr = AEDuplicateDesc(&bestData, theData);	else theErr = AECoerceDesc(&bestData, typeWanted, theData);	AEDisposeDesc(&bestData);	return theErr;}/* * SetDocPropertyData(theWindow, theProperty, theData) * *     Purpose: Sets the data for the application's properties *      Inputs: theWindow = the associated window *				theProperty = the property wanted *				theData = the data to set it to *     Returns: an OSErr describing the result * */OSErr SetDocPropertyData(WindowPtr theWindow, DescType theProperty, AEDesc *theData){	ImageHandle theImage = FindImage(theWindow);	Str255 theString;	OSErr theErr;	if (!theImage) return errAEWriteDenied;	switch (theProperty) {		case pName:			theErr = AEExtractPString(theData, 0, theString);			if (theErr == noErr) theErr = DoSetWindowTitle(theWindow, theString);			break;		default:			theErr = errAEWriteDenied;			break;	}	return theErr;}/* * MakeDocumentObject(theWindow, theObject) * *     Purpose: Creates a document object descriptor for the given window *      Inputs: theWindow = pointer to the window, or nil for all windows *				theObject = pointer to an AEDesc to store the result *     Returns: an OSErr describing what went wrong * */OSErr MakeDocumentObject(WindowPtr theWindow, AEDesc *theObject){	long index = (theWindow == (WindowPtr)kAEAll) ? kAEAll : 0;	AEDesc theKey, theContainer;	OSErr theErr = noErr;	WindowPtr window;		if (!index) {		for (window = GetFirstWindow(), index++; window && (window != theWindow); 			 window = NextWindow(window))			if (FindImage(window)) index++;	}	theErr = AECreateDesc((index == kAEAll) ? typeAbsoluteOrdinal : typeLongInteger, 			 (void *)&index, sizeof(long), &theKey);	if (theErr == noErr) {		theContainer.descriptorType = typeNull;		theContainer.dataHandle = nil;		theErr = CreateObjSpecifier(cDocument, &theContainer, formAbsolutePosition, 				 &theKey, false, theObject);		AEDisposeDesc(&theKey);	}	return theErr;}/* * MakeDocPropertyObject(theWindow, theProperty, theObject) * *     Purpose: Creates a property object descriptor for the given drawing area's property *      Inputs: theWindow = pointer to the window, or nil for all windows *				theObject = pointer to an AEDesc to store the result *     Returns: an OSErr describing what went wrong * */OSErr MakeDocPropertyObject(WindowPtr theWindow, DescType theProperty, AEDesc *theObject){	AEDesc theKey, theContainer;	OSErr theErr = noErr;		theErr = MakeDocumentObject(theWindow, &theContainer);	if (theErr == noErr) {		theErr = AECreateDesc(typeType, (void *)&theProperty, sizeof(DescType), &theKey);		if (theErr == noErr) {			theErr = CreateObjSpecifier(typeProperty, &theContainer, formPropertyID, 					 &theKey, false, theObject);			AEDisposeDesc(&theKey);		}		AEDisposeDesc(&theContainer);	}	return theErr;}/* * IsValidForOne(keyForm, keyData) * *     Purpose: Determines if the given keyForm is valid for specifying a single object *					for classes that can only contain a single element *      Inputs: keyForm = the key form specified *				keyData = the associated key data *     Returns: true if this keyForm works for a single object; false otherwise * */Boolean IsValidForOne(DescType keyForm, AEDesc *keyData){	long index;		switch (keyForm) {		case formAbsolutePosition:			index = *(long *)*keyData->dataHandle;			switch (keyData->descriptorType) {				case typeLongInteger:					if ((index < -1) || (index > 1)) return false;					return true;				case typeAbsoluteOrdinal:					switch (index) {						case kAEFirst:						case kAELast:						case kAEMiddle:						case kAEAny:						case kAEAll:							return true;						default:							return false;					}			}		case formName:		case formRelativePosition:		case formTest:		case formRange:		case formPropertyID:		default:			return errAEBadKeyForm;			break;	}}/* * PurgeNonDocuments(theList) * *     Purpose: Scans through the given list of objects and removes any non-documents *      Inputs: theList = a handle to an object token list *     Returns: nothing * */void PurgeNonDocuments(ObjectTokenHandle theList){	short i, newCount = 0, count = (*theList)->count;	Ptr window;		for (i = 0; i < count; i++) {		window = (*theList)->object[i];		if (FindImage((WindowPtr)window)) (*theList)->object[newCount++] = window;	}	if (newCount != count) {		SetHandleSize((Handle)theList, sizeof(ObjectToken) + (newCount - 1) * sizeof(Ptr));		(*theList)->count = newCount;	}}/* * GetNamedDocument(keyData, theList) * *     Purpose: Looks up a window from a text string *      Inputs: keyData = descriptor containing search key data *				theList = handle to the set of windows to search *     Returns: an OSErr describing the result * */OSErr GetNamedDocument(AEDesc *keyData, ObjectTokenHandle *theList){	short i, count = (**theList)->count;	uchar theString[256];	WindowPtr *window;	ImageHandle image;	OSErr theErr;		if (!count) return errAENoSuchObject;	if ((theErr = AEExtractPString(keyData, 0, theString)) != noErr) return theErr;	for (i = 0, window = (WindowPtr *)(**theList)->object; i < count; i++, window++)		if (image = FindImage(*window))			if (EqualString((*image)->file.name, theString, false, false)) break;	if (i == count) return errAENoSuchObject;	DisposeHandle((Handle)*theList);	if (!(*theList = MakeSingleObjectToken((Ptr)*window))) return memFullErr;	return noErr;}/* * CountDocuments() * *     Purpose: Counts the number of documents in the application *      Inputs: none *     Returns: the number of windows found * */short CountDocuments(void){	ImageHandle image;	short count = 0;		for (image = gImageRoot; image; image = (*image)->next) count++;	return count;}/* * IndexedDocument(index) * *     Purpose: Locates a document in the linked list by index *      Inputs: index = the number of the image in the list we seek *     Returns: a pointer to the desired window, or nil if none found * */ WindowPtr IndexedDocument(short index){	ImageHandle theImage;	short i;		for (i = 1, theImage = gImageRoot; theImage && (i < index); 		 i++, theImage = (*theImage)->next);	return theImage ? (*theImage)->window : nil;}/* * GetDocumentIndex(theWindow) * *     Purpose: Determines a document's numerical index in the linked list *      Inputs: theWindow = a pointer to the document's window whose index we shall find *     Returns: the index * */ short GetDocumentIndex(WindowPtr theWindow){	ImageHandle curImage;	short i;	for (i = 1, curImage = gImageRoot; curImage && ((*curImage)->window != theWindow);		 i++, curImage = (*curImage)->next);	return curImage ? i : 1;}
/*********************************************************//* This source code copyright (c) 1991-2001, Aaron Giles *//* See the Read Me file for licensing information.       *//* Contact email: mac@aarongiles.com                     *//*********************************************************/#if THINK_C#include "THINK.Header"#elif applec#pragma load ":Headers:MPW.Header"#elif __MWERKS__//#include "MW.Header"#else#include "JPEGView.h"#endif/* * MonitorInit() * *     Purpose: Sets up the linked list of screen devices, and sets their inverse color *					table resolutions to 5 bits per component *      Inputs: none *     Returns: nothing * */void MonitorInit(void) {	DeviceLoopDrawingUPP addDevice = NewDeviceLoopDrawingProc((ProcPtr)AddDevice);	MonitorHandle theMonitor;	GDHandle theDevice;	if (!(theDevice = GetMainDevice())) FatalError(errNoDeviceInfo);	if (!(theMonitor = gMainMonitor = NewMonitor(theDevice))) FatalError(errNoMemory);	DeviceLoop(nil, addDevice, (long)&theMonitor, allDevices + singleDevices);	DisposeRoutineDescriptor((UniversalProcPtr)addDevice);	SetUpDevices();	KeepSpinning();}/* * AddDevice(depth, deviceFlags, targetDevice, userData) * *     Purpose: DeviceLoop routine to add the current device to the linked list *      Inputs: depth = depth of the device *				deviceFlags = device flags for the device *				targetDevice = handle to the target device *				userData = a refCon we can use *     Returns: nothing * */pascal void AddDevice(short depth, short deviceFlags, GDHandle targetDevice, 	MonitorHandle *theMonitor){#if applec#pragma unused(depth, deviceFlags)#endif	if (targetDevice != (*gMainMonitor)->device) {		if ((**theMonitor)->next = NewMonitor(targetDevice))			*theMonitor = (**theMonitor)->next;	}}/* * NewMonitor() * *     Purpose: Allocates and initializes a new monitor record based on the given device *      Inputs: theDevice = handle to the new device *     Returns: a pointer to the new monitor record * */MonitorHandle NewMonitor(GDHandle theDevice){	MonitorHandle theMonitor;	Rect theRect;		if (theMonitor = (MonitorHandle)NewHandle(sizeof(MonitorItem))) {		if ((*theMonitor)->gray = NewRgn()) {			if ((*theMonitor)->screen = NewRgn()) {				(*theMonitor)->next = nil;				(*theMonitor)->device = theDevice;				(*theMonitor)->oldres = (*theDevice)->gdResPref;				(*theMonitor)->color = IsColorDevice(theDevice);				(*theMonitor)->depth = MonitorDepth(theMonitor);				(*theMonitor)->full = false;				theRect = (*theMonitor)->rect = (*theDevice)->gdRect;				RectRgn((*theMonitor)->screen, &theRect);				SectRgn((*theMonitor)->screen, GetGrayRgn(), (*theMonitor)->gray);				return theMonitor;			}			DisposeHandle((Handle)(*theMonitor)->gray);		}		DisposeHandle((Handle)theMonitor);	}	return nil;}	/* * MonitorChanged() * *     Purpose: Checks to see if any monitors have changed depth/greys *      Inputs: none *     Returns: a pointer to the changed monitor, or nil if everything is the same * */ MonitorHandle MonitorChanged(void){	MonitorHandle theMonitor;	Boolean color;	short depth;		for (theMonitor = gMainMonitor; theMonitor; theMonitor = (*theMonitor)->next) {		color = IsColorDevice((*theMonitor)->device);		depth = MonitorDepth(theMonitor);		if (((*theMonitor)->color != color) || ((*theMonitor)->depth != depth)) {			(*theMonitor)->color = color;			(*theMonitor)->depth = depth;			return theMonitor;		}	}	return nil;}/* * SetUpDevices() * *     Purpose: Restores the new inverse table resolutions on the various devices *      Inputs: none *     Returns: nothing * */void SetUpDevices(void) {	MonitorHandle theMonitor;	for (theMonitor = gMainMonitor; theMonitor; theMonitor = (*theMonitor)->next) {		/*(*(*theMonitor)->device)->gdResPref = 5;		if ((*(*theMonitor)->device)->gdITable)			(*(*(*theMonitor)->device)->gdITable)->iTabSeed = GetCTSeed();		GDeviceChanged((*theMonitor)->device);*/	}}/* * ResetDevices() * *     Purpose: Restores the original inverse table resolutions on the various devices *      Inputs: none *     Returns: nothing * */void ResetDevices(void) {	MonitorHandle theMonitor;		if (!gMenuVisible && !(*gMainMonitor)->full) UnhideMenuBar();	for (theMonitor = gMainMonitor; theMonitor; theMonitor = (*theMonitor)->next)		if ((*theMonitor)->full) NormalScreen(theMonitor);}/* * ResetColors() * *	   Purpose: Restores the colors of all active screens to their original CLUTs *	    Inputs: none *	   Returns: nothing * */ void ResetColors(void){	MonitorHandle theMonitor;	RgnHandle theRgn;		if (gInBackground) return;	if (theRgn = NewRgn()) {		for (theMonitor = gMainMonitor; theMonitor; theMonitor = (*theMonitor)->next) {			if (!(*(*theMonitor)->device)->gdType) {				RestoreDeviceClut((*theMonitor)->device);				UnionRgn(theRgn, (*theMonitor)->gray, theRgn);			}		}		if (!EmptyRgn(theRgn)) {			LMSetPaintWhite(true);			PaintBehind((WindowPeek)GetFirstWindow(), theRgn);			LMSetPaintWhite(false);			PaintOne(nil, theRgn);			LMSetPaintWhite(true);		}		DisposeRgn(theRgn);	}}/* * FullScreen(theMonitor) * *	   Purpose: Fixes up theMonitor to allow full-screen windows *	    Inputs: theMonitor = pointer to the monitor we're interested in, or nil for all *	   Returns: nothing * */ void FullScreen(MonitorHandle theMonitor){	RgnHandle updateRgn;		if (!theMonitor) {		for (theMonitor = gMainMonitor; theMonitor; theMonitor = (*theMonitor)->next)			FullScreen(theMonitor);		return;	}	if ((*theMonitor)->full) return;	UnionRgn(GetGrayRgn(), (*theMonitor)->screen, GetGrayRgn());	if (theMonitor == gMainMonitor && gInBackground && (updateRgn = GetMenuRgn()))		DiffRgn(GetGrayRgn(), updateRgn, GetGrayRgn());	if (updateRgn = NewRgn()) {		DiffRgn((*theMonitor)->screen, (*theMonitor)->gray, updateRgn);		PaintOne((WindowPeek)FrontWindow(), updateRgn);		PaintBehind((WindowPeek)FrontWindow(), updateRgn);		CalcVis((WindowPeek)FrontWindow());		CalcVisBehind((WindowPeek)FrontWindow(), updateRgn);		DisposeRgn(updateRgn);		if (theMonitor == gMainMonitor && !gInBackground) {			LMSetMBarHeight(0);			gMenuVisible = false;			RestoreMenuArea();		}		(*theMonitor)->full = true;	}}/* * NormalScreen(theMonitor) * *	   Purpose: Fixes up theMonitor to remove the effects of FullScreen *	    Inputs: theMonitor = pointer to the monitor we're interested in, or nil for all *	   Returns: nothing * */ void NormalScreen(MonitorHandle theMonitor){	Boolean oldMenuVis = gMenuVisible;	RgnHandle updateRgn;	GrafPort tempPort;		if (!theMonitor) {		for (theMonitor = gMainMonitor; theMonitor; theMonitor = (*theMonitor)->next)			NormalScreen(theMonitor);		return;	}	if (!(*theMonitor)->full) return;	if (updateRgn = NewRgn()) {		if (theMonitor == gMainMonitor) SaveMenuArea();		DiffRgn((*theMonitor)->screen, (*theMonitor)->gray, updateRgn);		PushPort();		OpenPort(&tempPort);		CopyRgn(GetGrayRgn(), tempPort.visRgn);		MySetPort((CGrafPtr)&tempPort);		RGBForeColor(&gBlack);		PaintRgn(updateRgn);		ClosePort(&tempPort);		PopPort();		DiffRgn(GetGrayRgn(), updateRgn, GetGrayRgn());		CalcVis((WindowPeek)FrontWindow());		CalcVisBehind((WindowPeek)FrontWindow(), updateRgn);		DisposeRgn(updateRgn);		if (theMonitor == gMainMonitor) {			LMSetMBarHeight(gMenuHeight);			gMenuVisible = true;			AdjustMenus();			DrawMenuBar();			NudgeWindoids();		}	}	(*theMonitor)->full = false;}/* * NudgeWindoids() * *	   Purpose: Check all windoids to be sure they're not under the menubar *	    Inputs: none *	   Returns: nothing * */ void NudgeWindoids(void){	WindowPtr theWindow = GetFirstWindow();	Rect theRect, menuRect, tempRect;		menuRect = (*gMainMonitor)->rect;	menuRect.bottom = menuRect.top + GetMBarHeight();	while (theWindow) {		if (WindowKind((WindowPtr)theWindow) == floatingWindowKind) {			theRect = theWindow->portRect;			theRect.top -= gTitleBarHeight * 2 / 3;			GlobalRect(&theRect, theWindow);			if (SectRect(&theRect, &menuRect, &tempRect)) {				theRect.top = GetMBarHeight() + gTitleBarHeight * 2 / 3;				MoveWindow(theWindow, theRect.left, theRect.top, false);			}		}		theWindow = NextWindow(theWindow);	}}/* * UnhideMenuBar() * *	   Purpose: Displays the menubar on the main screen, if it's currently full-screen *	    Inputs: none *	   Returns: nothing * */ void UnhideMenuBar(void){	Boolean oldMenuVis = gMenuVisible;	RgnHandle menuRgn = GetMenuRgn();		if (!(*gMainMonitor)->full || gMenuVisible) return;	SaveMenuArea();	DiffRgn(GetGrayRgn(), menuRgn, GetGrayRgn());	CalcVis((WindowPeek)FrontWindow());	CalcVisBehind((WindowPeek)FrontWindow(), menuRgn);	LMSetMBarHeight(gMenuHeight);	gMenuVisible = true;	if (oldMenuVis) MyInvalMenuBar();	else {		AdjustMenus();		DrawMenuBar();	}}/* * RehideMenuBar() * *	   Purpose: Removes the menu bar from the main screen, if it's currently full-screen *	    Inputs: none *	   Returns: nothing * */ void RehideMenuBar(void){	RgnHandle menuRgn = GetMenuRgn();		if (!(*gMainMonitor)->full || !gMenuVisible || gInBackground) return;	UnionRgn(GetGrayRgn(), menuRgn, GetGrayRgn());	PaintOne((WindowPeek)FrontWindow(), menuRgn);	PaintBehind((WindowPeek)FrontWindow(), menuRgn);	CalcVis((WindowPeek)FrontWindow());	CalcVisBehind((WindowPeek)FrontWindow(), menuRgn);	LMSetMBarHeight(0);	gMenuVisible = false;	RestoreMenuArea();}/* * FrontWindowRgn(theRgn) * *	   Purpose: Finds the frontmost window that lies unobscured in theRgn *	    Inputs: theRgn = the region of interest, in global coordinates *	   Returns: a pointer to the window found, or nil if none qualify * */WindowPtr FrontWindowRgn(RgnHandle theRgn){	WindowPtr theWindow = FWFrontWindow();	WindowPtr theResult = nil;	RgnHandle testRgn;		if (testRgn = NewRgn()) {		for ( ; theWindow; theWindow = NextWindow(theWindow)) {			CopyRgn(theWindow->visRgn, testRgn);			GlobalRgn(testRgn, theWindow);			SectRgn(theRgn, testRgn, testRgn);			if (EqualRgn(theRgn, testRgn)) {				theResult = theWindow;				break;			} else if (!EmptyRgn(testRgn)) break;		}		DisposeRgn(testRgn);	}	return theResult;} /* * GetMenuRgn() * *     Purpose: Calculates the region occupied by the menubar *      Inputs: none *     Returns: a handle to the region of the menu bar * */RgnHandle GetMenuRgn(void){	static RgnHandle gMenuRgn = nil;	if (!gMenuRgn) {		gMenuRgn = NewRgn();		gMenuHeight = GetMBarHeight();	}	if (gMenuRgn) SetRectRgn(gMenuRgn, (*gMainMonitor)->rect.left, (*gMainMonitor)->rect.top,							(*gMainMonitor)->rect.right, (*gMainMonitor)->rect.top + gMenuHeight);	return gMenuRgn;}/* * GetActiveRect(theMonitor, theRect) * *	   Purpose: Determines the active area of the monitor, with or without menu bar as *					appropriate *	    Inputs: theMonitor = a handle to the monitor record *				theRect = a pointer to the destination rectangle *	   Returns: nothing * */ void GetActiveRect(MonitorHandle theMonitor, Rect *theRect){	if ((theMonitor != gMainMonitor) || !gMenuVisible) *theRect = (*theMonitor)->rect;	else *theRect = (*(*theMonitor)->gray)->rgnBBox;}/* * GetMinMaxMonitor(theImage) * *	   Purpose: Calculates the shallowest and deepest monitors intersecting the given *					image's window *	    Inputs: theImage = a pointer to the image *	   Returns: nothing * */ void GetMinMaxMonitor(ImageHandle theImage){	MonitorHandle theMonitor, deepMon = nil, shallowMon = nil;	ushort pass, depth, shallowest = 100, deepest = 0;	Rect winRect, tmpRect;	if (!WindowVisible((*theImage)->window)) {		if (!(*theImage)->dmon) (*theImage)->dmon = gMainMonitor;		if (!(*theImage)->smon) (*theImage)->smon = gMainMonitor;		return;	}	for (pass = 1; pass <= 2; pass++) {		winRect = (pass == 1) ? (*GetContRgn((*theImage)->window))->rgnBBox :								(*GetStrucRgn((*theImage)->window))->rgnBBox;		InsetRect(&winRect, 1, 1);		for (theMonitor = gMainMonitor; theMonitor; theMonitor = (*theMonitor)->next) {			if (SectRect(&winRect, &(*theMonitor)->rect, &tmpRect)) {				depth = (*theMonitor)->depth;				if (depth > deepest) {					deepest = depth;					deepMon = theMonitor;				}				if (depth < shallowest) {					shallowest = depth;					shallowMon = theMonitor;				}			}		}		if (deepMon && shallowMon) break;	}	if (deepMon && shallowMon) {		(*theImage)->dmon = deepMon;		(*theImage)->smon = shallowMon;	} else if (deepMon) (*theImage)->dmon = (*theImage)->smon = deepMon;	else if (shallowMon) (*theImage)->dmon = (*theImage)->smon = shallowMon;	else (*theImage)->dmon = (*theImage)->smon = gMainMonitor;}/* * GetDeepMonitor(pickColor) * *	   Purpose: Returns a pointer to the current deepest monitor *	    Inputs: pickColor = flag: true to pick color monitors first *	   Returns: a MonitorHandle pointing to the deepest monitor * */ MonitorHandle GetDeepMonitor(Boolean pickColor){	ushort depth, deepest = 0, color = pickColor;	MonitorHandle theMonitor, deepMon = nil;		for (theMonitor = gMainMonitor; theMonitor; theMonitor = (*theMonitor)->next) {		depth = (*theMonitor)->depth;		if ((depth > deepest) || 			((depth == deepest) && ((pickColor && !color) || (!pickColor && color)))) {			deepest = depth;			deepMon = theMonitor;			color = (*theMonitor)->color;		}	}	return deepMon;}/* * GetBigMonitor(pickColor) * *	   Purpose: Returns a handle to the current biggest monitor *	    Inputs: pickColor = flag: true to pick color monitors first *	   Returns: a MonitorHandle pointing to the biggest monitor * */ MonitorHandle GetBigMonitor(Boolean pickColor){	ulong biggest = 0, size, color = pickColor;	MonitorHandle theMonitor, bigMon = nil;		for (theMonitor = gMainMonitor; theMonitor; theMonitor = (*theMonitor)->next) {		size = (long)Height(&(*theMonitor)->rect) * (long)Width(&(*theMonitor)->rect);		if ((size > biggest) || 			((size == biggest) && ((pickColor && !color) || (!pickColor && color)))) {			biggest = size;			bigMon = theMonitor;			color = (*theMonitor)->color;		}	}	return bigMon;}/* * GetImageMonitor(theImage) * *     Purpose: Returns a pointer to the deepest monitor intersected by the image window *      Inputs: theImage = pointer to the image we're interested in *     Returns: a MonitorHandle of the deepest intersecting monitor * */ MonitorHandle GetImageMonitor(ImageHandle theImage) {	MonitorHandle theMonitor, deepMon = nil;	ushort depth, deepest = 0;	Rect theRect, tmpRect;		if (!theImage || !(*theImage)->window) return nil;	theRect = (*theImage)->wrect;	GlobalRect(&theRect, (*theImage)->window);	for (theMonitor = gMainMonitor; theMonitor; theMonitor = (*theMonitor)->next) {		if (SectRect(&theRect, &(*theMonitor)->rect, &tmpRect)) {			depth = (*theMonitor)->depth;			if (depth > deepest) {				deepest = depth;				deepMon = theMonitor;			}		}	}	return deepMon;}/* * GetMostDevice(theRect) * *     Purpose: Returns a pointer to the monitor containing the most area of the window *      Inputs: theRect = pointer to a global rectangle *     Returns: a MonitorHandle of the relevant intersecting monitor * */ MonitorHandle GetMostDevice(Rect *theRect) {	MonitorHandle theMonitor, mostMon = gMainMonitor;	long area, largest = 0;	Rect tmpRect;	for (theMonitor = gMainMonitor; theMonitor; theMonitor = (*theMonitor)->next) {		SectRect(&(*theMonitor)->rect, theRect, &tmpRect);		area = (long)Width(&tmpRect) * (long)Height(&tmpRect);		if (area > largest) {			largest = area;			mostMon = theMonitor;		}	}	return mostMon;}
/*********************************************************//* This source code copyright (c) 1991-2001, Aaron Giles *//* See the Read Me file for licensing information.       *//* Contact email: mac@aarongiles.com                     *//*********************************************************/#ifndef __GENERALMENUS__#define __GENERALMENUS__void MenuBarInit(void);extern long MakeMenuSelection(short menu, short item);extern void HandleMenuChoice(long theChoice);void SaveMenuArea(void);void RestoreMenuArea(void);void DisableMenus(void);void EnableMenus(void);void AdjustMenus(void);void AdjustFile(short var);void AdjustEdit(short var);Boolean CanSelectScreen(void);void AdjustView(short var);void AdjustColor(short var);void AdjustWindow(short var);void AddWindowItem(ImageHandle theImage);void DeleteWindowItem(ImageHandle theImage);#define MenuEnable(menu, item, enabled) \	if (enabled) EnableItem(menu, item);\	else DisableItem(menu, item)#define SetMBarHeight(ht) (*(short *)MBarHeight = (ht))#endif
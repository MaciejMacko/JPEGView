/*********************************************************//* This source code copyright (c) 1991-2001, Aaron Giles *//* See the Read Me file for licensing information.       *//* Contact email: mac@aarongiles.com                     *//*********************************************************/#ifndef __STATISTICSWINDOW__#define __STATISTICSWINDOW__OSErr OpenStats(void);void CloseStats(void);void SizeStats(Boolean zoomed);void ZoomStats(void);WindowPtr GetStatWindow(void);extern void HandleStatClick(EventRecord *theEvent);void SetStatistics(void);void DrawStatWindow(void);void DrawStatSummary(ImageHandle theImage, short *vPos);void DrawStatFileName(ImageHandle theImage, short *vPos);void DrawStatImageSize(ImageHandle theImage, short *vPos);void DrawStatImageStatus(ImageHandle theImage, short *vPos);void DrawStatImageColors(ImageHandle theImage, short *vPos);void DrawStatCompression(ImageHandle theImage, short *vPos);void DrawStatFileFormat(ImageHandle theImage, short *vPos);void DrawStatImageLength(ImageHandle theImage, short *vPos);void DrawStatDisplayedSize(ImageHandle theImage, short *vPos);void DrawStatDisplayedColors(ImageHandle theImage, short *vPos);void DrawStatDisplayQuality(ImageHandle theImage, short *vPos);void DrawStatDisplayTime(ImageHandle theImage, short *vPos);void DrawStatOffscreen(ImageHandle theImage, short *vPos);void DrawStatFree(ImageHandle theImage, short *vPos);void DrawStatItem(short vPos, short item, StringPtr theNum);extern void IdleCredits(void);void DoStatsHelp(Point globalPt);#endif
/*********************************************************//* This source code copyright (c) 1991-2001, Aaron Giles *//* See the Read Me file for licensing information.       *//* Contact email: mac@aarongiles.com                     *//*********************************************************/#ifndef __GIF__#define __GIF__extern Boolean idGIF(uchar *theData, long theSize, short refNum, FSSpec *theSpec);extern OSErr OpenGIF(ImageHandle theImage);extern OSErr DrawGIF(Handle theHandle, JVDrawParamsHandle theParams);#endif
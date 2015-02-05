/*********************************************************//* This source code copyright (c) 1991-2001, Aaron Giles *//* See the Read Me file for licensing information.       *//* Contact email: mac@aarongiles.com                     *//*********************************************************/// void DitherCopyTo8Even(ulong *srcRow, void *dstRowStart, uchar *iTable, ColorSpec *cTable,//						  DitherErrorsPtr eBuffer, DitherErrorsPtr oBuffer,//						  short *rgnBuffer, uchar *errorMap, short columns)// internals:	ulong *src = srcRow;					// pointer to the 32-bit source row	uchar *dst = (uchar *)dstRowStart;		// pointer to the 8-bit destination row	ulong pixel;							// scratch for the current pixel	long red;								// red scratch area	long green;								// green scratch area	long blue;								// blue scratch area	long red2;								// 2 * the red error; used for fast calculations	long green2;							// 2 * the green error	long blue2;								// 2 * the blue error	long rgnCol;							// region column counter	// initialize our counters and clear out next row	red = green = blue = 0;	oBuffer->red = oBuffer->green = oBuffer->blue = 0;	// set up the region column counter	if (!(rgnCol = *rgnBuffer++)) rgnCol -= *rgnBuffer++;	// begin main loop here	while (columns--) {			// combine the accumulated errors and round to a pixel value		red += eBuffer->red + 8;		red >>= 4;		red = errorMap[red];		green += eBuffer->green + 8;		green >>= 4;		green = errorMap[green];		blue += eBuffer->blue + 8;		blue >>= 4;		blue = errorMap[blue];		eBuffer++;				// offset the current pixel by the errors		pixel = *src++;		blue += pixel & 0xff;		pixel >>= 8;		green += pixel & 0xff;		pixel >>= 8;		red += pixel & 0xff;				// check for overflows and handle them		if (red > 255) red = 255;		else if (red < 0) red = 0;		if (blue > 255) blue = 255;		else if (blue < 0) blue = 0;		if (green > 255) green = 255;		else if (green < 0) green = 0;		// get the inverse color table entry		pixel = red >> 3;		pixel <<= 5;		pixel += green >> 3;		pixel <<= 5;		pixel += blue >> 3;		pixel = iTable[pixel];		// determine whether or not to draw this pixel, and handle switches		if (rgnCol > 0) {			dst++;			if (!--rgnCol) rgnCol = rgnBuffer[-1] - rgnBuffer[0], rgnBuffer++;		} else {			*dst++ = pixel;			if (!++rgnCol) rgnCol = rgnBuffer[0] - rgnBuffer[-1], rgnBuffer++;		}		// calculate the new errors		red -= cTable[pixel].rgb.red >> 8;		green -= cTable[pixel].rgb.green >> 8;		blue -= cTable[pixel].rgb.blue >> 8;		// calculate 2 * the new error for fast ops ahead		red2 = red << 1;		green2 = green << 1;		blue2 = blue << 1;		// store 1/16, 3/16, and 5/16 into next row; leave 7/16 for neighbor		oBuffer[1].red = red;		oBuffer[1].green = green;		oBuffer[1].blue = blue;		oBuffer[-1].red += (red += red2);		oBuffer[-1].green += (green += green2);		oBuffer[-1].blue += (blue += blue2);		oBuffer->red += (red += red2);		oBuffer->green += (green += green2);		oBuffer->blue += (blue += blue2);		red += red2;		green += green2;		blue += blue2;		oBuffer++;		// end of loop: go until done	}
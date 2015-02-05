/*********************************************************//* This source code copyright (c) 1991-2001, Aaron Giles *//* See the Read Me file for licensing information.       *//* Contact email: mac@aarongiles.com                     *//*********************************************************/// void CopyScaledFrom32(CopyDataPtr theData);#define kThreshold (1L << 9)#define AddHalfPixel(source) \	pixel = (source),\	blue += pixel & 0xff,\	pixel >>= 8,\	green += pixel & 0xff,\	pixel >>= 8,\	red += pixel & 0xff,\	divisor++#define AddFullPixel(source) \	pixel = (source),\	blue += (pixel & 0xff) << 1,\	pixel >>= 8,\	green += (pixel & 0xff) << 1,\	pixel >>= 8,\	red += (pixel & 0xff) << 1,\	divisor += 2	DitherErrors evenDithering[kStackRowWidth + 2], 	// stack-based buffers for dithering; if these				 oddDithering[kStackRowWidth + 2];	// 		aren't enough, we allocate bigger ones	ulong scaledRow[kStackRowWidth + 2];				// stack-based buffer for scaling{	DitherErrorsPtr eBuffer,			// pointers to whichever dithering					oBuffer;			// 		buffers we decide to use	ulong *scaled,				 		// pointer to the start of the scaling buffer		  *srcScaled, 					// temp. source pointer used during scaling		  *dstScaled;					// destination pointer used during scaling	Ptr src = theData->srcBase, 		// source pointer into original image data		dst = theData->dstBase;			// dest. pointer into real destination	ulong srcPixelContributionY, 		// contribution in Y made by each source pixel		  srcPixelContributionX, 		// contribution in X made by each source pixel		  srcRemainingY = theData->yRemainder,// remaining Y contribution for source		  srcRemainingX;				// remaining X contribution for current src	ulong dstNeededY, 					// amount needed for complete dest. Y pixel		  dstNeededX,					// amount needed for complete dest. X pixel		  red, green, blue, pixel,		// temp. counter for rgb and holder for pixel		  divisor;						// holder for the divisor	uchar *table;						// pointer to divisor table	short *rgnPtr; 						// pointer to the region data	long row = theData->height, 		// row counter		  col;							// column counter	Handle dynamicBuffer;				// handle to any allocation we make	RgnBuffer rgnData;					// buffer for the region data	// Allocate space for our buffers, if not enough room on stack	// NOTE: This only works if the machine is in 32-bit addressing mode!!!	if (theData->width > kStackRowWidth) {		dynamicBuffer = AnyNewHandle((theData->width + 2) * (2 * sizeof(DitherErrors) + sizeof(ulong)));		if (!dynamicBuffer) return;		HLock(dynamicBuffer);		eBuffer = (DitherErrorsPtr)*dynamicBuffer + 1;		oBuffer = eBuffer + theData->width + 2;		scaled = (ulong *)(oBuffer + theData->width + 1) + 1;	} else {		eBuffer = &evenDithering[1];		oBuffer = &oddDithering[1];	}	scaled = &scaledRow[1];		// Clear out the dithering and scaling buffers	if (theData->evodd & 1) 		ClearMem((Ptr)&eBuffer[-1], (theData->width + 2) * sizeof(DitherErrors));	else ClearMem((Ptr)&oBuffer[-1], (theData->width + 2) * sizeof(DitherErrors));		// Initialize the region data	rgnData.zero = 0;	rgnPtr = InitRegion(*theData->theRgn, &rgnData, &theData->boxRect);		// Set up the source/destination quantities	srcPixelContributionY = (theData->dstHeight << 10) / theData->srcHeight + 1;	srcPixelContributionX = (theData->dstWidth << 10) / theData->srcWidth + 1;	// The outermost (row) loop begins here; set up our pointers into the data	while (row--) {		srcScaled = (ulong *)src;		dstScaled = (ulong *)scaled;			// reset the columns counter and reset the source X remainder		col = theData->width;		srcRemainingX = theData->xRemainder;		// The inner (column) loop begins here; set up our counters in the dest system		while (col--) {			dstNeededX = dstNeededY = 1L << 10;			red = green = blue = divisor = 0;						// Determine what sort of scaling is best for us			if (dstNeededY < srcRemainingY && dstNeededX < srcRemainingX) {								// no scaling needed for this particular destination pixel				*dstScaled++ = *srcScaled;				goto dontStore;			} else if (dstNeededY < srcRemainingY) {				// scaling needed in the X direction only				if (srcRemainingX >= kThreshold) AddHalfPixel(*srcScaled);				srcScaled++;				dstNeededX -= srcRemainingX;			ScaleXLoop:				if (dstNeededX > srcPixelContributionX) {					AddFullPixel(*srcScaled++);					dstNeededX -= srcPixelContributionX;					goto ScaleXLoop;				}								if (dstNeededX >= kThreshold) AddHalfPixel(*srcScaled);				srcRemainingX = srcPixelContributionX;			} else if (dstNeededX < srcRemainingX) {				// scaling needed in the Y direction only				ulong origSrcRemainingY = srcRemainingY;				ulong *srcStart = srcScaled;				if (srcRemainingY >= kThreshold) AddHalfPixel(*srcScaled);				srcScaled = (ulong *)((uchar *)srcScaled + theData->srcRow);				dstNeededY -= srcRemainingY;							ScaleYLoop:				if (dstNeededY > srcPixelContributionY) {					AddFullPixel(*srcScaled);					srcScaled = (ulong *)((uchar *)srcScaled + theData->srcRow);					dstNeededY -= srcPixelContributionY;					goto ScaleYLoop;				}								if (dstNeededY >= kThreshold) AddHalfPixel(*srcScaled);				srcScaled = srcStart;				srcRemainingY = origSrcRemainingY;			} else {				// scaling needed in both directions				ulong origSrcRemainingX = srcRemainingX;				ulong origSrcRemainingY = srcRemainingY;				ulong *srcStart = srcScaled, *srcRowStart = srcScaled;								if (srcRemainingY >= kThreshold) {					if (srcRemainingX >= kThreshold) AddHalfPixel(*srcScaled);					srcScaled++;					dstNeededX -= srcRemainingX;										ScaleXYLoop1:					if (dstNeededX > srcPixelContributionX) {						AddFullPixel(*srcScaled++);						dstNeededX -= srcPixelContributionX;						goto ScaleXYLoop1;					}					if (dstNeededX >= kThreshold) AddHalfPixel(*srcScaled);					srcRemainingX = origSrcRemainingX;					dstNeededX = 1L << 10;				}				srcScaled = (ulong *)((uchar *)srcRowStart + theData->srcRow);				dstNeededY -= srcRemainingY;			ScaleXYLoop2:				if (dstNeededY > srcPixelContributionY) {					srcRowStart = srcScaled;										if (srcRemainingX >= kThreshold) AddHalfPixel(*srcScaled);					srcScaled++;					dstNeededX -= srcRemainingX;								ScaleXYLoop3:					if (dstNeededX > srcPixelContributionX) {						AddFullPixel(*srcScaled++);						dstNeededX -= srcPixelContributionX;						goto ScaleXYLoop3;					}					if (dstNeededX >= kThreshold) AddHalfPixel(*srcScaled);					srcScaled = (ulong *)((uchar *)srcRowStart + theData->srcRow);					dstNeededY -= srcPixelContributionY;					srcRemainingX = origSrcRemainingX;					dstNeededX = 1L << 10;					goto ScaleXYLoop2;				}								if (dstNeededY >= kThreshold) {									if (srcRemainingX >= kThreshold) AddHalfPixel(*srcScaled);					srcScaled++;					srcStart++;					dstNeededX -= srcRemainingX;										ScaleXYLoop4:					if (dstNeededX > srcPixelContributionX) {						AddFullPixel(*srcScaled++);						srcStart++;						dstNeededX -= srcPixelContributionX;						goto ScaleXYLoop4;					}					if (dstNeededX >= kThreshold) AddHalfPixel(*srcScaled);								} else {					srcStart++;					dstNeededX -= srcRemainingX;				ScaleXYLoop5:					if (dstNeededX > srcPixelContributionX) {						srcStart++;						dstNeededX -= srcPixelContributionX;						goto ScaleXYLoop5;					}				}				srcRemainingX = srcPixelContributionX;				srcScaled = srcStart;				srcRemainingY = origSrcRemainingY;			}						switch (divisor) {				case 0:					*dstScaled++ = *srcScaled;					break;				case 1:					pixel = red;					pixel <<= 8;					pixel += green;					pixel <<= 8;					pixel += blue;					*dstScaled++ = pixel;					break;				case 2:					pixel = red >> 1;					pixel <<= 8;					pixel += green >> 1;					pixel <<= 8;					pixel += blue >> 1;					*dstScaled++ = pixel;					break;				case 3:				case 4:				case 5:				case 6:				case 7:					table = (uchar *)(theData->sTable[divisor - 3]);					pixel = table[red];					pixel <<= 8;					pixel += table[green];					pixel <<= 8;					pixel += table[blue];					*dstScaled++ = pixel;					break;				default:					pixel = red / divisor;					pixel <<= 8;					pixel += green / divisor;					pixel <<= 8;					pixel += blue / divisor;					*dstScaled++ = pixel;					break;			}			// apply the final adjustment to the X counters		dontStore:			if (!(srcRemainingX -= dstNeededX)) {				srcScaled++;				srcRemainingX = srcPixelContributionX;			}		}				// We now have a row; set up to do a dithered copy from it		if ((++theData->evodd) & 1)			theData->odd(scaled, dst, theData->itAddr, theData->ctAddr, eBuffer, oBuffer,						 rgnData.buffer, theData->errTable, theData->width);		else theData->even(scaled, dst, theData->itAddr, theData->ctAddr, eBuffer, oBuffer,						 rgnData.buffer, theData->errTable, theData->width);		if (!row) break;		if (!--rgnData.yCount) rgnPtr = UpdateRegion(rgnPtr, &rgnData, &theData->boxRect);		// apply final adjustments to the Y counters		dstNeededY = 1L << 10;		while (dstNeededY >= srcRemainingY) {			src += theData->srcRow;			dstNeededY -= srcRemainingY;			srcRemainingY = srcPixelContributionY;		}		srcRemainingY -= dstNeededY;		dst += theData->dstRow;	}		// Return space allocated for buffers	if (theData->width > kStackRowWidth) DisposeHandle(dynamicBuffer);}
/*********************************************************//* This source code copyright (c) 1991-2001, Aaron Giles *//* See the Read Me file for licensing information.       *//* Contact email: mac@aarongiles.com                     *//*********************************************************/// void DecodeRow(GIFDataPtr theData)	ushort *parent = (ushort *)theData->tree;	uchar *code = (uchar *)(parent + 4096);	short i, j, currentCode, oldCode;	if (!theData->dest) { 		theData->charSize = *theData->src++; 		gCodeSize = theData->charSize + 1; 		gStack = theData->stack;		gBytesLeft = gBitsInBuffer = gBitBuffer = 0; 		gTreeSize = (1 << theData->charSize);		gClearCode = gTreeSize++; 		gEndCode = gTreeSize++; 		gInput = theData->src;		theData->theReturn = true;		return;	}	for (i = 0; i < theData->width; i++) {		if (gStack != theData->stack) { 			*theData->dest++ = *(--gStack); 			continue; 		}		while (gCodeSize > gBitsInBuffer) {			if (!gBytesLeft) { 				gBytesLeft = *gInput++; 				if (!gBytesLeft) {					theData->theReturn = true;					return;				}			}			gBytesLeft--; 			gBitBuffer |= (ulong)*gInput++ << gBitsInBuffer; 			gBitsInBuffer += 8;		}		currentCode = gBitBuffer & ((1 << gCodeSize) - 1); 		gBitBuffer >>= gCodeSize; 		gBitsInBuffer -= gCodeSize;		if (currentCode == gEndCode) {			theData->theReturn = true;			return;		} else if (currentCode == gClearCode) {			gCodeSize = theData->charSize + 1; 			gTreeSize = gEndCode + 1; 			do {				while (gCodeSize > gBitsInBuffer) {					if (!gBytesLeft) { 						gBytesLeft = *gInput++; 						if (!gBytesLeft) {							theData->theReturn = true;							return;						}					}					gBytesLeft--; 					gBitBuffer |= (ulong)*gInput++ << gBitsInBuffer; 					gBitsInBuffer += 8;				}				currentCode = gBitBuffer & ((1 << gCodeSize) - 1); 				gBitBuffer >>= gCodeSize; 				gBitsInBuffer -= gCodeSize;			} while (currentCode == gClearCode);			*theData->dest++ = gFirstCode = gLastCode = currentCode; 			continue;		}		oldCode = currentCode;		if (currentCode > gTreeSize) {			theData->end = nil;			break;		} else if (currentCode == gTreeSize) { 			*gStack++ = gFirstCode; 			currentCode = gLastCode; 		}		for (j = currentCode; j >= gClearCode; j = parent[j]) *gStack++ = code[j];		*theData->dest++ = gFirstCode = j;		if (gTreeSize != 4096) {				code[gTreeSize] = gFirstCode; 			parent[gTreeSize] = gLastCode;			if ((++gTreeSize >= (1 << gCodeSize)) && (gCodeSize != 12)) gCodeSize++; 		}		gLastCode = oldCode;	}	theData->theReturn = false;	if (gInput > theData->end) theData->end = nil;
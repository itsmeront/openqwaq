/**
 * Project OpenQwaq
 *
 * Copyright (c) 2005-2011, Teleplace, Inc., All Rights Reserved
 *
 * Redistributions in source code form must reproduce the above
 * copyright and this condition.
 *
 * The contents of this file are subject to the GNU General Public
 * License, Version 2 (the "License"); you may not use this file
 * except in compliance with the License. A copy of the License is
 * available at http://www.opensource.org/licenses/gpl-2.0.php.
 *
 */

 /* NativeFontPlugin header file */

char *ioListFont(int fontIndex);
int ioCreateFont(int fontNameIndex, int fontNameLength, int pixelSize, int flags);
int ioDestroyFont(int fontIndex);
int ioFontWidthOfChar(int fontIndex, int characterIndex);
int ioFontFullWidthOfChar(int fontIndex, int characterIndex, int width[3]);
int ioFontNumKernPairs(int fontIndex);
int ioFontGetKernPair(int fontIndex, int kernIndex, int kernPair[3]);

int ioFontGlyphOfChar(int fontIndex, int characterIndex, 
		      int formBitsIndex, int formWidth, int formHeight, int formDepth);
int ioFontEncoding(int fontIndex);
int ioFontAscent(int fontIndex);
int ioFontDescent(int fontIndex);

int ioFontEmbeddingFlags(int fontIndex);
int ioGetFontDataSize(int fontIndex);
int ioGetFontData(int fontIndex, char *buffer, int bufSize);

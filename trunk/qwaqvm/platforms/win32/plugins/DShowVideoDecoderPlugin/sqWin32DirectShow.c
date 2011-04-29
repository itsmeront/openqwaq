/* sqWin32DirectShow.c

  Copyright (c) 2002, 2007 Yoshiki Ohshima
  All rights reserved.

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the 'Software'),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, provided that the above copyright notice(s) and this
  permission notice appear in all copies of the Software and that both the
  above copyright notice(s) and this permission notice appear in supporting
  documentation.

  THE SOFTWARE IS PROVIDED 'AS IS'.  USE ENTIRELY AT YOUR OWN RISK.

*/

#include "sq.h"
#include "DShowVideoDecoderPlugin.h"

int
dshowShutdown()
{
  return DestroyStillGraph();
}

static int
swapBuffer32FromTo(unsigned char *src, unsigned char *dst, int srcW, int srcH, int dstScanLine)
{
  unsigned char s0, s1, s2;

  int x;
  int y;

  unsigned char* pixel;

  int srcScanStep;
  int dstScanStep;
  
  srcScanStep = srcW*3;
  dstScanStep = dstScanLine * 4;

  for (y = 0; y < srcH; y++) {
    pixel = src + ((srcW * 3) * (srcH - y - 1));
    for (x = 0; x < srcW; x++) {
      /*      s0 = *(pixel+0);
      s1 = *(pixel+1);
      s2 = *(pixel+2);

      *(dst+0) = s0;
      *(dst+1) = s1;
      *(dst+2) = s2;
      *(dst+3) = 0;
      */

      /* the following line reads one out-of-bounds byte. (and tons of un-aligned word load.)
	 If you care, use the above lines. */
      *((unsigned int*)dst) = (*((unsigned int*)pixel)) | 0xFF000000;

      dst += 4;
      pixel += 3;
    }
    /* src += srcScanStep; */
    /* dst += dstScanStep; */
  }
  
  return true;
}

static int
swapBuffer16FromTo(unsigned char *src, unsigned char *dst, int srcW, int srcH, int dstScanLine)
{
  unsigned char s0, s1, s2, s3, s4, s5;

  int x;
  int y;

  unsigned short pix1;
  unsigned short pix2;
  
  unsigned char *pixel;

  int srcScanStep;
  int dstScanStep;

  srcScanStep = srcW*3;
  dstScanStep = dstScanLine * 2;

  for (y = 0; y < srcH; y++) {
    pixel = src + ((srcW * 3) * (srcH - y - 1));
    for (x = 0; x < srcW; x += 2) {
      s0 = *(pixel+0);
      s1 = *(pixel+1);
      s2 = *(pixel+2);
      s3 = *(pixel+3);
      s4 = *(pixel+4);
      s5 = *(pixel+5);

      pix1 = (((s5 >> 3) << 10) | ((s4 >> 3) << 5) | (s3 >> 3));
      pix2 = (((s2 >> 3) << 10) | ((s1 >> 3) << 5) | (s0 >> 3));
      *(unsigned int*)dst =
	(pix1 ? pix1 : 1) | (pix2 ? (pix2 << 16) : (1 << 16));

      dst += 4;
      pixel += 6;
    }
    /* src += srcScanStep; */
    /* dst += dstScanStep; */
  }
  
  return true;
}

int
convertToSqueakForm(unsigned char *bits, int width, int height, int depth)
{
  unsigned int size;
  int decodeW;
  int decodeH;
  unsigned char *frame;

  if (depth < 16) {
    return false;
  }

  frame = DecoderGetLastFrame();
  size = DecoderDecodeSize();
  decodeW = (size>>16)&0xFFFF;
  decodeH = size & 0xFFFF;

  if (width < decodeW) {
    return false;
  }

  if (height < decodeH) {
    return false;
  }

  if (depth == 16) {
    return swapBuffer16FromTo(frame, bits, decodeW, decodeH, width);
  } else if (depth == 32) {
    return swapBuffer32FromTo(frame, bits, decodeW, decodeH, width);
  }

  return false;
}
  

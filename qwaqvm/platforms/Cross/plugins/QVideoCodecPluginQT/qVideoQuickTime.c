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

/*
 * qVideoQuickTime.c
 * QVideoCodecPluginQT
 *
 * QuickTime-specific code that may be useful for both encoding
 * and decoding.
 */

#include "qVideoQuickTime.h"
#include "../QVideoCodecPlugin/qVideoCommon.h"

int qQuickTimeMajorVersion;
int qQuickTimeMinorVersion;
int qQuickTimeMinorMinorVersion;

void qPrintCodecNameList(void)
{
  CodecNameSpecListPtr list;
  CodecNameSpec *spec;
  OSErr err;
  int i;

  fprintf(QSTDERR, "\nAVAILABLE CODECS:");
  
  err = GetCodecNameList(&list, 1);
  if (err != noErr) {
    fprintf(QSTDERR, "\n\t(encountered error %d)", err);
    return;
  }

  spec = &(list->list[0]);
  for (i = 0; i < list->count; i++) {
    fprintf(QSTDERR, "\n\t%d: %s", i, spec->typeName);
    spec++;
  }

  // This seems to cause problems...
  // DisposeCodecNameList(list);
}


void qPrintFourCharCode(FourCharCode code)
{
  char *p = (char*)(&code);
  fprintf(QSTDERR, "'%c%c%c%c'", p[3], p[2], p[1], p[0]);  // works for little-endian.
}


void qPrintImageDescription(ImageDescriptionHandle hIm)
{
  fprintf(QSTDERR, "\n\twidth: %d", (*hIm)->width);
  fprintf(QSTDERR, "\n\theight: %d", (*hIm)->height);
  fprintf(QSTDERR, "\n\tcodec: "); qPrintFourCharCode((*hIm)->cType);
  fprintf(QSTDERR, "\n\tdepth: %d", (*hIm)->depth);
  fprintf(QSTDERR, "\n\t............");
  fprintf(QSTDERR, "\n\tidSize: %d", (*hIm)->idSize);
  fprintf(QSTDERR, "\n\tversion: %d", (*hIm)->version);
  fprintf(QSTDERR, "\n\trevision: %d", (*hIm)->revisionLevel);
  fprintf(QSTDERR, "\n\tvendor: %d", (*hIm)->vendor);
  fprintf(QSTDERR, "\n\ttemporal quality: %d", (*hIm)->temporalQuality);
  fprintf(QSTDERR, "\n\tspatial quality: %d", (*hIm)->spatialQuality);
  fprintf(QSTDERR, "\n\thRes: %f", FixedToFloat((*hIm)->hRes));
  fprintf(QSTDERR, "\n\tvRes: %f", FixedToFloat((*hIm)->vRes));
  fprintf(QSTDERR, "\n\tdataSize: %d", (*hIm)->dataSize);
  fprintf(QSTDERR, "\n\tframeCount: %d", (*hIm)->frameCount);
  fprintf(QSTDERR, "\n\tclutID: %d", (*hIm)->clutID);
}




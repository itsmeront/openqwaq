/**
 * Project OpenQwaq
 *
 * Copyright (c) 2005-20011, Teleplace, Inc., All Rights Reserved
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
 * qVideoQuickTime.h
 * QVideoCodecPluginQT
 *
 * QuickTime-specific code that may be useful for both encoding
 * and decoding.
 */

#ifndef __Q_VIDEO_QUICKTIME_H__
#define __Q_VIDEO_QUICKTIME_H__

#include "qVideoQuickTimePlatformSpecific.h"

extern int qQuickTimeMajorVersion;
extern int qQuickTimeMinorVersion;
extern int qQuickTimeMinorMinorVersion;

/* qPrintCodecNameList: Print names of available codes on stderr.
   Arguments: None
   ReturnValue: None
*/
void qPrintCodecNameList(void);


/*qImageDescription: Print contents of the specified ImageDescription.
  Arguments:
    hIm: handle to the image description.
*/
void qPrintImageDescription(ImageDescriptionHandle hIm);


/* qPrintFourCharCode: Print an Apple/QuickTime 'four-char code'
   Arguments: the code
   Return value: None.
*/
void qPrintFourCharCode(FourCharCode code);

#endif //#ifndef __Q_VIDEO_QUICKTIME_H__

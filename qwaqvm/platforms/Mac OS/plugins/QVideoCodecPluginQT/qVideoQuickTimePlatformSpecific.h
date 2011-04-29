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
 * qVideoQuickTimePlatformSpecific.h
 * QVideoCodecPluginQT
 *
 * QuickTime-specific code that may be useful for both encoding
 * and decoding.
 */

#ifndef __Q_VIDEO_QUICKTIME_PLATFORM_SPECIFIC_H__
#define __Q_VIDEO_QUICKTIME_PLATFORM_SPECIFIC_H__

#include <QuickTime/QuickTime.h>
#include <Carbon/Carbon.h>

#define QwaqSetRect SetRect

#ifdef __LITTLE_ENDIAN__
#define Q_PIXEL_FORMAT k32BGRAPixelFormat
#else
#define Q_PIXEL_FORMAT k32ARGBPixelFormat
#endif

#endif //#ifndef __Q_VIDEO_QUICKTIME_PLATFORM_SPECIFIC_H__
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

/******************************************************************************
 *
 * qDShowUtil.h
 * QWebcamPlugin (win32)
 *
 * A few convenient functions.
 *
 ******************************************************************************/

#ifndef __Q_DSHOW_UTIL_H__
#define __Q_DSHOW_UTIL_H__

#define INITGUID

// Google-revealed tricks to overcome missing dxtrans in older DirectX SDK
#pragma include_alias( "dxtrans.h", "qedit.h" )
#define __IDxtCompositor_INTERFACE_DEFINED__
#define __IDxtAlphaSetter_INTERFACE_DEFINED__
#define __IDxtJpeg_INTERFACE_DEFINED__
#define __IDxtKey_INTERFACE_DEFINED__
#include <qedit.h>

#include <windows.h>
#include <objbase.h>
#include <strmif.h>
#include <dshow.h>
#include <qedit.h>

namespace Qwaq
{

void WINAPI DeleteMediaType(__inout_opt AM_MEDIA_TYPE *pmt);
IPin *GetInPin(IBaseFilter * pFilter, int Num);
IPin *GetOutPin( IBaseFilter * pFilter, int Num );
HRESULT GetPin(IBaseFilter *pFilter, PIN_DIRECTION dirrequired, int iNum, IPin **ppPin);

IEnumMoniker *pqDeviceEnumerator();

} //namespace Qwaq

#endif //#ifndef __Q_DSHOW_UTIL_H__

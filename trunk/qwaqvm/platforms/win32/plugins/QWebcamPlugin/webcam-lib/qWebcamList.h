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

#ifndef __Q_WEBCAM_LIST_H__
#define __Q_WEBCAM_LIST_H__

#include "qWebcam.h"

// XXXXX: what's this for?
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

namespace Qwaq
{
// Enumeration of available devices.  These devIndexes range [ 0, qGetNumCamDevices()-1 ]
int   qPrivateGetNumCamDevices(void);
char* qPrivateGetCamDeviceName(int devIndex);
char* qPrivateGetCamDeviceUID(int devIndex);
int   qPrivateGetCamDeviceIsBusy(int devIndex);         // Boolean 0 or 1
void  qPrivateStopLists(void);
}

#endif __Q_WEBCAM_LIST_H__

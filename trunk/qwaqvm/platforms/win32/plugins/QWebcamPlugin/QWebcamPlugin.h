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

/******************************************************************************
 *
 * QWebcamPlugin.h
 * QWebcamPlugin (win32)
 *
 * Declares the functions that are directly used by the generated code in
 * QWebcamPlugin.c.  The definitions of these functions are found in qGlue.h
 *
 ******************************************************************************/


#ifndef __Q_WEBCAM_PLUGIN_H__
#define __Q_WEBCAM_PLUGIN_H__

/* Squeak pluggable primitive support. */
#if defined(_MSC_VER) || defined(__MINGW32__)
#  undef EXPORT
#  define EXPORT(returnType) __declspec( dllexport ) returnType __cdecl 
#endif 

#include "qFeedbackChannel-interface.h"
#include "sqMemoryAccess.h"

#ifdef __cplusplus
extern "C" {
#endif


void qInitModule();
void qShutdownModule();

int qCreateCamera(char* args, int argsSize, char* feedbackChannelHandle);
int qCreateCameraByUID(char* uid, char* args, int argsSize, char* feedbackChannelHandle);
int qDestroyCamera(int cameraIndex);
int qCameraRun(int cameraIndex);
int qCameraPause(int cameraIndex);
int qCameraStop(int cameraIndex);
void qCameraAdjust(int cameraIndex, char* args, int argsSize);
int qCameraIsValid(int cameraIndex);
int qCameraGetParams(int cameraIndex, char* externalAddress);
char* qCameraName(int cameraIndex);
char* qCameraUID(int cameraIndex);
void  qCopyBufferToBitmap(char *bitmapData, void *bufferAddr, sqInt bufSize);

// Enumeration of available devices.  These devIndexes range [ 0, qGetNumDevices()-1 ]
int   qGetNumCamDevices();
char* qGetCamDeviceName(int devIndex);
char* qGetCamDeviceUID(int devIndex);
int   qGetCamDeviceIsBusy(int devIndex);         // Boolean 0 or 1

/*
EXPORT(int) InitializeDecoder(int width, int height);
EXPORT(int) DestroyStillGraph();

EXPORT(int) DecoderPause();
EXPORT(int) DecoderStop();
EXPORT(int) DecoderRun();

EXPORT(int) SetCallBackFunction(void (*f)(double sampleTime));

EXPORT(int) SetRenderRect(int left, int top, int width, int height);
EXPORT(int) ShowRenderWindow(int show);
EXPORT(int) SetRenderWindow(void* hWnd);
EXPORT(int) DecoderCaptureFrame();
EXPORT(int) DecoderCopyLastFrameInto(unsigned char *bits, int width, int height, int depth);
EXPORT(unsigned int) DecoderDecodeSize();
EXPORT(unsigned char*) DecoderGetLastFrame();
EXPORT(int) DecoderSetDecodeSize(int w, int h);
EXPORT(int) DecoderGetLastError();
*/

#ifdef __cplusplus
}
#endif


#endif //#ifndef __Q_WEBCAM_PLUGIN_H__

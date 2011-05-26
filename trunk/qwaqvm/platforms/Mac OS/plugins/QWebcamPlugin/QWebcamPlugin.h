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
 * QWebcamPlugin.h	(MacOS)
 *
 * Declares the functions that are directly used 
 * by the generated code in QWebcamPlugin.c. 
 ******************************************************************************/

#ifndef __Q_WEBCAM_PLUGIN_H__
#define __Q_WEBCAM_PLUGIN_H__

#include "qFeedbackChannel-interface.h"	  // The plugin uses these also

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
char*	qCameraName(int cameraIndex);
char*	qCameraUID(int cameraIndex);
void  qCopyBufferToBitmap(char *bitmapData, void *bufferAddr, long bufSize);

// Enumeration of available devices.  
// These devIndexes range [ 0, qGetNumDevices()-1 ]
//
int		qGetNumCamDevices ();
char*	qGetCamDeviceName(int devIndex);
char*	qGetCamDeviceUID(int devIndex);
int		qGetCamDeviceIsBusy(int devIndex);		// Boolean 0 or 1
	
#ifdef __cplusplus
}
#endif


#endif //#ifndef __Q_WEBCAM_PLUGIN_H__

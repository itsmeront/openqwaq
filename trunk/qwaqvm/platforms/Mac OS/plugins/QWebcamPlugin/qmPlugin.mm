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
 *  QWebcamPlugin c-to-c++ entry points implementation for Mac OS.
 *
 *  Maintains the indexed master table of WebcamQTKit instances,
 *  delegating requests into them for each API call.
 */

#include "QWebcamPlugin.h"
#include "qLogger.hpp"
#include "qBuffer.h"
#include "qException.h"
#include "qFeedbackChannel.h"
#include "qWebcamQTKit.h"

using namespace Qwaq;

const int MAX_CAMERAS = 5;

static BufferPool2 gBufferPool;

static WebcamQTKit * gCameras[MAX_CAMERAS];

///////////////////////////////////////////
// Private table of known devices.

typedef struct qCamDeviceCache {
	char* camName;
	char* camUID;
	int	  isBusy;
} qCamDeviceCache_struct;

static qCamDeviceCache ** gKnownDevices = NULL;
static int gNumKnownDevices = 0;

///////////////////////////////////////////
// forward declarations of helper functions
///////////////////////////////////////////

static int findUnusedCameraIndex();

/////////////////////////////////////////////////////
// definitions of functions called by QWebcamPlugin.c
/////////////////////////////////////////////////////

void qInitModule(void)
{
	int i;
	for (i = 0; i<MAX_CAMERAS; i++) gCameras[i] = NULL;
	qInitLogging();
	qLogToFile("QWebcamPlugin.log");
	qLog() << "Successfully initialized QWebcamPlugin";
}


void qShutdownModule(void)
{
	qLog() << "Shutting down QWebcamPlugin";

	int i;
	for(i=0; i<MAX_CAMERAS; i++) {
		if(gCameras[i] != NULL) {
			delete gCameras[i];
			gCameras[i] = NULL;
		}
	}
	qLog() << "    - destroyed cameras";

	FeedbackChannel::releaseAllChannels();
	qLog() << "    - released feedback channels";
	qShutdownLogging();
}

int qCameraIsValid(int cameraIndex)
{
	if (cameraIndex < 0 || cameraIndex >= MAX_CAMERAS) return 0;
	return (gCameras[cameraIndex] != NULL) && ( gCameras[cameraIndex]->isValidCamera());
}


int qCreateCamera(char* args, int argsSize, char* feedbackChannelHandle)
{
	// Older variant - open the default device.
	return qCreateCameraByUID(NULL, args, argsSize, feedbackChannelHandle);
}

int qCreateCameraByUID(char* uid, char* args, int argsSize, char* feedbackChannelHandle)
{
	FeedbackChannel* channel = *((FeedbackChannel**)feedbackChannelHandle);
	int index = findUnusedCameraIndex();
	if (index < 0) {
		return QE_NO_FREE_OBJECT_SLOTS;
	}
	
	if ( ((WebcamParams*) args)->version < WebcamParams::currentVersion()) {
		qerr << endl << qTime() << "Cannot use params version " << (((WebcamParams*)args)->version);
		return -1;
	}
	
	gCameras[index] = new WebcamQTKit( uid, (WebcamParams*) args, gBufferPool, channel);
	
	if (! (gCameras[index]->isValidCamera())) {
		qerr << endl << qTime() << "Got camera " << (uid ? uid : "default") << " but not usable.";
		qDestroyCamera(index);
		return QE_DEVICE_UNAVAILABLE;
	}
	
	qerr.flush();
	return QS_OK;
}

int qDestroyCamera(int cameraIndex)
{
	if (cameraIndex < 0 || cameraIndex >= MAX_CAMERAS) {
		return QE_INDEX_OUT_OF_RANGE;
	}
	WebcamQTKit * cam = gCameras[cameraIndex];
	gCameras[cameraIndex] = NULL;
	if (cam) {
		delete cam;
	} else {
		qerr << endl << qTime() << "DestroyCamera: no camera at index " << cameraIndex;
	}
	return QS_OK;
}

void qCameraAdjust(int cameraIndex, char* args, int argsSize)
{
	if (cameraIndex < 0 || cameraIndex >= MAX_CAMERAS) {
		qerr << endl << qTime() << " AdjustCamera: no camera at index: " << cameraIndex;
		return;
	}
	WebcamQTKit * cam = gCameras[cameraIndex];
	if (cam) {
		cam->adjustParameters( * (WebcamParams*) args);
	} else {
		qerr << endl << qTime() << " : AdjustCamera: no camera at index " << cameraIndex;
	}
	return;
	
}

int qCameraRun(int cameraIndex)
{
	if (cameraIndex < 0 || cameraIndex >= MAX_CAMERAS) {
		return QE_INDEX_OUT_OF_RANGE;
	}
	WebcamQTKit * cam = gCameras[cameraIndex];
	if (cam) {
		cam->run();
	} else {
		qerr << endl << qTime() << "Run: no camera at index " << cameraIndex;
	}
	return QS_OK;
}

int qCameraPause(int cameraIndex)
{
	if (cameraIndex < 0 || cameraIndex >= MAX_CAMERAS) {
		return QE_INDEX_OUT_OF_RANGE;
	}
	WebcamQTKit * cam = gCameras[cameraIndex];
	if (cam) {
		cam->pause();
	} else {
		qerr << endl << qTime() << "Pause: no camera at index " << cameraIndex;
	}
	return QS_OK;
}

int qCameraStop(int cameraIndex)
{
	if (cameraIndex < 0 || cameraIndex >= MAX_CAMERAS) {
		return QE_INDEX_OUT_OF_RANGE;
	}
	WebcamQTKit * cam = gCameras[cameraIndex];
	if (cam) {
		cam->stop();
	} else {
		qerr << endl << qTime() << "Stop: no camera at index " << cameraIndex;
	}
	return QS_OK;
}


int qCameraGetParams(int cameraIndex, char* externalAddress)
{
	return 0;
}

char* qCameraName (int cameraIndex)
{
	if (cameraIndex < 0 || cameraIndex >= MAX_CAMERAS) {
		return NULL;
	}
	return gCameras[cameraIndex]->cameraName();
}

char* qCameraUID (int cameraIndex)
{
	if (cameraIndex < 0 || cameraIndex >= MAX_CAMERAS) {
		return NULL;
	}
	return gCameras[cameraIndex]->cameraUID ();
}

static void freeDeviceList ()
{
	if (! gKnownDevices) {
		return;
	}
	qCamDeviceCache * cam;
	int i = 0;
	while ( (cam = gKnownDevices[i++]) && (i <= gNumKnownDevices) ) {
		free(cam->camName);
		free(cam->camUID);
		free(cam);
	}
	free(gKnownDevices);
	gKnownDevices = NULL;
	gNumKnownDevices = 0;
}

// Private - add a capture-device to the current knownDevices array.
static void addKnownDevice( QTCaptureDevice *device, int i)
{
	gKnownDevices[i] = (qCamDeviceCache*) malloc(sizeof(qCamDeviceCache));
	gKnownDevices[i]->camName = strdup([[device localizedDisplayName] UTF8String ]);
	gKnownDevices[i]->camUID = strdup([[device uniqueID] UTF8String]);
	gKnownDevices[i]->isBusy = 0;
}

// Private - find the current system-default capture device.
// We treat this specially, as we activate it first.

static QTCaptureDevice * defaultCaptureDevice() {
	QTCaptureDevice* device = [QTCaptureDevice defaultInputDeviceWithMediaType: QTMediaTypeVideo];
	if (! device) {
		device = [QTCaptureDevice defaultInputDeviceWithMediaType: QTMediaTypeMuxed];
	}
	return device;
}


// The number of distinct selectable devices for video input
int qGetNumCamDevices ()
{
	int count;
	freeDeviceList();
	
	// We'll add default device first
    QTCaptureDevice *defaultDevice = defaultCaptureDevice();
	if (! defaultDevice ) {
		return 0;
	}
	
    // then find the rest
    NSArray* devicesWithMediaType =
		[QTCaptureDevice inputDevicesWithMediaType:QTMediaTypeVideo];
    NSArray* devicesWithMuxType =
		[QTCaptureDevice inputDevicesWithMediaType:QTMediaTypeMuxed];
    NSMutableSet* devicesSet =
		[NSMutableSet setWithArray: devicesWithMediaType];
    [devicesSet addObjectsFromArray: devicesWithMuxType];
	[devicesSet removeObject: defaultDevice];
	
	count = [devicesSet count] + 1;
	gKnownDevices = (qCamDeviceCache**) calloc (count+1, sizeof(qCamDeviceCache *));

	// Add that default device.
	addKnownDevice (defaultDevice, 0);
	
	// add all devices from set to cache
    NSEnumerator *enumerator = [devicesSet objectEnumerator];
	int i = 1;
    id value;
    while ((value = [enumerator nextObject]) && (i < count)) {
		addKnownDevice((QTCaptureDevice*)value, i++);
    }

	return (gNumKnownDevices = i);
}

// The user-friendly name for the specified device.
char* qGetCamDeviceName(int devIndex) {
	if (devIndex >= gNumKnownDevices || devIndex < 0) {
			qLog() << endl << "getCameraName: Invalid index " << devIndex;
		return NULL;
	}
	return gKnownDevices[devIndex]->camName;
}

// The persistent identifier for the specified device.
char* qGetCamDeviceUID(int devIndex) {
	if (devIndex >= gNumKnownDevices || devIndex < 0) {
		qLog() << endl << "getCameraUID: Invalid index " << devIndex;
		return NULL;
	}
	return gKnownDevices[devIndex]->camUID;
}

// If we can be sure the specified device is in use elsewhere, return 1, else 0.
int qGetCamDeviceIsBusy (int devIndex) {
	if (devIndex >= gNumKnownDevices || devIndex < 0) {
		qLog() << endl << "getCameraIsBusy: Invalid index " << devIndex;
		return 0;
	}
	return gKnownDevices[devIndex]->isBusy;
}

void   qCopyBufferToBitmap(char *bitmapData, void *bufferAddress, long bufferSize)
{
	memcpy(bitmapData, *(char**)bufferAddress, bufferSize);
}

//////////////////////////////
// helper function definitions
//////////////////////////////

static int findUnusedCameraIndex()
{
	int i;
	for (i=0; i<MAX_CAMERAS; ++i) {
		if (gCameras[i] == NULL) return i;
	}
	return -1;
}

@implementation CamOutputHandler

// This delegate method is called whenever the preview-video-output receives a frame
- (void)captureOutput:(QTCaptureOutput *)captureOutput didOutputVideoFrame:(CVImageBufferRef)videoFrame withSampleBuffer:(QTSampleBuffer *)sampleBuffer fromConnection:(QTCaptureConnection *)connection
{
	
	@synchronized (self) {
		if (! qCameraIsValid(camIndex) ) {
			return;
		}
		WebcamQTKit * cam = gCameras[camIndex];

		void* base;
		size_t size, height;
		if (CVPixelBufferLockBaseAddress(videoFrame, 0)) {
			qerr << endl << qTime() << "Cannot lock frame buffer.";
			return;
		}
		
		// Get info about the raw pixel-buffer data.
		base = (void*) CVPixelBufferGetBaseAddress(videoFrame);
		height = CVPixelBufferGetHeight(videoFrame);
		size = height * CVPixelBufferGetBytesPerRow(videoFrame);
		
		double nsinterval;
		QTGetTimeInterval ([sampleBuffer decodeTime], & nsinterval);
		cam->receiveFrame (base, size, nsinterval);
		
		// We're done with the pixel-buffer
		CVPixelBufferUnlockBaseAddress(videoFrame, 0);
    }
}

- (void) camIndex: (int) index
{
	camIndex = index;
}

@end



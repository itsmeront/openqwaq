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
 *  QWebcamPlugin implementation for linux.
 */

#include "QWebcamPlugin.h"
#include "qException.h"
#include "qLogger.hpp"
#include "qFeedbackChannel.h"
#include "sqVirtualMachine.h"
#include "v4l2.h"

extern struct VirtualMachine* interpreterProxy;

using namespace Qwaq;

/* This is lifted from platforms/Cross/plugins/QWebcamPlugin/qWebcam.h.
 * We don't really need a webcam class; it gets in the way of eliminating
 * the ring buffer between driver and Squeak, because we don't have to
 * subclass Buffer2.  But we do need WebcamParams & SqueakVideoFrameEvent.
 */
	///////////////////////////////////////////////////
	// Parameters used during the creation of a webcam.
	// Use '#pragma pack' so that the corresponding
	// Squeak FFI classes needn't add padding-fields.
	///////////////////////////////////////////////////
#pragma pack(push,1)
	struct WebcamParams
	{
		static int currentVersion() { return 1; }

		// version 1 fields
		int version;
		int width;
		int height;
		double fps;
		// version 2 fields
		//    (nothing here yet)
	};
#pragma pack(pop)

	//////////////////////////////////////////////////////////
	// Event structures that are passed back to Squeak via a
	// FeedbackChannel.  We use '#pragma pack' so that they
	// have the exact same memory layout as the Squeak FFI
	// classes used to access them... we don't want to have
	// to insert padding in the field definitions in Squeak.
	//////////////////////////////////////////////////////////

	enum VideoEventTypes { EVENT_VIDEOFRAME = 1 };

#pragma pack(push,1)
	// Same memory layout as Squeak class QVideoFrameEvent up through buffer
	struct SqueakVideoFrameEvent
	{
		int type;
		double sampleTime;
		unsigned int bufferSize;
		v4l_buffer_desc *buffer;
	};
#pragma pack(pop)

	//
	// A LinuxVideoFrameEvent is created each time we receive a new
	// frame from the filter-graph.
	//
	class LinuxVideoFrameEvent : public FeedbackEvent
	{
	public:
		LinuxVideoFrameEvent(v4l_buffer_desc *vbdp);
		virtual int getSqueakEventSize();
		virtual void fillInSqueakEvent(char* ptr);

		SqueakVideoFrameEvent squeakEvent;
	private: // not really; squeakEvent->buffer points to this.
		v4l_buffer_desc v4lFrame;
	};

LinuxVideoFrameEvent::LinuxVideoFrameEvent(v4l_buffer_desc *vbdp)
{
	v4lFrame = *vbdp;
    squeakEvent.type = EVENT_VIDEOFRAME;
    squeakEvent.bufferSize = vbdp->outputBitmapSize;
    /* Using v4lFrame as pointer allows dereference by qCopyBufferToBitmap */
	/* and hence we avoid copying the buffer. */
    squeakEvent.buffer = &v4lFrame;
	fflush(stdout);
}

int
LinuxVideoFrameEvent::getSqueakEventSize()
{
	return sizeof(SqueakVideoFrameEvent);
}

void
LinuxVideoFrameEvent::fillInSqueakEvent(char* ptr)
{
	assert(squeakEvent.buffer == &v4lFrame);
	*(struct SqueakVideoFrameEvent *)ptr = squeakEvent;
	fflush(stdout);
}

/* only one camera so far, and hence 1 feedback channel */

static Camera camera;
static FeedbackChannel *feedbackChannel;
static usqLong cameraStartTime;
static char camuuid[] = "9566EE98-05AA-4344-9E23-B729D8D98F20";

#define validCameraIndex(i) ((i) == 0)
#define validCamera(i) ((i) == 0 && camera)

using namespace Qwaq;

/////////////////////////////////////////////////////
// definitions of functions called by QWebcamPlugin.c
/////////////////////////////////////////////////////

void qInitModule(void)
{
	qInitLogging();
	qLogToFile("QWebcamPlugin.log");
	qLog() << "Successfully initialized QWebcamPlugin";
}

static void pollCameraForFrame(void);

static void
close_camera()
{
	interpreterProxy->addHighPriorityTickee(pollCameraForFrame, 0);
	if (camera) {
		int err = v4l_close(camera);
		camera = 0;
		qLog() << "    - closed camera";
	}
	else
		qLog() << "    - camera not open";
}

void
qShutdownModule(void)
{
	qLog() << "Shutting down QWebcamPlugin";

	close_camera();

	FeedbackChannel::releaseAllChannels();
	qLog() << "    - released feedback channels";
	qShutdownLogging();
}

int
qCameraIsValid(int cameraIndex)
{
	return validCameraIndex(cameraIndex) && camera;
}


int
qCreateCamera(char* args, int argsSize, char* feedbackChannelHandle)
{
	// Older variant - open the default device.
	return qCreateCameraByUID(NULL, args, argsSize, feedbackChannelHandle);
}

int
qCreateCameraByUID(char* uid, char* args, int argsSize, char* feedbackChannelHandle)
{
	int err;

	if (camera)
		return QE_NO_FREE_OBJECT_SLOTS;
	
	if ( ((WebcamParams*) args)->version < WebcamParams::currentVersion()) {
		qerr << endl << qTime() << " Cannot use params version " << (((WebcamParams*)args)->version);
		return -1;
	}

	if ((err = v4l_open(&camera))) {
		qerr << endl << qTime() << " Open camera failed with " << err;
		return QE_DEVICE_UNAVAILABLE;
	}

	feedbackChannel = *(FeedbackChannel**)feedbackChannelHandle;
	qerr.flush();
	return QS_OK;
}

int
qDestroyCamera(int cameraIndex)
{
	if (!validCamera(cameraIndex))
		return QE_INDEX_OUT_OF_RANGE;

	if (camera)
		close_camera();
	return QS_OK;
}

/* N.B.  This could do with allowing error return.  The camera index can be
 * invalid, argsSize can be too small, etc.
 */
void
qCameraAdjust(int cameraIndex, char* args, int argsSize)
{
	if (!(validCamera(cameraIndex))) {
		qerr << endl << qTime() << " AdjustCamera: no camera at index: " << cameraIndex;
		return;
	}
	/* N.B.  Still to do is to stop the camera and restart it if it is running.
	 * This should be pushed into the accessors v4l_setfps etc, which are macros
	 * for now.
	 */
	if (camera) {
		int err;
		assert(((WebcamParams*)args)->fps >= 1.0);
		if ((err = v4l_setfps(camera,(int)((WebcamParams*)args)->fps))) {
			qerr << endl << qTime() << " v4l_setfps failed with " << err;
			interpreterProxy->primitiveFail();
		}
		if ((err = v4l_setextent(camera,
								 ((WebcamParams*)args)->width,
								 ((WebcamParams*)args)->height))) {
			qerr << endl << qTime() << " v4l_setextent failed with " << err;
			interpreterProxy->primitiveFail();
		}
	}
	return;
}

int
qCameraRun(int cameraIndex)
{
	int err;

	if (!validCamera(cameraIndex))
		return QE_INDEX_OUT_OF_RANGE;

	if ((err = v4l_initformat(camera))) {
		qerr << endl << qTime() << " v4l_initformat failed with " << err;
		interpreterProxy->primitiveFail();
		return err;
	}
	if ((err = v4l_mmap(camera))) {
		qerr << endl << qTime() << " v4l_mmap failed with " << err;
		interpreterProxy->primitiveFail();
		return err;
	}
	if ((err = v4l_capture(camera))) {
		qerr << endl << qTime() << " v4l_capture failed with " << err;
		interpreterProxy->primitiveFail();
		return err;
	}
	cameraStartTime = 0;
# define PERIODMS (1000 / (v4l_getfps(camera) * 2))
	interpreterProxy->addHighPriorityTickee(pollCameraForFrame, PERIODMS);

	return QS_OK;
}

int
qCameraStop(int cameraIndex)
{
	int err;

	if (!validCamera(cameraIndex))
		return QE_INDEX_OUT_OF_RANGE;

	if ((err = v4l_stop(camera))) {
		qerr << endl << qTime() << " v4l_stop failed with " << err;
		interpreterProxy->primitiveFail();
	}
	interpreterProxy->addHighPriorityTickee(pollCameraForFrame, 0);

	return QS_OK;
}


int
qCameraPause(int cameraIndex) { return qCameraStop(cameraIndex); }

int
qCameraGetParams(int cameraIndex, char* externalAddress) { return 0; }

char*
qCameraName(int cameraIndex)
{
	if (!validCameraIndex(cameraIndex)) {
		interpreterProxy->primitiveFail();
		return 0;
	}

	return v4ldev_simple_name(cameraIndex);
}

char* qCameraUID(int cameraIndex)
{
	return validCameraIndex(cameraIndex) ? camuuid : NULL;
}

void
qCopyBufferToBitmap(char *bitmapData, void *frameDescPtr, long bufSize)
{
	v4l_buffer_desc *frameDesc = *(v4l_buffer_desc **)frameDescPtr;

	0 && printf("qCBTB %p -> %p %d\n",frameDescPtr,frameDesc,frameDesc->bufferIndex);
	fflush(stdout);
	assert(frameDesc->camera == camera);
	assert(frameDesc->outputBitmapSize == bufSize);
	v4l_copyframe_to_rgba32(frameDesc, bitmapData, bufSize);
	v4l_releaseframe(frameDesc->camera, frameDesc);
}

// The number of distinct selectable devices for video input
int
qGetNumCamDevices()
{
	/* even if v4l knows there are multiple devices only support one as yet. */
	return v4ldev_num_video_devices() ? 1 : 0;
}

char*
qGetCamDeviceName(int deviceIndex)
{
	if (!validCameraIndex(deviceIndex)) {
		interpreterProxy->primitiveFail();
		return 0;
	}

	return v4ldev_device_name(deviceIndex);
}

// The persistent identifier for the specified device.
char*
qGetCamDeviceUID(int deviceIndex)
{
	return validCameraIndex(deviceIndex) ? camuuid : NULL;
}

// If we can be sure the specified device is in use elsewhere, return 1, else 0.
int
qGetCamDeviceIsBusy(int deviceIndex)
{
	if (!validCameraIndex(deviceIndex)) {
		qLog() << endl << "getCameraIsBusy: Invalid index " << deviceIndex;
		return 0;
	}
	if (camera)
		return 0;
	return v4ldev_is_busy(deviceIndex);
}



/* polling implementation.  Use a high-priority ticker to poll the driver
 * periodically for a frame and push an event on the feedbackChannel when
 * available.
 */
static void
pollCameraForFrame(void)
{
	int err;
	if (camera
	 && v4l_querydataready(camera,&err)) {
		v4l_buffer_desc frame;

		if ((err = v4l_grabframe(camera, &frame))) {
			qerr << endl << qTime() << " pollCameraForFrame(): v4l_grabframe returned " << err;
			return;
		}
		// Instantiate a new VideoFrameEvent to communicate back to Squeak.
		// It gets destroyed in qFeedbackChannel.cpp::qFeedbackChannelPopEvent.
		LinuxVideoFrameEvent *evt = new LinuxVideoFrameEvent(&frame);
		if (!evt) {
			qerr << endl << qTime() << " pollCameraForFrame(): failed to instantiate VideoFrameEvent";
			return;
		}
#define MicrosecondsPerSecond 1000000
		if (cameraStartTime == 0) {
			evt->squeakEvent.sampleTime = 0.0;
			cameraStartTime = frame.timestamp;
		}
		else
			evt->squeakEvent.sampleTime = (double)(frame.timestamp - cameraStartTime)
										/ MicrosecondsPerSecond;
		FeedbackEventPtr ptr(evt);
		0 && printf("evt: %p ptr: %p %d\n", evt, &ptr, frame.bufferIndex);
		fflush(stdout);
		feedbackChannel->push(ptr);
	}
	else if (camera && err)
		qerr << endl << qTime() << " pollCameraForFrame(): v4l_querydataready returned " << err;
}

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


// WebcamQTKit implementation;
// Implements 'Webcam' using the Mac-OSX QTKit video session system.
//
// Historically, the C++ structure is cloned from the Windows WebcamPlugin;
// the objective-C implementation started life in QwaqMediaPlugin.

#include "qWebcamQTKit.h"
#include "qLogger.hpp"
#include "qException.h"

using namespace Qwaq;

WebcamQTKit::WebcamQTKit(char* const uid, WebcamParams* params, BufferPool2& bufferPool, FeedbackChannel* ch) : Webcam(params, bufferPool, ch)
{
	qLog() << "New Webcam(QTKit)";
	
	targetFrameInterval = 0.0;
	targetSampleTime = 0.0;
	
	handler = NULL;
	session = NULL;
	device = NULL;
	input = NULL;
	output = NULL;
	camName = NULL;
	camUID = NULL;
	
	targetFrameInterval = 1.0 / parameters.fps;
	targetSampleTime = 0.0;
	camUID = uid ? strdup(uid) : NULL;
	
	if (setupCamera(uid) != 0) {
		destroySession();
	}
	qerr.flush();
}


WebcamQTKit::~WebcamQTKit()
{
	destroySession();
	if (camName) {
		free(camName);
		camName = NULL;
	}
	if (camUID) {
		free(camUID);
		camUID = NULL;
	}
}

bool
WebcamQTKit::isValidCamera()
{
	// If we made a non-nil handler, we got full QTKit camera
	return (handler != NULL);
}

void
WebcamQTKit::run()
{
	qLog() << "Camera run " << (device ? ([[device localizedDisplayName] UTF8String]) : "but no device.");
	targetSampleTime = 0.0;
	if (session) {
		if (! [session isRunning]) {
			configureOutput();
			[ session startRunning ];
		}
	} else {
		qerr << endl << qTime() << "Run: no session to run.";
	}
}

void
WebcamQTKit::pause()
{
	if (session) {
		qLog() << "Camera pause";
		if ([session isRunning]) [ session stopRunning ];
	} else {
		qerr << endl << qTime() << "Pause: no session to pause.";		
	}
	targetSampleTime = 0.0;
}


void
WebcamQTKit::stop()
{
	if (session) {
		qLog() << "Camera stop";
		if ([session isRunning]) [ session stopRunning ];
	}
	targetSampleTime = 0.0;
}

void
WebcamQTKit::adjustParameters(WebcamParams& params)
{
	if (params.width != parameters.width  ||  params.height != parameters.height) {
		qerr << endl << qTime() << "adjustParameters: cannot change size on-the-fly";
		qerr.flush();
	}
	
	if (params.fps != parameters.fps) {
		// Need to adjust frame-rate settings
		parameters.fps = params.fps;
		if (targetSampleTime == 0.0) {
			// No samples received yet; just set the target interval
			targetFrameInterval = 1.0 / parameters.fps;
		}
		else {
			// Adjust the targetSampleTime for the new target interval
			targetSampleTime -= targetFrameInterval;
			targetFrameInterval = 1.0 / parameters.fps;
			targetSampleTime += targetFrameInterval;
		}
	}
}

// User-friendly device name.

char* WebcamQTKit::cameraName() {
	if (!device) {
		return NULL;
	}
	if (camName) {
		return camName;
	}
	return camName = strdup([[device localizedDisplayName] UTF8String]);
}

// Persistent camera uniqueID

char* WebcamQTKit::cameraUID() {
	if (!device) {
		return NULL;
	}
	if (camUID) {
		return camUID;
	}
	return camUID = strdup([[device uniqueID] UTF8String]);
}


// Assemble a full video-grabbing session from the default video input device.

int WebcamQTKit::setupCamera (char * uid)
{
	qLog() << "Creating capture session";
	destroySession();
	
	session = [[QTCaptureSession alloc] init];
	device =  camDevice(uid);
	if (! device ) {
		qerr << endl << qTime() << "No video input device (" << (uid ? uid : "system default") << ").";
		[session release];
		session = nil;
		return -4;
	}
	[device retain];
	qLog() << "CreateCamera: Got device " 
		    << ([[device localizedDisplayName] UTF8String])
			<< " UUID "<< ([[device uniqueID] UTF8String]);
	
	// add an input for this device
	NSError* error = nil;
	[device open: &error];
	if (error != nil) {
		qerr << endl << qTime() << "Cannot open camera device: " << [[error localizedDescription] UTF8String];
		destroySession();
		return -5;
	}
	
	input = [QTCaptureDeviceInput deviceInputWithDevice: device];
	if (! input) {
		qerr << endl << qTime() << "Camera input not received.";
		destroySession();
		return -5;
	}
	[input retain];
	
    if (! [session addInput: input error: &error] ) {
		qerr << endl << qTime() << "Cannot attach device input: " << [[error localizedDescription] UTF8String];
		destroySession();
		return -5;
	}
   	
	handler = [[CamOutputHandler alloc] init];
	// Add a decompressed video output that returns raw frames to the session
	// We could use the preview-output as it (even in older 10.5) is happy to drop frames,
 	// and uses lower quality assumptions to keep the rates low.
	// output = [[QTCaptureVideoPreviewOutput alloc] init];
	output = [[QTCaptureDecompressedVideoOutput alloc] init];
	
	configureOutput ();
	
	[output setDelegate: handler];
	if ( ! [session addOutput: output error: &error ] ) {
		qerr << endl << qTime() << "Cannot attach capture output: " << [[error localizedDescription] UTF8String];
		destroySession();
		return -5;
	}
	
	return 0;
}

// QTCaptureDevice acquisition.
// Lookup of the system default capture device.

QTCaptureDevice * WebcamQTKit::defaultCamDevice() 
{
	qLog() << "Requesting default device ";
	QTCaptureDevice * cam = [QTCaptureDevice defaultInputDeviceWithMediaType: QTMediaTypeVideo];
	if (! cam) {
		cam = [QTCaptureDevice defaultInputDeviceWithMediaType: QTMediaTypeMuxed];
	}
	return cam;	
}

// QTCaptureDevice acquisition.
// Looking up a device given its uniqueID string.
// (If the uid argument is nil, looks up the system default capture device.)

QTCaptureDevice * WebcamQTKit::camDevice(char* uid)
{
	if (! uid) {
		return defaultCamDevice();
	}
	qLog() << "Requesting uid " << uid;
	qLog().flush();
	
	// then find the rest
    NSArray* devices = [QTCaptureDevice inputDevicesWithMediaType: QTMediaTypeVideo];
    NSEnumerator *enumerator = [devices objectEnumerator];

	QTCaptureDevice* value;
    while (value = ((QTCaptureDevice*) [enumerator nextObject])) {	// while not nil
		const char * cuid = [[ value uniqueID ] UTF8String ];
		if (cuid && ( strcmp( uid, cuid ) == 0)) {
			return value;
		}}
	
	devices = [QTCaptureDevice inputDevicesWithMediaType:QTMediaTypeMuxed];
	enumerator = [devices objectEnumerator];
	while (value = ((QTCaptureDevice*) [enumerator nextObject])) {	// while not nil
		const char * cuid = [[ value uniqueID ] UTF8String ];
		if (cuid && ( strcmp( uid, cuid ) == 0)) {
			return value;
		}}
		
	return NULL;
}

// We get pixel buffers from the session-output (via my handler)
// in the size and format specified here.

void WebcamQTKit::configureOutput ()
{
	// Set the pixel buffer for the output to the format we can consume.
	NSDictionary * attrs = [NSMutableDictionary dictionaryWithCapacity: 4];
	
	// CF versus objective-c warnings... dunno.
	[attrs setValue: [NSNumber numberWithInt: parameters.width] forKey: ((NSString*)kCVPixelBufferWidthKey) ];
	[attrs setValue: [NSNumber numberWithInt: parameters.height] forKey: ((NSString*)kCVPixelBufferHeightKey) ];
	[attrs setValue: [NSNumber numberWithUnsignedInt: Q_PIXEL_FORMAT] forKey: ((NSString*)kCVPixelBufferPixelFormatTypeKey) ];
	[output setPixelBufferAttributes: attrs];	
	
	// These require > 10.5, and a QTCaptureDecompressedVideoOutput
	@try {
		[output setMinimumVideoFrameInterval: (targetFrameInterval * 0.999) ];
		[output setAutomaticallyDropsLateVideoFrames: YES];
	} @catch (id theException) {
		qerr << endl << qTime() << "No video-source rate control here.";
	}
}

// Take down any and all QTKit capture-session objects.

void WebcamQTKit::destroySession () 
{
	qLog() << "Camera session ends.";
	if (session) {
		if ([session isRunning]) [session stopRunning ];
		if (output) [session removeOutput: output ];
		if (input) [ session removeInput: input ];
		
		[session release];
		session = nil;
	}
	if (input) {
		[input release];
		input = nil;
	}
	if (output) {
		[output release];
		output = nil;
	}
	if (handler) {
		[handler release];
		handler = nil;
	}
	
	if (device) {
		qLog() << "Closing camera " << [[device localizedDisplayName] UTF8String];
		if ([device isOpen]) [device close];
		[device release];
		device = nil;
	}
}

// This is the per-frame callback, relayed from my 'CamOutputHandler' (an objective-C proxy)"

void WebcamQTKit::receiveFrame (void*pixelBufferBase, size_t size, double frameTime) {
	
	// qLog() << "frame " << frameTime;

	if (targetSampleTime == 0.0) {
		// This is the first frame... set up expected time for next sample
		targetSampleTime = frameTime;
	} else if (frameTime > (targetSampleTime - 0.2*targetFrameInterval)) {
		// Accept this sample, and update next targetSampleTime.
		targetSampleTime += targetFrameInterval;
		if (targetSampleTime < frameTime) {
			// Atypical, but worth bulletproofing against
			targetSampleTime = frameTime + targetFrameInterval;
		}
	} else {
		// qLog() << "Early frame! " << targetSampleTime;
		return;	// The sample is early.  Discard it and wait for the next one.
	}
	
	BufferPool2::ptr_type buf = pool.getBuffer(size);
	if (!buf) {
		qerr << endl << qTime() << "receiveFrame: cannot get Buffer size: " << size;
		return;
	}
	
	buf->copyFrom(size, (unsigned char*) pixelBufferBase);
	
	// Instantiate a new VideoFrameEvent to communicate back to Squeak.
	VideoFrameEvent *evt = new VideoFrameEvent(buf);
	if (!evt) {
		qerr << endl << qTime() << "receiveFrame: cannot make VideoFrameEvent";
		return;
	}
	evt->squeakEvent.sampleTime = frameTime;
	
	// Pass the frame event to Squeak
	grabbedFrame(evt);
}

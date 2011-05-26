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
 * qVideoDecoder.c
 * QVideoCodecPluginQT
 *
 * Implements API-specific (in this case, QuickTime-specific) parts
 * of the plugins.
 */

/* Must declare the decoder struct... may contain API-specific fields. */
#include "../QVideoCodecPlugin/qReadback.h"
#include "qVideoQuickTime.h"
typedef struct QDecoder
{
	int semaIndex;
	QCallbackData callbackData;
	int outFrameCount;
	int inFrameCount;
	int width;
	int height;
	ICMDecompressionSessionRef session;
	ImageDescription imDesc;
	TimeScale timeScale;
	CodecType codecType;
} QDecoder;

/* Now that we've defined 'struct QDecoder', we can include 'QVideoDecoder.inc' */
#include "../QVideoCodecPlugin/QVideoDecoder.inc"


/* Now we must define the API-specific functions declared in the .inc file */

/* qQuickTimeDecoderCallback: Callback function for QuickTime to notify us when a decoded
                              frame is available.  This is used by both camera and decoder
							  objects.
   Arguments:
   Comments:
     This function is an ICMDecompressionTrackingCallback (see ImageCompression.h)
	 It can handle
*/
OSErr qQuickTimeDecoderCallback(void *decompressionTrackingRefCon,
								OSStatus result,
								ICMDecompressionTrackingFlags decompressionTrackingFlags,
								CVPixelBufferRef pixelBuffer,
								TimeValue64 displayTime,
								TimeValue64 displayDuration,
								ICMValidTimeFlags validTimeFlags,
								void *reserved,
								void *sourceFrameRefCon);

int qCreateDecoderAPI(QDecoder **dptr, char* args, int semaIndex, int width, int height)
{
	QDecoder* decoder;			// the decompressor to be initialized
	ImageDescription* imDesc;	// description of input frame images
	ImageDescriptionHandle myImHandle;
	
	CFNumberRef number = NULL;
	CFMutableDictionaryRef pixelBufferAttributes = NULL;
	ICMDecompressionTrackingCallbackRecord trackingCallbackRecord;
	QVideoArgs* decoderArgs = (QVideoArgs*)args;
	CodecInfo codecInfo;
	
	OSType pixelFormat = Q_PIXEL_FORMAT;
	
	OSStatus err;             // status of QuickTime functions

	fprintf(QSTDERR, "\nqCreateDecoderQT: START DECODER CREATION! (width: %d height: %d)", width, height);

	decoder = (QDecoder*)malloc(sizeof(QDecoder));
	if (decoder == NULL) {
		fprintf(QSTDERR, "\nqCreateDecoderQT: failed to malloc decoder struct");
		return -2; 
	}

	// Get codec info
	decoder->codecType = *((CodecType*)(decoderArgs->codecType));
	err = GetCodecInfo(&codecInfo, decoder->codecType, NULL);
	if (err != noErr) {
		fprintf(QSTDERR, "\nqCreateDecoderQT: cannot get codec info");
		fprintf(QSTDERR, "\n\tQUICKTIME ERROR CODE: %d", err);
		free(decoder);
		return -5;
	}

	// We have found an available camera to initialize.
	decoder->timeScale = decoderArgs->timeScale;
	decoder->semaIndex = semaIndex;
	decoder->width = width;
	decoder->height = height;
	decoder->inFrameCount = decoder->outFrameCount = 0;
	qInitCallbackData(&(decoder->callbackData));
	
	fprintf(QSTDERR, "\nqCreateDecoderQT: INITIALIZED STRUCTURE");
	
	imDesc = &(decoder->imDesc);
	imDesc->idSize          = 86;                     // grabbed from camera (total size of ImageDescription including extra data)
	imDesc->cType			= decoder->codecType;
	//imDesc->resvd1;                                 // reserved for Apple use
	//imDesc->resvd2;                                 // reserved for Apple use
	imDesc->dataRefIndex    = 0;                      // docs say that this must be set to zero
	imDesc->version			= codecInfo.version;
	imDesc->revisionLevel   = codecInfo.revisionLevel;
	imDesc->vendor          = codecInfo.vendor;         
	imDesc->temporalQuality = codecNormalQuality;
	imDesc->spatialQuality  = codecNormalQuality;
	imDesc->width           = width;                  // in pixels
	imDesc->height          = height;                 // in pixels
	imDesc->hRes            = FloatToFixed(72.0);     // DPI, I presume                
	imDesc->vRes            = FloatToFixed(72.0);     // ditto
	imDesc->dataSize        = 0;                      // every frame will have a different size
	imDesc->frameCount      = 0;                      // # of frames this desc applies to (is '1' what we want?)
	memcpy(imDesc->name, codecInfo.typeName, 32 );
	imDesc->depth           = 24;                     // might eventually want to support 32 (to support an alpha-channel)
	imDesc->clutID          = -1;                     // no color-lookup table
		
	fprintf(QSTDERR, "\nqCreateDecoderQT: INITIALIZED IMAGE DESCRIPTION");
	
	pixelBufferAttributes = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );

		number = CFNumberCreate( NULL, kCFNumberIntType, &width );
		CFDictionaryAddValue( pixelBufferAttributes, kCVPixelBufferWidthKey, number );
		CFRelease( number );
		
		number = CFNumberCreate( NULL, kCFNumberIntType, &height );
		CFDictionaryAddValue( pixelBufferAttributes, kCVPixelBufferHeightKey, number );
		CFRelease( number );
		
		number = CFNumberCreate( NULL, kCFNumberSInt32Type, &pixelFormat );
		CFDictionaryAddValue( pixelBufferAttributes, kCVPixelBufferPixelFormatTypeKey, number );
		CFRelease( number );
		
		CFDictionaryAddValue( pixelBufferAttributes, kCVPixelBufferCGBitmapContextCompatibilityKey, kCFBooleanTrue );
		CFDictionaryAddValue( pixelBufferAttributes, kCVPixelBufferCGImageCompatibilityKey, kCFBooleanTrue );

	fprintf(QSTDERR, "\nqCreateDecoderQT: SET UP PIXEL-BUFFER ATTRIBUTES");

	trackingCallbackRecord.decompressionTrackingCallback = (ICMDecompressionTrackingCallback)qQuickTimeDecoderCallback;
	trackingCallbackRecord.decompressionTrackingRefCon = decoder;
	
	fprintf(QSTDERR, "\nqCreateDecoderQT: SET UP CALLBACK RECORD");
	
	// Actually create the session.  First, we need to allocate copy the image description into a handle
	// (Quicktime requires this... we dispose of the handle immediately afterward).
	myImHandle = (ImageDescriptionHandle)NewHandle(sizeof(ImageDescription));
	**myImHandle = *imDesc;
	fprintf(QSTDERR, "\nDECOMPRESSOR IMAGE DESCRIPTION:");
	qPrintImageDescription(myImHandle);
	
	//  err = ICMDecompressionSessionCreate(kCFAllocatorDefault, myImHandle, NULL, pixelBufferAttributes, &trackingCallbackRecord, &(decomp->session));
	err = ICMDecompressionSessionCreate(NULL, myImHandle, NULL, pixelBufferAttributes, &trackingCallbackRecord, &(decoder->session));

	DisposeHandle((Handle)myImHandle);
	
	if (err != noErr) {
		fprintf(QSTDERR, "\nqCreateDecoderQT: cannot create session");
		fprintf(QSTDERR, "\n\tQUICKTIME ERROR CODE: %d", err);
		free(decoder);
		return -5;
	}
	
	fprintf(QSTDERR, "\nqCreateDecoderQT: FINISHED!!!");
	
	*dptr = decoder;
	return 0;
}


void qDestroyDecoderAPI(QDecoder *decoder)
{
	ICMDecompressionSessionRelease(decoder->session);
	qFreeCallbackData(&(decoder->callbackData));
	free(decoder);
}


int qDecodeAPI(QDecoder *decoder, char* bytes, int byteSize)
{
	OSErr err;	
	ICMFrameTimeRecord frameTime = {{0}};
	TimeValue time;
	
	decoder->inFrameCount++;
	time = decoder->inFrameCount * 100;  // make up something plausible
	
	frameTime.recordSize = sizeof(ICMFrameTimeRecord);
	*(TimeValue64 *)&frameTime.value = time;
	frameTime.scale = decoder->timeScale;
	frameTime.rate = fixed1;
	frameTime.frameNumber = decoder->inFrameCount;
	frameTime.flags = icmFrameTimeIsNonScheduledDisplayTime;
	
	err = ICMDecompressionSessionDecodeFrame(decoder->session, (UInt8*)bytes, byteSize, NULL, &frameTime, NULL);
	if (err != noErr) {
		fprintf(QSTDERR, "\nqDecodeQT(): cannot decode frame");
		fprintf(QSTDERR, "\n\tQUICKTIME ERROR CODE: %d", err);
		return -5;
	}
	ICMDecompressionSessionSetNonScheduledDisplayTime(decoder->session, time, decoder->timeScale, 0 );

	return noErr;
}


OSErr qQuickTimeDecoderCallback(void *decompressionTrackingRefCon,
								OSStatus result,
								ICMDecompressionTrackingFlags decompressionTrackingFlags,
								CVPixelBufferRef pixelBuffer,
								TimeValue64 displayTime,
								TimeValue64 displayDuration,
								ICMValidTimeFlags validTimeFlags,
								void *reserved,
								void *sourceFrameRefCon)
{
	OSStatus err;

	// The decompressionTrackingRefCon might actually be a QCamera or a QDecoder, but we are
	// careful to ensure that they begin with the same layout as QDecoderCallbackData.
	QDecoder* decoder = (QDecoder*)decompressionTrackingRefCon;

	// Declare up here because we need to compile on archaic GCC on Win32
	void* base;
	size_t width;
	size_t height;
	size_t size;

//	fprintf(QSTDERR, "\n\tdecode %d ", decoder->outFrameCount);
	
	if (!pixelBuffer) {
		fprintf(QSTDERR, "\nqQuickTimeDecoderCallback(): no pixel buffer (why?)");
		return noErr;
	}
	if (!(kICMDecompressionTracking_EmittingFrame & decompressionTrackingFlags)) {
		fprintf(QSTDERR, "\nqQuickTimeDecoderCallback(): no frame emitted (why?)");	
		return noErr;
	}

	decoder->outFrameCount++;
	
	// Lock the pixel-buffer until we're done with it.
	err = CVPixelBufferLockBaseAddress(pixelBuffer, 0);
	if (err != noErr) {
		fprintf(QSTDERR, "\nqQuickTimeDecoderCallback(): can't lock CVPixelBuffer");
		// XXXX: so what do we do about it?
		return err;
	}
	// Get info about the raw pixel-buffer data.
	base = (void*)CVPixelBufferGetBaseAddress(pixelBuffer);
	width = CVPixelBufferGetWidth(pixelBuffer);
	height = CVPixelBufferGetHeight(pixelBuffer);
//	size = width*height*4;
	size = height * CVPixelBufferGetBytesPerRow(pixelBuffer);
	
	// Stash the data so that Squeak can retrieve it.
	qStoreCallbackData(base, &(decoder->callbackData), size);

	// We're done with the pixel-buffer
	CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);
	
	// Signal the semaphore so that Squeak can grab the data that we just stashed.
	interpreterProxy->signalSemaphoreWithIndex(decoder->semaIndex);
	
	return noErr;
}

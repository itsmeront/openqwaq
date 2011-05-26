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
 * qVideoEncoder.c
 * QVideoCodecPluginQT
 *
 * Implements API-specific (in this case, QuickTime-specific) parts
 * of the plugins.
 */

/* Must declare the encoder struct... may contain API-specific fields. */
#include "../QVideoCodecPlugin/qReadback.h"
#include "qVideoQuickTime.h"
typedef struct QEncoder
{
	int semaIndex;
	QCallbackData callbackData;
	int outFrameCount;
	int inFrameCount;
	int width;
	int height;
	ComponentInstance compressor;
	ICMCompressionSessionRef session;
	TimeScale timeScale;
	CodecType codecType;
} QEncoder;

#include "../QVideoCodecPlugin/QVideoEncoder.inc"

/* Now we must define the API-specific functions declared in the .inc file */


/*	qQuickTimeEncoderCallback:	Callback function for QuickTime to notify us when an encoded
								frame is available. 
	Arguments:
	Comments:
		This function is an ICMEncodedFrameOutputCallback (see ImageCompression.h)
*/
OSErr qQuickTimeEncoderCallback(void *encodedFrameOutputRefCon, 
								ICMCompressionSessionRef session,
								OSStatus error,
								ICMEncodedFrameRef frame,
								void *reserved);


/******************** Public function definitions **********************/

int qCreateEncoderAPI(QEncoder **eptr, char* encoderArgs, int semaIndex, int width, int height)
{
	QEncoder* encoder;
	OSStatus err;
	QVideoArgs* args = (QVideoArgs*)encoderArgs;

	//XXXX: we should be able to configure this
	SInt32 averageDataRate = 100000;
	
	ICMEncodedFrameOutputRecord encodedFrameOutputRecord = {0};
	ICMCompressionSessionOptionsRef sessionOptions = NULL;
	
	CFMutableDictionaryRef pixelBufferAttributes = NULL;
	CFNumberRef number = NULL;
	OSType pixelFormat = Q_PIXEL_FORMAT;
	
fprintf(QSTDERR, "\nqCreateEncoderQT(): ABOUT TO TRY TO CREATE ENCODER");
fprintf(QSTDERR, "\n\t time-scale: %d", args->timeScale);
fprintf(QSTDERR, "\n\t big-endian: %d", (args->isBigEndian)[0]);
fprintf(QSTDERR, "\n\t codec-type: %c%c%c%c", 
						((unsigned char*)&(args->codecType))[3], 
						((unsigned char*)&(args->codecType))[2], 
						((unsigned char*)&(args->codecType))[1], 
						((unsigned char*)&(args->codecType))[0]);
							
	encoder = (QEncoder*)malloc(sizeof(QEncoder));
	if (encoder == NULL) {
		fprintf(QSTDERR, "\nqCreateDecoderQT: failed to malloc encoder struct");
		return -2; 
	}

	encoder->semaIndex = semaIndex;
	encoder->timeScale = args->timeScale;
	encoder->codecType = *((CodecType*)(args->codecType));
	encoder->width = width;
	encoder->height = height;

	err = ICMCompressionSessionOptionsCreate( NULL, &sessionOptions );
	if(err != noErr) {
		fprintf(QSTDERR, "\nqCreateEncoderQT(): could not create session options");
		fprintf(QSTDERR, "\n\tQUICKTIME ERROR CODE: %d", err);
		free(encoder);
		return -5;
	}
	
	// Create and configure the compressor component.
	OSType codecManufacturer = 'appl';
	ComponentDescription componentDescription;
	componentDescription.componentType = FOUR_CHAR_CODE('imco');
	componentDescription.componentSubType = encoder->codecType;
	componentDescription.componentManufacturer = codecManufacturer;
	componentDescription.componentFlags = 0;
	componentDescription.componentFlagsMask = 0;
	
	Component compressorComponent = FindNextComponent(0, &componentDescription);
	if(compressorComponent == NULL) {
		fprintf(QSTDERR, "\nqCreateEncoderQT(): could not find a matching compressor");
		ICMCompressionSessionOptionsRelease(sessionOptions);
		free(encoder);
		return -5;
	}
	err = OpenAComponent(compressorComponent, &(encoder->compressor));
	if(err != noErr){
		fprintf(QSTDERR, "\nqCreateEncoderQT(): failed to open compressor component");
		fprintf(QSTDERR, "\n\tQUICKTIME ERROR CODE: %d", err);
		ICMCompressionSessionOptionsRelease(sessionOptions);
		free(encoder);
		return -5;
	}
	// If we want to use H.264, we need to muck around a bit.
	// XXXX: "clean up" this code.
	if(encoder->codecType == FOUR_CHAR_CODE('avc1'))
	{
		// Profile is currently fixed to Baseline
		// The level is adjusted by the use of the
		// bitrate, but the SPS returned reveals
		// level 1.1 in case of QCIF and level 1.3
		// in case of CIF
		
		Handle h264Settings = NewHandleClear(0);
		
		err = ImageCodecGetSettings(encoder->compressor, h264Settings);
		if(err != noErr) {
			fprintf(QSTDERR, "\nqCreateEncoderQT(): failed to get codec settings");
			fprintf(QSTDERR, "\n\tQUICKTIME ERROR CODE: %d", err);
			ICMCompressionSessionOptionsRelease(sessionOptions);
			CloseComponent(encoder->compressor);
			free(encoder);
			return -5;
		}
		
		// For some reason, the QTAtomContainer functions will crash if used on the atom
		// container returned by ImageCodecGetSettings.
		// Therefore, we have to parse the atoms self to set the correct settings.
		unsigned i;
		unsigned settingsSize = GetHandleSize(h264Settings) / 4;
		UInt32 *data = (UInt32 *)*h264Settings;
		for(i = 0; i < settingsSize; i++)
		{
			// Forcing Baseline profile
#if defined(__BIG_ENDIAN__)
			if(data[i] == FOUR_CHAR_CODE('sprf'))
			{
				i+=4;
				data[i] = 1;
			}
#else
			if(data[i] == FOUR_CHAR_CODE('frps'))
			{
				i+=4;
				// data[i] = CFSwapInt32(1);
				data[i] = 16777216;  // avoid CFSwapInt32;
			}
#endif
			
			// if video sent is CIF size, we set this flag to one to have the picture
			// encoded in 5 slices instead of two.
			// If QCIF is sent, this flag remains zero to send two slices instead of
			// one.
#if defined(__BIG_ENDIAN__)
			else if(/*videoSize == XMVideoSize_CIF &&*/ data[i] == FOUR_CHAR_CODE('susg'))
			{
				i+=4;
				data[i] = 1;
			}
#else
			else if(/*videoSize == XMVideoSize_CIF &&*/ data[i] == FOUR_CHAR_CODE('gsus'))
			{
				i+=4;
				// data[i] = CFSwapInt32(1);
				data[i] = 16777216;  // avoid CFSwapInt32;
			}
#endif
		}
		err = ImageCodecSetSettings(encoder->compressor, h264Settings);
		if(err != noErr) {
			fprintf(QSTDERR, "\nqCreateEncoderQT(): failed to set codec settings");
			fprintf(QSTDERR, "\n\tQUICKTIME ERROR CODE: %d", err);
			ICMCompressionSessionOptionsRelease(sessionOptions);
			CloseComponent(encoder->compressor);		
			free(encoder);
			return -5;
		}
	}
	err = ICMCompressionSessionOptionsSetProperty(sessionOptions,
												  kQTPropertyClass_ICMCompressionSessionOptions,
												  kICMCompressionSessionOptionsPropertyID_CompressorComponent,
												  sizeof(encoder->compressor),
												  &(encoder->compressor));


	// XXXX: allow (some of) the following options to be set from the 'encoderArgs'

	// We must set this flag to enable P or B frames.
	err = ICMCompressionSessionOptionsSetAllowTemporalCompression( sessionOptions, true );
	if(err != noErr) {
		fprintf(QSTDERR, "\nqCreateEncoderQT(): could not enable temporal compression");
		fprintf(QSTDERR, "\n\tQUICKTIME ERROR CODE: %d", err);
		ICMCompressionSessionOptionsRelease(sessionOptions);
		CloseComponent(encoder->compressor);		
		free(encoder);
		return -5;
	}
	
	// We must set this flag to enable B frames.
// XXXX:	err = ICMCompressionSessionOptionsSetAllowFrameReordering( sessionOptions, true );
	err = ICMCompressionSessionOptionsSetAllowFrameReordering( sessionOptions, false );
	if(err != noErr) {
		fprintf(QSTDERR, "\nqCreateEncoderQT(): could not enable frame reordering");
		fprintf(QSTDERR, "\n\tQUICKTIME ERROR CODE: %d", err);
		ICMCompressionSessionOptionsRelease(sessionOptions);
		CloseComponent(encoder->compressor);		
		free(encoder);
		return -5;
	}
	
	// Set the maximum key frame interval, also known as the key frame rate.
	// XXXX: even 5 frames might be a bit long for videoconferencing
	err = ICMCompressionSessionOptionsSetMaxKeyFrameInterval( sessionOptions, 3 );
	if(err != noErr) {
		fprintf(QSTDERR, "\nqCreateEncoderQT(): could not set maximum keyframe interval");
		fprintf(QSTDERR, "\n\tQUICKTIME ERROR CODE: %d", err);
		ICMCompressionSessionOptionsRelease(sessionOptions);
		CloseComponent(encoder->compressor);
		free(encoder);
		return -5;
	}
	
	// This allows the compressor more flexibility (ie, dropping and coalescing frames).
	// XXXX: does this mean that playback has to be more careful about when to actually display decoded frames?
// XXXX:	err = ICMCompressionSessionOptionsSetAllowFrameTimeChanges( sessionOptions, true );
	err = ICMCompressionSessionOptionsSetAllowFrameTimeChanges( sessionOptions, false );
	if(err != noErr) {
		fprintf(QSTDERR, "\nqCreateEncoderQT(): could not set enable frame time changes");
		fprintf(QSTDERR, "\n\tQUICKTIME ERROR CODE: %d", err);
		ICMCompressionSessionOptionsRelease(sessionOptions);
		CloseComponent(encoder->compressor);
		free(encoder);
		return -5;
	}
	
	// XXXX: CaptureAndCompressIPBMovie set this to true
	err = ICMCompressionSessionOptionsSetDurationsNeeded( sessionOptions, false );
	if(err != noErr) {
		fprintf(QSTDERR, "\nqCreateEncoderQT(): could not set whether frame durations are needed");
		fprintf(QSTDERR, "\n\tQUICKTIME ERROR CODE: %d", err);
		ICMCompressionSessionOptionsRelease(sessionOptions);
		CloseComponent(encoder->compressor);
		free(encoder);
		return -5;
	}
													
	// Set the average data rate.
	// XXXX: another good one to parameterize
	// XXXX: can we change this one at runtime?
	err = ICMCompressionSessionOptionsSetProperty(	sessionOptions, 
													kQTPropertyClass_ICMCompressionSessionOptions,
													kICMCompressionSessionOptionsPropertyID_AverageDataRate,
													sizeof( averageDataRate ),
													&averageDataRate );
	if(err != noErr) {
		fprintf(QSTDERR, "\nqCreateEncoderQT(): could not set average data rate");
		fprintf(QSTDERR, "\n\tQUICKTIME ERROR CODE: %d", err);
		ICMCompressionSessionOptionsRelease(sessionOptions);
		CloseComponent(encoder->compressor);
		free(encoder);
		return -5;
	}
	
	encodedFrameOutputRecord.encodedFrameOutputCallback = (void*)qQuickTimeEncoderCallback;
	encodedFrameOutputRecord.encodedFrameOutputRefCon = encoder;
	encodedFrameOutputRecord.frameDataAllocator = NULL;
	
	// Specify attributes for the compression-session's pixel-buffer pool.
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

	err = ICMCompressionSessionCreate(	NULL,
										width,
										height,
										args->codecType,
										args->timeScale,
										sessionOptions,
										pixelBufferAttributes,
										&encodedFrameOutputRecord,
										&(encoder->session));
	CFRelease(pixelBufferAttributes);
	if(err != noErr) {
		fprintf(QSTDERR, "\nqCreateEncoderQT(): could not create compression session");
		fprintf(QSTDERR, "\n\tQUICKTIME ERROR CODE: %d", err);
		ICMCompressionSessionOptionsRelease(sessionOptions);
		CloseComponent(encoder->compressor);
		free(encoder);
		return -5;
	}
	
	ICMCompressionSessionOptionsRelease(sessionOptions);
	
	*eptr = encoder;
	return 0;
}


void qDestroyEncoderAPI(QEncoder* encoder)
{	
	// push out any remaining frames before releasing the compression session.
	ICMCompressionSessionCompleteFrames(encoder->session, 1, 0, 0);
	
	ICMCompressionSessionRelease(encoder->session);
	CloseComponent(encoder->compressor);
	free(encoder);
}


int qEncodeAPI(QEncoder* encoder, char* bytes, int byteSize)
{
	OSErr err;
	CVPixelBufferPoolRef pixelBufferPool;
	CVPixelBufferRef pixelBuffer;
	unsigned char* baseAddress;
	size_t bufferSize;	
	
	// Grab a pixel buffer from the pool (ICMCompressionSessionEncodeFrame() needs the input
	// data to be passed in as a CVPixelBufferRef).
	pixelBufferPool = ICMCompressionSessionGetPixelBufferPool(encoder->session);
	err = CVPixelBufferPoolCreatePixelBuffer(kCFAllocatorDefault, pixelBufferPool, &pixelBuffer);
	if (err != noErr) {
		fprintf(QSTDERR, "\nqEncodeQT(): could not obtain a pixel buffer from pool");
		fprintf(QSTDERR, "\n\tQUICKTIME ERROR CODE: %d", err);
		return -5;
	}
	
	// Lock the pixel-buffer so that we can copy our data into it for encoding
	// XXXX: would be nice to avoid this copy.
	err = CVPixelBufferLockBaseAddress(pixelBuffer, 0);
	if (err != noErr) {
		fprintf(QSTDERR, "\nqEncodeQT(): could not lock the pixel buffer");
		fprintf(QSTDERR, "\n\tQUICKTIME ERROR CODE: %d", err);
		CVPixelBufferRelease(pixelBuffer);
		return -5;
	}
	baseAddress = CVPixelBufferGetBaseAddress(pixelBuffer);
//	bufferSize = CVPixelBufferGetWidth(pixelBuffer) * CVPixelBufferGetHeight(pixelBuffer) * 4;
	bufferSize = CVPixelBufferGetBytesPerRow(pixelBuffer) * CVPixelBufferGetHeight(pixelBuffer);
	
	// XXXX: for now, just for debugging.  For production, we should notice if this happens and deal with it "appropriately".
	if (byteSize != bufferSize) {
		fprintf(QSTDERR, "\nqEncodeQT(): input data size (%d) does not match pixel-buffer data size (%d)", byteSize, bufferSize);
	}
	
	// Copy the data and unlock the buffer
	memcpy(baseAddress, bytes, bufferSize);
	CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);
  
	// Encode the frame (now in pixel-buffer form).
	err = ICMCompressionSessionEncodeFrame(	encoder->session, 
											pixelBuffer,
											0, 0, 0, // we're not specifying a frame time
											NULL,
											NULL,
											NULL);
	if (err != noErr) {
		fprintf(QSTDERR, "\nqEncodeQT(): could not encode the frame");
		fprintf(QSTDERR, "\n\tQUICKTIME ERROR CODE: %d", err);
		CVPixelBufferRelease(pixelBuffer);
		return -5;
	}
	
	CVPixelBufferRelease(pixelBuffer);
	return 0;
}


OSErr qQuickTimeEncoderCallback(void *encodedFrameOutputRefCon, 
								ICMCompressionSessionRef session,
								OSStatus error,
								ICMEncodedFrameRef frame,
								void *reserved)
{
	QEncoder* encoder = (QEncoder*)encodedFrameOutputRefCon;
	QCallbackData *cbd = &(encoder->callbackData);
	char* buf;
		
	int frameDataSize = ICMEncodedFrameGetDataSize(frame);
	ICMFrameType frameType = ICMEncodedFrameGetFrameType(frame);  // is this a keyframe?

	ImageDescriptionHandle handle;
	OSStatus err;
	long count;
	
	// XXXX DEBUGGGING: let's see if this mofo has any extensions
	/*
	err = ICMEncodedFrameGetImageDescription(frame, &handle);
	if (err != noErr) {
		fprintf(QSTDERR, "\nqQuickTimeEncoderCallback(): cannot get an image description of the frame");
		fprintf(QSTDERR, "\n\tQUICKTIME ERROR CODE: %d", err);	
		return noErr; // I don't know what the QuickTime error code should be (or how QT would interpret it)
	}	
	err = CountImageDescriptionExtensionType(handle, 'avcC', &count);
	if (err != noErr) {
		fprintf(QSTDERR, "\nqQuickTimeEncoderCallback(): cannot get extension count for type 'avcC'");
		fprintf(QSTDERR, "\n\tQUICKTIME ERROR CODE: %d", err);	
		return noErr; // I don't know what the QuickTime error code should be (or how QT would interpret it)
	}
	fprintf(QSTDERR, "\ncount for extension type 'avcC': %d", count);
	*/

	int dataErr = qPrepareCallbackDataForWrite(cbd, frameDataSize+1);
	if (dataErr) {
		fprintf(QSTDERR, "\nqQuickTimeEncoderCallback: cannot get a callback buffer");
		return noErr; // I don't know what the QuickTime error code should be (or how QT would interpret it)
	}
	buf = cbd->data;

	// QwaqMediaPlugin-V1: first byte denotes whether the frame is a keyframe or not.
	if (frameType == kICMFrameType_I) buf[0] = 1;  // yep, that's a keyframe
	else buf[0] = 0;                              // nosirree, that ain't no keyframe

//XXXX: debugging: is the frame a keyframe?
//fprintf(QSTDERR, "\nkeyframe value: %d", frameType);

	// Store the data.
	memcpy(buf+1, ICMEncodedFrameGetDataPtr(frame), frameDataSize);

	interpreterProxy->signalSemaphoreWithIndex(encoder->semaIndex);	
	return noErr;
}


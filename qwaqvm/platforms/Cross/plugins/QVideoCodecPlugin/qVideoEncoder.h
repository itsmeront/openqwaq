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
 * qVideoEncoder.h
 * QVideoCodecPlugin
 *
 */

#ifndef __Q_VIDEO_ENCODER_H__
#define __Q_VIDEO_ENCODER_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*	qInitEncoderStorage: Called when the plugin is being initialized.
	Arguments: none
	Return value: none
*/
void qInitEncoderStorage(void);


/* qCreateEncoder: Create and initialize a new compression session.
   Arguments:
     encoderArgs: 
     semaIndex:   Callback semaphore index
	 width:
	 height:
   Return value: 
     The index of the created encoder, or one of the following
     negative values if the encoder could not be created:
       -4 : The maximum number of encoder has been reached.
       -2 : There was an error while initializing the encoder.
	   -5 : There was a QuickTime error while initializing the camera.
*/
int qCreateEncoder(char* encoderArgs, int encoderArgsSize, int semaIndex, int width, int height);


/*	qDestroyEncoder:	Destroy the indicated encoder.  Does nothing
						if the index is invalid or if the encoder 
						has not been instantiated.
	Arguments:
		encoderIndex: The index of the encoder to destroy.
	Return value: none
*/
void qDestroyEncoder(int encoderIndex);


/*	qDestroyAllEncoders:	Destroy all active encoders.
	Arguments: none
	Return value: none
*/
void qDestroyAllEncoders(void);


/*	qEncoderIsValid: Answer 1 if the encoder has been set up, and 0 otherwise.
	Arguments:
		cameraIndex: The index of the desired encoder.
	Return value: Return 1 if the encoder has already been set up, otherwise 0.
*/
int qEncoderIsValid(int encoderIndex);


/*	qEncode:	Encode the specified bytes.  When encoding is finished, a calback function
				will notify a Squeak semaphore that there is data to be read back.
	Arguments:
		encoderIndex: The index of the encoder to use.
		bytes: Pointer to buffer to be encoded.
		bytesSize: The number of bytes to be encoded.
	Return value:
		0	:	success
		-1	:	invalid handle index
		-5	:	QuickTime error
*/
int qEncode(int encoderIndex, char* bytes, int byteSize);


/*	qEncoderRead: Read data that has been encoded.
	Arguments:
		encoderIndex: The index of the encoder to read back data from.
		bytes: Pointer to buffer that encoded data will be copied into.
		maxLength: The size of the buffer; at most this number of bytes will be read.
	Return value:
		>0	:	number of bytes that were read
		-1	:	invalid handle index
		-5	:	QuickTime error
*/
int qEncoderRead(int encoderIndex, char* bytes, int maxLength);


/*	qEncoderGetProperty: Read data that has been encoded.
	Arguments:
		encoderIndex: The index of the encoder to read back data from.
		propertyName: A string representing the property that we're interested in.
		resultSize: Pointer to an integer that we'll stash the size of the result into (or zero).
	Return value:
		A char* to the property-data, or NULL if the property doesn't exist.  We allocate this data,
		but the caller is responsible for freeing it.  The result is NULL if and only if the
		resultSize is 0.
*/
char* qEncoderGetProperty(int encoderIndex, char* propertyName, int* resultSize);

#ifdef __cplusplus
}
#endif

#endif //#ifndef __Q_VIDEO_ENCODER_H__


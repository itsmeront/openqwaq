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
 * qVideoDecoder.h
 * QVideoCodecPlugin
 *
 */

#ifndef __Q_VIDEO_DECODER_H__
#define __Q_VIDEO_DECODER_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*	qInitDecoderStorage: Called when the plugin is being initialized.
	Arguments: none
	Return value: none
*/
void qInitDecoderStorage(void);


/* qCreateDecoder: Create and initialize a new decompression session.
   Arguments:
     decoderArgs: 
     semaIndex: Callback semaphore index
	 width:
	 height:
   Return value: 
     The index of the created decoder, or one of the following
     negative values if the decoder could not be created:
       -4 : The maximum number of decoders has been reached.
       -2 : There was an error while initializing the decoder.
	   -5 : There was a QuickTime error while initializing the camera.
*/
int qCreateDecoder(char* decoderArgs, int decoderArgsSize, int semaIndex, int width, int height);


/*	qDestroyDecoder:	Destroy the indicated decoder.  Does nothing
						if the index is invalid or if the decoder 
						has not been instantiated.
	Arguments:
		decoderIndex: The index of the decoder to destroy.
	Return value: none
*/
void qDestroyDecoder(int decoderIndex);


/*	qDestroyAllDecoders:	Destroy all active decoders.
	Arguments: none
	Return value: none
*/
void qDestroyAllDecoders(void);


/*	qDecoderIsValid: Answer 1 if the decoder has been set up, and 0 otherwise.
	Arguments:
		cameraIndex: The index of the desired decoder.
	Return value: Return 1 if the decoder has already been set up, otherwise 0.
*/
int qDecoderIsValid(int decoderIndex);


/*	qDecode:	Decode the specified bytes.  When decoding is finished, a calback function
				will notify a Squeak semaphore that there is data to be read back.
	Arguments:
		decoderIndex: The index of the decoder to decode the data with.
		bytes: Pointer to the data to decode.
		bytesSize: The number of bytes of data to decode.
		offset: Offset within 'bytes' that decoding should start at.
	Return value:
		0	:	success
		-1	:	invalid handle index
		-2	:	not all bytes were consumed
		-5	:	QuickTime error
*/
int qDecode(int decoderIndex, char* bytes, int byteSize, int offset);


/*	qDecoderRead: Read data that has been decoded.
	Arguments:
		decoderIndex: The index of the decoder to read back data from.
		bytes: Pointer to buffer that decoded data will be copied into.
		bytesSize: The size of the buffer; at most this number of bytes will be read.
	Return value:
		>0	:	number of bytes that were read
		-1	:	invalid handle index
		-2  :	Squeak buffer is too small to copy entire frame from plugin
		-3  :   Metadata-size does not match expected value.
		-5	:	QuickTime error
*/
int qDecoderRead(int decoderIndex, char* frameBuffer, int bufferSize, char* metadata, int metadataSize);


#ifdef __cplusplus
}
#endif

#endif //#ifndef __Q_VIDEO_DECODER_H__

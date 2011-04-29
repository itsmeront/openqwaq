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
 *  qAudioSpeex.c
 *  QAudioPlugin
 *
 */

#include "qAudioSpeex.h"
#include <speex/speex.h>
#include <speexclient/speex_jitter_buffer.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define SAMPLING_RATE 16000
#define FRAME_SIZE 320

typedef struct QSpeexCodec
{
	int frameSize;

	void* encState;
	SpeexBits encBits;
	
	void* decState;
	SpeexBits decBits;
	
	SpeexJitter jitter;
	int fake_timestamp;
} QSpeexCodec;


void qSpeexDestroyHandle(QSpeexCodecPtr handle)
{
	if (!handle) return;
	
	speex_encoder_destroy(handle->encState);
	speex_decoder_destroy(handle->decState);
	
	speex_bits_destroy(&handle->encBits);
	speex_bits_destroy(&handle->decBits);
	
	speex_jitter_destroy(&handle->jitter);
	
	free(handle);
}


QSpeexCodecPtr qSpeexCreateHandle(void)
{
	QSpeexCodecPtr handle = (QSpeexCodecPtr) malloc(sizeof(QSpeexCodec));
	if (!handle) return handle;
	
	handle->frameSize = 320;  // assume wideband
	
	// Visual Studio: need to use "speex_lib_get_mode()" instead of 
	// referring directly to "speex_wb_mode"
	handle->encState = speex_encoder_init(speex_lib_get_mode(SPEEX_MODEID_WB));
	handle->decState = speex_decoder_init(speex_lib_get_mode(SPEEX_MODEID_WB));
	
	speex_bits_init(&handle->encBits);
	speex_bits_init(&handle->decBits);
	
	// The sampling rate isn't used, but we add it since the example code does (speexclient.c)
	speex_jitter_init(&handle->jitter, handle->decState, SAMPLING_RATE);
	handle->fake_timestamp = 0;
	return handle;
}


int qSpeexEncode(QSpeexCodecPtr handle, void* samples, int sampleSize)
{
	int offset;

	speex_bits_reset(&handle->encBits);

	/** Floods the console **/
	/**   fprintf(stderr, "encoding bytes: \n");	**/
	for(offset=0; offset<sampleSize; offset+=handle->frameSize) {
		short* ptr = ((short*)samples) + offset;
		speex_encode_int(handle->encState, ptr, &handle->encBits);
	}
	speex_bits_insert_terminator(&handle->encBits);
	return speex_bits_nbytes(&handle->encBits);
}


void qSpeexEncodeRead(QSpeexCodecPtr handle, void* outputBytes, int outputSize)
{
	speex_bits_write(&handle->encBits, outputBytes, outputSize);
}


int qSpeexDecode(QSpeexCodecPtr handle, void* inputBytes, int inputSize, void* outputSamples, int outputSize)
{
	int offset, remaining;
	short *out = (short*)outputSamples;
	
	/* If there is no input to read, we certainly can't read it. */
	if (inputBytes != NULL) {
		speex_bits_read_from(&handle->decBits, inputBytes, inputSize);
	}
	
	for (offset=0; offset<outputSize; offset+=handle->frameSize) {
		if (inputBytes != NULL) {
			if (!speex_bits_remaining(&handle->decBits)) {
				// Ran out of input data
				return 2;
			}
			speex_decode_int(handle->decState, &handle->decBits, out+offset);
		}
		else {
			/* Extrapolate output-buffer based on current decoder state. */
			speex_decode_int(handle->decState, NULL, out+offset);		
		}
	}
	remaining = speex_bits_remaining(&handle->decBits);
	if (remaining >= 8) {
		/* If less than a byte is left over, that's OK. */
		fprintf(stderr, "qSpeexDecode(): %d bits left over\n", remaining);
		return 1; // Still have encoded bits left over
	}
	else return 0; // A-OK!!
}


int qSpeexDecodeToTestJB(QSpeexCodecPtr handle, void* inputBytes, int inputSize, void* outputSamples, int outputSize)
{
	short *out = (short*)outputSamples;

	int incr = handle->frameSize;
	
	speex_jitter_put(&handle->jitter, inputBytes, inputSize, handle->fake_timestamp, NULL);
	handle->fake_timestamp += incr;
	speex_jitter_get(&handle->jitter, out, 0, NULL);
	
	return 0;
}




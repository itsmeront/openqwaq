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
 * qVideoDecoder.cpp
 * QVideoCodecPluginMC
 *
 * Implements API-specific (in this case, MainConcept-specific) parts
 * of the plugins.
 */

// Must declare the decoder struct... may contain API-specific fields. 
#include "../QVideoCodecPlugin/qVideoDecoder.h"
#include "qVideoMainConcept.h"
#include "mcfourcc.h"
#include "mcdefs.h"

#include "../QwaqLib/qBuffer.h"
#include "../QwaqLib/qSharedQueue.h"
#include "../QwaqLib/qLogger.hpp"

using namespace Qwaq;

typedef struct QDecoder
{
	int decoderIndex;

	int semaIndex;
	int outFrameCount;
	int inFrameCount;
	int width;
	int height;
	bufstream_tt *vs;
	SEQ_Params *pSeq;
	frame_tt outframe;
	
	BufferPool2 pool;
	SharedQueue<BufferPtr> queue;

// DXVA 2.0 hardware-accelerated decoding on Windows XP and Windows 7
// XXXXX: disabled for now because of crashiness.
#ifdef WIN32
// #define DXVA2
// dxva2_config_t dxva2;
#endif

} QDecoder;


// Use pragma so that the corresponding Squeak FFI class doesn't need to add padding-fields
#pragma pack(push, 1)
typedef struct QDecodedFrameMetadata
{
	int version;
	// Sequence numbers are assigned in order of the chunks/frames pushed-int/received-from
	// the decoder, nothing intrinsic to the H.264 stream itself.
	int inputSequenceNumber;
	int outputSequenceNumber;
	struct PIC_Params mcPicParams;  //internal MC/H.264 data... ISO/IEC 13818-2 section 6.2.3 and 6.2.3.1
	int sanity;  // must be 0x0012abcd
} QFrameMetadata;
#pragma pack(pop)

// Now that we've defined 'struct QDecoder', we can include 'qVideoDecoder.inc' 
extern "C"
{
#include "../QVideoCodecPlugin/qVideoDecoder.inc"
}

// Now we must define the API-specific functions declared in the .inc file 

int qCreateDecoderAPI(QDecoder **dptr, char* args, int argsSize, int semaIndex, int width, int height)
{
	QDecoder* decoder;			// the decompressor to be initialized
	QVideoArgs* decoderArgs = (QVideoArgs*)args;
	// Read flags from args.
	bool useFrameReordering = false;
	if (argsSize > 32) { // otherwise, we're dealing with old image code that doesn't specify flags
		useFrameReordering = (((decoderArgs->flags)[3] & 0x1) > 0);
	}
	
	int err;

	qLog() << "qCreateDecoderMC: START DECODER CREATION!! (width: " << width << " height: " << height << ")";

	decoder = new QDecoder;
	if (decoder == NULL) {
		qLog() << "qCreateDecoderMC: failed to instantiate decoder";
		return -2; 
	}

#ifdef DXVA2
	// Initialize DXVA info
	decoder->dxva2.d3d_device_manager = NULL;
	decoder->dxva2.num_allocated_surfaces = 0;
#endif

	// XXXXX: for now, we ignore codec type (assume always H.264)
	//decoder->codecType = *((CodecType*)(decoderArgs->codecType));
	//err = GetCodecInfo(&codecInfo, decoder->codecType, NULL);
	//if (err != noErr) {
	//	qerr << endl << "qCreateDecoderQT: cannot get codec info");
	//	qerr << endl << "\tQUICKTIME ERROR CODE: %d", err);
	//	free(decoder);
	//	return -5;
	//}

	decoder->semaIndex = semaIndex;
	decoder->width = width;
	decoder->height = height;
	decoder->inFrameCount = decoder->outFrameCount = 0;
	decoder->pSeq = NULL; // will be read from the data stream

	decoder->vs = open_h264in_Video_stream_ex(qencoder_get_rc, 0, 0);
	if (decoder->vs == NULL) {
		qLog() << "qCreateDecoderMC: failed to open h264 decode stream";
		free(decoder);
		return -5;
	}

	err = decoder->vs->auxinfo(decoder->vs, 0, PARSE_INIT, 0, 0);
	if (err) {
		qLog() << "qCreateDecoderMC: auxinfo(PARSE_INIT) failed";
		qDestroyDecoderAPI(decoder);
		return -5;
	}

	// Using SMP_BY_SLICES instead of the default SMP_BY_SLICES appears to reduce
	// decoder latency.
	err = decoder->vs->auxinfo(decoder->vs, SMP_BY_SLICES, SET_SMP_MODE, 0, 0);
	err = decoder->vs->auxinfo(decoder->vs, 2, SET_CPU_NUM, 0, 0);
 
	// Turn on LOW_LATENCY_FLAG, and possibly INTERN_REORDERING_FLAG.  Beware, the latter adds
	// latency, even to non-reordered input.
	uint32_t decoderParseOptions = LOW_LATENCY_FLAG;
	if (useFrameReordering) {
		qLog() << "qCreateDecoderMC: setting INTERN_REORDERING_FLAG";
		decoderParseOptions |= INTERN_REORDERING_FLAG;
	}
	err = decoder->vs->auxinfo(decoder->vs, decoderParseOptions, PARSE_OPTIONS, 0, 0);
	if (err) {
		qLog() << "qCreateDecoderMC: auxinfo(LOW_LATENCY_FLAG,PARSE_OPTIONS) failed";
		qDestroyDecoderAPI(decoder);
		return -5;
	}

	err = decoder->vs->auxinfo(decoder->vs, 0, PARSE_SEQ_HDR, NULL, 0);
	if (err) {
		qLog() << "qCreateDecoderMC: auxinfo(PARSE_SEQ_HDR) failed";
		qDestroyDecoderAPI(decoder);
		return -5;
	}

	// Setup frame info structure
	memset (&(decoder->outframe), 0, sizeof(decoder->outframe));
	decoder->outframe.four_cc   = FOURCC_BGR4;
	
	qLog() << "qCreateDecoderMC: INITIALIZED STRUCTURE";
	qLog() << "qCreateDecoderMC: FINISHED!!!";
	
	*dptr = decoder;
	return 0;
}


void qDestroyDecoderAPI(QDecoder *decoder)
{
	if (decoder == NULL) return;  // even though we won't be called if there is no decoder to destroy...
	decoder->vs->done(decoder->vs, 0);
	decoder->vs->free(decoder->vs);
	delete decoder;
}


int tryToReadSequenceHeader(QDecoder* decoder, char* bytes, int byteCount) {
	int bytesProcessed = 0;
	bufstream_tt *vs = decoder->vs;
	int err;
	int pSeqWidth, pSeqHeight;

	// If we already have one, don't try to read another one.
	if (decoder->pSeq != NULL) {
		qLog() << "tryToReadSequenceHeader(" << decoder->decoderIndex << "): already have a sequence header";
		return 0;
	}

	bytesProcessed += vs->copybytes(vs, (uint8_t*)(bytes+bytesProcessed), byteCount-bytesProcessed);
	if (bytesProcessed == 0) {
		qLog() << "tryToReadSequenceHeader(" << decoder->decoderIndex << "): patiently waiting for a sequence header";
		return 0;
	}
	int state = vs->auxinfo(vs, 0, GET_PARSE_STATE, 0, 0);
	if (! (state&SEQ_HDR_FLAG)) {
		qLog() << "tryToReadSequenceHeader(" << decoder->decoderIndex << "): patiently waiting for a sequence header (some bytes processed)";
		return bytesProcessed;
	}

#ifdef DXVA2
	qerr << endl << "tryToReadSequenceHeader(" << decoder->decoderIndex << "): attempting to set up DXVA hardware-accelerated decoding...  " << flush;
	err = vs->auxinfo(vs, ACC_MODE_DXVA2, HWACC_SET_ACC_MODE, &(decoder->dxva2), sizeof(dxva2_config_t));
	if (err) qerr << "failed.";
	else qerr << "success!";
#endif

	//Yeah, we found a sequence header!
	err = vs->auxinfo(vs, 0, GET_SEQ_PARAMSP, &(decoder->pSeq), sizeof(decoder->pSeq));
	if (err) {
		// What went wrong?  We should have been able to read the sequence header!?!
		qerr << endl << "tryToReadSequenceHeader(" << decoder->decoderIndex << "): failed to read seq.hdr.";
		return bytesProcessed; // XXXXX: what should the error return value be?
	}
	pSeqWidth = decoder->pSeq->horizontal_size;
	pSeqHeight = decoder->pSeq->vertical_size;
	if (pSeqWidth == 0 && pSeqHeight == 0) {
		// This isn't a "full" sequence header (I'm not quite sure what that means).
		// Wait paitently for the next one.
		qerr << endl << "tryToReadSequenceHeader(" << decoder->decoderIndex << "): read a sequence header, but not a 'full' one";
		return bytesProcessed;
	}
	if (pSeqWidth != decoder->width || pSeqHeight != decoder->height) {
		qLog() << "tryToReadSequenceHeader(" << decoder->decoderIndex << "): unexpected dimensions in sequence header";
		qerr << endl << "                             expected: " << decoder->width << " x " << decoder->height;
		qerr << endl << "                             read: " << decoder->pSeq->horizontal_size << " x " << decoder->pSeq->vertical_size;
		qerr << endl << "                             clearing pSeq to try again (pointer was: " << (unsigned int)(decoder->pSeq) << ")";
		decoder->pSeq = NULL;
		return bytesProcessed; // XXXXX: what should the error return value be?
	}

	// Initialize frame parser
	err = vs->auxinfo(vs, 0, INIT_FRAME_PARSER, 0, 0);
	if (err) {
		qLog() << "tryToReadSequenceHeader(" << decoder->decoderIndex << "): error initializing frame parser";
		return bytesProcessed; // XXXXX: what should the error return value be?
	}

	// Setup frame info structure
	decoder->outframe.width = pSeqWidth;
	decoder->outframe.height = pSeqHeight;

	err = vs->auxinfo(vs, SKIP_NONE, PARSE_FRAMES, NULL, 0);
	if (err) {
		qLog() << "tryToReadSequenceHeader(" << decoder->decoderIndex << "): error initializing frame options";
		return bytesProcessed; // XXXXX: what should the error return value be?
	}

	qLog() << "tryToReadSequenceHeader(" << decoder->decoderIndex << "): read sequence header (extent: " << pSeqWidth << "x" << pSeqHeight << ")";
	return bytesProcessed;
}

int qDecodeAPI(QDecoder *decoder, char* bytes, int byteCount)
{
	int bytesProcessed = 0;
	int state;
	bufstream_tt *vs = decoder->vs;

	if (byteCount == 0) {
		// Flush the decoder.
		qLog() << "qDecodeMC(" << decoder->decoderIndex << "): flushing the decoder.";
		vs->copybytes(vs, 0, 0);
	}

	if (decoder->pSeq == NULL) {
		bytesProcessed += tryToReadSequenceHeader(decoder, bytes+bytesProcessed, byteCount-bytesProcessed);
		if (decoder->pSeq == NULL) return 0;
	}

	// Set the frame-count, so that we can retrieve it when the frame is decoded.
	vs->auxinfo(vs, 0, SET_USER_DATA, &(decoder->inFrameCount), sizeof(int));

	if (bytesProcessed != byteCount) {
		// Give the decoder as many bytes as it wants to take
		bytesProcessed += vs->copybytes(vs, (uint8_t*)(bytes+bytesProcessed), byteCount-bytesProcessed);	
	}

	// See what state the decoder is in after the bytes we just pushed into it.
	state = vs->auxinfo(vs, 0, CLEAN_PARSE_STATE, 0, 0);

	// Parse all of the data that we can.
	while (state & PARSE_DONE_FLAG) {
		int frameAvailable;
		int frameError;
		
		if (state & SEQ_HDR_FLAG) {
			vs->auxinfo(vs, 0, GET_SEQ_PARAMSP, &(decoder->pSeq), sizeof(decoder->pSeq));
			if (decoder->pSeq->horizontal_size != decoder->width || decoder->pSeq->vertical_size != decoder->height) {
				qLog() << "qDecodeMC(" << decoder->decoderIndex << "): new sequence header dimensions changed";
				return -1; // XXXXX: what should the error return value be?
			}
		}
		
		// Check if a frame is available to be read.
		frameAvailable = (state & PIC_VALID_FLAG) && (state & PIC_FULL_FLAG);
		frameError = (state & PIC_ERROR_FLAG) || (state & PIC_MV_ERROR_FLAG);

		if (frameAvailable) {

			// Obtain a buffer that will contain both the decoded frame, and its metadata
			BufferPtr output = decoder->pool.getBuffer(sizeof(QDecodedFrameMetadata) + decoder->width * decoder->height * 4);
			if (output == NULL) {
				// This will likely result in the stream being screwed up.  But, I don't see what the 
				// alternative is (I'm assuming that we need to do the auxinfo(GET_PIC) in order for
				// parsing to progress).
				qLog() << "qDecodeMC(" << decoder->decoderIndex << "): failed to obtain output buffer";
				return -1; // XXXXX: what should the error return value be?
			}
			
			// Fill in the metadata;
			QDecodedFrameMetadata *metadata = (QDecodedFrameMetadata*)output->getPointer();
			metadata->version = 1;
			// don't have a fail-safe way to obtain sequence-numbers here... 
			// see end of function, where qDecodeAPI() is called recursively
			metadata->inputSequenceNumber = metadata->outputSequenceNumber = 0; 
			vs->auxinfo(vs, 0, GET_PIC_PARAMSP, &(metadata->mcPicParams), sizeof(struct PIC_Params));
			metadata->sanity = 0x0012abcd;
			// Retrieve user-data that we set when pushing data into the decoder (stash it in the metadata).
			int *userDataPtr;
			vs->auxinfo(vs, 0, GET_USER_DATAP, &userDataPtr, sizeof(int**));
			metadata->inputSequenceNumber = *userDataPtr;
			decoder->outFrameCount = metadata->inputSequenceNumber;  // we now know which frame we just decoded
				
		
			// If there was an error, don't consume the frame, nor signal the semaphore. 
			if (frameError) {
				qerr << endl << (char*) ((state & PIC_ERROR_FLAG) ?  "Decode PIC_ERR" : "Decode PIC_MV_ERR");
			} else {
				// Obtain and stash the decoded frame.
				unsigned char* pixelPtr = (unsigned char*)output->getPointer() + sizeof(QDecodedFrameMetadata);
#ifdef UPSIDE_DOWN
				decoder->outframe.stride[0] = -4 * decoder->width;
				decoder->outframe.plane[0] = pixelPtr + (decoder->width * 4)*(decoder->height - 1);
#else
				decoder->outframe.stride[0] = 4 * decoder->width;
				decoder->outframe.plane[0] = pixelPtr;
#endif
				vs->auxinfo(vs, DECODE_FULL_SIZE , GET_PIC, &(decoder->outframe), sizeof(decoder->outframe));
				decoder->queue.add(output);
				interpreterProxy->signalSemaphoreWithIndex(decoder->semaIndex);
			}
		}
	
		//See if there is anything else to do!
		if (bytesProcessed != byteCount) {
			size_t used = vs->copybytes (vs, (uint8_t*)(bytes+bytesProcessed), byteCount-bytesProcessed);
			if (used == 0) {
				qLog() << "qDecodeMC(" << decoder->decoderIndex << "): Copy-bytes consumed nothing!";
			}
			bytesProcessed += used;
		}
		state = vs->auxinfo(vs, 0, CLEAN_PARSE_STATE, 0, 0);
	}

	// These days, we seem to consume all of the data that we push in 
	// (the only time we didn't was with an early version of the Mac SDK).
	// So, now we treat this as an anomolous event, and log it.  This facilitates 
	// some other things that we want to do (like keeping track of seq. numbers).
 	if (bytesProcessed != byteCount) {
		// Oops, that's weird 
		qLog() << "qDecodeMC(" << decoder->decoderIndex << "): processed " << bytesProcessed << " out of " << byteCount << " bytes!?!?!?";
		return -2;
	}

	return 0;  // A-OK!
}


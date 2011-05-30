/*
 * Project OpenQwaq
 *
 * Copyright (c) 2011, 3d Immersive Collaboration Consultants, LLC, All Rights Reserved
 *
 * qVideoDecoder.cpp
 * QVideoCodecPluginFree
 *
 * Created by Josh Gargus on 5/29/11.
 */
 
 // Must declare the decoder struct... may contain API-specific fields. 
#include "../QVideoCodecPlugin/qVideoDecoder.h"

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
	
	BufferPool2 pool;
	SharedQueue<BufferPtr> queue;
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
//	struct PIC_Params mcPicParams;  //internal MC/H.264 data... ISO/IEC 13818-2 section 6.2.3 and 6.2.3.1
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
	
	qLog() << "qCreateDecoderFree: START DECODER CREATION!! (width: " << width << " height: " << height << ")";
	
	decoder = new QDecoder;
	if (decoder == NULL) {
		qLog() << "qCreateDecoderFree: failed to instantiate decoder";
		return -2; 
	}
	
	qerr << endl << "qCreateDecoderFree(): not yet implemented";
	qDestroyDecoderAPI(decoder);
	return -1;
}	


void qDestroyDecoderAPI(QDecoder *decoder)
{
	if (decoder == NULL) return;  // ... even though we won't be called if there is no decoder to destroy.
	delete decoder;
}

int qDecodeAPI(QDecoder *decoder, char* bytes, int byteCount)
{
	return 0;  // A-OK!
}



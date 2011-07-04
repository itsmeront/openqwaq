/*
 * Project OpenQwaq
 *
 * Copyright (c) 2011, 3d Immersive Collaboration Consultants, LLC, All Rights Reserved
 *
 * qVideoDecoder.cpp
 * QVideoCodecPluginFree
 *
 * Created by Josh Gargus on 6/11/11.
 *
 *
 
 TODO:
 - seeking: see #flushDecoder
   - probably use avcodec_flush_buffers
 - reordering:

 *
 */

/* Must declare the decoder struct... may contain API-specific fields. */ 
#include "../QVideoCodecPlugin/qVideoDecoder.h"

#include "../QwaqLib/qBuffer.h"
#include "../QwaqLib/qSharedQueue.h"
#include "../QwaqLib/qLogger.hpp"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
}

using namespace Qwaq;


typedef struct QDecoder { 
	int decoderIndex;
	
	int semaIndex;
	int outFrameCount;
	int inFrameCount;
	int width;
	int height;
	
	AVCodec *codec;
	AVCodecContext *ctxt;
	AVCodecParserContext *parser;
	AVFrame *pic_yuv;
	AVFrame *pic_bgra;
	
	struct SwsContext *scaler;
	
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
	// XXXXX I'm not setting the sanity value yet.
	int sanity;  // must be 0x0012abcd
} QFrameMetadata;
#pragma pack(pop)

/* Now that we've defined 'struct QDecoder', we can include 'qVideoDecoder.inc' */
extern "C"
{
#include "../QVideoCodecPlugin/qVideoDecoder.inc"
}

/* Now we must define the API-specific functions declared in the .inc file */

int qCreateDecoderAPI(QDecoder **dptr, char *args, int argsSize, int semaIndex, int width, int height) 
{
	QDecoder *decoder;
	QVideoArgs *vargs = (QVideoArgs*)args;
	
	qerr << endl << "qCreateDecoderFree(): START DECODER CREATION! (width: " << width << " height: " << height << ")";
	
	decoder = new QDecoder;
	if (decoder == NULL) {
		qerr << endl << "qCreateDecoderFree(): failed to instantiate decoder";
		return -2;
	}
	
	decoder->semaIndex = semaIndex;
	decoder->width = width;
	decoder->height = height;
	decoder->inFrameCount = decoder->outFrameCount = 0;
	decoder->codec = NULL;
	decoder->ctxt = NULL;
	decoder->parser = NULL;
	decoder->pic_yuv = NULL;	
	decoder->pic_bgra = NULL;
	decoder->scaler = NULL;
	
	// Allocate a YUV frame to decode the H.264 frame into, and a
	// BGRA frame to return back to Squeak.
	decoder->pic_yuv = avcodec_alloc_frame();
	decoder->pic_bgra = avcodec_alloc_frame();	
	if (!decoder->pic_yuv || !decoder->pic_bgra) {
		qerr << endl << "qCreateDecoderFree(): cannot allocate AVFrame";
		qDestroyDecoderAPI(decoder);
		return -2;	
	}

	decoder->codec = avcodec_find_decoder(CODEC_ID_H264);
	if (!decoder->codec) {
		qerr << endl << "qCreateDecoderFree(): cannot find H.264 decoder";
		qDestroyDecoderAPI(decoder);
		return -2;
	}
		
	decoder->ctxt = avcodec_alloc_context();
	if (!decoder->ctxt) {
		qerr << endl << "qCreateDecoderFree(): cannot allocate context";
		qDestroyDecoderAPI(decoder);
		return -2;
	}
	
	if (avcodec_open(decoder->ctxt, decoder->codec) < 0) {
		qerr << endl << "qCreateDecoderFree(): could not open codec";
		qDestroyDecoderAPI(decoder);
		return -2;		
    }
	
	decoder->parser = av_parser_init(CODEC_ID_H264);
	if (!decoder->parser) {
		qerr << endl << "qCreateDecoderFree(): cannot create parser";
		qDestroyDecoderAPI(decoder);
		return -2;
	}
	decoder->parser->flags |= PARSER_FLAG_ONCE;

	decoder->scaler = sws_getContext(width, height, PIX_FMT_YUV420P, width, height, PIX_FMT_BGRA, SWS_POINT, NULL, NULL, NULL);
	if (!decoder->scaler) {
		qerr << endl << "qCreateDecoderFree(): cannot allocate scaler";		
		qDestroyDecoderAPI(decoder);
		return -1;
	}
	
	qerr << endl << "qCreateDecoderFree(): opened codec!!!"; 
	*dptr = decoder;
	return 0; // success!
}


void qDestroyDecoderAPI(QDecoder *decoder)
{
	if (decoder == NULL) return;  // ... even though we won't be called if there is no decoder to destroy.

	if (decoder->ctxt) {
		avcodec_close(decoder->ctxt);
		av_free(decoder->ctxt);
		decoder->ctxt = NULL;
	}
	
	if (decoder->parser) {
		av_parser_close(decoder->parser);
		decoder->parser = NULL;
	}
	
	if (decoder->pic_bgra) {
		av_free(decoder->pic_bgra);
		decoder->pic_bgra = NULL;
	}
	if (decoder->pic_yuv) {
		av_free(decoder->pic_yuv);
		decoder->pic_yuv = NULL;
	}
	
	if (decoder->scaler) {
		sws_freeContext(decoder->scaler);
		decoder->scaler = NULL;
	}
	
	decoder->codec = NULL;
	delete decoder;
}


int qDecodeAPI(QDecoder *decoder, char* bytes, int byteSize) 
{
	
	uint8_t parsedData[50000];
	int parsedDataLength = 500000;
	av_parser_parse2(
		decoder->parser, 
		decoder->ctxt, 
		(uint8_t**)&parsedData, 
		&parsedDataLength, 
		(const uint8_t*)bytes, 
		byteSize, 
		0, 
		0, 
		AV_NOPTS_VALUE
	);
	
//	qerr << endl << "PARSED DATA LENGTH: " << parsedDataLength <<  flush;
//	if (!parsedDataLength) { return 0; }
	
	int gotPicture;
    AVPacket packet;
	
	av_init_packet(&packet);
//	packet.data = parsedData;
//	packet.size = parsedDataLength;
    packet.data = (uint8_t*)bytes;
    packet.size = byteSize;
	
	int err = avcodec_decode_video2(decoder->ctxt, decoder->pic_yuv, &gotPicture, &packet);
	if (err < 0) { 
		// XXXXX better formatting on error message
		qerr << endl << "DECODER ERROR: " << err << flush; 
		return -1;
	}
	else if (gotPicture) {
		// XXXXX might want to provide picture-type in metadata returned to Squeak.
//		qerr << endl << "DECODED PICTURE OF TYPE: " << av_get_picture_type_char(decoder->pic_yuv->pict_type);
		
		BufferPtr output = decoder->pool.getBuffer(decoder->width * decoder->height * 4);
		uint8_t *ptr = (uint8_t*)output->getPointer();
		int stride = decoder->width * 4;
		sws_scale(decoder->scaler, decoder->pic_yuv->data, decoder->pic_yuv->linesize, 0, decoder->height, &ptr, &stride);
		decoder->queue.add(output);
		interpreterProxy->signalSemaphoreWithIndex(decoder->semaIndex);
	}	

	return 0;
}


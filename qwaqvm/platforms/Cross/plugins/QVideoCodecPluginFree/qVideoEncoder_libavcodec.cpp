/*
 * Project OpenQwaq
 *
 * Copyright (c) 2011, 3d Immersive Collaboration Consultants, LLC, All Rights Reserved
 *
 * qVideoEncoder_libavcodec.cpp
 * QVideoCodecPluginFree
 *
 * Created by Josh Gargus on 5/29/11.
 *
 * Not currently used... instead we use x264 directly, since it gives us better control
 * over low-latency parameters for videoconferencing.
 */

/* Must declare the encoder struct... may contain API-specific fields. */ 
#include "../QVideoCodecPlugin/qVideoEncoder.h"

#include "../QwaqLib/qBuffer.h"
#include "../QwaqLib/qSharedQueue.h"
#include "../QwaqLib/qLogger.hpp"

extern "C" {
#include "libavcodec/avcodec.h"
}

using namespace Qwaq;


typedef struct QEncoder { 
	int encoderIndex;
	
	int semaIndex;
	int outFrameCount;
	int inFrameCount;
	int width;
	int height;
	
	AVCodec *codec;
	AVCodecContext *ctxt;
	AVFrame *pic;
	
	BufferPool2 pool;
	SharedQueue<BufferPtr> queue;
} QEncoder;

/* Now that we've defined 'struct QEncoder', we can include 'qVideoEncoder.inc' */
extern "C"
{
#include "../QVideoCodecPlugin/qVideoEncoder.inc"
}

/* Now we must define the API-specific functions declared in the .inc file */

int qCreateEncoderAPI(QEncoder **eptr, char *args, int argsSize, int semaIndex, int width, int height) 
{
	QEncoder *encoder;
	QVideoArgs *vargs = (QVideoArgs*)args;
	
	qerr << endl << "qCreateEncoderFree(): START ENCODER CREATION! (width: " << width << " height: " << height << ")";
	
	encoder = new QEncoder;
	if (encoder == NULL) {
		qerr << endl << "qCreateEncoderFree(): failed to instantiate encoder";
		return -2;
	}
	
	encoder->semaIndex = semaIndex;
	encoder->width = width;
	encoder->height = height;
	encoder->inFrameCount = encoder->outFrameCount = 0;
	encoder->codec = NULL;
	encoder->ctxt = NULL;
	encoder->pic = NULL;
	
	encoder->pic = avcodec_alloc_frame();
	if (!encoder->pic) {
		qerr << endl << "qCreateEncoderFree(): cannot allocate AVFrame";
		qDestroyEncoderAPI(encoder);
		return -2;	
	}

	encoder->codec = avcodec_find_encoder(CODEC_ID_H264);
//	encoder->codec = avcodec_find_encoder(CODEC_ID_MPEG1VIDEO);
	if (!encoder->codec) {
		qerr << endl << "qCreateEncoderFree(): cannot find H.264 encoder";
		qDestroyEncoderAPI(encoder);
		return -2;
	}
	
	encoder->ctxt = avcodec_alloc_context();
	if (!encoder->ctxt) {
		qerr << endl << "qCreateEncoderFree(): cannot allocate context";
		qDestroyEncoderAPI(encoder);
		return -2;
	}
	AVCodecContext *c = encoder->ctxt;
	avcodec_get_context_defaults3(c, encoder->codec);
	
	c->width = width;
	c->height = height;
	c->bit_rate = vargs->bitRate;
	c->codec_id = CODEC_ID_H264;    
	c->codec_type = AVMEDIA_TYPE_VIDEO; 
	c->pix_fmt = PIX_FMT_ARGB; // ARGB, RGBA, ABGR, BGRA ???!?
	qerr << endl << "WARN qCreateEncoderFree(): hardcode 30fps";

	// Visual Studio isn't happy with this syntax.  I guess a C99 thing?
//	c->time_base = (AVRational){1,30};
	c->time_base.num = 1;
	c->time_base.den = 30;

	qerr << endl << "qCreateEncoderFree(): about to set context parameters";
	
	// libx264-medium.ffpreset preset
	c->coder_type = 1;  // coder = 1
	c->flags|=CODEC_FLAG_LOOP_FILTER;   // flags=+loop
	c->me_cmp|= 1;  // cmp=+chroma, where CHROMA = 1
	c->partitions|=X264_PART_I8X8+X264_PART_I4X4+X264_PART_P8X8+X264_PART_B8X8; // partitions=+parti8x8+parti4x4+partp8x8+partb8x8
	c->me_method=ME_HEX;    // me_method=hex
	c->me_subpel_quality = 7;   // subq=7
	c->me_range = 16;   // me_range=16
	c->gop_size = 250;  // g=250
	c->keyint_min = 25; // keyint_min=25
	c->scenechange_threshold = 40;  // sc_threshold=40
	c->i_quant_factor = 0.71; // i_qfactor=0.71
	c->b_frame_strategy = 1;  // b_strategy=1
	c->qcompress = 0.6; // qcomp=0.6
	c->qmin = 10;   // qmin=10
	c->qmax = 51;   // qmax=51
	c->max_qdiff = 4;   // qdiff=4
	c->max_b_frames = 3;    // bf=3
	c->refs = 3;    // refs=3
	c->directpred = 1;  // directpred=1
	c->trellis = 1; // trellis=1
	c->flags2|=CODEC_FLAG2_BPYRAMID+CODEC_FLAG2_MIXED_REFS+CODEC_FLAG2_WPRED+CODEC_FLAG2_8X8DCT+CODEC_FLAG2_FASTPSKIP;  // flags2=+bpyramid+mixed_refs+wpred+dct8x8+fastpskip
	c->weighted_p_pred = 2; // wpredp=2
	
	qerr << endl << "qCreateEncoderFree(): about to open codec" << flush;
	int errCode = avcodec_open(c, encoder->codec);
	if (errCode < 0) {
		char errbuf[128];
		const char *errbuf_ptr = errbuf;
		if (av_strerror(errCode, errbuf, sizeof(errbuf)) < 0) { errbuf_ptr = strerror(AVUNERROR(errCode)); }
	
		// Not sure if it's safe to close an unopened codec, so free/clear it here.
		qerr << endl << "qCreateEncoderFree(): cannot open codec-context: " << errbuf << " (" << errCode << ")";
		av_free(c);
		encoder->ctxt = NULL;
		qDestroyEncoderAPI(encoder);
		return -1;
	}
	qerr << endl << "qCreateEncoderFree(): opened codec!!!"; 
	*eptr = encoder;
	return 0; // success!
}


void qDestroyEncoderAPI(QEncoder *encoder)
{
	if (encoder == NULL) return;  // ... even though we won't be called if there is no encoder to destroy.

	if (encoder->ctxt) {
		avcodec_close(encoder->ctxt);
		av_free(encoder->ctxt);
		encoder->ctxt = NULL;
	}
	
	if (encoder->pic) {
		av_free(encoder->pic);
		encoder->pic = NULL;
	}
	
	encoder->codec = NULL;
	delete encoder;
}


int qEncodeAPI(QEncoder *encoder, char* bytes, int byteSize) 
{
	qerr << endl << "qEncodeAPI(): not yet implemented";
	return -1;
}


char* qEncoderGetPropertyAPI(QEncoder *encoder, char* propertyName, int* resultSize)
{
	qerr << endl << "qEncoderGetPropertyAPI(): no properties accessible yet";
	*resultSize = 0;
	return NULL;
}
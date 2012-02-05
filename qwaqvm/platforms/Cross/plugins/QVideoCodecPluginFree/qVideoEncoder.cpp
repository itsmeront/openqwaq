/*
 * Project OpenQwaq
 *
 * Copyright (c) 2011, 3d Immersive Collaboration Consultants, LLC, All Rights Reserved
 *
 * qVideoEncoder.cpp
 * QVideoCodecPluginFree
 *
 * Created by Josh Gargus on 5/29/11.
 */

/* Must declare the decoder struct... may contain API-specific fields. */ 
#include "../QVideoCodecPlugin/qVideoEncoder.h"

#include "../QwaqLib/qBuffer.h"
#include "../QwaqLib/qSharedQueue.h"
#include "../QwaqLib/qLogger.hpp"

extern "C" {
#ifdef WIN32
#include "inttypes.h"
#endif
#include "x264.h"
#include "libswscale/swscale.h"
#include "qLibAVLogger.h"
}

using namespace Qwaq;


typedef struct QEncoder { 
	int encoderIndex;
	
	int semaIndex;
	int outFrameCount;
	int inFrameCount;
	int width;
	int height;

	x264_t *x264;
	x264_picture_t pic_in, pic_out;
	unsigned char *pic_rgb24;
	
	struct SwsContext *scaler;

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
	
	// The profile is stored in the 2 least-significant bits of the second flag byte.
	enum H264_PROFILE { ZEROLATENCY=0, BASELINE, MAIN, HIGH };
	H264_PROFILE profile = (H264_PROFILE) (vargs->flags[2] & 3);
	
	encoder->semaIndex = semaIndex;
	encoder->width = width;
	encoder->height = height;
	encoder->inFrameCount = encoder->outFrameCount = 0;
	
	encoder->x264 = NULL;
	encoder->pic_rgb24 = NULL;
	encoder->scaler = NULL;
	
	// Initialize x264 encoder.  We use low-latency settings optimized for video-conferencing,
	// as described by: http://x264dev.multimedia.cx/archives/249
	x264_param_t param; 

	// Initialize params with x264 defaults, which we will then modify.
	x264_param_default_preset(&param, "fast", (profile == ZEROLATENCY) ? "zerolatency" : NULL);

	// ZEROLATENCY is for videoconferencing...
	if (profile == ZEROLATENCY) {
		param.i_slice_max_size = 1380; // XXXXX should fit in QwON datagram-slice... can tune this later
		
		// fancy x264 hotness to avoid large keyframes, instead spreading refreshes out across frames.
		param.b_intra_refresh = 1;
		param.i_frame_reference = 1; // needed for intra-refresh
		
		// send SPS/PPS before every keyframe so that receivers can
		// join in the middle of a stream without performing a session
		// negotiation (eg: SIP) to receive this information.
		param.b_repeat_headers = 1;
		
		// B-frames add too much latency for videoconferencing
		param.i_bframe = 0;
		param.i_bframe_pyramid = 0;
	}
	// ... everything else is for session-recording.
	else {
		// B-frames allow higher compression rates.
		// TODO: enable them (but not until after we fix "lip-syncing")
		param.i_bframe = 0;
		param.i_bframe_pyramid = 0;
	}
		
	param.i_width = width;
	param.i_height = height;
	param.i_fps_num = vargs->frameRate;
	param.i_fps_den = 1;

	param.i_frame_total = 0;  // don't know in advance how many frames.
	param.b_annexb = 1; // expected by both MainConcept and libavcodec decoders
	
	// Pretend that we have a fixed frame rate, 
	// even though we actually throttle the capture
	// rate in Squeak.  x264 does support variable
	// frame-rate modes, but I'm not quite sure how
	// to make it work.
	param.b_vfr_input = 0;
	
	// Don't believe that we pay attention to this... see comments below
	// about quality- vs. bitrated based targets.
	param.rc.i_bitrate = vargs->bitRate;

	// We used to use these, but they don't seem to be needed???
//	param.rc.i_vbv_max_bitrate = vargs->bitRate;
//	param.rc.i_vbv_buffer_size = 30; 

	// Quality.  We use a quality-based target rather than a bitrate
	// target, so it's a bit tricky to choose a value that gives the
	// desired bitrate.  This setting has been empirically selected
	// to look good enough, and be low-bitrate enough.
	param.rc.f_rf_constant = 25;
	param.rc.i_rc_method = X264_RC_CRF;
	
	// Yeah, I know that "zero latency" isn't a "profile" per-se...
	char *profileString;
	switch (profile) {
		case ZEROLATENCY:
			// Treat it as high-profile, since we know that the
			// H.264 decoder that we use for video-chat supports it.
		case HIGH:
			profileString = (char*)"high";
			break;
		case MAIN:
			profileString = (char*)"main";
			break;
		case BASELINE:
			profileString = (char*)"baseline";
			break;
		default:
			qerr << endl << "qCreateEncoderFree(): 100% impossible to reach here";
			profileString = (char*)"baseline";
	}
	x264_param_apply_profile(&param, profileString);

	// Do this first, because we want the image-plane to be set (either to NULL on failure
	// or a ptr on success, we don't care) before calling qDestroyEncoderAPI().
	if (x264_picture_alloc(&encoder->pic_in, X264_CSP_I420, width, height) == -1) {
		qerr << endl << "qCreateEncoderFree(): cannot allocate input-picture";
		qDestroyEncoderAPI(encoder);
		return -1;
	}
	
	// Set up logging, so that X264 log messages go to the same place as the rest of the plugin's.
	param.pf_log = q_x264_log_callback;
//	param.i_log_level = X264_LOG_DEBUG;
	
	encoder->x264 = x264_encoder_open(&param);
	if (!encoder->x264) {
		qerr << endl << "qCreateEncoderFree(): cannot allocate x264 encoder";		
		qDestroyEncoderAPI(encoder);
		return -1;
	}

	encoder->scaler = sws_getContext(width, height, PIX_FMT_BGRA, width, height, PIX_FMT_YUV420P, SWS_POINT, NULL, NULL, NULL);
	if (!encoder->scaler) {
		qerr << endl << "qCreateEncoderFree(): cannot allocate scaler";		
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

	// Because of our order of initialization, we know this is safe to call; see comment in qCreateEncoderAPI().
	x264_picture_clean(&encoder->pic_in);
	
	if (encoder->x264) {
		x264_encoder_close(encoder->x264);
		encoder->x264 = NULL;
	}

	if (encoder->scaler) {
		sws_freeContext(encoder->scaler);
		encoder->scaler = NULL;
	}
	
	delete encoder;
	qerr << endl << "qDestroyEncoderFree(): DESTROYED ENCODER"; 
}


int qEncodeAPI(QEncoder *encoder, char* bytes, int byteSize) 
{
	// Transform to I420 colorspace, as required by x264.
	int stride = encoder->width*4;
		
	sws_scale(encoder->scaler, (const uint8_t* const*)&bytes, &stride, 0, encoder->height, encoder->pic_in.img.plane, encoder->pic_in.img.i_stride);
	
	// XXXXX need to experiment with this setting, both whether to enable it
	// at all, and also the frequency.  Is this only useful for lossy transport?
	// I don't think so... one of the benefits is that it smooths out frame-sizes,
	// so that we don't have large keyframes bunging up the pipeline.
	if (encoder->inFrameCount % 30 == 0) {
		// x264_encoder_intra_refresh(encoder->x264);
	}

	// Encode the frame.
	x264_nal_t *nals;
	int nalCount;
	int frameSize = x264_encoder_encode(encoder->x264, &nals, &nalCount, &encoder->pic_in, &encoder->pic_out);
	
	if (frameSize < 0) {
		qerr << endl << "encode failed with code: " << frameSize;
		return -1;
	}
	else if (frameSize > 0) {
		// Uncomment this if you feel the need for debugging.
//		qerr << endl << "encoded NAL types:  ";
//		for (int i=0; i<nalCount; i++) {
//			qerr << nals[i].i_type << " ";
//		}
//		qerr << flush;
	
		// There is data to return to Squeak!
		BufferPtr output = encoder->pool.getBuffer(frameSize+1);
		unsigned char* outputPtr = output->getPointer();
		// Figure out if this is a keyframe
		outputPtr[0] = encoder->pic_out.b_keyframe ? 1 : 0;
		// The NALs are guaranteed to be sequential in memory, so we start copying from the 
		// payload of the first one.
		memcpy(outputPtr+1, nals[0].p_payload, frameSize);
		// Enqueue the buffer for readback.
		encoder->queue.add(output);
		interpreterProxy->signalSemaphoreWithIndex(encoder->semaIndex);
	}
	return 0; // success!
}


char* qEncoderGetPropertyAPI(QEncoder *encoder, char* propertyName, int* resultSize)
{
	if (!(strcmp(propertyName, "PARAMETER_SETS"))) {
		x264_nal_t *nals;
		int nalCount;
		x264_encoder_headers(encoder->x264, &nals, &nalCount);
		
		// Compute the total size of all NAL units.
		int totalNalSize = 0;
		for (int i=0; i<nalCount; i++) {
			totalNalSize += nals[i].i_payload;
		}
		// Allocate the necessary space.
		char *result = (char*)malloc(totalNalSize);
		if (!result) {	
			*resultSize = 0;
			return NULL;
		}		
		// Since the NAL units are guaranteed to be adjacent in memory,
		// we just need one memcpy.
		memcpy(result, nals[0].p_payload, totalNalSize);
		*resultSize = totalNalSize;
		return result;
	}
	else if (!(strcmp(propertyName, "CAN_I_GET_A_HELL_YEAH"))) {
		char *result = (char*)malloc(13);
		if (!result) {
			*resultSize = 0;
			return NULL;
		}
		*resultSize = 12;  //skip the trailing zero
		strcpy(result, "Hell Yeah!!!");
		return result; 
	}
	else {
		*resultSize = 0;
		return NULL;
	}
}
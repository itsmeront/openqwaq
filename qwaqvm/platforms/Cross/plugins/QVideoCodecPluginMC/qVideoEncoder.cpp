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
 * qVideoEncoder.cpp
 * QVideoCodecPluginMC
 *
 */

/* Must declare the decoder struct... may contain API-specific fields. */
#include "../QVideoCodecPlugin/qVideoEncoder.h"
#include "qVideoMainConcept.h"
#include "buf_single.h"
#include "mcfourcc.h"

#include "../QwaqLib/qBuffer.h"
#include "../QwaqLib/qSharedQueue.h"
#include "../QwaqLib/qLogger.hpp"

using namespace Qwaq;


typedef struct {
	int encoderIndex;

	int semaIndex;
	int outFrameCount;
	int inFrameCount;
	int width;
	int height;

	h264_v_settings_tt settings;
	h264venc_tt *venc;
	bufstream_tt *bs;
	char* outbuf;

	BufferPool2 pool;
	SharedQueue<BufferPtr> queue;

} QEncoder;

/* Now that we've defined 'struct QEncoder', we can include 'qVideoEncoder.inc' */
extern "C"
{
#include "../QVideoCodecPlugin/qVideoEncoder.inc"
}

/* Now we must define the API-specific functions declared in the .inc file */

void encoderCallback(void *sink, uint8_t *buf, int32_t bufSize, uint16_t frameType);

// Callbacks passed to MC-encoder (and MC-decoder) so that all logging goes to one place.
extern "C"
{
	void MC_Error_printf(const char* format_str, ...) 
	{
		qerr << endl << "MC Error: ";
		va_list args;
		va_start(args, format_str);
		qvprintf(format_str, args);
		va_end(args);
	}
	void MC_Progress_printf(const char* format_str, ...) 
	{
		qerr << endl << "MC Progress: ";
		va_list args;
		va_start(args, format_str);
		qvprintf(format_str, args);
		va_end(args);
	}
	void MC_Warn_printf(const char* format_str, ...) 
	{
		qerr << endl << "MC Warn: ";
		va_list args;
		va_start(args, format_str);
		qvprintf(format_str, args);
		va_end(args);
	}
	void MC_Info_printf(const char* format_str, ...) 
	{
		qerr << endl << "MC Info: ";
		va_list args;
		va_start(args, format_str);
		qvprintf(format_str, args);
		va_end(args);
	}
	void MC_Debug_printf(void* userData, unsigned int level, const char* format_str, ...) 
	{	
		// We can set this lower once we get a feel for it.
		if (level < 4) {	
			qerr << endl << "MC Debug (" << level << "): ";
			va_list args;
			va_start(args, format_str);
			qvprintf(format_str, args);
			va_end(args);
		}
	}
	void *qencoder_get_rc(char* name)
	{
		if      (!strcmp(name,"malloc")) return (void*) malloc; 
		else if (!strcmp(name,"free")) return (void*) free;
		else if (!strcmp(name,"err_printf")) return (void*)MC_Error_printf;
		else if (!strcmp(name,"prg_printf")) return (void*)MC_Progress_printf;
		else if (!strcmp(name,"wrn_printf")) return (void*)MC_Warn_printf;
		else if (!strcmp(name,"inf_printf")) return (void*)MC_Info_printf;
		else if (!strcmp(name,"dbg_printf")) return (void*)MC_Debug_printf;
		return NULL;
	}
}


int qCreateEncoderAPI(QEncoder **eptr, char *args, int argsSize, int semaIndex, int width, int height)
{
	QEncoder *encoder;
	QVideoArgs *vargs = (QVideoArgs*)args;
	uint32_t frameSize = width * height * 4;

	qerr << endl << "qCreateEncoderMC(): START ENCODER CREATION! (width: " << width << " height: " << height << ")";

	encoder = new QEncoder;
	if (encoder == NULL) {
		qerr << endl << "qCreateEncoderMC(): failed to instantiate encoder";
		return -2; 
	}

	encoder->semaIndex = semaIndex;
	encoder->width = width;
	encoder->height = height;
	encoder->inFrameCount = encoder->outFrameCount = 0;
	encoder->venc = NULL;
	encoder->bs = NULL;
	encoder->outbuf = NULL;

	encoder->outbuf = (char*)malloc(width*height*4);
	if (encoder->outbuf == NULL) {
		qerr << endl << "qCreateEncoderMC(): failed to malloc output buffer";
		qDestroyEncoderAPI(encoder);
		return -2;
	}
	encoder->bs = 
		open_mem_buf_single(
		reinterpret_cast<uint8_t*>(encoder->outbuf), 
		frameSize, 
		frameSize, 
		0, 
		encoder, 
		encoderCallback
		);
	if (encoder->bs == NULL) {
		qerr << endl << "qCreateEncoderMC(): failed to create bufstream";
		qDestroyEncoderAPI(encoder);
		return -1; // XXXXX: what should the error actually be?  I pulled this out of my butt.
	}

	// Use default video settings.  (BTW: we don't need no steeeeenking PAL)
	// XXXXX: support more "video types".
#if __linux__ && 0 /* attempt to evade h264OutVideoDefaults error. obsolete */
# define VideoDefaults H264_BASELINE
#elif 0 /* old default */
# define VideoDefaults H264_MAIN
#else /* current default */
# define VideoDefaults H264_iPOD_640x480
#endif
	h264OutVideoDefaults(&(encoder->settings), VideoDefaults, 0);

	encoder->settings.def_horizontal_size = width;
	encoder->settings.def_vertical_size   = height;
	encoder->settings.time_scale          = 90000;
	encoder->settings.frame_rate          = vargs->frameRate; 
	encoder->settings.num_units_in_tick   = (int32_t)(((double)encoder->settings.time_scale) / vargs->frameRate);
	
	// Bit-rate must be a multiple of 64.  We give ourselves 20% headroom for max. bit-rate
	encoder->settings.bit_rate = (vargs->bitRate / 64) * 64;
//	encoder->settings.bit_rate_mode = H264_CBR;
	encoder->settings.bit_rate_mode = H264_VBR;
	encoder->settings.max_bit_rate = (int32_t)(encoder->settings.bit_rate * 1.2 / 64) * 64;

	
		//XXXXX: no reordering
	encoder->settings.reordering_delay = 1;  // '1' means no reordering
	encoder->settings.use_b_slices = 0;
	encoder->settings.b_slice_pyramid = 0;
	encoder->settings.b_slice_reference = 0;

	//encoder->settings.enable_fast_inter_decisions;
	//encoder->settings.enable_fast_intra_decisions;
		
	// Performance settings... hard-code this for live encoding
	encoder->settings.live_mode = 1;
	encoder->settings.buffering = 1;
	encoder->settings.num_threads = 0;
	h264OutVideoPerformance(	&(encoder->settings), 
								0,  // as of SDK 8.2, this is no longer used 
								9,  // CPU use setting goes from 0-15... pick something in the middle
								0 );

//	h264OutVideoChkSettings(NULL, &(encoder->settings), H264_CHECK_AND_ADJUST | H264_CHECK_FOR_LEVEL, NULL);
	
	const char* bitRateModeNames[] = {"H264_CBR", "H264_CQT", "H264_VBR", "H264_TQM"};
	const char* streamTypeNames[] = {"H264_STREAM_TYPE_I", "H264_STREAM_TYPE_I_SEI", "H264_STREAM_TYPE_II"};
	
	qerr 
		<< endl << "QCreateEncoderMC(): encoder parameters: "
		<< endl << "     set num_units_in_tick to " << encoder->settings.num_units_in_tick
		<< endl << "     time_scale: " << encoder->settings.time_scale
		<< endl << "     frame rate: " << vargs->frameRate
		<< endl << "     bitrates(input / adjusted average / adjusted maximum):   "  << vargs->bitRate << " / " << encoder->settings.bit_rate << " / " << encoder->settings.max_bit_rate
		<< endl << "     bitrate mode: " <<  (bitRateModeNames[encoder->settings.bit_rate_mode])
		<< endl << "     width " << vargs->width << "    height " << vargs->height
		<< endl << "     stream type: " << encoder->settings.stream_type << " " << (streamTypeNames[encoder->settings.stream_type])
		<< endl << "     write_au_delimiters: " << encoder->settings.write_au_delimiters
		<< endl << "     write_seq_end_code: " << encoder->settings.write_seq_end_code
		<< endl << "     write_timestamps: " << encoder->settings.write_timestamps
		<< endl << "     write_single_sei_per_nalu: " << encoder->settings.write_single_sei_per_nalu
		<< endl << "     write_seq_par_set: " << encoder->settings.write_seq_par_set
		<< endl << "     write_pic_par_set: " << encoder->settings.write_pic_par_set
		<< endl << "     frame_mbs_mode: " << encoder->settings.frame_mbs_mode
		<< endl << "     vui_presentation: " << encoder->settings.vui_presentation
		<< endl << "     reordering_delay: " << encoder->settings.reordering_delay
		<< endl << "     use_b_slices: " << encoder->settings.use_b_slices
		<< endl << "     b_slice_pyramid: " << encoder->settings.b_slice_pyramid
		<< endl << "     b_slice_reference: " << encoder->settings.b_slice_reference
		<< flush;						

	encoder->venc = h264OutVideoNew(qencoder_get_rc, &(encoder->settings), 0, 0xFFFFFFFF, 0, 0);
	if (!encoder->venc) {
		qerr << endl << "qCreateEncoderMC(): failed to initialize encoder" << flush;
		qDestroyEncoderAPI(encoder);
		return -1; // XXXXX: what should the error actually be?  I pulled this out of my butt.
	}

	// XXXXX: I don't think that we need set any 'option_flags', nor the 'opt_ptr'.
	h264OutVideoInit(encoder->venc, encoder->bs, 0, NULL);

	*eptr = encoder;
	return 0; // success!
}


void qDestroyEncoderAPI(QEncoder *encoder)
{
	if (encoder == NULL) return;  // even though we won't be called if there is no encoder to destroy...
	if (encoder->venc) h264OutVideoDone(encoder->venc, 1);  // XXXXX: we abort encoding... need another way to flush pending frames.
	if (encoder->outbuf) free(encoder->outbuf);
	if (encoder->bs) close_mem_buf_single(encoder->bs);
	delete encoder;
}


int qEncodeAPI(QEncoder *encoder, char* bytes, int byteSize)
{
	int err;
	
	err = h264OutVideoPutFrame(	encoder->venc, 
								reinterpret_cast<uint8_t*>(bytes), 
								encoder->width*4, 
								encoder->width,
								encoder->height,
								FOURCC_BGR4,
								0, 
								NULL );

	if (err != H264ERROR_NONE) {
		qerr << endl << "qEncodeMC(): failed to push frame into encoder (err: " << err << ")";
		return -1;  // XXXXX:  what error val should this be?
	}

	return 0;
}


char* qEncoderGetPropertyAPI(QEncoder *encoder, char* propertyName, int* resultSize)
{
	if (!(strcmp(propertyName, "PARAMETER_SETS"))) {
		char *result = (char*)malloc(1000);
		if (!result) {
			*resultSize = 0;
			return NULL;
		}
		h264OutVideoGetParSets(encoder->venc, &(encoder->settings), (uint8_t*)result, resultSize);
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


void encoderCallback(void *enc, uint8_t *buf, int32_t bufSize, uint16_t frameType)
{
	QEncoder *encoder = (QEncoder*)enc;

	/* Store data in the output buffer. */
	BufferPtr output = encoder->pool.getBuffer(bufSize+1);
	unsigned char* outputPtr = output->getPointer();
	outputPtr[0] = (frameType == 1 ? 1 : 0);
	memcpy(outputPtr + 1, buf, bufSize);
	
	/* Put the data on the output queue. */
	encoder->queue.add(output);

	interpreterProxy->signalSemaphoreWithIndex(encoder->semaIndex);
}

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
 *  qAudioDecoderAAC.cpp
 *  QAudioPlugin
 *
 */

#include "QAudioPlugin.h"
#include "qAudioDecoderAAC.hpp"
#include "qLogger.hpp"

#include "dec_aac.h"

using namespace Qwaq;

AudioDecoderAAC::AudioDecoderAAC(unsigned char* config, unsigned configSize) 
{	
	inputFrameCount = 0;
	
	// We limit the number of times that we complain about various things, so as to not
	// overwhelm the log-file
	complainCountForBufSizeMismatch = 0;
	complainCountForNoFrameDecoded = 0;
	complainCountForSomeDataUnused = 0;
	
qLog() << "AudioDecoderAAC(): beginning decoder creation";


	// Create an AAC stream, and push in the AudioSpecificConfig data.
	stream = open_AACin_Audio_stream();
	
qerr << endl << "\t...created stream";

	stream->auxinfo(stream, 0, PARSE_AUD_HDR, NULL, 0);
	uint32_t n = stream->copybytes(stream, config, configSize);
	if (n == 0) {
		qLog() << "AudioDecoderAAC(): could not find audio header" << flush;
		close_bufstream(stream,0);
		stream = NULL; // close_bufstream() macro also does this, but it doesn't hurt to be explicit
		return;
	}
	
qerr << endl << "\t...read AudioSpecificConfig (" << n << " bytes)";

	// Configure the decoder
	aac_decoder_config dec_config;
	memset(&dec_config, 0xff, sizeof(dec_config));  // as in SDK documentation
	
	// I'm guessing that the bitstream from the MP4Box NHML-dump is RAW.
	dec_config.bitstream_format = AAD_BSF_RAW;
//	dec_config.bitstream_format = AAD_BSF_ADTS;
	dec_config.max_output_channels = AAD_DMX_MONO; // if input is stereo, mix down to mono
	dec_config.output_format = AAD_DSF_16LE;  // XXXXX: might have to fiddle with this
	dec_config.decode_he = AAD_HE_DONT_DECODE;
	
	if (stream->auxinfo(stream, 0, INIT_FRAME_PARSER, &dec_config, sizeof(dec_config)))
	{
		qLog() << "AudioDecoderAAC(): could not init frame-parser" << flush;
		close_bufstream(stream,0);
		stream = NULL; // close_bufstream() macro also does this, but it doesn't hurt to be explicit
		return;
	}
	
qerr << endl << "\t...initialized frame parser";	
		
	// Squeak will pass in an output buffer each time decode() is called, so a NULL buffer here is OK.
	// XXXXX: not sure what is right to put for bfr_size... if I put 0, it doesn't actually copy data 
	// into my buffer.  Probably, we should pass the output-buffer size as part of the initialization
	// data, and use that value.  But, there doesn't *seem* to be harm in passing an extra-large value.
	outputFrame.bfr = NULL;
	outputFrame.bfr_size = 81920;
	stream->auxinfo(stream, 0, PARSE_FRAMES, &outputFrame, sizeof(outputFrame));
	
qerr << endl << "\t...SUCCESS!" << flush;	
}

AudioDecoderAAC::~AudioDecoderAAC()
{	
	if (stream != NULL) {
		close_bufstream(stream,0);
		stream = NULL; // close_bufstream() macro also does this, but it doesn't hurt to be explicit
	}
	qLog() << "AudioDecoderAAC(): destroyed" << flush;
}

bool
AudioDecoderAAC::isValid()
{
	return stream != NULL;
}

int 
AudioDecoderAAC::decode(unsigned char* input, int inputSize, unsigned short* output, int outputSize, unsigned flags)
{
	++inputFrameCount;

	outputFrame.bfr = (unsigned char*)output;
	outputFrame.bfr_size = outputSize * 2;

	uint32_t n = stream->copybytes(stream, input, inputSize);
	if (n != inputSize) {
		if (complainCountForSomeDataUnused++ < 5) {
			qLog() << "AudioDecoderAAC::decode():  WARNING! used: " << n << " of "
					<< inputSize << " bytes of input data" << flush;
		}
	}
	
	int state = stream->auxinfo(stream, 0, GET_PARSE_STATE, 0, 0);
	if (state & PARSE_DONE_FLAG) {
		aac_decoded_frame_info * frame_info;
		// get decoded frame info
		// note that channel configuration and sampling rate may
		// vary from frame to frame (especially when
		// dec_config.decode_he field is 1)
		stream->auxinfo(stream, 0, GET_PIC_PARAMSP, &frame_info, sizeof(*frame_info));
		
		if (frame_info->decoded_frame_size != outputSize*sizeof(unsigned short)) {
			if (complainCountForBufSizeMismatch++ < 5) {
				qLog() << "AudioDecoderAAC::decode():  WARNING! buf-size mismatch: " 
					<< frame_info->decoded_frame_size << " vs. " << outputSize << flush;
			}
		}
	}
	else {
		if (complainCountForNoFrameDecoded++ < 5) {
			qLog() << "AudioDecoderAAC::decode():  WARNING! no audio decoded for input frame: " 
					<< inputFrameCount << "     (buffered: " << outputFrame.bfr_size << ")" << flush;
		}
	}
	
	return 0;
}

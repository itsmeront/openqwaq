/*
 * Project OpenQwaq
 *
 * Copyright (c) 2011, 3d Immersive Collaboration Consultants, LLC, All Rights Reserved
 *
 */
 
/*
 * qAudioDecoderAAC_libav.cpp
 * QAudioPlugin
 *
 * Created by Josh Gargus on 6/19/11.
 */

// Only enable if MainConcept codecs are disabled
#ifndef _MAINCONCEPT_

#include "QAudioPlugin.h"
#include "qAudioDecoderAAC_libav.hpp"
#include "qLogger.hpp"

extern "C" {
#include "libavcodec/avcodec.h"
}

using namespace Qwaq;

const int ALIGNED_BUF_SIZE = 8192;
struct Qwaq::AudioDecoderAAC_libav_priv {
	AVCodec *codec;
	AVCodecContext *ctxt;
	AVCodecParserContext *parser;
	uint8_t *extradata;
	int extradata_size;

	// "everything" in libav needs to be 16-byte aligned, or things get crashy.
	uint8_t *outbuf;
};

static AudioDecoderAAC_libav_priv* createPrivateContext(unsigned char* config, unsigned configSize);
static void destroyPrivateContext(AudioDecoderAAC_libav_priv* priv);

AudioDecoderAAC_libav_priv* createPrivateContext(unsigned char* config, unsigned configSize)
{
	AudioDecoderAAC_libav_priv* priv = new AudioDecoderAAC_libav_priv();
	if (!priv) return NULL;
	
	priv->codec = NULL;
	priv->ctxt = NULL;
	priv->parser = NULL;
	priv->extradata = NULL;
	priv->extradata_size = 0;
	priv->outbuf = NULL;

	// If AudioSpecificConfig is specified (see AAC spec: ISO-14496-3 section 1.6),
	// then allocate space for it
	if (configSize) {
		priv->extradata = (uint8_t*)av_malloc(configSize);
		if (!priv->extradata) { 
			qLog() << "createPrivateContext: cannot malloc extradata";
			return NULL; 
		}
		memcpy(priv->extradata, config, configSize);
		priv->extradata_size = configSize;
	}

	priv->codec = avcodec_find_decoder(CODEC_ID_AAC);  
	if (!priv->codec) {
		qLog() << "createPrivateContext(): cannot find AAC decoder";
		destroyPrivateContext(priv);
		return NULL;
	}
	qLog() << "   1: found decoder";

	priv->ctxt = avcodec_alloc_context();
	if (!priv->ctxt) {
		qLog() << "createPrivateContext(): cannot allocate AAC context";
		destroyPrivateContext(priv);
		return NULL;
	}
	qLog() << "   2: allocated context";
		
	priv->ctxt->extradata = priv->extradata;
	priv->ctxt->extradata_size = priv->extradata_size;
	if (avcodec_open(priv->ctxt, priv->codec) < 0) {
		qLog() << "createPrivateContext(): cannot open AAC codec";
		destroyPrivateContext(priv);
		return NULL;
	}
	qLog() << "   3: opened codec";

	priv->parser = av_parser_init(CODEC_ID_AAC);
	if (!priv->parser) {
		qLog() << "createPrivateContext(): cannot instantiate AAC parser";
		destroyPrivateContext(priv);
		return NULL;
	}
	qLog() << "   4: initialized codec";

	priv->outbuf = (uint8_t*)av_malloc(ALIGNED_BUF_SIZE);
	if (!priv->outbuf) {
		qLog() << "createPrivateContext(): cannot allocate aligned output buffers";
		destroyPrivateContext(priv);
		return NULL;
	}
	qLog() << "   5: allocated aligned output buffer";

	return priv;
}

void destroyPrivateContext(AudioDecoderAAC_libav_priv* priv) 
{
	if (!priv) return;  // nothing to do

	if (priv->ctxt) {
		avcodec_close(priv->ctxt);
		av_free(priv->ctxt);
		priv->ctxt = NULL;
	}
	
	if (priv->parser) {
		av_parser_close(priv->parser);
		priv->parser = NULL;
	}

	if (priv->outbuf) {
		av_free(priv->outbuf);
		priv->outbuf = NULL;
	}

	delete priv;
}


AudioDecoderAAC_libav::AudioDecoderAAC_libav(unsigned char* config, unsigned configSize) 
{	
	inputFrameCount = 0;
	
	// We limit the number of times that we complain about various things, so as to not
	// overwhelm the log-file
	complainCountForBufSizeMismatch = 0;
	complainCountForNoFrameDecoded = 0;
	complainCountForSomeDataUnused = 0;
	
	priv = createPrivateContext(config, configSize);
	if (priv) {
		qLog() << "AudioDecoderAAC_libav(): created decoder" << flush;
	}
	else {
		qLog() << "ERROR AudioDecoderAAC_libav() could not create decoder" << flush;
	}
}

AudioDecoderAAC_libav::~AudioDecoderAAC_libav()
{	
	if (priv) destroyPrivateContext(priv);
	qLog() << "AudioDecoderAAC_libav(): destroyed decoder" << flush;
}

bool
AudioDecoderAAC_libav::isValid()
{
	return priv != NULL;
}

int 
AudioDecoderAAC_libav::decode(unsigned char* input, int inputSize, unsigned short* output, int outputSize, unsigned flags)
{
	if (!isValid()) {
		qLog() << "AAC decoder is not in a valid state" << flush;
		return -1;
	}

	++inputFrameCount;

	AVPacket packet;
	av_init_packet(&packet);
	packet.data = (uint8_t*)input;
	packet.size = inputSize;
	int outSize = outputSize*2;  // outputSize is number of samples, not bytes
	
	// Sanity-check to make sure our 16-byte-aligned buffer is big enough
	if (outSize > ALIGNED_BUF_SIZE) {
		qLog() << "aligned output buffer is too small (" << ALIGNED_BUF_SIZE << " bytes vs " << outSize << " bytes)";
		return -1;
	}

	// HACK!! avcodec_decode_audio3() has a FIXME about this... because not all audio decoders
	// have a check to ensure that the available space is sufficient, it is extra-conservative
	// and requires a big-ass buffer.  We know how much space AAC output-buffers require, so
	// we fake it out.
	outSize = AVCODEC_MAX_AUDIO_FRAME_SIZE; 
	
	qLog() << "about to decode AAC packet of size: " << outputSize << flush;


	int err = avcodec_decode_audio3(priv->ctxt, (int16_t*)priv->outbuf, &outSize, &packet);
	if (err < 0) {
		qLog() << "error decoding AAC packet: " << err << flush;
		return -1;
	}
	
	// Sanity check for above hack... verify that we did have enough space available.
	if (outSize > outputSize*2) {
		qLog() << "CATASTROPHIC ERROR: stomped on memory because buffer was too small" << flush;
		return -1;
	}

	// Copy data out from aligned memory
	memcpy(output, priv->outbuf, outSize);

	return 0;
}

#endif // #ifndef _MAINCONCEPT_

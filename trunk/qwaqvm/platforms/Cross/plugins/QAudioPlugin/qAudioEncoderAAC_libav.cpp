/*
 * Project OpenQwaq
 *
 * Copyright (c) 2011, 3d Immersive Collaboration Consultants, LLC, All Rights Reserved
 *
 */
 
/*
 * qAudioEncoderAAC_libav.cpp
 * QAudioPlugin
 *
 * Created by Josh Gargus on 6/19/11.
 */

// Only enable if MainConcept codecs are disabled
#ifndef _MAINCONCEPT_

#include "QAudioPlugin.h"
#include "qAudioEncoderAAC_libav.hpp"
#include "qLogger.hpp"

extern "C" {
#include "libavcodec/avcodec.h"
}

using namespace Qwaq;

// See Squeak class QAudioEncoderConfigAAC
// (this is cut'n'pasted several places)
#pragma pack(push, 1)
struct Qwaq::AudioEncoderAAC_config
{
	uint32_t inputChannels;
	uint32_t mpegVersion;
	uint32_t aacObjectType;
	uint32_t audioBitrateIndex;
	uint32_t headerType;
	uint32_t hfCutoff;
	uint32_t vbr;
	uint32_t he;
	uint32_t bitsPerSample;
	uint32_t protectAdtsStream;
	uint32_t reserved[54];
};
#pragma pack(pop)

struct Qwaq::AudioEncoderAAC_libav_priv {
	AVCodec *codec;
	AVCodecContext *ctxt;
	
	AudioEncoderAAC_config config;	
};

static AudioEncoderAAC_libav_priv* createPrivateContext(unsigned char* config, unsigned configSize);
static void destroyPrivateContext(AudioEncoderAAC_libav_priv* priv);

AudioEncoderAAC_libav_priv* createPrivateContext(unsigned char* config, unsigned configSize)
{
	// Ensure that the size of the configuration-data from Squeak is as expected.
	if (configSize != sizeof(AudioEncoderAAC_config)) {
		qerr << endl << "createPrivateContext(): unexpected config-size: " << configSize << flush;
		return NULL;
	}
	
	// Check sanity of config data.
	AudioEncoderAAC_config* sqc = (AudioEncoderAAC_config*)config;
	if (sqc->mpegVersion != 7) {
		qerr << endl << "createPrivateContext(): unexpected MPEG version: " << sqc->mpegVersion;
		return NULL;
	}
	if (sqc->aacObjectType != FF_PROFILE_AAC_LOW+1) { // libav constants are off by one compared to ISO 14496-3-1.5.2.1
		qerr << endl << "createPrivateContext(): only AAC-LC is supported";
		return NULL;
	}
	if (sqc->bitsPerSample != 16) {
		qerr << endl << "createPrivateContext(): only 16-bit input samples are supported";
		return NULL;
	}
		
	// Instantiate the encoder object.
	AudioEncoderAAC_libav_priv* priv = new AudioEncoderAAC_libav_priv();
	if (!priv) {
		qerr << endl << "createPrivateContext(): cannot instantiate encoder object";
		return NULL;
	}
	// Stash the initial config data, for no particularly good reason.
	memcpy(&(priv->config), sqc, sizeof(AudioEncoderAAC_config));
		
	priv->codec = NULL;
	priv->ctxt = NULL;
	
	// Find the codec-type for AAC, and allocate a context for it.
	priv->codec = avcodec_find_encoder(CODEC_ID_AAC);  
	if (!priv->codec) {
		qerr << endl << "createPrivateContext(): cannot find AAC encoder";
		destroyPrivateContext(priv);
		return NULL;
	}
	priv->ctxt = avcodec_alloc_context();
	if (!priv->ctxt) {
		qerr << endl << "createPrivateContext(): cannot allocate AAC context";
		destroyPrivateContext(priv);
		return NULL;
	}

	// Configure the encoder parameters.  
	priv->ctxt->bit_rate = 64000;  // wish we could support VBR!
	priv->ctxt->sample_rate = 16000;
	priv->ctxt->sample_fmt = SAMPLE_FMT_S16;
	priv->ctxt->channels = sqc->inputChannels;
	priv->ctxt->profile = FF_PROFILE_AAC_LOW; // the only one supported, according to the libavcodec source code.
	priv->ctxt->time_base.num = 1;
	priv->ctxt->time_base.den = priv->ctxt->sample_rate;
	priv->ctxt->codec_type = AVMEDIA_TYPE_AUDIO;
	
	// Open the codec.
	if (avcodec_open(priv->ctxt, priv->codec) < 0) {
		qerr << endl << "createPrivateContext(): cannot open AAC codec";
		destroyPrivateContext(priv);
		return NULL;
	}

	return priv;
}

void destroyPrivateContext(AudioEncoderAAC_libav_priv* priv) {
	if (!priv) return;  // nothing to do

	if (priv->ctxt) {
		avcodec_close(priv->ctxt);
		av_free(priv->ctxt);
		priv->ctxt = NULL;
	}
	
	delete priv;
}


AudioEncoderAAC_libav::AudioEncoderAAC_libav(FeedbackChannel* feedbackChannel, unsigned char* config, unsigned configSize) 
: AudioEncoder(feedbackChannel), inFrameCount(0), outFrameCount(0)
{	
	priv = createPrivateContext(config, configSize);
	if (priv) {
		qLog() << "AudioEncoderAAC_libav(): created encoder" << flush;
	}
}

AudioEncoderAAC_libav::~AudioEncoderAAC_libav()
{	
	if (priv) destroyPrivateContext(priv);
	qLog() << "AudioEncoderAAC_libav(): destroyed encoder" << flush;
}

int AudioEncoderAAC_libav::asyncEncode(short *bufferPtr, int sampleCount)
{
	if (!feedback) {
		qLog() << "AudioEncoderAAC_libav::asyncEncode() ...  missing feedback channel!" << flush;
		return -1;
	}
	if (!priv) {
		qLog() << "AudioEncoderAAC_libav::asyncEncode() ...  no encoder available!" << flush;
		return -1;
	}
	if (sampleCount != priv->ctxt->frame_size * priv->ctxt->channels) {
		qLog() << "AudioEncoderAAC_libav::asyncEncode() ...  sampleCount must be frame-size * #channels" << flush;
		return -1;	
	}
	
	uint8_t output[10000];
	int frameBytes = priv->ctxt->frame_size * priv->ctxt->channels * 2;  // 16 bits per sample
	int packetSize = avcodec_encode_audio(priv->ctxt, output, 10000, bufferPtr);
	
	// Check for error.
	if (packetSize < 0) {
		qLog() << "AudioEncoderAAC_libav::asyncEncode() ...  encode failed with status: " << packetSize << flush;
		return -1;
	}
	else if (packetSize == 0) {
		qLog() << "AudioEncoderAAC_libav::asyncEncode() ...  no packet was encoded";
		return 0;
	}

	// Success!  Return the data to Squeak via a feedback channel.
	qLog() << "AudioEncoderAAC_libav::asyncEncode() ...  encoded packet of size: " << packetSize;
	pushFeedbackData(output, packetSize);

	return 0;
}


#endif // #ifndef _MAINCONCEPT_
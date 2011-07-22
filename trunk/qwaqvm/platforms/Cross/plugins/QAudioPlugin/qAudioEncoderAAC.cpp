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
 *  qAudioEncoderAAC.cpp
 *  QAudioPlugin
 *
 */

// Disable use of MainConcept libraries
#ifdef _MAINCONCEPT_

#include "QAudioPlugin.h"
#include "qAudioEncoderAAC.hpp"
#include "qLogger.hpp"
#include "buf_single.h"

#include "enc_aac.h"

using namespace Qwaq;

void encoderCallback(void *encoderPtr, uint8_t *buf, int32_t bufSize, uint16_t ignored);
void encoderCallback(void *encoderPtr, uint8_t *buf, int32_t bufSize, uint16_t ignored)
{
	((AudioEncoderAAC*)encoderPtr)->pushFeedbackData(buf, bufSize);
}


AudioEncoderAAC::AudioEncoderAAC(FeedbackChannel* feedbackChannel, unsigned char* config, unsigned configSize)
: AudioEncoder(feedbackChannel), inFrameCount(0), outFrameCount(0), mc_bufstream(NULL), mc_encoder(NULL)
{
	feedback = feedbackChannel;
	if (!feedback) {
		qLog() << "ERROR: AudioEncoderAAC cannot have NULL feedbackChannel" << flush;
		return;
	}
	
	mc_bufstream = open_mem_buf_single(callbackBuffer,
									MAX_ENCODED_SEGMENT_SIZE,
									640,
									0,
									this,
									encoderCallback);
	if (!mc_bufstream) {
		qLog() << "ERROR: AudioEncoderAAC could not initialize bufstream" << flush;
		return;
	}
	
	// Back in the day (i.e. before Qwaq-Sound-jcg.537) we use to pass in some
	// bogus config settings that we expected to be ignored.  Now we're actually
	// using config-settings from Squeak, but different ones.  This allows us to
	// differentiate the old, bogus ones (which we want to continue to ignore) from
	// the awesome new ones.
	bool useConfigSettingsFromSqueak = configSize >= 256;
	
	if (useConfigSettingsFromSqueak) {
		qLog() << "using settings from Squeak for AAC encoder";
		memcpy(&mc_settings, (aac_a_settings*)config, sizeof(aac_a_settings));
	}
	else {
		// OLD CODE
		qLog() << "using hard-wired settings for AAC encoder";
		aacOutAudioDefaults(&mc_settings, MCPROFILE_DEFAULT);
		mc_settings.input_channels = AAC_CHANNELS_1_MONO;
		mc_settings.mpeg_version = MPEG4_AAC_AUDIO;
		mc_settings.aac_object_type = AAC_LC;  // XXXXX: might want to experiment with "HE", which needs to be handled a bit specially.
		mc_settings.audio_bitrate_index = AAC_AUDIOBITRATE_064;
		mc_settings.vbr = AAC_VBR_NOTUSED;
	
		// XXXXX: I'd like to produce raw AAC, but if I try, the encoder doesn't call my callback ?!?!
		//        So instead, just produce the ADTS headers and strip them off in Squeak.
		//	mc_settings.header_type = AAC_HEADER_RAW;
		mc_settings.header_type = AAC_HEADER_ADTS;
	}
	
	// XXXXX: HARDWIRING!!!! 16kHz is all you get
	mc_encoder = aacOutAudioNew(NULL, &mc_settings, 0, 0, 16000);
	if (!mc_encoder) {
		qLog() << "ERROR: AudioEncoderAAC could not instantiate MC encoder" << flush;
		// XXXXX: should also clean up the bufstream
		return;
	}

	int rc = aacOutAudioInit(mc_encoder, mc_bufstream);
	if (rc != 0) {
		qLog() << "ERROR: AudioEncoderAAC could not attach encoder to bufstream" << flush;
		// XXXXX: should also clean up the bufstream/encoder
		return;
	}
	
	qLog() << "Instantiated AAC Encoder with parameters: " << endl
		<< "\tinput_channels: " << mc_settings.input_channels << endl
		<< "\tmpeg_version: " << mc_settings.mpeg_version << "\t\t(MPEG2=6, MPEG4=7)" << endl
		<< "\taac_object_type: " << mc_settings.aac_object_type << "\t(AAC_LC=2, the only choice... HE is handled below)" << endl
		<< "\taudio_bitrate_index: " << mc_settings.audio_bitrate_index << endl
		<< "\theader_type: " << mc_settings.header_type << "\t\t(RAW=0, ADTS=1, LATMLOAD=2)" << endl
		<< "\thf_cutoff: " << mc_settings.hf_cutoff << endl
		<< "\tvbr: " << mc_settings.vbr << "\t\t(NOTUSED=0, LOW=1-3, MEDIUM=4-6, HIGH=7-9)" << endl
		<< "\the: " << mc_settings.he << "\t\t(NOTUSED=0, IMPLICIT=1 (HEv1), IMPLICIT_WITH_PS=2 (HEv2))" << endl
		<< "\tbits_per_sample: " << mc_settings.bits_per_sample << endl
		<< "\tprotect_adts_stream: " << mc_settings.protect_adts_stream << "\t(NO=0, YES=1)" << flush;
}


AudioEncoderAAC::~AudioEncoderAAC()
{
	if (mc_encoder) {
		aacOutAudioDone(mc_encoder, 0);	
		aacOutAudioFree(mc_encoder);
		mc_encoder = NULL;
	}
	if (mc_bufstream) {
		close_mem_buf_single(mc_bufstream);
		mc_bufstream = 0;
	}
}

		
int AudioEncoderAAC::asyncEncode(short *bufferPtr, int sampleCount)
{
	if (!feedback) {
		qLog() << "AudioEncoderAAC::asyncEncode() ...  missing feedback channel!" << flush;
		return -1;
	}
	if (!mc_encoder) {
		qLog() << "AudioEncoderAAC::asyncEncode() ...  no encoder available!" << flush;
		return -1;
	}
	int rc = aacOutAudioPutBytes(mc_encoder, (unsigned char*) bufferPtr, 2*sampleCount);
	if (rc != aacOutErrNone) {
		qLog() << "AudioEncoderAAC::asyncEncode() ...  error while encoding: " << rc << flush;
		return -1;
	}
	return 0;
}

#endif // #ifdef _MAINCONCEPT_

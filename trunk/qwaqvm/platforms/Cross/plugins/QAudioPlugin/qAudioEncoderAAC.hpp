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
 *  qAudioEncoderAAC.hpp
 *  QAudioPlugin
 *
 */

#ifndef __Q_AUDIO_ENCODER_AAC_HPP__
#define __Q_AUDIO_ENCODER_AAC_HPP__

#include "qAudioEncoder.hpp"
#include "qFeedbackChannel.h"

#include "enc_aac.h"

namespace Qwaq
{

class AudioEncoderAAC : public AudioEncoder
{
	public:
		AudioEncoderAAC(FeedbackChannel* feedbackChannel, unsigned char* config, unsigned configSize);
		virtual ~AudioEncoderAAC();
		
		// Encode the audio, and return the result later via a FeedbackChannel.
		virtual int asyncEncode(short *bufferPtr, int sampleCount);
		
//		virtual bool isValid();


	protected: 

		int outFrameCount;
		int inFrameCount;

		bufstream_tt *mc_bufstream;
		aac_a_settings mc_settings;
		aacaenc_tt *mc_encoder;	
		uint8_t callbackBuffer[MAX_ENCODED_SEGMENT_SIZE];
}; // class AudioEncoderAAC

}; // namespace Qwaq

#endif // #ifndef __Q_AUDIO_ENCODER_AAC_HPP__

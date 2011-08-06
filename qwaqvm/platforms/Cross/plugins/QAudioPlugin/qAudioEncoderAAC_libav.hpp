/*
 * Project OpenQwaq
 *
 * Copyright (c) 2011, 3d Immersive Collaboration Consultants, LLC, All Rights Reserved
 *
 */
 
/*
 * qAudioEncoderAAC_libav.hpp
 * QAudioPlugin
 *
 * Created by Josh Gargus on 6/19/11.
 */

#ifndef __Q_AUDIO_ENCODER_AAC_LIBAV_HPP__
#define __Q_AUDIO_ENCODER_AAC_LIBAV_HPP__
#ifndef _MAINCONCEPT_

#include "qAudioEncoder.hpp"
#include "qFeedbackChannel.h"
#include "qRingBuffer.hpp"

namespace Qwaq
{

// libav-specific stuff
struct AudioEncoderAAC_libav_priv;
// configuration passed from Squeak
struct AudioEncoderAAC_config;

class AudioEncoderAAC_libav : public AudioEncoder
{
	public:
		AudioEncoderAAC_libav(FeedbackChannel* feedbackChannel, unsigned char* config, unsigned configSize);
		virtual ~AudioEncoderAAC_libav();
		
		// Encode the audio, and return the result later via a FeedbackChannel.
		virtual int asyncEncode(short *bufferPtr, int sampleCount);		
//		virtual bool isValid();

	protected:
		struct AudioEncoderAAC_libav_priv* priv;

		QRingBuffer ring;
	
		int outFrameCount;
		int inFrameCount;
}; // class AudioEncoderAAC_libav

}; // namespace Qwaq

#endif // #ifdef _MAINCONCEPT_
#endif // #ifndef __Q_AUDIO_ENCODER_AAC_LIBAV_HPP__
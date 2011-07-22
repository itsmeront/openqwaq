/*
 * Project OpenQwaq
 *
 * Copyright (c) 2011, 3d Immersive Collaboration Consultants, LLC, All Rights Reserved
 *
 */
 
/*
 * qAudioEncoderAAC_apple.hpp
 * QAudioPlugin
 *
 * Created by Josh Gargus on 7/17/11.
 */
 
#ifndef __Q_AUDIO_ENCODER_AAC_APPLE_HPP__
#define __Q_AUDIO_ENCODER_AAC_APPLE_HPP__

#include "qAudioEncoder.hpp"
#include "qFeedbackChannel.h"

namespace Qwaq
{

// CoreAudio-specific stuff
struct AudioEncoderAAC_apple_priv;
// configuration passed from Squeak
struct AudioEncoderAAC_config;

class AudioEncoderAAC_apple : public AudioEncoder
{
	public:
		AudioEncoderAAC_apple(FeedbackChannel* feedbackChannel, unsigned char* config, unsigned configSize);
		virtual ~AudioEncoderAAC_apple();
		
		// Encode the audio, and return the result later via a FeedbackChannel.
		virtual int asyncEncode(short *bufferPtr, int sampleCount);
		
	protected:
		struct AudioEncoderAAC_apple_priv* priv;
		
		int outFrameCount;
		int inFrameCount;
}; // class AudioEncoderAAC_apple

}; // namespace Qwaq

#endif // #ifndef __Q_AUDIO_ENCODER_AAC_APPLE_HPP__
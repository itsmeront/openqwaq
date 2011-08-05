/*
 * Project OpenQwaq
 *
 * Copyright (c) 2011, 3d Immersive Collaboration Consultants, LLC, All Rights Reserved
 *
 */
 
/*
 * qAudioDecoderAAC_libav.hpp
 * QAudioPlugin
 *
 * Created by Josh Gargus on 6/19/11.
 */


#ifndef __Q_AUDIO_DECODER_AAC_LIBAV_HPP__
#define __Q_AUDIO_DECODER_AAC_LIBAV_HPP__

// Only use libav if we're not using MainConcept.
#ifndef _MAINCONCEPT_

#include "qAudioDecoder.hpp"

namespace Qwaq
{

// libav-specific stuff
struct AudioDecoderAAC_libav_priv;

class AudioDecoderAAC_libav : public AudioDecoder
{
	public:
		AudioDecoderAAC_libav(unsigned char* config, unsigned configSize);
		virtual ~AudioDecoderAAC_libav();
		
		virtual bool isValid();
		virtual int decode(unsigned char* input, int inputSize, short* output, int outputSize, unsigned flags);
		
	protected:
		struct AudioDecoderAAC_libav_priv* priv;
		
		unsigned inputFrameCount;
		unsigned complainCountForBufSizeMismatch;
		unsigned complainCountForNoFrameDecoded;
		unsigned complainCountForSomeDataUnused;
}; // class AudioDecoderAAC_libav

}; // namespace Qwaq

#endif // #ifndef _MAINCONCEPT_
#endif // #ifndef __Q_AUDIO_DECODER_AAC_LIBAV_HPP__

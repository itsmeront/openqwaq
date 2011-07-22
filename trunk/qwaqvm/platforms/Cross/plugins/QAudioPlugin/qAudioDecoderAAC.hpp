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
 *  qAudioDecoderAAC.hpp
 *  QAudioPlugin
 *
 */

#ifndef __Q_AUDIO_DECODER_AAC_HPP__
#define __Q_AUDIO_DECODER_AAC_HPP__
#ifdef _MAINCONCEPT_

#include "qAudioDecoder.hpp"
#include "bufstrm.h"
#include "mcdefs.h"

namespace Qwaq
{

class AudioDecoderAAC : public AudioDecoder
{
	public:
		AudioDecoderAAC(unsigned char* config, unsigned configSize);
		virtual ~AudioDecoderAAC();
		
		virtual bool isValid();
		virtual int decode(unsigned char* input, int inputSize, unsigned short* output, int outputSize, unsigned flags);
		
	protected:
		bufstream_tt *stream;
		unsigned inputFrameCount;
		
		aud_bfr_tt outputFrame;

		unsigned complainCountForBufSizeMismatch;
		unsigned complainCountForNoFrameDecoded;
		unsigned complainCountForSomeDataUnused;

}; // class AudioDecoderAAC

}; // namespace Qwaq

#endif // #ifdef _MAINCONCEPT_
#endif // #ifndef __Q_AUDIO_DECODER_AAC_HPP__

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
 *  qAudioDecoder.cpp
 *  QAudioPlugin
 *
 */

#include "QAudioPlugin.h"
#include "qAudioDecoder.hpp"
#ifdef _MAINCONCEPT_
#include "qAudioDecoderAAC.hpp"
#endif
#include "qLogger.hpp"

using namespace Qwaq;

boost::mutex AudioDecoder::s_Mutex;
unsigned AudioDecoder::s_Key = 0;
AudioDecoder::map_type AudioDecoder::s_Map;

unsigned qCreateAudioDecoder(unsigned codecType, unsigned char* config, unsigned configSize)
{
	AudioDecoder *decoder;
	switch (codecType) {
#ifdef _MAINCONCEPT_
		case CodecTypeAAC:
			decoder = new AudioDecoderAAC(config, configSize);
			break;
#endif 
		default:
			qLog() << "qCreateAudioDecoder(): unexpected decoder type: " << codecType << flush;
			return 0;
	};
	return decoder->key();
}

void qDestroyAudioDecoder(unsigned handle)
{
	AudioDecoder::releaseKey(handle);
}

int qAudioDecode(unsigned handle, unsigned char* input, int inSize, unsigned short* output, int outSize, unsigned flags)
{
	shared_ptr<AudioDecoder> decoder = AudioDecoder::withKey(handle);
	if (!decoder.get()) {
		qLog() << "qAudioDecode(): can't get audio-decoder with key: " << handle << flush;
		return -1;
	}
	return decoder->decode(input, inSize, output, outSize, flags);
}

AudioDecoder::AudioDecoder()
{
	addToMap();
}

AudioDecoder::~AudioDecoder()
{

}

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
 *  qAudioEncoder.cpp
 *  QAudioPlugin
 *
 */

#include "QAudioPlugin.h"
#include "qAudioEncoder.hpp"
#ifdef _MAINCONCEPT_
#include "qAudioEncoderAAC.hpp"
#endif
#include "qLogger.hpp"

using namespace Qwaq;

boost::mutex AudioEncoder::s_Mutex;
unsigned AudioEncoder::s_Key = 0;
AudioEncoder::map_type AudioEncoder::s_Map;

// Also see Squeak class QEncodedAudioSegmentFeedback
struct QEncodedAudioSegment
{
	int type;
	unsigned dataSize;
	unsigned char data[MAX_ENCODED_SEGMENT_SIZE];  // plenty
};
class QEncodedAudioSegmentFeedback : public FeedbackEvent
{
	public:
		QEncodedAudioSegmentFeedback() { squeakEvent.type = 2; }
		virtual int getSqueakEventSize() { return squeakEvent.dataSize + 8; }
		virtual void fillInSqueakEvent(char* ptr) { memcpy(ptr, &squeakEvent, squeakEvent.dataSize + 8); }
		
		QEncodedAudioSegment squeakEvent;
};
		

void AudioEncoder::pushFeedbackData(void* data, unsigned dataSize)
{
	if (!feedback) {
		qLog() << "AudioEncoderAAC::pushFeedbackData() ...  missing feedback channel!" << flush;
		return;
	}
	if (dataSize > MAX_ENCODED_SEGMENT_SIZE) {
		qLog() << "Exceeded maximum encoded audio segment size of " << MAX_ENCODED_SEGMENT_SIZE << " bytes";
		return;
	}
	QEncodedAudioSegmentFeedback *evt = new QEncodedAudioSegmentFeedback;
	if (!evt) {
		qLog() << "Failed to allocate QEncodedAudioSegmentFeedback" << flush;
		return;
	}
	evt->squeakEvent.dataSize = dataSize;
	memcpy(evt->squeakEvent.data, data, dataSize);
	FeedbackEventPtr p(evt);
	feedback->push(p);
}
		
unsigned qCreateAudioEncoder(unsigned codecType, void* feedbackChannel, unsigned char* config, unsigned configSize)
{
	AudioEncoder *encoder;
	
	// Depending on the codec type, 'feedbackChannel' may be NULL.
	FeedbackChannel *channel = NULL;
	if (feedbackChannel) channel = *((FeedbackChannel**)feedbackChannel);
	
	switch (codecType) {
#ifdef _MAINCONCEPT_
		case CodecTypeAAC:
			encoder = new AudioEncoderAAC(channel, config, configSize);
			break;
#endif
		default:
			qLog() << "qCreateAudioEncoder(): unexpected encoder type: " << codecType << flush;
			return 0;
	};
	return encoder->key();
}


void qDestroyAudioEncoder(unsigned handle)
{
	AudioEncoder::releaseKey(handle);
}


int qAudioAsyncEncode(unsigned handle, short *bufferPtr, int sampleCount)
{
	shared_ptr<AudioEncoder> encoder = AudioEncoder::withKey(handle);
	if (!encoder.get()) {
		qLog() << "qAudioAsyncEncode(): can't get audio-encoder with key: " << handle << flush;
		return -1;
	}
	return encoder->asyncEncode(bufferPtr, sampleCount);
}


AudioEncoder::AudioEncoder(FeedbackChannel *feedbackChannel) : feedback(feedbackChannel)
{
	addToMap();
}


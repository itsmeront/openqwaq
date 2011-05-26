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
 *  qAudioSinkSpeex.h
 *  QAudioPlugin
 *
 */

#ifndef __Q_AUDIO_SINK_SPEEX_HPP__
#define __Q_AUDIO_SINK_SPEEX_HPP__

#include <boost/thread/mutex.hpp>

#include <speex/speex.h>
#include <speexclient/speex_jitter_buffer.h>

#include "QAudioPlugin.h"
#include "qTickee.hpp"
#include "qEventTimeLogger.hpp"

#include <fstream>
#include <vector>

namespace Qwaq {

struct DeferredPut {
	DeferredPut(char* b, int bsize, int tstamp) : bytes(b), byteSize(bsize), appTimestamp(tstamp) { }
	char* bytes;
	int byteSize;
	int appTimestamp;
};
typedef std::vector<DeferredPut> DeferredPutVect;

#define QWAQ_JITTER_BUFFER_GET_ACTIVITY_THRESHOLD 101
#define QWAQ_JITTER_BUFFER_SET_ACTIVITY_THRESHOLD 102
#define QWAQ_JITTER_BUFFER_SET_ENABLE_DROPPING 103

class QAudioSinkSpeex : public Tickee
{
	public:
		QAudioSinkSpeex();
		virtual ~QAudioSinkSpeex();
		
		virtual void tick();
		virtual unsigned tickPriority() { return 10; }	// top priority
		virtual void addSource(shared_tickee src) {} // meaningless for Speex sink
		virtual void removeSource(shared_tickee src) {} // meaningless for Speex sink
		
		void pushEncodedSpeex(void* bytes, int byteSize, int appTimestamp);
		int jitterbufferCtl(int ctlType, int ctlVal);
		
		// These days, does more than the name would suggest.  Signals the end
		// of an utterance... play out any buffers we have queued up, then reset.
		void resetTimestamps();
	
		int getJitterbufferStats(QJitterbufferStats* stats);

		virtual void printDebugInfo();

		// Log for debugging... capture EVERYTHING
		void startDebugRecording(char* filePath);
		void stopDebugRecording();
		bool isDebugRecording() { return isRecording; }
		
		virtual sqInt getEventTimings();
						
	protected:
		QEventTimeLogger timeLogger;
	
		void* decState;
		SpeexJitter jitter;
		
		// Don't read too much into the meaning of these two.  There used to be a
		// correlation such that when you read get_timestamp[FOO], you could say
		// "oh, there's the sample that I pushed in at put_timestamp[FOO]" (at least,
		// if we understand how the Speex jitter-buffer works, but that's a different
		// story).  This correspondence has been broken ever since we started playing
		// out all remaining sound before resetting after an end-of-utterance, and
		// will undoubtedly be further muddied in the future.
		int put_timestamp;
		int get_timestamp;
		
		long total_gets;
		long total_extrapolations;
		long recent_gets;
		long recent_extrapolations;
		long total_drops;
		long recent_drops;
		
		// Speex parameters that should be reset each time we re-init the JB.
		int margin;
		int delayStep;
		int concealSize;
		int maxLateRate;
		int lateCost;
		
		// When true, will reset Speex-JB once we finish playing out currently-buffered audio.
		bool wantsReset;
		// When true, we will drop packets if we start to buffer too many.
		bool enableDropping;
		// Keep track of how many packets are buffered in the Speex JB.
		int bufferedPacketCount;
		
		boost::mutex speexMutex;
		
		// Must be called from within a function that has already locked the mutex.
		void initSpeexState();
		void destroySpeexState();

		int runOfGets;
		int runOfPuts;
		
		// For supporting debug-logging... capture EVERYTHING
		std::ofstream recordFile;
		bool isRecording;
		// Utility method to record bytes as ASCII-hex
		void recordBytesOnFile(void* bytes, int byteSize);
		
		void getBufferFromJitterbuffer();
		
		DeferredPutVect deferredPuts;
};

}; // namespace Qwaq

#endif // #ifndef __Q_AUDIO_SINK_SPEEX_HPP__

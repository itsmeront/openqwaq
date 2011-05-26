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
 *  qAudioSinkSpeex.cpp
 *  QAudioPlugin
 *
 */

#include "qAudioSinkSpeex.hpp"
#include "qLogger.hpp"
using namespace Qwaq;

#include <boost/thread/locks.hpp>
typedef boost::mutex::scoped_lock scoped_lock;

const int SAMPLING_RATE = 16000;

QAudioSinkSpeex::QAudioSinkSpeex() : Tickee(), timeLogger(100) // enough for 2 seconds
{
	className = "QAudioSinkSpeex"; // for logging

	decState = NULL;
	put_timestamp = get_timestamp = 0;
	
	margin = 1280;
	delayStep = 320;  // by default, it's the frame size (see speex_jitter_init())
	concealSize = 320;
	maxLateRate = 4;
	lateCost = 0;
	
	isRecording = false;
	enableDropping = true;
	bufferedPacketCount = 0;
	
	{ scoped_lock lk(speexMutex); initSpeexState(); }
	log() << " ** CREATED" << flush;

	runOfPuts = runOfGets = 0;
	jitter.activity_threshold = 0;
	
	total_gets = total_extrapolations = total_drops = 0;
	recent_gets = recent_extrapolations = recent_drops = 0;
}

QAudioSinkSpeex::~QAudioSinkSpeex()
{
	{ 
		scoped_lock lk(speexMutex); 
		destroySpeexState(); 
		if (isRecording) {
			recordFile.close();
			isRecording = false;
		}
	}
	log() << " ** DESTROYED" << flush;
}

void QAudioSinkSpeex::getBufferFromJitterbuffer()
{
	// Update jitterbuffer stats
	++total_gets;
	++recent_gets;
	
	int ret;  // Speex return value
	void* datagramTimestamp;
	int activity, activityThreshold;
	int dropCount = 0;
	
	
	while (1) {
		ret = speex_jitter_get(&jitter, buffer, NULL, &datagramTimestamp);
		speex_decoder_ctl(jitter.dec, SPEEX_GET_ACTIVITY, &activity);
		jitter_buffer_ctl(jitter.packets, JITTER_BUFFER_GET_AVAILABLE_COUNT, &bufferedPacketCount);
		
		if (ret == 2) { //extrapolation
			++total_extrapolations;
			++recent_extrapolations;
			if (isRecording) {
				recordFile << "get2-[" << get_timestamp << "-" << bufferedPacketCount << "]-640-";
				recordBytesOnFile(buffer, 640);
				recordFile << endl;
			}
			return;
		}
		
		// The margin is specified in terms of samples, and a packet contains 320 samples.
		int marginExcess = bufferedPacketCount - (margin / 320);
		// There are a few ways that packet-dropping can be disabled, such as 
		// being disabled on a per-sink basis, or if we've already dropped too 
		// many packets this tick. 
		if (!enableDropping || dropCount >= 2) { 
			activityThreshold = -1; //disabled
		} else {
			// Purely heuristic.  The more excess we have, the more willing we 
			// are to drop something. If we have "negative excess", the threshold 
			// will be below zero, and the activity will always be higher (since 
			// it ranges from 0-100).  Clamp to a relatively low value... if we 
			// fall very far behind due to a large network glitch, it's better to 
			// catch up a bit more slowly than to drop "meaningful" packets.
			activityThreshold = (marginExcess > 25) ? 25 : marginExcess;
			
		}
		if (activity >= activityThreshold) {
			// We've decided not to drop this packet.
			if (datagramTimestamp != NULL) {
				// Allow Squeak to keep track of how long this packet was in the JB.
				timeLogger.add((int)datagramTimestamp);
			}
			if (isRecording) {
				recordFile 
					<< "get0-[" << get_timestamp << "-" << bufferedPacketCount << "-" << activity 
					<< "-" << ((int)((double)(int)datagramTimestamp) / 20.0) * 320 << "]-640-";
				recordBytesOnFile(buffer, 640);
				recordFile << endl;
			}
			return;
		}
		else {
			// We've buffered more than we want, and we're below the activity threshold,
			// so drop a packet and try again.
			++total_drops;
			++recent_drops;
			++dropCount;
			if (isRecording) {
				recordFile 
					<< "drop0-[" << get_timestamp << "-" << bufferedPacketCount << "-" << activity << "]-640-";
				recordBytesOnFile(buffer, 640);
				recordFile << endl;
			}
		}
	}
}

void QAudioSinkSpeex::tick()
{
	int verbosity;
	verbosity = (put_timestamp > 0) ? 7 : 10; 
	log(verbosity) << "tick()...  gettime=" << get_timestamp << "  puttime=" << put_timestamp << flush;

#if ITIMER_HEARTBEAT /* eem 22/2/2010 */
	/* avoid deadlock in the interrupt-driven sound pipeline.  If the speexMutex
	 * is already locked (by one of the functions below) simply return.  This
	 * is unlikely (e.g. one lockup in 5 minutes of use) and so there is no need
	 * to have lock holders check for a missed tick when releasing the lock.
	 */
	if (!speexMutex.try_lock())
		return;
#else
	scoped_lock lk(speexMutex);
#endif
	runOfGets++;
	runOfPuts = 0;

	// Do this here, because we'll potentially use the variable (and possibly its value) 
	// in several places.
	jitter_buffer_ctl(jitter.packets, JITTER_BUFFER_GET_AVAILABLE_COUNT, &bufferedPacketCount);
	
	if (put_timestamp < margin && !wantsReset) {
		// We haven't buffered enough audio yet.
		hasBuffer = false;
		
		if (isRecording) {
			recordFile << "get9-[" << get_timestamp << "-" << bufferedPacketCount << "]" << endl;
		}
#if ITIMER_HEARTBEAT
		speexMutex.unlock();
#endif
		return;
	}
	
	if (wantsReset && !bufferedPacketCount) {
		// We want a reset, and have finished playing out the remaining buffers
		
		if (isRecording) {
			// Log before we reset the timestamps
			recordFile << "reset2-[" << put_timestamp << "-" << get_timestamp << "]" << endl;
		}
	
		destroySpeexState();
		initSpeexState();
		hasBuffer = false;
		
		// If any 'puts' came in while we were waiting for the buffer to drain
		// after a 'reset1', now is the time to enqueue these guys.
		for (DeferredPutVect::iterator it = deferredPuts.begin(); it != deferredPuts.end(); it++) {
			char* bytes = it->bytes;
			int byteSize = it->byteSize;
			put_timestamp += FRAME_SIZE;
			speex_jitter_put(&jitter, bytes, byteSize, put_timestamp, (void*)(it->appTimestamp));
			
			// Record the incoming buffer for debugging.
			if (isRecording) {
				jitter_buffer_ctl(jitter.packets, JITTER_BUFFER_GET_AVAILABLE_COUNT, &bufferedPacketCount);
				recordFile << "put3-[" << put_timestamp << "-" << bufferedPacketCount << "]-" << byteSize << "-";
				recordBytesOnFile(bytes, byteSize);
				recordFile << endl;
			}
			
			free(bytes); // was malloced in tick()
		}
		deferredPuts.clear();
		
#if ITIMER_HEARTBEAT
		speexMutex.unlock();
#endif
		return;		
	}

	// Pull a buffer from the JB.
	get_timestamp += FRAME_SIZE;
	getBufferFromJitterbuffer();
	hasBuffer = true;

#if ITIMER_HEARTBEAT
	speexMutex.unlock();
#endif
}

// Must be called from within a function that has already locked the mutex.
void QAudioSinkSpeex::initSpeexState()
{	
	if (decState != NULL) {
		log() << "initSpeexState() ...  state is already initialized.  Exiting" << flush;
		return;
	}
	
	decState = speex_decoder_init(speex_lib_get_mode(SPEEX_MODEID_WB));
	speex_jitter_init(&jitter, decState, SAMPLING_RATE);
	jitter_buffer_ctl(jitter.packets, JITTER_BUFFER_SET_MARGIN, &margin);
	jitter_buffer_ctl(jitter.packets, JITTER_BUFFER_SET_DELAY_STEP, &delayStep);
	jitter_buffer_ctl(jitter.packets, JITTER_BUFFER_SET_CONCEALMENT_SIZE, &concealSize);
	jitter_buffer_ctl(jitter.packets, JITTER_BUFFER_SET_MAX_LATE_RATE, &maxLateRate);
	jitter_buffer_ctl(jitter.packets, JITTER_BUFFER_SET_LATE_COST, &lateCost);
	
	// If you call this explicitly, the jitterbuffer will assume that you want to
	// manage this explicitly.  We do, because the JB makes some dubious decisions
	// when left to its own devices.
	jitter_buffer_update_delay(jitter.packets, NULL, NULL);
	
	put_timestamp = get_timestamp = 0;
	wantsReset = false;
}

// Must be called from within a function that has already locked the mutex.
void QAudioSinkSpeex::destroySpeexState()
{	
	if (decState == NULL) {
		log() << "destroySpeexState() ...  state is already destroyed.  Exiting" << flush;
		return;
	}
	
	speex_decoder_destroy(decState);
	speex_jitter_destroy(&jitter);
	decState = NULL;
}

void QAudioSinkSpeex::pushEncodedSpeex(void* bytes, int byteSize, int appTimestamp)
{
	scoped_lock lk(speexMutex);
	
	runOfPuts++;
	runOfGets = 0;

	if (wantsReset) {
		// Let JB finish draining from last reset... after it is done, we will push
		// in packets that arrived in the meantime.
		char *deferred = (char*)malloc(byteSize);
		if (deferred) {
			memcpy(deferred, bytes, byteSize);
			deferredPuts.push_back(DeferredPut(deferred, byteSize, appTimestamp));
			if (isRecording) {
				jitter_buffer_ctl(jitter.packets, JITTER_BUFFER_GET_AVAILABLE_COUNT, &bufferedPacketCount);
				recordFile << "put2-[" << put_timestamp << "-" << bufferedPacketCount << "]-" << byteSize << "-";
				recordBytesOnFile(bytes, byteSize);
				recordFile << endl;
			}
		}
		else {
			log() << "pushEncodedSpeex(): failed to instantiate deferred packet!!" << flush;
			if (isRecording) {
				recordFile << "error-[failed to instantiate deferred packet]" << endl;
			}
		}
		return;
	}

	if (!decState) {
		log() << "pushEncodedSpeex(): not initialized!!" << flush;
		return;
	}
	
	put_timestamp += FRAME_SIZE;
	speex_jitter_put(&jitter, (char*)bytes, byteSize, put_timestamp, (void*)appTimestamp);
	
	// Record the incoming buffer for debugging.
	if (isRecording) {
		jitter_buffer_ctl(jitter.packets, JITTER_BUFFER_GET_AVAILABLE_COUNT, &bufferedPacketCount);
		recordFile << "put1-[" << put_timestamp << "-" << bufferedPacketCount << "]-" << byteSize << "-";
		recordBytesOnFile(bytes, byteSize);
		recordFile << endl;
	}
}	

int QAudioSinkSpeex::jitterbufferCtl(int ctlType, int ctlVal)
{
	scoped_lock lk(speexMutex);
	
	// 
	switch (ctlType) {
		case JITTER_BUFFER_SET_MARGIN:
			margin = ctlVal;
			if (isRecording) recordFile << "param-margin-" << margin << endl;
			break;
		case JITTER_BUFFER_SET_DELAY_STEP:
			delayStep = ctlVal;
			if (isRecording) recordFile << "param-delayStep-" << delayStep << endl;
			break;
		case JITTER_BUFFER_SET_CONCEALMENT_SIZE:
			concealSize = ctlVal;
			if (isRecording) recordFile << "param-concealSize-" << concealSize << endl;
			break;
		case JITTER_BUFFER_SET_MAX_LATE_RATE:
			maxLateRate = ctlVal;
			if (isRecording) recordFile << "param-maxLateRate-" << maxLateRate << endl;
			break;
		case JITTER_BUFFER_SET_LATE_COST:
			lateCost = ctlVal;
			if (isRecording) recordFile << "param-lateCost-" << lateCost << endl;
			break;
		case QWAQ_JITTER_BUFFER_SET_ACTIVITY_THRESHOLD:
			jitter.activity_threshold = ctlVal;
			if (isRecording) recordFile << "param-activityThreshold-" << ctlVal << endl;
			return ctlVal;
		case QWAQ_JITTER_BUFFER_GET_ACTIVITY_THRESHOLD:
			return jitter.activity_threshold;
		case QWAQ_JITTER_BUFFER_SET_ENABLE_DROPPING:
			enableDropping = (ctlVal == 0) ? false : true;
			log() << "set ENABLE_DROPPING to: " << enableDropping << flush;
			if (isRecording) recordFile << "param-enableDropping-" << enableDropping << endl;
			return ctlVal;
	}
	
	// Avoid messing with the stack variable if the command is a 'get'
	int tmp = ctlVal;
	jitter_buffer_ctl(jitter.packets, ctlType, &tmp);
	log() << "jitterBufferCtl(" << ctlType << ", " << ctlVal << ") == " << tmp << flush;
	return tmp;
}
		
void QAudioSinkSpeex::resetTimestamps()
{
	scoped_lock lk(speexMutex);
	log(3) << "resetTimestamps() ... was put/get: " << put_timestamp << "/" << get_timestamp << flush;
	
	// We no longer actually do the reset here... we want to finish playing out the remaining 
	// buffered frames (see tick() ).
	wantsReset = true;
	
	if (isRecording) {
		recordFile << "reset1-[" << put_timestamp << "-" << get_timestamp << "]" << endl;
	}
}


int 
QAudioSinkSpeex::getJitterbufferStats(QJitterbufferStats* stats)
{
	// Communicate back to Squeak the version that we're using to 
	// fill in the fields.  Never fill in fields corresponding to
	// a higher version than requested by Squeak.
	if (stats->version > 2) {
		stats->version = 2;  // the highest version we support
	}
	
	// VERSION 1 fields (always used)
	{
		stats->total_gets = total_gets;
		stats->total_extrapolations = total_extrapolations;
	
		// Get the info accumulated since last request, and reset it.
		stats->recent_gets = recent_gets;
		stats->recent_extrapolations = recent_extrapolations;
		recent_gets = recent_extrapolations = 0;
	}
	
	// VERSION 2 fields
	if (stats->version >= 2) {
		// Get the info accumulated since last request, and reset it.
		stats->total_drops = total_drops;
		stats->recent_drops = recent_drops;
		recent_drops = 0;
		stats->amount_buffered = bufferedPacketCount * 320;  // 320 samples per 20ms packet
	}
		
	return 0; // success (we don't yet have any error conditions)
}


void QAudioSinkSpeex::printDebugInfo()
{
	log() 
		<< "printDebugInfo(): " << "\n\t"
		<< "put-time: " << put_timestamp << "\n\t"
		<< "get-time: " << get_timestamp << "\n\t"
		<< "has buffer: " << (hasBuffer ? "true" : "false") << flush;
}


void QAudioSinkSpeex::startDebugRecording(char* filePath) 
{
	if (isRecording) stopDebugRecording();
	
	log() << "starting debug recording: " << filePath << flush;

	scoped_lock lk(speexMutex);
	recordFile.open(filePath);
	if (recordFile.fail()) return;
	isRecording = true;
	/* 
		version 1.1:
			- added 'activityThreshold' parameter
		version 1.2:
			- added 'enableDropping' parameter
	*/
	recordFile << "version-1.2" << endl;
	recordFile << "param-margin-" << margin << endl;
	recordFile << "param-delayStep-" << delayStep << endl;
	recordFile << "param-concealSize-" << concealSize << endl;
	recordFile << "param-maxLateRate-" << maxLateRate << endl;
	recordFile << "param-lateCost-" << lateCost << endl;
	recordFile << "param-activityThreshold-" << jitter.activity_threshold << endl;
	recordFile << "param-enableDropping-" << enableDropping << endl;
}

void QAudioSinkSpeex::stopDebugRecording()
{
	log() << "stopping debug recording" << flush;

	scoped_lock lk(speexMutex);
	if (isRecording) recordFile.close();
	isRecording = false;
}

// Utility method to record bytes as ASCII-hex.  Ugly but it works.
void QAudioSinkSpeex::recordBytesOnFile(void* bytes, int byteSize)
{
	for (int i = 0; i < byteSize; i++) {
		unsigned char byte = ((unsigned char*)bytes)[i];
		unsigned char highNibble = byte >> 4;
		unsigned char lowNibble = byte & 0x0f;
		if (highNibble < 10) { recordFile << (unsigned char)('0' + highNibble); }
		else { recordFile << (unsigned char)('a' + (highNibble-10)); }
		if (lowNibble < 10) { recordFile << (unsigned char)('0' + lowNibble); }
		else { recordFile << (unsigned char)('a' + (lowNibble-10)); }
	}
}

// For now, only Speex-sinks support this.
sqInt QAudioSinkSpeex::getEventTimings()
{
	return timeLogger.getIntoNewOop();
}

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
 *  qAudioSinkOpenAL.cpp
 *  QAudioPlugin
 *
 */

#include "qAudioSinkOpenAL.hpp"
#include "qAudioOpenAL.h"
#include "qLogger.hpp"
using namespace Qwaq;

typedef boost::unique_lock<boost::shared_mutex> unique_mutex_lock;
typedef boost::shared_lock<boost::shared_mutex> shared_mutex_lock;

QAudioSinkOpenAL::QAudioSinkOpenAL() : Tickee()
{
	className = (char*) "QAudioSinkOpenAL"; // for logging
	
	oalSource = 0;
	oalCount = 0;
	transform[0] = transform[1] = transform[2] = transform[3] = transform[4] = 0.0;
	transform[5] = 1.0;
	gain = 1.0;
	
	// Use same values that #createSource does; doesn't matter, since we explicitly set them.
	refDist = 50.0; 
	maxDist = 10000.0; 
	rolloff = 0.2; 
	innerAngle = 180.0; 
	outerAngle = 360.0;
	outerGain = 0.8;
		
	silentStreak = 0;
	isStarted = false;
	isGainBakedIn = false;
	log() << " ** CREATED" << flush;
}

QAudioSinkOpenAL::~QAudioSinkOpenAL()
{
	log() << " ** DESTROYED ";
	shared_mutex_lock lk(g_oal_mutex);
	deleteOpenALResources(0);
}

void QAudioSinkOpenAL::tick()
{	
	boost::shared_ptr<Tickee> strong = source.lock();
	if (strong.get() == NULL || !strong->hasValidBuffer()) {
		
		hasBuffer = false;
		
		silentStreak++;
		if (silentStreak == 3) {
			isStarted = false;  // so that next time we receive a buffer, we queue a few up for OpenAL.
		}
		else if (silentStreak == 50) {
			shared_mutex_lock lk(g_oal_mutex);
			if (g_oal_context) {
				alcMakeContextCurrent(g_oal_context);
				if (oalSource != 0) {
					log(1) << "tick has no buffers to play for 1 second; releasing source/buffers";
					deleteOpenALResources(1); 
				}
			}
		} 
		// If we've been playing, we must free buffers here (because playAudioDirectly won't be able 
		// to do it later if not yet isStarted). Otherwise, we'll have no freeBuffers later on, and 
		// the OS audio may extrapolate (as OSX is want to do, producing a machine gun sound).
		// Note that this means that even when we bail here, the OS can keep playing what it has.
		if (oalSource != 0) {  
			shared_mutex_lock lk(g_oal_mutex);
			ALint nBuffAlloc = 0, nBuffProc = 0;
			alGetSourcei(oalSource, AL_BUFFERS_QUEUED, &nBuffAlloc);
			if (nBuffAlloc > 0) { // If we've been playing...
				alGetSourcei(oalSource, AL_BUFFERS_PROCESSED, &nBuffProc);
				while ( (nBuffProc-- > 0) && returnOneBuffer() );
			}	
		}
		return;
	}

	// XXXXX: should lock the source while we're using its buffer.
	// Not necessary right now since the buffers are part of the sinks,
	// but if we dynamically allocate the buffers later, then this will
	// apply.
	hasBuffer = true;
	silentStreak = 0;
	short* sourceBuffer = strong->getBuffer();
	memcpy(buffer, sourceBuffer, FRAME_SIZE * sizeof(short));
	isGainBakedIn = (gain == 1.0f);
	playAudioDirectly(sourceBuffer, FRAME_SIZE * sizeof(short));
}


// Override getBuffer() to lazily scale the samples by the gain (don't want to
// do this if we don't have to).
short* QAudioSinkOpenAL::getBuffer() 
{
	if (!isGainBakedIn) {
		isGainBakedIn = true;
		if (gain == 0.0) {
			memset(buffer, 0, FRAME_SIZE * sizeof(short));
		}
		else {
			for (int i = 0; i < FRAME_SIZE; i++) {
				buffer[i] = (short)(gain * (float)(buffer[i]));
			}
		}
	}
	return buffer;
}


void QAudioSinkOpenAL::setTransform(double posX, double posY, double posZ, double dirX, double dirY, double dirZ)
{
	bool changed = false;
	if (transform[0] != posX) { transform[0] = posX; changed = true; }
	if (transform[1] != posY) { transform[1] = posY; changed = true; }
	if (transform[2] != posZ) { transform[2] = posZ; changed = true; }
	if (transform[3] != dirX) { transform[3] = dirX; changed = true; }
	if (transform[4] != dirY) { transform[4] = dirY; changed = true; }
	if (transform[5] != dirZ) { transform[5] = dirZ; changed = true; }
	if (!changed) return;
		
	// Ensure that the OpenAL context doesn't change out from under us.
	shared_mutex_lock lk(g_oal_mutex);
	if (oalSource == 0) return; // don't bother creating one yet... only when we need to play something.
	ensureValidSource();
	if (oalSource == 0) return;  // We weren't able to create a valid source, so there's nothing to do.

	// We have a valid source... set all of the properties that we have.
	alSource3f(oalSource, AL_POSITION, (float)posX, (float)posY, (float)posZ);
	alSource3f(oalSource, AL_DIRECTION, (float)dirX, (float)dirY, (float)dirZ);
}


void QAudioSinkOpenAL::setInitialPropertiesOpenAL(double rDist, double mDist, double roll, double inAngle, double outAngle, double outGain)
{
	shared_mutex_lock lk(g_oal_mutex);
	
	refDist = rDist; 
	maxDist = mDist; 
	rolloff = roll; 
	innerAngle = inAngle; 
	outerAngle = outAngle;
	outerGain = outGain;
	
	setInitialSourceProperties();
}


// must be called from a method that already has a lock on 'g_oal_mutex'
void QAudioSinkOpenAL::setInitialSourceProperties()
{
	if (oalSource == 0) return;
	
	alSourcef(oalSource, AL_REFERENCE_DISTANCE, (float)refDist);
	alSourcef(oalSource, AL_MAX_DISTANCE, (float)maxDist);
	alSourcef(oalSource, AL_ROLLOFF_FACTOR, (float)rolloff);
	alSourcef(oalSource, AL_CONE_INNER_ANGLE, (float)innerAngle);
	alSourcef(oalSource, AL_CONE_OUTER_ANGLE, (float)outerAngle);
	alSourcef(oalSource, AL_CONE_OUTER_GAIN, (float)outerGain);
	alSourcef(oalSource, AL_GAIN, (float)gain);
}


void QAudioSinkOpenAL::setGain(float newGain)
{
	if (gain == newGain) return; // no change
	gain = newGain;
	
	// Ensure that the OpenAL context doesn't change out from under us.
	shared_mutex_lock lk(g_oal_mutex);
	if (oalSource == 0) return; // don't bother creating one yet... only when we need to play something.
	ensureValidSource();
	if (oalSource == 0) return; // We weren't able to create a valid source, so there's nothing to do.

	// Finally, set the gain.
	alSourcef(oalSource, AL_GAIN, (float)gain);
}


// Recover one buffer that has finished processing.
// We exercise care here to discover any buffers returned twice,
// as has been observed on Mac OS.
// Private, called in loop from play-directly, which holds the locks.
bool QAudioSinkOpenAL::returnOneBuffer () 
{
	ALenum err;
	ALuint reclaim[1];
	alSourceUnqueueBuffers(oalSource, 1, reclaim);
	err = alGetError();
	if (err) {
		log() << "returnOneBuffer: Unqueue error " << err << flush;
		return false;
	}
	ALuint bufid = reclaim[0];
	bool goodToReclaim = true;
	
	for (int j = 0; j < freeBuffers.size(); j++) {
		if (freeBuffers[j] == bufid) {
			log() << "returnOneBuffer: BUF LISTED FREE: " << bufid << flush;
			goodToReclaim = false;
			goto DONE_CHECKS;
		}
	}
	for (int j = 0; j < queuedBuffers.size(); j++) {
		if (queuedBuffers[j] == bufid) {
			log() << "returnOneBuffer: BUF QUEUED TO PLAY: " << bufid << flush;
			goodToReclaim = false;
			goto DONE_CHECKS;
		}
	}
	
	// This last check is a positive one - do we know this buffer id?
	goodToReclaim = false;
	for (int j = 0; j < NUM_BUFFERS; j++) {
		if (bufid == oalBuffers[j]) {
			goodToReclaim = true;
			goto DONE_CHECKS;
		}
	}
	// If we fall through that last test, we didn't find a match.
	if (! goodToReclaim) {
		log() << "returnOneBuffer: BUF NOT MINE: " << bufid << flush;
	}
	
  DONE_CHECKS: 
	if (goodToReclaim) {
		// Healthy... OpenAL is giving back a buffer we think it should.
		freeBuffers.push_back(bufid);
	}
	return goodToReclaim;
}


void QAudioSinkOpenAL::playAudioDirectly(ALvoid* data, ALsizei byteSize)
{
	// Ensure that the OpenAL context doesn't change out from under us.
	shared_mutex_lock lk(g_oal_mutex);

	ALenum	err;
	ALint	freedBufferCount = 0;
	
	// This creates a source if we need one, return true iff it makes a new source.
	bool	newSource = ensureValidSource();
	
	// Now take the lock so we can trust the source.
	unique_mutex_lock lk2(mutex);
	if (oalSource == 0) {
		// XXXXX: need error handling
		log() << "playAudioDirectly():  invalid source";
		return;
	}
	
	// Reclaim buffers that are finished playing.
	if (! newSource ) {
		// We do one at a time here to try to avoid problems with MAC-OS OpenAL
		// It sometimes gives the same buffer twice, and sometimes frees twice internally.
		
		alGetSourcei(oalSource, AL_BUFFERS_PROCESSED, &freedBufferCount);
		err = alGetError();
		if (err) {
			log() << "playAudioDirectly: Cannot check buffers: " << err << flush;
			freedBufferCount = 0;	// Paranoia
		}
		while ( (freedBufferCount > 0) && returnOneBuffer() ) {
			freedBufferCount --;
		}
	}
	
	// Ensure that there is a free buffer.
	if (freeBuffers.size() == 0) {
		log(1) << "playAudioDirectly(): No free buffers" ;
		return;
	}
	
	// Play that funky music.
	ALuint playBuffer = freeBuffers.back();
	freeBuffers.pop_back();
	alBufferData(playBuffer, AL_FORMAT_MONO16, data, byteSize, SAMPLING_RATE);
	queuedBuffers.push_back(playBuffer);
	
	err = alGetError();
	if (err != AL_NO_ERROR) {
		log() << "playAudioDirectly():  ERROR 2a code: " << err;
	}	
	
	// We don't start playing until we've got a couple of buffers to play,
	// to avoid some static if there's lag while getting a new utterance going.
	if (!isStarted) {
		if (queuedBuffers.size() < 2) return;
		isStarted = true;
	}
	// Once started, we give OpenAL everything we've got for it.
	for (int i = 0; i < queuedBuffers.size(); i++) {
		alSourceQueueBuffers(oalSource, 1, &(queuedBuffers[i]));
		err = alGetError();
		if (err != AL_NO_ERROR) {
			log() << "playAudioDirectly():  ERROR 2b (queuing): " << err << flush;
			freeBuffers.push_back(queuedBuffers[i]);
		}
	}
	queuedBuffers.clear();

	// XXXXX: try to get some idea of how many buffers are available
	if (freeBuffers.size() >= NUM_BUFFERS-1) {
		log(7) << "playAudioDirectly(): FREE BUFFER COUNT: " << freeBuffers.size() ;
	}
	
	err = alGetError();
	if (err != AL_NO_ERROR) {
		log() << "playAudioDirectly():  ERROR 2c code: " << err << flush;
	}
	
	int isPlaying = AL_INITIAL;
	alGetSourcei(oalSource, AL_SOURCE_STATE, &isPlaying);
	err = alGetError();
	if (err != AL_NO_ERROR) {
		log() << "playAudioDirectly():  ERROR 2c code: " << err << flush;
	}
	if (isPlaying != AL_PLAYING) {
		alSourcePlay(oalSource);
		// Verify that it worked.
		alGetSourcei(oalSource, AL_SOURCE_STATE, &isPlaying);
		err = alGetError();
		if (err || (isPlaying != AL_PLAYING)) {
			log() << "playAudioDirectly(): FAILED TO START PLAYING " << err << flush;
		}
	}
	
	err = alGetError();
	if (err != AL_NO_ERROR) {
		log() << "playAudioDirectly():  ERROR 3  code: " << err << flush;
	}
	
}

// Oddity: the following returns true if it makes a source.
// If there already is a source, it returns false. 
// (The check for a valid source is oal_source > 0 !)
bool QAudioSinkOpenAL::ensureValidSource()
{
	shared_mutex_lock lk(g_oal_mutex);
	unique_mutex_lock lk2(mutex);

	alGetError ();	/* Clear error state, prevent false positives. */
	
	// If there is no context, then invalidate ourself; there's nothing else to do
	if (g_oal_context == NULL) {
		log() << "ensureValidSource():  no OpenAL context exists" ;
		oalSource = 0;
		return false;
	}

	alcMakeContextCurrent(g_oal_context);
	if (oalSource != 0  &&  oalCount == g_oal_count) {
		// Yippee!!  We have a valid source.
		log(10) << "ensureValidSource():  valid source already exists" ;
		return false;
	}

	// If we reach here, there is a valid context, but we don't have a valid
	// source.  Create one.
	alGenSources(1, &oalSource);
	ALenum err = alGetError();
	if (err != AL_NO_ERROR) {
		log() << "ensureValidSource():  failed to create new source.  code: " << err ;
		oalSource = 0;  // Just to be certain
		return false;
	}
	
	// So far, so good.  Now bring source up-to-date with whatever state we have.
	oalCount = g_oal_count;
	alSource3f(oalSource, AL_POSITION, (float)transform[0], (float)transform[1], (float)transform[2]);
	alSource3f(oalSource, AL_DIRECTION, (float)transform[3], (float)transform[4], (float)transform[5]);
	setInitialSourceProperties();
	
	// XXXXX: I would expect that the default is 1.0... let's play with it.
	alSourcef(oalSource, AL_PITCH, 1.0);

	// Check if everything is good so far.. if not, bail.
	err = alGetError();
	if (err != AL_NO_ERROR) {
		log() << "ensureValidSource():  failed to init source " << oalSource << " code: " << err ;
		alDeleteSources(1, &oalSource);
		oalSource = 0;
		return false;
	}

	// Let's generate some buffers.
	// XXXXX: need error checking... must delete source before bailing
	queuedBuffers.clear();
	freeBuffers.clear();

	alGenBuffers(NUM_BUFFERS, oalBuffers);
	for (int i = 0; i < NUM_BUFFERS; i++) freeBuffers.push_back(oalBuffers[i]);
	err = alGetError();
	if (err != AL_NO_ERROR) {
		log() << "ensureValidSource():  failed to allocate buffers " << oalSource << " code: " << err ;
		alDeleteSources(1, &oalSource);
		freeBuffers.clear();
		oalSource = 0;
		return false;
	}
	
	log(1) << "ensureValidSource():  created source: " << oalSource ;	
	return true;
}

void QAudioSinkOpenAL::printDebugInfo()
{
	boost::shared_ptr<Tickee> strong = source.lock();

	log() 
		<< "printDebugInfo(): " << "\n\t"
		<< "pos( " << transform[0] << "," << transform[1] << "," << transform[2] << ")    dir(" << transform[3] << "," << transform[4] << "," << transform[5] << "\n\t"
		<< "gain: " << gain << "\n\t"
		<< "free/queued buffers: " << freeBuffers.size() << "/" << queuedBuffers.size() << "\n\t"
		<< "refDist: " << refDist << "    maxDist: " << maxDist << "    rolloff: " << rolloff << "\n\t"
		<< "innerAngle: " << innerAngle << "    outerAngle: " << outerAngle << "    outerGain: " << outerGain << "\n\t"
		<< (strong.get() == NULL ? "no audio source" : "audio source: (printed separately)") ;

	if (strong.get() != NULL ) strong->printDebugInfo();
}

void QAudioSinkOpenAL::deleteOpenALResources(int verbosity)
{
	// NB. OAL mutex is held by callers.
	unique_mutex_lock lock(mutex);

	if (oalSource != 0 ) {
		if (oalCount == g_oal_count) {
			// Only delete the source if we're still in the same context as created.
			
			// Help OS-X lose associations between sources and buffers.  This is redundant paranoia.
			alSourceStop(oalSource);
			ALuint dummy[NUM_BUFFERS];
			alSourceUnqueueBuffers(oalSource, NUM_BUFFERS, dummy);

			log(verbosity) << " ** Deleting Source " << oalSource << flush;
			alDeleteSources(1, &oalSource);
		}
		alGetError ();	/* Clear error state, prevent false positives. */
		
		// Buffers span contexts, sources do not span contexts.
		// So delete the buffers if we have nonzero id, since that means we own buffers.
		log(verbosity) << " ** Deleting buffers " << oalSource;
		alDeleteBuffers(NUM_BUFFERS, oalBuffers);
		ALenum err = alGetError();
		if (err != AL_NO_ERROR) {
			log() << "deleteOpenALResources: delete buffers error: " << err << flush;
		}
		
		freeBuffers.clear();
		queuedBuffers.clear();
		oalSource = 0;
	}
}

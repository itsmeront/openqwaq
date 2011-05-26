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
 *  qAudioSinkMixer.cpp
 *  QAudioPlugin
 *
 */

#include "qAudioSinkMixer.hpp"
#include "qLogger.hpp"
using namespace Qwaq;

#include <algorithm>

QAudioSinkMixer::QAudioSinkMixer() 
{
	className = "QAudioSinkMixer"; // for logging
	log() << " ** CREATED" << flush;
	
	outputEvenIfNoInput = true;
}

QAudioSinkMixer::~QAudioSinkMixer()
{
	log() << " ** DESTROYED" << flush;
}

// XXXXX: I'm not locking anything, but I should be!!!!
void QAudioSinkMixer::tick()
{
	int sum[FRAME_SIZE];
	int min=0, max=0;
	int activeSources = 0;
	for (int i=0; i<FRAME_SIZE; i++) sum[i]=0;
	
	// Obtain strong references to all Tickees.  If any weak-ref is now NULL,
	// clean up the list.

	{
		scoped_lock lk(mutex);
		bool needsFixup = false;
		for (vector<weak_tickee>::iterator it = sources.begin(); it != sources.end(); ++it) {
			shared_tickee strong = it->lock();
			if (!strong) {
				// We found a weak-ref that is now NULL... we'll need to clean up later.
				needsFixup = true;
			}
			else if (strong->hasValidBuffer()) {
				// We found a source that has a buffer for us to mix.  So mix it.
				activeSources++;
				short *buf = strong->getBuffer();
				for (int i=0; i<FRAME_SIZE; i++) sum[i] += buf[i];
			}
		}
		
		if (needsFixup) {
			// Iterate through all source-ptrs, keeping only those that still exist
			vector<weak_tickee> newSources;
			for (vector<weak_tickee>::iterator it = sources.begin(); it != sources.end(); ++it) {
				if (!it->expired()) { newSources.push_back(*it); }
			}
			// Finally, swap in the new collection.
			log(2) << "tick() removed " << sources.size() - newSources.size() << 
				" out of " << sources.size() << " expired weak_ptrs";
			sources = newSources;
		}
	}
	
	if (activeSources == 0) {
		if (outputEvenIfNoInput) {
			for (int i=0; i<FRAME_SIZE; i++) buffer[i] = 0;
			hasBuffer = true;
		}
		else { hasBuffer = false; }
		return;
	}
	hasBuffer = true;
		
	for (int i=0; i<FRAME_SIZE; i++) {
		if (sum[i] < min) min = sum[i];
		if (sum[i] > max) max = sum[i];
	}
	
	// XXXXX: HACK! scale values down so they all fit (no clipping).
	// A production algorithm would include some level of hysteresis.
	float maxRatio = (float)max / 32768;
	float minRatio = (float)min / -32767;
	maxRatio = (maxRatio > minRatio) ? maxRatio : minRatio;
	if (maxRatio < 1.0) maxRatio = 1;
	for (int i=0; i<FRAME_SIZE; i++) 
		buffer[i] = (short)(sum[i]/maxRatio);
	hasBuffer = true;
	
	log(5) << "ticked " << activeSources << " out of " << sources.size() << " sources...   max/min = " << max << "/" << min;
}

void QAudioSinkMixer::addSource(shared_tickee src)
{
	log(4) << "adding source (total: " << sources.size() + 1 << ")";
	scoped_lock lk(mutex);
	sources.push_back(src);
}

void QAudioSinkMixer::removeSource(shared_tickee src)
{
	log(4) << "removing source (" << sources.size() << " before removal)";
	{
		scoped_lock lk(mutex);
		for (vector<weak_tickee>::iterator it = sources.begin(); it != sources.end(); it++) {
			if (src == (it->lock())) {
				sources.erase(it);
				return;
			}
		}
	}
	log(2) << "attempted to remove non-existent source";
}

void QAudioSinkMixer::printDebugInfo()
{
	log() 
		<< "printDebugInfo(): " << "\n\t"
		<< sources.size() << " inputs" << flush;
}

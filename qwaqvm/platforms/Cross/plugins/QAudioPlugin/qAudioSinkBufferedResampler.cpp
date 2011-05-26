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
 *  qAudioSinkBufferedResampler.cpp
 *  QAudioPlugin
 *
 */

#include "qAudioSinkBufferedResampler.hpp"
#include "qLogger.hpp"

using namespace Qwaq;


QAudioSinkBufferedResampler::QAudioSinkBufferedResampler() : ring(FRAME_SIZE*sizeof(short)*100)
{
	className = "QAudioSinkBufferedResampler"; // for logging
	inputRate = outputRate = 0;
	resampler = NULL;
	bufferedFrameCount = 1;
	preBuffered = false;
	pushFailed = false;
	
	log() << " ** CREATED" << flush;
}


QAudioSinkBufferedResampler::~QAudioSinkBufferedResampler()
{
	if (resampler) { speex_resampler_destroy(resampler); }
	
	log() << " ** DESTROYED" << flush;
}


void QAudioSinkBufferedResampler::tick()
{
	int bufferSize = FRAME_SIZE*sizeof(short);
		
	if (ring.dataSize() < bufferSize) {
		hasBuffer = preBuffered = false;
	}
	else {
		if (preBuffered) {
			// We already waited a tick, so we're good to go!
			ring.get(buffer, bufferSize);
			hasBuffer = true;
		}
		else {
			// We haven't yet buffered the desired amount, so keep waiting for
			// more data to arrive.
			hasBuffer = false;
			if (ring.dataSize() >= bufferSize * bufferedFrameCount) { preBuffered = true; }
		}
	}
}


void QAudioSinkBufferedResampler::pushRawAudio(short* bufferPtr, int sampleCount)
{
	if (!resampler) { // must lazily create the resampler
		if (inputRate == 0) {
			log() << "pushRawAudio()... no input-rate specified" << flush;
			return;
		}
		if (outputRate == 0) {
			log() << "pushRawAudio()... no output-rate specified" << flush;
			return;
		}
		if (outputRate != 16000) {
			log() << "pushRawAudio()... output-rate must be 16kHz" << flush;
			return;
		}
		int err;
		resampler = speex_resampler_init(1, // mono sound
										inputRate, 
										outputRate, 
										8,  // pretty good quality (ranges from 1-10)
										&err);
		if (!resampler) {
			log() << "pushRawAudio()... resampler creation failed with status: " << err << flush;
			return;
		}
		else {
			log() << "pushRawAudio()... instantiated resampler from " << inputRate << "kHz to " << outputRate << "kHz";
		}
	}
	
	unsigned inSize = sampleCount;
	unsigned outSize = 10000;
	short tempBuf[10000]; // More than enough for 200ms of 44.1kHz sound
	if ((sampleCount / inputRate * outputRate) > 10000) { // Sanity check
		log() << "pushRawAudio()... sample count incorrect for input/output rates: " << sampleCount;
		printDebugInfo();
		return;
	}
	speex_resampler_process_int(resampler, 0, bufferPtr, &inSize, tempBuf, &outSize);
	try {
		ring.put(tempBuf, outSize*sizeof(short));
	} catch (std::string s) {
		// If we're at default verbosity, only log once, otherwise log every time
		log(pushFailed ? 1 : 0) 
			<< "pushRawAudio(): ring-buffer push failed("
			<< (outSize*sizeof(short)) << "/" << ring.dataSize() << "/" << ring.totalSize()
			<< "): " << s << flush;
		pushFailed = true;
	}
}


void QAudioSinkBufferedResampler::setInputSamplingRate(unsigned rate)
{
	if (inputRate == rate) return;
	inputRate = rate;
	if (resampler) {
		speex_resampler_destroy(resampler);
		resampler = NULL;
	}
	// resampler will be lazily created when necessary
}


void QAudioSinkBufferedResampler::setOutputSamplingRate(unsigned rate)
{
	if (outputRate == rate) return;
	outputRate = rate;
	if (resampler) {
		speex_resampler_destroy(resampler);
		resampler = NULL;
	}
	// resampler will be lazily created when necessary
}

// Set the number of frames that should be buffered before starting to play.
// This will take effect the next time that we receive the first frame after 
// being completely drained.
void QAudioSinkBufferedResampler::setBufferedFrameCount(unsigned frameCount)
{
	unsigned bufferSize = FRAME_SIZE*sizeof(short);
	unsigned maxFrames = ring.totalSize() / bufferSize;

	// Will take effect the next time that the buffer is drained.
	if (frameCount > maxFrames) {
		log() << "setBufferedFrameCount(): requested count: " << frameCount
			<< " is greater than the maximum: " << maxFrames << flush;
		bufferedFrameCount = maxFrames;
	}
	else { bufferedFrameCount = frameCount; }
	bufferedFrameCount = (frameCount > maxFrames) ? maxFrames : frameCount;
}
		

void QAudioSinkBufferedResampler::printDebugInfo()
{
	log()
		<< "printDebugInfo(): " << "\n\t"
		<< "input rate: " << inputRate << "\n\t"
		<< "output rate: " << outputRate << flush;
}

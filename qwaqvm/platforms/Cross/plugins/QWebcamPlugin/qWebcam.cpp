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

#include "qWebcam.h"
#include "qLogger.hpp"


using namespace Qwaq;


VideoFrameEvent::VideoFrameEvent(BufferPool2::ptr_type buf)
{
	buffer = buf;
	squeakEvent.bufferSize = buf->getUsedSize();
	squeakEvent.buffer = buf->getPointer();
}



Webcam::Webcam(WebcamParams* params, BufferPool2& bufferPool, FeedbackChannel* ch) : pool(bufferPool), feedbackChannel(ch)
{
	parameters = *params;
}


Webcam::~Webcam()
{
	// We don't destroy the feedback channel; the QPluginFeedbackChannel object
	// in Squeak will destroy the channel is destroyed.
	feedbackChannel = NULL;
}


void
Webcam::grabbedFrame(VideoFrameEvent* f)
{
	if (!f) {
		qerr << endl << qTime() << "Webcam::grabbedFrame(): video frame is NULL";
		return;
	}

	FeedbackEventPtr ptr(f);
	feedbackChannel->push(ptr);
}

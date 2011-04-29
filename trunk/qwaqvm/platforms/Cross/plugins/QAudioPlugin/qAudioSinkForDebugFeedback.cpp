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
 *  qAudioSinkForDebugFeedback.cpp
 *  QAudioPlugin
 *
 */

#include "qAudioSinkForDebugFeedback.hpp"
#include "qLogger.hpp"
using namespace Qwaq;

// See Squeak class QDecodedAudioSegmentFeedback
struct DebugAudioFrame
{
	int type;
	short samples[320];
};
class DebugAudioFrameEvent : public FeedbackEvent
{
public:
	DebugAudioFrameEvent() { squeakEvent.type = 1; } 
	virtual int getSqueakEventSize() { return sizeof(DebugAudioFrame); }
	virtual void fillInSqueakEvent(char* ptr) { *((DebugAudioFrame*)ptr) = squeakEvent; }
	
	DebugAudioFrame squeakEvent;
};


QAudioSinkForDebugFeedback::QAudioSinkForDebugFeedback(FeedbackChannel *feedbackChannel) : Tickee()
{
	className = "QAudioSinkForDebugFeedback"; // for logging
	feedback = feedbackChannel;
	
	log() << " ** CREATED" << flush;
}

QAudioSinkForDebugFeedback::~QAudioSinkForDebugFeedback()
{
	log() << " ** DESTROYED" << flush;
}

void QAudioSinkForDebugFeedback::tick()
{
	boost::shared_ptr<Tickee> strong = source.lock();
	if (strong.get() == NULL || !strong->hasValidBuffer()) {
		hasBuffer = false;
		return;
	}
	if (feedback) {
		DebugAudioFrameEvent *evt = new DebugAudioFrameEvent;
		memcpy(evt->squeakEvent.samples, strong->getBuffer(), 640);
		FeedbackEventPtr p(evt);
		feedback->push(p);
	}
}

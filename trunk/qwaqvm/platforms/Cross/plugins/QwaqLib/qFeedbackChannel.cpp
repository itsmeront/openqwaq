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

#include "qFeedbackChannel.h"
#include "qFeedbackChannel-interface.h"
#include "qLogger.hpp"
#include "qException.h"
#include "sqVirtualMachine.h"

using namespace Qwaq;

extern "C" 
{
	extern struct VirtualMachine* interpreterProxy;
}

FeedbackChannelMap gFeedbackChannels;
static int createdChannel = 0;

////////////////////////////////////////////////////////////////////
// definitions of functions declared in qFeedbackChannel-interface.h
////////////////////////////////////////////////////////////////////

int
qCreateFeedbackChannel(char* address, int semaphoreIndex)
{
	FeedbackChannel** fcptr = (FeedbackChannel**) address;
	*fcptr = new FeedbackChannel(semaphoreIndex);
	if (!*fcptr) {
		qLog() << "qCreateFeedbackChannel(): failed to instantiate FeedbackChannel";
		return QE_ALLOCATION_FAILED;
	}
	createdChannel = 1;

	// Register the FeedbackChannel in the global registry, so that we can
	// be sure to free it if the plugin is unloaded.
	gFeedbackChannels[*fcptr] = boost::shared_ptr<FeedbackChannel>(*fcptr);

	qLog() << "qCreateFeedbackChannel(): created channel with semaphore index: " << semaphoreIndex;

	return QS_OK;
}


void
qDestroyFeedbackChannel(char* address)
{
	FeedbackChannel* fc = *((FeedbackChannel**) address);

	// Remove the 'root pointer' to the channel; it will be destroyed when
	// no-one else is pointing at it.
	gFeedbackChannels.erase(fc);
}



// N.B. qFeedbackChannelReadEvent does /not/ destroy the event.  See
// qFeedbackChannelPopEvent below.

int
qFeedbackChannelReadEvent(char* address)
{
	// If the address is NULL, or if the channel has no queued
	// events, exit early: answer 'nil'.
	FeedbackChannel* fc = *((FeedbackChannel**) address);
	if (!createdChannel) {
		qerr << endl << qTime() << "qFeedbackChannelReadEvent(): no feedback channels created yet!";
		return interpreterProxy->nilObject();
	}
	if (!fc) {
		qerr << endl << qTime() << "qFeedbackChannelReadEvent(): channel address is NULL!";
		return interpreterProxy->nilObject();
	}

	// Get a pointer to the next event in the channel.  If there isn't one, return 'nil'.
	// (this is not an erroneous condition; it happens in normal usage)
	FeedbackEventPtr evt = fc->front();
	if (!evt) return interpreterProxy->nilObject();

	// Instantiate a new ByteArray that will will hold the necessary information
	// about the event.  Answer 'nil' if we can't instantiate the object.
	sqInt cls = interpreterProxy->classByteArray();
	sqInt sz = evt->getSqueakEventSize();
	int oop = interpreterProxy->instantiateClassindexableSize(cls, sz);
	if (oop == interpreterProxy->nilObject()) {
		qerr << endl << qTime() << "qFeedbackChannelReadEvent(): failed to instantiate bytes of size: " << sz;
		return oop;
	}

	// Check for overwriting.  Remember the word following the object.
	sqInt poIdx = (interpreterProxy->byteSizeOf(oop) + 3) >> 2;
	sqInt pastOut = interpreterProxy->fetchLong32ofObject(poIdx, oop);
	// Tell the event to fill in the relevant information, and return the filled-in ByteArray.
	evt->fillInSqueakEvent((char*)interpreterProxy->firstIndexableField(oop));
	// Check for overwriting.  Compare the following word with its old value.
	if (pastOut != interpreterProxy->fetchLong32ofObject(poIdx, oop)) {
		qerr << endl << qTime() << "qFeedbackChannelReadEvent(): fillInSqueakEvent overwrote event object of size " << sz;
		interpreterProxy->primitiveFail();
		return interpreterProxy->nilObject();
	}
	return oop;
}


// N.B. qFeedbackChannelPopEvent is where events get destroyed.  Since we use
// smart pointers to refer to events, the pop() below also reclaims the pushed
// event.

void
qFeedbackChannelPopEvent(char* address)
{	
	FeedbackChannel* fc = *((FeedbackChannel**) address);
	if (!fc) {
		qerr << endl << qTime() << "qFeedbackChannelPopEvent(): channel address is NULL!";
		return;
	}
	fc->pop();
}


void
qFeedbackChannelSetLogging(char* address, int trueOrFalse)
{
	FeedbackChannel* fc = *((FeedbackChannel**) address);
	if (!fc) {
		qerr << endl << qTime() << "qFeedbackChannelSetLogging(): channel address is NULL!";
		return;
	}
	fc->setLogging(trueOrFalse);
}



///////////////////////////////////////
// Definitions of FeedbackEvent methods
///////////////////////////////////////



/////////////////////////////////////////
// Definitions of FeedbackChannel methods
/////////////////////////////////////////

void
FeedbackChannel::releaseAllChannels()
{
	gFeedbackChannels.clear();
}


FeedbackEventPtr
FeedbackChannel::front()
{
	scoped_lock lk(mutex);
	if (queue.empty()) return FeedbackEventPtr();
	else return queue.front();
}


void
FeedbackChannel::pop()
{
	scoped_lock lk(mutex);
	
	if (isLogging) {
		if (queue.empty()) qerr << endl << " popping from empty channel " << (unsigned)this;
		else qerr << endl << " popping " << queue.front()->description() << " from channel " << (unsigned)this;
	}
	
	if (!queue.empty()) queue.pop();
}


void
FeedbackChannel::push(FeedbackEventPtr& ptr)
{
	{
		scoped_lock lk(mutex);
		queue.push(ptr);
	}
	interpreterProxy->signalSemaphoreWithIndex(semaphoreIndex);
	
	if (isLogging) {
		qerr << endl << " pushed event " << ptr->description() << " on feedback-channel " << (unsigned)this;
	}
}


void 
FeedbackChannel::setLogging(bool trueOrFalse)
{ 
	isLogging = trueOrFalse; 
	qLog() << "Turned logging " << (trueOrFalse ? "ON" : "OFF") << 
		" for feedback-channel " << (unsigned)this;
}






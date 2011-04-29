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
 *  qPhoneUser.cpp
 *  QAudioPlugin
 *
 */

#include "QAudioPlugin.h"
#if EXCLUDE_IAX
/* stub functions for QAudioPlugin.c so the plugin links. */
int qIaxInit() { return -1; }
int qIaxShutdown() { return -1; }
unsigned qCreatePhoneUser(char* username, char* password, char* hostname, void* feedbackChannelHandle) { return 0; }
unsigned qDestroyPhoneInterfaceIAX(unsigned handle) { return 1; }
#else /* EXCLUDE_IAX */
#include "qPhoneUser.hpp"
#include "qPhoneEvents.hpp"
#include "qLogger.hpp"

#include "iaxclient.h"

using namespace Qwaq;

boost::mutex PhoneUser::s_Mutex;
unsigned PhoneUser::s_Key = 0;
PhoneUser::map_type PhoneUser::s_Map;


extern "C" {
extern int isIaxInitialized;  // defined in a C file
}

// XXXXX: SOOOOOPER-HACK!!!!  This mofo assumes that the PhoneUser that we want 
// to handle the event (i.e. pass it back on its feedback-channel) exists and
// has key==1.
// A note on return values... iaxclient expects <0 to mean error, ==0 means fall
// through to default event handling (currently no-op except for text events), and
// and >0 to mean sucessfully handled event.  I learnied this from the extensive
// documentation known as "iaxclient_lib.c".  We pretend to always be successful.
int phoneUserIAXCallback(iaxc_event e) 
{
	PhoneUser::ptr_type p = PhoneUser::withKey(1);
	if (!p.get()) {
		qLog() << "phoneUserIAXCallback(): could not find PhoneUser to handle IAX event" << flush;
		return 1;
	}
	p->handleEvent(e);
	return 1;
}
// XXXXX: also a hack
int phoneUserGSMAudioCallback(unsigned char* data, int datalen)
{
	PhoneUser::ptr_type p = PhoneUser::withKey(1);
	if (!p.get()) {
		qLog() << "phoneUserGSMAudioCallback(): could not find PhoneUser to handle audio event" << flush;
		return 1;
	}
	p->handleAudio(data, datalen);
	return 1;
}

int qIaxInit()
{
	/* Don't initialize if we already are. */
	if (isIaxInitialized) {
		qLog() << "qIaxInit(): already initialized" << flush;
		return 0;
	}

	/* "64 calls should be enough for anyone" - J. Gargus */
	if (-1 == iaxc_initialize(64)) {
		qLog() << "qIaxInit(): ERROR: iaxc_initialize() failed" << flush;
		return -1;
	}
	
	iaxc_set_event_callback(phoneUserIAXCallback);
	iaxc_set_gsm_audio_callback(phoneUserGSMAudioCallback);
	if (iaxc_start_processing_thread() <0) {
		iaxc_shutdown();
		qLog() << "qIaxInit(): ERROR while starting processing thread" << flush;
		return -1;
	}
	
	qLog() << "qIaxInit(): obtained output buffer #" << iaxc_get_output_buffer();
	qLog() << "qIaxInit(): success" << flush;
	
	isIaxInitialized = 1; /* true */
	return 0;
}

int qIaxShutdown()
{
	if (!isIaxInitialized) {
		qLog() << "qIaxShutdown(): already shutdown" << flush;
		return 0;
	}

	iaxc_stop_processing_thread();
	iaxc_shutdown();
	
	qLog() << "qIaxShutdown(): success" << flush;
	isIaxInitialized = 0; /* false */
	return 0;
}

unsigned qCreatePhoneUser(char* username, char* password, char* hostname, void* feedbackChannelHandle) 
{
	FeedbackChannel* channel = *((FeedbackChannel**)feedbackChannelHandle);

	PhoneUser *user = new PhoneUser(username, password, hostname, channel);
	if (!user) {
		qLog() << "Failed to create PhoneUser (" << 
			username << ", " <<
			password << ", " <<
			hostname << flush;
		return 0;
	}

	return user->key();
}

unsigned qDestroyPhoneInterfaceIAX(unsigned handle)
{
	PhoneUser::ptr_type p = PhoneUser::withKey(handle);
	if (!p.get()) return 1; //error, could not find phone-user
	PhoneUser::releaseKey(handle);
	return 0;
}


PhoneUser::PhoneUser(char* usernm, char* passwd, char* hostnm, FeedbackChannel* channel) :
	username(usernm),
	password(passwd),
	hostname(hostnm),
	feedback(channel)

{
	addToMap();
	iaxcID = iaxc_register(usernm, passwd, hostnm);
	qLog() << "Created phone user (name=" << usernm << ", host=" << hostnm << ", id=" << iaxcID << ", key=" << key() << ")" << flush;
}

PhoneUser::~PhoneUser()
{
	iaxc_unregister(iaxcID);
	qLog() << "Released phone user (name=" << username << ", host=" << hostname << ", id=" << iaxcID << ", key=" << key() << ")" << flush;
}

void
PhoneUser::handleEvent(iaxc_event& e)
{
	FeedbackEventPtr p;

	// Wrap the iaxclient event so that we can pass it back to Squeak.
	switch(e.type) {
		case IAXC_EVENT_TEXT:
			p = FeedbackEventPtr(new PhoneTextEvent(e.ev.text));
			break;
		case IAXC_EVENT_LEVELS:
			p = FeedbackEventPtr(new PhoneLevelsEvent(e.ev.levels));
			break;
		case IAXC_EVENT_STATE:
			p = FeedbackEventPtr(new PhoneCallStateEvent(e.ev.call));
			break;
		case IAXC_EVENT_NETSTAT:
			p = FeedbackEventPtr(new PhoneNetstatEvent(e.ev.netstats));
			break;
		case IAXC_EVENT_URL:
			p = FeedbackEventPtr(new PhoneUrlEvent(e.ev.url));
			break;
		case IAXC_EVENT_VIDEO:
			p = FeedbackEventPtr(new PhoneVideoEvent(e.ev.video));
			break;
		case IAXC_EVENT_REGISTRATION:
			p = FeedbackEventPtr(new PhoneRegistrationEvent(e.ev.reg));
			break;
		case IAXC_EVENT_AUDIO:
			p = FeedbackEventPtr(new PhoneAudioEvent(e.ev.audio));
			break;
		case IAXC_EVENT_VIDEOSTATS:
			p = FeedbackEventPtr(new PhoneVideostatsEvent(e.ev.videostats));
			break;
		default:
			qLog() << "PhoneUser.handleEvent(): unknown event type: " << e.type << flush;
			return;
	}
	
	// Make sure that we successfully instantiated the feedback-event
	if (!p.get()) {
		qLog() << "PhoneUser.handleEvent(): could not instantiate phone event" << flush;
		return;
	}
	
	// Enqueue the event.
	feedback->push(p);
}

// Return a GSM-encoded frame back to Squeak
void
PhoneUser::handleAudio(unsigned char* data, int datalen)
{
	if (datalen == 0) return; // No frame to process.

	if (datalen != 33) {
		// Not the expected length for a GSM frame
		qLog() << "PhoneUser.handleAudio(): wrong size for GSM frame (expected 33, got " << datalen << ")" << flush;
		return;
	}
	PhoneGSMFrameEvent *evt = new PhoneGSMFrameEvent;
	if (!evt) {
		qLog() << "PhoneUser.handleAudio(): could not instantiate audio event" << flush;
		return;
	}
	memcpy(evt->squeakEvent.data, data, datalen);
	evt->squeakEvent.callNo = iaxc_selected_call();
	FeedbackEventPtr p(evt);
	feedback->push(p);
}
#endif /* EXCLUDE_IAX */

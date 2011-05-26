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
 *  qPhoneEvents.cpp
 *  QAudioPlugin
 *
 */

#include "QAudioPlugin.h"

#if EXCLUDE_IAX
/* stub functions for QAudioPlugin.c so the plugin links. */
int iaxc_call(const char * num) { return -1; }
int iaxc_select_call(int callNo) { return -1; }
int iaxc_selected_call() { return -1; }
int iaxc_quelch(int callNo, int MOH) { return -1; }
int iaxc_unquelch(int call) { return -1; }
long iaxc_write_output_buffer(int index, void * data, long len) { return -1; }
void iaxc_reject_call_number(int callNo) {}
void iaxc_dump_call(void) {}
void iaxc_answer_call(int callNo) {}
void iaxc_send_busy_on_incoming_call(int callNo) {}
void qGenerateTestPhoneEvents(void* wdnnsfch) {}
#else /* EXCLUDE_IAX */
#include "qPhoneEvents.hpp"
#include "qFeedbackChannel.h"

using namespace Qwaq;

void qGenerateTestPhoneEvents(void* feedbackChannelHandle)
{
	FeedbackChannel* channel = *((FeedbackChannel**)feedbackChannelHandle);
		
	PhoneTextEvent* ev1 = new PhoneTextEvent;
	if (ev1) {
		struct iaxc_ev_text *s = &(ev1->squeakEvent.ev);
		s->type = 42;
		s->callNo = 43;
		strcpy(s->message, "PhoneTextEvent: message");

		FeedbackEventPtr p(ev1);
		channel->push(p);
	}
	PhoneLevelsEvent* ev2 = new PhoneLevelsEvent;
	if (ev2) {
		struct iaxc_ev_levels *s = &(ev2->squeakEvent.ev);
		s->input = 0.25;
		s->output = 0.75;
		
		FeedbackEventPtr p(ev2);
		channel->push(p);
	}	
	PhoneCallStateEvent* ev3 = new PhoneCallStateEvent;
	if (ev3) {
		struct iaxc_ev_call_state *s = &(ev3->squeakEvent.ev);
		s->callNo = 42;
		s->state = 43;
		s->format = 44;
		s->vformat = 45;
		strcpy(s->remote, "PhoneCallStateEvent: remote");
		strcpy(s->remote_name, "PhoneCallStateEvent: remote_name");
		strcpy(s->local, "PhoneCallStateEvent: local");
		strcpy(s->local_context, "PhoneCallStateEvent: local_context");

		FeedbackEventPtr p(ev3);
		channel->push(p);
	}
	PhoneNetstatEvent* ev4 = new PhoneNetstatEvent;
	if (ev4) {
		struct iaxc_ev_netstats *s = &(ev4->squeakEvent.ev);
		s->callNo = 42;		
		s->rtt = 43;
		s->local.jitter = 44;
		s->local.losspct = 45;
		s->local.losscnt = 46;
		s->local.packets = 47;
		s->local.delay = 48;
		s->local.dropped = 49;
		s->local.ooo = 50;
		s->remote.jitter = 51;
		s->remote.losspct = 52;
		s->remote.losscnt = 53;
		s->remote.packets = 54;
		s->remote.delay = 55;
		s->remote.dropped = 56;
		s->remote.ooo = 57;
		
		FeedbackEventPtr p(ev4);
		channel->push(p);
	}	
	PhoneUrlEvent* ev5 = new PhoneUrlEvent;
	if (ev5) {
		struct iaxc_ev_url *s = &(ev5->squeakEvent.ev);
		s->callNo = 42;
		s->type = 43;
		strcpy(s->url, "PhoneUrlEvent: url");
		
		FeedbackEventPtr p(ev5);
		channel->push(p);
	}	
	PhoneVideoEvent* ev6 = new PhoneVideoEvent;
	if (ev6) {
		struct iaxc_ev_video *s = &(ev6->squeakEvent.ev);
		s->callNo = 42;
		s->ts = 43;
		s->format = 44;
		s->width = 45;
		s->height = 46;
		s->encoded = 47;
		s->size = 48;
		s->data = (char*)0x42;
		
		FeedbackEventPtr p(ev6);
		channel->push(p);
	}
	
	PhoneRegistrationEvent* ev8 = new PhoneRegistrationEvent;
	if (ev8) {
		struct iaxc_ev_registration *s = &(ev8->squeakEvent.ev);
		s->id = 42;
		s->reply = 43;
		s->msgcount = 44;
		
		FeedbackEventPtr p(ev8);
		channel->push(p);
	}
	PhoneAudioEvent* ev10 = new PhoneAudioEvent;
	if (ev10) {
		struct iaxc_ev_audio *s = &(ev10->squeakEvent.ev);
		s->callNo = 42;
		s->ts = 43;
		s->format = 44;
		s->encoded = 45;
		s->source = 46;
		s->size = 47;
		s->data = (unsigned char*)0x42;
		
		FeedbackEventPtr p(ev10);
		channel->push(p);
	}
	PhoneVideostatsEvent* ev11 = new PhoneVideostatsEvent;
	if (ev11) {
		struct iaxc_ev_video_stats *s = &(ev11->squeakEvent.ev);
		s->callNo = 42;
		s->stats.received_slices = 43;
		s->stats.acc_recv_size = 44;
		s->stats.sent_slices = 45;
		s->stats.acc_sent_size = 46;
		s->stats.dropped_frames = 47;
		s->stats.inbound_frames = 48;
		s->stats.outbound_frames = 49;
		s->stats.avg_inbound_fps = 50.0;
		s->stats.avg_inbound_bps = 51;
		s->stats.avg_outbound_fps = 52.0;
		s->stats.avg_outbound_bps = 53;
		// XXXXX: not sure what to do about the 'start_time' (a 'struct timeval')
		// Ignore for now, because we're not even getting video events.
		
		FeedbackEventPtr p(ev11);
		channel->push(p);
	}
	
}
#endif /* EXCLUDE_IAX */

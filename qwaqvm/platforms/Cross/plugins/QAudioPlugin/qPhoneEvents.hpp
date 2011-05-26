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
 *  qPhoneEvents.hpp
 *  QAudioPlugin
 *
 */

#ifndef __Q_PHONE_EVENTS_HPP__
#define __Q_PHONE_EVENTS_HPP__

#include "qFeedbackChannel.h"

#include "iaxclient.h"

namespace Qwaq
{
	// Wrap the iaxclient event types in a structure that includes their type.
	struct SqueakPhoneTextEvent {
		int type; // IAXC_EVENT_TEXT
		struct iaxc_ev_text ev;
	};
	struct SqueakPhoneLevelsEvent {
		int type; // IAXC_EVENT_LEVELS
		struct iaxc_ev_levels ev;
	};
	struct SqueakPhoneCallStateEvent {
		int type; // IAXC_EVENT_STATE
		struct iaxc_ev_call_state ev;
	};
	struct SqueakPhoneNetstatEvent {
		int type; // IAXC_EVENT_NETSTAT
		struct iaxc_ev_netstats ev;
	};
	struct SqueakPhoneUrlEvent {
		int type; // IAXC_EVENT_URL
		struct iaxc_ev_url ev;
	};
	struct SqueakPhoneVideoEvent {
		int type; // IAXC_EVENT_VIDEO
		struct iaxc_ev_video ev;
	};
	struct SqueakPhoneRegistrationEvent {
		int type; // IAXC_EVENT_REGISTRATION
		struct iaxc_ev_registration ev;
	};
	struct SqueakPhoneAudioEvent {
		int type; // IAXC_EVENT_AUDIO
		struct iaxc_ev_audio ev;
	};
	struct SqueakPhoneVideostatsEvent {
		int type; // IAXC_EVENT_VIDEOSTATS
		struct iaxc_ev_video_stats ev;
	};

	
	// Template to simplify creation of FeedbackEvent subclasses corresponding 
	// to the above events.
	template <class TemplateEvent>
	class PhoneEvent : public FeedbackEvent
	{
	public:
		virtual int getSqueakEventSize() { return sizeof(TemplateEvent); }
		virtual void fillInSqueakEvent(char* ptr) { *((TemplateEvent*)ptr) = squeakEvent; }
		
		TemplateEvent squeakEvent;	
	};

	// Use the template to create the corresponding FeedbackEvent subclasses.
	class PhoneTextEvent : public PhoneEvent<SqueakPhoneTextEvent>
	{ public: 
		PhoneTextEvent() { squeakEvent.type = IAXC_EVENT_TEXT; }
		PhoneTextEvent(iaxc_ev_text &e) { squeakEvent.type = IAXC_EVENT_TEXT; squeakEvent.ev = e; }
		virtual std::string description() { return "PhoneTextEvent"; };
	};
	class PhoneLevelsEvent : public PhoneEvent<SqueakPhoneLevelsEvent>
	{ public: 
		PhoneLevelsEvent() { squeakEvent.type = IAXC_EVENT_LEVELS; }
		PhoneLevelsEvent(iaxc_ev_levels &e) { squeakEvent.type = IAXC_EVENT_LEVELS; squeakEvent.ev = e; }
		virtual std::string description() { return "PhoneLevelsEvent"; };
	};
	class PhoneCallStateEvent : public PhoneEvent<SqueakPhoneCallStateEvent>
	{ public: 
		PhoneCallStateEvent() { squeakEvent.type = IAXC_EVENT_STATE; }
		PhoneCallStateEvent(iaxc_ev_call_state &e) { squeakEvent.type = IAXC_EVENT_STATE; squeakEvent.ev = e; }
		virtual std::string description() { return "PhoneCallStateEvent"; };
	};
	class PhoneNetstatEvent : public PhoneEvent<SqueakPhoneNetstatEvent>
	{ public: 
		PhoneNetstatEvent() { squeakEvent.type = IAXC_EVENT_NETSTAT; }
		PhoneNetstatEvent(iaxc_ev_netstats &e) { squeakEvent.type = IAXC_EVENT_NETSTAT; squeakEvent.ev = e; }
		virtual std::string description() { return "PhoneNetstatEvent"; };
	};
	class PhoneUrlEvent : public PhoneEvent<SqueakPhoneUrlEvent>
	{ public: 
		PhoneUrlEvent() { squeakEvent.type = IAXC_EVENT_URL; }
		PhoneUrlEvent(iaxc_ev_url &e) { squeakEvent.type = IAXC_EVENT_URL; squeakEvent.ev = e; }
		virtual std::string description() { return "PhoneUrlEvent"; };
	};
	class PhoneVideoEvent : public PhoneEvent<SqueakPhoneVideoEvent>
	{ public: 
		PhoneVideoEvent() { squeakEvent.type = IAXC_EVENT_VIDEO; }
		PhoneVideoEvent(iaxc_ev_video &e) { squeakEvent.type = IAXC_EVENT_VIDEO; squeakEvent.ev = e; }
		virtual std::string description() { return "PhoneVideoEvent"; };
	};	
	class PhoneRegistrationEvent : public PhoneEvent<SqueakPhoneRegistrationEvent>
	{ public: 
		PhoneRegistrationEvent() { squeakEvent.type = IAXC_EVENT_REGISTRATION; }
		PhoneRegistrationEvent(iaxc_ev_registration &e) { squeakEvent.type = IAXC_EVENT_REGISTRATION; squeakEvent.ev = e; }
		virtual std::string description() { return "PhoneRegistrationEvent"; };
	};
	class PhoneAudioEvent : public PhoneEvent<SqueakPhoneAudioEvent>
	{ public: 
		PhoneAudioEvent() { squeakEvent.type = IAXC_EVENT_AUDIO; }
		PhoneAudioEvent(iaxc_ev_audio &e) { squeakEvent.type = IAXC_EVENT_AUDIO; squeakEvent.ev = e; }
		virtual std::string description() { return "PhoneAudioEvent"; };
	};
	class PhoneVideostatsEvent : public PhoneEvent<SqueakPhoneVideostatsEvent>
	{ public: 
		PhoneVideostatsEvent() { squeakEvent.type = IAXC_EVENT_VIDEOSTATS; }
		PhoneVideostatsEvent(iaxc_ev_video_stats &e) { squeakEvent.type = IAXC_EVENT_VIDEOSTATS; squeakEvent.ev = e; }
		virtual std::string description() { return "PhoneVideostatsEvent"; };
	};
	
	// XXXXX: Hack: TSTTCPI for reading GSM-encoded frame back into Squeak
	struct SqueakPhoneGSMFrameEvent {
		int type;
		int callNo;
		unsigned char data[33];
	};
	class PhoneGSMFrameEvent : public PhoneEvent<SqueakPhoneGSMFrameEvent>
	{ public:
		PhoneGSMFrameEvent() { squeakEvent.type = 1000000; /* put this in an enum later */ }
		virtual std::string description() { return "PhoneGSMFrameEvent"; };
	};
}

#endif //#ifndef __Q_PHONE_EVENTS_HPP__

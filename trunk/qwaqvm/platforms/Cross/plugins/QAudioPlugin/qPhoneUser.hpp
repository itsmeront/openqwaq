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
 *  qPhoneUser.hpp
 *  QAudioPlugin
 *
 *  Asterisk interface (via iaxclient).
 */

#ifndef __Q_PHONE_USER_HPP__
#define __Q_PHONE_USER_HPP__

#include <string>
using std::string;

#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/locks.hpp>
using boost::shared_ptr;
typedef boost::mutex::scoped_lock scoped_lock;

#include "qFeedbackChannel.h"

#include "iaxclient.h"

namespace Qwaq
{

class PhoneUser
{
	public:
		PhoneUser(char* usernm, char* passwd, char* hostnm, FeedbackChannel* channel);
		~PhoneUser();
		
		//enum status { ERROR=-1, OK };
		
		void handleEvent(iaxc_event& e);
		void handleAudio(unsigned char* data, int datalen);
		
		// MAGIC HERE!!!
		typedef boost::shared_ptr<PhoneUser> ptr_type;
		#include <qMappedResourceBoilerplate.hpp>
		
	protected:
		string username;
		string password;
		string hostname;
		
		int iaxcID;
		
		FeedbackChannel* feedback;
		
}; // class PhoneUser
	
}; // namespace Qwaq

#endif // #ifndef __Q_PHONE_USER_HPP__

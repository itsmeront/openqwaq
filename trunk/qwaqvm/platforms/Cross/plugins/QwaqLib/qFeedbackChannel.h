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

/******************************************************************************
 *
 * qFeedbackChannel.h
 * QwaqLib (cross-platform)
 *
 * Queues up feedback events to be read back into Squeak.  Signals a Squeak
 * semaphore when an event becomes available.  A Squeak Process that is blocked
 * on the semaphore will wake up and read the event in two stages:
 *    Stage 1:
 *       - invoke a primitive that reads back a fixed-size description of the 
 *         event.  The description has two fields:
 *            - event type: integer that is interpreted by the application
 *            - payload size: integer that specifies the size of a variable-
 *              length payload
 *    Stage 2:
 *       - invoke a primitive that reads back the event itself, into two
 *         ByteArray subinstances:
 *            - event header:
 *                 - header size and fields are application- and event-specific
 *                      - plugin and Squeak code must agree on header layout
 *                      - event type from Stage 1 tells Squeak code what 
 *                        type of header to pass in.
 *            - payload
 *                 - payload data is read into a ByteArray that must be at least
 *                   as big as the size specified by Stage 1.
 *
 ******************************************************************************/

#ifndef __Q_FEEDBACK_CHANNEL_H__
#define __Q_FEEDBACK_CHANNEL_H__

#define BOOST_DYN_LINK
#include <queue>
#include <map>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>

namespace Qwaq {
	class FeedbackEvent;
	class FeedbackChannel;
	typedef boost::shared_ptr<FeedbackEvent> FeedbackEventPtr;
	typedef boost::shared_ptr<FeedbackChannel> FeedbackChannelPtr;
	typedef std::map<FeedbackChannel*, FeedbackChannelPtr> FeedbackChannelMap;
	

	class FeedbackEvent
	{
	public:
		// XXXXX: fix me
		virtual int getSqueakEventSize() = 0;
		virtual void fillInSqueakEvent(char* ptr) = 0;
		virtual std::string description() { return ""; };
	};
	

	class FeedbackChannel
	{
	public:
		static void releaseAllChannels();

	public:
		FeedbackChannel(int semInd) : semaphoreIndex(semInd), isLogging(false) { }

		FeedbackEventPtr front();
		void pop();
		void push(FeedbackEventPtr& ptr);
		void setLogging(bool trueOrFalse);

	protected:
		typedef boost::mutex::scoped_lock scoped_lock;

		int semaphoreIndex; // index of the Squeak semaphore to notify when an event arrives
		bool isLogging;
		std::queue<FeedbackEventPtr> queue;
		boost::mutex mutex;
	};

} //namespace Qwaq

#endif //#ifndef __Q_FEEDBACK_CHANNEL_H__

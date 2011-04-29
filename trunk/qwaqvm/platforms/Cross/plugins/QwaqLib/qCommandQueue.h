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
 * qCommandQueue.h
 * QwaqLib (cross-platform)
 *
 * Allows enqueued commands to be processed by worker threads.
 *
 ******************************************************************************/


#ifndef __Q_COMMAND_QUEUE_H__
#define __Q_COMMAND_QUEUE_H__

#include "qSharedQueue.h"
#include "qException.h"
#include <boost/thread/barrier.hpp>
#include <boost/thread/xtime.hpp>
#include <boost/shared_ptr.hpp>

namespace Qwaq {

	class Command;

	class CommandQueue
	{
		typedef boost::shared_ptr<Command> pointerT;

	public:
		void add(pointerT cmd);
		void processNext();

	protected:
		SharedQueue<pointerT> queue;
	};

	// Command is an abstract class... subclasses must 
	class Command
	{
		friend class CommandQueue;
	public:
		Command(bool waitForCompletion = false);
		~Command();

		// Print the elapsed times from when the command was enqueued
		void printElapsedTimes();

	protected:
		virtual void execute(void) = 0;
	
		bool complete;
		boost::barrier* barrier;
		boost::xtime startWaiting;
		boost::xtime startProcessing;
		boost::xtime finishProcessing;
		boost::xtime finishWaiting;

		ExceptionPtr exPtr;
	};




} //namespace Qwaq

#endif //#ifndef __Q_COMMAND_QUEUE_H__

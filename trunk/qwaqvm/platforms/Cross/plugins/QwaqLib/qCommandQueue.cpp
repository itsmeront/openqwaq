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

#include "qCommandQueue.h"
#include "qLogger.hpp"

#include <boost/thread.hpp>

namespace Qwaq
{

	// Put the current thread to sleep for the specified number of microseconds
	void sleep_for(int seconds, int microseconds)
	{
		boost::thread currentThread;
		boost::xtime theTime;
		boost::xtime_get(&theTime, boost::TIME_UTC);
		theTime.sec += seconds;
		if ((theTime.nsec/1000 + microseconds) > 1000000) {
			theTime.sec++;
			theTime.nsec += (microseconds - 1000000)*1000;
		}
		else {
			theTime.nsec += microseconds*1000;
		}
		currentThread.sleep(theTime);
	}

	void CommandQueue::add(pointerT cmd)
	{
		xtime_get(&cmd->startWaiting, boost::TIME_UTC);
		queue.add(cmd);
		if (cmd->barrier) {
			// Presence of barrier indicates that we want to 
			// wait until the command has been processed.
			cmd->barrier->wait();
			sleep_for(0, 1200);
			xtime_get(&cmd->finishWaiting, boost::TIME_UTC);
		}
		qerr.flush();
	}

	void CommandQueue::processNext()
	{
		pointerT cmd = queue.next(); // block until there is another command to process
		xtime_get(&cmd->startProcessing, boost::TIME_UTC);
		cmd->execute();
		cmd->complete = true;
		xtime_get(&cmd->finishProcessing, boost::TIME_UTC);
		cmd->finishWaiting = cmd->finishProcessing; // will be stomped later if someone is actually waiting
		if (cmd->barrier) cmd->barrier->wait();
	}

	Command::Command(bool waitForCompletion) : complete(false), barrier(NULL)
	{
		if (waitForCompletion) {
			barrier = new boost::barrier(2);
			if (!barrier)
				qerr << endl << "Command::Command(): could not instantiate barrier";
		}
	}

	Command::~Command()
	{
		if (barrier) delete barrier;
	}

	void Command::printElapsedTimes()
	{
		qerr << "Elapsed secs: ";

		double seconds = startProcessing.sec - startWaiting.sec;
		seconds += ((double)(startProcessing.nsec - startWaiting.nsec) / 1000000000.0);
		qerr << seconds << " start, ";

		seconds = finishProcessing.sec - startWaiting.sec;
		seconds += ((double)(finishProcessing.nsec - startWaiting.nsec) / 1000000000.0);
		qerr << seconds << " end, ";

		seconds = finishWaiting.sec - startWaiting.sec;
		seconds += ((double)(finishWaiting.nsec - startWaiting.nsec) / 1000000000.0);
		qerr << seconds << " end wait.";
	}

} //namespace Qwaq

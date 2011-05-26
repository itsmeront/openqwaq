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
 *  qTicker.cpp
 *  QAudioPlugin
 *
 *  tick.er (n): one who ticks a tickee 
 */

#include "qTicker.hpp"
#include "qTickee.hpp"
#include "qLogger.hpp"
#include "qThreadUtils.hpp"
using namespace Qwaq;

#include <boost/thread/locks.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
typedef boost::mutex::scoped_lock scoped_lock;

static Ticker* ticker;

extern "C" {

#include "sqVirtualMachine.h"
extern struct VirtualMachine* interpreterProxy;

static unsigned long tickCount = 0;
static unsigned long badTickCount = 0;
static unsigned long long targetTime;
char squashAudioBleats = 0; // for debugging

// Static function that is passed to interpreterProxy->addHighPriorityTickee
void rootTickee(void) 
{
	unsigned long long nowUsecs;
	int errorMsecs;

	if (qGetLogVerbosity() >= 7) {
		if (!ticker->queryIsRunning())
			qLog() << "got tick while not running" << flush;
		else if (!tickCount)
			qLog() << "got first tick" << flush;
		else if (!(tickCount % 250))
			qLog() << "got tick" << flush;
	}

	if (!ticker->queryIsRunning())
		return;

	nowUsecs = interpreterProxy->utcMicroseconds();
	targetTime += ticker->queryInterval() * 1000;
	ticker->tick();

	// This is weird, but if I just use:
	//    errorMsecs = (nowUsecs - targetTime) / 1000;
	// then errorMsecs can be some really big number (at least with Microsoft Visual C++).
	// I probably just don't know my C/C++ well enough.
	if (nowUsecs > targetTime)
		errorMsecs = (nowUsecs - targetTime) / 1000;
	else 
		errorMsecs = (targetTime - nowUsecs) / -1000;

	// All of this is to check how accurate our ticks are
	tickCount++;
	if (!squashAudioBleats) {
		if (errorMsecs > 2) {
			if (errorMsecs > 20000) { 
				// User probably closed the laptop lid, or something like that.
				qLog() << "rootTickee(): VERY LARGE CLOCK ERROR (" << errorMsecs << "msecs)... resetting target time" << flush;
				targetTime = nowUsecs;
			}
			else {
				badTickCount++;
				if (errorMsecs > 40) {
					// The OpenAL sinks buffer one 20ms frame worth of sound, so as long as the 
					// error is less that 40ms we should be glitch-free.
					qLog() << "rootTickee():  WAITED WAY TOO LONG: " << errorMsecs << "msecs" << flush;
				}
			}
		}
		if (tickCount % 100 == 0) {
			// If >= 5% of ticks took too long, log it.
			if (badTickCount >= 5)
				qLog() << "rootTickee():  " << badTickCount << "% of ticks took >2ms too long" << flush;
			badTickCount = 0;
		}
	}
}

}  // extern "C"

Ticker::Ticker()
{
	verbosity = 0;
	interval = 100; //milliseconds
	isRunning = false;
}


Ticker::~Ticker()
{
	stop();  // just to make sure
}


void Ticker::start(void)
{
	qLog() << "STARTING TICKER" << flush;
	if (isRunning) { // already running
		qerr << "  (already running)" << endl;
		return;
	}

	ticker = this;
	targetTime = interpreterProxy->utcMicroseconds();
	interpreterProxy->addHighPriorityTickee(rootTickee,interval);
	isRunning = true;		

	qLog(1) << "finished starting ticker" << flush;
}


void Ticker::stop(void)
{
	qLog() << "STOPPING TICKER" << flush;
	if (!isRunning) { // already stopped
		qerr << "  (already stopped)" << endl;
		return;
	}
	isRunning = false;
	interpreterProxy->addHighPriorityTickee(rootTickee,0);

	qLog(1) << "finished stopping ticker" << flush;
}


void Ticker::setInterval(unsigned msecs)
{
	interval = msecs;
	if (isRunning)
		interpreterProxy->addHighPriorityTickee(rootTickee,interval);
}


void Ticker::addTickee(StrongTickee tickee)
{
	if (!tickee.get()) {
		qLog() << "Ticker::addTickee():  tickee is NULL!!" << endl;
		return;
	}

	unsigned priority = tickee->tickPriority();
	if (priority<0 || priority>TICKER_PRIORITY_LEVELS) {
		qLog() << "Ticker::addTickee():   invalid priority: " << priority << endl;
		throw "bad priority";
	}

	// We don't bother to check we already have a reference to the Tickee, since 
	// this method is only called when the Tickee is being instantiated.
	WeakTickeeVect &weaks = tickees[priority];
	scoped_lock lk(mutex);
	weaks.push_back(tickee);
}	


void Ticker::tick(void)
{
	qLog(8) << "(tick)" << flush;

	// We will fill this with strong references to the tickees that we will tick.
	StrongTickeeVect strongs;

	{
		// Only lock mutex while obtaining strong references.
		scoped_lock lk(mutex);
		for (unsigned priority = TICKER_PRIORITY_LEVELS; priority > 0; priority--) {
			obtainStrongRefsForPriority(priority, strongs);
		}
	}

	// Tick those tickees!
	for (StrongTickeeVect::iterator it = strongs.begin(); it != strongs.end(); it++) {
		(*it)->tick();
	}	
}


// Fill 'strongs' with all of the valid references at the specified priority level.
void Ticker::obtainStrongRefsForPriority(unsigned priority, StrongTickeeVect& strongs)
{
	// This should never happen
	if (priority<0 || priority>TICKER_PRIORITY_LEVELS) {
		qLog() << "Ticker::obtainStrongRefsForPriority():   invalid priority: " << priority << endl;
		throw "bad priority";
	}

	WeakTickeeVect &weaks = tickees[priority];
	if (weaks.size() > 0) {
		qLog(9) << "ticker priority " << priority << ":  scheduling " << weaks.size() << " tickees" << flush;

		WeakTickeeVect::iterator it = weaks.begin();
		while (it != weaks.end()) {
			try {
				// Attempt to get a strong reference to the tickee; we 
				// will fail if the weak_ptr no longer references a 
				// valid object.
				StrongTickee st(*it);
				strongs.push_back(st);
				it++;
			}
			catch (...) { 
				// The WeakTickee no longer references a valid object, so
				// erase it from the list.  erase() returns the appropriate
				// iterator to continue looping.
				it = weaks.erase(it);
			}
		}		
	}
}


bool Ticker::queryIsRunning() { return isRunning; }
unsigned Ticker::queryInterval() { return interval; }

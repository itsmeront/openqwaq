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
 *  qTicker.hpp
 *  QAudioPlugin
 *
 *  tick.er (n): one who ticks a tickee
 */

#ifndef __Q_TICKER_HPP__
#define __Q_TICKER_HPP__

#include <vector>
#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/thread/thread.hpp>

using std::vector;
using boost::weak_ptr;
using boost::shared_ptr;

const int TICKER_PRIORITY_LEVELS = 10;

namespace Qwaq
{

class Tickee;

class Ticker
{
	protected:
		typedef weak_ptr<Tickee> WeakTickee;
		typedef	shared_ptr<Tickee> StrongTickee;
		typedef std::vector<WeakTickee> WeakTickeeVect;
		typedef std::vector<StrongTickee> StrongTickeeVect;

	public:
		Ticker();
		~Ticker();
		void start();
		void stop();
		void setVerbosity(unsigned v) { verbosity = v; }
		void setInterval(unsigned msecs);

		// Under normal conditions, tick() is not called directly.  Instead,
		// it is called by rootTickee(), which itself is called from the VM's
		// heartbeat, which may or may not run in a different thread.  However,
		// for debugging it is useful to run tick() under Squeak control.
		// Hence, tick() is public instead of protected.
		void tick(void);
		// extern "C" { static void rootTickee(); }
		// these two are for the convenience of rootTickee
		bool queryIsRunning();
		unsigned queryInterval();

		void addTickee(StrongTickee tickee);

	protected:
		WeakTickeeVect tickees[TICKER_PRIORITY_LEVELS+1];
		boost::mutex mutex;

		unsigned verbosity;
		unsigned interval;
		bool isRunning;

		void obtainStrongRefsForPriority(unsigned priority, StrongTickeeVect& strongs);
};

};

#endif // #ifndef __Q_TICKER_HPP__

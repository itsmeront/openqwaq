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
 *  qThreadUtils.cpp
 *  QwaqLib (cross-platform)
 *
 */
 
#include "qThreadUtils.hpp"
using namespace Qwaq;

#ifdef WIN32
#include <windows.h>
#endif

// Priority ranges from 0 (low) to 10 (high)
// Return 0 for success, negative int for failure
int Qwaq::qSetThreadPriority(boost::thread& t, int priority)
{
#ifdef WIN32
	int winPriority;
	if (priority >= 10) winPriority = THREAD_PRIORITY_TIME_CRITICAL;
	else if (priority == 9) winPriority = THREAD_PRIORITY_HIGHEST;
	else if (priority > 5) winPriority = THREAD_PRIORITY_ABOVE_NORMAL;
	else if (priority == 5) winPriority = THREAD_PRIORITY_NORMAL;
	else if (priority > 0) winPriority = THREAD_PRIORITY_BELOW_NORMAL;
	else winPriority = THREAD_PRIORITY_LOWEST;

	HANDLE winThread = (HANDLE)t.native_handle();
	if (!SetThreadPriority(winThread, winPriority)) return -2;
#else
	struct sched_param sched;
	sched.sched_priority = (priority > 5) ? sched_get_priority_max(SCHED_FIFO) : sched_get_priority_min(SCHED_FIFO);
	int result = pthread_setschedparam(t.native_handle(), SCHED_FIFO, &sched);
	if (result != 0) return -2;
#endif
	return 0;
}

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
 *  qEventTimeLogger.hpp
 *  QwaqLib (cross-platform)
 *
 */


#ifndef __Q_EVENT_TIME_LOGGER_HPP__
#define __Q_EVENT_TIME_LOGGER_HPP__

#include <boost/thread/mutex.hpp>

/* gcc version 4.1.2 20080704 (Red Hat 4.1.2-46) blows up compiling ctype defs
 * if sqVirtualMachine.h is included early.
 */
#include "sqVirtualMachine.h"

namespace Qwaq
{
	
class QEventTimeLogger
{
	public:
		QEventTimeLogger(int maxEntriesCount);
		~QEventTimeLogger();
		
		void add(int id);
		void add(int id, usqLong timestamp);
		
		void getInto(void* bytes, int byteSize);
		void getIntoOop(sqInt oop);
		sqInt getIntoNewOop();
		
	protected:
		#pragma pack(push, 1)
		struct QEventTimeLoggerEntry
		{
			int appDataID;
			usqLong timestamp;
		};
		struct QEventTimeLoggerHeader
		{
			int version;
			int numEntries;
			int numDiscarded;
			int res1, res2, res3, res4, res5;  // reserved
		};
		#pragma pack(pop)
		
		QEventTimeLoggerEntry* entries;
		unsigned count;
		unsigned maxCount;
		unsigned discarded;
		
		boost::mutex mutex;
};

}; // namespace Qwaq

#endif // #ifndef __Q_EVENT_TIME_LOGGER_HPP__

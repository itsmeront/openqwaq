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
 *  qTickee.hpp
 *  QAudioPlugin
 *
 *  tick.ee (n): one who is ticked by a ticker
 */

#ifndef __Q_TICKEE_HPP__
#define __Q_TICKEE_HPP__

#include "qBuffer.h"
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/thread/locks.hpp>

/* gcc version 4.1.2 20080704 (Red Hat 4.1.2-46) blows up compiling ctype defs
 * if sqMemoryAccess.h is included early.
 */
#include "sqMemoryAccess.h"

using boost::shared_ptr;
typedef boost::mutex::scoped_lock scoped_lock;

const int FRAME_SIZE = 320;

namespace Qwaq
{

typedef BufferPool2::Buffer Buffer;
typedef BufferPool2::ptr_type BufferPtr;

class Tickee;
typedef boost::shared_ptr<Tickee> shared_tickee;
typedef boost::weak_ptr<Tickee> weak_tickee;
//bool weak_tickee_eq(weak_tickee const &p, weak_tickee const &q) { return !(p<q) && !(q<p); }

class Tickee
{
	public:
		Tickee();
		virtual ~Tickee();
	 
		// Compute and cache audio buffer (so that other's may access it via getBuffer());
		// it is cached until the next time tick() is called.  Also, perform the approriate
		// side-effect actions (these will vary widely between subclasses); 
		virtual void tick() = 0;
		virtual unsigned tickPriority() = 0;
		//BufferPtr getBuffer() { return buffer; }
		virtual short* getBuffer() { return buffer; }
		bool hasValidBuffer() { return hasBuffer; }
		
		virtual void addSource(shared_ptr<Tickee> src) = 0;
		virtual void removeSource(shared_ptr<Tickee> src) = 0;
		virtual void printDebugInfo() = 0;
		
		virtual int genericControl(int ctlType, int ctlVal) { return 0; }
		
		virtual sqInt getEventTimings();
		
	protected:
		// MAGIC HERE!!!
		typedef shared_tickee ptr_type;
		#include <qMappedResourceBoilerplate.hpp>
		
	protected:
		// Wrapper around qLog() that identifies this object
		std::ostream& log(int verbosity=0);
		
		//BufferPtr buffer;
		short buffer[FRAME_SIZE];
		//short *buffer;
		bool hasBuffer;
		const char* className;  //for logging
};



};

#endif // #ifndef __Q_TICKEE_HPP__

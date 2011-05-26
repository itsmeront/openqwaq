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
 *  qTickee.cpp
 *  QAudioPlugin
 *
 *  tick.ee (n): one who is ticked by a ticker
 */

#include "qTickee.hpp"
#include "qTicker.hpp"
#include "qLogger.hpp"

#include "sqVirtualMachine.h"

using namespace Qwaq;

extern Ticker g_Ticker;
extern "C" { extern struct VirtualMachine* interpreterProxy; }

boost::mutex Tickee::s_Mutex;
unsigned Tickee::s_Key = 0;
Tickee::map_type Tickee::s_Map;

Tickee::Tickee() : className("Tickee")
{
	// XXXXX: Debugging: allocate some extra space with a known 
	// bit-pattern on each side of the buffer.  
/*
	buffer = (short*) malloc((FRAME_SIZE + 50) * sizeof(short));
	if (!buffer) {
		qLog() << "Tickee::Tickee() :  cannot allocate buffer" << flush;
		throw "Tickee::Tickee() :  cannot allocate buffer";
	}
	for (int i=0; i < FRAME_SIZE+50; i++) {
		buffer[i] = 5461;  //bits: 1010101010101
	}
	buffer = buffer+25;
*/

	// see qMappedResourceBoilerplate.hpp... this registers all Tickee 
	// sub-instances in a map that Squeak primitives can use to look them up.
	addToMap();
	
	// Currently, all new instances are immediately added to the ticker
	// in qCreateSink().  Since we always do this, it might make sense to 
	// move it here.  Anyway, now you know where it happens.
}


Tickee::~Tickee()
{
//XXXXX: for debugging... must also change constructor/class-definition
//	if (buffer != NULL) free(buffer-25); // see comment in constructor
	hasBuffer = false; 	
}

std::ostream& Tickee::log(int verbosity)
{
	return qLog(verbosity) << className << "[" << key() << "]::";
}


// For now, only Speex-sinks support this.
sqInt Tickee::getEventTimings()
{
	interpreterProxy->primitiveFailFor(PrimErrUnsupported);
	return 0;
}

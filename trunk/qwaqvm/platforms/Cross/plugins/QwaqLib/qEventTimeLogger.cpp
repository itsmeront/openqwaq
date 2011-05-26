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
 *  qEventTimeLogger.cpp
 *  QwaqLib (cross-platform)
 *
 */

#include "qEventTimeLogger.hpp"
using namespace Qwaq;

typedef boost::mutex::scoped_lock scoped_lock;
extern "C" { extern struct VirtualMachine* interpreterProxy; }

QEventTimeLogger::QEventTimeLogger(int maxEntriesCount)
{
	entries = NULL;
	count = 0;
	maxCount = maxEntriesCount;
	discarded = 0;
}


QEventTimeLogger::~QEventTimeLogger()
{
	scoped_lock lk(mutex);
	if (entries) free(entries);	
}


void QEventTimeLogger::add(int id)
{
	add(id, interpreterProxy->utcMicroseconds());
}


void QEventTimeLogger::add(int id, usqLong time)
{
	scoped_lock lk(mutex);

	// There's no room for this entry; discard it.
	if (!entries || count >= maxCount) {
		discarded += 1;
		// If we haven't been asked for output for a long time,
		// free the memory used to store entries.
		if (entries && (discarded > (maxCount*2))) {
			free(entries);
			entries = NULL;
			count = 0;
		}
	}
	// There's room for the entry, so stash it.
	else {
		entries[count].appDataID = id;
		entries[count].timestamp = time;
		count++;
	}
}


void QEventTimeLogger::getInto(void* bytes, int byteSize)
{
	scoped_lock lk(mutex);
	
	if (!bytes) {
		interpreterProxy->primitiveFailFor(PrimErrBadArgument);
		return;
	}
	if (byteSize < (sizeof(QEventTimeLoggerHeader) + (count * sizeof(QEventTimeLoggerEntry)))) {
		interpreterProxy->primitiveFailFor(PrimErrBadArgument);
		return;
	}
	
	QEventTimeLoggerHeader* hdr = (QEventTimeLoggerHeader*)bytes;
	hdr->version = 1;
	hdr->numEntries = count;
	hdr->numDiscarded = discarded;
	hdr->res1 = hdr->res2 = hdr->res3 = hdr->res4 = hdr->res5 = 0;
	
	// Someone is requesting data, but we haven't been storing the data!
	// Presumably they'll ask again soon, so start recording.
	if (!entries) {
		hdr->numEntries = -1;
		entries = (QEventTimeLoggerEntry*) malloc(maxCount * sizeof(QEventTimeLoggerEntry));
		if (!entries) {
			// Couldn't allocate.
			interpreterProxy->primitiveFailFor(PrimErrNoCMemory);
		}
	}
	else {
		void* ptr = (char*)bytes + sizeof(QEventTimeLoggerHeader);
		memcpy(ptr, entries, count * sizeof(QEventTimeLoggerEntry));
	}
	
	count = discarded = 0;
}


void QEventTimeLogger::getIntoOop(sqInt oop)
{
	if (!interpreterProxy->isBytes(oop)) {
		interpreterProxy->primitiveFailFor(PrimErrBadArgument);
		return;
	}
	void* bytes = interpreterProxy->firstIndexableField(oop);
	int byteSize = interpreterProxy->byteSizeOf(oop);
	if (interpreterProxy->failed()) return; 
	getInto(bytes, byteSize);
}


sqInt QEventTimeLogger::getIntoNewOop()
{
	sqInt cls = interpreterProxy->classByteArray();
	sqInt byteSize = count * sizeof(QEventTimeLoggerEntry) + sizeof(QEventTimeLoggerHeader);
	sqInt oop = interpreterProxy->instantiateClassindexableSize(cls, byteSize);
	if (oop == interpreterProxy->nilObject()) {
		interpreterProxy->primitiveFailFor(PrimErrNoMemory);
		return 0;
	}
	getIntoOop(oop);
	return oop;
}

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

#include "qBuffer.h"
#include "qLogger.hpp"

using namespace Qwaq;
typedef boost::mutex::scoped_lock scoped_lock;



void
BufferPool2::Buffer::copyFrom(size_t sz, unsigned char* ptr)
{
	memcpy(pointer, ptr, sz);
}


void
BufferPool2::Buffer::copyTo(size_t sz, unsigned char* targetPtr)
{
	memcpy(targetPtr, pointer, sz);

}


void
BufferPool2::Buffer::copyFromOffsetTo(size_t offset, size_t sz, unsigned char* targetPtr)
{
	memcpy(targetPtr, pointer+offset, sz);
}


BufferPool2::Buffer::Buffer(BufferPool2& p, size_t full, size_t used, unsigned char* ptr) 
	: pool(p), fullSize(full), usedSize(used), pointer(ptr)
{

}


BufferPool2::Buffer::~Buffer()
{
	pool.returnBuffer(*this);
}


BufferPool2::~BufferPool2()
{
	clear();
	// XXXXX: small race-condition here... ideally we would retain the lock that
	// we obtain in clear(), or use a recursive mutex.  But anyway, we shouldn't
	// be destroying the pool unless we know we're done with it.
	{
		scoped_lock lk(mutex);
		if (usedCount > 0) {
			std::string s("~BufferPool2(): pool destroyed while buffers still exist");
			qerr << endl << qTime() << s;
			throw s;
		}
	}
}


void
BufferPool2::clear()
{
	scoped_lock lk(mutex);
	map_it_type it;
	for (it = available.begin(); it != available.end(); ++it) {
		delete[] it->second;
	}
	available.clear();
	freeCount = 0;
	freeBytes = 0;
}


void
BufferPool2::returnBuffer(BufferPool2::Buffer& b)
{
	scoped_lock lk(mutex);

	// paranoia: guard against inserting empty buffer
	if (b.fullSize == 0) return;

	available.insert(std::make_pair(b.fullSize, b.pointer));
	adjustUsedBytesBy(-((int)b.fullSize));
}


BufferPool2::ptr_type
BufferPool2::getBuffer(size_t minimumSize)
{
	scoped_lock lk(mutex);

	map_it_type it = available.lower_bound(minimumSize);
	unsigned char* ptr;
	size_t sz;

	// Try to use a free buffer, but allocate one if necessary
	if (it == available.end()) {
		// Need to allocate more space
		sz = minimumSize;
		ptr = new unsigned char[sz];
		if (!ptr) sz = 0; // allocation failed
		else {
			// We'll adjust free vs. used below in allocateBuffer()
			++freeCount;
			freeBytes += sz;
		}
	}
	else {
		// Use the existing buffer
		sz = it->first;
		ptr = it->second;
		available.erase(it);
	}

	return allocateBuffer(sz, minimumSize, ptr);
}


BufferPool2::ptr_type
BufferPool2::allocateBuffer(size_t fullSize, size_t usedSize, unsigned char* ptr)
{
	Buffer* buf = NULL;
	if (ptr != NULL) {
		// Attempt to allocate a new Buffer
		buf = new Buffer(*this, fullSize, usedSize, ptr);
		if (!buf) {
			// Couldn't allocate buffer for some reason...
			// avoid leaking memory
			available.insert(std::make_pair(fullSize, ptr));
		}
		else {
			// Success... adjust the used/free stats
			adjustUsedBytesBy(fullSize);
		}
	}
	return ptr_type(buf);
}


void
BufferPool2::adjustUsedBytesBy(int amountUsed)
{
	usedBytes += amountUsed;
	freeBytes -= amountUsed;
	if (amountUsed > 0) {
		++usedCount;
		--freeCount;
	}
	else if (amountUsed < 0) {
		--usedCount;
		++freeCount;
	}
}

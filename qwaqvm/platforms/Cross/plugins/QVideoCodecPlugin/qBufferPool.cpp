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
 * qBufferPool.cpp
 * QVideoCodecPlugin
 *
 * Maintains a pool of buffers.  Allows you to request a buffer of (at least) the
 * specified size, and return it to the pool afterward.  If no buffer in the pool 
 * is big enough, a new one is allocated.
 *
 * Thread-safe.
 */

#include "qBufferPool.h"
#include <iostream>

typedef boost::mutex::scoped_lock scoped_lock;

namespace Qwaq
{
	BufferPool::BufferPool() : totalBytes(0) { }

	BufferPool::~BufferPool()
	{
		std::set<Buffer*>::iterator it;
		for (it = freeBuffers.begin(); it != freeBuffers.end(); it++) {
			delete *it;
		}
		for (it = usedBuffers.begin(); it != usedBuffers.end(); it++) {
			delete *it;
		}
	}

	Buffer* BufferPool::getBuffer(int byteSize)
	{
		scoped_lock lk(mutex);
		std::set<Buffer*>::iterator it;
		// Look through the free buffers for one that's big enough.
		for (it = freeBuffers.begin(); it != freeBuffers.end(); it++) {
			Buffer* buf = *it;
			if (buf->maxSize >= byteSize) {
				freeBuffers.erase(it);
				usedBuffers.insert(buf);
				buf->usedSize = byteSize;
				return buf;
			}
		}
		// If we reached this point, we didn't find a buffer big enough.
		// Allocate one.
		Buffer* buf = new Buffer(byteSize);
		if (buf == NULL) return NULL;
		if (buf->maxSize == 0) {
			// Allocation failed
			delete buf;
			return NULL;
		}
		totalBytes += buf->maxSize;
		usedBuffers.insert(buf);
		return buf;
	}

	void BufferPool::putBuffer(Buffer* buf)
	{
		freeBuffers.insert(buf);
		std::set<Buffer*>::iterator it = usedBuffers.find(buf);
		if (it == usedBuffers.end()) {
			// THIS SHOULD NOT HAPPEN (if it does, you screwed up)!!!
			std::cerr << "\nQwaq::BufferPool::Couldn't find used buffer" << std::endl;
			return;
		}
		usedBuffers.erase(it);
	}
}

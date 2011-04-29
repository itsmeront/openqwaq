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
 * qBufferPool.h
 * QVideoCodecPlugin
 *
 * Maintains a pool of buffers.  Allows you to request a buffer of (at least) the
 * specified size, and return it to the pool afterward.  If no buffer in the pool 
 * is big enough, a new one is allocated.
 *
 * Thread-safe.
 */

#ifndef __Q_BUFFER_POOL_H__
#define __Q_BUFFER_POOL_H__

#define BOOST_DYN_LINK
#include <set>
#include <boost/thread/mutex.hpp>

namespace Qwaq {

	// Buffers don't destroy their 'buf'... that is up to the BufferPool that created it.
	// When a Buffer is returned to the BufferPool, that means that no-one else is using it,
	// and the BufferPool may manage it as it sees fit.
	struct Buffer
	{
		// Allocates the specified number of bytes; if unsuccessful, its size is zero.
		Buffer(int byteSize)
		{
			buf = new unsigned char[byteSize];
			maxSize = usedSize = ((buf == NULL) ? 0 : byteSize);
		}
		~Buffer()
		{
			if (buf != NULL) delete[](buf);
			buf = NULL;
		}
		size_t usedSize;
		size_t maxSize;
		unsigned char* buf;
	};


	class BufferPool
	{
	public:
		BufferPool();
		~BufferPool();
		Buffer* getBuffer(int byteSize);
		void putBuffer(Buffer* buffer);

	protected:
		std::set<Buffer*> usedBuffers;
		std::set<Buffer*> freeBuffers;
		size_t totalBytes;
		boost::mutex mutex;
	};
} //namespace Qwaq
#endif //#ifndef __Q_BUFFER_POOL_H__

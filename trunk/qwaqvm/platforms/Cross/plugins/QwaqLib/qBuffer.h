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

/******************************************************************************
 *
 * qBufferPool2.h
 * QwaqLib (cross-platform)
 *
 * Creates a thread-safe way to obtain buffers of memory from a pool.  These 
 * buffers are then reused instead of allocating a new buffer each time.  When
 * a buffer is no longer referenced, it is automatically returned to the pool
 * (thus avoiding memory leaks).
 *
 ******************************************************************************/

#ifndef __Q_BUFFER_POOL_2_H__
#define __Q_BUFFER_POOL_2_H__

#include <map>
using std::pair;
using std::multimap;

#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

namespace Qwaq
{

class BufferPool2
{
public:

	// A sized chunk of memory.  These may not be allocated explicitly; 
	// clients may obtain a (smart pointer to a) Buffer only via 
	// BufferPool2::getBuffer().  When the Buffer is no longer referenced,
	// its memory is automatically returned to the pool.
	class Buffer
	{
	friend class BufferPool2;

	public:
		~Buffer();

		// pointer is only guaranteed to remain valid during the lifetime
		// of the buffer that it is obtained from
		unsigned char* getPointer(void) { return pointer; }

		size_t getFullSize(void) { return fullSize; }
		size_t getUsedSize(void) { return usedSize; }
		void copyFrom(size_t sz, unsigned char* ptr);
		void copyTo(size_t sz, unsigned char* ptr);
		void copyFromOffsetTo(size_t offset, size_t sz, unsigned char* ptr);

	protected:
		// The only way to create a Buffer is via BufferPool2::getBuffer()
		explicit Buffer(const Buffer& b);  
		Buffer(BufferPool2& p, size_t full, size_t used, unsigned char* ptr);
		
		BufferPool2& pool;
		size_t fullSize;
		size_t usedSize;
		unsigned char* pointer;
	};

public:
	typedef std::multimap<size_t, unsigned char*> map_type;
	typedef map_type::iterator map_it_type;
	typedef boost::shared_ptr<Buffer> ptr_type;

	// This is the main operation that will be used by clients of BufferPool2
	ptr_type getBuffer(size_t minimumSize);

	BufferPool2() : usedCount(0), freeCount(0), usedBytes(0), freeBytes(0) { }
	~BufferPool2();
	void clear(void);

	int getUsedCount() { return usedCount; }
	int getFreeCount() { return freeCount; }
	int getTotalCount() { return usedCount + freeCount; }
	size_t getUsedBytes() { return usedBytes; }
	size_t getFreeBytes() { return freeBytes; }
	size_t getTotalBytes() { return usedBytes + freeBytes; }

protected:
	// Called when Buffer's destructor is called
	void returnBuffer(Buffer& b);

	// Helper methods
	ptr_type allocateBuffer(size_t fullSize, size_t usedSize, unsigned char* ptr);
	void adjustUsedBytesBy(int amountUsed);

	map_type available;
	int usedCount;
	int freeCount;
	size_t usedBytes;
	size_t freeBytes;

	boost::mutex mutex;
};

typedef BufferPool2::ptr_type BufferPtr;

} //namespace Qwaq

#endif //#ifndef __Q_BUFFER_POOL_2_H__

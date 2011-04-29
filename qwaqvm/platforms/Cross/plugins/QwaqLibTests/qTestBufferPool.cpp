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

#include "qTestBufferPool.h"
#include "qBuffer.h"

using namespace Qwaq;

typedef BufferPool2::ptr_type bufptr;

// Basic allocation and auto-cleanup.
void testBufferPool_1(void)
{
	BufferPool2 pool;
	{
		// Allocate 3 buffers, and verify the
		// usage stats.
		bufptr p1 = pool.getBuffer(1000);
		{
			bufptr p2 = pool.getBuffer(1500);
			bufptr p3 = pool.getBuffer(1700);
			assert(pool.getTotalCount() == 3);
			assert(pool.getTotalBytes() == 4200);
		}

		// Two pointers just went out-of-scope; ensure 
		// that they cleaned up after themselves.
		assert(pool.getUsedBytes() == 1000);
		assert(pool.getFreeBytes() == 3200);
		assert(pool.getUsedCount() == 1);
		assert(pool.getTotalCount() == 3);
		assert(pool.getTotalBytes() == 4200);

		// We should be able to find one suitable free
		// buffer, but we have to allocate another one.
		bufptr p4 = pool.getBuffer(1600);
		bufptr p5 = pool.getBuffer(1600);
		assert(pool.getTotalCount() == 4);
		assert(pool.getTotalBytes() == 5800);
	}
	// All pointers have gone out of scope... assert 
	// that the poll has all of its memory back
	assert(pool.getUsedCount() == 0);
	assert(pool.getFreeCount() == 4);
	assert(pool.getUsedBytes() == 0);
	assert(pool.getFreeBytes() == 5800);

	// Ensure that the pool's memory is cleared properly.
	pool.clear();
	assert(pool.getUsedCount() == 0);
}

// Test assignment between buffer pointers.
void testBufferPool_2(void)
{
	BufferPool2 pool;

	bufptr p1;
	{
		bufptr p2 = pool.getBuffer(500);
		p1 = p2;
	}
	// Only one pointer has gone out of scope...
	assert(pool.getUsedCount() == 1);
	assert(pool.getUsedBytes() == 500);

	// The pool will have to allocate another 
	// buffer before the memory held by the pointer
	// is returned to the pool.
	p1 = pool.getBuffer(500);
	assert(pool.getUsedCount() == 1);
	assert(pool.getUsedBytes() == 500);
	assert(pool.getFreeBytes() == 500);
}

// Verify destructor throws exception when there are outstanding buffers
void testBufferPool_3(void)
{
	bufptr p1;
	try {
		BufferPool2 pool;
		p1 = pool.getBuffer(10); // Only a small memory leak
	}
	catch (...) {
		// Caught exception, as expected.  The program will crash
		// when the bufptr goes out of scope, because it will try
		// to return its memory to the already-destroyed pool.
		return; 
	}
	assert(0);
}


// Test multithreading (not yet implemented)
void testBufferPool_4(void) { }
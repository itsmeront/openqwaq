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
 *  qRingBuffer.cpp
 *  QwaqLib
 *
 */

#include "qRingBuffer.hpp"
#include "qLogger.hpp"
using namespace Qwaq;

#include <string>
using std::string;

#include <boost/thread/locks.hpp>
typedef boost::mutex::scoped_lock scoped_lock;

QRingBuffer::QRingBuffer(int size)
{
	_read = _write = _storedDataSize = 0;
	_totalDataSize = size;
	_data = (char*)malloc(_totalDataSize);
	if (!_data) {
		_totalDataSize = 0;
		return;
	}
}

QRingBuffer::~QRingBuffer()
{
	free(_data);
}

void 
QRingBuffer::put(void* data, int size)
{
	scoped_lock lk(_mutex);
	if (_totalDataSize < size) throw string("ring-buffer is too small for data");
	
	// Get pointers to memory to copy to/from.
	char* dst = _data+_write;
	char* src = (char*)data;
		
	// Copy the data.  We might need to wrap around.
	int used = _totalDataSize - _write;
	if (used > size) used = size;
	memcpy(dst, src, used);
	if (used < size) memcpy(_data, src+used, size-used);
	
	// Bookkeeping
	_write = (_write + size) % _totalDataSize;
	_storedDataSize += size;
	if (_storedDataSize > _totalDataSize) {
		// We wrote over some old data.  Adjust both the stored data-size,
		// and the position of the read-cursor.
		_storedDataSize = _totalDataSize;
		_read = _write;
	}
}

void
QRingBuffer::get(void* data, int size)
{
	scoped_lock lk(_mutex);
	if (_storedDataSize < size) throw string("ring-buffer get(): not enough data available");
	if (size == 0) return;
	
	// Get pointers to memory to copy to/from.
	char* dst = (char*)data;
	char* src = _data+_read;

	// Copy the data.  We might need to wrap around.
	int used = _totalDataSize - _read;	// size of contiguous block
	if (used > size) used = size;			// only use what we need
	memcpy(dst, src, used);
	if (used < size) memcpy(dst+used, _data, size-used);
	
	// Bookkeeping.
	_read = (_read + size) % _totalDataSize;
	_storedDataSize -= size;
}

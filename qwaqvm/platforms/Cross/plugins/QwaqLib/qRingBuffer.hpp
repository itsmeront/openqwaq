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
 *  qRingBuffer.hpp
 *  QwaqLib
 *
 */

#ifndef __Q_RING_BUFFER_HPP__
#define __Q_RING_BUFFER_HPP__

#include <boost/thread/mutex.hpp>

namespace Qwaq {

class QRingBuffer
{
	public:
		QRingBuffer(int size);
		~QRingBuffer();

		// Answer the total amount currently in the ring-buffer.
		int dataSize() { return _storedDataSize; }
		int totalSize() { return _totalDataSize; }
		
		bool hasData() { return dataSize() > 0; }
		void put(void* data, int size);
		void get(void* data, int size);
		void clear();

	protected:
		char* _data;
		int _totalDataSize;
		int _storedDataSize;
		int _read;
		int _write;
		boost::mutex _mutex;
};

}; // namespace Qwaq

#endif // #ifndef __Q_RING_BUFFER_HPP__

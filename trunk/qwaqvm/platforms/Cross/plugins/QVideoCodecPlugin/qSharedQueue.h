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
 * qSharedQueue.h
 * QVideoCodecPlugin
 *
 * Thread-safe queue.
 */


#ifndef __Q_SHARED_QUEUE_H__
#define __Q_SHARED_QUEUE_H__

#define BOOST_DYN_LINK
#include <queue>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

namespace Qwaq {

	template <class Elem>
	class SharedQueue
	{
		typedef boost::mutex::scoped_lock scoped_lock;

	public: 
		// Take the first element off of the queue and return it.
		// If no first element exists, block until one is added.
		Elem next();
		// Add an element to the queue.
		void add(Elem e);

	protected:
		std::queue<Elem> queue;
		boost::mutex mutex;
		boost::condition condition;
	};


	template <class Elem>
	Elem SharedQueue<Elem>::next()
	{
		scoped_lock lk(mutex);
		while (queue.empty()) condition.wait(lk);
		Elem result = queue.front();
		queue.pop();
		return result;
	}

	template <class Elem>
	void SharedQueue<Elem>::add(Elem e)
	{
		{
			scoped_lock lk(mutex);
			queue.push(e);
		}
		condition.notify_one();
	}
} //namespace Qwaq
#endif //#ifndef __Q_SHARED_QUEUE_H__
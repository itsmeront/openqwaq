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
 *  qMappedResourceBoilerplate.hpp
 *  QAudioPlugin
 *
 *  This code snippet is intended to be inserted into a class declaration.  By 
 *  following a few simple rules (described below), new class instances are 
 *  automatically registered in a map with an automatically-assigned integer key.
 *  The map contains a reference-counting shared-pointer to the object.
 *
 *  The typical usage is for plugin-side objects that Squeak code needs to refer
 *  to in repeated primitive calls.  Why the shared-pointers?  The (ill-conceived?)
 *  idea is to simplify multithreaded code.  The Squeak code can release the object
 *  at any time, but if it is being used elsewhere (i.e. by another thread) then it
 *  will not be destroyed immediately, only when its ref-count reaches zero.  This
 *  is used, for example, by QAudioPlugin "tickees" so that Squeak can immediately
 *  release them even if it might be in use by the high-priority ticker thread.
 *
 *  Requirements/Usage:
 *      - client class must already #include std::map, boost::shared_ptr, and boost::mutex::scoped_lock
 *		- instances of client classes MUST be allocated on the heap via 'new'.
 *      - 'ptr_type' must be defined by client class
 *			- eg: 'typedef boost::shared_ptr<CLIENT_CLASS> ptr_type;
 *		- use 'addToMap()' in the client class constructor
 *      - use 'withKey()' to obtain the client class instance with the given key.
 *		- use 'releaseKey()' when the client class instance is no longer needed.
 */

public:
	unsigned key() { return m_Key; }
	ptr_type ptr() { return withKey(m_Key); }
	
protected:
	typedef std::map<unsigned, ptr_type> map_type;
	typedef map_type::iterator iterator_type;
	
	unsigned m_Key;

	static boost::mutex s_Mutex;
	static map_type s_Map;
	static unsigned s_Key;  // Would like "s_Key = 1;" but compiler barfs.
	static unsigned nextKey() {
		// Valid range is 1..UINT_MAX.  Here because I can't init s_Key in-line.
		if (s_Key == 0) s_Key = 1;  // 
	
		// XXXXX: if your map is full, you're screwed
		while(true) {
			iterator_type it = s_Map.find(s_Key);
			if (it == s_Map.end()) return s_Key;  // found an empty slot
			if (++s_Key == UINT_MAX) s_Key = 1;
		}
	}

public:
	static ptr_type withKey(unsigned key) {
		scoped_lock lk(s_Mutex);
		iterator_type it = s_Map.find(key);
		ptr_type result;
		if (it != s_Map.end()) result = it->second;
		return result;
	}
	
	static void releaseKey(unsigned key) {
		scoped_lock lk(s_Mutex);
		iterator_type it = s_Map.find(key);
		if (it != s_Map.end()) s_Map.erase(it);
	}
	
	static void releaseAll() {
		scoped_lock lk(s_Mutex);
		s_Map.clear();
	}
	
private:
	void addToMap() {
		scoped_lock lk(s_Mutex);
		m_Key = nextKey();
		s_Map[m_Key] = ptr_type(this);
	}

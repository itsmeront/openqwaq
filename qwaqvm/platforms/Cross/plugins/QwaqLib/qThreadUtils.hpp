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
 *  qThreadUtils.hpp
 *  QwaqLib (cross-platform)
 *
 */

#ifndef __Q_THREAD_UTILS_HPP__
#define __Q_THREAD_UTILS_HPP__

#include <boost/thread/thread.hpp>

namespace Qwaq 
{

// Priority ranges from 0 (low) to 10 (high).
// Return 0 for success, negative int for failure
int qSetThreadPriority(boost::thread& t, int priority);

}; // namespace Qwaq

#endif // #ifndef __Q_THREAD_UTILS_HPP__

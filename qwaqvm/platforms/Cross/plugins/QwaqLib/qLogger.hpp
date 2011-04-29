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
 *  qLogger.hpp
 *  QAudioPlugin
 *
 *  Deprecating 'qLogger.h', since it is a C++ header.
 */

#ifndef __Q_LOGGER_HPP__
#define __Q_LOGGER_HPP__

#include "qLogger.h"

#include <string>
#include <iostream>
using std::endl;
using std::flush;
extern std::ostream qerr;

void qLogToFile(std::string fileName);

// Start a log entry (put a new-line and the current time).
// Returns an ostream to continue logging onto.
std::ostream& qLog();

// If verbosity <= the current verbosity level, do the same
// as qLog().  Otherwise simply answer a null ostream that
// silently gobbles any output.
std::ostream& qLog(int verbosity);

#endif // #ifndef __Q_LOGGER_HPP__

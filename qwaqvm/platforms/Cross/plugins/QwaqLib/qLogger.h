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
 * qLogger.h
 * QwaqLib (cross-platform)
 *
 * Provide a simple logging facility.  'qerr' acts like 'cerr', except that
 * it may log to a file instead.  If no file has been specified, it logs to 
 * 'cerr'.
 *
 * In order to use this in a plugin, you must call qInitLogging() and
 * qShutdownLogging() from initialise/shutdownModule(), respectively.
 *
 ******************************************************************************/


#ifndef __Q_LOGGER_H__
#define __Q_LOGGER_H__

#include <stdarg.h>

#ifdef __cplusplus
extern "C" { 
#endif

void qInitLogging();
void qShutdownLogging();
void qLogToFile(const char* fileName);
void qSetLogVerbosity(int verbosityLevel);
int qGetLogVerbosity();

// Hacky functions so that we can still log from C files (not only C++)
void qLogFromC(const char* stringToLog);
void qLogResultFromC(const char* stringToLog, int result);

// Better, more general log-from-C.
void qprintf(const char* format_str, ...);
void qvprintf(const char* format_str, va_list args);

// Current time in format: Www Mmm dd hh:mm:ss yyyy
char* qTime();

#ifdef __cplusplus
}
#endif

#endif //#ifndef __Q_LOGGER_H__

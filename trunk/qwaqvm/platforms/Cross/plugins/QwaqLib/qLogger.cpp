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

#include "qLogger.hpp"

extern "C" {

#include "sqVirtualMachine.h"
extern struct VirtualMachine* interpreterProxy;

}

#include <fstream>
#include <iostream>
//#include <windows.h>
#include <time.h>

std::ostream qerr(NULL);
std::ostream qnull(NULL);
static std::ofstream *gpFileStream;
static std::string gFileName;
int gVerbosity;
static void cacheLogTime();

void qInitLogging()
{
	gpFileStream = NULL;
	gFileName = "";
	qerr.rdbuf(std::cerr.rdbuf());
	gVerbosity = 0;
	/* update the cached log time every second on the second. */
	interpreterProxy->addSynchronousTickee(cacheLogTime, 1000, 1000);
}

// If we currently have a file open, close it.
static void closeLogFile()
{
	if (gpFileStream != NULL) {
		qerr.rdbuf(std::cerr.rdbuf());
		gpFileStream->close();
		delete gpFileStream;
		gpFileStream = NULL;
	}
}

void qShutdownLogging()
{
	interpreterProxy->addSynchronousTickee(cacheLogTime, 0, 0);
	qerr.flush();
	closeLogFile();
}

void qLogToFile(std::string fileName)
{
	if (gFileName == fileName) return; // nothing has changed
	qerr	<< endl
			<< "Changing error output from: " 
			<< (gFileName.empty() ? "cerr" : gFileName)
			<< "  to: "
			<< (fileName.empty() ? "cerr" : fileName);			
	gFileName = fileName;

	// If we currently have a file open, close it.
	closeLogFile();

	// If a new fileName is specified, attempt to open and use
	// the file with that name.
	if (gFileName.empty()) return;  // no file was specified.
	gpFileStream = new std::ofstream(gFileName.c_str(), std::ios::app);
	if (gpFileStream != NULL)
		qerr.rdbuf(gpFileStream->rdbuf());

	qerr	<< endl
			<< "Error output changed to: "
			<< (gFileName.empty() ? "cerr" : gFileName);
}


void qLogToFile(const char* fileName)
{
	std::string s(fileName);
	qLogToFile(s);
}


void qSetLogVerbosity(int verbosityLevel)
{
	qLog() << "changing log verbosity from: " << gVerbosity << "  to: " << verbosityLevel << flush;
	gVerbosity = verbosityLevel;
}


int qGetLogVerbosity() { return gVerbosity; }


static char cached_log_time[26];

static void
cacheLogTime()
{
	time_t rawtime;
	time(&rawtime);
#if defined(WIN32)
	strcpy(cached_log_time,ctime(&rawtime));
#else
	ctime_r(&rawtime,cached_log_time);
#endif
	cached_log_time[24] = 0; // hack: trim off carriage return at end of string
}

// Current time in format: Www Mmm dd hh:mm:ss yyyy
char* qTime() { return cached_log_time; }


std::ostream& qLog()
{
	return qLog(gVerbosity);
}


std::ostream& qLog(int verbosity)
{
	if (verbosity <= gVerbosity) {
		qerr << endl << qTime() << " :  ";
		return qerr;
	}
	return qnull;
}


void qLogFromC(const char* stringToLog)
{
	qLog() << stringToLog << flush;
}

void qLogResultFromC(const char* stringToLog, int result)
{
	qLog() << stringToLog << result << flush;
}

void qprintf(const char* format_str, ...)
{
	va_list args;
	va_start(args, format_str);
	qvprintf(format_str, args);
	va_end(args);
}

void qvprintf(const char* format_str, va_list args)
{
	char buffer[10000];
	int result = vsprintf(buffer, format_str, args);
	if (result < 0) {
		qerr << "[[** qvprintf() failure... exceeded buffer size of 10KB **]]";
	}
	else {
		qerr << buffer;
	}
}

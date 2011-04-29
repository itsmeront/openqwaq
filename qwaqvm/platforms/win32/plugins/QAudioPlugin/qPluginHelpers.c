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
 *  qPluginHelpers.c
 *  QAudioCodecPlugin
 *
 */

#include "QAudioPlugin.h"
#include "qLogger.h"

int isIaxInitialized;

int qInitModule(void)
{
	isIaxInitialized = 0; /* false */

	qInitLogging();
/*	qLogToFile("QAudioPlugin.log");   /* We now explicitly set the log-file from Squeak */
	qOalInit();
	qIaxInit();
	return 1; /* true */
}
int qShutdownModule(void)
{
	qTickerStop();
	qIaxShutdown();
	qOalShutdown();
	qShutdownLogging();
	return 1; /* true */
}


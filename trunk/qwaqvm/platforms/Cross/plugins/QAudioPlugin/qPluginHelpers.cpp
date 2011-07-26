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
 *  qPluginHelpers.cpp
 *  QAudioCodecPlugin
 *
 */

#include "QAudioPlugin.h"
#include "qLogger.hpp"
#include "qTickee.hpp"

extern "C" {
#include "qLibAVLogger.h"
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}

#if !EXCLUDE_IAX
int isIaxInitialized;
#endif

int qInitModule(void)
{
#if !EXCLUDE_IAX
	isIaxInitialized = 0; /* false */
#endif

	qInitLogging();
//	qLogToFile("QAudioPlugin.log");   /* We now explicitly set the log-file from Squeak */
	qOalInit();
#if !EXCLUDE_IAX
	qIaxInit();
#endif
//	qPortAudioInit();

	av_log_set_callback(q_av_log_callback);
	av_register_all();
	avcodec_init();
	
	return 1; /* true */
}
int qShutdownModule(void)
{
	qLog() << "SHUTDOWN QAudioPlugin" << flush;
	qTickerStop();
	Qwaq::Tickee::releaseAll();
#if !EXCLUDE_IAX
	qIaxShutdown();
#endif
	qOalShutdown();
//	qPortAudioShutdown();
	qShutdownLogging();
	return 1; /* true */
}

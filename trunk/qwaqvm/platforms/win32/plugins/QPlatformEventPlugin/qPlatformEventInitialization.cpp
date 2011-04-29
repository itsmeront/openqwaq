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

#include "QPlatformEventPlugin.h"
#include "qLogger.hpp"
#include "qFeedbackChannel.h"

#if defined(WIN32)
#include "qPlatformEventWin32.h"
#elif defined(__APPLE__)
#include "qPlatformEventOSX.h"
#endif

using namespace Qwaq;

void qInitModule(void)
{
	qInitLogging();
	qLogToFile("QPlatformEventPlugin.log");

	int err;

#if defined(WIN32)
	err = qInitModuleWin32();
#elif defined(__APPLE__)
	err = qInitModuleOSX();
#else
	qerr << endl << "QPlatformEventPlugin: qInitModule() could not determine current platform";
	err = 1;
#endif

	if(err) {
		qerr << endl << "QPlatformEventPlugin: qInitModule() encountered an error: " << err;
	}
}

void qShutdownModule(void)
{
	qerr << endl << qTime() << " :   Shutting down QPlatformEventPlugin" << endl;

	FeedbackChannel::releaseAllChannels();
	qerr << endl << "    - released feedback channels";

	qShutdownLogging();
}
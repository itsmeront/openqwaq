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
 * qVideoQuickTimePlatformSpecific.c
 * QVideoCodecPluginQT
 *
 */

#include "qVideoQuickTime.h"
#include "../QVideoCodecPlugin/qVideoCodecPlatformSpecific.h"
#include "../QVideoCodecPlugin/qVideoCommon.h"

/* Declared in ../QwaqVideoCodecPlugin/qVideoCodecPlatformSpecific.h */
void qInitPlatform(void)
{
	long qtVersion;
	OSErr err = InitializeQTML(0);

	if (err != noErr) {
		fprintf(QSTDERR, "\nqInitPlatformAndAPI(): InitializeQTML() failed");
		TerminateQTML(); //not sure if this is necessary
		qQuickTimeMajorVersion = qQuickTimeMinorVersion = qQuickTimeMinorMinorVersion = 0;
		return;
	}
	EnterMovies();

	err = Gestalt(gestaltQuickTimeVersion, &qtVersion);
	if (err != noErr) {
		fprintf(QSTDERR, "\nqInitPlatformAndAPI(): GESTALT() failed");
		TerminateQTML();
		qQuickTimeMajorVersion = qQuickTimeMinorVersion = qQuickTimeMinorMinorVersion = 0;
		return;
	}

	qQuickTimeMajorVersion = (qtVersion & 0xff000000) >> 24;
	qQuickTimeMinorVersion = (qtVersion & 0x00f00000) >> 20;
	qQuickTimeMinorMinorVersion = (qtVersion & 0x000f000) >> 16;
}

/* Declared in ../QwaqVideoCodecPlugin/qVideoCodecPlatformSpecific.h */
void qShutdownPlatform(void)
{
	TerminateQTML();
}

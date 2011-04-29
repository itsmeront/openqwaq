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
 * qVideoCommon.h
 * QVideoCodecPlugin
 *
 */

#ifndef __Q_VIDEO_COMMON_H__
#define __Q_VIDEO_COMMON_H__

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
extern FILE* QSTDERR;

#ifdef __cplusplus
extern "C"
{
#endif

/* qVideoCodecStartup: Initialize the plugin.
   Arguments: none
   Return value: none
*/
void qVideoCodecStartup(void);

/* qVideoCodecShutdown: Initialize the plugin.
   Arguments: none
   Return value: none
*/
void qVideoCodecShutdown(void);

/* qSetErrorLog: Set the filename of the error log
   Arguments: name, the name of the file (or an empty string to use stderr)
   Return value: zero for success, -1 for error
*/
int qSetErrorLog(char* fileName, int fileNameLength);


/* We will be extending this quite a bit to support different bitrates, etc. */
typedef struct QVideoArgs
{
	char isBigEndian[4]; // only first byte is uses; rest are for alignment
	unsigned char codecType[4];
	long timeScale;
	int width;
	int height;
	int bitRate;
	double frameRate;
	unsigned char flags[4];
} QVideoArgs; 

#ifdef __cplusplus
} //extern "C"
#endif


#endif //#ifndef __Q_VIDEO_COMMON_H__

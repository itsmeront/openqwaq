/*
 * Project OpenQwaq
 *
 * Copyright (c) 2011, 3d Immersive Collaboration Consultants, LLC, All Rights Reserved
 *
 * qVideoFreePlatformSpecific.c
 * QVideoCodecPluginFree
 *
 * Created by Josh Gargus on 5/30/11.
 */
 
 
 
/* this is in Cross for now, but if we really need platform-specific initialization,
    create variants in "Mac OS", "win32", etc., and reference them in the appropriate
	Makefiles. */
 
#include "qVideoCodecPlatformSpecific.h"
#include "qVideoCommon.h"

/* Declared in ../QVideoCodecPlugin/qVideoCodecPlatformSpecific.h */
void qInitPlatform(void)
{
	/* nothing needs to be done */
}

/* Declared in ../QVideoCodecPlugin/qVideoCodecPlatformSpecific.h */
void qShutdownPlatform(void)
{
	/* nothing needs to be done */
}
 


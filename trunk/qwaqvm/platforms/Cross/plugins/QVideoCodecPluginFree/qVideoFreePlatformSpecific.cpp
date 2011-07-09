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

// Everything needs to be extern "C".  Originally, this was a C file, but I renamed 
// it to be a C++ file because otherwise VisualStudio has trouble compiling it.
extern "C" {
#include "qVideoCodecPlatformSpecific.h"
#include "qVideoCommon.h"
#include "qLibAVLogger.h"

#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"

/* Declared in ../QVideoCodecPlugin/qVideoCodecPlatformSpecific.h */
void qInitPlatform(void)
{
	av_log_set_callback(q_av_log_callback);
	av_register_all();
	avcodec_init();
}

/* Declared in ../QVideoCodecPlugin/qVideoCodecPlatformSpecific.h */
void qShutdownPlatform(void)
{
	/* nothing needs to be done */
}


/* The internet gives conflicting info about this.  According to some sources, it exists
   on OS X, and indeed if I type "man posix_memalign" then it shows up (but I still can't
   link).  Others say that the OS X ABI is 16-byte aligned, and so posix_memalign(); they
   also say that it's safe to define your own that simply calls malloc().
*/
#if defined __APPLE__
#include <memory.h>
int posix_memalign(void **memptr, size_t alignment, size_t size)
{
	*memptr = malloc(size);
	return *memptr ? 0 : ENOMEM;
}
#endif 

} // extern "C"
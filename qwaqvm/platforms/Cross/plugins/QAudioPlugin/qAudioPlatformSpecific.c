/*
 * Project OpenQwaq
 *
 * Copyright (c) 2011, 3d Immersive Collaboration Consultants, LLC, All Rights Reserved
 *
 */
 
/*
 * qAudioPlatformSpecific.c
 * QAudioPlugin
 *
 * Created by Josh Gargus on 6/20/11.
 */
 
 
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
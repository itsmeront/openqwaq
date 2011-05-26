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
 *  qAudioOpenAL.h
 *  QAudioPlugin
 *
 */

#if __linux__
# include <AL/al.h>
# include <AL/alc.h>
#else
# include <OpenAL/al.h>
# include <OpenAL/alc.h>
#endif


#ifdef __cplusplus
#include <boost/thread/shared_mutex.hpp>

extern ALCdevice* g_oal_device;
extern ALCcontext* g_oal_context;
extern unsigned g_oal_count; // number of contexts that have been created
extern int g_oal_error;
extern boost::shared_mutex g_oal_mutex;
#endif //#ifdef __cplusplus


#ifndef __Q_AUDIO_OPENAL_H__
#define __Q_AUDIO_OPENAL_H__

typedef enum {
	Q_OAL_NO_ERROR = 0,					/* Everything is fine */
	Q_OAL_OPENAL_ERROR = 1,				/* There was an error in the OpenAL API call */
	Q_OAL_CONTEXT_EXISTS = 2,			/* Context creation failed because a context already exists */
	Q_OAL_CONTEXT_NOT_FROM_HERE = 3		/* Context deletion failed because the context was not created by the plugin */
} Q_AUDIO_OPENAL_ERRORS;

/*
 * Functions called directly by the generated Slang code for QAudioPlugin.
 */ 
#ifdef __cplusplus
extern "C" {
#endif //#ifdef __cplusplus
void qOalInit();
void qOalShutdown();
int qOalGetPluginError();
void* qOalCreateContext();
int qOalDestroyContext(void* ctxt);
void qOalSetAudioDevice(char* deviceName);
#ifdef __cplusplus
}
#endif //#ifdef __cplusplus

#endif /* #define __Q_AUDIO_OPENAL_H__ */

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
 *  qAudioOpenAL.cpp
 *  QAudioPlugin
 *
 */

#include "qAudioOpenAL.h"
#include "qLogger.hpp"

#include <stddef.h>

#include <boost/thread/locks.hpp>

typedef boost::unique_lock<boost::shared_mutex> unique_mutex_lock;
typedef boost::shared_lock<boost::shared_mutex> shared_mutex_lock;

char* g_oal_devicename;
ALCdevice* g_oal_device;
ALCcontext* g_oal_context;
unsigned g_oal_count; // number of contexts that have been created
int g_oal_error;
boost::shared_mutex g_oal_mutex;

/* These actually do the work, but with no locking. */
void* qPrivateOalCreateContext();
int qPrivateOalDestroyContext(void* ctxt);

void qOalInit()
{
	unique_mutex_lock lk(g_oal_mutex);
	g_oal_devicename = NULL;
	g_oal_device = NULL;
	g_oal_context = NULL;
	g_oal_count = 0;
	g_oal_error = Q_OAL_NO_ERROR;
}

int qOalGetPluginError() {
	unique_mutex_lock lk(g_oal_mutex);
	int result = g_oal_error;
	g_oal_error = Q_OAL_NO_ERROR;
	return result;
}


void qOalShutdown()
{
	if (g_oal_context != NULL) {
		qOalDestroyContext(g_oal_context);
		g_oal_context = NULL;
	}

	// If we put this before  qOalDestroyContext(), we'll deadlock when 
	// that function also tries to grab the mutex.
	unique_mutex_lock lk(g_oal_mutex);

	if (g_oal_device != NULL) {
		alcCloseDevice(g_oal_device);
		g_oal_device = NULL;
	}
	
	if (g_oal_devicename != NULL) {
		free(g_oal_devicename);
		g_oal_devicename = NULL;
	}
}

void* qOalCreateContext() 
{
	qLog() << endl << "ATTEMPTING TO CREATE OPENAL CONTEXT...  " << flush;
	unique_mutex_lock lk(g_oal_mutex);
	return qPrivateOalCreateContext();
}

void* qPrivateOalCreateContext() 
{
	/* Ensure that a context doesn't already exist. */
	if (g_oal_device != NULL || g_oal_context != NULL) {
		g_oal_error = Q_OAL_CONTEXT_EXISTS;
		qerr << "context already exists: 0x" << g_oal_context << endl << flush;
		return NULL;
	}
	/* Create a device; ensure that we were successful. */
	g_oal_device = alcOpenDevice(g_oal_devicename);
	if (!g_oal_device) {
		g_oal_error = Q_OAL_OPENAL_ERROR;
		qerr << "OpenAL error: failed to open device: " << (g_oal_devicename ? g_oal_devicename : "(NULL)") << endl << flush;
		return NULL;
	}
	/* Create the context.  If unsuccessful, also close the device. */
	g_oal_context = alcCreateContext(g_oal_device, NULL);
	if (!g_oal_context) {
		g_oal_error = Q_OAL_OPENAL_ERROR;
		qerr << "context creation failed: " << alcGetError(g_oal_device) << endl << flush;
		alcCloseDevice(g_oal_device);
		g_oal_device = NULL;
		return NULL;
	}
	qerr << "success!  (0x" << g_oal_context << ")" << endl << flush;
	++g_oal_count;
	return (void*)g_oal_context;
}

int qOalDestroyContext(void* ctxt)
{
	qLog() << endl << "DESTROYING OPENAL CONTEXT (0x" << ctxt << ")" << flush;
	unique_mutex_lock lk(g_oal_mutex);
	return qPrivateOalDestroyContext(ctxt);
}

int qPrivateOalDestroyContext(void* ctxt)
{
	ALCenum err;
	int retVal = Q_OAL_NO_ERROR;
	if (g_oal_context != ctxt) {
		qLog() << "... not destroying; we didn't create this context (ours is: 0x" 
			   << g_oal_context << ", not " << ctxt << ")" << flush;
		return g_oal_error = Q_OAL_CONTEXT_NOT_FROM_HERE;
	}
	alcMakeContextCurrent(NULL);
	alcDestroyContext(g_oal_context);
	err = alcGetError(g_oal_device);
	if (ALC_NO_ERROR != err) {
		g_oal_error = retVal = Q_OAL_OPENAL_ERROR;
		qLog() << "FAILED TO DESTROY OPENAL CONTEXT: " << err << flush;
	}
	g_oal_context = NULL;
	if (ALC_TRUE != alcCloseDevice(g_oal_device)) {
		g_oal_error = retVal = Q_OAL_OPENAL_ERROR;
		qLog() << "FAILED TO CLOSE OPENAL DEVICE" << flush;
	}
	g_oal_device = NULL;
	return retVal;
}

void qOalSetAudioDevice(char* deviceName)
{
	qLog() << "SETTING OPENAL DEVICE NAME TO: " << (deviceName ? deviceName : "(NULL)") << flush;
	
	unique_mutex_lock lk(g_oal_mutex);
	if (g_oal_context) {
		qLog() << endl << "DESTROYING OPENAL CONTEXT (0x" << g_oal_context << ")" << flush;
		qPrivateOalDestroyContext(g_oal_context);
	}
	if (g_oal_devicename) free(g_oal_devicename);
	g_oal_devicename = deviceName;
}

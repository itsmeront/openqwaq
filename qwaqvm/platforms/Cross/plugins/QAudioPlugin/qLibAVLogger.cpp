/*
 * Project OpenQwaq
 *
 * Copyright (c) 2011, 3d Immersive Collaboration Consultants, LLC, All Rights Reserved
 *
 * qLibAVLogger.cpp
 * QVideoCodecPluginFree
 *
 * Created by Josh Gargus on 6/3/11.
 */

#include "../QwaqLib/qLogger.hpp"

extern "C" {
#include "qLibAVLogger.h"

/* So that we can spew libav log messages to the same place as the rest... */
void q_av_log_callback(void* ptr, int level, const char* fmt, va_list args) {
	qerr << endl << "LIBAV[" << level << "]: ";
	qvprintf(fmt, args);
}

/* Ditto for x264 logs */
void q_x264_log_callback(void* ptr, int level, const char* fmt, va_list args) {
	qerr << endl << "X264[" << level << "]: ";
	qvprintf(fmt, args);
}

} // extern "C"




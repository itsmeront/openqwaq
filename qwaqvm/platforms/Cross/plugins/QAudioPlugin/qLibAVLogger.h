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

/* Callback passed to libav logger, so that we can spew libav log messages 
   to the same place as the rest... */
void q_av_log_callback(void* ptr, int level, const char* fmt, va_list args);

/* Ditto for x264 logs*/
void q_x264_log_callback(void* ptr, int level, const char* fmt, va_list args);
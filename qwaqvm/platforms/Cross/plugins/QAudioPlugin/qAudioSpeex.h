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
 *  qAudioSpeex.h
 *  QAudioPlugin
 *
 */

#ifndef __Q_AUDIO_SPEEX_H__
#define __Q_AUDIO_SPEEX_H__

struct QSpeexCodec;
typedef struct QSpeexCodec* QSpeexCodecPtr;

void qSpeexDestroyHandle(QSpeexCodecPtr handle);
QSpeexCodecPtr qSpeexCreateHandle(void);
int qSpeexEncode(QSpeexCodecPtr handle, void* buffer, int bufferSize);
void qSpeexEncodeRead(QSpeexCodecPtr handle, void* outputBytes, int outputSize);
int qSpeexDecode(QSpeexCodecPtr handle, void* inputBytes, int inputSize, void* outputSamples, int outputSize);
int qSpeexDecodeToTestJB(QSpeexCodecPtr handle, void* inputBytes, int inputSize, void* outputSamples, int outputSize);

#endif /* #define __Q_AUDIO_SPEEX_H__ */

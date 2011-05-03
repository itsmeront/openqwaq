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
 *  QAudioCodecPlugin.h
 *  QAudioCodecPlugin
 *
 */

#ifndef __Q_AUDIO_CODEC_PLUGIN_H__
#define __Q_AUDIO_CODEC_PLUGIN_H__

#define EXCLUDE_IAX 1

#include "qAudioSpeex.h"
#include "qAudioOpenAL.h"
#include "qLogger.h"

#include "qFeedbackChannel-interface.h"

/* gcc version 4.1.2 20080704 (Red Hat 4.1.2-46) blows up compiling ctype defs
 * if sqMemoryAccess.h is included early.
 */
#include "sqMemoryAccess.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct QJitterbufferStats {
	// version 1 fields
	long version;
	long total_gets;
	long total_extrapolations;
	long recent_gets;
	long recent_extrapolations;	
	
	// version 2 fields
	long total_drops;
	long recent_drops;
	long amount_buffered;
} QJitterbufferStats;

typedef enum {
	QPrimitiveResultUnimplemented = -1,
	QPrimitiveResultOK = 0,
	QPrimitiveResultBadArgCount,
	QPrimitiveResultBadGlue,
	QPrimitiveResultMissingObject = 10,
	QPrimitiveResultBadDynamicCast,
	QPrimitiveResultBadSamplingRate = 20
} QPrimitiveResultCode;

/* Some declarations from "iaxclient.h" (otherwise will have conflicts). */
int iaxc_call(const char * num); 
long iaxc_write_output_buffer(int index, void * data, long len);
int iaxc_select_call(int callNo);
void iaxc_reject_call_number(int callNo);
int iaxc_selected_call();
void iaxc_dump_call(void);
void iaxc_answer_call(int callNo);
int iaxc_quelch(int callNo, int MOH);
int iaxc_unquelch(int call);
void iaxc_send_busy_on_incoming_call(int callNo);

#ifdef WIN32
/* XXXXX: These will not be necessary once we have compiled our hacked iaxclient for Window */
typedef int (*iaxc_gsm_audio_callback_t)(unsigned char* data, int datalen);
void iaxc_set_gsm_audio_callback(iaxc_gsm_audio_callback_t func);
int iaxc_get_output_buffer();
#endif

int qInitModule();
int qShutdownModule();

int qIaxInit();
int qIaxShutdown();

int qPortAudioInit();
int qPortAudioShutdown();

/* Audio decoding */
unsigned qCreateAudioDecoder(unsigned codecType, unsigned char* config, unsigned configSize);
void qDestroyAudioDecoder(unsigned handle);
int qAudioDecode(unsigned handle, unsigned char* input, int inSize, unsigned short* output, int outSize, unsigned flags);

/* Audio encoding */
unsigned qCreateAudioEncoder(unsigned codecType, void* feedbackChannel, unsigned char* config, unsigned configSize);
void qDestroyAudioEncoder(unsigned handle);
int qAudioAsyncEncode(unsigned handle, short *bufferPtr, int sampleCount);


void qTickerSetVerbosity(unsigned newVerbosity);
void qTickerSetInterval(unsigned milliseconds);
void qTickerStart();
void qTickerStop();
void qTickerTickNow();

/* QAudioSink - creation and destruction */
unsigned qCreateSinkOpenAL(void);
unsigned qCreateSinkPortAudio(void);
unsigned qCreateSinkSpeex(void);
unsigned qCreateSinkPOTS(void);
unsigned qCreateSinkMixer(void);
unsigned qCreateSinkBufferedResampler(void);
unsigned qCreateSinkForDebugFeedback(void* feedbackChannel);
void qDestroySink(unsigned handle);

/* QAudioSink - generic */
void qPrintTickeeDebugInfo(unsigned handle);
QPrimitiveResultCode qSinkAddSource(unsigned sinkHandle, unsigned sourceHandle);
QPrimitiveResultCode qSinkRemoveSource(unsigned sinkHandle, unsigned sourceHandle);
int qSinkControl(unsigned handle, int ctlType, int ctlVal);
sqInt qSinkGetEventTimings(unsigned handle);

/* QAudioSinkOpenAL */
QPrimitiveResultCode qSinkSetTransformOpenAL(unsigned handle, double posX, double posY, double posZ, double dirX, double dirY, double dirZ);
QPrimitiveResultCode qSinkSetInitialPropertiesOpenAL(unsigned handle, double refDist, double maxDist, double rolloff, double innerAngle, double outerAngle, double outerGain);
QPrimitiveResultCode qSinkSetGainOpenAL(unsigned handle, double gain);
QPrimitiveResultCode qSinkPlayDebugAudioDirectlyViaOpenAL(unsigned handle, void* bytes, int byteSize);

unsigned qCreatePhoneUser(char* username, char* password, char* hostname, void* feedbackChannelHandle);
unsigned qDestroyPhoneInterfaceIAX(unsigned handle);
void qGenerateTestPhoneEvents(void* feedbackChannelHandle);

/* QAudioSinkSpeex */
void qSinkPushEncodedSpeex(unsigned handle, void* bytes, int byteSize, int timestamp);
int qSinkJitterbufferCtlSpeex(unsigned handle, int ctlType, int ctlVal);
void qSinkResetJitterbufferTimestamps(unsigned handle);
void qSinkSpeexStartDebugRecording(unsigned handle, char* filePath);
void qSinkSpeexStopDebugRecording(unsigned handle);
int qSinkSpeexIsDebugRecording(unsigned handle);
/* return 0 for success, negative int otherwise */
int qSinkSpeexGetJitterbufferStats(unsigned handle, QJitterbufferStats* stats); 

/* QAudioSinkBufferedResampler */
QPrimitiveResultCode qSinkPushRawAudio(unsigned handle, short *bufferPtr, int sampleCount);
QPrimitiveResultCode qSinkSetInputSamplingRate(unsigned handle, unsigned rate);
QPrimitiveResultCode qSinkSetOutputSamplingRate(unsigned handle, unsigned rate);
QPrimitiveResultCode qSinkSetBufferedFrameCount(unsigned handle, unsigned frameCount);

void qPortAudioPrintDeviceInfo();
void qPortAudioResetDevice();

#ifdef __cplusplus
}
#endif

#endif /* #define __Q_AUDIO_CODEC_PLUGIN_H__ */

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
 *  qAudioPluginGlue.cpp
 *  QAudioPlugin
 *
 */

#include "QAudioPlugin.h"
#include "qTicker.hpp"
#include "qLogger.hpp"
#include "qAudioSinkSpeex.hpp"
#include "qAudioSinkOpenAL.hpp"
#if __APPLE__ && __MACH__
// Support PortAudio on Mac OS X only at present.
# include "qAudioSinkPortAudio.hpp"
# include "qPortAudioInterface.hpp"
#else
// Don't support PortAudio on Windows or linux yet.
# define EXCLUDE_PORTAUDIO 1
void qPortAudioResetDevice() {}
void qPortAudioPrintDeviceInfo() {}
unsigned qCreateSinkPortAudio(void) { return 0; } // 0 means failure
#endif
#include "qAudioSinkMixer.hpp"
#include "qAudioSinkBufferedResampler.hpp"
#include "qAudioSinkForDebugFeedback.hpp"

using namespace Qwaq;
using std::ostream;

Ticker g_Ticker;
extern "C" { extern struct VirtualMachine* interpreterProxy; }

void qTickerSetVerbosity(unsigned newVerbosity)
{
	g_Ticker.setVerbosity(newVerbosity);
}


void qTickerSetInterval(unsigned milliseconds)
{
	g_Ticker.setInterval(milliseconds);
}


void qTickerStart()
{
	g_Ticker.start();
}


void qTickerStop()
{
	g_Ticker.stop();
}


void qTickerTickNow()
{
	g_Ticker.tick();
}


template<class Sink> 
unsigned qCreateSink(void)
{
	Sink *sink = new Sink();
	if (!sink) {
		qLog() << "qCreateSink(): failed to instantiate new sink" << flush;
		return 0;
	}
	g_Ticker.addTickee(sink->ptr());
	return sink->key();
}


unsigned qCreateSinkForDebugFeedback(void* feedbackChannel)
{
	QAudioSinkForDebugFeedback *sink = new QAudioSinkForDebugFeedback(*((FeedbackChannel**)feedbackChannel));
	if (!sink) return 0;
	g_Ticker.addTickee(sink->ptr());
	return sink->key();
}


unsigned qCreateSinkOpenAL(void)
{
	return qCreateSink<QAudioSinkOpenAL>();
}


#if !EXCLUDE_PORTAUDIO
unsigned qCreateSinkPortAudio(void)
{
	unsigned handle = qCreateSink<QAudioSinkPortAudio>();
	if (handle != 0) {
		// Automatically add these to the master QPortAudioInterface.
		QPortAudioInterface::addPortAudioSource(Tickee::withKey(handle));
	}
	return handle;
}
#endif

unsigned qCreateSinkMixer(void)
{
	return qCreateSink<QAudioSinkMixer>();
}


unsigned qCreateSinkBufferedResampler(void)
{
	return qCreateSink<QAudioSinkBufferedResampler>();
}


unsigned qCreateSinkSpeex(void)
{
	return qCreateSink<QAudioSinkSpeex>();
}


unsigned qCreateSinkPOTS(void) 
{ 
	return 0;
}


void qDestroySink(unsigned handle)
{
	Tickee::releaseKey(handle);
}


void qPrintTickeeDebugInfo(unsigned handle)
{
	shared_ptr<Tickee> tickee = Tickee::withKey(handle);
	if (tickee.get()) {
		tickee->printDebugInfo();
	}
	else {
		qLog() << "qPrintTickeeDebugInfo(): can't get tickee with key: " << handle << flush;
	}
}


QPrimitiveResultCode qSinkAddSource(unsigned sinkHandle, unsigned sourceHandle)
{
	shared_ptr<Tickee> sink = Tickee::withKey(sinkHandle);
	shared_ptr<Tickee> source = Tickee::withKey(sourceHandle);
	qLog(2) << "Adding source: " << sourceHandle << " to sink: " << sinkHandle;
	if (!sink.get()) {
		qLog() << "qSinkAddSource():  can't get sink with key: " << sinkHandle << flush;
		return QPrimitiveResultMissingObject;
	}
	if (!source.get()) {
		qLog() << "qSinkAddSource():  can't get source with key: " << sourceHandle << flush;
		return QPrimitiveResultMissingObject;
	}
	sink->addSource(source);
	return QPrimitiveResultOK;
}


QPrimitiveResultCode qSinkRemoveSource(unsigned sinkHandle, unsigned sourceHandle)
{
	shared_ptr<Tickee> sink = Tickee::withKey(sinkHandle);
	shared_ptr<Tickee> source = Tickee::withKey(sourceHandle);
	qLog(2) << "Removing source: " << sourceHandle << " from sink: " << sinkHandle;
	if (!sink.get()) {
		qLog() << "qSinkRemoveSource():  can't get sink with key: " << sinkHandle << flush;
		return QPrimitiveResultMissingObject;
	}
	if (!source.get()) {
		qLog() << "qSinkRemoveSource():  can't get source with key: " << sourceHandle << flush;
		return QPrimitiveResultMissingObject;
	}
	sink->removeSource(source);
	return QPrimitiveResultOK;
}


QPrimitiveResultCode qSinkSetTransformOpenAL(unsigned handle, double posX, double posY, double posZ, double dirX, double dirY, double dirZ)
{
	shared_ptr<Tickee> tickee = Tickee::withKey(handle);
	if (!tickee.get()) {
		qLog() << "qSinkSetTransformOpenAL():  can't get sink with key: " << handle << flush;
		return QPrimitiveResultMissingObject;
	}
	shared_ptr<QAudioSinkOpenAL> sink = boost::dynamic_pointer_cast<QAudioSinkOpenAL>(tickee);
	if (!sink.get()) {
		qLog() << "qSinkSetTransformOpenAL():  failed dynamic cast" << flush;
		return QPrimitiveResultBadDynamicCast;
	}
	sink->setTransform(posX, posY, posZ, dirX, dirY, dirZ);
	return QPrimitiveResultOK;
}


QPrimitiveResultCode qSinkSetGainOpenAL(unsigned handle, double gain)
{
	shared_ptr<Tickee> tickee = Tickee::withKey(handle);

	if (!tickee.get()) {
		qLog() << "qSinkSetGainOpenAL():  can't get sink with key: " << handle << flush;
		return QPrimitiveResultMissingObject;
	}
	shared_ptr<QAudioSinkOpenAL> sink = boost::dynamic_pointer_cast<QAudioSinkOpenAL>(tickee);
	if (!sink.get()) {
		qLog() << "qSinkSetGainOpenAL():  failed dynamic cast: " << handle << flush;
		return QPrimitiveResultBadDynamicCast;
	}
	sink->setGain(gain);
	return QPrimitiveResultOK;
}


QPrimitiveResultCode qSinkSetInitialPropertiesOpenAL(unsigned handle, double refDist, double maxDist, double rolloff, double innerAngle, double outerAngle, double outerGain)
{
	shared_ptr<Tickee> tickee = Tickee::withKey(handle);
	if (!tickee.get()) {
		qLog() << "qSinkSetInitialPropertiesOpenAL():  can't get sink with key: " << handle << flush;
		return QPrimitiveResultMissingObject;
	}
	shared_ptr<QAudioSinkOpenAL> sink = boost::dynamic_pointer_cast<QAudioSinkOpenAL>(tickee);
	if (!sink.get()) {
		qLog() << "qSinkSetInitialPropertiesOpenAL():  failed dynamic cast: " << handle << flush;
		return QPrimitiveResultBadDynamicCast;
	}
	sink->setInitialPropertiesOpenAL(refDist, maxDist, rolloff, innerAngle, outerAngle, outerGain);
	return QPrimitiveResultOK;
}


QPrimitiveResultCode qSinkPlayDebugAudioDirectlyViaOpenAL(unsigned handle, void* bytes, int byteSize)
{
	shared_ptr<Tickee> tickee = Tickee::withKey(handle);
	if (!tickee.get()) {
		qLog() << "qSinkPlayDebugAudioDirectlyViaOpenAL():  can't get sink with key: " << handle << flush;
		return QPrimitiveResultMissingObject;
	}
	shared_ptr<QAudioSinkOpenAL> sink = boost::dynamic_pointer_cast<QAudioSinkOpenAL>(tickee);
	if (!sink.get()) {
		qLog() << "qSinkPlayDebugAudioDirectlyViaOpenAL():  failed dynamic cast: " << handle << flush;
		return QPrimitiveResultBadDynamicCast;
	}
	sink->playAudioDirectly((ALvoid*)bytes, (ALsizei)byteSize);
	return QPrimitiveResultOK;
}


// For Speex sinks.
void qSinkPushEncodedSpeex(unsigned handle, void* bytes, int byteSize, int timestamp)
{
	shared_ptr<QAudioSinkSpeex> sink = boost::dynamic_pointer_cast<QAudioSinkSpeex>(Tickee::withKey(handle));
	if (!sink.get()) {
		qLog() << "qSinkPushEncodedSpeex():  can't get sink with key: " << handle << flush;
		return;
	}
	sink->pushEncodedSpeex(bytes, byteSize, timestamp);
}


// For Speex sinks.
int qSinkJitterbufferCtlSpeex(unsigned handle, int ctlType, int ctlVal)
{
	shared_ptr<QAudioSinkSpeex> sink = boost::dynamic_pointer_cast<QAudioSinkSpeex>(Tickee::withKey(handle));
	if (!sink.get()) {
		qLog() << "qSinkJitterbufferCtlSpeex():  can't get sink with key: " << handle << flush;
		return -1;
	}
	return sink->jitterbufferCtl(ctlType, ctlVal);
}


// We've had this a while for Speex sinks, now we do it for everyone.
int qSinkControl(unsigned handle, int ctlType, int ctlVal)
{
	shared_ptr<Tickee> sink = boost::dynamic_pointer_cast<Tickee>(Tickee::withKey(handle));
	if (!sink.get()) {
		qLog() << "qSinkControl():  can't get sink with key: " << handle << flush;
		return -1;
	}
	return sink->genericControl(ctlType, ctlVal);
}


sqInt qSinkGetEventTimings(unsigned handle)
{
	shared_ptr<Tickee> sink = boost::dynamic_pointer_cast<Tickee>(Tickee::withKey(handle));
	if (!sink.get()) {
		qLog() << "qSinkControl():  can't get sink with key: " << handle << flush;
		interpreterProxy->primitiveFailFor(PrimErrBadArgument);
		return 0;
	}
	return sink->getEventTimings();
}

// For Speex sinks.
void qSinkResetJitterbufferTimestamps(unsigned handle)
{
	shared_ptr<QAudioSinkSpeex> sink = boost::dynamic_pointer_cast<QAudioSinkSpeex>(Tickee::withKey(handle));
	if (!sink.get()) {
		qLog() << "qSinkPushEncodedSpeex():  can't get sink with key: " << handle << flush;
		return;
	}
	sink->resetTimestamps();
}


void qSinkSpeexStartDebugRecording(unsigned handle, char* filePath)
{
	shared_ptr<QAudioSinkSpeex> sink = boost::dynamic_pointer_cast<QAudioSinkSpeex>(Tickee::withKey(handle));
	if (!sink.get()) {
		qLog() << "qSinkSpeexStartDebugRecording():  can't get sink with key: " << handle << flush;
		return;
	}
	sink->startDebugRecording(filePath);
}


void qSinkSpeexStopDebugRecording(unsigned handle)
{
	shared_ptr<QAudioSinkSpeex> sink = boost::dynamic_pointer_cast<QAudioSinkSpeex>(Tickee::withKey(handle));
	if (!sink.get()) {
		qLog() << "qSinkSpeexStopDebugRecording():  can't get sink with key: " << handle << flush;
		return;
	}
	sink->stopDebugRecording();
}


int qSinkSpeexIsDebugRecording(unsigned handle)
{
	shared_ptr<QAudioSinkSpeex> sink = boost::dynamic_pointer_cast<QAudioSinkSpeex>(Tickee::withKey(handle));
	if (!sink.get()) {
		qLog() << "qSinkSpeexIsDebugRecording():  can't get sink with key: " << handle << flush;
		return 0;
	}
	return sink->isDebugRecording();
}


int qSinkSpeexGetJitterbufferStats(unsigned handle, QJitterbufferStats* stats)
{
	shared_ptr<QAudioSinkSpeex> sink = boost::dynamic_pointer_cast<QAudioSinkSpeex>(Tickee::withKey(handle));
	if (!sink.get()) {
		qLog() << "qSinkSpeexGetJitterbufferStats():  can't get sink with key: " << handle << flush;
		return -1;
	}
	return sink->getJitterbufferStats(stats);
}


QPrimitiveResultCode qSinkPushRawAudio(unsigned handle, short *bufferPtr, int sampleCount)
{
	shared_ptr<Tickee> tickee = Tickee::withKey(handle);
	if (!tickee.get()) {
		qLog() << "qSinkPushRawAudio():  can't get sink with key: " << handle << flush;
		return QPrimitiveResultMissingObject;
	}
	shared_ptr<QAudioSinkBufferedResampler> sink = boost::dynamic_pointer_cast<QAudioSinkBufferedResampler>(tickee);
	if (!sink.get()) {
		qLog() << "qSinkPushRawAudio():  failed dynamic cast: " << handle << flush;
		return QPrimitiveResultBadDynamicCast;
	}
	sink->pushRawAudio(bufferPtr, sampleCount);  // XXXXX: should return error code
	return QPrimitiveResultOK;	
}


QPrimitiveResultCode qSinkSetInputSamplingRate(unsigned handle, unsigned rate)
{
	shared_ptr<Tickee> tickee = Tickee::withKey(handle);
	if (!tickee.get()) {
		qLog() << "qSinkSetInputSamplingRate():  can't get sink with key: " << handle << flush;
		return QPrimitiveResultMissingObject;
	}
	shared_ptr<QAudioSinkBufferedResampler> sink = boost::dynamic_pointer_cast<QAudioSinkBufferedResampler>(tickee);
	if (!sink.get()) {
		qLog() << "qSinkSetInputSamplingRate():  failed dynamic cast: " << handle << flush;
		return QPrimitiveResultBadDynamicCast;
	}
	sink->setInputSamplingRate(rate);  // XXXXX: should return error code
	return QPrimitiveResultOK;	
}


QPrimitiveResultCode qSinkSetOutputSamplingRate(unsigned handle, unsigned rate)
{
	shared_ptr<Tickee> tickee = Tickee::withKey(handle);
	if (!tickee.get()) {
		qLog() << "qSinkSetOuputSamplingRate():  can't get sink with key: " << handle << flush;
		return QPrimitiveResultMissingObject;
	}
	shared_ptr<QAudioSinkBufferedResampler> sink = boost::dynamic_pointer_cast<QAudioSinkBufferedResampler>(tickee);
	if (!sink.get()) {
		qLog() << "qSinkSetOuputSamplingRate():  failed dynamic cast: " << handle << flush;
		return QPrimitiveResultBadDynamicCast;
	}
	sink->setOutputSamplingRate(rate);  // XXXXX: should return error code
	return QPrimitiveResultOK;	
}


QPrimitiveResultCode qSinkSetBufferedFrameCount(unsigned handle, unsigned frameCount)
{
	shared_ptr<Tickee> tickee = Tickee::withKey(handle);
	if (!tickee.get()) {
		qLog() << "qSinkSetBufferedFrameCount():  can't get sink with key: " << handle << flush;
		return QPrimitiveResultMissingObject;
	}
	shared_ptr<QAudioSinkBufferedResampler> sink = boost::dynamic_pointer_cast<QAudioSinkBufferedResampler>(tickee);
	if (!sink.get()) {
		qLog() << "qSinkSetBufferedFrameCount():  failed dynamic cast: " << handle << flush;
		return QPrimitiveResultBadDynamicCast;
	}
	sink->setBufferedFrameCount(frameCount);  // XXXXX: should return error code
	return QPrimitiveResultOK;	
}



int qPortAudioInit()
{
#if !EXCLUDE_PORTAUDIO
	QPortAudioInterface::init();
#endif
	return 0;
}


int qPortAudioShutdown()
{
#if !EXCLUDE_PORTAUDIO
	QPortAudioInterface::shutdown();
#endif
	return 0;
}




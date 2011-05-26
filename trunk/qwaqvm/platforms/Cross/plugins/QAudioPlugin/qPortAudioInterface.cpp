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
 *  qPortAudioInterface.cpp
 *  QAudioPlugin
 *
 */

#if !EXCLUDE_PORTAUDIO
#include "qPortAudioInterface.hpp"
#include "qLogger.hpp"
using namespace Qwaq;

#include "qTicker.hpp"
extern Ticker g_Ticker;

shared_ptr<QPortAudioInterface> QPortAudioInterface::g_interface;

// Declare/define the callback from PortAudio.
extern "C" {
int c_callback(const void *input, void *output, unsigned long frameCount, 
			const PaStreamCallbackTimeInfo *timeInfo,
			PaStreamCallbackFlags statusFlags,
			void *userData);
void c_callback_StreamFinished(void* userData);
}
int c_callback(const void *input, void *output, unsigned long frameCount, 
			const PaStreamCallbackTimeInfo *timeInfo,
			PaStreamCallbackFlags statusFlags,
			void *userData)
{
	QPortAudioInterface *interface = (QPortAudioInterface*)userData;
	return interface->callback(input, output, frameCount, timeInfo, statusFlags);
}

void c_callback_StreamFinished(void* userData)
{
	QPortAudioInterface *interface = (QPortAudioInterface*)userData;
	interface->callbackStreamFinished();
}

QPortAudioInterface::QPortAudioInterface() : isRunning(false), ring(FRAME_SIZE*sizeof(short)*3)
{
	className = "QPortAudioInterface";  // for logging
	
	outputEvenIfNoInput = false;

	log() << "<constructor>" << flush;
	PaError err = Pa_OpenDefaultStream(&stream, 0, 1, paInt16, 16000.0, 320, c_callback, this);
	if (err != paNoError) {
		log() << "constructor failed: " << Pa_GetErrorText(err) << flush;
		stream = NULL; // XXXXX: does this leak memory?  Error behavior not well-defined.
	}
	else {
		err = Pa_SetStreamFinishedCallback(stream, c_callback_StreamFinished);
		if (err != paNoError) {
			log() << "failed to set 'stream-finished' callback: " << Pa_GetErrorText(err) << flush;
		}
	}
}

QPortAudioInterface::~QPortAudioInterface()
{
	log() << "<destructor>" << flush;
	
	if (stream) {
		PaError err = Pa_CloseStream(stream);
		if (err != paNoError) {
			log() << "destructor failed: " << Pa_GetErrorText(err) << flush;
		}
	}
	log() << "<destructor> finished" << flush;
}

void QPortAudioInterface::tick()
{
	QAudioSinkMixer::tick();
	if (hasBuffer) {
		ring.put(buffer, FRAME_SIZE*sizeof(short));
		if (!isRunning && (ring.dataSize() >= FRAME_SIZE*sizeof(short))) {
			PaError err = Pa_StartStream(stream);
			if (err != paNoError) { 
				log() << " cannot start stream: " << Pa_GetErrorText(err) << flush;
			}
			else {
				log() << " successfully started stream" << flush;
				isRunning = true;
			}
		}
	}
}

// XXXXX: there should be a lock here, but it should be OK
// because it is only called once, when the module is initialized.
void QPortAudioInterface::init()
{
	PaError err = Pa_Initialize();
	if (err != paNoError) {
		qLog() << "QPortAudioInterface::init() failed: " << Pa_GetErrorText(err) << flush;
		return;
	}
	QPortAudioInterface* interface = new QPortAudioInterface();
	g_interface = boost::shared_dynamic_cast<QPortAudioInterface, Tickee>(interface->ptr());
	g_Ticker.addTickee(g_interface);
	qLog() << "INITIALIZED QPortAudioInterface" << flush;
}

// XXXXX: there should be a lock here, but it should be OK
// because it is only called once, when the module is shutdown.
void QPortAudioInterface::shutdown()
{
	if (g_interface.get()) {
		Tickee::releaseKey(g_interface->key());
		g_interface.reset();
	}
	PaError err = Pa_Terminate();
	if (err != paNoError) {
		qLog() << "QPortAudioInterface::shutdown() failed: " << Pa_GetErrorText(err) << flush;
		return;
	}
	qLog() << "SHUTDOWN QPortAudioInterface" << flush;
}		
		
void QPortAudioInterface::addPortAudioSource(shared_tickee source)
{
	g_interface->addSource(source);	
}
		
		
int QPortAudioInterface::callback(const void *input, void *output, unsigned long frameCount, 
								const PaStreamCallbackTimeInfo *timeInfo,
								PaStreamCallbackFlags statusFlags)
{
	if (ring.hasData()) {
		// There is data in the ring-buffer, so copy it to the output, and keep going.
		ring.get(output, frameCount*sizeof(short));
		return paContinue;
	}
	else {
		// There is no data in the ring-buffer, so zero the output, and stop playing.
		memset(output, 0, frameCount*sizeof(short));
		log() << " no more data in the ring-buffer" << flush;
		return paComplete;
	}
}

void QPortAudioInterface::callbackStreamFinished()
{
	PaError err = Pa_StopStream(stream);
	if (err != paNoError) {
		log() << " callbackStreamFunction(): error while stopping stream: " << Pa_GetErrorText(err) << flush;
	}
	log() << " received stream-finshed callback" << flush;
	isRunning = false;
}


void QPortAudioInterface::printDebugInfo()
{
	log() 
		<< "printDebugInfo(): " << "\n\t"
		<< "16kHz mono" << "\n\t"
		<< sources.size() << " inputs" << flush;
}


void qPortAudioPrintDeviceInfoHelper(std::ostream& oStr, PaDeviceIndex ind)
{
	const PaDeviceInfo *info = Pa_GetDeviceInfo(ind);
	oStr << "\n\t\tDevice #" << ind
		<< "\n\t\t\tHost API index: " << info->hostApi
		<< "\n\t\t\tName: " << info->name
		<< "\n\t\t\tMax Input Channels: " << info->maxInputChannels
		<< "\n\t\t\tMax Output Channels: " << info->maxOutputChannels
		<< "\n\t\t\tLow Input Latency: " << info->defaultLowInputLatency
		<< "\n\t\t\tLow Output Latency: " << info->defaultLowOutputLatency
		<< "\n\t\t\tHigh Input Latency: " << info->defaultHighInputLatency
		<< "\n\t\t\tHigh Output Latency: " << info->defaultHighOutputLatency
		<< "\n\t\t\tDefault Sample Rate: " << info->defaultSampleRate
		<< "\n";
}

void qPortAudioPrintDeviceInfo()
{
	PaDeviceIndex ind = Pa_GetDefaultOutputDevice();
	qLog() << "Printing PortAudio device info..." << "\n\tDEFAULT OUTPUT:";
	qPortAudioPrintDeviceInfoHelper(qerr, ind);

	ind = Pa_GetDefaultInputDevice();
	qerr << "\n\tDEFAULT INPUT:";	
	qPortAudioPrintDeviceInfoHelper(qerr, ind);
	
	PaDeviceIndex count = Pa_GetDeviceCount();
	qerr << "\n\tALL DEVICES:";	
	for (PaDeviceIndex i=0; i<count; i++) {
		qPortAudioPrintDeviceInfoHelper(qerr, i);
	}
}

void qPortAudioResetDevice()
{
	QPortAudioInterface::shutdown();
	QPortAudioInterface::init();
}
#endif /* !EXCLUDE_PORTAUDIO */

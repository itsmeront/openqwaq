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
 *  qPortAudioInterface.hpp
 *  QAudioPlugin
 *
 */

#ifndef __Q_PORT_AUDIO_INTERFACE_HPP__
#define __Q_PORT_AUDIO_INTERFACE_HPP__

#include "qAudioSinkPortAudio.hpp"
#include "qAudioSinkMixer.hpp"
#include "qTickee.hpp"
#include "qRingBuffer.hpp"

#include "portaudio.h"

namespace Qwaq {

class QPortAudioInterface;

class QPortAudioInterface : public QAudioSinkMixer
{
	public:
		QPortAudioInterface();
		~QPortAudioInterface();
		
		virtual void tick();
		virtual unsigned tickPriority() { return 1; } // always the end of the line
		virtual void printDebugInfo();
		
		static void init();
		static void shutdown();
		static void addPortAudioSource(shared_tickee source);
		
		// PortAudio callback.  Called when input is available and/or output is required.
		int callback(const void *input, void *output, unsigned long frameCount, 
					const PaStreamCallbackTimeInfo *timeInfo,
					PaStreamCallbackFlags statusFlags);
		// PortAudio callback.  Called when stream has finished playing.					
		void callbackStreamFinished();
		
	protected:
		PaStream* stream;
		bool isRunning;
		QRingBuffer ring;
		
		static shared_ptr<QPortAudioInterface> g_interface;
//		static QPortAudioInterface *g_interface;
};


}; // namespace Qwaq

#endif // #ifndef __Q_PORT_AUDIO_INTERFACE_HPP__

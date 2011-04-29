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
 *  qAudioSinkBufferedResampler.hpp
 *  QAudioPlugin
 *
 */

#ifndef __Q_AUDIO_SINK_BUFFERED_RESAMPLER_HPP__
#define __Q_AUDIO_SINK_BUFFERED_RESAMPLER_HPP__

#include "qTickee.hpp"
#include "qRingBuffer.hpp"
#include <speex/speex_resampler.h>

namespace Qwaq {

class QAudioSinkBufferedResampler : public Tickee
{
	public:
		QAudioSinkBufferedResampler();
		~QAudioSinkBufferedResampler();
		
		virtual void tick();
		virtual unsigned tickPriority() { return 10; }  // top priority
		virtual void addSource(shared_tickee src) {} // meaningless for buffered-resampler
		virtual void removeSource(shared_tickee src) {} // meaningless for buffered-resampler
		virtual void printDebugInfo();
		void pushRawAudio(short* bufferPtr, int sampleCount);
		void setInputSamplingRate(unsigned rate);
		void setOutputSamplingRate(unsigned rate);
		void setBufferedFrameCount(unsigned frameCount);
		
		
	protected:
		SpeexResamplerState *resampler;
		QRingBuffer ring;
		unsigned inputRate;
		unsigned outputRate;
		
		unsigned bufferedFrameCount;
		bool preBuffered;
		bool pushFailed;
};

}; // namespace Qwaq

#endif // #ifndef __Q_AUDIO_SINK_BUFFERED_RESAMPLER_HPP__

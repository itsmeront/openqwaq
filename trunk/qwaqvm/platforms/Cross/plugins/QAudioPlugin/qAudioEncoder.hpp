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
 *  qAudioEncoder.hpp
 *  QAudioPlugin
 *
 */
 
#ifndef __Q_AUDIO_ENCODER_HPP__
#define __Q_AUDIO_ENCODER_HPP__

#include "qAudioCodec.hpp"
#include "qFeedbackChannel.h"

#include <boost/shared_ptr.hpp>
#include <boost/thread/locks.hpp>
using boost::shared_ptr;
typedef boost::mutex::scoped_lock scoped_lock;

namespace Qwaq
{

const int MAX_ENCODED_SEGMENT_SIZE = 2000; // plenty

class AudioEncoder
{
	public:
		AudioEncoder(FeedbackChannel* feedbackChannel);
		virtual ~AudioEncoder() { }
	
		// Encode the audio, and return the result later via a FeedbackChannel.
		virtual int asyncEncode(short *bufferPtr, int sampleCount) = 0;
		
		// Used by async-encoding to push audio back to Squeak.
		void pushFeedbackData(void* data, unsigned dataSize);
		
	protected:
		// MAGIC HERE!!!
		typedef boost::shared_ptr<AudioEncoder> ptr_type;
		#include <qMappedResourceBoilerplate.hpp>
		
	protected:
		FeedbackChannel *feedback;		
}; // class AudioEncoder

}; // namespace Qwaq

#endif // #ifndef __Q_AUDIO_ENCODER_HPP__

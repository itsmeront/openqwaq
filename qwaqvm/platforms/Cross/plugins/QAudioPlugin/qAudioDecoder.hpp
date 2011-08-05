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
 *  qAudioDecoder.hpp
 *  QAudioPlugin
 *
 */

#ifndef __Q_AUDIO_DECODER_HPP__
#define __Q_AUDIO_DECODER_HPP__

#include "qAudioCodec.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
using boost::shared_ptr;
typedef boost::mutex::scoped_lock scoped_lock;

namespace Qwaq
{

class AudioDecoder
{
	public:
		AudioDecoder();
		virtual ~AudioDecoder();
		
		virtual bool isValid() = 0;
		virtual int decode(unsigned char* input, int inputSize, short* output, int outputSize, unsigned flags) = 0;
		
	protected:	
		// MAGIC HERE!!!
		typedef boost::shared_ptr<AudioDecoder> ptr_type;
		#include <qMappedResourceBoilerplate.hpp>
		
	protected:


}; // class AudioDecoder

}; // namespace Qwaq

#endif // #ifndef __Q_AUDIO_DECODER_HPP__

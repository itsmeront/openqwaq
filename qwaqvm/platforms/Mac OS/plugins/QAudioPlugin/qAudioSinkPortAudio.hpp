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
 *  qAudioSinkPortAudio.hpp
 *  QAudioPlugin
 *
 */

#ifndef __Q_AUDIO_SINK_PORT_AUDIO_HPP__
#define __Q_AUDIO_SINK_PORT_AUDIO_HPP__

#include "qTickee.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

//#include "portaudio.h"

namespace Qwaq {

class QAudioSinkPortAudio : public Tickee
{
	public:
		QAudioSinkPortAudio();
		virtual ~QAudioSinkPortAudio();
		
		virtual void tick();
		virtual unsigned tickPriority() { return 7; }
		virtual void addSource(shared_ptr<Tickee> src) { source = src; }
		virtual void removeSource(shared_ptr<Tickee> src) { source.reset(); }
		virtual void printDebugInfo() { }
		
	protected:
		boost::weak_ptr<Tickee> source;
};

}; // namespace Qwaq

#endif // #ifndef __Q_AUDIO_SINK_PORT_AUDIO_HPP__

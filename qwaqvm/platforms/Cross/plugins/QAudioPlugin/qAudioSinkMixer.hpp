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
 *  qAudioSinkMixer.h
 *  QAudioPlugin
 *
 */

#ifndef __Q_AUDIO_SINK_MIXER_HPP__
#define __Q_AUDIO_SINK_MIXER_HPP__

#include "QAudioPlugin.h"
#include "qTickee.hpp"

#include <vector>
using std::vector;

namespace Qwaq {

class QAudioSinkMixer : public Tickee
{
	public:
		QAudioSinkMixer();
		virtual ~QAudioSinkMixer();
		
		virtual void tick();
		virtual unsigned tickPriority() { return 4; }
		virtual void addSource(shared_tickee src);
		virtual void removeSource(shared_tickee src);
		virtual void printDebugInfo();
		
	protected:		
		vector<weak_tickee> sources;
		boost::mutex mutex;
		
		bool outputEvenIfNoInput;
};

}; // namespace Qwaq

#endif // #ifnder __Q_AUDIO_SINK_MIXER_HPP__

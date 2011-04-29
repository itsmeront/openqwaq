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
 *  qAudioSinkForDebugFeedback.hpp
 *  QAudioPlugin
 *
 */

#ifndef __Q_AUDIO_SINK_FOR_DEBUG_HPP__
#define __Q_AUDIO_SINK_FOR_DEBUG_HPP__

#include "qTickee.hpp"
#include "qFeedbackChannel.h"

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace Qwaq {

class QAudioSinkForDebugFeedback : public Tickee
{
	public:
		QAudioSinkForDebugFeedback(FeedbackChannel *feedbackChannel);
		virtual ~QAudioSinkForDebugFeedback();
		
		virtual void tick();
		virtual unsigned tickPriority() { return 1; } // always the end of the line
		virtual void addSource(shared_ptr<Tickee> src) { source = src; }
		virtual void removeSource(shared_ptr<Tickee> src) { source.reset(); }
		
		virtual void printDebugInfo() { }; // nothing for now
		
	protected:
		boost::weak_ptr<Tickee> source;
		FeedbackChannel* feedback;
};


}; // namespace Qwaq


#endif //#ifndef __Q_AUDIO_SINK_FOR_DEBUG_HPP__

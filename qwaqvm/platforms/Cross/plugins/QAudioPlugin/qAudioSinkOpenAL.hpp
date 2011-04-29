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
 *  qAudioSinkOpenAL.hpp
 *  QAudioPlugin
 *
 */

#ifndef __Q_AUDIO_SINK_OPENAL_HPP__
#define __Q_AUDIO_SINK_OPENAL_HPP__

#include "qTickee.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <map>
#include <vector>
#include <queue>

#if __linux__
#  include <AL/al.h>
#else
# include <OpenAL/al.h>
#endif

namespace Qwaq {

class QAudioSinkOpenAL : public Tickee
{
	public:
		QAudioSinkOpenAL();
		virtual ~QAudioSinkOpenAL();
		
		virtual void tick();
		virtual unsigned tickPriority() { return 7; }
		virtual void addSource(shared_ptr<Tickee> src) { source = src; }
		virtual void removeSource(shared_ptr<Tickee> src) { source.reset(); }
		void setTransform(double posX, double posY, double posZ, double dirX, double dirY, double dirZ);
		void setInitialPropertiesOpenAL(double rDist, double mDist, double roll, double inAngle, double outAngle, double outGain);
		void setGain(float gain);
		
		void playAudioDirectly(ALvoid* data, ALsizei dataSize);
		virtual short* getBuffer();
		
		virtual void printDebugInfo();
		
	protected:
		const static int NUM_BUFFERS = 4;  // Number of OpenAL audio buffers
		const static int SAMPLING_RATE = 16000; // It's what we use.  Love it.
	
		boost::weak_ptr<Tickee> source;
		ALuint oalSource;
		unsigned oalCount;	// Used to determine whether the OpenAL context has changed
		double transform[6]; // position X,Y,Z, and direction X,Y,Z
		float gain;
		
		// properties that are typically only set at initialization
		double refDist; 
		double maxDist; 
		double rolloff; 
		double innerAngle; 
		double outerAngle;
		double outerGain;

		ALuint oalBuffers[NUM_BUFFERS];
		std::vector<ALuint> freeBuffers;
		std::vector<ALuint> queuedBuffers;
		bool isStarted;
		bool isGainBakedIn;
		
		unsigned silentStreak;
		
		bool ensureValidSource();  // Check if there is a valid source; if not, try to create one.
		void setInitialSourceProperties();  // Set the initial properties for the source.
		void deleteOpenALResources(int verbosity);  // Check if we are using any sources/buffers, and if so, release them
		bool returnOneBuffer();
	
		boost::shared_mutex mutex;
};


}; // namespace Qwaq


#endif // #ifndef __Q_AUDIO_SINK_OPENAL_HPP__

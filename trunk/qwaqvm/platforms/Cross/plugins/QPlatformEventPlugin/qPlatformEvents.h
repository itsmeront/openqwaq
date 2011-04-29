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

/******************************************************************************
 *
 * qPlatformEvents.h
 * QPlatformEventPlugin (cross-platform)
 *
 * Defines specific events to be passed back into Squeak from the 
 * QPlatformEventPlugin (via a FeedbackChannel object).
 *
 ******************************************************************************/

#ifndef __Q_PLATFORM_EVENTS_H__
#define __Q_PLATFORM_EVENTS_H__

#include "qBuffer.h"
#include "qFeedbackChannel.h"

namespace Qwaq
{
	// Each type of event has its own unique type flag.
	enum PlatformEventTypes { EVENT_FILEHELPER = 1 };


	// Event originating from a file-helper application.	
	class PlatformFileHelperEvent : public FeedbackEvent
	{

#pragma pack(push,1)
		// Same memory layout as Squeak class QFileHelperEvent.
		// We use '#pragma pack' so that they have the exact same
		// memory layout as the Squeak FFI classes used to access
		// them... we don't want to have to insert padding in the
		// field definitions in Squeak.
		typedef struct SqueakEvent 
		{
			int type;
			void* stringptr;
		};
#pragma pack(pop)

	public:
		PlatformFileHelperEvent(string fileNameString) : fileName(fileNameString) {}

		virtual int getSqueakEventSize() { return sizeof(SqueakEvent); }
		virtual void fillInSqueakEvent(char* ptr) {
			SqueakEvent *eventptr = (SqueakEvent*)ptr;
			eventptr->type = EVENT_FILEHELPER;
			eventptr->stringptr = &fileName;
		}

		string fileName;
	};
} //namespace Qwaq

#endif //#ifndef __Q_PLATFORM_EVENTS_H__
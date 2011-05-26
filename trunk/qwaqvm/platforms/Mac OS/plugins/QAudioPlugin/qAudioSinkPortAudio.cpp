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
 *  qAudioSinkPortAudio.cpp
 *  QAudioPlugin
 *
 */

#include "qAudioSinkPortAudio.hpp"
#include "qPortAudioInterface.hpp"

using namespace Qwaq;


QAudioSinkPortAudio::QAudioSinkPortAudio()
{
	
}

QAudioSinkPortAudio::~QAudioSinkPortAudio()
{

}
		
void QAudioSinkPortAudio::tick()
{
	shared_tickee strong = source.lock();
	if (!strong || !(strong->hasValidBuffer())) {
		hasBuffer = false;
		return;
	}
	// Would be nice to not copy the data (just pass the pointer through).
	// OTOH, we will probably want to adjust the gain anyway.
	memcpy(buffer, strong->getBuffer(), FRAME_SIZE*sizeof(short));
	hasBuffer = true;
}
		

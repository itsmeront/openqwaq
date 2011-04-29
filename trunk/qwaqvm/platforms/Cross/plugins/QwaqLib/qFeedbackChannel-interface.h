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
 * qFeedbackChannel-interface.h
 * QwaqLib (cross-platform)
 *
 * Declares the functions used by the generated Slang plugin code to
 * instantiate and interact with instances of the class FeedBackChannel.
 *
 ******************************************************************************/

#ifndef __Q_FEEDBACK_CHANNEL_INTERFACE_H__
#define __Q_FEEDBACK_CHANNEL_INTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif

//
// Instantiate a Qwaq::FeedbackChannel, and store its address in the
// ByteArray 'address'.  Return 0 for success, and a negative integer
// for failure.
// XXXXX: figure out proper error codes to use.
//	
int
qCreateFeedbackChannel(char* address, int semaphoreIndex);


//
// Destroy the Qwaq::FeedbackChannel identified by 'address' (if it exists).
//
void
qDestroyFeedbackChannel(char* address);


//
// Get the next event from the specified channel.  Answer an oop that is 
// either a newly-instantiated ByteArray, or (if there is no next event)
// nil.
int
qFeedbackChannelReadEvent(char* address);


//
// Pop the next event (if any) off the front of the specified channel.
void
qFeedbackChannelPopEvent(char* address);


//
// Turn logging on or off for the specified channel. 'trueOrFalse' is 1
// for true and 0 for false.
//
void
qFeedbackChannelSetLogging(char* address, int trueOrFalse);



//XXXXX
//ensure that all FeedbackChannel instances are cleaned up when the module is unloaded



#ifdef __cplusplus
} //extern "C"
#endif




#endif //#ifndef __Q_FEEDBACK_CHANNEL_INTERFACE_H__

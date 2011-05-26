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

/******************************************************************************
 *
 * qWebcam.h
 * QWebcamPlugin (cross-platform)
 *
 * Base class provides the framework for setting up a camera, controlling it,
 * and reading data back from it.  Subclasses interface to the appropriate 
 * multimedia API (eg: QuickTime or DirectShow).
 *
 * IMPORTANT!!
 * In addition to implementing the abstract virtual functions, subclasses are
 * responsible for calling grabbedFrame() (with an appropriate argument) each
 * time they grab a frame from the camera.
 *
 ******************************************************************************/

#ifndef __Q_WEBCAM_H__
#define __Q_WEBCAM_H__

#include "qBuffer.h"
#include "qFeedbackChannel.h"

namespace Qwaq
{
	///////////////////////////////////////////////////
	// Parameters used during the creation of a webcam.
	// Use '#pragma pack' so that the corresponding
	// Squeak FFI classes needn't add padding-fields.
	///////////////////////////////////////////////////
#pragma pack(push,1)
	struct WebcamParams
	{
		static int currentVersion() { return 1; }

		// version 1 fields
		int version;
		int width;
		int height;
		double fps;
		// version 2 fields
		//    (nothing here yet)
	};
#pragma pack(pop)


	//////////////////////////////////////////////////////////
	// Event structures that are passed back to Squeak via a
	// FeedbackChannel.  We use '#pragma pack' so that they
	// have the exact same memory layout as the Squeak FFI
	// classes used to access them... we don't want to have to
	// insert padding in the field definitions in Squeak.
	//////////////////////////////////////////////////////////
#pragma pack(push,1)

	enum VideoEventTypes { EVENT_VIDEOFRAME = 1 };

	// Same memory layout as Squeak class QVideoFrameEvent
	struct SqueakVideoFrameEvent
	{
		int type;
		double sampleTime;
		unsigned int bufferSize;
		unsigned char* buffer;

		SqueakVideoFrameEvent() : type(EVENT_VIDEOFRAME) { }
	};
#pragma pack(pop)


	//
	// A VideoFrameEvent is created each time we receive a new
	// frame from the filter-graph.
	//
	class VideoFrameEvent : public FeedbackEvent
	{
	public:
		VideoFrameEvent(BufferPool2::ptr_type buf);

		virtual int getSqueakEventSize() { return sizeof(SqueakVideoFrameEvent); }
		virtual void fillInSqueakEvent(char* ptr) { *((SqueakVideoFrameEvent*)ptr) = squeakEvent; }

		SqueakVideoFrameEvent squeakEvent;
		BufferPool2::ptr_type buffer;
	};


	class Webcam
	{
	public:
		Webcam(WebcamParams* params, BufferPool2& bufferPool, FeedbackChannel* ch);
		virtual ~Webcam();

		virtual void run() = 0;
		virtual void pause() = 0;
		virtual void stop() = 0;
		virtual void adjustParameters(WebcamParams& params) = 0;

	protected:
		// subclasses MUST call this each time they grab a frame
		// (or else the grabbed frame will never make it to Squeak).
		// The video frame must be allocated on the heap (responsibility
		// for managing its life-cycle will belong to 'feedbackChannel').
		void grabbedFrame(VideoFrameEvent* f);

		WebcamParams parameters;
		BufferPool2& pool;
		FeedbackChannel* feedbackChannel;
	};



} //namespace Qwaq

#endif //#ifndef __Q_WEBCAM_H__

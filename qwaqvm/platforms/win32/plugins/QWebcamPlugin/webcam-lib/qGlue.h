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
 * qGlue.h
 * QWebcamPlugin (win32)
 *
 * Implements the same functions as used by DShowVideoDecoderPlugin...
 * thus we maintain compatability (although we are paving the way for
 * a more flexible interface later)
 *
 ******************************************************************************/


#ifndef __Q_GLUE_H__
#define __Q_GLUE_H__

#include "QWebcamPlugin.h"
#include "qCommandQueue.h"
#include "qFeedbackChannel.h"
#include "qWebcamDShow.h"

namespace Qwaq {
	// Command used to create a new camera.
	class CreateCameraCommand : public Command
	{
	public:
		CreateCameraCommand(char* uid, char* params, int paramsSize, FeedbackChannel* ch); 
		virtual void execute(void);
		int result;

	protected:
		std::string devicePath;
		WebcamParams parameters;
		FeedbackChannel* channel;
	};

	// Command used to destroy an existing camera.
	class DestroyCameraCommand : public Command
	{
	public:
		DestroyCameraCommand(int camIndex);
		virtual void execute(void);
		int result;

	protected:
		int cameraIndex;
	};

	// Command used to adjust camera properties on-the-fly.
	// Unlike other camera commands, we don't wait for this 
	// one to finish.
	class AdjustCameraCommand : public Command
	{
	public:
		AdjustCameraCommand(int camIndex, char* params, int paramsSize);
		virtual void execute(void);

	protected:
		int cameraIndex;
		WebcamParams parameters;
	};

	// Command used to run, pause, or stop a camera.
	class ControlCameraCommand : public Command
	{
	public:
		enum CommandType { RUN, PAUSE, STOP };
		ControlCameraCommand(int cameraIndex, CommandType t);
		virtual void execute(void);
		int result;
	protected:
		int cameraIndex;
		CommandType type;
	};

	// Command used to refresh the device list.
	class ListCamerasCommand : public Command
	{
	public:
		ListCamerasCommand();
		virtual void execute(void);
		int result;
	};

	// Command that does nothing (sometimes we simply need
	// to push something through the event queue).
	class NoOpCommand : public Command
	{
	public:
		NoOpCommand() : Command(true) { }
		virtual void execute(void) { };
	};

} // namespace Qwaq

#endif //#ifndef __Q_GLUE_H__

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
 * QWebCam implementation subclass for to the objective-c QTKit capture regime.
 ******************************************************************************/

#ifndef __Q_QTKIT_WEBCAM_H__
#define __Q_QTKIT_WEBCAM_H__

#include "qWebcam.h"
#include <QTKit/QTKit.h>
#include "CamOutputHandler.h"

#ifdef __LITTLE_ENDIAN__
#define Q_PIXEL_FORMAT k32BGRAPixelFormat
#else
#define Q_PIXEL_FORMAT k32ARGBPixelFormat
#endif

namespace Qwaq
{
	class WebcamQTKit : public Webcam
	{
	public:
		WebcamQTKit(char* const uid, WebcamParams* params, BufferPool2& bufferPool, FeedbackChannel* ch);

		virtual ~WebcamQTKit ();
		
		virtual void run();
		virtual void pause();
		virtual void stop();
		virtual void adjustParameters(WebcamParams& params);
		
		bool isValidCamera();	// So we can test instantiation

		void receiveFrame (void*pixelBufferBase, size_t size, double frameTime);
	
		char* cameraName();	// Matching the device-enumeration camDeviceName.
		char* cameraUID ();	// Matching the device-enumeration camDeviceUID
		
	protected:
	
		double	targetFrameInterval ;
		double	targetSampleTime ;
		
		CamOutputHandler* handler ;
		QTCaptureSession *session ;
		QTCaptureDevice  *device ;
		QTCaptureDeviceInput *input ;
		QTCaptureOutput *output ;
		
		char* camUID;
		char* camName;
		
	protected:
		void destroySession ();
		int	 setupCamera (char* uid);
		void configureOutput ();
		
		QTCaptureDevice * defaultCamDevice ();
		QTCaptureDevice * camDevice(char* uid);
		
	};
};

#endif //#ifndef __Q_QTKIT_WEBCAM_H__
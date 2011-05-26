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
 * qWebcamDShow.h
 * QWebcamPlugin (win32)
 *
 * Subclass of Webcam that interfaces with DirectShow.
 *
 ******************************************************************************/

#ifndef __Q_DIRECT_SHOW_WEBCAM_H__
#define __Q_DIRECT_SHOW_WEBCAM_H__

#include "qSharedQueue.h"
#include "qWebcam.h"

// XXXXX: what's this for?
#define INITGUID

// Google-revealed tricks to overcome missing dxtrans in older DirectX SDK
#pragma include_alias( "dxtrans.h", "qedit.h" )
#define __IDxtCompositor_INTERFACE_DEFINED__
#define __IDxtAlphaSetter_INTERFACE_DEFINED__
#define __IDxtJpeg_INTERFACE_DEFINED__
#define __IDxtKey_INTERFACE_DEFINED__
#include <qedit.h>

#include <windows.h>
#include <objbase.h>
#include <strmif.h>
#include <dshow.h>

namespace Qwaq
{
	class WebcamDShow : public Webcam
	{
	public:
		WebcamDShow(std::string devpath, WebcamParams* params, BufferPool2& bufferPool, FeedbackChannel* ch);
		virtual ~WebcamDShow();
		
		virtual void run();
		virtual void pause();
		virtual void stop();
		virtual void adjustParameters(WebcamParams& params);

		char*	cameraName();
		char*	cameraUID();

	protected:
		void receiveSample(double SampleTime, IMediaSample *pSample);

		void initFilterGraph();
		void createUninitializedFilterGraph();
		void destroyFilterGraph();
		void initializeCapture ();
		IBaseFilter*  getCaptureDevice (std::string devicePath);
		IBaseFilter*  getAnyCaptureDevice();
		void initGrabberExtent();
		bool privateSetGrabberExtent(int w, int h);
		void createGrabber();
		void initVideoInfo();
		void installGrabberInGraph();
		void createVideoWindow();

		IBaseFilter* pCaptureDevice;
		IMediaControl* pMediaControl;
		IBaseFilter* pGrabBase;
		ISampleGrabber* pGrabber;
		ICaptureGraphBuilder2* pBuild;
		IGraphBuilder* pGraph;
		IAMStreamConfig* pStreamConfig;
		IVideoWindow* pWindow;

		AM_MEDIA_TYPE mMediaType;
		VIDEOINFOHEADER mVideoInfoHeader;

		// Just declare GrabberCB here; we'll define it in a second... 
		class GrabberCB;
		GrabberCB* pGrabberCB;

		double targetFrameInterval;
		double targetSampleTime;

		std::string camName;
		std::string camDevicePath;

	protected:
		/* Nested class which handles callbacks from DirectShow filter graph. */
		class GrabberCB : public ISampleGrabberCB
		{
		public:
			GrabberCB(WebcamDShow& wc) : webcam(wc), refCount(0), frameCount(0) { }

			// Methods required by ISampleGrabberCB and IUnknown
			virtual HRESULT STDMETHODCALLTYPE BufferCB(double SampleTime, BYTE* pBuffer, long BufferLen);
			virtual HRESULT STDMETHODCALLTYPE SampleCB(double SampleTime, IMediaSample *pSample);
			virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);
			virtual ULONG STDMETHODCALLTYPE AddRef(void);
			virtual ULONG STDMETHODCALLTYPE Release(void);

		protected:
			WebcamDShow& webcam;
			int refCount;
			int frameCount;
		};
	};


};

#endif //#ifndef __Q_DIRECT_SHOW_WEBCAM_H__

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

#include "qWebcamDShow.h"
#include "qDShowUtil.h"
#include "qLogger.hpp"
#include "qException.h"
#include <assert.h>

// For converting BSTR to std::string
#include <comdef.h>

using namespace Qwaq;

WebcamDShow::WebcamDShow(std::string devpath, WebcamParams* params, BufferPool2& bufferPool, FeedbackChannel* ch) 
 : Webcam(params, bufferPool, ch), camDevicePath(devpath)
{
	qerr << endl << qTime() << " :  instantiating new WebcamDShow";
	
	pCaptureDevice = NULL;
	pMediaControl = NULL;
	pGrabBase = NULL;
	pGrabber = NULL;
	pBuild = NULL;
	pGraph = NULL;
	pStreamConfig = NULL;
	pWindow = NULL;

	pGrabberCB = new GrabberCB(*this);
	if (!pGrabberCB) {
		qerr << endl << "could not instantiate a GrabberCB";
		throw QE_ALLOCATION_FAILED;
	}

	try {
		initFilterGraph();
	} 
	catch(...) {
		qerr << endl << "WebcamDShow caught exception while initializing filter graph";
		WebcamDShow::~WebcamDShow();
		throw;
	}

	targetFrameInterval = 1.0 / parameters.fps;
	targetSampleTime = 0.0;
}


WebcamDShow::~WebcamDShow()
{
	try {
		destroyFilterGraph();
		qerr << endl << qTime() << " :  ~WebcamDShow() destroyed filter graph";
	}
	catch(...) {
		qerr	<< endl << qTime()
				<< " :  ~WebcamDShow() caught exception while destroying filter graph (BAD!)";
	}
	if (pGrabberCB) delete pGrabberCB;

}


void 
WebcamDShow::receiveSample(double SampleTime, IMediaSample *pSample)
{
	HRESULT hr;

	if (targetSampleTime == 0.0) {
		// This is the first frame... set up expected time for next sample
		targetSampleTime = SampleTime;
	}

	// Check if we are ready for another sample (subtract a small fudge-factor
	// from the targetSampleTime so that we don't accidentally drop frames
	// due to jitter).
	if (SampleTime > (targetSampleTime - 0.2*targetFrameInterval)) {
		// Accept this sample, and update next targetSampleTime.
		targetSampleTime += targetFrameInterval;
		if (targetSampleTime < SampleTime) {
			// Atypical, but worth bulletproofing against
			targetSampleTime = SampleTime + targetFrameInterval;
		}
	}
	else {
		// The sample is early.  Discard it and wait for the next one.
		return;
	}

	// Get a buffer to store the frame data in.
	int sz = pSample->GetActualDataLength();
	BufferPool2::ptr_type buf = pool.getBuffer(sz);
	if (!buf) {
		qerr << endl << qTime() << "WebcamDShow::receiveSample(): could not obtain Buffer of size: " << sz;
		return;
	}

	// Copy the frame data into the buffer
	unsigned char *data_ptr;
	hr = pSample->GetPointer(&data_ptr);
	if (FAILED(hr)) {
		qerr << endl << qTime() << "WebcamDShow::receiveSample(): failed to get pointer to frame data" << hr;
		return;
	}
	buf->copyFrom(sz, data_ptr);

	// Instantiate a new VideoFrameEvent to communicate back to Squeak.
	VideoFrameEvent *evt = new VideoFrameEvent(buf);
	if (!evt) {
		qerr << endl << qTime() << "WebcamDShow::receiveSample(): failed to instantiate VideoFrameEvent";
		return;
	}
	evt->squeakEvent.sampleTime = SampleTime;

	// Pass the frame event to Squeak
	grabbedFrame(evt);


//	LONGLONG start, end;
//	REFERENCE_TIME refStart, refEnd;
//
//	qerr << endl << "RECEIVED SAMPLE: ";
//	qerr << endl << "    - sample time: " << SampleTime;
//
//	hr = pSample->GetMediaTime(&start, &end);
//	if (FAILED(hr)) qerr << endl << "     - couldn't get media times ";
//	else qerr << endl << "     - media start/end: " << start << "/" << end;
//
//	hr = pSample->GetTime(&refStart, &refEnd);
//	if (FAILED(hr)) qerr << endl << "     - couldn't get reference times ";
//	else qerr << endl << "     - reference start/end: " << refStart << "/" << refEnd;
}


void
WebcamDShow::run()
{
	pMediaControl->Run();
}


void
WebcamDShow::pause()
{
	pMediaControl->Pause();
	targetSampleTime = 0.0;
}


void
WebcamDShow::stop()
{
	pMediaControl->Stop();
	targetSampleTime = 0.0;
}


void
WebcamDShow::adjustParameters(WebcamParams& params)
{
	if (params.width != parameters.width  ||  params.height != parameters.height) {
		qerr << endl << qTime() << "WebcamDShow::adjustParameters(): cannot change height/width on-the-fly";
		qerr.flush();
	}
	if (params.fps != parameters.fps) {
		// Need to adjust frame-rate settings
		parameters.fps = params.fps;

		// Don't print out FPS changes because we'll probably be doing them quite often.
//		qerr << endl << qTime() << "WebcamDShow::adjustParameters(): FPS changed to " << parameters.fps;
//		qerr.flush();

		if (targetSampleTime == 0.0) {
			// No samples received yet; just set the target interval
			targetFrameInterval = 1.0 / parameters.fps;
		}
		else {
			// Adjust the targetSampleTime for the new target interval
			targetSampleTime -= targetFrameInterval;
			targetFrameInterval = 1.0 / parameters.fps;
			targetSampleTime += targetFrameInterval;
		}
	}
}


void 
WebcamDShow::initFilterGraph()
{
	createUninitializedFilterGraph();
	initializeCapture();
	initGrabberExtent();
	createGrabber();
	// Yoshiki had a path (never taken) to deal with DV cameras instead
	// of webcams.  If we need this, consult the DShowVideoCodec plugin 
	// source code (or just look at the MSDN docs)
	initVideoInfo();
	installGrabberInGraph();
	createVideoWindow();
}


void 
WebcamDShow::destroyFilterGraph()
{
	if (pWindow) pWindow->Release();
	if (pGrabBase) pGrabBase->Release();
	if (pCaptureDevice) pCaptureDevice->Release();
	if (pMediaControl) pMediaControl->Release();
	if (pStreamConfig) pStreamConfig->Release();

#ifdef REGISTER_FILTERGRAPH
	// Remove filter graph from the running object table   
	if (g_dwGraphRegister) {
		RemoveGraphFromRot(g_dwGraphRegister);
		g_dwGraphRegister = 0;
	}
#endif

	if (pGrabber) pGrabber->Release();
	if (pGraph) pGraph->Release();
	if (pBuild) pBuild->Release();
}


void
WebcamDShow::createUninitializedFilterGraph()
{
	HRESULT hr;
	hr = CoCreateInstance(	CLSID_CaptureGraphBuilder2, 
							NULL, 
							CLSCTX_INPROC, // XXXXX: MSDN examples use CLSCTX_INPROC_SERVER... what's the diff?
							IID_ICaptureGraphBuilder2,
							(void**)&pBuild );
	if (FAILED(hr)) {
		qerr << endl << "createUninitializedFilterGraph(): failed to create ICaptureGraphBuilder2: " << hr;
		throw QE_COM_ERROR;
	}

	hr = CoCreateInstance(	CLSID_FilterGraph,
							NULL,
							CLSCTX_INPROC,
							IID_IGraphBuilder,
							(void**)&pGraph );
	if (FAILED(hr)) {
		qerr << endl << "createUninitializedFilterGraph(): failed to create IFilterGraph: " << hr;
		throw QE_COM_ERROR;
	}

	pBuild->SetFiltergraph(pGraph);

	// Get a media control interface.
	hr = pGraph->QueryInterface(IID_IMediaControl, (void**)&pMediaControl);
	if (!pMediaControl) {
		qerr << endl << "createUninitializedFilterGraph(): failed to get media control interface: " << hr;
		throw QE_COM_ERROR;
	}
}

char* WebcamDShow::cameraName() 
{
	if (camName.length() < 1) {
		return NULL;
	}
	return (char*) camName.c_str();
}

char* WebcamDShow::cameraUID()
{
	if (camDevicePath.length() < 1) {
		return NULL;
	}
	return (char*) camDevicePath.c_str();
}

// Open the first available video capture device.
// If successful, returns non-nil.
// My camName and camDevicePath string members are set to identify the result.
IBaseFilter* WebcamDShow::getAnyCaptureDevice()
{
	HRESULT hr;
	IEnumMoniker *pEnumerator = NULL;
	IBaseFilter  *pResult = NULL;

	// Create a video-input device enumerator

	pEnumerator = pqDeviceEnumerator();
	if (!pEnumerator) {
		throw QE_DEVICE_UNAVAILABLE;
	}

	// Iterate over the available input devices
	IMoniker* pMoniker;
	ULONG fetched;
	IPropertyBag* pBag;
	VARIANT variant;
	qerr << endl << qTime() << " :  Iterating over camera devices:" << endl;
	while (S_OK == (hr = pEnumerator->Next(1, &pMoniker, &fetched))) 
	{
		hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pBag);
		if (hr != S_OK) {
			qerr << "     - failed to get property-bag for device: " << hr << endl;
			pMoniker->Release();
			continue;  // try the next device
		}
		variant.vt = VT_BSTR;

		// Device user-friendly name.
		hr = pBag->Read(L"FriendlyName", &variant, NULL);
		if (hr != S_OK) {
			qerr << "     - failed to get 'friendly name' for device: " << hr << endl;
			pBag->Release();
			pMoniker->Release();
			continue;
		}
		qerr << "     - found device: " << _bstr_t(variant.bstrVal) << endl;
		camName = _bstr_t(variant.bstrVal);
		SysFreeString(variant.bstrVal);
		// UID - 'DevicePath'
		hr = pBag->Read(L"DevicePath", &variant, NULL);
		if (hr != S_OK) {
			qerr << "     - failed to get uuid (devicepath): " << hr << endl;
			pBag->Release();
			pMoniker->Release();
			continue;
		}
		camDevicePath = _bstr_t(variant.bstrVal);
		SysFreeString(variant.bstrVal);
		pBag->Release();

		// Attempt to connect (we use the first device that we successfully connect to).
		hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pResult);
		pMoniker->Release();
		if (S_OK == hr) {
			qerr << "          - successfully bound to " << camName << endl;
			return pResult;
		} else {
			qerr << "          - failed to bind to " << camName <<": " << hr << endl;
		}
	}
	camName = "";
	camDevicePath = "";
	return NULL;
}

// Open the indicated video capture device.
IBaseFilter* WebcamDShow::getCaptureDevice(std::string devpath)
{
	HRESULT hr;
	IEnumMoniker *pEnumerator = NULL;
	IBaseFilter  *pResult = NULL;

	// Create a video-input device enumerator

	pEnumerator = pqDeviceEnumerator();
	if (!pEnumerator) {
		throw QE_DEVICE_UNAVAILABLE;
	}

	// Iterate over the available input devices
	IMoniker* pMoniker;
	ULONG fetched;
	IPropertyBag* pBag;
	VARIANT variant;
	qerr << endl << qTime() << " : Searching devices for " << devpath << ":" << endl;
	while (S_OK == (hr = pEnumerator->Next(1, &pMoniker, &fetched))) 
	{
		hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pBag);
		if (hr != S_OK) {
			qerr << "     - failed to get property-bag for device: " << hr << endl;
			pMoniker->Release();
			continue;  // try the next device
		}
		variant.vt = VT_BSTR;

		// CHeck for match with 'DevicePath'
		hr = pBag->Read(L"DevicePath", &variant, NULL);
		if (hr != S_OK) {
			qerr << "     - failed to get uuid (devicepath): " << hr << endl;
			pBag->Release();
			pMoniker->Release();
			continue;
		}

		if (devpath.compare(_bstr_t(variant.bstrVal)) == 0) {
			camDevicePath = devpath;
		} else {
			// This is not the droid
			SysFreeString(variant.bstrVal);
			pBag->Release();
			pMoniker->Release();
			continue;
		}

		// Device user-friendly name.
		hr = pBag->Read(L"FriendlyName", &variant, NULL);
		if (hr != S_OK) {
			qerr << "     - failed to get 'friendly name' for device: " << hr << endl;
			pBag->Release();
			pMoniker->Release();
			continue;
		}
		qerr << "     - found device: " << _bstr_t(variant.bstrVal) << endl;
		camName = _bstr_t(variant.bstrVal);
		SysFreeString(variant.bstrVal);
		pBag->Release();

		// Attempt to connect (we use the first device that we successfully connect to).
		hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pResult);
		pMoniker->Release();
		if (S_OK == hr) {
			qerr << "          - Successfully bound to " << camName << endl;
			return pResult;
		} else {
			qerr << "          - Failed to bind to " << camName <<": " << hr << endl;
			pResult = NULL;
		}
	}
	return NULL;
}

// Open the capture device as indicated by my camDevicePath;
// if camDevicePath is not specified, open the first available camera device.
// If successful, this sets pCaptureDevice, and pStreamConfig,
// and wires the capture device into the filter graph.
void WebcamDShow::initializeCapture()
{
	HRESULT hr;

	if (camDevicePath.length() > 0) {
		pCaptureDevice = getCaptureDevice(camDevicePath);
	} else {
		pCaptureDevice = getAnyCaptureDevice();
	}

	if (pCaptureDevice == NULL) throw QE_DEVICE_UNAVAILABLE;

	// Now we have the device... create a 'stream-config' object on it.
	// Don't listen to the MSDN docs for "Configure the Video Output Format",
	// they are lying to you (they pass completely bogus arguments to FindInterface)
	hr = pBuild->FindInterface(	NULL, 
								&MEDIATYPE_Video, 
								pCaptureDevice, 
								IID_IAMStreamConfig,
								(void**)&pStreamConfig);
	if (pStreamConfig == NULL) {
		qerr << endl << "getDefaultCaptureDevice() :  couldn't get StreamConfig for device";
		throw QE_DSHOW_ERROR;
	}

	hr = pGraph->AddFilter(pCaptureDevice, L"Cap");
	if (FAILED(hr)) {
		qerr << endl << "initFilterGraph(): failed to add camera to graph: " << hr;
		throw QE_COM_ERROR;
	}
}


void
WebcamDShow::initGrabberExtent()
{
	/* Try to set the camera dimensions to the requested size, otherwise
	   try the smaller standard sizes. */
	bool result = privateSetGrabberExtent(parameters.width, parameters.height);
	if (!result && (640 < parameters.width)) {
		result = privateSetGrabberExtent(640, 480);
	}
	if (!result && (320 < parameters.width)) {
		result = privateSetGrabberExtent(320, 240);
	}
	if (!result && (160 < parameters.width)) {
		result = privateSetGrabberExtent(160, 120);
	}
	if (!result) {
		qerr << endl << "initGrabberExtent(): could not set init grabber extent";
		throw QE_DSHOW_ERROR;
	}
}


bool 
WebcamDShow::privateSetGrabberExtent(int w, int h)
{
	HRESULT hr;
	int iCount = 0, iSize = 0;

	hr = pStreamConfig->GetNumberOfCapabilities(&iCount, &iSize);
	qerr	<< endl 
			<< "privateSetGrabberExtent():  attempting to set size to "
			<< w << "x" << h << "; found " << iCount << " capabilities";

	// Check the size to make sure we pass in the correct structure.
	if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS))
	{
		// Use the video capabilities structure.
		int iFormat;
		for (iFormat = 0; iFormat < iCount; iFormat++)
		{
			VIDEO_STREAM_CONFIG_CAPS scc;
			AM_MEDIA_TYPE *pMediaType;
			hr = pStreamConfig->GetStreamCaps(iFormat, &pMediaType, (BYTE*)&scc);
			if (SUCCEEDED(hr))
			{
				/* Examine the format, and possibly use it. */
				/* There may be several formats with the same output size.  Just pick the first one. */
				if ((w == scc.MaxOutputSize.cx) && (h == scc.MaxOutputSize.cy)) {
					hr = pStreamConfig->SetFormat(pMediaType);
					if (SUCCEEDED(hr)) {
						qerr << endl << "    - succeeded";
						parameters.width = scc.MaxOutputSize.cx;
						parameters.height = scc.MaxOutputSize.cy;
						// Delete the media type when you are done.
						DeleteMediaType(pMediaType);
						return true;
					}
					qerr << endl << "    - failed (result: " << hr << ")";
				}
				// Delete the media type when you are done.
				DeleteMediaType(pMediaType);
			}
		}
	}
	return false;
}


void 
WebcamDShow::createGrabber()
{
	HRESULT hr;
	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_SERVER, IID_ISampleGrabber, (void**)&pGrabber);
	if (!pGrabber) {
		// is qedit.dll registered?  (don't know why this question in particular; it's from Yoshiki's code)
		qerr << endl << "createGrabber(): failed to instantiate SampleGrabber (is qedit.dll registered?)";
		throw QE_COM_ERROR;
	}

	hr = pGrabber->QueryInterface(IID_IBaseFilter, (void**)&pGrabBase);
	if (!pGrabBase) {
		// is qedit.dll registered? (don't know why this question in particular; it's from Yoshiki's code)
		qerr << endl << "createGrabber(): failed to get IBaseFilter interface for grabber (is qedit.dll registered?)";
		throw QE_COM_ERROR;
	}
}


void
WebcamDShow::initVideoInfo()
{

	memset(&mMediaType, 0, sizeof(AM_MEDIA_TYPE));
	memset(&mVideoInfoHeader, 0, sizeof(VIDEOINFOHEADER));

	mVideoInfoHeader.rcSource.top = 0;
	mVideoInfoHeader.rcSource.left = 0;
	mVideoInfoHeader.rcSource.right = 320;
	mVideoInfoHeader.rcSource.bottom = 240;
	mVideoInfoHeader.rcTarget.top = 0;
	mVideoInfoHeader.rcTarget.left = 0;
	mVideoInfoHeader.rcTarget.right = 320;
	mVideoInfoHeader.rcTarget.bottom = 240;

	mVideoInfoHeader.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	mVideoInfoHeader.bmiHeader.biPlanes = 1;
	mVideoInfoHeader.bmiHeader.biBitCount = 32;
	mVideoInfoHeader.bmiHeader.biCompression = BI_BITFIELDS;

	mMediaType.majortype = MEDIATYPE_Video;
	mMediaType.subtype = MEDIASUBTYPE_RGB32;
	mMediaType.formattype = FORMAT_VideoInfo;
	mMediaType.cbFormat = sizeof(VIDEOINFOHEADER);
	mMediaType.pbFormat = (BYTE*)&mVideoInfoHeader;
	mMediaType.lSampleSize = 0;
}

void
WebcamDShow::installGrabberInGraph()
{
	// We just set up mMediaType in 'initVideoInfo()'
	HRESULT hr = pGrabber->SetMediaType(&mMediaType);
	if (FAILED(hr)) {
		qerr << endl << "installGrabberInGraph(): could not set media type: " << hr;
		throw QE_DSHOW_ERROR;
	}

	// Add the grabber to the graph
	hr = pGraph->AddFilter(pGrabBase,  L"Grabber");
	if (FAILED(hr)) {
		qerr << endl << "installGrabberInGraph(): could not add sample grabber to graph: " << hr;
		throw QE_DSHOW_ERROR;
	}

	// Find the input and output pins, and connect them
	IPin * pCapOut = GetOutPin(pCaptureDevice, 0);
	IPin * pGrabIn = GetInPin(pGrabBase, 0);
	hr = pGraph->Connect(pCapOut, pGrabIn);
	pCapOut->Release();
	pGrabIn->Release();
	if (FAILED(hr)) {
		qerr	<< endl << "installGrabberInGraph(): could not connect capture pin #0 to grabber: " << hr
				<< endl << "      - is the capture device being used by another application?";
		throw QE_DSHOW_ERROR;
	}

	// Don't buffer the samples as they pass through
	pGrabber->SetBufferSamples(FALSE);

	// Only grab one at a time, stop stream after grabbing one sample
	// XXXXX: this seems NOT what we want.
	pGrabber->SetOneShot(FALSE);

	// Set the callback, so we can grab the one sample.
	// The second argument is zero, meaning we get the original media samples
	// rather than a copied buffer.
	pGrabber->SetCallback(pGrabberCB, 0);

	// Add our graph to the running object table, which will allow
	// the GraphEdit application to "spy" on our graph
#ifdef REGISTER_FILTERGRAPH
	hr = AddGraphToRot(pGraph, &g_dwGraphRegister);
	if (FAILED(hr)) {
		Error(TEXT("Failed to register filter graph with ROT!"));
		g_dwGraphRegister = 0;
	}
#endif
}

void
WebcamDShow::createVideoWindow()
{
	// Even though we never show the window, we still need it (otherwise eg: the Logitech driver
	// will open an annoying window itself).

	HRESULT hr;
	hr = pGraph->QueryInterface(IID_IVideoWindow, (void**)&pWindow);
	if (FAILED(hr)) {
		qerr << endl << "createVideoWindow(): failed to create video window";
		throw QE_DSHOW_ERROR;
	}

	pWindow->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS);

	// We never show the window, so we set the following properties:
	pWindow->put_AutoShow(OAFALSE);
	pWindow->put_Visible(OAFALSE);
	pWindow->put_Owner((OAHWND)NULL);
	// Otherwise we might use:
	//	pWindow->putAutoShow(OATRUE);
	//	pWindow->put_Owner((OAHWND)stWindow)); //we would need to get 'stWindow' somehow
	//	pWindow->put_Visible(OATRUE);

	pWindow->put_Left(0);
	pWindow->put_Top(0);
	pWindow->put_Width(parameters.width);
	pWindow->put_Height(parameters.height);
}


HRESULT 
WebcamDShow::GrabberCB::BufferCB(double SampleTime, BYTE* pBuffer, long BufferLen)
{
	// This shouldn't happen... we configured DirectShow to return samples, not buffers.
	qerr << endl << "GrabberCB::BufferCB(): should never be called; we expect to receive samples, not buffers";
	return S_OK;
}

HRESULT
WebcamDShow::GrabberCB::SampleCB(double SampleTime, IMediaSample *pSample)
{
	webcam.receiveSample(SampleTime, pSample);
	return S_OK;
}

HRESULT
WebcamDShow::GrabberCB::QueryInterface(REFIID riid, void** ppvObject)
{
	if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ISampleGrabberCB)) {
		*ppvObject = this;
		return NOERROR;
	}
	*ppvObject = NULL;
	return E_NOINTERFACE;
}

ULONG
WebcamDShow::GrabberCB::AddRef()
{
	return ++refCount;
}

ULONG
WebcamDShow::GrabberCB::Release()
{
	return --refCount;
}






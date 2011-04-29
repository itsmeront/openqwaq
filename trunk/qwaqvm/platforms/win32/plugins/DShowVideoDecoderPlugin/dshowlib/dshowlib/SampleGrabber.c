/* SampleGrabber.c

  Copyright (c) 2002, 2007 Yoshiki Ohshima
  All rights reserved.

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the 'Software'),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, provided that the above copyright notice(s) and this
  permission notice appear in all copies of the Software and that both the
  above copyright notice(s) and this permission notice appear in supporting
  documentation.

  THE SOFTWARE IS PROVIDED 'AS IS'.  USE ENTIRELY AT YOUR OWN RISK.

*/
#define INITGUID
#include <windows.h>
#include <objbase.h>
#include <stdio.h>

#include <strmif.h>
#include <dshow.h>
#include <qedit.h>

#include "SampleGrabber.h"

#ifdef _DEBUG
#  define REGISTER_FILTERGRAPH
#endif



//  Free an existing media type (ie free resources it holds)
//  (as suggested by MSDN docs, copy this code from the Platform SDK
//   so that we don't have to link the DirectShow base class library)
void WINAPI FreeMediaType(__inout AM_MEDIA_TYPE* mt)
{
    if (mt->cbFormat != 0) {
        CoTaskMemFree((PVOID)mt->pbFormat);

        // Strictly unnecessary but tidier
        mt->cbFormat = 0;
        mt->pbFormat = NULL;
    }
    if (mt->pUnk != NULL) {
        mt->pUnk->lpVtbl->Release(mt->pUnk);
        mt->pUnk = NULL;
    }
}

// general purpose function to delete a heap allocated AM_MEDIA_TYPE structure
// which is useful when calling IEnumMediaTypes::Next as the interface
// implementation allocates the structures which you must later delete
// the format block may also be a pointer to an interface to release
//  (as suggested by MSDN docs, copy this code from the Platform SDK
//   so that we don't have to link the DirectShow base class library)

void WINAPI DeleteMediaType(__inout_opt AM_MEDIA_TYPE *pmt)
{
    // allow NULL pointers for coding simplicity

    if (pmt == NULL) {
        return;
    }

    FreeMediaType(pmt);
    CoTaskMemFree((PVOID)pmt);
}




typedef struct _CSampleGrabberCB {
	ISampleGrabberCB iGrabber;
	int ref;

	int cameraType;
	AM_MEDIA_TYPE VideoType;

	// Only one of these two will be used, depending on what kind 
	// of camera we are grabbing from.
	DVINFO dvinfo;
	VIDEOINFOHEADER videoinfo;

	int decodeWidth;
	int decodeHeight;

	IBaseFilter *pCaptureDevice;
	IMediaControl *pMediaControl;
} CSampleGrabberCB;


static int initStillGraph(CSampleGrabberCB* pCB);
static int destroyStillGraph(CSampleGrabberCB *pCB);

static ISampleGrabberCBVtbl m_cb_vtbl;
static CSampleGrabberCB m_CB;

static DWORD g_dwGraphRegister = 0;  // For running object table

static IGraphBuilder *pGraph = NULL;
static ISampleGrabber *pGrabber = NULL;

// XXXXX: Josh... I'm not sure whether we only need this thing during
// initialization, or if we want it to hang around.  I'm assuming the 
// latter for now
static ICaptureGraphBuilder2 *pBuild;

static HANDLE hThread = NULL;
static HANDLE mutex = NULL;

static int captureNextFrame = false;
static unsigned char *frameBuf = NULL;
static double capturedSampleTime = 0.0;

static HWND stWindow = NULL;
static int putOwnerCalled = false;

static int renderX;
static int renderY;
static int renderW;
static int renderH;

static void (*frameCallBack)(double sampleTime) = NULL;

static int action = DECODER_NOTHING;

int decoderLastError = DECODER_ERROR_FALSE;

//#define REPORT_ERROR_VIA_DIALOGS 1

#ifdef REPORT_ERROR_VIA_DIALOGS
void
Error(TCHAR * pText)
{
  MessageBox(NULL, pText, TEXT("YOO-HOO!!! Error!"), MB_OK | MB_TASKMODAL | MB_SETFOREGROUND);
}
void
MyError(char* format, ...)
{
	char buf[1024];
	TCHAR buf2[256];
	va_list ap;
	int i;
	
	va_start(ap, format);
	_vsnprintf(buf, 1024, format, ap);
	va_end(ap);

	i = 0;
	while (buf2[i] = buf[i]) i++;

	Error(buf2);
}
#else
void
Error(TCHAR * pText) { }
void
MyError(char* format, ...) { }
#endif


// Lazily-initialized accessor for IMediaControl.
IMediaControl* getMediaControl(CSampleGrabberCB *pCB)
{
	HRESULT hr;
	if (pCB->pMediaControl == NULL) {
		if (pGraph == NULL || !putOwnerCalled) {
			decoderLastError = DECODER_ERROR_UNINITIALIZED;
			MyError("Can't get media control because graph hasn't been initialized");
			return NULL;
		}

		hr = pGraph->lpVtbl->QueryInterface(pGraph, &IID_IMediaControl, &(pCB->pMediaControl));
		if (FAILED(hr)) {
			decoderLastError = DECODER_ERROR_INTERFACE;
			MyError("Can't get media control: failure while querying interface (result: %x)", hr);
			return NULL;
		}
	}
	return pCB->pMediaControl;
}

static DWORD WINAPI
decodeProcess(LPVOID pCSGCB);

static HRESULT
AddGraphToRot(IGraphBuilder *pUnkGraph, DWORD *pdwRegister) 
{
    IMoniker * pMoniker;
    IRunningObjectTable *pROT;
    WCHAR wsz[128];
    HRESULT hr;

    if (FAILED(GetRunningObjectTable(0, &pROT)))
        return E_FAIL;

    sprintf_s(wsz, 128, L"FilterGraph %08x pid %08x", (DWORD_PTR)pUnkGraph, 
              GetCurrentProcessId());

    hr = CreateItemMoniker(L"!", wsz, &pMoniker);
    if (SUCCEEDED(hr)) 
    {
        hr = pROT->lpVtbl->Register(pROT, 0, (IUnknown*)pUnkGraph, pMoniker, pdwRegister);
		RELEASE(pMoniker);
    }
    RELEASE(pROT);
    return hr;
}

// Removes a filter graph from the Running Object Table
static void RemoveGraphFromRot(DWORD pdwRegister)
{
    IRunningObjectTable *pROT;

    if (SUCCEEDED(GetRunningObjectTable(0, &pROT))) 
    {
        pROT->lpVtbl->Revoke(pROT, pdwRegister);
	RELEASE(pROT);
    }
}

static HRESULT __stdcall
CSampleGrabberCBQueryInterface(ISampleGrabberCB *pGrabberCB, REFIID riid, void ** ppv)
{

  CSampleGrabberCB *this = IMPL(CSampleGrabberCB, iGrabber, pGrabberCB);

  if (IsEqualIID(riid, &IID_IUnknown) || IsEqualIID(riid, &IID_ISampleGrabberCB)) {
      *ppv = (void *) (&this->iGrabber);
      return NOERROR;
  }
  *ppv = NULL;
  return E_NOINTERFACE;
}

static ULONG __stdcall
CSampleGrabberCBAddRef(ISampleGrabberCB *pGrabber)
{
  return 1;
}

static ULONG __stdcall
CSampleGrabberCBRelease(ISampleGrabberCB *pGrabber)
{
  return 0;
}

static HRESULT __stdcall
CSampleGrabberSampleCB(ISampleGrabberCB *pGrabber,
		       double SampleTime,
		       IMediaSample * pSample)
{
  return S_OK;
}

static HRESULT __stdcall
CSampleGrabberBufferCB(ISampleGrabberCB *pGrabber,
		       double dblSampleTime,
		       BYTE * pBuffer,
		       long lBufferSize)
{
  if (captureNextFrame) {
    captureNextFrame = false;
    memcpy(frameBuf, pBuffer, lBufferSize);
    capturedSampleTime = dblSampleTime;
  }

  if (frameCallBack) {
    /* frameCallBack(captureSampleTime); */
    frameCallBack(dblSampleTime);
  }
  
  return S_OK;
}

static void
initializeVtbl(CSampleGrabberCB *cb)
{
  cb->iGrabber.lpVtbl = &m_cb_vtbl;
  cb->iGrabber.lpVtbl->QueryInterface = CSampleGrabberCBQueryInterface;
  cb->iGrabber.lpVtbl->AddRef = CSampleGrabberCBAddRef;
  cb->iGrabber.lpVtbl->Release = CSampleGrabberCBRelease;
  
  cb->iGrabber.lpVtbl->BufferCB = CSampleGrabberBufferCB;
  cb->iGrabber.lpVtbl->SampleCB = CSampleGrabberSampleCB;
}

static HRESULT
GetPin(IBaseFilter *pFilter, PIN_DIRECTION dirrequired, int iNum, IPin **ppPin)
{
  HRESULT hr;
  IEnumPins * pEnum = NULL;

  ULONG ulFound;
  IPin *pPin;
  
  *ppPin = NULL;
  
  hr = pFilter->lpVtbl->EnumPins(pFilter, &pEnum);
  if (FAILED(hr)) 
    return hr;

  hr = E_FAIL;

  while (S_OK == pEnum->lpVtbl->Next(pEnum, 1, &pPin, &ulFound)) {
    PIN_DIRECTION pindir = (PIN_DIRECTION)3;
    pPin->lpVtbl->QueryDirection(pPin, &pindir);
    if (pindir == dirrequired) {
      if(iNum == 0) {
	*ppPin = pPin;
	// Found requested pin, so clear error
	// pPin is the return value, so it shouldn't be released
	hr = S_OK;
	break;
      }

      iNum--;
    }
    
    pPin->lpVtbl->Release(pPin);
  }

  if (pEnum != NULL) {
    pEnum->lpVtbl->Release(pEnum);
    pEnum = NULL;
  }

  return hr;
}

static IPin *
GetInPin(IBaseFilter * pFilter, int Num)
{
  IPin *pPin = NULL;
  GetPin(pFilter, PINDIR_INPUT, Num, &pPin);
  return pPin;
}


static IPin *
GetOutPin( IBaseFilter * pFilter, int Num )
{
  IPin *pPin = NULL;
  GetPin(pFilter, PINDIR_OUTPUT, Num, &pPin);
  return pPin;
}

static void
cleanupLiveObjects(CSampleGrabberCB *pCB)
{
  IVideoWindow *pWindow = NULL;
  HRESULT hr;

  	if (pCB->pCaptureDevice != NULL) {
		RELEASE(pCB->pCaptureDevice);
		pCB->pCaptureDevice = NULL;
	}

	if (pCB->pMediaControl != NULL) {
		RELEASE(pCB->pMediaControl);
		pCB->pMediaControl = NULL;
	}

#ifdef REGISTER_FILTERGRAPH
  // Remove filter graph from the running object table   
  if (g_dwGraphRegister) {
    RemoveGraphFromRot(g_dwGraphRegister);
    g_dwGraphRegister = 0;
  }
#endif

  if (pGraph != NULL) {
    hr = pGraph->lpVtbl->QueryInterface(pGraph, &IID_IVideoWindow, &pWindow);
    if (!FAILED(hr)) {
      pWindow->lpVtbl->put_Visible(pWindow, false);
      pWindow->lpVtbl->put_Owner(pWindow, (OAHWND)NULL);
      RELEASE(pWindow);
    }
  }

  putOwnerCalled = false;

  RELEASE(pGrabber);
  RELEASE(pGraph);
}
 
static int
destroyStillGraph(CSampleGrabberCB *pCB)
{
	cleanupLiveObjects(pCB);

	stWindow = NULL;
	frameCallBack = NULL;

	if (frameBuf != NULL) {
		VirtualFree(frameBuf, m_CB.decodeWidth * m_CB.decodeHeight * 4, MEM_RELEASE);
		frameBuf = NULL;

		m_CB.decodeWidth = m_CB.decodeHeight = 0;
	}

	CoUninitialize();

	if (hThread) {
		HANDLE tempThread = hThread;
		hThread = NULL;
		CloseHandle(mutex);
		mutex = NULL;
		TerminateThread(tempThread, 0);
	}
	decoderLastError = DECODER_ERROR_TRUE;
}


static int
decoderRun(CSampleGrabberCB *pCB)
{
	HRESULT hr;
	IMediaControl *pControl = getMediaControl(pCB);

	if (pControl == NULL) return false; // error code will have been set in getMediaControl()

	hr = pControl->lpVtbl->Run(pControl);
	if (FAILED(hr)) {
		decoderLastError = DECODER_ERROR_CONTROL;
		return false;
	}

	decoderLastError = DECODER_ERROR_TRUE;
	return true;
}


static int
decoderStop(CSampleGrabberCB *pCB)
{
	HRESULT hr;
	IMediaControl *pControl = getMediaControl(pCB);

	if (pControl == NULL) return false; // error code will have been set in getMediaControl()

	hr = pControl->lpVtbl->Stop(pControl);
	if (FAILED(hr)) {
		decoderLastError = DECODER_ERROR_CONTROL;
		return false;
	}

	decoderLastError = DECODER_ERROR_TRUE;
	return true;
}
  

static int
decoderPause(CSampleGrabberCB *pCB)
{
	HRESULT hr;
	IMediaControl *pControl = getMediaControl(pCB);

	if (pControl == NULL) return false; // error code will have been set in getMediaControl()

	hr = pControl->lpVtbl->Pause(pControl);
	if (FAILED(hr)) {
		decoderLastError = DECODER_ERROR_CONTROL;
		return false;
	}

	decoderLastError = DECODER_ERROR_TRUE;
	return true;
}

static DWORD WINAPI
decodeProcess(LPVOID pCSGCB)
{
	CSampleGrabberCB *pCB = (CSampleGrabberCB*)pCSGCB;

	MyError("Requested width: %d height: %d", pCB->decodeWidth, pCB->decodeHeight);

	if (initStillGraph(pCB) == false) {
		destroyStillGraph(pCB);    
		return 0;
	}

	while (1) {
		WaitForSingleObject(mutex, INFINITE);
		if (hThread == NULL) {
			return 0;
		}
		switch (action) {
	case DECODER_RUN:
		action = DECODER_NOTHING;
		decoderRun(pCB);
		break;
	case DECODER_STOP:
		action = DECODER_NOTHING;
		decoderStop(pCB);
		break;
	case DECODER_PAUSE:
		action = DECODER_NOTHING;
		decoderPause(pCB);
		break;
	case DECODER_DESTROY:
		action = DECODER_NOTHING;
		destroyStillGraph(pCB);
		return 0;
		}
	}
}

static void
initVideoInfo(CSampleGrabberCB *pCB)
{
	// Get pointers to these two fields to make subsequent code cleaner
	AM_MEDIA_TYPE *VideoType = &(pCB->VideoType);
	VIDEOINFOHEADER *videoinfo = &(pCB->videoinfo);

	memset(VideoType, 0, sizeof(AM_MEDIA_TYPE));
	memset(videoinfo, 0, sizeof(VIDEOINFOHEADER));

	VideoType->majortype = MEDIATYPE_Video;
	VideoType->subtype = MEDIASUBTYPE_RGB24;

	videoinfo->rcSource.top = 0;
	videoinfo->rcSource.left = 0;
	videoinfo->rcSource.right = 320;
	videoinfo->rcSource.bottom = 240;
	videoinfo->rcTarget.top = 0;
	videoinfo->rcTarget.left = 0;
	videoinfo->rcTarget.right = 320;
	videoinfo->rcTarget.bottom = 240;

	videoinfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	videoinfo->bmiHeader.biPlanes = 1;
	videoinfo->bmiHeader.biBitCount = 32;
	videoinfo->bmiHeader.biCompression = BI_BITFIELDS;

	VideoType->formattype = FORMAT_VideoInfo;
	VideoType->cbFormat = sizeof(videoinfo);
	VideoType->pbFormat = (BYTE*)&videoinfo;
	VideoType->lSampleSize = 0;
}

static void
initDVInfo(CSampleGrabberCB *pCB)
{
	// Get pointers to these two fields to make subsequent code cleaner
	AM_MEDIA_TYPE *VideoType = &(pCB->VideoType);
	VIDEOINFOHEADER *dvinfo = &(pCB->dvinfo);

	memset(VideoType, 0, sizeof(AM_MEDIA_TYPE));
	memset(dvinfo, 0, sizeof(DVINFO));

	VideoType->formattype = FORMAT_DvInfo;
	VideoType->subtype = MEDIASUBTYPE_RGB24;
	VideoType->cbFormat = sizeof(DVINFO);
	VideoType->pbFormat = (BYTE*)dvinfo;
	VideoType->lSampleSize = 0;
}

static int
allocateFrameBuf()
{
  // ask for the connection media type so we know how big
  // it is, so we can write out bitmaps
  //
  HRESULT hr;
  AM_MEDIA_TYPE mt;
  VIDEOINFOHEADER * vih = NULL;

  if (pGrabber == NULL) {
    decoderLastError = DECODER_ERROR_UNINITIALIZED;
	MyError("allocateFrameBuf(): no grabber");
    return false;
  }

  hr = pGrabber->lpVtbl->GetConnectedMediaType(pGrabber, &mt);
  if (FAILED(hr)) {
    decoderLastError = DECODER_ERROR_MEDIATYPE;
		MyError("allocateFrameBuf(): can't get media type");
    return false;
  }
  
  if (frameBuf != NULL) {
    VirtualFree(frameBuf, m_CB.decodeWidth * m_CB.decodeHeight * 4, MEM_RELEASE);
    frameBuf = NULL;
  }

  vih = (VIDEOINFOHEADER*)mt.pbFormat;
  m_CB.decodeWidth  = vih->bmiHeader.biWidth;
  m_CB.decodeHeight = vih->bmiHeader.biHeight;
  /* FreeMediaType(&mt); */
  frameBuf = VirtualAlloc(NULL, m_CB.decodeWidth * m_CB.decodeHeight * 4, MEM_COMMIT, PAGE_READWRITE);
  decoderLastError = DECODER_ERROR_TRUE;
  return true;
}

static void
getDefaultCapDevice(IBaseFilter ** ppCap)
{
  HRESULT hr;

  ICreateDevEnum *pCreateDevEnum = NULL;
  IEnumMoniker *pEm = NULL;

  *ppCap = NULL;

  hr = CoCreateInstance(&CLSID_SystemDeviceEnum, NULL, CLSCTX_SERVER, &IID_ICreateDevEnum, &pCreateDevEnum);

  if (FAILED(hr)) {
    return;
  }

  pCreateDevEnum->lpVtbl->CreateClassEnumerator(pCreateDevEnum, &CLSID_VideoInputDeviceCategory, &pEm, 0);
  if (!pEm) {
    RELEASE(pCreateDevEnum);
    return;
  }

  pEm->lpVtbl->Reset(pEm);
 
  // go through and find first video capture device
  //
  while (1) {
    ULONG ulFetched = 0;
    IMoniker *pM = NULL;
    //IPropertyBag *pBag = NULL;
    //VARIANT var;
    hr = pEm->lpVtbl->Next(pEm, 1, &pM, &ulFetched);
    if (hr != S_OK)
      break;

    // get the property bag interface from the moniker
    //
    //hr = pM->lpVtbl->BindToStorage(pM, 0, 0, &IID_IPropertyBag, (void**) &pBag);
    //if (hr != S_OK)
    //continue;

    // ask for the english-readable name
    //
    //var.vt = VT_BSTR;
    //hr = pBag->lpVtbl->Read(pBag, L"FriendlyName", &var, NULL);
    //if (hr != S_OK)
    //continue;

    // ask for the actual filter
    //
    hr = pM->lpVtbl->BindToObject(pM, 0, 0, &IID_IBaseFilter, (void**) ppCap);
    if(*ppCap) {
      RELEASE(pM);
      break;
    }

    RELEASE(pM);
  }

  RELEASE(pEm);
  RELEASE(pCreateDevEnum);
  return;
}

static IIPDVDec*
getDVDec()
{
  HRESULT hr;

  IIPDVDec *pDVDec = NULL;
  IEnumFilters *pFilters = NULL;
  IBaseFilter *pFilter = NULL;
  ULONG n;

  if (pGraph == NULL) {
	  Error(TEXT("getDVDec(): no pGraph"));
    return NULL;
  }

  if(FAILED(pGraph->lpVtbl->EnumFilters(pGraph, &pFilters))) {
	  Error(TEXT("getDVDec(): failed to enumerate filters"));
    return NULL;
  }
  
  while(pFilters->lpVtbl->Next(pFilters, 1, &pFilter, &n) == S_OK) {
    hr = pFilter->lpVtbl->QueryInterface(pFilter, &IID_IIPDVDec, (void**)&pDVDec);
    if (!FAILED(hr)) {
      m_CB.cameraType = CAMERA_DV;
      RELEASE(pFilter);
      RELEASE(pFilters);
      return pDVDec;
    }
    RELEASE(pFilter);
	Error(TEXT("getDVDec(): attemopt"));  // I saw 4 of these
  }
  RELEASE(pFilters);
  m_CB.cameraType = CAMERA_OTHER;
  Error(TEXT("getDVDec(): failed to find match"));
  return NULL;
}


static int
initStillGraph(CSampleGrabberCB* pCB)
{
	HRESULT hr;
	int result;

	IBaseFilter *pGrabBase = NULL;

	int desiredWidth = pCB->decodeWidth;
	int desiredHeight = pCB->decodeHeight;

	pCB->pCaptureDevice = NULL;
	pCB->cameraType = CAMERA_NONE;
	pCB->pMediaControl = NULL;

	initializeVtbl(pCB);

	hr = CoInitialize(NULL); /* (NULL, COINIT_APARTMENTTHREADED); */
	if ((hr != S_OK)) {
		if (hr != S_FALSE) {  /* If it is S_FALSE, it looks like because of multiple call to CoInitialize */
			MyError("Could not initialize COM. (result: %x)", hr);
		}
		decoderLastError = DECODER_ERROR_COM;
	}

	// Create the Capture Graph Builder
	hr = CoCreateInstance(&CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC, &IID_ICaptureGraphBuilder2, (void**)&pBuild);
	if (SUCCEEDED(hr)) {
		// create a filter graph
		hr = CoCreateInstance(&CLSID_FilterGraph, NULL, CLSCTX_INPROC /*CLSCTX_SERVER*/, &IID_IGraphBuilder, &pGraph);
	}
	if (hr != S_OK) {
		MyError("Could not create filter graph. (result: %x)", hr);
		decoderLastError = DECODER_ERROR_COM;
		return false;
	}

	pBuild->lpVtbl->SetFiltergraph(pBuild, pGraph);

	getDefaultCapDevice(&(pCB->pCaptureDevice));
	if(!pCB->pCaptureDevice) {
		Error(	TEXT("No video capture device was detected on your system.\r\n\r\n")
				TEXT("This sample requires a functional video capture device, such\r\n")
				TEXT("as a USB web camera.") );
		cleanupLiveObjects(pCB);
		decoderLastError = DECODER_ERROR_NO_CAMERA;
		return false;
	}

	// add the capture filter to the graph
	//
	pGraph->lpVtbl->AddFilter(pGraph, pCB->pCaptureDevice, L"Cap");
	if(FAILED(hr)) {
		Error(TEXT("Could not put capture device in graph"));
		cleanupLiveObjects(pCB);
		decoderLastError = DECODER_ERROR_FILTERGRAPH;
		return false;
	}


	/* Try to set the camera dimensions to the requested size, otherwise
	   try the smaller standard sizes. */
	result = DecoderSetDecodeSize(desiredWidth, desiredHeight);
	if (!result && (640 < desiredWidth)) {
		result = DecoderSetDecodeSize(640, 480);
	}
	if (!result && (320 < desiredWidth)) {
		result = DecoderSetDecodeSize(320, 240);
	}
	if (!result && (160 < desiredWidth)) {
		result = DecoderSetDecodeSize(160, 120);
	}
	if (!result) {
		MyError("Could not set camera to requested dimensions (%dx%d),\n"
				"nor to standard settings (640x480, 320x240, 160x120)",
				desiredWidth, 
				desiredHeight);
	}


	// create a sample grabber
	//

	hr = CoCreateInstance(&CLSID_SampleGrabber, NULL, CLSCTX_SERVER, &IID_ISampleGrabber, &pGrabber);
	if (!pGrabber) {
		Error(TEXT("Could not create SampleGrabber (is qedit.dll registered?)"));
		cleanupLiveObjects(pCB);
		decoderLastError = DECODER_ERROR_COM;
		return false;
	}

	hr = pGrabber->lpVtbl->QueryInterface(pGrabber, &IID_IBaseFilter, &pGrabBase);
	if (!pGrabBase) {
		Error(TEXT("Could not get pGrabBase (is qedit.dll registered?)"));
		RELEASE(pGrabber);
		cleanupLiveObjects(pCB);
		decoderLastError = DECODER_ERROR_COM;
		return false;
	}

	// force it to connect to video, 24 bit
	//
	{
		pCB->cameraType = CAMERA_OTHER;
		if (pCB->cameraType == CAMERA_OTHER) {
			initVideoInfo(pCB);
		} else if (pCB->cameraType == CAMERA_DV) {
			initDVInfo(pCB);
		} else {
			RELEASE(pGrabber);
			RELEASE(pGrabBase);
			cleanupLiveObjects(pCB);
			decoderLastError = DECODER_ERROR_NO_CAMERA;
			return false;
		}

		hr = pGrabber->lpVtbl->SetMediaType(pGrabber, &(pCB->VideoType));
		if (FAILED(hr)) {
			Error( TEXT("Could not set media type"));
			RELEASE(pGrabber);
			RELEASE(pGrabBase);
			cleanupLiveObjects(pCB);
			decoderLastError = DECODER_ERROR_MEDIATYPE;
			return false;
		}
	}

	// add the grabber to the graph
	//
	hr = pGraph->lpVtbl->AddFilter(pGraph, pGrabBase,  L"Grabber");
	if (FAILED(hr)) {
		Error( TEXT("Could not put sample grabber in graph"));
		RELEASE(pGrabber);
		RELEASE(pGrabBase);
		cleanupLiveObjects(pCB);
		decoderLastError = DECODER_ERROR_FILTERGRAPH;
		return false;
	}

	// find the two pins and connect them
	//
	{
		IPin * pCapOut = GetOutPin(pCB->pCaptureDevice, 0);
		IPin * pGrabIn = GetInPin(pGrabBase, 0);
		hr = pGraph->lpVtbl->Connect(pGraph, pCapOut, pGrabIn);
		if (FAILED(hr)) {
			Error(TEXT("Could not connect capture pin #0 to grabber.\r\n")
				TEXT("Is the capture device being used by another application?"));
			RELEASE(pCapOut);
			RELEASE(pGrabIn);

			RELEASE(pGrabber);
			RELEASE(pGrabBase);

			cleanupLiveObjects(pCB);
			decoderLastError = DECODER_ERROR_FILTERGRAPH;
			return false;
		}

		RELEASE(pCapOut);
		RELEASE(pGrabIn);
	}

	// render the sample grabber output pin, so we get a preview window
	//
	{
		IPin * pGrabOut = GetOutPin(pGrabBase, 0);
		hr = pGraph->lpVtbl->Render(pGraph, pGrabOut);
		if (FAILED(hr)) {
			Error( TEXT("Could not render sample grabber output pin"));

			RELEASE(pGrabOut);

			RELEASE(pGrabber);
			RELEASE(pGrabBase);

			cleanupLiveObjects(pCB);
			decoderLastError = DECODER_ERROR_FILTERGRAPH;
			return false;
		}

		RELEASE(pGrabOut);
	}

	if (!allocateFrameBuf()) {
		return false;
	}

	// don't buffer the samples as they pass through
	//
	pGrabber->lpVtbl->SetBufferSamples(pGrabber, FALSE);

	// only grab one at a time, stop stream after
	// grabbing one sample
	//
	pGrabber->lpVtbl->SetOneShot(pGrabber, FALSE);

	// set the callback, so we can grab the one sample
	//
	pGrabber->lpVtbl->SetCallback(pGrabber, &(pCB->iGrabber), 1);

	// Add our graph to the running object table, which will allow
	// the GraphEdit application to "spy" on our graph
#ifdef REGISTER_FILTERGRAPH
	hr = AddGraphToRot(pGraph, &g_dwGraphRegister);
	if (FAILED(hr)) {
		Error(TEXT("Failed to register filter graph with ROT!"));
		g_dwGraphRegister = 0;
	}
#endif

	RELEASE(pGrabBase);
	decoderLastError = DECODER_ERROR_TRUE;
	return true;
}

EXPORT(int)
SetCallBackFunction(void (*f)(double sampleTime))
{
  frameCallBack = f;
  decoderLastError = DECODER_ERROR_TRUE;
  return true;
}

EXPORT(int) 
DecoderGetLastError()
{
  return decoderLastError;
}

EXPORT(int)
DestroyStillGraph()
{
	if (pGraph == NULL) {
		decoderLastError = DECODER_ERROR_UNINITIALIZED;
		return false;
	}
	action = DECODER_DESTROY;
	SetEvent(mutex);
	decoderLastError = DECODER_ERROR_TRUE;
	return true;
}


EXPORT(int)
DecoderPause()
{
  if (pGraph == NULL) {
    decoderLastError = DECODER_ERROR_UNINITIALIZED;
    return false;
  }
  action = DECODER_PAUSE;
  SetEvent(mutex);
  decoderLastError = DECODER_ERROR_TRUE;
  return true;
}

EXPORT(int)
DecoderStop()
{
  if (pGraph == NULL) {
    decoderLastError = DECODER_ERROR_UNINITIALIZED;
    return false;
  }
  action = DECODER_STOP;
  SetEvent(mutex);
  decoderLastError = DECODER_ERROR_TRUE;
  return true;
}

EXPORT(int)
DecoderRun()
{
  if (pGraph == NULL) {
    decoderLastError = DECODER_ERROR_UNINITIALIZED;
    return false;
  }
  action = DECODER_RUN;
  SetEvent(mutex);
  decoderLastError = DECODER_ERROR_TRUE;
  return true;
}

EXPORT(int)
InitializeDecoder(int width, int height)
{
  SYSTEM_INFO sysInfo;
  DWORD pageSize;
  DWORD id = 0;

  if (hThread) {
    decoderLastError = DECODER_ERROR_THREAD;
    return false;
  }

  /* determine page boundaries */
  GetSystemInfo(&sysInfo);
  pageSize = sysInfo.dwPageSize;

  /* Ugly: stash the desired video dimensions into the not-yet-initialized m_CB */
  m_CB.decodeWidth = width;
  m_CB.decodeHeight = height;

  mutex = CreateEvent(NULL, FALSE, FALSE, NULL);

  hThread = 
    CreateThread(NULL,			   /* No security descriptor */
		 //pageSize,                 /* default stack size     */
		 128*1024,				   /* big.  really big */
		 (LPTHREAD_START_ROUTINE)decodeProcess, /* what to do */
		 (LPVOID) &m_CB,      /* parameter for thread   */
		 //CREATE_SUSPENDED,  /* creation parameter -- create suspended so we can check the return value */
		 CREATE_SUSPENDED | STACK_SIZE_PARAM_IS_A_RESERVATION,
		 &id);              /* return value for thread id */

  if(!hThread) {
    /* CreateThread failed */
    decoderLastError = DECODER_ERROR_THREAD;
    return false;
  }

  if(!SetThreadPriority(hThread, THREAD_PRIORITY_HIGHEST)) {
    decoderLastError = DECODER_ERROR_THREAD;
    return false;
  }

  decoderLastError = DECODER_ERROR_TRUE;

  if (ResumeThread(hThread) == (DWORD)-1) {	
    hThread = NULL;
    CloseHandle(mutex);
    mutex = NULL;
    decoderLastError = DECODER_ERROR_THREAD;
    return false;
  }

  return true;
}


EXPORT(int)
SetRenderWindow(void* hWnd)
{
  if (hWnd == NULL) {
    stWindow = NULL;
  } else {
    stWindow = *(HWND*)hWnd;
  }
  decoderLastError = DECODER_ERROR_TRUE;
  return true;
}


EXPORT(int)
SetRenderRect(int left, int top, int width, int height)
{
  renderX = left;
  renderY = top;
  renderW = width;
  renderH = height;
  decoderLastError = DECODER_ERROR_TRUE;
  return true;
}

EXPORT(int)
ShowRenderWindow(int show)
{
  // find the video window and stuff it in our window
  //
  HRESULT hr;
  IVideoWindow *pWindow = NULL;

  if (pGraph == NULL) {
    decoderLastError = DECODER_ERROR_UNINITIALIZED;
    return false;
  }

  if (stWindow == NULL) {
    decoderLastError = DECODER_ERROR_UNINITIALIZED;
    return false;
  }

  hr = pGraph->lpVtbl->QueryInterface(pGraph, &IID_IVideoWindow, &pWindow);
  if (FAILED(hr)) {
    decoderLastError = DECODER_ERROR_INTERFACE;
    return false;
  }

  pWindow->lpVtbl->put_WindowStyle(pWindow, WS_CHILD | WS_CLIPSIBLINGS);
  
  if (show) {
    pWindow->lpVtbl->put_AutoShow(pWindow, OATRUE);
    pWindow->lpVtbl->put_Owner(pWindow, (OAHWND)stWindow);
    pWindow->lpVtbl->put_Visible(pWindow, OATRUE);
  } else {
    pWindow->lpVtbl->put_AutoShow(pWindow, OAFALSE);
    pWindow->lpVtbl->put_Visible(pWindow, OAFALSE);
    pWindow->lpVtbl->put_Owner(pWindow, (OAHWND)NULL);
  }

  pWindow->lpVtbl->put_Left(pWindow, renderX);
  pWindow->lpVtbl->put_Top(pWindow, renderY);
  pWindow->lpVtbl->put_Width(pWindow, renderW);
  pWindow->lpVtbl->put_Height(pWindow, renderH);

  putOwnerCalled = true;
  RELEASE(pWindow);

  decoderLastError = DECODER_ERROR_TRUE;
  return true;
}


EXPORT(unsigned int)
DecoderDecodeSize()
{
  decoderLastError = DECODER_ERROR_TRUE;
  return (m_CB.decodeWidth << 16) | (m_CB.decodeHeight & 0xFFFF);
}


int DecoderSetDecodeSize_DV(int w, int h);
int DecoderSetDecodeSize_OTHER(int w, int h);

EXPORT(int)
DecoderSetDecodeSize(int w, int h)
{
	HRESULT hr;

	int ret;
	if (m_CB.cameraType == CAMERA_DV) {
		ret = DecoderSetDecodeSize_DV(w,h);
	}
	else { // must be CAMERA_OTHER
		ret = DecoderSetDecodeSize_OTHER(w,h);
	}

	if (ret) {
		if (pGrabber == NULL) {
			/* Postpone allocating the frame-buffer until later */
			return true;
		}
		return allocateFrameBuf();
	}
	decoderLastError = DECODER_ERROR_CAMERATYPE;

	return ret;
}


int
DecoderSetDecodeSize_DV(int w, int h)
{
  HRESULT hr;

  IIPDVDec *pDVDec = getDVDec();

  int ret = false;
  
  if (pDVDec == NULL) {
    decoderLastError = DECODER_ERROR_CAMERATYPE;
	Error(TEXT("Failed reeeely quick when trying to set camera size"));
    return false;
  }

  if (w == 720 && h == 480) {
    hr = pDVDec->lpVtbl->put_IPDisplay(pDVDec, DVDECODERRESOLUTION_720x480);
    ret = !FAILED(hr);
  }

  if (w == 360 && h == 240) {
    hr = pDVDec->lpVtbl->put_IPDisplay(pDVDec, DVDECODERRESOLUTION_360x240);
    ret = !FAILED(hr);
  }
    
  if (w == 180 && h == 120) {
    hr = pDVDec->lpVtbl->put_IPDisplay(pDVDec, DVDECODERRESOLUTION_180x120);
    ret = !FAILED(hr);
  }
    
  if (w == 88 && h == 60) {
    hr = pDVDec->lpVtbl->put_IPDisplay(pDVDec, DVDECODERRESOLUTION_88x60);
    ret = !FAILED(hr);
  }

  RELEASE(pDVDec);

  return ret;
}


int
DecoderSetDecodeSize_OTHER(int w, int h)
{
	IAMStreamConfig *pConfig = NULL;
	HRESULT hr;
	int iCount = 0, iSize = 0;

	// Don't listen to the MSDN docs for "Configure the Video Output Format",
	// it is lying to you (they pass completely bogus arguments to FindInterface)
	hr = pBuild->lpVtbl->FindInterface(	pBuild,
										NULL,
										&MEDIATYPE_Video,
										m_CB.pCaptureDevice,	// Pointer to the capture filter.
										&IID_IAMStreamConfig, 
										(void**)&pConfig);

	if (pConfig == NULL) {
		MyError("Failed to find IAMStreamConfig");
		return false;
	}

	hr = pConfig->lpVtbl->GetNumberOfCapabilities(pConfig, &iCount, &iSize);

	MyError("Attempting to set size to %dx%d; found %d capabilities", w, h, iCount);

	// Check the size to make sure we pass in the correct structure.
	if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS))
	{
		// Use the video capabilities structure.
		int iFormat;
		for (iFormat = 0; iFormat < iCount; iFormat++)
		{
			VIDEO_STREAM_CONFIG_CAPS scc;
			AM_MEDIA_TYPE *pMediaType;
			hr = pConfig->lpVtbl->GetStreamCaps(pConfig, iFormat, &pMediaType, (BYTE*)&scc);
			if (SUCCEEDED(hr))
			{
				/* Examine the format, and possibly use it. */
				/* There may be several formats with the same output size.  Just pick the first one. */
				if ((w == scc.MaxOutputSize.cx) && (h == scc.MaxOutputSize.cy)) {
					hr = pConfig->lpVtbl->SetFormat(pConfig, pMediaType);
					if (SUCCEEDED(hr)) {
						m_CB.decodeWidth = scc.MaxOutputSize.cx;
						m_CB.decodeHeight = scc.MaxOutputSize.cy;
						// Delete the media type when you are done.
						DeleteMediaType(pMediaType);
						return true;
					}
					MyError("Failed to set format with dimensions %dx%d (result: %x)\n\tE_OUTOFMEMORY:%x \n\tE_POINTER:%x \n\tVFW_E_INVALIDMEDIATYPE:%x \n\tVFW_E_NOT_CONNECTED:%x \n\tVFW_E_NOT_STOPPED:%x \n\tVFW_E_WRONG_STATE:%x", 
							scc.MaxOutputSize.cx, 
							scc.MaxOutputSize.cy, 
							hr,
							E_OUTOFMEMORY,
							E_POINTER,
							VFW_E_INVALIDMEDIATYPE,
							VFW_E_NOT_CONNECTED,
							VFW_E_NOT_STOPPED,
							VFW_E_WRONG_STATE);
				}
				// Delete the media type when you are done.
				DeleteMediaType(pMediaType);
			}
		}
		return 0;
	}
}


EXPORT(int)
DecoderCaptureFrame()
{
  if (pGraph == NULL) {
    decoderLastError = DECODER_ERROR_UNINITIALIZED;
    return false;
  }

  captureNextFrame = true;
  decoderLastError = DECODER_ERROR_TRUE;
  return true;
}

static void
copyFromTo(unsigned char *from, unsigned char *bits, int width, int height, int depth)
{
  memcpy(bits, from, width * height * (depth / 8));
}

EXPORT(unsigned char*)
DecoderGetLastFrame()
{
  decoderLastError = DECODER_ERROR_TRUE;
  return &frameBuf[0];
}

EXPORT(int)
DecoderCopyLastFrameInto(unsigned char *bits, int width, int height, int depth)
{
  copyFromTo(frameBuf, bits, width, height, depth);
  decoderLastError = DECODER_ERROR_TRUE;
  return true;
}
     

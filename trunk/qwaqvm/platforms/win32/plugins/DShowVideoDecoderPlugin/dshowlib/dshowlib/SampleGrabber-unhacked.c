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

static int initStillGraph();

typedef struct _CSampleGrabberCB {
  ISampleGrabberCB iGrabber;
  int ref;
} CSampleGrabberCB;

static ISampleGrabberCBVtbl m_cb_vtbl;
static CSampleGrabberCB m_CB;

static DWORD g_dwGraphRegister = 0;  // For running object table

static IGraphBuilder *pGraph = NULL;
static ISampleGrabber *pGrabber = NULL;

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

static int decodeWidth;
static int decodeHeight;
static void (*frameCallBack)(double sampleTime) = NULL;

static int cameraType = CAMERA_NONE;

static int action = DECODER_NOTHING;

int decoderLastError = DECODER_ERROR_FALSE;

#ifdef REPORT_ERROR_VIA_DIALOGS
void
Error(TCHAR * pText)
{
  MessageBox(NULL, pText, TEXT("YOO-HOO!!! Error!"), MB_OK | MB_TASKMODAL | MB_SETFOREGROUND);
}
#endif

static DWORD WINAPI
decodeProcess(LPVOID ignored);

static HRESULT
AddGraphToRot(IGraphBuilder *pUnkGraph, DWORD *pdwRegister) 
{
    IMoniker * pMoniker;
    IRunningObjectTable *pROT;
    WCHAR wsz[128];
    HRESULT hr;

    if (FAILED(GetRunningObjectTable(0, &pROT)))
        return E_FAIL;

    wsprintfW(wsz, L"FilterGraph %08x pid %08x", (DWORD_PTR)pUnkGraph, 
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
  cb->iGrabber.lpVtbl->BufferCB = CSampleGrabberBufferCB;
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
cleanupLiveObjects()
{
  IVideoWindow *pWindow = NULL;
  HRESULT hr;

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
 
static void
destroyStillGraph()
{
  cleanupLiveObjects();

  stWindow = NULL;
  frameCallBack = NULL;

  if (frameBuf != NULL) {
    VirtualFree(frameBuf, decodeWidth * decodeHeight * 4, MEM_RELEASE);
    frameBuf = NULL;

    decodeWidth  = 0;
    decodeHeight = 0;
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
decoderRun()
{
  IMediaControl *pControl = NULL;
  HRESULT hr;

  if (pGraph == NULL || !putOwnerCalled) {
    decoderLastError = DECODER_ERROR_UNINITIALIZED;
    return false;
  }

  hr = pGraph->lpVtbl->QueryInterface(pGraph, &IID_IMediaControl, &pControl);
  if (FAILED(hr)) {
    decoderLastError = DECODER_ERROR_INTERFACE;
    return false;
  }
  
  pControl->lpVtbl->Run(pControl);
  if (FAILED(hr)) {
    RELEASE(pControl);
    decoderLastError = DECODER_ERROR_CONTROL;
    return false;
  }

  RELEASE(pControl);

  decoderLastError = DECODER_ERROR_TRUE;
  return true;
}

static int
decoderStop()
{
  IMediaControl *pControl = NULL;
  HRESULT hr;

  if (pGraph == NULL || !putOwnerCalled) {
    decoderLastError = DECODER_ERROR_UNINITIALIZED;
    return false;
  }

  hr = pGraph->lpVtbl->QueryInterface(pGraph, &IID_IMediaControl, &pControl);
  if (FAILED(hr)) {
    decoderLastError = DECODER_ERROR_INTERFACE;
    return false;
  }
  
  pControl->lpVtbl->Stop(pControl);
  if (FAILED(hr)) {
    RELEASE(pControl);
    decoderLastError = DECODER_ERROR_CONTROL;
    return false;
  }

  RELEASE(pControl);
  decoderLastError = DECODER_ERROR_TRUE;
  return true;
}
  

static int
decoderPause()
{
  IMediaControl *pControl = NULL;
  HRESULT hr;

  if (pGraph == NULL || !putOwnerCalled) {
    decoderLastError = DECODER_ERROR_UNINITIALIZED;
    return false;
  }

  hr = pGraph->lpVtbl->QueryInterface(pGraph, &IID_IMediaControl, &pControl);
  if (FAILED(hr)) {
    decoderLastError = DECODER_ERROR_INTERFACE;
    return false;
  }
  
  pControl->lpVtbl->Pause(pControl);
  if (FAILED(hr)) {
    RELEASE(pControl);
    decoderLastError = DECODER_ERROR_CONTROL;
    return false;
  }

  RELEASE(pControl);
  decoderLastError = DECODER_ERROR_TRUE;
  return true;
}

static DWORD WINAPI
decodeProcess(LPVOID ignored)
{
  if (initStillGraph() == false) {
    destroyStillGraph();    
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
      decoderRun();
      break;
    case DECODER_STOP:
      action = DECODER_NOTHING;
      decoderStop();
      break;
    case DECODER_PAUSE:
      action = DECODER_NOTHING;
      decoderPause();
      break;
    case DECODER_DESTROY:
      action = DECODER_NOTHING;
      destroyStillGraph();
      return 0;
    }
  }
}

static void
initVideoInfo(AM_MEDIA_TYPE *VideoType, VIDEOINFOHEADER *videoinfo)
{
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
initDVInfo(AM_MEDIA_TYPE *VideoType, DVINFO *dvinfo)
{
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
    return false;
  }

  hr = pGrabber->lpVtbl->GetConnectedMediaType(pGrabber, &mt);
  if (FAILED(hr)) {
    decoderLastError = DECODER_ERROR_MEDIATYPE;
    return false;
  }
  
  if (frameBuf != NULL) {
    VirtualFree(frameBuf, decodeWidth * decodeHeight * 4, MEM_RELEASE);
    frameBuf = NULL;
  }

  vih = (VIDEOINFOHEADER*)mt.pbFormat;
  decodeWidth  = vih->bmiHeader.biWidth;
  decodeHeight = vih->bmiHeader.biHeight;
  /* FreeMediaType(&mt); */
  frameBuf = VirtualAlloc(NULL, decodeWidth * decodeHeight * 4, MEM_COMMIT, PAGE_READWRITE);
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
    return NULL;
  }

  if(FAILED(pGraph->lpVtbl->EnumFilters(pGraph, &pFilters))) {
    return NULL;
  }
  
  while(pFilters->lpVtbl->Next(pFilters, 1, &pFilter, &n) == S_OK) {
    hr = pFilter->lpVtbl->QueryInterface(pFilter, &IID_IIPDVDec, (void**)&pDVDec);
    if (!FAILED(hr)) {
      cameraType = CAMERA_DV;
      RELEASE(pFilter);
      RELEASE(pFilters);
      return pDVDec;
    }
    RELEASE(pFilter);
  }
  RELEASE(pFilters);
  cameraType = CAMERA_OTHER;
  return NULL;
}


static int
initStillGraph()
{
  HRESULT hr;

  IBaseFilter *pCap = NULL;
  IBaseFilter *pGrabBase = NULL;

  initializeVtbl(&m_CB);
  
  hr = CoInitialize(NULL); /* (NULL, COINIT_APARTMENTTHREADED); */
  if ((hr != S_OK)) {
    if (hr != S_FALSE) {  /* If it is S_FALSE, it looks like because of multiple call to CoInitialize */
#ifdef REPORT_ERROR_VIA_DIALOGS
      char buf[256];
      TCHAR buf2[256];
      int i;
      sprintf(buf, "Could not initialize COM. %x", hr);
      i = 0;
      while (buf2[i] = buf[i]) {
	i++;
      }
      Error(buf2);
#endif
    }
    decoderLastError = DECODER_ERROR_COM;
  }

  // create a filter graph
  //
  hr = CoCreateInstance(&CLSID_FilterGraph, NULL, CLSCTX_INPROC /*CLSCTX_SERVER*/, &IID_IGraphBuilder, &pGraph);
  if (hr != S_OK) {
#ifdef REPORT_ERROR_VIA_DIALOGS
    char buf[256];
    TCHAR buf2[256];
    int i;
    sprintf(buf, "Could not create filter graph. %x", hr);
    i = 0;
    while (buf2[i] = buf[i]) {
      i++;
    }
    Error(buf2);
#endif
    decoderLastError = DECODER_ERROR_COM;
    return false;
  }

  getDefaultCapDevice(&pCap);
  if(!pCap) {
#ifdef REPORT_ERROR_VIA_DIALOGS
    Error( TEXT("No video capture device was detected on your system.\r\n\r\n")
	   TEXT("This sample requires a functional video capture device, such\r\n")
	   TEXT("as a USB web camera.") );
#endif
    cleanupLiveObjects();
    decoderLastError = DECODER_ERROR_NO_CAMERA;
    return false;
  }

  // add the capture filter to the graph
  //
  pGraph->lpVtbl->AddFilter(pGraph, pCap, L"Cap");
  if(FAILED(hr)) {
#ifdef REPORT_ERROR_VIA_DIALOGS
    Error(TEXT("Could not put capture device in graph"));
#endif
    RELEASE(pCap);
    cleanupLiveObjects();
    decoderLastError = DECODER_ERROR_FILTERGRAPH;
    return false;
  }

  // create a sample grabber
  //
  
  hr = CoCreateInstance(&CLSID_SampleGrabber, NULL, CLSCTX_SERVER, &IID_ISampleGrabber, &pGrabber);
  if (!pGrabber) {
#ifdef REPORT_ERROR_VIA_DIALOGS
    Error(TEXT("Could not create SampleGrabber (is qedit.dll registered?)"));
#endif
    RELEASE(pCap);
    cleanupLiveObjects();
    decoderLastError = DECODER_ERROR_COM;
    return false;
  }

  hr = pGrabber->lpVtbl->QueryInterface(pGrabber, &IID_IBaseFilter, &pGrabBase);
  if (!pGrabBase) {
#ifdef REPORT_ERROR_VIA_DIALOGS
    Error(TEXT("Could not get pGrabBase (is qedit.dll registered?)"));
#endif
    RELEASE(pCap);
    RELEASE(pGrabber);
    cleanupLiveObjects();
    decoderLastError = DECODER_ERROR_COM;
    return false;
  }

  // force it to connect to video, 24 bit
  //
  {
    AM_MEDIA_TYPE VideoType;
    DVINFO dvinfo;
    VIDEOINFOHEADER videoinfo;
    
    cameraType = CAMERA_OTHER;
    if (cameraType == CAMERA_OTHER) {
      initVideoInfo(&VideoType, &videoinfo);
    } else if (cameraType == CAMERA_DV) {
      initDVInfo(&VideoType, &dvinfo);
    } else {
      RELEASE(pCap);
      RELEASE(pGrabber);
      RELEASE(pGrabBase);
      cleanupLiveObjects();
      decoderLastError = DECODER_ERROR_NO_CAMERA;
      return false;
    }
    
    hr = pGrabber->lpVtbl->SetMediaType(pGrabber, &VideoType);
    if (FAILED(hr)) {
#ifdef REPORT_ERROR_VIA_DIALOGS
      Error( TEXT("Could not set media type"));
#endif
      RELEASE(pCap);
      RELEASE(pGrabber);
      RELEASE(pGrabBase);
      cleanupLiveObjects();
      decoderLastError = DECODER_ERROR_MEDIATYPE;
      return false;
    }
  }

  // add the grabber to the graph
  //
  hr = pGraph->lpVtbl->AddFilter(pGraph, pGrabBase,  L"Grabber");
  if (FAILED(hr)) {
#ifdef REPORT_ERROR_VIA_DIALOGS
    Error( TEXT("Could not put sample grabber in graph"));
#endif
    RELEASE(pCap);
    RELEASE(pGrabber);
    RELEASE(pGrabBase);
    cleanupLiveObjects();
    decoderLastError = DECODER_ERROR_FILTERGRAPH;
    return false;
  }

  // find the two pins and connect them
  //
  {
    IPin * pCapOut = GetOutPin(pCap, 0);
    IPin * pGrabIn = GetInPin(pGrabBase, 0);
    hr = pGraph->lpVtbl->Connect(pGraph, pCapOut, pGrabIn);
    if (FAILED(hr)) {
#ifdef REPORT_ERROR_VIA_DIALOGS
      Error(TEXT("Could not connect capture pin #0 to grabber.\r\n")
	 TEXT("Is the capture device being used by another application?"));
#endif
      RELEASE(pCapOut);
      RELEASE(pGrabIn);

      RELEASE(pCap);
      RELEASE(pGrabber);
      RELEASE(pGrabBase);

      cleanupLiveObjects();
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
#ifdef REPORT_ERROR_VIA_DIALOGS
      Error( TEXT("Could not render sample grabber output pin"));
#endif
      
      RELEASE(pGrabOut);

      RELEASE(pCap);
      RELEASE(pGrabber);
      RELEASE(pGrabBase);

      cleanupLiveObjects();
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
  pGrabber->lpVtbl->SetCallback(pGrabber, &(m_CB.iGrabber), 1);
  
  // Add our graph to the running object table, which will allow
  // the GraphEdit application to "spy" on our graph
#ifdef REGISTER_FILTERGRAPH
  hr = AddGraphToRot(pGraph, &g_dwGraphRegister);
  if (FAILED(hr)) {
# ifdef REPORT_ERROR_VIA_DIALOGS
    Error(TEXT("Failed to register filter graph with ROT!"));
# endif
    g_dwGraphRegister = 0;
  }
#endif

  RELEASE(pGrabBase);
  RELEASE(pCap);
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
InitializeDecoder()
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

  mutex = CreateEvent(NULL, FALSE, FALSE, NULL);

  hThread = 
    CreateThread(NULL,			   /* No security descriptor */
		 //pageSize,                 /* default stack size     */
		 128*1024,				   /* big.  really big */
		 (LPTHREAD_START_ROUTINE)decodeProcess, /* what to do */
		 (LPVOID) NULL,      /* parameter for thread   */
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
  return (decodeWidth << 16) | (decodeHeight & 0xFFFF);
}

EXPORT(int)
DecoderSetDecodeSize(int w, int h)
{
  HRESULT hr;

  IIPDVDec *pDVDec = getDVDec();

  int ret = false;
  
  if (pDVDec == NULL) {
    decoderLastError = DECODER_ERROR_CAMERATYPE;
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

  if (ret) {
    return allocateFrameBuf();
  }
  decoderLastError = DECODER_ERROR_CAMERATYPE;
  return ret;
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
     

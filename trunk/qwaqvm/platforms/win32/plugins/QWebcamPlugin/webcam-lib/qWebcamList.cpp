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

#include "qWebcamList.h"
#include "qDShowUtil.h"
#include "qLogger.hpp"
#include "qException.h"
#include <assert.h>

// For converting BSTR to std::string
#include <comdef.h>

using namespace Qwaq;

namespace Qwaq {
///////////////////////////////////////////
// Private table of known devices.

typedef struct qCamDeviceCache {
	char* camName;
	char* camUID;
	int	  isBusy;
} qCamDeviceCache_struct;

static qCamDeviceCache ** gKnownDevices = NULL;
static int gNumKnownDevices = 0;

// Toss all the saved stuff from the previous gathered device list.
static void freeDeviceList ()
{
	int i = 0;
	if (! gKnownDevices) {
		return;
	}
	qCamDeviceCache * cam;
	for (i = 0; i < gNumKnownDevices; i++) {
		cam = gKnownDevices[i];
		if (cam) {
			free(cam->camName);
			free(cam->camUID);
		}}
	free (gKnownDevices);
	gKnownDevices = NULL;
	gNumKnownDevices = 0;
}

// Private - add a capture-device to the current knownDevices array.
static bool addKnownDevice( IMoniker *pMoniker )
{
	HRESULT hr;
	VARIANT name;
	VARIANT uid;
	IPropertyBag* pBag;

	hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pBag);
	if (hr != S_OK) {
		qerr << "     - failed to get property-bag for device: " << hr << endl;
		pMoniker->Release();
		return false; 
	}

	name.vt = VT_BSTR;
	hr = pBag->Read(L"FriendlyName", &name, NULL);
	if (hr != S_OK) {
		qerr << "     - No friendly name for device: " << hr << endl;
		pMoniker->Release();
		pBag->Release(); 
		return false;
	}
	uid.vt = VT_BSTR;
	hr = pBag->Read(L"DevicePath", &uid, NULL);
	if (hr != S_OK) {
		qerr << "     - No device path for device " << _bstr_t(name.bstrVal) << ": " << hr << endl;
		pMoniker->Release();
		pBag->Release(); 
		return false;
	}

	// qerr << "     - Found device: " << _bstr_t(name.bstrVal) << endl;
	pMoniker->Release();
	pBag->Release();

	if (gKnownDevices) {
		gKnownDevices = (qCamDeviceCache**) realloc(gKnownDevices, (sizeof(qCamDeviceCache*) * (++gNumKnownDevices)));
	} else {
		gKnownDevices = (qCamDeviceCache**) malloc( sizeof(qCamDeviceCache*));
		gNumKnownDevices = 1;
	}

	int i = gNumKnownDevices-1;
	gKnownDevices[i] = (qCamDeviceCache*) malloc(sizeof(qCamDeviceCache));
	gKnownDevices[i]->camName = strdup( (char*const) _bstr_t(name.bstrVal) );
	gKnownDevices[i]->camUID = strdup( (char*const) _bstr_t(uid.bstrVal) );
	gKnownDevices[i]->isBusy = 0;

	SysFreeString(name.bstrVal);
	SysFreeString(uid.bstrVal);

	return true;
}


static IEnumMoniker *gpEnumerator = NULL;
static ICreateDevEnum *gpCreateDevEnum = NULL;

// Create a video-input device enumerator;
// we reuse the enumerator for the life of the process.

IEnumMoniker *pqDeviceEnumerator() {

	if (gpEnumerator) {
		gpEnumerator->Release();
		gpEnumerator = NULL;
	}

	HRESULT hr;
	if (! gpCreateDevEnum) {
		// Create a factory that creates device enumerators.
		hr = CoCreateInstance(	
							CLSID_SystemDeviceEnum, 
							NULL, 
							CLSCTX_SERVER, // XXXXX: this is different than in initFilterGraph()... why?
							IID_ICreateDevEnum, 
							(void**)& gpCreateDevEnum );
		if (FAILED(hr)) {
			qerr << endl << "ListCameras: failed to create enumerator factory: " << hr;
			throw QE_COM_ERROR;
		}}

	// Create a video-input device enumerator
	hr = gpCreateDevEnum->CreateClassEnumerator(
							CLSID_VideoInputDeviceCategory, 
							&gpEnumerator, 
							0);
	if (!gpEnumerator) {
		if (S_FALSE == hr) {
			qerr << endl << "ListCameras: no cameras available";
			return NULL;
		}
		else {
			qerr << endl << "ListCameras: failed to create enumerator: " << hr;
			throw QE_COM_ERROR;
		}
	}
	gpEnumerator->Reset();
	return gpEnumerator;
}	

// Main device-enumeration procedure.  Returns number of web cams found;
// this number of qCamDeviceCache entries appear in the gKnownDevices array.

int qPrivateGetNumCamDevices (void) 
{
	HRESULT hr;
	ULONG fetched;
	IEnumMoniker *pEnumerator = NULL;

	freeDeviceList();

	// Get a video-input device enumerator
	pEnumerator = pqDeviceEnumerator();
	if (!pEnumerator) {
		return 0;
	}

	// Iterate over the available input devices
	IMoniker* pMoniker;
	while (S_OK == (hr = pEnumerator->Next(1, &pMoniker, &fetched))) {
		addKnownDevice(pMoniker);
	}
	// NOISY 
	qLog() << endl << "Listed " << gNumKnownDevices << " cams.";
	return gNumKnownDevices;
}

// The user-friendly name for the specified device.
char* qPrivateGetCamDeviceName(int devIndex) {
	if (devIndex >= gNumKnownDevices || devIndex < 0) {
			qLog() << endl << "getCamDeviceName: Invalid index " << devIndex;
		return NULL;
	}
	return gKnownDevices[devIndex]->camName;
}

// The persistent identifier for the specified device.
char* qPrivateGetCamDeviceUID(int devIndex) {
	if (devIndex >= gNumKnownDevices || devIndex < 0) {
		qLog() << endl << "getCamDeviceUID: Invalid index " << devIndex;
		return NULL;
	}
	return gKnownDevices[devIndex]->camUID;
}

// If we can be sure the specified device is in use elsewhere, return 1, else 0.
int qPrivateGetCamDeviceIsBusy (int devIndex) {
	if (devIndex >= gNumKnownDevices || devIndex < 0) {
		qLog() << endl << "getCamDeviceIsBusy: Invalid index " << devIndex;
		return 0;
	}
	return gKnownDevices[devIndex]->isBusy;
}

// Cleanup for releasing the plugin.

void qPrivateStopLists (void) {
	if (gpEnumerator) {
		gpEnumerator->Release();
		gpEnumerator = NULL;
	}
	if (gpCreateDevEnum) {
		gpCreateDevEnum->Release();
		gpCreateDevEnum = NULL;
	}
	freeDeviceList();
}


} // namespace

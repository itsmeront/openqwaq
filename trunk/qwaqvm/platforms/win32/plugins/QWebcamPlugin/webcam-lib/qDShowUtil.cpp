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

#include "qDShowUtil.h"
#include "qLogger.hpp"
#include "qException.h"

namespace Qwaq
{

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
        mt->pUnk->Release();
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



IPin *
GetInPin(IBaseFilter * pFilter, int Num)
{
	IPin *pPin = NULL;
	HRESULT hr = GetPin(pFilter, PINDIR_INPUT, Num, &pPin);
	if (FAILED(hr)) {
		qerr << endl << "failed to get input pin: " << hr;
		throw QE_DSHOW_ERROR;
	}
	return pPin;
}

IPin *
GetOutPin( IBaseFilter * pFilter, int Num )
{
	IPin *pPin = NULL;
	HRESULT hr = GetPin(pFilter, PINDIR_OUTPUT, Num, &pPin);
	if (FAILED(hr)) {
		qerr << endl << "failed to get output pin: " << hr;
		throw QE_DSHOW_ERROR;
	}
	return pPin;
}

HRESULT
GetPin(IBaseFilter *pFilter, PIN_DIRECTION dirrequired, int iNum, IPin **ppPin)
{
  HRESULT hr;
  IEnumPins * pEnum = NULL;

  ULONG ulFound;
  IPin *pPin;
  
  *ppPin = NULL;
  
  hr = pFilter->EnumPins(&pEnum);
  if (FAILED(hr)) 
    return hr;

  hr = E_FAIL;

  while (S_OK == pEnum->Next(1, &pPin, &ulFound)) {
    PIN_DIRECTION pindir = (PIN_DIRECTION)3;
    pPin->QueryDirection(&pindir);
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
    
    pPin->Release();
  }

  if (pEnum != NULL) {
    pEnum->Release();
    pEnum = NULL;
  }

  return hr;
}


} // namespace Qwaq

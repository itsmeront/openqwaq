/* SampleGrabber.h

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

/* pluggable primitive support */
#if defined(_MSC_VER) || defined(__MINGW32__)
#  undef EXPORT
#  define EXPORT(returnType) __declspec( dllexport ) returnType __cdecl 
#endif 

#ifndef RELEASE
#  define RELEASE(lp) if(lp) { lp->Release(); lp = NULL; }
#endif

#ifndef offsetof
#  define offsetof(s,m)  (size_t)&(((s *)0)->m)
#endif

#ifdef __cplusplus
extern "C" {
#endif

EXPORT(int) InitializeDecoder(int width, int height);
EXPORT(int) DestroyStillGraph();

EXPORT(int) DecoderPause();
EXPORT(int) DecoderStop();
EXPORT(int) DecoderRun();

EXPORT(int) SetCallBackFunction(void (*f)(double sampleTime));

EXPORT(int) SetRenderRect(int left, int top, int width, int height);
EXPORT(int) ShowRenderWindow(int show);
EXPORT(int) SetRenderWindow(void* hWnd);
EXPORT(int) DecoderCaptureFrame();
EXPORT(int) DecoderCopyLastFrameInto(unsigned char *bits, int width, int height, int depth);
EXPORT(unsigned int) DecoderDecodeSize();
EXPORT(unsigned char*) DecoderGetLastFrame();
EXPORT(int) DecoderSetDecodeSize(int w, int h);
EXPORT(int) DecoderGetLastError();

#ifdef __cplusplus
}
#endif

#ifndef __cplusplus
#define IMPL(class, member, pointer) \
    (&((class *)0)->member == pointer, ((class *) (((long) pointer) - offsetof(class, member))))

#define true 1
#define false 0
#endif

enum {CAMERA_NONE, CAMERA_OTHER, CAMERA_DV};
enum {DECODER_NOTHING, DECODER_RUN, DECODER_PAUSE, DECODER_STOP, DECODER_DESTROY};
enum {DECODER_ERROR_FALSE, DECODER_ERROR_TRUE,
      DECODER_ERROR_NO_CAMERA,
      DECODER_ERROR_COM,
      DECODER_ERROR_FILTERGRAPH,
      DECODER_ERROR_THREAD,
      DECODER_ERROR_UNINITIALIZED,
      DECODER_ERROR_INTERFACE,
      DECODER_ERROR_CONTROL,
      DECODER_ERROR_MEDIATYPE,
      DECODER_ERROR_CAMERATYPE,
      DECODER_ERROR_UNKNOWN};

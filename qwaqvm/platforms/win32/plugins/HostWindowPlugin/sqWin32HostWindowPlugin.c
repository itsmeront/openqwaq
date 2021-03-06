/****************************************************************************
*   PROJECT: Squeak port for Win32
*   FILE:    sqWin32HostWindowPlugin.c
*   CONTENT: hostwindow support
*
*   AUTHORS:  Andreas Raab (ar) , Bernd Eckardt (be)
*   ADDRESS: Magdeburg, Germany
*   EMAIL:   andreas.raab@gmx.de, bernd.eckardt@impara.de
*****************************************************************************/

#include <windows.h>

#if defined(__MINGW32_VERSION) && (__MINGW32_MAJOR_VERSION < 3)
/** Kludge to get multimonitor API's to compile in the mingw/directx7 mix. **/
/** Not needed in cygwin **/
/* Some foolishness to get dx7stubs and mingw/windows.h to be happy about mutlimon */
# undef SM_CMONITORS
# define HMONITOR_DECLARED
# include "multimon.h"
#endif /* defined(__MINGW32_VERSION) && (__MINGW32_MAJOR_VERSION < 3) */

#include "sq.h"
#include "HostWindowPlugin.h"

BITMAPINFO *BmiForDepth(int depth);
extern HINSTANCE hInstance;
extern MSG *lastMessage;

LRESULT CALLBACK HostWndProc (HWND hwnd,
                              UINT message,
                              WPARAM wParam,
                              LPARAM lParam)
{
RECT boundingRect;

switch(message){
  /*  mousing */

  case WM_MOUSEMOVE:
      recordMouseEvent(lastMessage);
      break;

  case WM_LBUTTONDOWN:
  case WM_RBUTTONDOWN:
  case WM_MBUTTONDOWN:
  DPRINTF("HostWndProc() - WM_xBUTTONDOWN, hwnd=0x%x, focus was=0x%x.  Calling recordMouseEvent(hwnd=0x%x, message=0x%x)\n", hwnd, GetFocus(), lastMessage->hwnd, lastMessage->message);
    if(GetFocus() != hwnd) SetFocus(hwnd);
    SetCapture(hwnd); /* capture mouse input */
      recordMouseEvent(lastMessage);
      break;

  case WM_LBUTTONUP:
  case WM_RBUTTONUP:
  case WM_MBUTTONUP:
    if(GetFocus() != hwnd) SetFocus(hwnd);
    ReleaseCapture(); /* release mouse capture */
      recordMouseEvent(lastMessage);
      break;


  /*keyboard events*/
  case WM_KEYDOWN:
  case WM_SYSKEYDOWN:
  case WM_KEYUP:
  case WM_SYSKEYUP:
  case WM_CHAR:
  case WM_SYSCHAR:
   recordKeyboardEvent(lastMessage);
   break;


  /*window events*/
  case WM_MOVE:
  case WM_SIZE:
  if ((GetWindowRect(hwnd, &boundingRect)) != 0){

	sqWindowEvent *windowevent = (sqWindowEvent*) sqNextEventPut();
	windowevent->type = EventTypeWindow;
	windowevent->timeStamp = lastMessage ? lastMessage->time : GetTickCount();
	windowevent->action = WindowEventMetricChange;
	windowevent->value1 = boundingRect.left ;
	windowevent->value2 = boundingRect.top;
	windowevent->value3 = boundingRect.right;
	windowevent->value4 = boundingRect.bottom;
	windowevent->windowIndex =(int) hwnd;
	}
	break;
	
  case WM_PAINT:	
  if ((GetWindowRect(hwnd, &boundingRect)) != 0){

	sqWindowEvent *windowevent = (sqWindowEvent*) sqNextEventPut();
	windowevent->type = EventTypeWindow;
	windowevent->timeStamp = lastMessage ? lastMessage->time : GetTickCount();
	windowevent->action = WindowEventPaint;
	windowevent->value1 = boundingRect.left ;
	windowevent->value2 = boundingRect.top;
	windowevent->value3 = boundingRect.right;
	windowevent->value4 = boundingRect.bottom;
	windowevent->windowIndex =(int) hwnd;
	}
    break;


  case WM_CLOSE:
	{
	sqWindowEvent *windowevent = (sqWindowEvent*) sqNextEventPut();
	windowevent->type = EventTypeWindow;
	windowevent->timeStamp = lastMessage ? lastMessage->time : GetTickCount();
	windowevent->action = WindowEventClose;
	windowevent->windowIndex =(int) hwnd;
	}
	break;
	
  case WM_ACTIVATE:
    {
        sqWindowEvent *windowevent = (sqWindowEvent*) sqNextEventPut();
        windowevent->type = EventTypeWindow;
        windowevent->timeStamp = lastMessage ? lastMessage->time : GetTickCount();
        if (wParam == WA_INACTIVE) windowevent->action = WindowEventIconise;
        else windowevent->action = WindowEventActivated;
       	windowevent->windowIndex =(int) hwnd;      
     }
	break; 
    	
  case WM_GETMINMAXINFO:
    {
        sqWindowEvent *windowevent = (sqWindowEvent*) sqNextEventPut();
        windowevent->type = EventTypeWindow;
        windowevent->timeStamp = lastMessage ? lastMessage->time : GetTickCount();
        if (IsIconic(hwnd) != 0)windowevent->action = WindowEventIconise;
        else windowevent->action = WindowEventActivated;
       	windowevent->windowIndex =(int) hwnd;      
     }
	break;   
 }
return DefWindowProc(hwnd,message,wParam,lParam);
}

/* closeWindow: arg is int windowIndex. Fail (return 0) if anything goes wrong
 * - typically the windowIndex invalid or similar */
int closeWindow(int index)
{
	HWND hwnd = (HWND)index;
	if (!IsWindow(hwnd)) return 0;
	DestroyWindow(hwnd);
	return 1;
}

/* createWindow: takes int width, height and origin x/y plus a char* list of
 * as yet undefined attributes. Returns an int window index or 0 for failure
 * Failure may occur because of an inability to add the window, too many
 * windows already extant (platform dependant), the specified size being
 * unreasonable etc. */
int createWindowWidthheightoriginXyattrlength(int w, int h, int x, int y, char * list, int attributeListLength)
{
  HWND hwnd;

  WNDCLASS wc;

  wc.style = CS_OWNDC; /* don't waste resources ;-) */
  wc.lpfnWndProc = (WNDPROC)HostWndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = hInstance;
  wc.hIcon = LoadIcon (hInstance, MAKEINTRESOURCE(2));
  wc.hCursor = NULL;
  wc.hbrBackground = GetStockObject (WHITE_BRUSH);
  wc.lpszMenuName = NULL;
  wc.lpszClassName = TEXT("SqueakHostWindowClass");

  /* Due to Win95 bug we have to ignore the return value
     of RegisterClass(). Win95 returns several strange
     error messages (such as ERR_FUNCTION_NOT_SUPPORTED)
     when the class is already registered (which should
     result in an ERR_CLASS_ALREADY_EXISTS) */
  RegisterClass(&wc);

  hwnd = CreateWindowEx(WS_EX_APPWINDOW | WS_EX_OVERLAPPEDWINDOW,
			      TEXT("SqueakHostWindowClass"),
			      TEXT("Squeak!"),
			      WS_VISIBLE | WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
			      x,
			      y,
			      w,
			      h,
			      NULL,
			      NULL,
			      hInstance,
			      NULL);
	return (int)hwnd;
}

/* ioShowDisplayOnWindow: similar to ioShowDisplay but adds the int windowIndex
 * Return true if ok, false if not, but not currently checked */
int ioShowDisplayOnWindow( unsigned* dispBits, int width, int height, int depth, int affectedL, int affectedR, int affectedT, int affectedB, int windowIndex)
{
  HWND wnd = (HWND)windowIndex;
  HDC dc;
  BITMAPINFO *bmi;
  int lines;
  int lsbDisplay;

  if(!IsWindow(wnd))
    return 0;

  if(affectedR < affectedL || affectedT > affectedB)
    return 1;

  /* Careful here:
     After resizing the main window the affected area can
     be larger than the area covered by the display bits ... */
  if (affectedR > width) affectedR= width-1;
  if (affectedB > height) affectedB= height-1;
  /* ... and don't forget left and top - else reverse_image_* will crash */
  if (affectedL > width) affectedL= width-1;
  if (affectedT > height) affectedT= height-1;

  /* Don't draw empty areas */
  if(affectedL == affectedR || affectedT == affectedB) return 1;
  /* reload the update rectangle */
  updateRect.left = affectedL;
  updateRect.top = affectedT;
  updateRect.right = affectedR;
  updateRect.bottom = affectedB;
  /* ----- EXPERIMENTAL ----- */
  lsbDisplay = depth < 0;
  if(lsbDisplay) depth = -depth;

  bmi = BmiForDepth(depth);
  if(!bmi)
    {
  	return 0;
    }

  /* reverse the image bits if necessary */

  if( !lsbDisplay && depth < 32 )
    if(depth == 16)
      reverse_image_words((unsigned int*) dispBits, (unsigned int*) dispBits,
			  depth, width, &updateRect);
    else
      reverse_image_bytes((unsigned int*) dispBits, (unsigned int*) dispBits,
			  depth, width, &updateRect);

  bmi->bmiHeader.biWidth = width;
  bmi->bmiHeader.biHeight = -height;
  bmi->bmiHeader.biSizeImage = 0;

  dc = GetDC(wnd);
  if(!dc) {
    printLastError(TEXT("ioShowDisplayBits: GetDC() failed"));
    return 0;
  }



  lines = SetDIBitsToDevice(dc,
  	    0, /* dst_x */
  	    0, /* dst_y */
  	    width,  /* dst_w */
  	    height, /* dst_h */
  	    0, /* src_x */
  	    0, /* src_y */
  	    0, /* start scan line in DIB */
  	    height, /* num scan lines in DIB */
	    (void*) dispBits,  /* bits */
	    bmi,
	    DIB_RGB_COLORS);

  if(lines == 0) {
    /* Note: the above is at least five times faster than what follows.
       Unfortunately, it also requires quite a bit of resources to
       be available. These are almost always available except in a
       few extreme conditions - but to compensate for those the
       following is provided. */
    int pitch, start, end, nPix, line, left;
    int bitsPtr;

    /* compute pitch of form */
    pitch = ((width * depth) + 31 & ~31) / 8;
    /* compute first word of update region */
    start = ((updateRect.left * depth) & ~31) / 8;
    /* compute last word of update region */
    end   = ((updateRect.right * depth) + 31 & ~31) / 8;
    /* compute #of bits covered in update region */
    nPix = ((end - start) * 8) / depth;
    left = (start * 8) / depth;
    bmi->bmiHeader.biWidth = nPix;
    bmi->bmiHeader.biHeight = 1;
    bmi->bmiHeader.biSizeImage = 0;
    bitsPtr = (int)(dispBits + start + (updateRect.top * pitch));
    for(line = updateRect.top; line < updateRect.bottom; line++) {
      lines = SetDIBitsToDevice(dc, left, line, nPix, 1, 0, 0, 0, 1,
				(void*) bitsPtr, bmi, DIB_RGB_COLORS);
      bitsPtr += pitch;
    }
  }

  ReleaseDC(wnd,dc);

  if(lines == 0) {
    printLastError(TEXT("SetDIBitsToDevice failed"));
    warnPrintf(TEXT("width=%d,height=%d,bits=%X,dc=%X\n"),
	       width, height, dispBits,dc);
  }
  /* reverse the image bits if necessary */

  if( !lsbDisplay && depth < 32 )
    if(depth == 16)
      reverse_image_words((unsigned int*) dispBits, (unsigned int*) dispBits,
			  depth, width, &updateRect);
    else
      reverse_image_bytes((unsigned int*) dispBits, (unsigned int*) dispBits,
			  depth, width, &updateRect);

  return 1;
}



/* ioSizeOfWindow: arg is int windowIndex. Return the size of the specified
 * window in (width<<16 | height) format like ioScreenSize.
 * Return -1 for failure - typically invalid windowIndex
 * -1 is chosen since it would correspond to a window size of +/-32k@+/-32k
 * which it is hoped is unlikely for some time to come */
int ioSizeOfWindow(int windowIndex)
{
 HWND hwnd = (HWND)windowIndex;
 RECT boundingRect;
 int wsize;

 if ((GetWindowRect(hwnd, &boundingRect)) != 0){

	wsize = ((boundingRect.right - boundingRect.left) << 16)| (boundingRect.bottom - boundingRect.top);
	return (wsize);
	}
	return -1;
}

/* ioSizeOfNativeWindow: arg is void* windowHandle, defined as unsigned long
 * for convenience. Return the size of the specified native window in
 * (width<<16 | height) format like ioScreenSize.
 * Return -1 for failure - typically invalid windowHandle
 * -1 is chosen since it would correspond to a window size of +/-32k@+/-32k
 * which it is hoped is unlikely for some time to come */
int ioSizeOfNativeWindow(unsigned long windowHandle)
{
	RECT boundingRect;

	return GetWindowRect((HWND)windowHandle, &boundingRect)
		? ((boundingRect.right - boundingRect.left) << 16)
			| ((boundingRect.bottom - boundingRect.top) & 0xFFFF)
		: -1;
}

int ioSizeOfNativeDisplay(unsigned long windowHandle)
{
	RECT boundingRect;

	return GetClientRect((HWND)windowHandle, &boundingRect)
		? ((boundingRect.right - boundingRect.left) << 16)
			| ((boundingRect.bottom - boundingRect.top) & 0xFFFF)
		: -1;
}

/* ioSizeOfWindowSetxy: args are int windowIndex, int w & h for the
 * width / height to make the window. Return the actual size the OS
 * produced in (width<<16 | height) format or -1 for failure as above. */
int ioSizeOfWindowSetxy(int windowIndex, int w, int h)
{
 HWND hwnd = (HWND)windowIndex;
 RECT boundingRect;
 int wsize;
 
 if ((GetWindowRect(hwnd, &boundingRect)) == 0)return -1;

 if (MoveWindow(hwnd,
            boundingRect.left,
            boundingRect.top,
            w,
            h,
            TRUE)==0) return -1;
 wsize = ioSizeOfWindow(windowIndex);
 return (wsize);
 
}

/* ioPositionOfWindow: arg is int windowIndex. Return the pos of the specified
 * window in (left<<16 | top) format like ioScreenSize.
 * Return -1 (as above) for failure - typically invalid windowIndex */
int ioPositionOfWindow(int windowIndex)
{
 HWND hwnd = (HWND)windowIndex;
 RECT boundingRect;
 int wpos;

 if ((GetWindowRect(hwnd, &boundingRect)) != 0){

	wpos = ((boundingRect.left) << 16)| (boundingRect.top);
	return (wpos);
	}
	return -1;
}

/* ioPositionOfNativeWindow: arg is void* windowHandle, defined as unsigned long
 * for convenience. Return the pos of the specified native window in
 * (left<<16 | top) format like ioScreenSize (& ioPositionOfWindow).
 * Return -1 (as above) for failure - typically invalid windowHandle */
int ioPositionOfNativeWindow(unsigned long windowHandle)
{
	RECT boundingRect;

	return GetWindowRect((HWND)windowHandle, &boundingRect)
		? (boundingRect.left << 16) | boundingRect.top
		: -1;
}

/* Creaky old mingw with the 2.95 compiler is missing the WINDOWINFO define. */
#if !defined(WS_ACTIVECAPTION)
typedef struct {
    DWORD cbSize;
    RECT rcWindow;
    RECT rcClient;
    DWORD dwStyle;
    DWORD dwExStyle;
    DWORD dwWindowStatus;
    UINT cxWindowBorders;
    UINT cyWindowBorders;
    ATOM atomWindowType;
    WORD wCreatorVersion;
} WINDOWINFO, *PWINDOWINFO, *LPWINDOWINFO;

BOOL WINAPI GetWindowInfo(HWND hwnd, PWINDOWINFO pwi);

static BOOL WINAPI (*GetWindowInfoFunc)(HWND hwnd, PWINDOWINFO pwi);
#endif

int ioPositionOfNativeDisplay(unsigned long windowHandle)
{
	RECT boundingRect;
	WINDOWINFO wi;
	int insideHeight, windowHeight, titleHeight, x, y;

	wi.cbSize = sizeof(wi);
#if !defined(WS_ACTIVECAPTION)
	if (!GetWindowInfoFunc) {
		HANDLE hUser = LoadLibrary( "user32.dll" );
		GetWindowInfoFunc = GetProcAddress(hUser, "GetWindowInfo");
	}
	GetWindowInfoFunc((HWND)windowHandle, &wi);
#else
	GetWindowInfo((HWND)windowHandle, &wi);
#endif
#if 0
	printf("rcWindow %d@%d corner: %d@%d\n",
			wi.rcWindow.left, wi.rcWindow.top,
			wi.rcWindow.right, wi.rcWindow.bottom);
	printf("rcClient %d@%d corner: %d@%d\n",
			wi.rcClient.left, wi.rcClient.top,
			wi.rcClient.right, wi.rcClient.bottom);
	printf("cxWindowBorders %d cyWindowBorders %d\n",
			wi.cxWindowBorders, wi.cyWindowBorders);
#endif

	if (!GetWindowRect((HWND)windowHandle, &boundingRect))
		return -1;
	windowHeight = wi.rcWindow.bottom - wi.rcWindow.top;
	insideHeight = wi.rcClient.bottom - wi.rcClient.top;
	titleHeight = windowHeight - insideHeight - wi.cyWindowBorders;
	x = boundingRect.left + wi.cxWindowBorders;
	y = boundingRect.top + titleHeight;
	return (x << 16) | (short)y;
}

/* ioPositionOfWindowSetxy: args are int windowIndex, int x & y for the
 * origin x/y for the window. Return the actual origin the OS
 * produced in (left<<16 | top) format or -1 for failure, as above */
int ioPositionOfWindowSetxy(int windowIndex, int x, int y)
{
 HWND hwnd = (HWND)windowIndex;
 RECT boundingRect;
 int wpos;

 if ((GetWindowRect(hwnd, &boundingRect)) == 0) return -1;

 if (MoveWindow(hwnd,
            x,
            y,
            (boundingRect.right - boundingRect.left),
	        (boundingRect.bottom - boundingRect.top),
            TRUE)==0) return -1;
 wpos = ioPositionOfWindow(windowIndex);
 return (wpos);
}

/* ioSetTitleOfWindow: args are int windowIndex, char* newTitle and
 * int size of new title. Fail with -1 if windowIndex is invalid, string is too long for platform etc. Leave previous title in place on failure */
int ioSetTitleOfWindow(int windowIndex, char * newTitle, int sizeOfTitle)
{
 HWND hwnd = (HWND)windowIndex;
 char *title;
 if (sizeOfTitle > 1023) sizeOfTitle = 1024; /*to avoid allocating a really huge array, shouln't be possible displaying this in titlebar, also in the near future*/
 title = malloc(sizeOfTitle+1);
 if (title == NULL) return -1;
 strncpy(title, newTitle, sizeOfTitle);
 title[sizeOfTitle]='\0';
 if(SetWindowText(hwnd, (LPSTR) title) == 0){
       free (title);
       return -1;
       }
 free (title);   

 return sizeOfTitle;
}

/* ioCloseAllWindows: intended for VM shutdown.
 * Close all the windows that appear to be open.
 * No useful return value since we're getting out of Dodge anyway.
 */
int ioCloseAllWindows(void){}


sqInt ioSetCursorPositionXY(sqInt x, sqInt y)
{
	return SetCursorPos(x,y)
		? 0
		: -1;
}

/* Return the pixel origin (topleft) of the platform-defined working area
   for the screen containing the given window. */
int ioPositionOfScreenWorkArea (int windowIndex) {
	HMONITOR hMonitor;
	MONITORINFO mi;
	RECT workArea;
  	hMonitor = MonitorFromWindow ((HWND) windowIndex, MONITOR_DEFAULTTONEAREST);
	mi.cbSize = (sizeof(mi));
	GetMonitorInfo(hMonitor, &mi);
	workArea = mi.rcWork;
	return (workArea.left << 16) | (workArea.top & 0xFFFF);
}

/* Return the pixel extent of the platform-defined working area
   for the screen containing the given window. */
int ioSizeOfScreenWorkArea (int windowIndex) {
	HMONITOR hMonitor;
	MONITORINFO mi;
	RECT workArea;
  	hMonitor = MonitorFromWindow ((HWND) windowIndex, MONITOR_DEFAULTTONEAREST);
	mi.cbSize = (sizeof(mi));
	GetMonitorInfo(hMonitor, &mi);
	workArea = mi.rcWork;
	return ((workArea.right - workArea.left) << 16) | ((workArea.bottom - workArea.top) & 0xFFFF);
}


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

#include <stdio.h>
#include <windows.h>

#if _WIN32_WINNT >= 0x0500 /* newer cygwins; MSVC builds */
# include <winuser.h>
# include <psapi.h>
#else /* _WIN32_WINNT >= 0x0500 i.e. on old mingw */
typedef struct tagGUITHREADINFO {
    DWORD cbSize;
    DWORD flags;
    HWND hwndActive;
    HWND hwndFocus;
    HWND hwndCapture;
    HWND hwndMenuOwner;
    HWND hwndMoveSize;
    HWND hwndCaret;
    RECT rcCaret;
} GUITHREADINFO, *LPGUITHREADINFO;

HANDLE hUser32 = NULL, hPsapi = NULL, hKernel32 = NULL;
static BOOL (WINAPI *PrintWindow)(HWND, HDC, UINT) = NULL;
static DWORD (WINAPI *GetModuleFileNameExW)(HANDLE,HANDLE,WCHAR*,DWORD)=NULL;
static DWORD (WINAPI *GetProcessId)(HANDLE);
static BOOL (WINAPI *GetGUIThreadInfo)(DWORD,LPGUITHREADINFO);
#endif /* _WIN32_WINNT >= 0x0500 */

#include "PSharePlugin.h"

#define HWND_MAX 1024
static HWND wndCache[HWND_MAX];
static int wndCount = 0;

static int pShareDebug = 0;

static BITMAPINFO *bmi32 = NULL;

/* Keymap from sqWin32Window.c */
static unsigned char keymap[256] = {
  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
 96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111,
112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
173,176,226,196,227,201,160,224,246,228,178,220,206,179,182,183,
184,212,213,210,211,165,208,209,247,170,185,221,207,186,189,217,
202,193,162,163,219,180,195,164,172,169,187,199,194,197,168,248,
161,177,198,215,171,181,166,225,252,218,188,200,222,223,240,192,
203,231,229,204,128,129,174,130,233,131,230,232,237,234,235,236,
245,132,241,238,239,205,133,249,175,244,242,243,134,250,251,167,
136,135,137,139,138,140,190,141,143,142,144,145,147,146,148,149,
253,150,152,151,153,155,154,214,191,157,156,158,159,254,255,216
};

/* The following is the inverse keymap */
static unsigned char iKeymap[256];

#define DBGLOG(what) { \
  FILE *f = fopen("pshare.log", "at");\
  if(f) {fprintf what; fflush(f); fclose(f);}\
}

/* pShareInit:
   Initialize personal share plugin.
*/
int pShareInit(void) {
  int i;

#if _WIN32_WINNT < 0x0500 /* i.e. on old mingw */
  hUser32 = LoadLibrary("user32.dll");
  PrintWindow = (void*) GetProcAddress(hUser32, "PrintWindow");
  GetGUIThreadInfo = (void*) GetProcAddress(hUser32, "GetGUIThreadInfo");

  hPsapi = LoadLibrary("psapi.dll");
  GetModuleFileNameExW = (void*)GetProcAddress(hPsapi, "GetModuleFileNameExW");
  hKernel32 = LoadLibrary("kernel32.dll");
  GetProcessId = (void*) GetProcAddress(hKernel32, "GetProcessId");
#endif /*  _WIN32_WINNT < 0x0500 */

  bmi32 = (BITMAPINFO*) calloc(1,sizeof(BITMAPINFO) + 4 * sizeof(DWORD));
  bmi32->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi32->bmiHeader.biPlanes = 1;
  bmi32->bmiHeader.biBitCount = 32;
  bmi32->bmiHeader.biCompression = BI_BITFIELDS;
  ((DWORD*) bmi32->bmiColors)[0] = 0x00FF0000; /* red mask */
  ((DWORD*) bmi32->bmiColors)[1] = 0x0000FF00; /* green mask */
  ((DWORD*) bmi32->bmiColors)[2] = 0x000000FF; /* blue mask */
  ((DWORD*) bmi32->bmiColors)[3] = 0xFF000000; /* alpha mask */
  bmi32->bmiHeader.biWidth = 0;
  bmi32->bmiHeader.biHeight = 0;

  for(i=0;i<256;i++) {
    iKeymap[keymap[i]] = i;
  }
  return 1;
}

/* pShareShutdown:
   Clean out the personal share plugin.
*/
int pShareShutdown(void) {
}

int pShareEnableDebug(int debug) {
  pShareDebug = debug;
  return debug;
}

/* pShareGetWindowID:
   Returns the window ID for the window with the given index (-1 on error).
*/
static BOOL CALLBACK fillWndCache(HWND hwnd, LPARAM lparam) {
  /* eliminate non-windows */
  if(!IsWindow(hwnd)) return TRUE;
  wndCache[wndCount++] = hwnd;
  return wndCount < HWND_MAX;
}

int pShareGetWindowID(int index) {
  if(index == 0) {
    wndCount = 0;
    EnumWindows(fillWndCache, 0);
  }
  if(index < wndCount) return (int)wndCache[index];
  return -1;
}

/* pShareGetWindowIcon:
   Returns the icon for the window with the given ID.
*/

static int pShareExtractIcon(HANDLE hIcon, int w, int h, int d, void*formBits){
  ICONINFO info;
  BITMAP bm;
  HDC mDC = NULL;
  HBITMAP hbm = NULL;
  HANDLE old = NULL;
  void *dibBits = NULL;
  int ok = 0;
  int verbose = 0;

  if(hIcon == 0) return 0;

  if(GetIconInfo(hIcon, &info) == 0) {
    if(verbose) printf("pShareGetIcon: GetIconInfo failed\n");
    return 0;
  }
  if(GetObject(info.hbmColor, sizeof(bm), &bm) == 0) {
    if(verbose) printf("pShareGetIcon: GetObject failed\n");
    goto failed;
  }
  if(bm.bmWidth != w || bm.bmHeight != h) {
    if(verbose) printf("pShareGetIcon: Icon w/h mismatch\n");
    goto failed;
  }

  bmi32->bmiHeader.biWidth = w;
  bmi32->bmiHeader.biHeight = -h;

  mDC = CreateCompatibleDC(NULL);
  if(!mDC) {
    if(verbose) printf("pShareGetIcon: CreateCompatibleDC failed\n");
    goto failed;
  }

  hbm = CreateDIBSection(mDC, bmi32, DIB_RGB_COLORS, &dibBits, NULL, 0);
  if (!hbm) {
    if(verbose) printf("pShareGetIcon: CreateDIBSection failed\n");
    goto failed;
  }

  old = SelectObject(mDC, hbm);
  DrawIconEx(mDC, 0, 0, hIcon, w, h, 0, 0, DI_NORMAL);
  memcpy(formBits, dibBits, w*h*4);
  SelectObject(mDC, old);

  { /* Fix some icons that don't have the alpha channel set correctly. */
    unsigned int i, *bits = formBits;
    /* See if there is any alpha at all in the bits */
    for(i=0; i<w*h; i++)
      if(bits[i] & 0xFF000000) break;
    /* If not, color key it in */
    if(i == w*h)
      for(i=0; i<w*h; i++)
	if(bits[i]) bits[i] |= 0xFF000000;
  }
  ok = 1; /* success */
 failed:
  if(hbm) DeleteObject(hbm);
  if(mDC) DeleteDC(mDC);
  if(info.hbmMask) DeleteObject(info.hbmMask);
  if(info.hbmColor) DeleteObject(info.hbmColor);
  return ok;
}

int pShareGetWindowIcon(int id, int w, int h, int d, void* formBits) {
  HWND hwnd = (HWND)id;
  HICON hIcon = NULL;

  if(!IsWindow(hwnd)) return 0;
  if(w != h) return 0; /* only square forms */
  if(d != 32) return 0; /* only 32bpp forms */

  hIcon = (HANDLE)GetClassLong(hwnd, GCL_HICONSM);
  return pShareExtractIcon(hIcon, w, h, d, formBits);
}

/* pShareGetDocumentIcon:
   Returns the icon for a particular document file.
*/
int pShareGetDocumentIcon(char *fileName, int w, int h, int d, void* formBits){
  WCHAR wideName[MAX_PATH];
  SHFILEINFO fileInfo;
  int sz, ok;

  if(w != h) return 0; /* only square forms */
  if(d != 32) return 0; /* only 32bpp forms */

  sz = MultiByteToWideChar(CP_UTF8, 0, fileName, -1, NULL, 0);
  if(sz >= MAX_PATH) {
    printf("pShareGetDocumentIcon: fileName is too long\n");
    return 0;
  }
  if(MultiByteToWideChar(CP_UTF8, 0, fileName, -1, wideName, MAX_PATH)==0) {
    printf("pShareGetDocumentIcon: UTF-8 conversion failed\n");
    return 0;
  }
  if(!SHGetFileInfoW(wideName, 0, &fileInfo, sizeof(fileInfo), SHGFI_ICON)) {
    printf("pShareGetDocumentIcon: SHGetFileInfoW failed (%s)\n", fileName);
    return 0;
  }
  ok = pShareExtractIcon(fileInfo.hIcon, w, h, d, formBits);
  if(fileInfo.hIcon) DestroyIcon(fileInfo.hIcon);
  return ok;
}

/* pShareGetWindowBitmap:
   Return the window bits for a particular window, diffing to the previous bits
*/

static void diffBits(int* srcBits, int *dstBits, int w, int h, int *rect) {
  int l,r,t,b,y;

  /* scan from the top */
  for(t = 0; t < h; t++) {
    if(memcmp(srcBits+(w*t), dstBits+(w*t), w*4) != 0) break;
  }
  if(t == h) {
    /* no changes at all */
    rect[0] = rect[1] = rect[2] = rect[3] = 0;
    return;
  }

  /* scan from bottom */
  for(b = h-1; b > t; b--) {
    if(memcmp(srcBits+(w*b), dstBits+(w*b), w*4) != 0) break;
  }

  /* scan from left and right */
  l = w;
  r = 0;
  for(y = t; y <=b; y++) {
    int base = w*y;
    register int x;
    /* scan from left */
    for(x = 0; x < l; x++) {
      if(srcBits[base+x] != dstBits[base+x]) break;
    }
    /* scan from right */
    l = x;
    for(x=w-1; x > r; x--) {
      if(srcBits[base+x] != dstBits[base+x]) break;
    }
    r = x;
  }
  rect[0] = l;
  rect[1] = t;
  rect[2] = r+1;
  rect[3] = b+1;
  return;
}

static RECT clipWindowRect;
static HWND clipWindowHandle;
static HDC  clipWindowDC;
static HWND overlapWindows[1000];
static RECT overlapRects[1000];
static int  overlapCount;
static int  overlapMatch;

static BOOL CALLBACK clipWindow(HWND hwnd, LPARAM lparam) {
  int result;
  RECT rect;

  /* If we've reached the target window, we are done */
  if(hwnd == clipWindowHandle) return 0;

  /* Skip invalid handles */
  if(!IsWindow(hwnd)) return 1;

  /* Skip invisible windows */
  if(!IsWindowVisible(hwnd)) return 1;

  /* Get the window rectangle */
  GetWindowRect(hwnd, &rect);

  /* Skip empty windows */
  if(rect.right <= rect.top || rect.bottom <= rect.top) return 1;

  /* Skip non-overlapping windows */
  if(rect.right <= clipWindowRect.left ||
     rect.left >= clipWindowRect.right ||
     rect.bottom <= clipWindowRect.top ||
     rect.top >= clipWindowRect.bottom) return 1;

  /* Exclude the rectangle from the clip region */
  result = ExcludeClipRect(clipWindowDC, 
			   rect.left-clipWindowRect.left, 
			   rect.top-clipWindowRect.top, 
			   rect.right-clipWindowRect.left,
			   rect.bottom-clipWindowRect.top);

  /* If the result is a non-empty region continue with the next window */
  if(result == SIMPLEREGION || result == COMPLEXREGION) {
    /* Remember the window and its bounds */
    overlapWindows[overlapCount] = hwnd;
    overlapRects[overlapCount] = rect;
    overlapCount++;
    return 1;
  }

  /* We're done if the result is an error or empty */
  return 0;
}

static BOOL CALLBACK verifyClip(HWND hwnd, LPARAM lparam) {
  RECT rect;

  /* If we've reached the target window, we are done */
  if(hwnd == clipWindowHandle) return 0;

  /* Skip invalid handles */
  if(!IsWindow(hwnd)) return 1;

  /* Skip invisible windows */
  if(!IsWindowVisible(hwnd)) return 1;

  /* Get the window rectangle */
  GetWindowRect(hwnd, &rect);

  /* Skip empty windows */
  if(rect.right <= rect.top || rect.bottom <= rect.top) return 1;

  /* Skip non-overlapping windows */
  if(rect.right <= clipWindowRect.left ||
     rect.left >= clipWindowRect.right ||
     rect.bottom <= clipWindowRect.top ||
     rect.top >= clipWindowRect.bottom) return 1;

  /* Compare window and bounds */
  if(overlapWindows[overlapCount] != hwnd || 
     !EqualRect(&overlapRects[overlapCount], &rect)) {
    /* we failed - either z-order or rects do not match*/
    overlapMatch = 0;
    return 0;
  }
  /* continue testing */
  overlapCount++;
  return 1;
}

int* pShareGetWindowBitmap(int id, int w, int h, int d, void* bits, int incr){
  HWND hwnd = (HWND)id;
  HDC hDC = NULL, mDC = NULL;
  HBITMAP hbm = NULL;
  HANDLE old = NULL;
  MSG msg;
  int printed = 0;
  unsigned int *dibBits = NULL;
  static int rect[4];

  if(!IsWindow(hwnd)) return NULL;

  /* we cannot render iconic windows */
  if(IsIconic(hwnd)) return NULL;

  if(d != 32) return NULL; /* only 32bpp forms */

  /* mark entire rect dirty */
  rect[0] = 0;
  rect[1] = 0;
  rect[2] = w;
  rect[3] = h;

  bmi32->bmiHeader.biWidth = w;
  bmi32->bmiHeader.biHeight = -h;

  hDC = GetWindowDC(hwnd);
  if(!hDC) {
    printf("pShareGetBitmap: GetWindowDC failed\n");
    goto failed;
  }
  mDC = CreateCompatibleDC(hDC);
  if(!mDC) {
    printf("pShareGetBitmap: CreateCompatibleDC failed\n");
    goto failed;
  }

  hbm = CreateDIBSection(mDC, bmi32, DIB_RGB_COLORS,(LPVOID*)&dibBits,NULL,0);
  if (!hbm) {
    printf("pShareGetBitmap: CreateDIBSection failed\n");
    goto failed;
  }
  old = SelectObject(mDC, hbm);

  if(incr) {

    /* Do an incremental update. Incremental updates do not use
       PrintWindow to avoid flickering and other strange effects. 
       Consequently, they cannot be used for the initial capture
       when the window might be fully obscured by forums itself.
    */
    int result;
    RECT r;

    /* clip the obscured regions of the window */
    clipWindowHandle = hwnd;
    clipWindowDC = mDC;
    GetWindowRect(hwnd, &clipWindowRect);

    /* Set the base clipping rect */
    IntersectClipRect(mDC, 0, 0, clipWindowRect.right - clipWindowRect.left,
		      clipWindowRect.bottom - clipWindowRect.top);
    /* Exclude all overlapping windows */
    overlapCount = 0;
    EnumWindows(clipWindow, 0);
    result = GetClipBox(mDC, &r);

    if(result == SIMPLEREGION || result == COMPLEXREGION) {
      HDC screenDC = CreateDC("DISPLAY", NULL, NULL, NULL);
      /* put the old bits back "under" the original */
      memcpy(dibBits, bits, w*h*4);
      /* Now BitBlt the update */
      BitBlt(mDC, 0, 0, w, h, screenDC, clipWindowRect.left, 
	     clipWindowRect.top, SRCCOPY);
      DeleteDC(screenDC);

      /* Verify that the window composition has not changed while
	 blitting. This happens when windows get moved around. */
      GetWindowRect(hwnd, &r);
      if(EqualRect(&clipWindowRect, &r)) {
	/* Verify the windows in front of the captured window */
	overlapMatch = 1;
	overlapCount = 0;
	EnumWindows(verifyClip, 0);
      } else {
	/* We've been blitting from an obsolete location */
	overlapMatch = 0;
      }
      if(overlapMatch) {
	/* We're still good, compute diff to see what has changed */
	diffBits(dibBits, bits, w, h, rect);
      } else {
	/* No match - skip this update */
	if(pShareDebug) {
	  printf("[PShare]: Skipping frame (window composition changed)\n");
	}
	rect[0] = rect[1] = rect[2] = rect[3] = 0;
      }
    } else {
      /* fully obscured window. do nothing */
      rect[0] = rect[1] = rect[2] = rect[3] = 0;
    }

  } else {
    /* Make a full (non-incremental) update of the window */

    /* NOTE: PrintWindow() can take a VERY long time (300ms).
       It can even overlap with window closing in which case
       the result of the BitBlt/ReleaseDC calls below are plain
       BSODs about some unpaged memory access or whatever
       (probably because the window has been unmapped already
       but we're still trying to draw onto it). So guard the
       following calls just so we don't constantly BSOD. */
    if(IsWindow(hwnd)) {
      PrintWindow(hwnd, mDC, 0);
    }

    /* Unfortunately, PrintWindow has some VERY weird effects.
       One of them is that the window itself stops redrawing completely.
       Fortunately, we can "fix" this by simply blitting the image
       we just captured back onto the window (weird, eh?) */
    if(IsWindow(hwnd)) {
      BitBlt(hDC, 0, 0, w, h, mDC, 0, 0, SRCCOPY);
    }
  }

  if(rect[2] > rect[0]) {
    /* Copy the bits. TODO: This should do a partial copy for diffs */
    memcpy(bits, dibBits, w*h*4);
  }

  SelectObject(mDC, old);

 failed:
  if(hbm) DeleteObject(hbm);
  if(mDC) DeleteDC(mDC);
  if(hDC && IsWindow(hwnd)) ReleaseDC(hwnd, hDC);
  return rect;
}

/* pShareGetWindowLabel:
   Returns the label for the window with the given ID.
*/
char *pShareGetWindowLabel(int id) {
  DWORD count;
  HWND hwnd = (HWND)id;
  static WCHAR wndLabel[1000];
  static char utf8Label[1024];

  if(!IsWindow(hwnd)) return NULL;
  count = GetWindowTextW(hwnd, wndLabel, 1000);
  if(count == 0) return NULL;
  if(WideCharToMultiByte(CP_UTF8, 0, wndLabel, -1, 
			 utf8Label, 1024, NULL, NULL) > 0) return utf8Label;
  return NULL;
}

/* pShareGetWindowAppName:
   Returns the name of the executable for the window with the given ID.
*/
char *pShareGetWindowAppName(int id) {
  DWORD count, pid;
  HWND hwnd = (HWND)id;
  HANDLE hProcess;
  static WCHAR wndLabel[1000];
  static char utf8Label[1024];

  if(!IsWindow(hwnd)) return NULL;
  /* Figure out pid for window */
  GetWindowThreadProcessId(hwnd, &pid);
  /* Get a process handle with query and vm_read access */
  hProcess = OpenProcess(0x0410, 0, pid);
  if(hProcess == NULL) return NULL;
  /* Ask for the module name */
  count = GetModuleFileNameExW(hProcess, NULL, wndLabel, 1000);
  if(count == 0) return NULL;

  /* TODO: Should really read the version info from above file
     and return the product name so that the app name is something
     along the lines of "Mozilla Firefox" instead of firefox.exe */

  if(WideCharToMultiByte(CP_UTF8, 0, wndLabel, -1, 
			 utf8Label, 1024, NULL, NULL) > 0) return utf8Label;
  return NULL;
}


/* pShareGetWindowOwner:
   Returns the 'owner' for a given window.
*/
static int matchNextWindow = 0;
static HWND matchedWindow = NULL;

static BOOL CALLBACK findOwnerWindow(HWND hwnd, LPARAM target) {
  if((int)hwnd == (int)target) {
    matchNextWindow = 1;
    return 1;
  }
  if(matchNextWindow && pShareIsWindowSharable((int)hwnd)) {
    matchedWindow = hwnd;
    return 0;
  }
  return 1;
}

int pShareGetWindowOwner(int id) {
  HWND hwnd = (HWND)id;
  HWND hOwner;
  DWORD tid, pid, style;
  GUITHREADINFO info;

  if(!IsWindow(hwnd)) return -1;

  /* Easy case first */
  hOwner = GetWindow(hwnd, GW_OWNER);
  while(hOwner) {
    if(pShareIsWindowSharable(hOwner)) return (int)hOwner;
    hOwner = GetWindow(hOwner, GW_OWNER);
  }

  /* Now for any menus (many of which have no owner).
     As a first step, eliminate all windows with styles that don't
     look like menus. 
  */
  style = GetWindowLong(hwnd, GWL_EXSTYLE);
  if((style & WS_EX_TOPMOST) == 0) return 0;

  style = GetWindowLong(hwnd, GWL_STYLE);
  if(style & (WS_CHILD | WS_DLGFRAME | 
	      WS_GROUP | WS_HSCROLL | WS_VSCROLL |
	      WS_MINIMIZE | WS_MAXIMIZE | WS_OVERLAPPED |
	      WS_SYSMENU | WS_THICKFRAME | WS_TABSTOP)) return 0;

  /* Treat this as a menu window and report its owner to be the next
     sharable window in the thread of the menu window.
  */
  tid = GetWindowThreadProcessId(hwnd, &pid);
  matchNextWindow = 0;
  matchedWindow = NULL;
  EnumThreadWindows(tid, findOwnerWindow, (LPARAM) hwnd);
  return (int)matchedWindow;
}

/* pShareGetWindowRect:
   Returns the bounding box for the window with the given ID. 
*/
int *pShareGetWindowRect(int id) {
  static int rect[4];
  HWND hwnd = (HWND)id;

  if(!IsWindow(hwnd)) return NULL;
  GetWindowRect(hwnd, (LPRECT)rect);
  return rect;
}

/* pShareGetWindowProcessID:
   Return the pid for a given window -1 on error. 
*/
int pShareGetWindowProcessID(int id) {
  DWORD pid = -1;
  HWND hwnd = (HWND)id;

  if(!IsWindow(hwnd)) return -1;
  GetWindowThreadProcessId(hwnd, &pid);
  return pid;
}

/* pShareIsWindowVisible:
   Return true/false/-1 if the window is visible 
*/
int pShareIsWindowVisible(int id) {
  HWND hwnd = (HWND)id;
  if(!IsWindow(hwnd)) return -1;
  return IsWindowVisible(hwnd);
}

/* pShareIsWindowMinimized:
   Return true/false/-1 if the window is minimized
*/
int pShareIsWindowMinimized(int id) {
  HWND hwnd = (HWND)id;
  if(!IsWindow(hwnd)) return -1;
  return IsIconic(hwnd);
}

/* pShareIsWindowValid:
   Return true/false if the window is valid.
*/
int pShareIsWindowValid(int id) {
  HWND hwnd = (HWND)id;
  return IsWindow(hwnd);
}

/* pShareIsWindowSharable:
   Return true/false/-1 if the window should be shared. 
*/
int pShareIsWindowSharable(int id) {
  DWORD style;
  RECT r;
  HWND hwnd = (HWND)id;
  HWND owner = NULL;

  /* Fail for invalid handles */
  if(!IsWindow(hwnd)) return -1;

  /* Eliminate invisible windows */
  if(!IsWindowVisible(hwnd)) return 0;

  /* Eliminate zero-size windows */
  GetWindowRect(hwnd, &r);
  if(r.left >= r.right || r.top >= r.bottom) return 0;

  /* Eliminate owned popup windows */
  owner = GetWindow(hwnd, GW_OWNER);
  while(owner) {
    if(pShareIsWindowSharable(owner)) return 0;
    owner = GetWindow(owner, GW_OWNER);
  }

  /* Do not share the desktop window */
  if(hwnd == GetDesktopWindow()) return 0;

  /* Eliminate some other windows, most importantly "Program Manager" */
  style = GetWindowLong(hwnd, GWL_STYLE);
  if(style & (WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX))
    return 1;

  return 0;
}

/* pShareLaunchFileOfSize:
   Launch a random document file. Return the pid. 
*/
int pShareLaunchFileOfSize(void *utf8Ptr, int utf8Len){
  DWORD pid;
  SHELLEXECUTEINFOW sei;
  int sz;
  WCHAR *docName;

  sz = MultiByteToWideChar(CP_UTF8, 0, utf8Ptr, utf8Len, NULL, 0);
  docName = (WCHAR*) calloc(sz+1, sizeof(WCHAR));
  if(MultiByteToWideChar(CP_UTF8, 0, utf8Ptr, utf8Len, docName, sz+1)==0){
    return -1;
  }
  ZeroMemory((PVOID)&sei, sizeof(sei));
  sei.cbSize = sizeof(sei);
  sei.fMask = 0x00000040 /* SEE_MASK_NOCLOSEPROCESS */;
  sei.nShow = SW_NORMAL;
  sei.lpVerb = L"open";
  sei.lpFile = docName;
  sei.lpParameters = NULL;
  if(!ShellExecuteExW(&sei)) return -1;

  /* The following test is here for the sake of XP w/o SP1
     (GetProcessId is only available on XP+SP1 or Vista) */
  if(GetProcessId != NULL) {
    pid = GetProcessId(sei.hProcess);
  } else {
    pid = 0;
  }
  CloseHandle(sei.hProcess);
  return pid;
}

/* pShareActivateWindow:
   Enables/disables sharing of a particular window.
   For now, this is a no-op on Windows since we retrieve the bits synchronously.
   If and when we change this to an asynchronous thread, we will use it.
*/
int pShareActivateWindow(int id, int shareOn) {
  HWND hwnd = (HWND)id;

  if(!IsWindow(hwnd)) return 0;
  if(!shareOn) return 1;
  /* restore the window if it iconic */
  if(IsIconic(hwnd)) {
    ShowWindow(hwnd, SW_RESTORE);
  }
  return 1;
}

/* If a window is not iconic arrange that it is unobscured so that
 * pShareGetWindowBitmap can access an unobscured image of the window's
 * contents.  Depending on the window manager this may require raising the
 * window to the top of the stacking order.
 */
int
pShareUnobscureWindow(int id)
{
	if (IsIconic((HWND)id))
		return 0;

	if (!SetWindowPos((HWND)id, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE))
		return -1;

	return 1;
}

/* pSharePostEvent:
   Posts an event to the given window.
*/
static UINT mouseMsgMap[8] = {
  WM_MOUSEMOVE,
  WM_RBUTTONDOWN, WM_RBUTTONUP,
  WM_MBUTTONDOWN, WM_MBUTTONUP,
  WM_LBUTTONDOWN, WM_LBUTTONUP,
  0
};
static UINT mouseNcMap[8] = {
  WM_NCMOUSEMOVE,
  WM_NCRBUTTONDOWN, WM_NCRBUTTONUP,
  WM_NCMBUTTONDOWN, WM_NCMBUTTONUP,
  WM_NCLBUTTONDOWN, WM_NCLBUTTONUP,
  0
};

static HWND findEventTarget(HWND top, POINT *pt) {
  HWND target = top;
  HWND child;

  child = ChildWindowFromPointEx(target,*pt, CWP_SKIPINVISIBLE);
  while(child != NULL && child != target) {
    char lbl[1000];
    /* convert point to child coordinates */
    MapWindowPoints(target, child, pt, 1);
    target = child;
    child = ChildWindowFromPointEx(target,*pt, CWP_SKIPINVISIBLE);
  }
  return target;
}

int pSharePostMouseEvent(int id, int type, int x, int y, int buttons) {
  HWND hwnd = (HWND)id;
  HWND target, child;
  RECT r;
  POINT pt;
  DWORD msg, wParam, lParam;

  if(!IsWindow(hwnd)) return 0;

  /* Convert x/y to client coordinates */
  pt.x = x;
  pt.y = y;
  ScreenToClient(hwnd, &pt);

  /* find the child window to receive the input */
  target = findEventTarget(hwnd, &pt);

  /* see if the point is in the client or non-client area,
     but only deliver NC events when the mouse is not down yet */
  GetClientRect(target, &r);
  if((type & 1) && 
     (r.left > pt.x || r.right <= pt. x || r.top > pt.y || r.bottom <= pt.y)) {
      /* NC mouse positions are global - convert everything back */
      ClientToScreen(target, &pt);
      lParam = (pt.x & 0xFFFF) + ( (pt.y & 0xFFFF) << 16);
      wParam = SendMessage(target, WM_NCHITTEST, 0, lParam);
      msg = mouseNcMap[ type & 7 ];
  } else {
    /* Regular mouse message */
    msg = mouseMsgMap[type & 7];
    wParam = 0;
    if(buttons & 1) wParam |= MK_RBUTTON;
    if(buttons & 2) wParam |= MK_MBUTTON;
    if(buttons & 4) wParam |= MK_LBUTTON;
    if(buttons & 8) wParam |= MK_SHIFT;
    lParam = (pt.x & 0xFFFF) + ( (pt.y & 0xFFFF) << 16);
  }
  /* now post the message to the last *parent* window */
  return PostMessage(target, msg, wParam, lParam);
}


/* Map virtual key (from sqWin32Window.c) */
static int mapVirtualKey(int virtKey) {
  switch (virtKey) {
    case VK_DELETE: return 127;
    case VK_INSERT: return 5;
    case VK_PRIOR:  return 11;
    case VK_NEXT :  return 12;
    case VK_END  :  return 4;
    case VK_HOME :  return 1;
    case VK_LEFT :  return 28;
    case VK_RIGHT:  return 29;
    case VK_UP   :  return 30;
    case VK_DOWN :  return 31;
    case VK_RETURN: return 13;
    /* remap appropriately so that we get _all_ key down events */
    case 127: return VK_DELETE;
    case 5: return VK_INSERT;
    case 11: return VK_PRIOR;
    case 12: return VK_NEXT;
    case 4: return VK_END;
    case 1: return VK_HOME;
    case 28: return VK_LEFT;
    case 29: return VK_RIGHT;
    case 30: return VK_UP;
    case 31: return VK_DOWN;
    /* case 13: return VK_RETURN; */
  }
  return 0;
}

int pSharePostKeyboardEvent(int id,int type,int x,int y,int key,int buttons) {
  HWND hwnd = (HWND)id;
  HWND target;
  RECT r;
  POINT pt;
  DWORD msg, wParam, lParam;
  DWORD tid, pid;
  GUITHREADINFO info;

  if(!IsWindow(hwnd)) {
    DBGLOG((f, "keyboard: bad hwnd %x\n", hwnd));
    return 0;
  }
  tid = GetWindowThreadProcessId(hwnd, &pid);
  info.cbSize = sizeof(GUITHREADINFO);
  if(!GetGUIThreadInfo(tid, &info)) {
    DBGLOG((f, "keyboard: GetGUIThreadInfo failed\n"));
    return 0;
  }

  /* Convert x/y to client coordinates */
  pt.x = x;
  pt.y = y;
  ScreenToClient(hwnd, &pt);

  /* find the child window to receive the input */
  target = findEventTarget(hwnd, &pt);

  target = info.hwndFocus;
  if(target == NULL) target = hwnd;

  switch(type) {
  case 1: /* key down */
    return 1;
    msg = WM_KEYDOWN;
    wParam = mapVirtualKey(key);
    lParam = 1;
    break;
  case 2: /* key stroke */
    msg = WM_CHAR;
    wParam = iKeymap[key & 255];
    lParam = 1;
    break;
  case 3: /* key up */
    return 1;
    msg = WM_KEYUP;
    wParam = mapVirtualKey(key);
    lParam = 1;
    break;
  default:
    DBGLOG((f, "keyboard: bad type %d\n", type));
    return 0;
  }
  if(pShareDebug) {
    printf("keyboard: hwnd=%x msg=%d, wParam=%d, lParam=%d\n", 
	   target, msg, wParam, lParam);
  }
  if(wParam == 0) return 1;
  return PostMessage(target, msg, wParam, lParam);
}

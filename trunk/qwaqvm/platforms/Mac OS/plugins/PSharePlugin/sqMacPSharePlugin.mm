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

#include <stdio.h>

#include "sqVirtualMachine.h"
#include "sqConfig.h"
#include "sqPlatformSpecific.h"

extern "C" {
#include "PSharePlugin.h"
};
#include <MacWindows.h>
#include <Cocoa/Cocoa.h>
#include <ApplicationServices/ApplicationServices.h>

typedef int CGSConnection;
typedef int CGSWindow;
typedef int CGSValue;

extern "C" {
	OSStatus CGSGetWindowBounds(CGSConnection cid, int wid, CGRect *ret);
	void CGContextCopyWindowCaptureContentsToRect(void *grafport, CGRect rect, CGSConnection cid, int wid, int zero);
	CGSConnection _CGSDefaultConnection();
	extern OSStatus CGSGetWindowCount(const CGSConnection cid, CGSConnection target, int *count);
	extern OSStatus CGSGetWindowList(const CGSConnection cid, CGSConnection target, int listSize, int *list, int *numberOfWindows);
	extern OSStatus CGSGetOnScreenWindowCount(const CGSConnection cid, CGSConnection target, int *count);
	extern OSStatus CGSGetOnScreenWindowList(const CGSConnection cid, CGSConnection target, int listSize, int *list, int *numberOfWindows);
	extern OSStatus CGSGetWindowOwner(const CGSConnection cid, const CGSWindow wid, CGSConnection *ownerCid);
	extern OSStatus CGSConnectionGetPID(const CGSConnection cid, pid_t *pid, const CGSConnection ownerCid);
	extern OSStatus CGSGetWindowProperty(const CGSConnection cid, CGSWindow wid, CGSValue key, CGSValue *outValue);
	extern OSStatus CGSGetSharedWindow(const CGSConnection cid, CGSWindow wid, CGSWindow * sharedWid);
	extern OSStatus CGSGetWindowLevel(const CGSConnection cid, CGSWindow wid, long *level);
};


typedef CGSWindow (* getnativefp)(WindowRef window);
typedef WindowRef (* getwreffp)(CGSWindow window);

getnativefp MyGetNativeWindowFromWindowRef = 0;
getwreffp MyGetWindowRefFromNativeWindow = 0;

CGSValue windowTitleKeyConstant = 0;


/* Initialize personal share plugin */
int pShareInit(void) {
	NSApplicationLoad();
	
	CFBundleRef carbon = CFBundleGetBundleWithIdentifier ((CFStringRef) @"com.apple.Carbon");
	if (carbon)
	{
		MyGetNativeWindowFromWindowRef = (getnativefp) CFBundleGetFunctionPointerForName (carbon, (CFStringRef) @"GetNativeWindowFromWindowRef");
		if (MyGetNativeWindowFromWindowRef)
			NSLog(@"got GetNativeWindowFromWindowRef");
		else
			NSLog(@"no GetNativeWindowFromWindowRef");
		
		MyGetWindowRefFromNativeWindow = (getwreffp) CFBundleGetFunctionPointerForName (carbon, (CFStringRef) @"GetWindowRefFromNativeWindow");
		if (MyGetWindowRefFromNativeWindow)
			NSLog(@"got GetWindowRefFromNativeWindow");
		else
			NSLog(@"no GetWindowRefFromNativeWindow");
		
	}
	CFRelease(carbon);
	windowTitleKeyConstant = (int)CFStringCreateWithCStringNoCopy(kCFAllocatorDefault, "kCGSWindowTitle", kCFStringEncodingUTF8, kCFAllocatorNull);
	return 1;
}

int pShareShutdown(void) {
	return 1;
}

static int windowCount = 0;
static CGSWindow *windowCache = NULL;

static int updateWindowCache(void) {
	int wndCount = 0;
	int outputCount = 0;
	CGSConnection defaultConnection = _CGSDefaultConnection();
	OSStatus countError = CGSGetOnScreenWindowCount(defaultConnection, NULL, &wndCount);
	if (countError != 0) {
		return -1;
	}
	if(windowCount != wndCount) {
		windowCount = wndCount;
		if(windowCache) free(windowCache);
		windowCache = (CGSWindow*)malloc(sizeof(CGSWindow) * windowCount);
	}
	OSStatus listStatus = CGSGetOnScreenWindowList(defaultConnection, NULL, windowCount, windowCache, &outputCount);
	if (listStatus != 0) {
		return -1;
	}
	return windowCount;
}

/* Returns the window ID for the window with the given index (-1 on error) */
int pShareGetWindowID(int index) {
	if(index == 0) updateWindowCache();
	if(index >= 0 && index < windowCount) {
		return windowCache[index];
	}
	return -1;
}


/* Returns the icon for a document file */
int pShareGetDocumentIcon(char* fileName, int w, int h, int d, void* bits) {
	NSString *nsStringFilename = [NSString stringWithUTF8String:fileName];
	NSImage *image = [[NSWorkspace sharedWorkspace] iconForFile:nsStringFilename];

	if(image == nil) {
		/* fprintf(stderr, "pShareGetDocumentIcon: No icon found for %s\n", fileName); */
		return 0;
	}
	NSArray *allReps = [image representations];
	for(int i=0; i < [allReps count]; i++) {
		NSBitmapImageRep *imageRep = [allReps objectAtIndex: i];
		NSSize size = [imageRep size];
		if(size.width != w || size.height != h) {
			/* fprintf(stderr, "pShareGetDocumentIcon: Skipping image resolution %d x %d\n", (int)size.width, (int)size.height); */
			continue;
		}
		unsigned char *srcBits = [imageRep bitmapData];
		unsigned char *dstBits = (unsigned char *) bits;
		for (int p = 0; p < w * h; p++) {
			dstBits[(p * 4) + 0] = srcBits[(p * 4) + 2];
			dstBits[(p * 4) + 1] = srcBits[(p * 4) + 1];
			dstBits[(p * 4) + 2] = srcBits[(p * 4) + 0];
			dstBits[(p * 4) + 3] = srcBits[(p * 4) + 3];
		}
		return 1;
	}
	/* fprintf(stderr, "pShareGetDocumentIcon: No suitable icon found\n"); */
	return 0;
}

/* Returns the icon for the window with the given ID */
int pShareGetWindowIcon(int windowID, int w, int h, int d, void* bits) {
	pid_t windowPid = pShareGetWindowProcessID(windowID);

	NSWorkspace *workspace = [NSWorkspace sharedWorkspace];
	NSArray *launchedApplications = [workspace launchedApplications];
	NSString *appPath = nil;
	for(int i = 0; i < [launchedApplications count]; i++) {
		NSDictionary *appDict = [launchedApplications objectAtIndex: i];
		int appPid = [[appDict objectForKey:@"NSApplicationProcessIdentifier"] intValue];
		if (appPid == windowPid) {
			appPath = [appDict objectForKey:@"NSApplicationPath"];
			break;
		}
	}
	if(appPath == nil) return 0;
	NSImage *image = [workspace iconForFile: appPath];
	if(image == nil) return 0;
	NSArray *allReps = [image representations];
	for(int i=0; i < [allReps count]; i++) {
		NSBitmapImageRep *imageRep = [allReps objectAtIndex: i];
		NSSize size = [imageRep size];
		if(size.width != w || size.height != h) {
			/* fprintf(stderr, "pShareGetIcon: Skipping image resolution %d x %d\n", (int)size.width, (int)size.height); */
			continue;
		}
		unsigned char *srcBits = [imageRep bitmapData];
		unsigned char *dstBits = (unsigned char *) bits;
		for (int p = 0; p < w * h; p++) {
			dstBits[(p * 4) + 0] = srcBits[(p * 4) + 2];
			dstBits[(p * 4) + 1] = srcBits[(p * 4) + 1];
			dstBits[(p * 4) + 2] = srcBits[(p * 4) + 0];
			dstBits[(p * 4) + 3] = srcBits[(p * 4) + 3];
		}
		return 1;
	}
	/* fprintf(stderr, "pShareGetIcon: No suitable icon found\n"); */
	return 0;
}

/* Returns the bitmap for the window with the given ID */
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

int *pShareGetWindowBitmap(int windowID, int w, int h, int d, void* bits, int diff) {
	static int rect[4];

	/* TODO: Figure out how to recycle the bits more effectively */
	int *macBits = (int*) malloc(w*h*4);
	if(!macBits) return NULL;

	CGSConnection defaultConnection = _CGSDefaultConnection();
	CGBitmapInfo bitmapInfo = (kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little);
	CGColorSpaceRef genericRGBColorSpace = CGColorSpaceCreateDeviceRGB();
	CGContextRef cgContext = CGBitmapContextCreate(macBits, w, h, 8, 4 * w, genericRGBColorSpace, bitmapInfo);

    CGRect cgrect;
    cgrect.origin = CGPointZero;
	cgrect.size.width = w;
	cgrect.size.height = h;
	CGContextCopyWindowCaptureContentsToRect(cgContext, cgrect, defaultConnection, windowID, 0);
	CFRelease(cgContext);
	CFRelease(genericRGBColorSpace);

	/* TODO: Figure out which 'other' windows should be copied on top
	   of the actual display bits (dialogs etc) */

	rect[0] = 0;
	rect[1] = 0;
	rect[2] = w;
	rect[3] = h;

	if(diff) {
		diffBits(macBits, (int*)bits, w, h, rect);
	}
	if(rect[2] > rect[0]) {
		/* If not empty, copy the bits. TODO: This should do a partial copy for diffs */
		memcpy(bits, macBits, w*h*4);
	}
	free(macBits);
	return rect;
}

/* Returns the label for the window with the given ID */
char *pShareGetWindowLabel(int windowID) {
	static char labelString[1000];
	CGSConnection defaultConnection = _CGSDefaultConnection();
	CGSValue title = NULL;
	
	OSStatus titleError = CGSGetWindowProperty(defaultConnection, windowID, windowTitleKeyConstant, &title);
	if (titleError != 0 || title == 0) {
		if (MyGetWindowRefFromNativeWindow != 0) {
			WindowRef ref = MyGetWindowRefFromNativeWindow(windowID);
			CFStringRef stringTitle = 0;
			OSStatus wrefTitleError = CopyWindowTitleAsCFString(ref, &stringTitle);
			if (wrefTitleError != 0 || title == 0) return NULL;
			title = (CGSValue) stringTitle;
		} else  {
			return NULL;
		}
	}

	NSString *stringTitle = (NSString *)title;
	strcpy(labelString, [stringTitle UTF8String]);
	return labelString;
}

/* Returns the app name for the window with the given ID */
char *pShareGetWindowAppName(int windowID) {
	static char appString[1000];
	pid_t windowPid = pShareGetWindowProcessID(windowID);

	NSWorkspace *workspace = [NSWorkspace sharedWorkspace];
	NSArray *launchedApplications = [workspace launchedApplications];
	NSString *appName = nil;
	for(int i = 0; i < [launchedApplications count]; i++) {
		NSDictionary *appDict = (NSDictionary*) [launchedApplications objectAtIndex: i];
		int appPid = [[appDict objectForKey:@"NSApplicationProcessIdentifier"] intValue];
		if (appPid == windowPid) {
			appName = [appDict objectForKey:@"NSApplicationName"];
			break;
		}
	}
	if(appName == nil) return NULL;
	strcpy(appString, [appName UTF8String]);
	return appString;
}

/* Returns the bounding box for the window with the given ID */
int *pShareGetWindowRect(int windowID) {
	static int rect[4];

	CGSConnection defaultConnection = _CGSDefaultConnection();
    CGRect cgrect;
    if(CGSGetWindowBounds(defaultConnection, windowID, &cgrect) != 0) return NULL;
	rect[0] = cgrect.origin.x;
	rect[1] = cgrect.origin.y;
	rect[2] = rect[0] + cgrect.size.width;
	rect[3] = rect[1] + cgrect.size.height;
	return rect;
}

/* Enables/disables sharing of a particular window */
int pShareActivateWindow(int id, int shareOn) { return 1; }

/* If a window is not iconic arrange that it is unobscured so that
 * pShareGetWindowBitmap can access an unobscured image of the window's
 * contents.  Depending on the window manager this may require raising the
 * window to the top of the stacking order.
 * On the Mac, whih has a sensible window manager where every window writes to
 * backing store, window contents aren't obscured by others.
 */
int pShareUnobscureWindow(int id) { return 1; }

/* Return the pid for a given window -1 on error */
int pShareGetWindowProcessID(int windowID) {

	CGSConnection defaultConnection = _CGSDefaultConnection();
	CGSConnection ownerConnection;
	OSStatus ownerStatus = CGSGetWindowOwner(defaultConnection, windowID, &ownerConnection);

	if (ownerStatus != 0) {
		fprintf(stderr, "pShareGetWindowProcessID: ownerStatus = %d\n", ownerStatus);
		return -1;
	}

	pid_t windowPid;
	OSStatus connectionStatus = CGSConnectionGetPID(ownerConnection, &windowPid, ownerConnection);
	if (connectionStatus != 0) {
		fprintf(stderr, "pShareGetWindowProcessID: connectionStatus = %d\n", connectionStatus);
		return -1;
	}
	return windowPid;
}

/* Return the owner id for a given window, -1 on error */
int pShareGetWindowOwner(int windowID) {
#if 0
	CGSConnection defaultConnection = _CGSDefaultConnection();
	CGSConnection ownerConnection;
	OSStatus ownerStatus = CGSGetWindowOwner(defaultConnection, windowID, &ownerConnection);
	fprintf(stderr, "ownerConnection for %d: %d\n", windowID, ownerConnection);
#endif
	return -1;
}

/* Return true/false/-1 if the window is minimized */
int pShareIsWindowMinimized(int windowID) {
	return -1;
}


/* Return true/false/-1 if the window is visible */
int pShareIsWindowVisible(int id) {
	/* TODO: Figure out for real whether window is visible or not */
	return 1;
}

/* Return true/false if the window is valid */
int pShareIsWindowValid(int id) {
	/* TODO: There's *got* to be a better way to do that... */
	updateWindowCache();
	for(int i=0; i < windowCount; i++) {
		if(windowCache[i] == id) return 1;
	}
	return 0;
}

/* Return true/false/-1 if the window should be shared */
int pShareIsWindowSharable(int windowID) {
	long level = -1;
	CGSConnection defaultConnection = _CGSDefaultConnection();
	CGSGetWindowLevel(defaultConnection, windowID, &level);
	if(level != 0) return 0; /* we don't share anything but first-class windows */
	/* TODO: Figure out additional rules, if any */
	return 1;
}

/* Launch a random document file. Return:
   -1: if the launch fails
    0: if the launch succeeds but we have no pid.
   pid: otherwise.
*/
int pShareLaunchFileOfSize(void *utf8Ptr, int utf8Len) {
	char buffer[1024];
	memcpy(buffer, utf8Ptr, utf8Len);
	buffer[utf8Len] = 0;
	NSString *nsFilename = [NSString stringWithUTF8String: buffer];
	NSWorkspace *workspace =[NSWorkspace sharedWorkspace];
	BOOL success = [workspace openFile: nsFilename];
	/* TODO: Figure out how to get the pid from the launch*/
	return success ? 0 : -1;
}

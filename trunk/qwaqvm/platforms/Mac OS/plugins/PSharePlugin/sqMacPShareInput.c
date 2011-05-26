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

/*
 *  Miramar-Input.cpp
 *  MiramarPlugin
 *
 */

#include <stdio.h>
#include "sqVirtualMachine.h"
#include "sqConfig.h"
#include "sqPlatformSpecific.h"
#include "PSharePlugin.h"

#include <MacWindows.h>
#include <Carbon/Carbon.h>


typedef int CGSConnection;
typedef int CGSWindow;
typedef int CGSValue;


#ifdef __cplusplus
extern "C" {
#endif
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
#ifdef __cplusplus
};
#endif

// These events were acquired by installing an event tap on TextEdit and saving the events as they got sent. 
// This was on 10.5.
// The events themselves are in network order and are mostly an anonymous bag of bits.
// There are certain offsets which correspond to things we want to change but have no API to change them with.


bool PSNForWindowID(int windowID, ProcessSerialNumber *psn) 
{	
	CGSConnection windowConnection;
	OSStatus ownerStatus = CGSGetWindowOwner(_CGSDefaultConnection(), windowID, &windowConnection);
	if (ownerStatus != 0)
	{
		return false;
	}
	
	pid_t windowPid;
	OSStatus connectionStatus = CGSConnectionGetPID(windowConnection, &windowPid, windowConnection);
	if (connectionStatus != 0)
	{
		return false;
	}
	
	OSStatus psnStatus = GetProcessForPID(windowPid, psn);
	if (psnStatus != 0)
	{
		return false;
	}
	
	return true;
}

bool PostActivateEventForWindowID(int windowID)
{
	static unsigned char appKitActivateEvent[] = { 0, 0, 0, 2, 0, 1, 64, 53, 0, 0, 0, 3, 0, 1, 64, 54, 0, 0, 0, 0, 0, 1, 64, 55, 0, 0, 0, 13, 0, 2, 192, 56, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 192, 57, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 58, 45, 155, 228, 192, 0, 0, 0, 215, 0, 1, 64, 59, 0, 0, 0, 0, 0, 1, 64, 51, 0, 0, 1, 64, 0, 1, 64, 52, 0, 0, 0, 0, 0, 1, 64, 106, 0, 0, 2, 201, 0, 1, 64, 107, 0, 0, 3, 132, 0, 1, 64, 83, 0, 0, 0, 9, 0, 15, 64, 84, 0, 0, 0, 67, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };
		
	int newwindowID = windowID;
	newwindowID = CFSwapInt32(windowID);
	
	UInt8 *windowIDBytes = (UInt8*)&newwindowID;
	
	int i;
	for (i = 0; i < 4; i++)
	{
		appKitActivateEvent[76 + i] = windowIDBytes[i];
	}
	
	CFDataRef data = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, appKitActivateEvent, sizeof(appKitActivateEvent), kCFAllocatorNull);
	CGEventRef newEvent = CGEventCreateFromData(NULL, data);
	CFRelease(data);
	
	ProcessSerialNumber psn;
	bool success = PSNForWindowID(windowID, &psn);
	
	if (success)
		CGEventPostToPSN(&psn, newEvent);
	
	CFRelease(newEvent);
	return success;
}

bool ModifyAndPostMouseEvent(unsigned char *theEvent, int windowID, float wx, float wy)
{			
	int newwx = *(int*)&wx;
	newwx = CFSwapInt32(newwx);
	int newwy = *(int*)&wy;
	newwy = CFSwapInt32(newwy);
	
	UInt8 *wxBytes = (UInt8*)&newwx;
	UInt8 *wyBytes = (UInt8*)&newwy;
	
	int newwindowID = windowID;
	newwindowID = CFSwapInt32(windowID);
	
	UInt8 *windowIDBytes = (UInt8*)&newwindowID;
	
	int i;
	for (i = 0; i < 4; i++)
	{
		theEvent[44 + i] = wxBytes[i];
		theEvent[48 + i] = wyBytes[i];
		theEvent[76 + i] = windowIDBytes[i];
		theEvent[188 + i] = windowIDBytes[i];
		theEvent[196 + i] = windowIDBytes[i];
	}
		
	// This size - 208- is the size of the only valid events to be passed into this method.
	CFDataRef eventData = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, theEvent, 208, kCFAllocatorNull);
	CGEventRef eventRef = CGEventCreateFromData(NULL, eventData);
	CFRelease(eventData);
	if (eventRef == 0)
	{
		fprintf(stderr, "event ref is null\n");
	}
	
	ProcessSerialNumber psn;
	bool success = PSNForWindowID(windowID, &psn);
	
	if (success) 
	{
		CGEventPostToPSN(&psn, eventRef);
	}
	
	CFRelease(eventRef);
	return success;
}

bool PostMouseDownForWindowID(int windowID, float wx, float wy)
{
	static unsigned char mouseDownEvent[] = { 0, 0, 0, 2, 0, 1, 64, 53, 0, 0, 0, 3, 0, 1, 64, 54, 0, 0, 0, 0, 0, 1, 64, 55, 0, 0, 0, 1, 0, 2, 192, 56, 68, 135, 192, 0, 67, 179, 0, 0, 0, 2, 192, 57, 68, 36, 64, 0, 67, 164, 0, 0, 0, 1, 0, 58, 45, 150, 184, 136, 0, 0, 0, 215, 0, 1, 64, 59, 0, 0, 1, 0, 0, 1, 64, 51, 0, 0, 1, 64, 0, 1, 64, 52, 0, 0, 254, 187, 0, 1, 64, 106, 0, 0, 2, 201, 0, 1, 64, 107, 0, 0, 3, 132, 0, 1, 64, 7, 0, 0, 0, 0, 0, 1, 64, 0, 0, 0, 0, 132, 0, 1, 64, 1, 0, 0, 0, 1, 0, 1, 64, 2, 0, 0, 0, 255, 0, 1, 64, 3, 0, 0, 0, 0, 0, 1, 64, 4, 0, 0, 0, 0, 0, 1, 64, 5, 0, 0, 0, 0, 0, 1, 64, 6, 0, 0, 0, 0, 0, 1, 64, 89, 0, 0, 0, 0, 0, 1, 64, 90, 0, 0, 0, 0, 0, 1, 64, 91, 0, 0, 1, 64, 0, 1, 64, 92, 0, 0, 1, 64, 0, 1, 64, 108, 0, 0, 0, 1, };

	return ModifyAndPostMouseEvent(mouseDownEvent, windowID, wx, wy);
}

bool PostMouseUpForWindowID(int windowID, float wx, float wy)
{
	static unsigned char mouseUpEvent[] = { 0, 0, 0, 2, 0, 1, 64, 53, 0, 0, 0, 3, 0, 1, 64, 54, 0, 0, 0, 0, 0, 1, 64, 55, 0, 0, 0, 2, 0, 2, 192, 56, 68, 135, 192, 0, 67, 179, 0, 0, 0, 2, 192, 57, 68, 36, 64, 0, 67, 164, 0, 0, 0, 1, 0, 58, 49, 224, 182, 120, 0, 0, 0, 215, 0, 1, 64, 59, 0, 0, 1, 0, 0, 1, 64, 51, 0, 0, 1, 64, 0, 1, 64, 52, 0, 0, 254, 187, 0, 1, 64, 106, 0, 0, 2, 201, 0, 1, 64, 107, 0, 0, 3, 132, 0, 1, 64, 7, 0, 0, 0, 0, 0, 1, 64, 0, 0, 0, 0, 132, 0, 1, 64, 1, 0, 0, 0, 1, 0, 1, 64, 2, 0, 0, 0, 0, 0, 1, 64, 3, 0, 0, 0, 0, 0, 1, 64, 4, 0, 0, 0, 0, 0, 1, 64, 5, 0, 0, 0, 0, 0, 1, 64, 6, 0, 0, 0, 0, 0, 1, 64, 89, 0, 0, 0, 0, 0, 1, 64, 90, 0, 0, 0, 0, 0, 1, 64, 91, 0, 0, 1, 64, 0, 1, 64, 92, 0, 0, 1, 64, 0, 1, 64, 108, 0, 0, 0, 0, };	

	return ModifyAndPostMouseEvent(mouseUpEvent, windowID, wx, wy);
}

bool PostMouseMoveForWindowID(int windowID, float wx, float wy)
{
	static unsigned char mouseMoveEvent[] = { 0, 0, 0, 2, 0, 1, 64, 53, 0, 0, 0, 3, 0, 1, 64, 54, 0, 0, 0, 0, 0, 1, 64, 55, 0, 0, 0, 5, 0, 2, 192, 56, 68, 153, 64, 0, 68, 25, 0, 0, 0, 2, 192, 57, 67, 151, 0, 0, 66, 166, 0, 0, 0, 1, 0, 58, 83, 219, 96, 72, 0, 0, 168, 107, 0, 1, 64, 59, 0, 0, 1, 0, 0, 1, 64, 51, 0, 0, 19, 42, 0, 1, 64, 52, 0, 1, 16, 239, 0, 1, 64, 106, 0, 0, 1, 186, 0, 1, 64, 107, 0, 0, 4, 176, 0, 1, 64, 7, 0, 0, 0, 0, 0, 1, 64, 0, 0, 0, 37, 92, 0, 1, 64, 1, 0, 0, 0, 1, 0, 1, 64, 2, 0, 0, 0, 0, 0, 1, 64, 3, 0, 0, 0, 0, 0, 1, 64, 4, 0, 0, 255, 255, 0, 1, 64, 5, 0, 0, 0, 1, 0, 1, 64, 6, 0, 0, 0, 0, 0, 1, 64, 89, 0, 0, 0, 0, 0, 1, 64, 90, 0, 0, 0, 0, 0, 1, 64, 91, 0, 0, 19, 42, 0, 1, 64, 92, 0, 0, 19, 42, 0, 1, 64, 108, 0, 0, 0, 0, };
	
	return ModifyAndPostMouseEvent(mouseMoveEvent, windowID, wx, wy);
}

bool PostMouseDragForWindowID(int windowID, float wx, float wy)
{
	static unsigned char mouseDragEvent[] = { 0, 0, 0, 2, 0, 1, 64, 53, 0, 0, 0, 3, 0, 1, 64, 54, 0, 0, 0, 0, 0, 1, 64, 55, 0, 0, 0, 6, 0, 2, 192, 56, 67, 94, 0, 0, 67, 85, 0, 0, 0, 2, 192, 57, 67, 47, 0, 0, 67, 7, 0, 0, 0, 1, 0, 58, 21, 137, 162, 120, 0, 0, 40, 25, 0, 1, 64, 59, 0, 0, 1, 0, 0, 1, 64, 51, 0, 0, 11, 239, 0, 1, 64, 52, 0, 1, 37, 119, 0, 1, 64, 106, 0, 0, 1, 186, 0, 1, 64, 107, 0, 0, 4, 176, 0, 1, 64, 7, 0, 0, 0, 0, 0, 1, 64, 0, 0, 0, 12, 21, 0, 1, 64, 1, 0, 0, 0, 1, 0, 1, 64, 2, 0, 0, 0, 255, 0, 1, 64, 3, 0, 0, 0, 0, 0, 1, 64, 4, 0, 0, 0, 0, 0, 1, 64, 5, 0, 0, 0, 3, 0, 1, 64, 6, 0, 0, 0, 0, 0, 1, 64, 89, 0, 0, 0, 0, 0, 1, 64, 90, 0, 0, 0, 0, 0, 1, 64, 91, 0, 0, 11, 239, 0, 1, 64, 92, 0, 0, 11, 239, 0, 1, 64, 108, 0, 0, 0, 1, };
	
	return ModifyAndPostMouseEvent(mouseDragEvent, windowID, wx, wy);
}
	
void ActivateWindowID(int windowID)
{
	PostActivateEventForWindowID(windowID);
	PostMouseDownForWindowID(windowID, -10.0f, 0.0f);
	PostMouseUpForWindowID(windowID, -10.0f, 0.0f);
}

/* Post a mouse event to a window */
int pSharePostMouseEvent(int windowID, int type, int x, int y, int buttons) {
	float wx = (float)x;
	float wy = (float)y;
	bool result = false;

	fprintf(stderr, "pSharePostMouseEvent: wid=%d, type=%d, x=%d, y=%d, buttons=%d\n", windowID, type, x, y, buttons);

	switch(type) {
		case 0: /* move event */
			if (buttons & 7) result = PostMouseDragForWindowID(windowID, wx, wy);
			else result = PostMouseMoveForWindowID(windowID, wx, wy);
			break;

		case 1: /* right button down */
		case 3: /* middle button down */
		case 5: /* left button down */
			ActivateWindowID(windowID);
			result = PostMouseDownForWindowID(windowID, wx, wy);
			break;

		case 2: /* right button up */
		case 4: /* middle button up */
		case 6: /* left button up */
			result = PostMouseUpForWindowID(windowID, wx, wy);
			break;

		default:
			fprintf(stderr, "pSharePostMouseEvent: Unknown mouse event type %d\n", type);
	}
	
	if (!result) {
		fprintf(stderr, "pSharePostMouseEvent: delivery failed for %d - %d x %d\n", type, x, y);
	}
	return result;
}

int pSharePostKeyboardEvent(int windowID, int type, int x, int y, int keyCode, int btns) {
	ActivateWindowID(windowID);
	ProcessSerialNumber psn;
	bool success = PSNForWindowID(windowID, &psn);
	if (!success) {
		fprintf(stderr, "pSharePostKeyboardEvent: PSNForWindowID failed\n");
		return 0;
	}

	switch(type) {
		case 1: { /* key down */
			CGEventRef keyDownEvent = CGEventCreateKeyboardEvent(NULL, (CGKeyCode)keyCode, true);
			CGEventPostToPSN(&psn, keyDownEvent);
			CFRelease(keyDownEvent);
			break;
		}
		case 2: { /* key stroke */
			CGEventRef keyDownEvent = CGEventCreateKeyboardEvent(NULL, (CGKeyCode)keyCode, true);
			CGEventRef keyUpEvent = CGEventCreateKeyboardEvent(NULL, (CGKeyCode)keyCode, false);
			CGEventPostToPSN(&psn, keyDownEvent);
			CGEventPostToPSN(&psn, keyUpEvent);
			CFRelease(keyDownEvent);
			CFRelease(keyUpEvent);
			break;
		}
		case 3: { /* key up */
			CGEventRef keyUpEvent = CGEventCreateKeyboardEvent(NULL, (CGKeyCode)keyCode, false);
			CGEventPostToPSN(&psn, keyUpEvent);
			CFRelease(keyUpEvent);
			break;
		}
		default:
			fprintf(stderr, "pSharePostKeyboardEvent: Unknown event type %d\n", type);
			return 0;
	}
	return 1;
}


/* Stub for linkability. */
int pShareEnableDebug (int foo) {return 0;}


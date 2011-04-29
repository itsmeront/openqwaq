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

/*
 *  PShaqrePlugin implementation for linux/X11.
 *
 * Contains portions from xwd.c
 *	Copyright 1987, 1998  The Open Group
 *
 * Author: Eliot Miranda
 */

#include <sys/param.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include "sqAssert.h"
#include "PSharePlugin.h"

/* from PSharePlugin.c */
extern struct VirtualMachine* interpreterProxy;

/* from vm-display-X11 */
extern Display     *stDisplay;   /* Squeak display */


/* private state */
#define WIN_CACHE_INCR 256
static Window *windowCache;
static unsigned int nCachedWindows, windowCacheSize;

static unsigned long *bitmapCache;
static unsigned long  bitmapCacheSize;

static	Atom _NET_WM_ICON = None;
static	Atom _NET_WM_PID = None;

static	Atom _NET_ACTIVE_WINDOW = None;

static	Atom _NET_WM_WINDOW_TYPE = None;
static  Atom _NET_WM_WINDOW_TYPE_MENU = None;
static  Atom _NET_WM_WINDOW_TYPE_UTILITY = None;
static  Atom _NET_WM_WINDOW_TYPE_DIALOG = None;
static  Atom _NET_WM_WINDOW_TYPE_DROPDOWN_MENU = None;
static  Atom _NET_WM_WINDOW_TYPE_POPUP_MENU = None;
static  Atom _NET_WM_WINDOW_TYPE_TOOLTIP = None;
static  Atom _NET_WM_WINDOW_TYPE_NOTIFICATION = None;
static  Atom _NET_WM_WINDOW_TYPE_COMBO = None;
static	Atom _NET_WM_WINDOW_TYPE_NORMAL = None;

/* pShareInit:
   Initialize personal share plugin.
*/
int
pShareInit(void)
{
#define internOrFail(a) if ((a = XInternAtom(stDisplay, #a, False)) == None) \
							return 0

	internOrFail(_NET_WM_ICON);
	internOrFail(_NET_WM_PID);

	internOrFail(_NET_ACTIVE_WINDOW);

	internOrFail(_NET_WM_WINDOW_TYPE);
	internOrFail(_NET_WM_WINDOW_TYPE_MENU);
	internOrFail(_NET_WM_WINDOW_TYPE_UTILITY);
	internOrFail(_NET_WM_WINDOW_TYPE_DIALOG);
	internOrFail(_NET_WM_WINDOW_TYPE_DROPDOWN_MENU);
	internOrFail(_NET_WM_WINDOW_TYPE_POPUP_MENU);
	internOrFail(_NET_WM_WINDOW_TYPE_TOOLTIP);
	internOrFail(_NET_WM_WINDOW_TYPE_NOTIFICATION);
	internOrFail(_NET_WM_WINDOW_TYPE_COMBO);
	internOrFail(_NET_WM_WINDOW_TYPE_NORMAL);

#undef internOrFail

	return 1;
}


/* pShareShutdown:
   Clean out the personal share plugin.
*/
int
pShareShutdown(void)
{
	if (windowCache) {
		free(windowCache);
		nCachedWindows = windowCacheSize = 0;
	}
	return 0;
}

/* UNIMPLEMENTED */
int
pShareEnableDebug(int debug) { return debug; }

static void
cacheWindow(Window w)
{
	if (nCachedWindows >= windowCacheSize) {
		windowCacheSize += WIN_CACHE_INCR;
		if (!(windowCache = realloc(windowCache,
									windowCacheSize * sizeof(Window *))))
			error("PSharePlugin cacheWindow out of memory");
	}
	windowCache[nCachedWindows++] = w;
}

static int isSharableWindowType(Window w);

static int
cacheChildrenOf(Window w)
{
	Window root, parent, *children;
	unsigned int nwindows, i;

	if (isSharableWindowType(w))
		cacheWindow(w);

	/* XQueryTree is ill-named.  It is really XQueryChildren, answering only
	 * immediate descendents of the second argument.
	 */
	if (!XQueryTree(stDisplay, w, &root, &parent, &children, &nwindows))
		return 0;

	for (i = 0; i < nwindows; i++)
		if (!cacheChildrenOf(children[i])) {
			XFree(children);
			return 0;
		}

	XFree(children);
	return 1;
}

static int
cacheAllWindows()
{
	nCachedWindows = 0;
	return cacheChildrenOf(DefaultRootWindow(stDisplay));
}

int
pShareGetWindowID(int index)
{
	if (index == 0
	 && !cacheAllWindows())
		return -1;

	return index >= 0 && index < nCachedWindows
			? windowCache[index]
			: -1;
}


/* pShareIsWindowValid:
   Return true/false if the window is valid.
*/
int
pShareIsWindowValid(int id)
{
	XWindowAttributes attrs;

	return XGetWindowAttributes(stDisplay, (Window)id, &attrs);
}


/* pShareIsWindowVisible:
   Return true/false/-1 if the window is visible
*/
int
pShareIsWindowVisible(int id)
{
	XWindowAttributes attrs;

	if (!XGetWindowAttributes(stDisplay, (Window)id, &attrs))
		return -1;

	return attrs.map_state == IsViewable;
}


/* pShareIsWindowMinimized:
   Return true/false/-1 if the window is minimized
*/
int
pShareIsWindowMinimized(int id)
{
	XWindowAttributes attrs;

	if (!XGetWindowAttributes(stDisplay, (Window)id, &attrs))
		return -1;

	return attrs.map_state == IsUnmapped
		|| attrs.map_state == IsUnviewable; /* i.e. some ancestor IsUnMapped */
}


/* pShareGetWindowRect:
   Returns the bounding box for the window with the given ID (including chome). 

	c.f. display_ioPositionOfNativeWindow & display_ioSizeOfNativeWindow
	in vm-display-X11/sqUnixX11.c
*/
int *
pShareGetWindowRect(int id)
{
static	int rect[4];
		int real_border_width;
		XWindowAttributes attrs;
		Window neglected_child;
		int rootx, rooty;

	if (!XGetWindowAttributes(stDisplay, (Window)id, &attrs)
	 || attrs.map_state == IsUnmapped || attrs.map_state == IsUnviewable
	 || !XTranslateCoordinates(stDisplay, (Window)id, attrs.root,
							   -attrs.border_width, -attrs.border_width,
							   &rootx, &rooty, &neglected_child))
		return 0;

    /* At least under Gnome a window's border width in its attributes is zero
     * but the relative position of the left-hand edge is the actual border
     * width.
     */
    real_border_width = attrs.border_width ? attrs.border_width : attrs.x;
#define OriginX 0
#define OriginY 1
#define CornerX 2
#define CornerY 3
	rect[OriginX] = rootx == attrs.x ? rootx : rootx - attrs.x;
	rect[OriginY] = rooty == attrs.y ? rooty : rooty - attrs.y;
	rect[CornerX] = rect[OriginX] + attrs.width + 2 * real_border_width;
	rect[CornerY] = rect[OriginY] + attrs.height + attrs.y + real_border_width;
    return rect;
}


char *
pShareGetWindowLabel(int id)
{
  XTextProperty win_text;
#define MAX_LABEL 256
  static char label[MAX_LABEL+1];

	if (!XGetWMName(stDisplay, (Window)id, &win_text)) {
		char *win_name;
		if (!XFetchName(stDisplay, (Window)id, &win_name))
			return 0;
		strncpy(label, win_name, MAX_LABEL);
		label[MAX_LABEL] = 0;
		(void)XFree(win_name);
		return label;
	}
	/* If there are multiple items and we need to support that see use of
	 * XmbTextPropertyToTextList in xwininfo.
	 * If UTF8 is required see stringprep_locale_to_utf8 & libidn.
	 */
	if (win_text.nitems <= 0)
		return 0;
	strncpy(label, win_text.value, MAX_LABEL);
	label[MAX_LABEL] = 0;
	(void)XFree(win_text.value);
	return label;
}

/* pShareGetWindowProcessID:
   Return the pid for a given window -1 on error. 
*/
int
pShareGetWindowProcessID(int id)
{
	Atom type;
	int format;
	unsigned long nitems, bytes_after;
	unsigned char *prop;
	pid_t pid;

	/* N.B. _NET_WM_PID is a voluntary property set by an application. */
	if (Success != XGetWindowProperty(	stDisplay	/* display */,
										(Window)id	/* w */,
										_NET_WM_PID	/* property */,
										0			/* long_offset */,
										1			/* long_length */,
										False		/* delete */,
										XA_CARDINAL	/* req_type */,
										&type		/* actual_type_return */,
										&format		/* actual_format_return */,
										&nitems		/* nitems_return */,
										&bytes_after/* bytes_after_return */,
										&prop))		/* prop_return */
		return -1;

	pid = *(pid_t *)prop;
	(void)XFree(prop);
	return pid;
}


/* Answer the _NET_WM_WINDOW_TYPE of a window, used to decide if a window should
 * be enumerated and/or shared.
 */
static int
wmWindowType(Window w)
{
	Atom type;
	int format;
	unsigned long nitems, bytes_after;
	unsigned char *prop;

	if (Success != XGetWindowProperty(	stDisplay	/* display */,
										w			/* w */,
										_NET_WM_WINDOW_TYPE	/* property */,
										0			/* long_offset */,
										1			/* long_length */,
										False		/* delete */,
										XA_ATOM		/* req_type */,
										&type		/* actual_type_return */,
										&format		/* actual_format_return */,
										&nitems		/* nitems_return */,
										&bytes_after/* bytes_after_return */,
										&prop))		/* prop_return */
		return -1;

	if (prop) {
		assert(nitems == 1);
		type = *(Atom *)prop;
		(void)XFree(prop);
	}
	else {
		assert(nitems == 0);
		type = -1;
	}
	return type;
}

/* Answer if a window has _NET_WM_WINDOW_TYPE other than those that refer to
 * window manager or transitory state.
 * N.B. _NET_WM_WINDOW_TYPE is a voluntary property set by an app.
 */
static int
isSharableWindowType(Window w)
{
	Atom type = wmWindowType(w);

	/* See http://standards.freedesktop.org/wm-spec/wm-spec-1.4.html#id2551529
	 * We exclude
	 *		_NET_WM_WINDOW_TYPE_DESKTOP
	 *		_NET_WM_WINDOW_TYPE_DOCK
	 *		_NET_WM_WINDOW_TYPE_TOOLBAR
	 *		_NET_WM_WINDOW_TYPE_SPLASH
	 *		_NET_WM_WINDOW_TYPE_DND
	 */

	return type == _NET_WM_WINDOW_TYPE_MENU
		|| type == _NET_WM_WINDOW_TYPE_UTILITY
		|| type == _NET_WM_WINDOW_TYPE_DIALOG
		|| type == _NET_WM_WINDOW_TYPE_DROPDOWN_MENU
		|| type == _NET_WM_WINDOW_TYPE_POPUP_MENU
		|| type == _NET_WM_WINDOW_TYPE_TOOLTIP
		|| type == _NET_WM_WINDOW_TYPE_NOTIFICATION
		|| type == _NET_WM_WINDOW_TYPE_COMBO
		|| type == _NET_WM_WINDOW_TYPE_NORMAL;
}


char *
pShareGetWindowAppName(int id)
{
		char exe_link_name[32];
static	char exe_name[PATH_MAX+1];
		int  exe_name_len;
		pid_t pid = pShareGetWindowProcessID(id);

	if ((int)pid == -1)
		return 0;

	sprintf(exe_link_name, "/proc/%d/exe", pid);
	if ((exe_name_len = readlink(exe_link_name, exe_name, PATH_MAX)) < 0)
		return 0;
	exe_name[exe_name_len] = 0;
	return exe_name;
}


/* pShareGetWindowOwner:
   Returns the first sharable 'owner' for a given window.
*/
int
pShareGetWindowOwner(int id)
{
	Window w = (Window)id;
	Window root, parent, *children;
	unsigned int nwindows;

	while (1) {
		if (!XQueryTree(stDisplay, w, &root, &parent, &children, &nwindows))
			return 0;

		XFree(children);

		if (root == parent)
			return 0;

		if (isSharableWindowType((int)parent))
			return parent;

		w = parent;
	}
	return 0;
}


int
pShareIsWindowSharable(int id)
{
	XWindowAttributes attrs;

	if (!isSharableWindowType(id))
		return 0;

	/* Fail for invalid handles */
	if (!XGetWindowAttributes(stDisplay, (Window)id, &attrs))
		return 0;

	/* Eliminate invisible windows */
	if (attrs.map_state != IsViewable)
		return 0;

	/* Eliminate zero-size windows */
	if (attrs.width == 0 || attrs.height == 0)
		return 0;

  return 1;
}


/* Search for an icon for the window with the given ID that matches the supplied
 * width and height and copy the icon bits into formBits.  Answer if the icon
 * was found.
 */
int
pShareGetWindowIcon(int id, int w, int h, int d, void* formBits)
{
	Atom type;
	int format;
	unsigned long nitems, bytes_after;
	unsigned long *prop;

	if (d != 32)
		return 0;

	if (Success != XGetWindowProperty(	stDisplay	/* display */,
										(Window)id	/* w */,
										_NET_WM_ICON/* property */,
										0			/* long_offset */,
										1			/* long_length */,
										False		/* delete */,
										XA_CARDINAL	/* req_type */,
										&type		/* actual_type_return */,
										&format		/* actual_format_return */,
										&nitems		/* nitems_return */,
										&bytes_after/* bytes_after_return */,
					(unsigned char **)  &prop))		/* prop_return */
		return 0;

	while (nitems > 2) {
		long npixels = prop[0] * prop[1];
		if (prop[0] == w && prop[1] == h) {
			memcpy(formBits, prop + 2, npixels * 4);
			(void)XFree(prop);
			return 0;
		}
		nitems -= npixels;
		prop += npixels;
	}
	(void)XFree(prop);
	return 0;
}

/* support for pShareGetWindowBitmap by Andreas, copied from sqWin32PShare.c */
static void
diffBits(int* srcBits, int *dstBits, int w, int h, int *rect) {
  int l,r,t,b,y;

  /* scan from the top */
  for(t = 0; t < h; t++) {
    if(memcmp(srcBits+(w*t), dstBits+(w*t), w*4) != 0) break;
  }
  if(t == h) {
    /* no changes at all */
    rect[OriginX] = rect[OriginY] = rect[CornerX] = rect[CornerY] = 0;
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
  rect[OriginX] = l;
  rect[OriginY] = t;
  rect[CornerX] = r+1;
  rect[CornerY] = b+1;
  return;
}

/* pShareGetWindowBitmap:
   Copy the bits of the specified window into bits, and answer a pointer to a
   rectangle that bounds the bits copied.  If incr is false copy the window's
   entire bitmap.  If incr is true, diff the bits in bits against the current
   window's contents, update the minimum bounding rectangle of differing bits,
   and answer that rectangle.

   We use a hack to compute the unobscured regions of a window.  On Mac there
   is no issue because windows display on backing store and so valid bits are
   unavailable.  On Win32 GDI provides COMPLEXREGION and ExcludeClipRect to
   compute a set of visible regions.  X11 provides the ability to set multiple
   clip regions (XSetClipRectangles) but not the ability to compute them and as
   there are about 14 cases of two rectangles overlapping the chance for error
   is high.

   A simpler, less efficient approach is as follows.  Grab the current contents
   of the window, possibly including bogus bits from overlapping windows.  Then
   enumerate windows higher in the stacking order and for each overlapping
   window fill the current contents with the corresponding region in bits, the
   previous contents of the window.  As we enumerate we can maintain the maximal
   visible rectangle, so that only a window that overlaps an entire edge reduces
   the vsible rectangle.  If this becomes empty the enumeration aborts. Once
   the enumeration completes we use diffBits to compute and update the maximal
   enclosing rectangle of visble bits.
*/
static int revertOverlappedRegions(Window,int *,XImage *,void *,int,int,int);
static Window topLevelWindowFor(Window w);

int*
pShareGetWindowBitmap(int id, int w, int h, int d, void* bits, int incr)
{
static	int     rect[4];
		int    *wr;
		XImage *im;
		Window root, parent, *children;
		unsigned int nkids;

	/* Note that pShareGetWindowRect includes the border & chrome and so is
	 * effectively the bounding box of the parent, and that XGetImage will fail
	 * if asked to go outside the bounds of its wndow argument.  Hence, given
	 * that we want to capture the border and chrome, we actually grab the bits
	 * of the parent wndow.
	 */
	if (!(wr = pShareGetWindowRect(id)))
		return 0;

	if (wr[CornerX] - wr[OriginX] != w
	 || wr[CornerY] - wr[OriginY] != h)
		return 0;

	if (!XQueryTree(stDisplay, (Window)id, &root, &parent, &children, &nkids))
		return 0;

	(void)XFree(children);

	/* XGetImage's result is defined only for unobscured regions, and while
	 * undefined for regions obscured by other windows will typically be the
	 * bits of those other windows.  A window may or may not have backing store
	 * (but how do we access it?) and if not, there is no way in X to get the
	 * contents of an obscured region.  So what to do?
	 *
	 * We take the same approach as the Windows code.  If doing an incremental
	 * update make sure to only show unobscured regions and to exclude obscured
	 * regions.  If not doing an incremental update then we could force the
	 * window to the front, but instead simply fail.  Bringing the window to the
	 * front is probably something better done in a separate primitive.
	 */
	if (!(im = XGetImage(stDisplay, parent, 0, 0, w, h, AllPlanes, ZPixmap)))
		return 0;

	if ((d != im->depth && d != im->bits_per_pixel)
	 || !revertOverlappedRegions(topLevelWindowFor(parent),wr,im,bits,w,h,d/8)) {
		XDestroyImage(im);
		return 0;
	}

#define BITS_PER_BYTE 8
	rect[OriginX] = rect[OriginY] = 0;
	rect[CornerX] = w; rect[CornerY] = h;
	if (incr && d == im->bits_per_pixel) {
		/* Copy the bits. TODO: This should do a partial copy for diffs */
		diffBits((int *)im->data, (int *)bits, w, h, rect);
		memcpy(bits, im->data, (w * d / BITS_PER_BYTE) * h);
	}
	else if (d == im->bits_per_pixel)
		memcpy(bits, im->data, (w * d / BITS_PER_BYTE) * h);
	else
		error("pShareGetWindowBitmap: copy 24 bpp to 32 bpp unimplemented");

	XDestroyImage(im);

	return rect;
}

/* fill any pixels in win's im obscured by other windows by the previous bits.
 * Answer whether any pixels in win's im are unobscured.
 */
static int
revertOverlappedRegions(Window win, int *wr,
						XImage *im, void *bits, int w, int h, int bpp)
{
	Window root, parent, peer, *children, *peers;
	unsigned int nkids, npeers, pi;
	int rect[4];

	if (!XQueryTree(stDisplay, win, &root, &parent, &children, &nkids))
		return 1;

	XFree(children);

	if (!XQueryTree(stDisplay, parent, &root, &parent, &peers, &npeers))
		return 1;

	memcpy(rect,wr,4 * sizeof(rect[0]));

	/* For each peer higher in the stacking order get its window region and if
	 * it overlaps copy the corresponding region from bits into im.
	 * Yes, children of other windows could overlap but we choose to punt on
	 * this.  Hopefully such children should be transient, and enumerating the
	 * entire window tree is tedious.
	 */
	for (pi = npeers - 1; pi >= 0 && (peer = peers[pi]) != win; pi--) {
		int i, x, y, r, b, offset, length, *pwr;

		if (!(pwr = pShareGetWindowRect((int)peer))) continue;

		/* See Rectangle>>intersects: */
		if (pwr[CornerX] <= rect[OriginX]) continue;
		if (pwr[CornerY] <= rect[OriginY]) continue;
		if (pwr[OriginX] >= rect[CornerX]) continue;
		if (pwr[OriginY] >= rect[CornerY]) continue;

		/* There's an intersection.  Set up the rectangle to blit /before/
		 * computing the maximally visible rectangle.
		 */
#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))
		x = max(pwr[OriginX] - rect[OriginX], 0);
		y = max(pwr[OriginY] - rect[OriginY], 0);
		r = min(pwr[CornerX], rect[CornerX]) - rect[OriginX];
		b = min(pwr[CornerY], rect[CornerY]) - rect[OriginY];

		if (pwr[OriginX] <= rect[OriginX] && pwr[CornerX] >= rect[CornerX]) {
			/* a complete overlap of top, bottom, middle or all of window */
			/* if top, bottom or all we can reduce the rectangle */
			if (pwr[OriginY] <= rect[OriginY]) {
				/* If the visible rectangle is void answer no visible bits */
				if (pwr[CornerY] >= rect[CornerY])
					return 0;
				assert(pwr[CornerY] > rect[OriginY]); /* established above */
				rect[OriginY] = pwr[CornerY];
			}
			if (pwr[CornerY] >= rect[CornerY]) {
				assert(pwr[OriginY] < rect[CornerY]); /* established above */
				rect[CornerY] = pwr[OriginY];
			}
		}
		if (pwr[OriginY] <= rect[OriginY] && pwr[CornerY] >= rect[CornerY]) {
			/* a complete overlap of left, right, middle or all of window */
			/* if left, right or all we can reduce the rectangle */
			if (pwr[OriginX] <= rect[OriginX]) {
				/* If the visible rectangle is void answer no visible bits */
				if (pwr[CornerX] >= rect[CornerX])
					return 0;
				assert(pwr[CornerX] > rect[OriginX]); /* established above */
				rect[OriginX] = pwr[CornerX];
			}
			if (pwr[CornerX] >= rect[CornerX]) {
				assert(pwr[OriginX] < rect[CornerX]); /* established above */
				rect[CornerX] = pwr[OriginX];
			}
		}
		/* If the visible rectangle is void answer no visible bits */
		if (rect[OriginX] > rect[CornerX] || rect[OriginY] > rect[CornerY])
			return 0;
		/* Can't use XCopyArea; it operates on Pixmaps. We could play with xpm
		 * but it's easier just to use memcpy.
		 */
		offset = x * bpp + im->bytes_per_line * y;
		length = (r - x) * bpp;
		for (i = y; i <= b; i++) {
			memcpy((char *)im->data + offset, (char *)bits + offset, length);
			offset += im->bytes_per_line;
		}
	}
	XFree(peers);
	return 1;
}

/* pShareActivateWindow:
   Enables/disables sharing of a particular window.
   For now, other than deiconifying if minimised, this is a no-op on X11
   since we retrieve the bits synchronously. If and when we change this to an
   asynchronous thread, we may use it.
*/
int
pShareActivateWindow(int id, int shareOn)
{
	int isMinimised = pShareIsWindowMinimized(id);

	if (isMinimised < 0)
		return 0;

	if (!shareOn)
		return 1;

	/* restore the window if it iconic */
	if (isMinimised)
		XMapWindow(stDisplay, (Window)id);

	return 1;
}

/* Answer the top-most window for the argument, i.e. the window on the root
 * screen.  This is for querying stacking order for the pShareUnobscureWindow
 * function.
 */
static Window
topLevelWindowFor(Window w)
{
	Window root, parent, *children;
	unsigned int nkids;

	if (!XQueryTree(stDisplay, w, &root, &parent, &children, &nkids))
		return 0;

	(void)XFree(children);

	return parent == root ? w : topLevelWindowFor(parent);
}

/* If a window is not iconic arrange that it is unobscured so that
 * pShareGetWindowBitmap can access an unobscured image of the window's
 * contents.  Depending on the window manager this may require raising the
 * window to the top of the stacking order.
 */
int
pShareUnobscureWindow(int id)
{
	Window topLevelWindow, root, parent, *topLevelWindows = 0;
	unsigned int numWindows;
	XEvent ev;

	if (pShareIsWindowMinimized(id))
		return 0;

	if (!(topLevelWindow = topLevelWindowFor((Window)id))
	 || !XQueryTree(stDisplay, DefaultRootWindow(stDisplay),
					&root, &parent, &topLevelWindows, &numWindows)
	 || numWindows <= 0
	 || !topLevelWindows)
		return -1;

	if (topLevelWindow == topLevelWindows[numWindows - 1])
		return 1;

	(void)XFree(topLevelWindows);

	/* See http://standards.freedesktop.org/wm-spec/1.3/ar01s03.html.
	 * XRaiseWindow is ignored by newer window managers such as Gnome.
	 * So a _NET_ACTIVE_WINDOW ClientEvent must also be sent to the root window.
	 */
	if (!XRaiseWindow(stDisplay, topLevelWindow))
		return -1;

	memset(&ev, 0, sizeof(ev));
	ev.xclient.type = ClientMessage;
	ev.xclient.send_event = True;
	ev.xclient.display = stDisplay;
	ev.xclient.window = topLevelWindow;
	ev.xclient.message_type = _NET_ACTIVE_WINDOW;
	ev.xclient.format = 32;
	ev.xclient.data.l[0] = 1; /* source indication, 1 => from an application */
	/* ev.xclient.data.l[1] = time stamp; should be _NET_WM_USER_TIME */
	/* ev.xclient.data.l[2] = stParent;  requestor current window; 0 if none */
	XSendEvent (stDisplay, root, False,
				SubstructureRedirectMask | SubstructureNotifyMask,
				&ev);

	XSync(stDisplay, False);

	return 1;
}

/* pShareGetDocumentIcon:
   Returns the icon for a particular document file.
*/
/* UNIMPLEMENTED (currently unimplementable?) */
int
pShareGetDocumentIcon(char *fileName, int w, int h, int d, void* formBits)
{ return 0; }

/* pShareLaunchFileOfSize:
   Launch a random document file. Return the pid. 
*/
/* UNIMPLEMENTED (currently unimplementable?) */
int
pShareLaunchFileOfSize(void *utf8Ptr, int utf8Len)
{ return -1; }

/* UNIMPLEMENTED (see XSendEvent) */
/* UNUSED delivering events to obscured windows is problematic on other plats */
int
pSharePostMouseEvent(int id, int type, int x, int y, int buttons)
{ return -1; }

/* UNIMPLEMENTED (see XSendEvent) */
/* UNUSED delivering events to obscured windows is problematic on other plats */
int
pSharePostKeyboardEvent(int id, int type, int x, int y, int key, int buttons)
{ return -1; }

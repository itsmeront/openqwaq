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

 /* Initialize personal share plugin */
int pShareInit(void);
int pShareShutdown(void);
int pShareEnableDebug(int);

/* Returns the window ID for the window with the given index (-1 on error) */
int pShareGetWindowID(int index);

/* Returns the icon for a document file */
int pShareGetDocumentIcon(char *fileName, int w, int h, int d, void* bits);

/* Returns the icon for the window with the given ID */
int pShareGetWindowIcon(int id, int w, int h, int d, void* bits);

/* Returns the bitmap for the window with the given ID */
int *pShareGetWindowBitmap(int id, int w, int h, int d, void* bits, int diff);

/* Returns the label for the window with the given ID */
char *pShareGetWindowLabel(int id);

/* Returns the app name for the window with the given ID */
char *pShareGetWindowAppName(int id);

/* Returns the bounding box for the window with the given ID */
int *pShareGetWindowRect(int id);

/* Enables/disables sharing of a particular window */
int pShareActivateWindow(int id, int shareOn);

/* Ensures a particular window is unobscured so pShareGetWindowBitmap can get valid bits. */
int pShareUnobscureWindow(int id);

/* Return the pid for a given window -1 on error */
int pShareGetWindowProcessID(int id);

/* Return the owner id for a given window, -1 on error */
int pShareGetWindowOwner(int id);

/* Return true/false/-1 if the window is visible */
int pShareIsWindowVisible(int id);

/* Return true/false/-1 if the window is minimized */
int pShareIsWindowMinimized(int id);

/* Return true/false if the window is valid */
int pShareIsWindowValid(int id);

/* Return true/false/-1 if the window should be shared */
int pShareIsWindowSharable(int id);

/* Launch a random document file. Return:
   -1: if the launch fails
    0: if the launch succeeds but we have no pid.
   pid: otherwise.
*/
int pShareLaunchFileOfSize(void *utf8Ptr, int utf8Len);

/* Post a mouse event to a window */
int pSharePostMouseEvent(int id, int type, int x, int y, int buttons);

/* Post a keyboard event to a window */
int pSharePostKeyboardEvent(int id, int type, int x, int y, int key, int btns);

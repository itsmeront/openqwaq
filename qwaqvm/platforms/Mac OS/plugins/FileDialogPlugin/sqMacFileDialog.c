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
 *  sqMacFileDialog.c
 *
 */
#include <Carbon/Carbon.h>

#include "sqVirtualMachine.h"
#include "sqConfig.h"
#include "sqPlatformSpecific.h"
#include "FileDialogPlugin.h"

extern struct VirtualMachine *interpreterProxy; /* signalSemaphoreWithIndex */

/* Only one dialog is supported here since we need callback support to keep things alive */
#define DLG_MAX 1
#define FILTER_MAX 256

typedef struct {
	int used;
	int doneSema;
	char *result;
	char filters[FILTER_MAX];
	NavDialogCreationOptions dco;
	NavDialogRef dlgRef;
} sqMacFileDialog;

static sqMacFileDialog allDialogs[DLG_MAX];

extern WindowPtr getSTWindow();

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

/* dlgFromHandle: Convert a dialog handle into a reference.
   Arguments:
     dlgHandle: Handle for the dialog.
   Return value: Dialog reference or NULL
*/
static sqMacFileDialog*  dlgFromHandle(int dlgHandle) {
  sqMacFileDialog *dlg;
  if(dlgHandle < 0 || dlgHandle >= DLG_MAX) return NULL;
  dlg = allDialogs + dlgHandle;
  if(dlg->used) return dlg;
  return NULL;
}

pascal Boolean MyFilterProc(AEDesc* item, void* info, 
                            NavCallBackUserData data,
                            NavFilterModes modes)
{
	NavFileOrFolderInfo* fileInfo = (NavFileOrFolderInfo*)info;
	sqMacFileDialog* dlg = (sqMacFileDialog *) data;
	char *patStart, *patEnd;
	char fileName[1024];
	int fileSize, delta;
	OSErr 	status;
	FSRef fsRef;

	if (! dlg) {
		printf("Error - filter callback from unknown file dialog\n");
		return 0;
	}
	
	if(dlg->filters[0] == 0) return 1; /* no filters */

	if(item->descriptorType == typeFSRef) {

		if(fileInfo->isFolder) return true;
		status = AEGetDescData(item,&fsRef,sizeof(FSRef));
        if(status != 0) return true;
		status = FSRefMakePath(&fsRef, fileName, 1023);
		fileSize = strlen(fileName);
		patStart = dlg->filters;
		while(*patStart) {
			while(*patStart == ';') patStart++;
			patEnd = patStart;
			while(*patEnd != 0 && *patEnd != ';') patEnd++;
			delta = patEnd-patStart;
			/* only match against *. patterns */
			if(delta >= 2 && patStart[0] == '*' && patStart[1] == '.') {
				/* special rule for *.* handling */
				if(delta == 3 && patStart[2] == '*') return 1;
				delta--; patStart++;
				if(delta <= fileSize && strncasecmp(patStart, fileName+fileSize-delta, delta) == 0)
					return 1;
			}
			patStart = patEnd;
		}
    }
    return 0;
}

/** Release the underlying dialog, and signal completion on its done-semaphore **/
void
disposeDialog (sqMacFileDialog* dlg) {
	if (dlg->dlgRef) {
		NavDialogDispose (dlg->dlgRef);
		dlg->dlgRef = 0;
	
		if(dlg->doneSema) {
			interpreterProxy->signalSemaphoreWithIndex(dlg->doneSema);
		}
	}
}


/** Copy the reply from the dialog into the dialog record **/
static void
getReply (sqMacFileDialog * dlg, NavUserAction action) {
	OSErr status = 0;
	NavReplyRecord reply;
	FSRef fsRef;
	
	if (! dlg->dlgRef) {
		return;
	}
	
	status = NavDialogGetReply(dlg->dlgRef, &reply);
	if(status == 0) {
		char result[1024];
		status = AEGetNthPtr(&(reply.selection), 1, typeFSRef, NULL, NULL, &fsRef, sizeof(FSRef), NULL);
		status = FSRefMakePath(&fsRef, result, 1023);
		if(status == 0 && (action == kNavUserActionSaveAs)) {
			/* Carbon quirk: PutFile leaves file name in saveFileName */
			char fileName[1024];
			fileName[0] = '/';
			CFStringGetCString(reply.saveFileName, fileName+1, 1022, 0);
			if(strlen(fileName) + strlen(result) >= 1023) status = -1;
			if(status == 0) strcat(result, fileName);
		}
		NavDisposeReply(&reply);
		
		if (status == 0) dlg->result = strdup(result);
	}
}

/*
 * Handling callback events for kNavCBUserAction events:
 * If the action is cancel or one of the variants of 'ok',
 * capture the result and discard the dialog.
 */
static void userAction (sqMacFileDialog * dlg, NavCBRecPtr params) {
	OSErr status = 0;
	NavReplyRecord reply;
	FSRef fsRef;
	NavUserAction action;
	
	action = NavDialogGetUserAction (dlg->dlgRef);
	
	switch (action) {
		case kNavUserActionCancel:
		case kNavUserActionOpen:
		case kNavUserActionSaveAs:
		case kNavUserActionChoose:   {
			getReply(dlg, action);
			disposeDialog(dlg);
			break;
		}
		default: 
			/* Ignore the other user actions (e.g. New Folder) */
			;
	}
}

/* 
 * The normal event callback.
 */
pascal void MyNavEventCallback(NavEventCallbackMessage selector, NavCBRecPtr params, void* data) {

	sqMacFileDialog * dlg  = (sqMacFileDialog*) data;
	NavReplyRecord reply;
	
	if (selector == kNavCBTerminate) {
		if (dlg && (dlg->dlgRef)) {
			printf("Warning - discarding a file dialog without prior user action.");
			disposeDialog (dlg);
		}
		return;
	}
	
	if (! dlg) {
		printf("Error - callback from unknown file dialog %d\n", selector);
		return;
	}

	if (dlg->dlgRef != params->context) {
		/* Consistency check failed - the user data is not intact */
		printf("Error - callback from unexpected file dialog %d\n", selector);
		return;
	}	

	/** User actions include (for instance) dismissing or OK'ing the dialog. **/
	if (selector == kNavCBUserAction) {
		userAction (dlg, params);
	}
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

/* fileDialogInitialize: Initialize file dialogs.
   Arguments: None.
   Return value: True if successful.
*/
int
fileDialogInitialize(void) {
	int i;
	for(i = 0; i < DLG_MAX; i++) {
		allDialogs[i].used = 0;
	}
	return 1;
}

/* fileDialogSetCallbackSemaphore: Set the callback semaphore for modal loops.
   Arguments:
	semaIndex - callback semaphore index.
   Return value: none.
*/
void
fileDialogSetCallbackSemaphore(int semaIndex) {
  /* -- ignored; Mac OS X doesn't need Smalltalk callbacks -- */
}

/* fileDialogCallbackReturn: Return from a previous callback.
   Return value: none.
*/
void
fileDialogCallbackReturn(void) {
  /* -- ignored; Mac OS X doesn't need Smalltalk callbacks -- */
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

/* fileDialogCreate: Create a new host dialog.
   Arguments: None.
   Return value: Dialog handle, or -1 on error.
*/
int
fileDialogCreate(void) {
  sqMacFileDialog *dlg;
  int i;

  for(i=0; i<DLG_MAX; i++) {
    if(!allDialogs[i].used) {
      dlg = allDialogs+i;
      memset(dlg, 0, sizeof(dlg));
      dlg->used = 1;
	  NavGetDefaultDialogCreationOptions(&dlg->dco);
	  /** The following make the dialog a modal rolldown dialog on the main window,
	   ** and allow the main window to continue processing the normal main loop
 	   ** while the dialog is present.
	   **/
	  dlg->dco.modality = kWindowModalityWindowModal;
	  dlg->dco.parentWindow = getSTWindow();
      return i;
    }
  }
  return -1;
}

/* fileDialogSetLabel: Set the label to be used for the dialog.
   Arguments:
     dlgHandle: Dialog handle.
     dlgLabel: Label for the dialog.
   Return value: None.
*/
void
fileDialogSetLabel(int dlgHandle, char* dlgLabel) {
	sqMacFileDialog *dlg = dlgFromHandle(dlgHandle);
	if(dlg) dlg->dco.windowTitle = CFStringCreateWithCString(NULL, dlgLabel, 0);
}

/* fileDialogSetFile: Set the initial file/path of the dialog.
   Arguments:
     dlgHandle: Dialog handle.
     filePath: Initial file path.
   Return value: None.

   On the mac, we only use the file name, to initialize the save file name
   for save file dialogs.  
*/
void
fileDialogSetFile(int dlgHandle, char* filePath) {
	sqMacFileDialog *dlg = dlgFromHandle(dlgHandle);
	if(dlg) {
		char* fname = strrchr(filePath, '/');
		if (fname) {
			fname++;
			if (! (*fname)) {
				fname = filePath;	/* Ended with slash - filePath is just a path. */
			}
		} else {
			fname = filePath;		/* No slash - filePath is just a name. */
		}
		dlg->dco.saveFileName = CFStringCreateWithCString(NULL, fname, 0);
	}
}

/* fileDialogAddFilter: Add a filter to the dialog.
   Arguments:
     dlgHandle: Dialog handle.
     filterDesc: Description of the filter ("Text files (*.txt)")
     filterPattern: Filter pattern ("*.txt")
   Return value: None.
*/
void
fileDialogAddFilter(int dlgHandle, char* filterDesc, char* filterPattern){
	sqMacFileDialog *dlg = dlgFromHandle(dlgHandle);
	if(dlg == NULL) return;
	if(strlen(dlg->filters) + strlen(filterPattern) + 1 < FILTER_MAX) {
		strcat(dlg->filters, ";");
		strcat(dlg->filters, filterPattern);
	}
}

/* fileDialogSetFilterIndex: Set the current filter.
   Arguments:
     dlgHandle: Dialog handle.
     index: Current filter index (1-based)
   Return value: None.
*/
void
fileDialogSetFilterIndex(int dlgHandle, int index) {
	/* -- ignored; the mac always sows all available files on open -- */
}

/* fileDialogGetFilterIndex: Set the current filter.
   Arguments:
     dlgHandle: Dialog handle.
   Return value: Current filter index (1-based)
*/
int
fileDialogGetFilterIndex(int dlgHandle) {
	/* -- ignored; the mac always uses the first filter index -- */
	return 1;
}

/* fileDialogDoneSemaphore: Set the semaphore to be signaled when done.
   Arguments:
     dlgHandle: Dialog handle.
     semaIndex: External semaphore index.
     Return value: None.
*/
void
fileDialogDoneSemaphore(int dlgHandle, int semaIndex) {
	sqMacFileDialog *dlg = dlgFromHandle(dlgHandle);
	if(dlg) dlg->doneSema = semaIndex;
}

/* fileDialogSetProperty: Set a boolean property.
   Arguments:
     dlgHandle: Dialog handle.
     propName: Name of the property (see below)
     propValue: Boolean value of property.
   Return value: True if the property is supported, false otherwise.
*/
int
fileDialogSetProperty(int dlgHandle, char* propName, int propValue) {
  return 0; /* no properties currently supported */
}

/* fileDialogShow: Show a file dialog.
   Arguments:
     dlgHandle: Dialog handle.
     fSaveAs: Display a "save as" dialog instead of an "open" dialog.
   Return value: True if successful, false otherwise.
*/
int
fileDialogShow(int dlgHandle, int fSaveAs) {
	sqMacFileDialog *dlg = dlgFromHandle(dlgHandle);
	NavEventUPP navEventPtr;
    	NavObjectFilterUPP filterProc;
	OSErr status = 0;

	if(!dlg) return 0;

	navEventPtr = NewNavEventUPP(MyNavEventCallback);
	filterProc =  NewNavObjectFilterUPP(MyFilterProc);

	if(fSaveAs) {
		status = NavCreatePutFileDialog(&dlg->dco, 0, 0, navEventPtr, dlg, &dlg->dlgRef);
	} else {
		status = NavCreateGetFileDialog(&dlg->dco, 0, navEventPtr, NULL, filterProc, dlg, &dlg->dlgRef);
	}
	if(status != 0) {
		printf("NavCreateGetFileDialog: error code %d (%x)\n", status, dlg->dlgRef);
		return 0;
	}
	status = NavDialogRun(dlg->dlgRef);
	DisposeNavEventUPP(navEventPtr);
	DisposeNavObjectFilterUPP(filterProc);
	if(status != 0) {
		printf("NavDialogRun: error code %d (%x)\n", status, dlg->dlgRef);
		return 0;
	}

	/** The dialog is running as a window-modal overlay atop the main Squeak window. **/
	return 1;
}

/* fileDialogDone: Answer whether a file dialog is finished.
   Arguments:
     dlgHandle: Dialog handle.
   Return value: True if dialog is finished or invalid (!); false if busy.
*/
int
fileDialogDone(int dlgHandle) {
	sqMacFileDialog *dlg = dlgFromHandle(dlgHandle);
	if (dlg && dlg->dlgRef) return 0;
	return 1;
}

/* fileDialogGetResult: Get the result of a file dialog invokation.
   Arguments:
     dlgHandle: Dialog handle.
   Return value: File path or NULL if canceled.
   N.B.  We do not fail this primitive with a 0 dlgHandle so that the image can
   use the success of this primitive to detect that the plugin does support
   native dialogs.
*/
char* fileDialogGetResult(int dlgHandle) {
	sqMacFileDialog *dlg = dlgFromHandle(dlgHandle);
	if(dlg) return dlg->result;
	return NULL;
}

/* fileDialogDestroy: Destroy the given file dialog.
   Arguments:
     dlgHandle: Dialog handle.
   Return value: True if successfully destroyed; false if not.
*/
int
fileDialogDestroy(int dlgHandle) {
	sqMacFileDialog *dlg = dlgFromHandle(dlgHandle);
	if(dlg->dlgRef) return 0; /* not finished */
	if(dlg->result) free(dlg->result);
	if(dlg->dco.saveFileName) CFRelease(dlg->dco.saveFileName);
	if(dlg->dco.windowTitle) CFRelease(dlg->dco.windowTitle);
	dlg->result = NULL;
	dlg->used = 0;
	return 1;
}

/* fileDialogGetLocation: Return a known file location.
   Arguments:
     location: Symbolic name for a path.
   Return value: Path for the given location or NULL.
*/
char *fileDialogGetLocation(char *location){
	OSErr err;
	FSRef fsRef;
	static char result[1024];

	result[0] = 0;

	err = -1;
	if(strcmp("home", location) == 0) {
		err = FSFindFolder(kUserDomain, kCurrentUserFolderType, 1, &fsRef);
	}
	if(strcmp("temp", location) == 0) {
		err = FSFindFolder(kLocalDomain, kTemporaryFolderType, 1, &fsRef);
	}
	if(strcmp("desktop", location) == 0) {
		err = FSFindFolder(kUserDomain, kDesktopFolderType, 1, &fsRef);
	}
	if(strcmp("preferences", location) == 0) {
		err = FSFindFolder(kUserDomain, kPreferencesFolderType, 1, &fsRef);
	}
	if(strcmp("applications", location) == 0) {
		err = FSFindFolder(kLocalDomain, kApplicationsFolderType, 1, &fsRef);
	}
	if(strcmp("fonts", location) == 0) {
		err = FSFindFolder(kLocalDomain, kFontsFolderType, 1, &fsRef);
	}
	if(strcmp("documents", location) == 0) {
		err = FSFindFolder(kUserDomain, kDocumentsFolderType, 1, &fsRef);
	}
	if(strcmp("music", location) == 0) {
		err = FSFindFolder(kUserDomain, kMusicDocumentsFolderType, 1, &fsRef);
	}
	if(strcmp("pictures", location) == 0) {
		err = FSFindFolder(kUserDomain, kPictureDocumentsFolderType, 1, &fsRef);
	}
	if(strcmp("videos", location) == 0) {
		err = FSFindFolder(kUserDomain, kMovieDocumentsFolderType, 1, &fsRef);
	}
	if(err == 0) {
		err = FSRefMakePath(&fsRef, result, 1023);
		if (err != 0) {
			printf("File Dialog getLocation error: %d\n", err);
		}
		if(err == 0) return result;
	}
	printf("fileDialogGetLocation(\'%s\'): error code %d (%x) \n", location, err, err);
	return NULL;
}


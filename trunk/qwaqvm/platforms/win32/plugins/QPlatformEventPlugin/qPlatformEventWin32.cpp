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

#include "qPlatformEventWin32.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
using namespace std;

#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>

#include "sqVirtualMachine.h"
#include "sqWin32.h"
#include "qLogger.hpp"
#include "qMessageIDs.h"
#include "qPlatformEvents.h"
using namespace Qwaq;

static messageHook *preMessageHook;
static messageHook nextPreMessageHook;

extern "C" 
{
	extern struct VirtualMachine* interpreterProxy;
}

// Need access to the feedback channels to push events into 'em.
extern FeedbackChannelMap gFeedbackChannels;

// Function called when a Windows message is received.
static int helperMessageHook(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

int qInitModuleWin32()
{
	initializeMessageIDs();

	preMessageHook = 
		(messageHook*) interpreterProxy->ioLoadFunctionFrom("preMessageHook","");
	if (!preMessageHook) {
		qerr << endl << "qInitModuleWin32() failed to look-up preMessageHook()";
		return -1;
	}
	nextPreMessageHook = *preMessageHook;
	*preMessageHook = (messageHook) helperMessageHook;

	return 0;
}


// XXXXX: This is duplicated in QwaqFileHelper.cpp
string getQwaqForumsDirPath(void)
{
	TCHAR szPath[MAX_PATH];

//	HRESULT hr = SHGetKnownFolderPath(FOLDERID_Documents, NULL, NULL, 0, szPath);
	HRESULT hr = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, NULL, szPath);
	PathAppend(szPath, TEXT("QwaqCroquet\\"));
	string foo(szPath);
	return foo;
}


void openTmpfileForSuffix(ifstream& file, unsigned int suffix)
{
	ostringstream fileNameStream(ostringstream::out);
	fileNameStream << getQwaqForumsDirPath() << "QwaqFileHelper-tmpfile-" << suffix;
	string fileName = fileNameStream.str();
	file.open(fileName.c_str(), fstream::in);
	if (!file) qerr << endl << "tmpfileForSuffix(): could not open: " << fileName;
}

int helperMessageHook(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == getFileHelperMessageID()) {
		ifstream tmpfile;
		openTmpfileForSuffix(tmpfile, wParam);
		if (!tmpfile) return 0;  // will already have been logged to qerr
		string versionString;
		getline(tmpfile, versionString);
		if (versionString != "<QwaqHelper version=\"1.0\" encoding=\"ISO-8859-1\">") {
			qerr << endl << "helperMessageHook(): unexpected version string: " << versionString;
			tmpfile.close();
			return 0;
		}
		string qurlFileName;
		getline(tmpfile, qurlFileName);
		tmpfile.close(); // we're done with the tmpfile

		PlatformFileHelperEvent* evt = new PlatformFileHelperEvent(qurlFileName);
		FeedbackEventPtr smartptr(evt);

		FeedbackChannelMap::iterator iter = gFeedbackChannels.begin();
		while (iter != gFeedbackChannels.end()) {
			qerr << endl << "JUST PUSHED FILE EVENT: " << qurlFileName;
			iter->first->push(smartptr);
			iter++;
		}
	}
	return 0;
}

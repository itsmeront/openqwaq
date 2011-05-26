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

// QwaqFileHelper.cpp : Defines the entry point for the console application.

// XXXXX: Unicode?  We don't need no steeeeenking unicode.

#include <windows.h>
#include <tchar.h>
#include <shlobj.h>
#include <shlwapi.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
using namespace std;

#include "qMessageIDs.h"

const char* QWAQ_WINDOW_CLASS = "SqueakWindowClass";
ofstream qerr;		// error reporting
ofstream stash;		// stash info that will be retrieved by Forums
ofstream input;		// file to read info from

// Close files and exit
void die(void)
{
	if (qerr.is_open()) qerr.close();
	if (stash.is_open()) stash.close();
	if (input.is_open()) stash.close();
	exit(0);
}


// XXXXX: This is duplicated in qPlatformEventWin32.cpp
string getQwaqForumsDirPath(void)
{
	TCHAR szPath[MAX_PATH];

//	HRESULT hr = SHGetKnownFolderPath(FOLDERID_Documents, NULL, NULL, 0, szPath);
	HRESULT hr = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, NULL, szPath);
	PathAppend(szPath, TEXT("QwaqCroquet\\"));
	string foo(szPath);
	SHCreateDirectoryEx(NULL, szPath, NULL);  // Ensure it exists. 
	return foo;
}


string getQwaqForumsExecutablePath(HINSTANCE hInst)
{
	TCHAR szPath[MAX_PATH];
	GetModuleFileName(hInst, szPath, MAX_PATH-1);
	string foo(szPath);
	return foo;

}


// Use a random number to generate a name for 
unsigned int initStash()
{
	SYSTEMTIME t;
	GetSystemTime(&t);
	unsigned int suffix = 
		(((t.wDay*24 + t.wHour)*60 + t.wMinute)*60 + t.wSecond)*1000 + t.wMilliseconds;

	ostringstream fileNameStream(ostringstream::out);
	fileNameStream << getQwaqForumsDirPath() << "QwaqFileHelper-tmpfile-" << suffix;
	stash.open(fileNameStream.str().c_str(), fstream::out | fstream::trunc);
	if (!stash) {
		qerr << "COULD NOT OPEN STASH-FILE WITH SUFFIX: " << suffix << endl;
		die();
	}
	return suffix;
}


// Iterate through top-level windows in order to find a running
// Qwaq Forums instance
HWND getForumsWindow(void)
{
	char buf[80];
	HWND win = GetTopWindow(0);
	while (win != 0) {
		if (0 == GetClassNameA(win, buf, 80)) {
			qerr << "ERROR GETTING WINDOW CLASS" << endl;
			die();
		}
		if (strcmp(QWAQ_WINDOW_CLASS, buf) == 0) return win;
		win = GetNextWindow(win, GW_HWNDNEXT);
	}
	return 0;
}


void launchNewQwaqForums(char* qurl, HINSTANCE hInst)
{
	string exePath = getQwaqForumsExecutablePath(hInst);
	ostringstream argStream(ostringstream::out);
	argStream << " \"\" -qwqFile: \"" << qurl << "\"";
//	argStream << "C:\\Users\\Administrator\\Desktop\\qwaq-dev\\QwaqForums-1.1.49\\vmmaker.1.image \"\" -qwqFile: \"" << qurl << "\"";

	qerr << "COULD NOT FIND EXISTING FORUMS INSTANCE.  LAUNCHING: " << exePath << endl;
	int hr = (int) ShellExecute(NULL,
								"open", 
								exePath.c_str(),
								argStream.str().c_str(),
								NULL,
								SW_NORMAL);
	if (hr <= 32) {
		qerr << "FAILED TO LAUNCH FORUMS (result: " << hr << ")" << endl;
	}
	die();
}



int WINAPI WinMain(      
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
	ostringstream fileNameStream(ostringstream::out);     // Write to Forums data directory so that we know we have permission. HRS 9/22/08
	fileNameStream << getQwaqForumsDirPath() << "QwaqFileHelper.log";
	qerr.open(fileNameStream.str().c_str(), fstream::out | fstream::app);  // Relocated from below. HRS 9/22/08
	if (!qerr) die();

	// We need to make this a Windows app (not a console app) so that we don't see a console window
	// briefly flash into existence.  So, hack argc/argv out of the Windows command-line so that we
	// don't have to change anything else.  And hack is the right word.
	int argc;
	LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argc);
	char* argv[2];
	if (argc != 2) {
		qerr << "MUST BE INVOKED WITH 2 ARGUMENTS" << endl;
		die();
	}

	wstring w1(argvW[0]);
	wstring w2(argvW[1]);
	string s1(w1.begin(), w1.end());
	string s2(w2.begin(), w2.end());
	argv[0] = (char*)(s1.c_str());
	argv[1] = (char*)(s2.c_str());


//int main(int argc, char* argv[])
//{
	
	initializeMessageIDs();

	/* // Relocated to above. HRS 9/22/08
	qerr.open("QwaqFileHelper.log", fstream::out | fstream::app);
	if (!qerr) die();
	if (argc != 2) {
		qerr << "MUST BE INVOKED WITH 2 ARGUMENTS" << endl;
		die();
	}
	*/

	qerr << "QURL FILE: " << argv[1] << endl;

	HWND win = getForumsWindow();
	if (!win) {
		launchNewQwaqForums(argv[1], hInstance);
		die();
	}
	unsigned int stashID = initStash();
	qerr << "STASH ID: " << stashID << endl;

	// Version 1.0 simply consists of the file-name of a QURL.
	stash << "<QwaqHelper version=\"1.0\" encoding=\"ISO-8859-1\">" << endl 
		<< argv[1] << endl
		<< "</QwaqHelper>" << endl;
	stash.flush();
	stash.close();

	if (win) {
		qerr << "FOUND TOP-LEVEL FORUMS WINDOW (handle: " << (unsigned int)win << ")" << endl;
		PostMessage(win, getFileHelperMessageID(), stashID, 0);
	}
	else {
		qerr << "COULD NOT FIND TOP-LEVEL FORUMS WINDOW" << endl;
	}

	die();
	return(0); // will not reach here.
}


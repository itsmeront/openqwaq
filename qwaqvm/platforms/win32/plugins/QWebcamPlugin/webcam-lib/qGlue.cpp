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

#include "qGlue.h"
#include "qWebcamDShow.h"
#include "qWebcamList.h"
#include "qLogger.hpp"
#include "qCommandQueue.h"
#include "qBuffer.h"
#include "qException.h"

#include "sqMemoryAccess.h"

//#define USE_BOOST_THREAD 1
#ifdef USE_BOOST_THREAD
#include <boost/thread/thread.hpp>
#endif

using namespace Qwaq;

const int MAX_CAMERAS=10;
static WebcamDShow* gCameras[MAX_CAMERAS];
static CommandQueue commands;
static BufferPool2 gBufferPool;

#ifdef USE_BOOST_THREAD
static boost::thread* gpWorkerThread;
#else
static HANDLE hThread;
#endif
static bool gWorkerIsRunning;


///////////////////////////////////////////
// forward declarations of helper functions
///////////////////////////////////////////

static int findUnusedCameraIndex();
static void workerThread();


/////////////////////////////////////////////////////
// definitions of functions called by QWebcamPlugin.c
/////////////////////////////////////////////////////

void qInitModule(void)
{
	int i;
	for(i=0; i<MAX_CAMERAS; i++) gCameras[i]=NULL;
	qInitLogging();
	qLogToFile("QWebcamPlugin.log");

	gWorkerIsRunning = true;

	// XXXXX: experimenting with removing foreground-thread priority boost (not enabled yet)
//	HANDLE currentThread = GetCurrentThread();
//	qLog() << "PRIORITY MAIN 1: " << GetThreadPriority(currentThread);
//	if (!currentThread) {
//		qLog() << "Couldn't get current thread";
//	}
//	else if (!SetThreadPriorityBoost(currentThread, FALSE)) {
//		qLog() << "Failed to disable priority boost";
//	}
//	else {
//		qLog() << endl << "DISABLED THREAD PRIORITY BOOST" << endl;
//	}
//	qLog() << "PRIORITY MAIN 2: " << GetThreadPriority(currentThread);


#ifdef USE_BOOST_THREAD
	gpWorkerThread = new boost::thread(workerThread);
	assert(gpWorkerThread != NULL);
#else
	hThread = CreateThread(	NULL,			   /* No security descriptor */
							//pageSize,                 /* default stack size     */
							128*1024,				   /* big.  really big */
							(LPTHREAD_START_ROUTINE)workerThread, /* what to do */
							NULL,
							//CREATE_SUSPENDED,  /* creation parameter -- create suspended so we can check the return value */
							CREATE_SUSPENDED | STACK_SIZE_PARAM_IS_A_RESERVATION,
							NULL);              /* return value for thread id */
	if (!hThread) {
		// XXXXX: need better error handling
		qLog() << "qInitModule(): failed to create thread";
		return;
	}
	if (!SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL)) {
		// XXXXX: need better error handling
		qLog() << "qInitModule(): failed to set thread priority";
		return;
	}
	qLog() << "qInitModule(): set thread priority to: " << GetThreadPriority(hThread);
	if (ResumeThread(hThread) == (DWORD)-1) {
		// XXXXX: need better error handling
		qLog() << "qInitModule(): failed to resume thread";
		return;
	}
#endif

	qLog() << "Successfully initialized QWebcamPlugin" << endl;
}

void qShutdownModule(void)
{
	qerr << endl << qTime() << " :   Shutting down QWebcamPlugin" << endl;

	// Shut down the worker thread.
	{
		gWorkerIsRunning = false;

		// Need to push one more event into the queue so that the worker
		// isn't blocked forever on processNext()
		boost::shared_ptr<Command> cmd(new NoOpCommand());
		commands.add(cmd);

#ifdef USE_BOOST_THREAD
		gpWorkerThread->join();
#else
		TerminateThread(hThread, 0);
#endif
		qerr << endl << "    - shutdown worker thread";
	}

	int i;
	for (i=0; i<MAX_CAMERAS; i++) {
		if (gCameras[i] != NULL) {
			delete gCameras[i];
			gCameras[i] = NULL;
		}
	}
	qerr << endl << "    - destroyed cameras";

	FeedbackChannel::releaseAllChannels();
	qerr << endl << "    - released feedback channels";

	qPrivateStopLists ();
	qShutdownLogging();
}

int qCameraIsValid(int cameraIndex)
{
	if (cameraIndex < 0 || cameraIndex >= MAX_CAMERAS) return 0;
	return gCameras[cameraIndex] != NULL;
}

char * qCameraName(int cameraIndex)
{
	if (cameraIndex < 0 || cameraIndex >= MAX_CAMERAS) return NULL;
	return gCameras[ cameraIndex ]->cameraName();
}


char * qCameraUID(int cameraIndex)
{
	if (cameraIndex < 0 || cameraIndex >= MAX_CAMERAS) return NULL;
	return gCameras[ cameraIndex ]->cameraUID();
}

	
/* Old creation API */
int qCreateCamera(char* args, int argsSize, char* feedbackChannelHandle)
{
	return qCreateCameraByUID(NULL, args, argsSize, feedbackChannelHandle);
}

/* New creation API.   (UID NULL picks the default device) */

int qCreateCameraByUID(char* uid, char* args, int argsSize, char* feedbackChannelHandle)
{
	FeedbackChannel* channel = *((FeedbackChannel**)feedbackChannelHandle);
	boost::shared_ptr<CreateCameraCommand> cmd(new CreateCameraCommand(uid, args, argsSize, channel));
	commands.add(cmd);
	qerr << endl << "qCreateCamera() "; cmd->printElapsedTimes();
	qerr.flush();
	return cmd->result;
}

int qDestroyCamera(int cameraIndex)
{
	boost::shared_ptr<DestroyCameraCommand> cmd(new DestroyCameraCommand(cameraIndex));
	commands.add(cmd);
	qerr << endl << "qDestroyCamera() "; cmd->printElapsedTimes();
	qerr.flush();
	return cmd->result;
}

void qCameraAdjust(int cameraIndex, char* args, int argsSize)
{
	boost::shared_ptr<AdjustCameraCommand> cmd(new AdjustCameraCommand(cameraIndex, args, argsSize));
	commands.add(cmd);
	// Don't wait for command to finish.
}

int qCameraRun(int cameraIndex)
{
	boost::shared_ptr<ControlCameraCommand> cmd(new ControlCameraCommand(cameraIndex, ControlCameraCommand::RUN));
	commands.add(cmd);
	qerr << endl << "qCameraRun() "; cmd->printElapsedTimes();
	qerr.flush();
	return cmd->result;
}

int qCameraPause(int cameraIndex)
{
	boost::shared_ptr<ControlCameraCommand> cmd(new ControlCameraCommand(cameraIndex, ControlCameraCommand::PAUSE));
	commands.add(cmd);
	qerr << endl << "qCameraPause() "; cmd->printElapsedTimes();
	qerr.flush();
	return cmd->result;
}

int qCameraStop(int cameraIndex)
{
	boost::shared_ptr<ControlCameraCommand> cmd(new ControlCameraCommand(cameraIndex, ControlCameraCommand::STOP));
	commands.add(cmd);
	qerr << endl << "qCameraStop() "; cmd->printElapsedTimes();
	qerr.flush();
	return cmd->result;
}

int qCameraGetParams(int cameraIndex, char* externalAddress)
{
//	assert(0);
	return 0;
}

// Enumeration of available devices.  These devIndexes range [ 0, qGetNumCamDevices()-1 ]

int qGetNumCamDevices () {
	boost::shared_ptr<ListCamerasCommand> cmd(new ListCamerasCommand());
	commands.add(cmd);
	//NOISY qerr << endl << "qGetCamDevices() "; cmd->printElapsedTimes();
	qerr.flush();
	return cmd->result;
}

// Return the name of cam n from the most -recent cam-list update.
// (I can see no reason to push this over to the other thread,
// since the lister command is always wait-for-completion;
// these just read the most-recent result.)
char*  qGetCamDeviceName(int devIndex) {
	return qPrivateGetCamDeviceName(devIndex);
}

char*  qGetCamDeviceUID(int devIndex) {
	return qPrivateGetCamDeviceUID(devIndex);
}

int    qGetCamDeviceIsBusy(int devIndex) {
	// Unused, as neither mac nor win tells us this up front, until we try to attach.
	return qPrivateGetCamDeviceIsBusy(devIndex);
}

void   qCopyBufferToBitmap(char *bitmapData, void *bufferAddress, sqInt bufferSize)
{
	memcpy(bitmapData, *(char**)bufferAddress, bufferSize);
}

//////////////////////////////////////////////
// definitions of command functions (enqueued,
// then processed in another thread)
//////////////////////////////////////////////

CreateCameraCommand::CreateCameraCommand(char* uid, char* params, int paramsSize, FeedbackChannel* ch) 
: Command(true), channel(ch), devicePath(uid ? uid : "")
{
	WebcamParams *p = (WebcamParams*)params;
	assert(p->version >= WebcamParams::currentVersion());
	parameters = *p;
}
	
void CreateCameraCommand::execute() 
{
	int index = findUnusedCameraIndex();
	if (index == -1) {
		// Couldn't find an available index
		result = QE_NO_FREE_OBJECT_SLOTS;
		return;
	}

	WebcamDShow* cam = NULL;
	try {
		WebcamDShow* cam;
		cam = new WebcamDShow(devicePath, &parameters, gBufferPool, channel);
		if (!cam) {
			qerr << endl << qTime() << " : CreateCameraCommand: allocation failed";
			result = QE_ALLOCATION_FAILED;
			return;
		}
		gCameras[index] = cam;
		result = index;
	}
	catch (Status status) {
		qerr << endl << qTime() << " : CreateCameraCommand: caught status code exception " << status;
		result = status;
	}
	catch (...) {
		qerr << endl << qTime() << " : CreateCameraCommand: caught exception";
		result = QE_CAUGHT_EXCEPTION;
	}
}


DestroyCameraCommand::DestroyCameraCommand(int camIndex) : Command(true), cameraIndex(camIndex) { }

void DestroyCameraCommand::execute()
{
	if (cameraIndex < 0 || cameraIndex >= MAX_CAMERAS) {
		result = QE_INDEX_OUT_OF_RANGE;
		return;
	}
	WebcamDShow* cam = gCameras[cameraIndex];
	gCameras[cameraIndex] = NULL;
	if (cam) {
		delete cam;
	} else {
		qerr << endl << qTime() << " : DestroyCameraCommand: no camera existed at index " << cameraIndex;
	}
	result = QS_OK;
}

AdjustCameraCommand::AdjustCameraCommand(int camIndex, char* params, int paramsSize)
	: Command(false), cameraIndex(camIndex)
{
	WebcamParams *p = (WebcamParams*)params;
	assert(p->version >= WebcamParams::currentVersion());
	parameters = *p;
}

void AdjustCameraCommand::execute()
{
	WebcamDShow* cam = NULL;
	if (cameraIndex >= 0 && cameraIndex < MAX_CAMERAS) {
		cam = gCameras[cameraIndex];
	}
	if (!cam) {
		qerr << endl << qTime() << " AdjustCameraCommand: no camera at index: " << cameraIndex;
		return;
	}
	cam->adjustParameters(parameters);
}

ControlCameraCommand::ControlCameraCommand(int camIndex, CommandType t) : Command(true), cameraIndex(camIndex), type(t) { }

void ControlCameraCommand::execute()
{
	if (cameraIndex < 0 || cameraIndex >= MAX_CAMERAS) {
		result = QE_INDEX_OUT_OF_RANGE;
		return;
	}
	WebcamDShow* cam = gCameras[cameraIndex];
	if (!cam) {
		qerr << endl << qTime() << " ControlCameraCommand: no camera at index: " << cameraIndex;
		result = QE_DEVICE_UNAVAILABLE;
		return;
	}
	switch (type) {
		case RUN:
			cam->run();
			break;
		case PAUSE:
			cam->pause();
			break;
		case STOP:
			cam->stop();
			break;
	}
	result = QS_OK;
}

ListCamerasCommand::ListCamerasCommand() : Command(true) {}

void ListCamerasCommand::execute () {
	result = qPrivateGetNumCamDevices();
}


//////////////////////////////
// helper function definitions
//////////////////////////////

static int findUnusedCameraIndex()
{
	int i;
	for (i=0; i < MAX_CAMERAS; i++) {
		if (gCameras[i] == NULL) return i;
	}
	return -1;
}

static void workerThread()
{
	// Initialize COM
	HRESULT hr = CoInitialize(NULL);
	if (hr != S_OK) {
		// XXXXX: need some way for the main thead to verify that this succeeded
		// before returning from qInitModule() (also, should return success or
		// failure all the way back through initialiseModule())
		qLog() << " :  workerThread(): couldn't initialize COM";
		return;
	}

	// Do the real work
	while (gWorkerIsRunning) commands.processNext();

	// Shut COM down
	CoUninitialize();
}

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
 * qDevices.cpp
 * QAudioPlugin
 * 
 * Provide access to sound device names.  Duplicates functionality in the
 * SoundPlugin, but doing so avoids linking QAudioPlugin to the SoundPlugin.
 */

#undef DEVICE_NAME_INCLUDES_LINE

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
using namespace std;

#include <Windows.h>

#include "QAudioPlugin.h"

#include <OpenAL/al.h>
#include <OpenAL/alc.h>
//#include <dsound.h>



/*********************
 * PRIVATE UTILITIES *
 *********************/

char buffer[1024];
char * getString() {return &buffer[0];}

// I cannot find a documented way to determine whether a line on a device is for input or output.
// Experimentally, the following seems to work.
bool isInput(MIXERLINE *mmLine)
{ 
	if (mmLine->dwComponentType == MIXERLINE_COMPONENTTYPE_DST_WAVEIN) return true;
	if (mmLine->dwComponentType == MIXERLINE_COMPONENTTYPE_DST_VOICEIN) return true;
	if (mmLine->dwComponentType == MIXERLINE_COMPONENTTYPE_DST_TELEPHONE) return true;
	return false;	
}

bool isOutput(MIXERLINE *mmLine) { return !isInput(mmLine); }

char *trimPrefix(char *s, char *prefix)
{
	while (!!*s && (*s++ == *prefix++)) 
	{ 
		if (!*prefix) return s; 
	}
	return NULL;
}


UINT iterateDeviceData(bool (*predicate)(MIXERLINE *), char * deviceName, int deviceNumber, MIXERLINE *mmLine)
{
	char *lineName;
	MMRESULT mmResult;
	UINT nDevs = mixerGetNumDevs();
	UINT deviceID, lineNumber, nDevices = 0; 
	MIXERCAPS mmCaps;

	for (deviceID = 0; deviceID < nDevs; deviceID++) {
		mmResult = mixerGetDevCaps(deviceID, &mmCaps, sizeof(MIXERCAPS));
		if (mmResult != MMSYSERR_NOERROR) return -1;
		//lineName = trimPrefix(deviceName, mmCaps.szPname);
		//if (!!lineName) lineName++;
		for (lineNumber = 0; lineNumber < mmCaps.cDestinations; lineNumber++) {
			mmLine->cbStruct = sizeof(MIXERLINE);
			mmLine->dwDestination = lineNumber;
			mmLine->dwSource = 0;
			mmResult = mixerGetLineInfo((HMIXEROBJ)deviceID, mmLine, MIXER_OBJECTF_MIXER | MIXER_GETLINEINFOF_DESTINATION);
			if (mmResult != MMSYSERR_NOERROR) return -1;
			if ((mmLine->cChannels > 0) && predicate(mmLine))
			{
				if (!!deviceName)
				{
					if (!strcmp(deviceName, mmCaps.szPname)) return deviceID;
#ifdef DEVICE_NAME_INCLUDES_LINE
					lineName = trimPrefix(deviceName, mmCaps.szPname);
					if (!!lineName) lineName++;
					if (!!lineName && !strcmp(lineName, mmLine->szName)) return deviceID;
#endif
				}
				else
				{
					if(nDevices == deviceNumber) return deviceID;
				}
				nDevices++;
			}
		}
	}
	if (deviceNumber < 0) return nDevices;
	return -1;
}

int getNumberOfSoundDevices(bool (*predicate)(MIXERLINE *))
{
	MIXERLINE mmLine;
	return iterateDeviceData(predicate, NULL, -1, &mmLine);
}

char * getDeviceName(bool (*predicate)(MIXERLINE *), int deviceNumber)
{
	MIXERLINE mmLine;
	UINT deviceID = iterateDeviceData(predicate, NULL, deviceNumber, &mmLine);
	if (deviceID == -1) return NULL;
	MIXERCAPS mmCaps;
	MMRESULT mmResult = mixerGetDevCaps(deviceID, &mmCaps, sizeof(MIXERCAPS));
	if (mmResult != MMSYSERR_NOERROR) return NULL;
	char *answer = getString();
	strcpy(answer, mmCaps.szPname);
#ifdef DEVICE_NAME_INCLUDES_LINE
	// If we need to distinguish between lines on a device, this is how.
	// However, it appears unnecessary experimentally, and gets in the way of default management with openAL.
	strcpy(answer + strlen(answer), " ");
	strcpy(answer + strlen(answer), mmLine.szName);
#endif
	return answer;
}


UINT prepControl(bool (*predicate)(MIXERLINE *), char *deviceName, DWORD controlType, MIXERLINE *mmLine, MIXERLINECONTROLS *mmControls, MIXERCONTROLDETAILS *mmDetails)
{	
	UINT deviceID;
	MMRESULT mmResult;

	deviceID = iterateDeviceData(predicate, deviceName, 0, mmLine);
	if (deviceID < 0) return MMSYSERR_BADDEVICEID;

	mmControls->cbStruct = sizeof(MIXERLINECONTROLS);
	mmControls->cControls = 1;
	mmControls->dwLineID = mmLine->dwDestination;
	mmControls->cbmxctrl = sizeof(MIXERCONTROL);
	mmControls->dwControlType = controlType;
	mmResult = mixerGetLineControls((HMIXEROBJ) deviceID, mmControls, MIXER_OBJECTF_MIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE);
	if (mmResult != MMSYSERR_NOERROR) return -1;

	mmDetails->cbStruct = sizeof(MIXERCONTROLDETAILS);
	mmDetails->cMultipleItems = 0;

	return(deviceID);
}

double getVolume(bool (*predicate)(MIXERLINE *), char *deviceName)
{ 
	MMRESULT mmResult;
	MIXERLINE mmLine;

	MIXERCONTROL mmControl;
	MIXERLINECONTROLS mmControls;
	mmControls.pamxctrl = &mmControl;

	MIXERCONTROLDETAILS_UNSIGNED mmValue[4];
	MIXERCONTROLDETAILS mmDetails;	
	mmDetails.paDetails = &mmValue;

	UINT deviceID = prepControl(predicate, deviceName, MIXERCONTROL_CONTROLTYPE_VOLUME, &mmLine, &mmControls, &mmDetails);
	if (deviceID == -1) return -1.0;

	mmDetails.dwControlID = mmControl.dwControlID;
	mmDetails.cChannels = mmLine.cChannels;
	mmDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED) * mmLine.cChannels;
	mmResult = mixerGetControlDetails((HMIXEROBJ)deviceID, &mmDetails, MIXER_OBJECTF_MIXER | MIXER_GETCONTROLDETAILSF_VALUE);
	if (mmResult != MMSYSERR_NOERROR) return -1.0;
	return ((double) mmValue[0].dwValue - mmControl.Bounds.dwMinimum) / (mmControl.Bounds.dwMaximum - mmControl.Bounds.dwMinimum);
}

double setVolume(double volume, bool (*predicate)(MIXERLINE *), char *deviceName)
{ 
	MMRESULT mmResult;
	MIXERLINE mmLine;

	MIXERCONTROL mmControl;
	MIXERLINECONTROLS mmControls;
	mmControls.pamxctrl = &mmControl;

	MIXERCONTROLDETAILS_UNSIGNED mmValue[4];
	MIXERCONTROLDETAILS mmDetails;		
	mmDetails.paDetails = &mmValue;

	UINT deviceID= prepControl(predicate, deviceName, MIXERCONTROL_CONTROLTYPE_VOLUME, &mmLine, &mmControls, &mmDetails);
	if (deviceID == -1) return -1.0;

	UINT32 v = (mmControl.Bounds.dwMaximum - mmControl.Bounds.dwMinimum);
	v *= volume;
	v += mmControl.Bounds.dwMinimum;
	mmValue[0].dwValue = v;
	mmDetails.dwControlID = mmControl.dwControlID;
	mmDetails.cChannels = 1;  // set as master.
	mmDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
	mmResult = mixerSetControlDetails((HMIXEROBJ)deviceID, &mmDetails, MIXER_OBJECTF_MIXER | MIXER_SETCONTROLDETAILSF_VALUE);
	if (mmResult != MMSYSERR_NOERROR) return -1.0;
	return ((double) mmValue[0].dwValue - mmControl.Bounds.dwMinimum) / (mmControl.Bounds.dwMaximum - mmControl.Bounds.dwMinimum);
}


/*********************************************************************************
 * PUBLIC INTERFACE                                                              *
 *                                                                               *
 * Strings returned are only valid until the next error. (They get overwritten.) *
 *********************************************************************************/

int getNumberOfSoundPlayerDevices() { return getNumberOfSoundDevices(isOutput); }
int getNumberOfSoundRecorderDevices() { return getNumberOfSoundDevices(isInput); }

char * getSoundPlayerDeviceName(int i) { return getDeviceName(isOutput, i); }
char * getSoundRecorderDeviceName(int i) { return getDeviceName(isInput, i); }

double getSoundPlayerVolume(char *deviceName) { return getVolume(isOutput, deviceName); }
double getSoundRecorderVolume(char *deviceName) { return getVolume(isInput, deviceName); }

double setSoundPlayerVolume(double volume, char *deviceName) { return setVolume(volume, isOutput, deviceName); }
double setSoundRecorderVolume(double volume, char *deviceName) { return setVolume(volume, isInput, deviceName); }


/*************************************************
 * DEVICE EXPLORATION -- i.e., development code. *
 *************************************************/
ofstream qout; // error reporting

// DirectSound device enumeration callback
/*
BOOL CALLBACK DSEnumProc(LPGUID lpGUID, 
             LPCTSTR lpszDesc,
             LPCTSTR lpszDrvName, 
             LPVOID lpContext )
{
	qout << (char *)lpContext << " name:" << lpszDrvName << " desc:" << lpszDesc << " guid:" << lpGUID << endl;
     return(TRUE);
}
*/

/* Explore the Windows Multimedia Audio Mixers (mixerXxx) API, side-effecting the file system with some results.
	Call this from plugin initialization or some such. */
void qDeviceExplorationHook()
{
	// Get a place to stash results.
	qout.open("QwaqDeviceExploration.log", fstream::out | fstream::app);

	/* 
	int nDevices, i;
	char * name;
	double volume;
	nDevices = getNumberOfSoundPlayerDevices();
	qout << nDevices << " player devices:" << endl;
	for (i = 0; i<nDevices; i++) {
		name = getSoundPlayerDeviceName(i);
		qout << "	" << name;
		volume = getSoundPlayerVolume(name);
		setSoundPlayerVolume(volume + 0.1, name);
		qout << " was " << volume << " now " << getSoundPlayerVolume(name) << endl;;
		}
	//setDefaultSoundPlayer(getSoundPlayerDeviceName(0));
		
	nDevices = getNumberOfSoundRecorderDevices();
	qout << nDevices << " recorder devices:" << endl;
	for (i = 0; i<nDevices; i++) {
		char * name = getSoundRecorderDeviceName(i);
		qout << "	" << name;
		volume = getSoundRecorderVolume(name);
		qout << " was " << volume;
		if (volume < 0) volume = 0.5;
		qout << " set to " << setSoundRecorderVolume(volume + 0.1, name);
		qout << " now " << getSoundRecorderVolume(name) << endl;
		}
	//setDefaultSoundRecorder(getSoundPlayerDeviceName(0));
	*/

	const ALCchar *	devices;
	ALCchar *	ptr;
	devices = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
	qout << endl << "default openAL device " << devices;
	devices = alcGetString(NULL, ALC_DEVICE_SPECIFIER);
	ptr = (ALCchar *) devices;
	while (!!*ptr)
	{
		qout << endl << "	" << ptr;
		while (!!*ptr++);
	}
	devices = alcGetString(NULL, ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER);
	qout << endl << "default openAL capture device " << devices;
	devices = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
	ptr = (ALCchar *) devices;
	while (!!*ptr)
	{
		qout << endl << "	" << ptr;
		while (!!*ptr++);
	}

	qout << endl;
	//if (FAILED(DirectSoundEnumerate((LPDSENUMCALLBACK)DSEnumProc,(VOID*)" output"))) qout << "output enumeration failed" << endl;
	//if (FAILED(DirectSoundCaptureEnumerate((LPDSENUMCALLBACK)DSEnumProc,(VOID*) " capture"))) qout << "output enumeration failed" << endl;

	qout << endl;
	// cleanup
	qout.close();
}

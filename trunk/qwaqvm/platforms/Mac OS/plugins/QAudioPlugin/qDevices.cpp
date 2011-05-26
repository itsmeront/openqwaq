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
 *  qDevices.cpp
 *  QAudioPlugin
 *
 * Provide access to sound device names.  Duplicates functionality in the
 * SoundPlugin, but doing so avoids linking QAudioPlugin to the SoundPlugin.
 */
 
 
#include <CoreAudio/CoreAudio.h> 
 
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
using namespace std;

#include "QAudioPlugin.h"



/*********************
 * PRIVATE UTILITIES *
 *********************/
// An OSX CoreAudio input with a unique AudioDeviceID may have the same name as the corresponding output device, 
// and a device can have both input and output. The CoreAudio APIs mostly take an isInput parameter to indicate 
// which you're working with.

static char buffer[1024];
char *	getString() {return &buffer[0];}
 

// A device might not have any channels of the given type. 
int numberOfChannels(Boolean isInput, UInt16 deviceID)
{
	OSStatus	err; 
    UInt32 		outSize = 0;
	AudioBufferList *theBufferList = NULL;
	UInt32      theIndex = 0;
	int			nChannels = 0;
	err = AudioDeviceGetPropertyInfo( deviceID, 0, isInput, kAudioDevicePropertyStreamConfiguration, &outSize, NULL);
	if ((err == noErr) && (outSize != 0)) 
	{
		theBufferList = (AudioBufferList*) malloc(outSize);
		err = AudioDeviceGetProperty( deviceID, 0, isInput, kAudioDevicePropertyStreamConfiguration, &outSize, theBufferList);
		if (err == noErr) {
			for (theIndex = 0; theIndex < theBufferList->mNumberBuffers; ++theIndex) 
				nChannels += theBufferList->mBuffers[theIndex].mNumberChannels;
			}
		free(theBufferList);
	}
	return nChannels;
}

OSStatus GetAudioDevices( Ptr * devices, UInt16 * devicesAvailable )
{
    OSStatus	err = noErr;
    UInt32 		outSize;
    Boolean		outWritable;
    
    // find out how many audio devices there are, if any
    err = AudioHardwareGetPropertyInfo(kAudioHardwarePropertyDevices, &outSize, &outWritable);	
    if ( err != noErr ) 
		return err;
   
    // calculate the number of device available
	*devicesAvailable = outSize / sizeof(AudioDeviceID);						
    if ( *devicesAvailable < 1 )
	{
		fprintf( stderr, "No devices\n" );
		return err;
	}
    
    // make space for the devices we are about to get
    *devices = (Ptr) malloc(outSize);		
    	
    memset( *devices, 0, outSize );			
    err = AudioHardwareGetProperty(kAudioHardwarePropertyDevices, &outSize, (void *) *devices);	
    if (err != noErr )
      return err;
	
    return err;
}


// Iterate through the device data, looking for a match.
// If deviceName is non-null, we answer the deviceID of the device with the same name, else 0.
// Otherwise, answer the deviceID of the nth matching device, else 0: 
//	If the requested deviceNumber is -1, 
//	none will match and we just answer the total number of devices of the requested kind.
// We do not count devices that have no channels of the requested isInput kind.
UInt32	iterateDeviceData(Boolean isInput, char *deviceName, int deviceNumber)
{
	AudioDeviceID	* 	devices = NULL;
    UInt16				devicesAvailable = 0;

	UInt16		loopCount = 0;
	char 		buffer[1024];

    OSStatus	err;
	UInt32		nDevices = 0;
	UInt32 		outSize;
	
	int			nChannels;
			
	// fetch a pointer to the list of available devices
	if (GetAudioDevices((Ptr*)&devices, &devicesAvailable) != noErr) return 0;
	// iterate over each device gathering information
	for (loopCount = 0; loopCount < devicesAvailable; loopCount++)
	{
		AudioDeviceID deviceID = devices[loopCount];
		nChannels = numberOfChannels(isInput, deviceID);
		if (nChannels > 0)
		{
			if (!!deviceName)
			{ 
				outSize = sizeof(buffer);
				err = AudioDeviceGetProperty( deviceID, 0, isInput, kAudioDevicePropertyDeviceName, &outSize, &buffer);
				if (err != noErr) return 0;

				if (!strcmp(deviceName, buffer))
				{
					free(devices);
					return deviceID;
				}
			}
			else
			{ 
				if (nDevices == deviceNumber)
				{
					free(devices);
					return deviceID;
				}
			}
			nDevices++;
		}
	}
	free(devices);
	if (deviceNumber < 0) return nDevices;
	return 0;
}

// Answer the deviceID matching the arguments, else 0.
UInt32	getDeviceID(Boolean isInput, char *deviceName)
{
	return iterateDeviceData(isInput, deviceName, 0);
}

// Answer the number of audio devices of the requested input/output type.
// The argument to getDeviceName must be less than this.
int getNumberOfSoundDevices(Boolean isInput)
{
	return iterateDeviceData(isInput, NULL, -1);
}

// Answer the unique human-readable name of input/output version of deviceID, or NULL if there isn't one.
char * getDeviceNameOfID(Boolean isInput, UInt32 deviceID)
{
	if (!deviceID) return NULL;
	char		deviceName[1024];
	UInt32 		outSize = sizeof(deviceName);

	OSStatus	err = AudioDeviceGetProperty( deviceID, 0, isInput, kAudioDevicePropertyDeviceName, &outSize, &deviceName);
	if (err != noErr) return NULL;
	return strcpy(getString(), deviceName);
}

// The numeric argument is from 0 to less than the number of devices of the requested kind.
// The answer can be used as argument to setDefaultDevice(), getVolume(), setVolume().
char * getDeviceName(Boolean isInput, int deviceNumber)
{
	return getDeviceNameOfID(isInput, iterateDeviceData(isInput, NULL, deviceNumber));
}
 
// Answer the name of the device that will be used when we next open (from either openAL or squeak sound).
char *	getDefaultDevice(Boolean isInput, AudioHardwarePropertyID propertyID)
{
	UInt32 deviceID;
	UInt32 deviceIDsize = sizeof(deviceID);
	OSStatus	err = AudioHardwareGetProperty(propertyID, &deviceIDsize, &deviceID);
	if (err != noErr) return NULL;
	return getDeviceNameOfID(isInput, deviceID);
}
 
 
// Arrange for deviceName to be used when we next open an audio device of the input/output kind specified by isInput.
void setDefaultDevice(char *deviceName, Boolean isInput, AudioHardwarePropertyID propertyID)
{
	UInt32	deviceID = getDeviceID(isInput, deviceName);
	if (!deviceID) return;
	AudioHardwareSetProperty(propertyID, sizeof(AudioDeviceID), &deviceID);
}


double getVolumeByChannel(Boolean isInput, UInt16 deviceID, UInt32 channel)
{
	Float32		volume;
	UInt32 		outSize = sizeof(volume);
	OSStatus	err;
	
	err = AudioDeviceGetProperty( deviceID, channel, isInput, kAudioDevicePropertyVolumeScalar, &outSize, &volume);
	if (err != noErr) return -1.0;
	return (double) volume;
}

double	setVolumeByChannel(double volume, Boolean isInput, UInt16 deviceID, UInt32 channel)
{
	Float32 v = (Float32) volume;
	OSStatus	err = AudioDeviceSetProperty(deviceID, NULL, channel, isInput, kAudioDevicePropertyVolumeScalar, sizeof(Float32), &v);
	if (err != noErr) return -1;
	return volume;
}

// Get the volume of device identified by the arguments, as a float between 0 and 1.
double	getVolume(Boolean isInput, char *deviceName)
{
	UInt32 channel;
	UInt16 deviceID = getDeviceID(isInput, deviceName);
	int nChannels = numberOfChannels(isInput, deviceID);
	
	if (!deviceID) return -1.0;
	
	if (1 == nChannels) channel = 0;	// Master channel.
	else channel = 1;	// Just answer the first channel volume.
	
	return getVolumeByChannel( isInput, deviceID, channel);
}

//  Set the volume ([0.0, 1.0]) of device identified by the arguments.
double	setVolume(double volume, Boolean isInput, char *deviceName)
{
	UInt16 deviceID = getDeviceID(isInput, deviceName);
	
	if (!deviceID) return -1.0;

	int nChannels = numberOfChannels(isInput, deviceID);
		
	if (1 == nChannels)
		return setVolumeByChannel(volume, isInput, deviceID, 0);	// Master channel only.
	else 	// Set volume on each channel
		for ( ; nChannels > 0; nChannels--) setVolumeByChannel(volume, isInput, deviceID, nChannels);
	return volume;
}

/***************************************************************************************
 * PUBLIC INTERFACE																	   *
 *																					   *
 * Strings returned are only valid until the next error. (They get overwritten.)	   *
 ***************************************************************************************/

int getNumberOfSoundPlayerDevices() { return getNumberOfSoundDevices(false); }
int getNumberOfSoundRecorderDevices() { return getNumberOfSoundDevices(true); }
 
char * getSoundPlayerDeviceName(int i) { return getDeviceName(false, i); }
char * getSoundRecorderDeviceName(int i) { return getDeviceName(true, i); }

char *	getDefaultSoundPlayer() { return getDefaultDevice(false, kAudioHardwarePropertyDefaultOutputDevice); }
char *	getDefaultSoundRecorder() { return getDefaultDevice(true, kAudioHardwarePropertyDefaultInputDevice); }

void setDefaultSoundPlayer(char *deviceName) { return setDefaultDevice(deviceName, false, kAudioHardwarePropertyDefaultOutputDevice); }
void setDefaultSoundRecorder(char *deviceName) { return setDefaultDevice(deviceName, true, kAudioHardwarePropertyDefaultInputDevice); }


double getSoundPlayerVolume(char *deviceName) { return getVolume(false, deviceName); }
double getSoundRecorderVolume(char *deviceName) { return getVolume(true, deviceName); }

double setSoundPlayerVolume(double volume, char *deviceName) { return setVolume(volume, false, deviceName); }
double setSoundRecorderVolume(double volume, char *deviceName) { return setVolume(volume, true, deviceName); }



/*************************************************
 * DEVICE EXPLORATION -- i.e., development code. *
 *************************************************/

/*	Explore the OSX API, side-effecting the file system with some results.
	Call this from plugin initialization or some such. */
void qDeviceExplorationHook()
{	
	ofstream qout;		// error reporting

	//	Get a place to stash results.
	qout.open("QwaqDeviceExploration.log", fstream::out | fstream::app); 
	
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
		qout << " was " << volume << " now " << getSoundPlayerVolume(name) << endl;
		}
	setDefaultSoundPlayer(getSoundPlayerDeviceName(0));
		
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
	setDefaultSoundRecorder(getSoundPlayerDeviceName(0));

	// Cleanup
	qout.close();
}

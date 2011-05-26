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
 * qReadback.inc
 * QVideoCodecPlugin
 *
 * Defines common functions for reading back data from the plugin.
 */

#include "qReadback.h"
#include "qVideoCommon.h"

void qInitCallbackData(QCallbackData* cbd)
{
	cbd->data = 0;
	cbd->maxSize = 0;
	cbd->readSize = 0;
	cbd->stompCount = 0;
}


int qPrepareCallbackDataForWrite(QCallbackData *cbd, int length)
{
	if (length < 0) {
		// Is you mental?
		qerr << endl << "qPrepareCallbackDataForWrite: length is negative: " << length;
		cbd->readSize = -1;
		cbd->stompCount = 0;
		return -1;
	}

	// Sorta weird, but no harm done.
	if (length == 0) {
		cbd->readSize = 0;
		cbd->stompCount = 0;
		return 0;
	}

	if (cbd->readSize > 0) {
		// We already have some data to read; make a note that we're
		// about to stomp on that data.
		cbd->stompCount++;
	}

	if (cbd->maxSize < length) {
		// We don't have enough space to store the data.
		if (cbd->data != 0) {
			// We need to release the existing memory before allocating more.
			free(cbd->data);
		}
		// Allocate more space...
		cbd->data = (char*) malloc(length);
		if (cbd->data == 0) {
			// Oops, malloc failed.
			qerr << endl << "qPrepareCallbackDataForWrite: malloc failed";
			cbd->maxSize = -1;
			cbd->readSize = -1;
			cbd->stompCount = 0;
			return -1;
		}
		// Yay, malloc succeeded.  Make a note of how much space we have now.
		qerr << endl << "qPrepareCallbackDataForWrite: allocated " << length << " bytes";
		cbd->maxSize = length;
	}
		
	// Make note of the amount of data that will eventually be read.
	cbd->readSize = length;
	
	return 0; // success!
}


void qStoreCallbackData(char *source, QCallbackData *cbd, int length)
{	
	int dataErr = qPrepareCallbackDataForWrite(cbd, length);
	if (dataErr) {
		qerr << endl << "qStoreCallbackData: cannot get a callback buffer";
		return;	
	}
	memcpy(cbd->data, source, length);
}


int qReadCallbackData(QCallbackData *cbd, char *target, int maxLength)
{
	int readSize = cbd->readSize;
	cbd->readSize = cbd->stompCount = 0;

	if (readSize > maxLength) {
		// Not enough space to copy data into.
		qerr << endl << "qReadCallbackData: insufficient space in target (" << readSize << " required, " << maxLength << " available)";
		return -2;
	}

	if (readSize < 0) {
		// Invalid data source
		qerr << endl << "qReadCallbackData: invalid data source";
		return -3;
	}

	if (readSize == 0) {
		// Nothing to do
		return 0;
	}

	// Copy-ho!
	memcpy(target, cbd->data, readSize);
	return readSize;
 }


void qFreeCallbackData(QCallbackData *cbd)
{
	if (cbd->data != NULL) { 
		free(cbd->data);
		cbd->data = NULL;
		cbd->maxSize = cbd->readSize = cbd->stompCount = 0;
	}
}

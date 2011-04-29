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
 * qReadback.h
 * QVideoCodecPlugin
 *
 * Defines common functions for reading back data from the plugin.
 */

#ifndef __Q_READBACK_H__
#define __Q_READBACK_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* 
 * QCallbackData and it's associated functions are used whenever data needs to be
 * temporarily stashed so that it can later be read back by Squeak.
 */
typedef struct
{
	char* data;
	int maxSize;			// how big is the allocated data buffer?
	int readSize;			// how much data was read by the last callback?
	int stompCount;			// how many times has data been stomped because it was
							// not read before the next data came in?
} QCallbackData;

/* qInitCallbackData: Callback data must be initialized before it can be used.
   Arguments:
     cbd - the QCallbackData structure that is to be initialized
   Return value: None.
*/
void qInitCallbackData(QCallbackData* cbd);


/* qPrepareCallbackDataForWrite:	Prepare a QCallbackData to recieve the specified amount
									of data.  The intent is much the same as 
									qPrivateStoreCallbackData(), except that the calling 
									function writes the data into the QCallbackData.
	HACK ALERT:	I added this to support writing to the callback buffer from multiple sources,
				instead of a single source data pointer.  Since this now seems likely to be common,
				I might have written things a bit differently (but I'm not sure how).
	Arguments:
		target: Pointer to the QCallbackData that will store the data.
		length: Number of bytes required by the caller.
	Return Value: 0 for success, negative for error.
*/
int qPrepareCallbackDataForWrite(QCallbackData* target, int length);


/* qStoreCallbackData:	Copy data from a callback function so that it
						can later be read back from Squeak.
	Arguments:
		source: Pointer to the data to be stored.
		target: Pointer to the QCallbackData that is to store the data.
		length: Number of bytes of data to be stored.
	Return Value: None.
*/
// XXXX: Surely there are some error values that we can return?!?
void qStoreCallbackData(char *source, QCallbackData *target, int length);


/* qReadCallbackData:	Copy data from temporary storage into the place
						where it will actually be used.
	Arguments:
		cbd: Pointer to the QCallbackData that the data will be read from.
		target: Pointer to the memory to copy the data into.
		maxLength: The amount of target memory available.
	Return Value:
		>=0  -  the amount of data that was read
		-2  -  insufficient target memory
		-3  -  invalid data source (no data to read)
	Notes: obviously, both 'cbd' and 'target' must be non-NULL. 
*/
int qReadCallbackData(QCallbackData *source, char *target, int maxLength);


/* qFreeCallbackData:	Free data buffer.
	Arguments:
		cbd: Pointer to the QCallbackData whose buffer is to be freed.
	Return Value: none
	Notes: obviously 'cbd' must be non-NULL. 
*/
void qFreeCallbackData(QCallbackData *cbd);

#ifdef __cplusplus
}
#endif

#endif //#ifndef __Q_READBACK_H__

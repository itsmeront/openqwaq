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

/******************************************************************************
 *
 * qMessageIDs.h
 * QwaqLib (Win32)
 *
 * Generate Windows message-IDs that are shared between the PlatformEventPlugin
 * and various helper applications.
 *
 ******************************************************************************/

#ifndef __Q_MESSAGE_IDS_H__
#define __Q_MESSAGE_IDS_H__

#include <windows.h>

// Call this first before accessing any of the various message-ID accessors below.
void initializeMessageIDs(void);

UINT getFileHelperMessageID(void);

#endif //#ifndef __Q_MESSAGE_IDS_H__

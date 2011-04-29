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

#include "qMessageIDs.h"

static TCHAR* fileHelperMessageText = TEXT("QWAQ_FILE_HELPER_MESSAGE");
static UINT fileHelperMessageID = 0;

void initializeMessageIDs(void)
{
	fileHelperMessageID = RegisterWindowMessage(fileHelperMessageText);
}

UINT getFileHelperMessageID(void) { return fileHelperMessageID; }
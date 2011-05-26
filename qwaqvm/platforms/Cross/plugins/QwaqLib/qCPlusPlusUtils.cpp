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

#include "qCPlusPlusUtils.h"

#include <string>

int stdStringLength(void* externalAddressPtr)
{
	std::string* stringPtr = *((std::string**)externalAddressPtr);
	return stringPtr->length();
}

char* stdStringCStr(void* externalAddressPtr)
{
	std::string* stringPtr = *((std::string**)externalAddressPtr);
	return (char*)(stringPtr->c_str());
}

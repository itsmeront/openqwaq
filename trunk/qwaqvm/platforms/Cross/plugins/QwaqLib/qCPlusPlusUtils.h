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

/******************************************************************************
 *
 * qCPlusPlusUtils.h
 * QwaqLib (cross-platform)
 *
 * Handy for interacting with some C++ objects from plugins.
 *
 ******************************************************************************/

#ifndef __Q_C_PLUS_PLUS_UTILS_H__
#define __Q_C_PLUS_PLUS_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

int stdStringLength(void* externalAddressPtr);
char* stdStringCStr(void* externalAddressPtr);

#ifdef __cplusplus
}
#endif

#endif //#ifndef __Q_C_PLUS_PLUS_UTILS_H__

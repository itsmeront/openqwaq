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
 * QPlatformEventPlugin.h
 * QPlatformEventPlugin (cross-platform)
 *
 ******************************************************************************/


#ifndef __Q_PLATFORM_EVENT_PLUGIN_H__
#define __Q_PLATFORM_EVENT_PLUGIN_H__

#include "qFeedbackChannel-interface.h"
#include "qCPlusPlusUtils.h"
	
#ifdef __cplusplus
extern "C" {
#endif

void qInitModule();
void qShutdownModule();

#ifdef __cplusplus
}
#endif

#endif //#ifndef __Q_PLATFORM_EVENT_PLUGIN_H__

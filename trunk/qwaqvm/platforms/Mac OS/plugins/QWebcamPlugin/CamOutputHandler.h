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
 *  CamOutputHandler.h
 *  QwaqMediaPlugin
 *
 * This private interface, internal to qmCamera.m,
 * defines the decoder-delegate that receives frames from the camera pipeline.
 */

@interface CamOutputHandler : NSObject
{
	int camIndex;
};

- (void) camIndex: (int) index;

@end

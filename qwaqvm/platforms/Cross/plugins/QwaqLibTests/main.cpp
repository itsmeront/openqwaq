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


#include "qLogger.h"

#include "qTestBufferPool.h"
#include "qTestReaderWriter.h"

int main(int argc, char* argv[])
{	
	qInitLogging();
	qerr << endl << "Testing BufferPool... ";
	
	testBufferPool_1();
	testBufferPool_2();
	// The exception is thrown as expected, but then we crash (also as expected).
	// Try it for yourself!
	//testBufferPool_3();
	testBufferPool_4();
	qerr << "success!";
	
	testReaderWriter_1();
}


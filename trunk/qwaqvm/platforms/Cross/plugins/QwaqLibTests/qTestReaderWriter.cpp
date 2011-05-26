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

#include "qTestReaderWriter.h"
#include "qLogger.h"

#include <boost/thread.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/bind.hpp>


const int MAX_WORKERS = 4;
static int global_value;
static int global_read_values[MAX_WORKERS];
boost::shared_mutex global_mutex;

void readThread(int index)
{
	boost::shared_lock<boost::shared_mutex> lk(global_mutex);
	boost::this_thread::sleep(boost::posix_time::milliseconds(300));
	global_read_values[index] = global_value;
} 

void writeThread(int a, int b, int c)
{
	boost::this_thread::sleep(boost::posix_time::milliseconds(10));
	boost::unique_lock<boost::shared_mutex> lk(global_mutex);
	global_value = a * b * c;
}

void testReaderWriter_1(void) 
{
	global_value = 0;
	
	boost::thread writer(boost::bind(&writeThread, 3, 4, 5));
	boost::thread reader0(boost::bind(&readThread,0));
	boost::thread reader1(boost::bind(&readThread,1));
	boost::thread reader2(boost::bind(&readThread,2));
	boost::this_thread::sleep(boost::posix_time::milliseconds(400));
	boost::thread reader3(boost::bind(&readThread,3));
	
	writer.join();
	reader0.join();
	reader1.join();
	reader2.join();
	reader3.join();
	
	bool success = true;
	if (global_read_values[0] != 0) success = false;
	if (global_read_values[1] != 0) success = false;
	if (global_read_values[2] != 0) success = false;
	if (global_read_values[3] != 60) success = false;
	
	qerr << "testReaderWriter_1():  ";
	if (success) qerr << "SUCCESS" << endl;
	else {
		qerr << "FAILURE (writer: " << global_value << "   readers: ";
		for (int i=0; i<MAX_WORKERS; i++)  qerr << global_read_values[i] << ", ";
		qerr << ")" << endl;
	}
}

/*
 *  vcacheopt.cpp
 *  CroquetPlugin
 *
 *  Created by Joshua Gargus on 6/18/10.
 *  Copyright 2010 Teleplace. All rights reserved.
 *
 */

 // Disable this for Windows, because we can't compile C++ with MinGW
 
#ifdef WIN32
 extern "C" {
	int optimizeVertexIndices(int* indices, int triCount) { return -1; }
}
#else

#include "vcacheopt.h"

extern "C" {
	int optimizeVertexIndices(int* indices, int triCount)
	{
		VertexCacheOptimizer vco;
		return (int) vco.Optimize(indices, triCount);	
	}	
}

#endif

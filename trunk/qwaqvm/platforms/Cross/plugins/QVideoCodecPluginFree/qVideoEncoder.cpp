/*
 * Project OpenQwaq
 *
 * Copyright (c) 2011, 3d Immersive Collaboration Consultants, LLC, All Rights Reserved
 *
 * qVideoEncoder.cpp
 * QVideoCodecPluginFree
 *
 * Created by Josh Gargus on 5/29/11.
 */

/* Must declare the decoder struct... may contain API-specific fields. */ 
#include "../QVideoCodecPlugin/qVideoEncoder.h"

#include "../QwaqLib/qBuffer.h"
#include "../QwaqLib/qSharedQueue.h"
#include "../QwaqLib/qLogger.hpp"

using namespace Qwaq;


typedef struct QEncoder { 
	int encoderIndex;
	
	int semaIndex;
	int outFrameCount;
	int inFrameCount;
	int width;
	int height;
	
	BufferPool2 pool;
	SharedQueue<BufferPtr> queue;
} QEncoder;

/* Now that we've defined 'struct QEncoder', we can include 'qVideoEncoder.inc' */
extern "C"
{
#include "../QVideoCodecPlugin/qVideoEncoder.inc"
}

/* Now we must define the API-specific functions declared in the .inc file */

int qCreateEncoderAPI(QEncoder **eptr, char *args, int argsSize, int semaIndex, int width, int height) 
{
	QEncoder *encoder;
	QVideoArgs *vargs = (QVideoArgs*)args;
	
	qerr << endl << "qCreateEncoderFree(): START ENCODER CREATION! (width: " << width << " height: " << height << ")";
	
	encoder = new QEncoder;
	if (encoder == NULL) {
		qerr << endl << "qCreateEncoderFree(): failed to instantiate encoder";
		return -2;
	}
	
	encoder->semaIndex = semaIndex;
	encoder->width = width;
	encoder->height = height;
	encoder->inFrameCount = encoder->outFrameCount = 0;
	
	qerr << endl << "qCreateEncoderFree(): not yet implemented";
	qDestroyEncoderAPI(encoder);
	return -1;
	
	//*eptr = encoder;
	//return 0; // success!
}


void qDestroyEncoderAPI(QEncoder *encoder)
{
	if (encoder == NULL) return;  // ... even though we won't be called if there is no encoder to destroy.
	delete encoder;
}


int qEncodeAPI(QEncoder *encoder, char* bytes, int byteSize) 
{
	qerr << endl << "qEncodeAPI(): not yet implemented";
	return -1;
}


char* qEncoderGetPropertyAPI(QEncoder *encoder, char* propertyName, int* resultSize)
{
	qerr << endl << "qEncoderGetPropertyAPI(): no properties accessible yet";
	*resultSize = 0;
	return NULL;
}
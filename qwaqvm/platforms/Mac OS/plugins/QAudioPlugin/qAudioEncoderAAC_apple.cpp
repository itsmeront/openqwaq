/*
 * Project OpenQwaq
 *
 * Copyright (c) 2011, 3d Immersive Collaboration Consultants, LLC, All Rights Reserved
 *
 */
 
/*
 * qAudioEncoderAAC_apple.cpp
 * QAudioPlugin
 *
 * Created by Josh Gargus on 7/16/11.
 */
 
 #include "QAudioPlugin.h"
 #include "qAudioEncoderAAC_apple.hpp"
 #include "qLogger.hpp"
 
 extern "C" {
 #include <AudioToolbox/AudioToolbox.h>
 #include <CoreAudio/CoreAudio.h>
 }
 
 using namespace Qwaq;
 
 // See Squeak class QAudioEncoderConfigAAC
 // (this is cut'n'pasted several places)
#pragma pack(push, 1)
struct Qwaq::AudioEncoderAAC_config
{
	uint32_t inputChannels;
	uint32_t mpegVersion;
	uint32_t aacObjectType;
	uint32_t audioBitrateIndex;
	uint32_t headerType;
	uint32_t hfCutoff;
	uint32_t vbr;
	uint32_t he;
	uint32_t bitsPerSample;
	uint32_t protectAdtsStream;
	uint32_t reserved[54];
};
#pragma pack(pop)

const int MAX_OUTPUT_BYTES = 2048; // more than enough... this is the same as the uncompressed PCM data
// XXXXX const int MAX_OUTPUT_BYTES = 80000; // way more than enough.
struct Qwaq::AudioEncoderAAC_apple_priv {

	AudioConverterRef converter;
	int inBytesPerPacket;
	int outBytesPerPacket;
	int framesPerPacket;
	int sampleRate;
	int maxPacketSize;
	int numChannels;
	
	// This shouldn't need to be long-lived, but let's make it so anyway.
	AudioStreamPacketDescription odesc;
	AudioBufferList obuflist;
	
	uint8_t buf[MAX_OUTPUT_BYTES];  // more than enough space... this is how much the unencoded audio takes.
	int16_t in_buf[1024];
	unsigned in_size;
	
	AudioEncoderAAC_config config;
};

static AudioEncoderAAC_apple_priv* createPrivateContext(unsigned char* config, unsigned configSize);
static void destroyPrivateContext(AudioEncoderAAC_apple_priv* priv);

AudioEncoderAAC_apple_priv* createPrivateContext(unsigned char* config, unsigned configSize)
{
	// Check sanity of config data.
	AudioEncoderAAC_config* sqc = (AudioEncoderAAC_config*)config;
	if (sqc->mpegVersion != 7) {
		qLog() << "apple AAC createPrivateContext(): unexpected MPEG version: " << sqc->mpegVersion << flush;
		return NULL;
	}
	if (sqc->aacObjectType != 2) { // libav constants are off by one compared to ISO 14496-3-1.5.2.1
		qLog() << "apple AAC createPrivateContext(): only AAC-LC is supported";
		return NULL;
	}
	if (sqc->bitsPerSample != 16) {
		qLog() << "apple AAC createPrivateContext(): only 16-bit input samples are supported" << flush;
		return NULL;
	}

	// Instantiate a private context object.
	AudioEncoderAAC_apple_priv* priv = new AudioEncoderAAC_apple_priv();
	if (!priv) {
		qLog() << "apple AAC createPrivateContext(): cannot instantiate encoder object" << flush;
		return NULL;
	}
		
	AudioStreamBasicDescription input, output;
	bzero(&input, sizeof(AudioStreamBasicDescription));
	bzero(&output, sizeof(AudioStreamBasicDescription));	
	
	input.mSampleRate = 16000.0; // XXXXX: HARDWIRING!!!! 16kHz is all you get
	input.mFormatID = kAudioFormatLinearPCM;
//	input.mFormatFlags = kAudioFormatFlagsNativeEndian | kLinearPCMFormatFlagIsSignedInteger;
	input.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger;
	input.mChannelsPerFrame = sqc->inputChannels;
	input.mBitsPerChannel = 16;
	input.mBytesPerFrame = input.mBitsPerChannel / 8 * input.mChannelsPerFrame;
	input.mFramesPerPacket = 1;
	input.mBytesPerPacket = input.mBytesPerFrame * input.mFramesPerPacket;

	output.mFormatID = kAudioFormatMPEG4AAC;
	output.mSampleRate = input.mSampleRate; // don't convert sample rate
	output.mChannelsPerFrame = sqc->inputChannels;
	// ... let CoreAudio figure out the rest...
	
	// Instantiate CoreAudio AudioConverter
	OSStatus err = AudioConverterNew(&input, &output, &priv->converter);
	if (err != noErr) {
		qLog() << "apple AAC createPrivateContext(): cannot create AudioConverter (err: " << err << ")" << flush;
		destroyPrivateContext(priv);
		return NULL;
	}
	
	// Let's set some parameters, shall we?
	UInt32 tmp, tmpsiz = sizeof(tmp);
	
	// Set encoder quality to maximum
	tmp = kAudioConverterQuality_Max;
	AudioConverterSetProperty(priv->converter, kAudioConverterCodecQuality, tmpsiz, &tmp);
	
	// Variable bitrate, within reason.
//	tmp = kAudioCodecBitRateControlMode_Constant;
	tmp = kAudioCodecBitRateControlMode_VariableConstrained;
	AudioConverterSetProperty(priv->converter, kAudioCodecPropertyBitRateControlMode, tmpsiz, &tmp);
	
	// Prepare to set output bitrate.  The parameter currently passed by Squeak is "4".  This
	// originates from the (IIRC) 1-10 scale used by the MainConcept encoder.  The bitrates 
	// here are pulled out of my ass, but seem reasonable.
	UInt32 bitrate;
	switch(sqc->vbr) {
		case 2: bitrate = 24000; break;
		case 3: bitrate = 32000; break;
		case 4: bitrate = 48000; break;
		case 5: bitrate = 64000; break;
		case 6: bitrate = 80000; break;
		case 7: bitrate = 112000; break;
		case 8: bitrate = 144000; break;
		case 9: bitrate = 192000; break;
		default: bitrate = (sqc->vbr <= 1) ? 16000 : 256000;
	};

	// We may need to further clamp the selected bitrate, since
	// some not all bitrates are available for a specific configuration
	// (sample-rate, number of channels, etc.)
	AudioValueRange bitrates[100];
	err = AudioConverterGetPropertyInfo(priv->converter, kAudioConverterApplicableEncodeBitRates, &tmpsiz, NULL);
	if (tmpsiz > sizeof(bitrates)) {
		// Sanity check... this won't happen.
		qLog() << "apple AAC createPrivateContext(): not enough space to retrieve bitrates" << flush;
		destroyPrivateContext(priv);
		return NULL;;
	}
	err = AudioConverterGetProperty(priv->converter, kAudioConverterApplicableEncodeBitRates, &tmpsiz, bitrates);
	ssize_t bitrateCounts = tmpsiz / sizeof(AudioValueRange);
	if (bitrate < bitrates[0].mMinimum) { 
		bitrate = bitrates[0].mMinimum; 
		qLog() << "... clamped bitrate to " << bitrate;
	}
	else if (bitrate > bitrates[bitrateCounts-1].mMinimum) { 
		bitrate = bitrates[bitrateCounts-1].mMinimum;
		qLog() << "... clamped bitrate to " << bitrate;
	}

	// Finally, we can set the bitrate
	AudioConverterSetProperty(priv->converter, kAudioConverterEncodeBitRate, sizeof(bitrate), &bitrate);

	// Get actual input and output settings.
	tmpsiz = sizeof(input);
	AudioConverterGetProperty(priv->converter, kAudioConverterCurrentInputStreamDescription, &tmpsiz, &input);
	tmpsiz = sizeof(output);
	AudioConverterGetProperty(priv->converter, kAudioConverterCurrentOutputStreamDescription, &tmpsiz, &output);
	tmpsiz = sizeof(tmp);
	AudioConverterGetProperty(priv->converter, kAudioConverterPropertyMaximumOutputPacketSize, &tmpsiz, &tmp);
	
	// Stash actual encoder settings, in case they come in handy later.
	priv->maxPacketSize = tmp;
	priv->outBytesPerPacket = output.mBytesPerPacket;
	priv->framesPerPacket = output.mFramesPerPacket;
	priv->sampleRate = output.mSampleRate;
	priv->numChannels = sqc->inputChannels;

	// Log the actual encoder settings, and verify their sanity.
	qLog() << "created encoder with:"
		<< "\n\t\t\t input/output channels: " << priv->numChannels	
		<< "\n\t\t\t max output packet-size: " << priv->maxPacketSize	
		<< "\n\t\t\t out bytes-per-packet: " << priv->outBytesPerPacket
		<< "\n\t\t\t frames-per-packet: " << priv->framesPerPacket
		<< "\n\t\t\t output sample-rate: " << priv->sampleRate
		<< "\n\t\t\t output bit-rate: " << bitrate;
	if (16000 != priv->sampleRate) {
		qerr << "\n\t\t\t ERROR: UNEXPECTED PARAMETER IN	 CREATED ENCODER";
		destroyPrivateContext(priv);
		return NULL;
	}
	
	
	return priv;
}


void destroyPrivateContext(AudioEncoderAAC_apple_priv* priv)
{
	if (!priv) return;  // nothing to do
	
	if (priv->converter) {
		AudioConverterDispose(priv->converter);
		priv->converter = NULL;
	}
	
	delete priv;
}

 
AudioEncoderAAC_apple::AudioEncoderAAC_apple(FeedbackChannel* feedbackChannel, unsigned char* config, unsigned configSize) 
: AudioEncoder(feedbackChannel), ring(50000), inFrameCount(0), outFrameCount(0)
{
	priv = createPrivateContext(config, configSize);
	if (priv) {
		qLog() << "AudioEncoderAAC_apple(): created encoder" << flush;
	}
	else {
		qLog() << "ERROR AudioEncoderAAC_apple() could not create encoder" << flush;
	}	
}

AudioEncoderAAC_apple::~AudioEncoderAAC_apple()
{	
	if (priv) destroyPrivateContext(priv);
	qLog() << "AudioEncoderAAC_apple(): destroyed encoder" << flush;
}


OSStatus encoderCallback(AudioConverterRef, UInt32*, AudioBufferList*, AudioStreamPacketDescription**, void*);

int AudioEncoderAAC_apple::asyncEncode(short *bufferPtr, int sampleCount)
{
	if (!feedback) {
		qLog() << "AudioEncoderAAC_apple::asyncEncode() ...  missing feedback channel!" << flush;
		return -1;
	}
	if (!priv) {
		qLog() << "AudioEncoderAAC_apple::asyncEncode() ...  missing CoreAudio converter!" << flush;
		return -1;
	}
	
	// We first push the data into a ring-buffer, which allows us to deal with input that isn't a 
	// multiple of 1024.
	try {
		ring.put(bufferPtr, sampleCount*2);
	}
	catch(std::string e) {
		qLog() << "AudioEncoderAAC_apple::asyncEncode() ...  " << e;
		ring.clear();
		return -1;
	}
	
	priv->in_size = priv->numChannels * 1024;
	while (ring.dataSize() > priv->in_size*2) { 
		ring.get(priv->in_buf, priv->in_size*2);

		AudioStreamPacketDescription odesc;
		odesc.mStartOffset = 0;
		odesc.mVariableFramesInPacket = 0;  // since this a constant value (always 1024), we use zero
		odesc.mDataByteSize = 0;
		AudioBufferList obuflist;
		obuflist.mNumberBuffers = 1;
		obuflist.mBuffers[0].mNumberChannels = priv->numChannels;
		obuflist.mBuffers[0].mDataByteSize = MAX_OUTPUT_BYTES;
		obuflist.mBuffers[0].mData = priv->buf; 
				
		UInt32 npackets = 1;
		OSStatus err = AudioConverterFillComplexBuffer( priv->converter, encoderCallback, priv, &npackets, &obuflist, &odesc );

		if (err) {
			qLog() << "AudioEncoderAAC_apple::asyncEncode() ...  encode failed with status: " << err << flush;
			return -1;
		}

		// Fill in the data to be read back by Squeak.
		int encodedSize = obuflist.mBuffers[0].mDataByteSize;
		pushFeedbackData(priv->buf, encodedSize);
	}
	return 0;
}


OSStatus encoderCallback(	AudioConverterRef				inAudioConverter,
							UInt32*							ioNumberDataPackets,
							AudioBufferList*				ioData,
							AudioStreamPacketDescription**	outDataPacketDescription,
							void*							inUserData)
{
	AudioEncoderAAC_apple_priv* priv = (AudioEncoderAAC_apple_priv*)inUserData;

	// Verify arguments.
	if (*ioNumberDataPackets != priv->in_size) {
		qLog() << "AAC encoder expected " << priv->in_size << " samples, but received " << *ioNumberDataPackets;
		return kAudioConverterErr_UnspecifiedError;
	}
	if (outDataPacketDescription != NULL) {
		// Not filling it in will probably result in an error, but we never expect it to be non-NULL.
		qLog() << "AAC encoder did not expect to have to fill in a packet-description";
	}

	ioData->mBuffers[0].mDataByteSize = priv->in_size * 2;
	ioData->mBuffers[0].mData = priv->in_buf; 

	return noErr;
}

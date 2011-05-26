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
 *	v4l2.h QWebcamPlugin video engine interface for linux above Video4Linux2.
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct camera *Camera; /* opaque camera type */

/* indirect reference to an mmap'ed frame buffer in the driver, helping in
 * avoiding an intermediate buffer between Squeak and the driver.
 */
typedef struct {
	Camera camera;
	void *buffer;
	unsigned long outputBitmapSize;
	int bufferIndex;
	unsigned long long timestamp;
} v4l_buffer_desc;

/* image conversion */
int v4l_copyframe_to_rgba32(v4l_buffer_desc *frame, void *bitmap, long bytesiz);

/* Camera device query */
extern int v4ldev_num_video_devices(void);
extern int v4ldev_is_busy(int cameraIndex);
extern char *v4ldev_simple_name(int cameraIndex); /* webcam name */
extern char *v4ldev_device_name(int cameraIndex); /* device name /dev/video0 */

/* Camera control */

/* Attempt to open the default camera (/dev/video0) and if successful allocate
 * a struct Camera, assign it through camerap and answer 0.  If unsuccessful,
 * answer an error code.  Failures include there being no /dev/video0 or the
 * driver not supporting the required interface.
 */
extern int v4l_open(Camera *camerap);

/* Attempt to close the camera referenced by the argument and if successful
 * answer 0, otherwise answer an error code.
 */
extern int v4l_close(Camera camera);
extern int v4l_setfps(Camera camera, int fps);
extern int v4l_getfps(Camera camera);
extern int v4l_setextent(Camera camera, int width, int height);
/* Attempt to set an opened camera into the current format and if successful
 * answer 0, otherwise answer an error code.
 */
extern int v4l_initformat(Camera camera);
/* Attempt to mmap the cameras's frame buffers, answering 0 if successful, or
 * answer 0, otherwise answer an error code.
 */
extern int v4l_mmap(Camera camera);
/* Attempt to start capturing video, answering 0 if successful or an error.
 */
extern int v4l_capture(Camera camera);
/* Attempt to stop capture, answering 0 if successful or an error.
 */
extern int v4l_stop(Camera camera);

/* uses poll to query expected_deadline?? */
extern int v4l_querydataready(Camera camera, int *errp);
/* does DQBUF then MainConcept to convert & copy data then QBUF */
extern int v4l_grabframe(Camera camera, v4l_buffer_desc *frame);
extern int v4l_releaseframe(Camera camera, v4l_buffer_desc *frame);

/* error codes */
#define VE_UNINITIALIZED 1000
#define VE_NOTENOUGHBUFFERS 1001
#define VE_UNIMPLEMENTED 1002
#define VE_TOOSMALL 1003

#ifdef __cplusplus
}
#endif

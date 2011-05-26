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
 *	v4l2.c QWebcamPlugin video engine for linux above Video4Linux2.
 */

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>

#include "v4l2.h"
#if USE_MC /* Use MainConcept for YUYV to RGB conversion */
# include "trans_video_colorspace.h" /* MainConcept, for image conversion */

static void *ucc = NULL; /* MainConcept conversion context */
static void *ucc_buffer = NULL; /* MainConcept conversion buffer */
#endif

#define VIDEO_DEVICE "/dev/video0"

/* Debug tracing. */
#if 1
# define v4l_ioctl(a,b,c) ioctl(a,b,c)
#else
# define v4l_ioctl(a,b,c) dioctl(a,b,#b,c)

static int
dioctl(int fd, int request, char *request_name, void *arg)
{
	int r = ioctl (fd, request, arg);

	printf("\nioctl(%d,%s...) = %d (%d)\n",
			fd, request_name, r, (r < 0 ? errno : 0));
	fflush(stdout);
	return r;
}
#endif /* 0 */


/* Device query. */
int
v4ldev_num_video_devices(void) { return 1; }

/* what should we do about the race condition? */
int
v4ldev_is_busy(int cameraIndex)
{
#if 1 /* http://v4l2spec.bytesex.org/v4l2spec/capture.c uses O_NONBLOCK but */
	  /* mplayer does not. Might not using it avoid checking ioctl for EINTR? */
	int fd = open(VIDEO_DEVICE, O_RDWR | O_NONBLOCK, 0);
#else
	int fd = open(VIDEO_DEVICE, O_RDWR, 0);
#endif

	if (fd >= 0)
		(void)close(fd);
	return fd >= 0;
}

char *
v4ldev_simple_name(int cameraIndex)
{
	int fd;
	static struct v4l2_capability capability_for_name;

	if ((fd = open(VIDEO_DEVICE, O_RDWR | O_NONBLOCK, 0)) < 0
	 || v4l_ioctl(fd, VIDIOC_QUERYCAP, &capability_for_name) < 0)
		strcpy(capability_for_name.card, "unknown");

	(void)close(fd);
	return capability_for_name.card;
}

char *
v4ldev_device_name(int cameraIndex) { return VIDEO_DEVICE; }

/* Camera control.
 *
 * See "Video for Linux Two API Specification" http://v4l2spec.bytesex.org/spec.
 * This depends on a driver supporting the "Streaming I/O (Memory Mapping)"
 * scheme indicated as supported by the V4L2_CAP_STREAMING flag and controlled
 * by the VIDIOC_REQBUFS, VIDIOC_QBUF & VIDIOC_DQBUF ioctls.
 *
 * We request that the driver allocate some number of frame buffers in real
 * memory that we mmap into our address space.  We then use the VIDIOC_QBUF
 * ioctl to allow the driver to write to a buffer, and the  VIDIOC_DQBUF ioctl
 * to save a buffer for reading.  We use poll to find out when a buffer is
 * available.
 */


/* http://v4l2spec.bytesex.org/spec/x5791.htm */
struct mmapped_fb { void *start; size_t length; };

struct camera {
	char *devname;
	int fd, nfbs, running, fps, width, height;
	struct v4l2_capability	cap;
	struct v4l2_format		format;
	struct mmapped_fb		*mmapped_fbs;
	struct timeval			start_time;
};

int
v4l_open(Camera *camerap)
{
	Camera c;
	int err;

	if (!(*camerap = c = calloc(1, sizeof(*c))))
		return ENOMEM;

	c->devname = VIDEO_DEVICE;

	if ((c->fd = open(c->devname, O_NONBLOCK)) < 0) {
abort1:	err = errno;
abort2:	*camerap = 0;
		if (c->fd >= 0)
			(void)close(c->fd);
		free(c);
		return err;
	}

	if (v4l_ioctl(c->fd, VIDIOC_QUERYCAP, &c->cap) < 0)
		goto abort1;

	if ((c->cap.capabilities & (V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING))
	  != (V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING)) {
		err = ENODEV;
		goto abort2;
	}

	return 0;
}

static void
unmap_and_free_buffers(Camera camera)
{
	int i;

	for (i = 0; i < camera->nfbs; i++)
		(void)munmap(camera->mmapped_fbs[i].start,
					 camera->mmapped_fbs[i].length);
	(void)free(camera->mmapped_fbs);
	camera->mmapped_fbs = 0;
	camera->nfbs = 0;
}

/* Attempt to close the camera referenced by the argument and if successful
 * answer 0, otherwise answer an error code.
 */
int
v4l_close(Camera camera)
{
	int err = 0;

	if (camera->fd >= 0) {
		v4l_stop(camera);
		err = close(camera->fd);
	}
	unmap_and_free_buffers(camera);
	(void)free(camera);
#if USE_MC
	if (ucc) {
		UCC_Destroy(ucc);
		ucc = 0;
	}
	if (ucc_buffer) {
		(void)free(ucc_buffer);
		ucc_buffer = 0;
	}
#endif /* USE_MC */
	return err;
}

int
v4l_setfps(Camera camera, int fps)
{
	camera->fps = fps;
	return 0;
}

int
v4l_getfps(Camera camera) { return camera->fps; }

int
v4l_setextent(Camera camera, int width, int height)
{
	camera->width = width;
	camera->height = height;
	return 0;
}

/* Attempt to set an opened camera into the current format and if successful
 * answer 0, otherwise answer an error code.
 */
int
v4l_initformat(Camera camera)
{
	memset(&camera->format, 0, sizeof(camera->format));
	camera->format.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (!camera->width || !camera->height) {
		camera->format.fmt.pix.width   = 320;
		camera->format.fmt.pix.height  = 240;
	}
	else {
		camera->format.fmt.pix.width   = camera->width;
		camera->format.fmt.pix.height  = camera->height;
	}
	camera->format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	camera->format.fmt.pix.field       = V4L2_FIELD_INTERLACED;

	if (v4l_ioctl(camera->fd, VIDIOC_S_FMT, &camera->format) < 0) {
		int err = errno;
		perror("ioctl VIDIOC_QUERYBUF");
		return err;
	}
	assert(camera->format.fmt.pix.pixelformat == V4L2_PIX_FMT_YUYV);
	return 0;
}

/* Attempt to mmap the cameras's frame buffers, answering 0 if successful, or
 * an error.  See http://v4l2spec.bytesex.org/spec/x5791.htm.
 */
int
v4l_mmap(Camera camera)
{
	int err, frames_to_buffer, i;
	struct v4l2_requestbuffers reqbuf;

	if (camera->mmapped_fbs)
		return 0;

	if (!camera->fps)
		return VE_UNINITIALIZED;
	frames_to_buffer = (camera->fps + 2) / 3;

	memset(&reqbuf, 0, sizeof(reqbuf));
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbuf.memory = V4L2_MEMORY_MMAP;
	reqbuf.count = frames_to_buffer;
	if (v4l_ioctl(camera->fd, VIDIOC_REQBUFS, &reqbuf))
		return errno;

	if (reqbuf.count < ((frames_to_buffer * 2) / 3))
		return VE_NOTENOUGHBUFFERS;

	camera->nfbs = reqbuf.count;
	camera->mmapped_fbs = calloc(camera->nfbs, sizeof(*camera->mmapped_fbs));
	if (!camera->mmapped_fbs) {
		camera->nfbs = 0;
		return ENOMEM;
	}

	for (i = 0; i < camera->nfbs; i++) {
		struct v4l2_buffer buffer;
		int count = 0;

		memset(&buffer, 0, sizeof(buffer));
		buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buffer.memory = V4L2_MEMORY_MMAP;
		buffer.index = i;

		if (v4l_ioctl(camera->fd, VIDIOC_QUERYBUF, &buffer) < 0) {
			err = errno;
			perror("ioctl VIDIOC_QUERYBUF");
			camera->nfbs = i - 1;
			unmap_and_free_buffers(camera);
			return err;
		}

		camera->mmapped_fbs[i].length = buffer.length; /* remember for munmap */
#if 0
		do {
			camera->mmapped_fbs[i].start = mmap(NULL,
											buffer.length,
											PROT_READ|PROT_WRITE,/*recommended*/
# if 0
		/* this answers EACCES */			MAP_SHARED,			 /*recommended*/
# elif 1
		/* this answers EAGAIN */			MAP_PRIVATE | MAP_LOCKED,
# else
		/* this answers EINVAL */			MAP_PRIVATE,
# endif
											camera->fd,
											buffer.m.offset);

			if (MAP_FAILED == camera->mmapped_fbs[i].start)
				if (errno != EAGAIN
				 || ++count >= 10) {
					err = errno;
					perror("mmap");
					camera->nfbs = i - 1;
					unmap_and_free_buffers(camera);
					return err;
				}
		} while (MAP_FAILED == camera->mmapped_fbs[i].start);
#else
		camera->mmapped_fbs[i].start = mmap(NULL,
											buffer.length,
											PROT_READ|PROT_WRITE,/*recommended*/
# if 0
		/* this answers EACCES */			MAP_SHARED,			 /*recommended*/
# else
											MAP_PRIVATE,
# endif
											camera->fd,
											buffer.m.offset);

		if (MAP_FAILED == camera->mmapped_fbs[i].start) {
			err = errno;
			perror("mmap");
			camera->nfbs = i - 1;
			unmap_and_free_buffers(camera);
			return err;
		}
#endif
	}
	return 0;
}

/* Attempt to start capturing video, answering 0 if successful or an error.
 * See http://v4l2spec.bytesex.org/spec/r13817.htm.
 *
 * N.B. Still to do: set the frame rate.
 * See http://v4l2spec.bytesex.org/spec/r11680.htm#V4L2-CAPTUREPARM
 * The driver supports it if it has the V4L2_CAP_TIMEPERFRAME capability (which
 * it does not with the HP EliteBook 8530p's built-in camera).
 *
 * So we need to implement dropping frames, e.g. in v4l_querydataready below.
 */
int
v4l_capture(Camera camera)
{
	int i, err;
	struct v4l2_buffer enqueue_request;
	enum v4l2_buf_type type;
	struct timeval now;

	if (camera->running)
		return 0;

	if (!camera->nfbs)
		return VE_UNINITIALIZED;

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (v4l_ioctl(camera->fd, VIDIOC_STREAMON, &type) < 0) {
		err = errno;
		perror("ioctl VIDIOC_STREAMON");
		return err;
	}
	(void)gettimeofday(&now,0);
	memset(&enqueue_request,0,sizeof(enqueue_request));
	enqueue_request.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	enqueue_request.memory = V4L2_MEMORY_MMAP;
	for (i = err = 0; i < camera->nfbs; i++) {
		enqueue_request.index = i;
		if (v4l_ioctl(camera->fd, VIDIOC_QBUF, &enqueue_request) < 0) {
			if (!err)
				err = errno;
			perror("ioctl VIDIOC_QBUF");
		}
	}
	camera->running = 1;
	camera->start_time = now;
	return 0;
}

/* Attempt to stop capturing video, answering 0 if successful or an error.
 * See http://v4l2spec.bytesex.org/spec/r13817.htm.
 */
int
v4l_stop(Camera camera)
{
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (!camera->running)
		return 0;

	if (v4l_ioctl(camera->fd, VIDIOC_STREAMOFF, &type) < 0) {
		int err = errno;
		perror("ioctl VIDIOC_STREAMOFF");
		return err;
	}
	camera->running = 0;
	return 0;
}

/* Poll the camera, answering if a frame is available.
 */
int
v4l_querydataready(Camera camera, int *errp)
{
	struct pollfd fd;
	int result;

	if (!camera->fd) {
		*errp = EBADF;
		return 0;
	}
	*errp = 0;
	if (!camera->running)
		return 0;
	fd.fd = camera->fd;
	fd.events = POLLIN | POLLPRI;
	if ((result = poll(&fd, 1, 0)) >= 0)
		return result;

	*errp = errno;
	perror("poll");
	return 0;
}

/* Dequeue a frame from the camera's current set.  Assumes a frame is available,
 * which should have been determined using v4l_querydataready.
 */
int
v4l_grabframe(Camera camera, v4l_buffer_desc *frame)
{
	struct v4l2_buffer dequeue_request;

	memset(&dequeue_request,0,sizeof(dequeue_request));
	dequeue_request.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	dequeue_request.memory = V4L2_MEMORY_MMAP;
	if (v4l_ioctl(camera->fd, VIDIOC_DQBUF, &dequeue_request) < 0) {
		int err = errno;
		perror("ioctl VIDIOC_DQBUF");
		return err;
	}

#define usecs(tv) ((unsigned long long)((tv).tv_sec) * 1000000UL \
				 + (unsigned long long)((tv).tv_usec))

	frame->camera = camera;
	frame->timestamp = usecs(dequeue_request.timestamp)
					 - usecs(camera->start_time);
	frame->buffer = camera->mmapped_fbs[dequeue_request.index].start;
	frame->bufferIndex = dequeue_request.index;
	frame->outputBitmapSize = camera->width * camera->height * sizeof(int);
	return 0;
}

/* Enqueue a frame once its contents have been consumed.
 */
int
v4l_releaseframe(Camera camera, v4l_buffer_desc *frame)
{
	struct v4l2_buffer enqueue_request;
	int err = 0;

	memset(&enqueue_request,0,sizeof(enqueue_request));
	enqueue_request.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	enqueue_request.memory = V4L2_MEMORY_MMAP;
	enqueue_request.index = frame->bufferIndex;
	if (v4l_ioctl(camera->fd, VIDIOC_QBUF, &enqueue_request) < 0) {
		err = errno;
		perror("ioctl VIDIOC_QBUF");
	}
	return err;
}

/* Copy a frame's image into an 32-bpp RGBA bitmap. */
#if USE_MC
/* Use the MainConcept framework's UCC_Transform to do the grunt work.
 */
#define MC_BYTE_ALIGNMENT 16
#define MC_ALIGNMENT_MASK (MC_BYTE_ALIGNMENT - 1)

static inline void *
ensure_aligned_buffer(unsigned long width, unsigned long height)
{
static unsigned long buffer_width, buffer_height;
static void *aligned_buffer;

	if (buffer_width != width
	 || buffer_height != height) {
		if (ucc_buffer)
			(void)free(ucc_buffer);
		ucc_buffer = malloc(width * height * 4 + MC_BYTE_ALIGNMENT);
		aligned_buffer = (void *)(((unsigned long)ucc_buffer + MC_ALIGNMENT_MASK) & ~MC_ALIGNMENT_MASK);
	}
	return aligned_buffer;
}

int
v4l_copyframe_to_rgba32(v4l_buffer_desc *frame, void *bitmap, long bytesiz)
{
	ucc_frame_t input, output;
	ucc_config_t config;
	Camera camera = frame->camera;

	if (!ucc) {
		ucc = UCC_Create(NULL);
		if (!ucc)
			return ENOMEM; /* presumptuous */
	}

	memset(&input, 0, sizeof(input));
	memset(&output, 0, sizeof(output));
	memset(&config, 0, sizeof(config));

	output.pixel_format = UCC_PIXEL_FORMAT_B8G8R8A8_32;
	output.width = camera->width;
	output.height = camera->height;
	output.stride[0] = 4 * output.width;
	output.plane[0] = ((unsigned long)bitmap & MC_ALIGNMENT_MASK)
						? ensure_aligned_buffer(output.width,output.height)
						: bitmap;
	output.video_full_range_flag = 1;
	output.progressive_frame_flag = 1;

	if (bytesiz < output.height * output.stride[0])
		return VE_TOOSMALL;

	switch (camera->format.fmt.pix.pixelformat) {
	case V4L2_PIX_FMT_YUYV:
		input.pixel_format = UCC_PIXEL_FORMAT_YUYV;
		input.width = camera->width;
		input.height = camera->height;
		input.stride[0] = 2 * input.width;
		input.plane[0] = frame->buffer;
		input.colorimetry.color_primaries = UCC_COLOR_PRIMARIES_BT709_5;
		input.colorimetry.transfer_characteristics = UCC_TRANSFER_CHARACTERISTICS_BT709_5;
		input.colorimetry.matrix_coefficients = UCC_MATRIX_COEFFICIENTS_BT709_5_System_1125;
		input.video_full_range_flag = 1;
		input.progressive_frame_flag = 1;
		break;
	default:
		fprintf(stderr,
				"unimplemented conversion for pixel format %x\n",
				camera->format.fmt.pix.pixelformat);
		return VE_UNIMPLEMENTED;
	}
	UCC_Transform(ucc, &output, &input, &config);
	if (output.plane[0] != bitmap)
		memcpy(bitmap, output.plane[0], output.height * output.stride[0]);
	return 0;
}
#else /* USE_MC */
int
v4l_copyframe_to_rgba32(v4l_buffer_desc *frame, void *bitmap, long bytesiz)
{
	Camera camera = frame->camera;

	switch (camera->format.fmt.pix.pixelformat) {
	case V4L2_PIX_FMT_YUYV: {
		/* See e.g.
		 *		http://www.fourcc.org/fccyvrgb.php
		 *		http://en.wikipedia.org/wiki/YUYV
		 *		http://www.virtualdub.org/blog/pivot/entry.php?id=138
		 */
		unsigned long *input = frame->buffer;	/* two pixels of YUYV/2 */
		unsigned long *output = bitmap;			/* one pixel of ARGB */
		long n = bytesiz;

		assert((camera->width % 2) == 0);
		assert((bytesiz % 8) == 0);

		while ((n -= 8) >= 0) {
			unsigned long yuyv = *input++;
			long y0 = (yuyv & 0xFF) - 16;
			long y1 = ((yuyv >> 16) & 0xFF) - 16;
			long u_Cb = ((yuyv >> 8) & 0xFF) - 0x80;
			long v_Cr = ((yuyv >> 24) & 0xFF) - 0x80;
			double r_minus_y = 1.596 * v_Cr;
			double g_minus_y = -0.813 * v_Cr - 0.391 * u_Cb;
			double b_minus_y = 2.018 * u_Cb;

			double lum;
			long r, g, b;

#define clipassign(d,v,l) if ((d = v + l) > 255) d = 255; else if (d < 0) d = 0

			lum = 1.164 * y0;
			clipassign(r,r_minus_y, lum);
			clipassign(g,g_minus_y, lum);
			clipassign(b,b_minus_y, lum);
			*output++ = b + (g << 8) + (r << 16);

			lum = 1.164 * y1;
			clipassign(r,r_minus_y, lum);
			clipassign(g,g_minus_y, lum);
			clipassign(b,b_minus_y, lum);
			*output++ = b + (g << 8) + (r << 16);
		}
		break;
	}
	default:
		fprintf(stderr,
				"unimplemented conversion for pixel format %x\n",
				camera->format.fmt.pix.pixelformat);
		return VE_UNIMPLEMENTED;
	}
}
#endif /* USE_MC */

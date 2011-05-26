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

 #include <stdio.h>
#include "libnsgif.h"
#include "GIFLibPlugin.h"

#define DBG(str) 
/* MessageBox(0, str, "GIFLibPlugin", MB_OK) */

int gifPluginVersion(void) {
  return 0; /* means: unspecified but available */
}


static int formWidth = 0;
static int formHeight = 0;
static char* formBits = NULL;

static void *bitmap_create(int width, int height) {
  DBG("bitmap_create");
  if(width == formWidth && height == formHeight)
    return formBits;
  return NULL;
}


void bitmap_set_opaque(void *bitmap, bool opaque) {
  DBG("bitmap_set_opaque");
}


bool bitmap_test_opaque(void *bitmap){
  DBG("bitmap_test_opaque");
  return false;
}


unsigned char *bitmap_get_buffer(void *bitmap) {
  DBG("bitmap_get_buffer");
  return bitmap;
}


void bitmap_destroy(void *bitmap) {
  DBG("bitmap_destroy");
  formBits = NULL;
}


void bitmap_modified(void *bitmap) {
  DBG("bitmap_modified");
}


/* gifReadImage: Reads an image from a memory buffer.
   Returns: non-zero if successful.
*/
int gifReadImage(int w, int h, int d, char* bits, char *data, int nBytes) {
  gif_bitmap_callback_vt bitmap_callbacks = {
    bitmap_create,
    bitmap_destroy,
    bitmap_get_buffer,
    bitmap_set_opaque,
    bitmap_test_opaque,
    bitmap_modified
  };

  gif_animation gif;
  gif_result code;

  formWidth = w;
  formHeight = h;
  formBits = bits;

  /* create our gif animation */
  gif_create(&gif, &bitmap_callbacks);

  /* begin decoding */
  do {
    code = gif_initialise(&gif, nBytes, data);
    if (code != GIF_OK && code != GIF_WORKING) {
      DBG("gif_initialise failure");
      return 0;
    }
  } while (code != GIF_OK);

  if(gif.width != w || gif.height != h || gif.frame_count != 1 || d != 32) {
    DBG("image format mismatch");
    return 0;
  }

  code = gif_decode_frame(&gif, 0);
  if (code != GIF_OK) {
    DBG("gif_decode_frame error");
    return 0;
  }

  /* clean up */
  gif_finalise(&gif);

  return 1;
}

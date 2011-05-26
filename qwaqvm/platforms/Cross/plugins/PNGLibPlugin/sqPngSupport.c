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
#include "png.h"
#include "PNGLibPlugin.h"

#define PNG_BYTES_TO_CHECK 8

#define DBG(str)

typedef struct pngReadState {
  char *data;
  int length;
  int position;
} pngReadState;

int pngPluginVersion(void) {
  /* should probably return the libpng version but where to find? */
  return 0; /* means: unspecified but available */
}

/* readBytes: Wrapper for in-memory read of data */
static void readBytes(png_structp png_ptr, png_bytep data, png_size_t length) {
  pngReadState *rs = (pngReadState*) png_ptr->io_ptr;

  if(rs->position + length > rs->length) {
    /* read beyond end of file - how do we handle this??? */
    png_error(png_ptr, "Read beyond end of file");
  }
  memcpy(data, rs->data + rs->position, length);
  rs->position += length;
}

/* pngReadImage: Reads an image from a memory buffer.
   Returns: non-zero if successful.
*/
int pngReadImage(int w, int h, int d, char* bits, char *data, int nBytes) {
  png_bytep *row_pointers = NULL;
  pngReadState rs;
  png_structp png_ptr;
  png_infop info_ptr;
  png_uint_32 width, height;
  int bit_depth, color_type, interlace_type;

  DBG("pngReadImage: png_sig_cmp");
  /* Compare the first PNG_BYTES_TO_CHECK bytes of the signature. */
  if(png_sig_cmp(data, (png_size_t)0, PNG_BYTES_TO_CHECK)) return 0;

  DBG("pngReadImage: png_create_read_struct");
  /* Create the png_struct */
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png_ptr == NULL) return 0;

  DBG("pngReadImage: png_create_info_struct");
  /* Allocate/initialize the image information data.  REQUIRED */
  info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == NULL) {
    png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
    return 0;
  }

  /* Set error handling.  REQUIRED if you aren't supplying your own
   * error handling functions in the png_create_read_struct() call.
   */
  if (setjmp(png_jmpbuf(png_ptr))) {
    DBG("pngReadImage: triggered png_error");
    /* If we get here, we had a problem reading the file */
    png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
    if(row_pointers) free(row_pointers);
    return 0;
  }
  rs.data = data;
  rs.position = 0;
  rs.length = nBytes;
  png_set_read_fn(png_ptr, (void *)&rs, readBytes);


  DBG("pngReadImage: png_read_info");
  /* The call to png_read_info() gives us all of the information from the
   * PNG file before the first IDAT (image data chunk).  REQUIRED
   */
  png_read_info(png_ptr, info_ptr);
  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
	       &interlace_type, int_p_NULL, int_p_NULL);

  /* Set up the data transformations you want.  Note that these are all
   * optional.  Only call them if you want/need them.  Many of the
   * transformations only work on specific types of images, and many
   * are mutually exclusive.
   */

  /* tell libpng to strip 16 bit/color files down to 8 bits/color */
  png_set_strip_16(png_ptr);

   /* flip the RGB pixels to BGR (or RGBA to BGRA) */
  if (color_type & PNG_COLOR_MASK_COLOR) {
    DBG("pngReadImage: png_set_bgr");
    png_set_bgr(png_ptr);
  }

  if(d == 32 && color_type == PNG_COLOR_TYPE_RGB) {
    /* Add filler (or alpha) byte (before/after each RGB triplet) */
    DBG("pngReadImage: png_set_filler");
    png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
  }

  if(0) {
    /* Extract multiple pixels with bit depths of 1, 2, and 4 from a single
     * byte into separate bytes (useful for paletted and grayscale images).
     */
    png_set_packing(png_ptr);
    
    /* Change the order of packed pixels to least significant bit first
     * (not useful if you are using png_set_packing). */
    png_set_packswap(png_ptr);

    /* Expand paletted colors into true RGB triplets */
    if (color_type == PNG_COLOR_TYPE_PALETTE)
      png_set_palette_to_rgb(png_ptr);
  }
  
  /* Expand grayscale images to the full 8 bits from 1, 2, or 4 bits/pixel */
  if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    png_set_gray_1_2_4_to_8(png_ptr);

  /* Expand paletted or RGB images with transparency to full alpha channels
   * so the data will be available as RGBA quartets.
   */
  if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(png_ptr);


  if(0) {
    /* Set the background color to draw transparent and alpha images over.
     * It is possible to set the red, green, and blue components directly
     * for paletted images instead of supplying a palette index.  Note that
     * even if the PNG file supplies a background, you are not required to
     * use it - you should use the (solid) application background if it has one
     */

    png_color_16 my_background, *image_background;
  
    if (png_get_bKGD(png_ptr, info_ptr, &image_background))
      png_set_background(png_ptr, image_background,
			 PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
    else
      png_set_background(png_ptr, &my_background,
			 PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);
  }

  if(0) {
   /* Tell libpng to handle the gamma conversion for you.  The final call
    * is a good guess for PC generated images, but it should be configurable
    * by the user at run time by the user.  It is strongly suggested that
    * your application support gamma correction.
    */

    int intent;
    double screen_gamma = 1.2;

    if (png_get_sRGB(png_ptr, info_ptr, &intent))
      png_set_gamma(png_ptr, screen_gamma, 0.45455);
    else {
      double image_gamma;
      if (png_get_gAMA(png_ptr, info_ptr, &image_gamma))
	png_set_gamma(png_ptr, screen_gamma, image_gamma);
      else
	png_set_gamma(png_ptr, screen_gamma, 0.45455);
    }
  }

  if(0) {
    /* If you want to shift the pixel values from the range [0,255] or
     * [0,65535] to the original [0,7] or [0,31], or whatever range the
     * colors were originally in:
     */
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_sBIT)) {
      png_color_8p sig_bit;
      png_get_sBIT(png_ptr, info_ptr, &sig_bit);
      png_set_shift(png_ptr, sig_bit);
    }
  }

  if(0) {
    /* flip the RGB pixels to BGR (or RGBA to BGRA) */
    if (color_type & PNG_COLOR_MASK_COLOR)
      png_set_bgr(png_ptr);
    
    /* swap the RGBA or GA data to ARGB or AG (or BGRA to ABGR) */
    png_set_swap_alpha(png_ptr);
    
    /* swap bytes of 16 bit files to least significant byte first */
    png_set_swap(png_ptr);
  }

  { /* initialize the png row pointers */
    int row, ppw = 32 / d;
    int pitch = ((w + ppw - 1) / ppw) * 4;
    int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

    if(0) {
      char info[100];
      sprintf(info, "Form pitch: %d\nPNG rowbytes: %d",pitch, rowbytes);
      DBG(info);
    }

    /* XXXX: It seems that this test is pretty pointless; PNG appears
       to return the raw rowbytes value, not the padded one.
       A better test is probably to compare the w/h/d values
       of png with the form values to ensure correctness. */
    if(pitch < rowbytes) {
      png_error(png_ptr, "Row bytes mismatch");
    }
    row_pointers = (png_bytep*) calloc(h, sizeof(void*));
    for (row = 0; row < h; row++) {
      row_pointers[row] = bits + (row*pitch);
    }
  }

  DBG("pngReadImage: png_read_image");
  /* Read the entire image in one go */
  png_read_image(png_ptr, row_pointers);

  DBG("pngReadImage: Cleaning up");
  /* clean up after the read, and free any memory allocated - REQUIRED */
  png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);

  if(row_pointers) free(row_pointers);

  return 1;
}

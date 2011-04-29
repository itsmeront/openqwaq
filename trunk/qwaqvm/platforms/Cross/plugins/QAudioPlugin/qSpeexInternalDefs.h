/**
 * Project OpenQwaq
 *
 * Copyright (c) 2005-20011, Teleplace, Inc., All Rights Reserved
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
 *  qSpeexInternalDefs.h
 *  QAudioPlugin
 *
 *  DANGER: contains internal definitions of opaque Speex data structures.  
 *  If you use these, you have better make sure that they correspond to the
 *  same Speex version as your binary libraries.
 *
 */


#include <speex/speex_jitter.h>
#include <speex/speex.h>

#define SPEEX_JITTER_MAX_BUFFER_SIZE 200
#define MAX_TIMINGS 40
#define MAX_BUFFERS 3

struct TimingBuffer {
   int filled;                         /**< Number of entries occupied in "timing" and "counts"*/
   int curr_count;                     /**< Number of packet timings we got (including those we discarded) */
   spx_int32_t timing[MAX_TIMINGS];    /**< Sorted list of all timings ("latest" packets first) */
   spx_int16_t counts[MAX_TIMINGS];    /**< Order the packets were put in (will be used for short-term estimate) */
};

/** Jitter buffer structure */
struct JitterBuffer_ {
   spx_uint32_t pointer_timestamp;                             /**< Timestamp of what we will *get* next */
   spx_uint32_t last_returned_timestamp;                       /**< Useful for getting the next packet with the same timestamp (for fragmented media) */
   spx_uint32_t next_stop;                                     /**< Estimated time the next get() will be called */
   
   spx_int32_t buffered;                                       /**< Amount of data we think is still buffered by the application (timestamp units)*/
   
   JitterBufferPacket packets[SPEEX_JITTER_MAX_BUFFER_SIZE];   /**< Packets stored in the buffer */
   spx_uint32_t arrival[SPEEX_JITTER_MAX_BUFFER_SIZE];         /**< Packet arrival time (0 means it was late, even though it's a valid timestamp) */
   
   void (*destroy) (void *);                                   /**< Callback for destroying a packet */

   spx_int32_t delay_step;                                     /**< Size of the steps when adjusting buffering (timestamp units) */
   spx_int32_t concealment_size;                               /**< Size of the packet loss concealment "units" */
   int reset_state;                                            /**< True if state was just reset        */
   int buffer_margin;                                          /**< How many frames we want to keep in the buffer (lower bound) */
   int late_cutoff;                                            /**< How late must a packet be for it not to be considered at all */
   int interp_requested;                                       /**< An interpolation is requested by speex_jitter_update_delay() */
   int auto_adjust;                                            /**< Whether to automatically adjust the delay at any time */
   
   struct TimingBuffer _tb[MAX_BUFFERS];                       /**< Don't use those directly */
   struct TimingBuffer *timeBuffers[MAX_BUFFERS];              /**< Storing arrival time of latest frames so we can compute some stats */
   int window_size;                                            /**< Total window over which the late frames are counted */
   int subwindow_size;                                         /**< Sub-window size for faster computation  */
   int max_late_rate;                                          /**< Absolute maximum amount of late packets tolerable (in percent) */
   int latency_tradeoff;                                       /**< Latency equivalent of losing one percent of packets */
   int auto_tradeoff;                                          /**< Latency equivalent of losing one percent of packets (automatic default) */
   
   int lost_count;                                             /**< Number of consecutive lost packets  */
};

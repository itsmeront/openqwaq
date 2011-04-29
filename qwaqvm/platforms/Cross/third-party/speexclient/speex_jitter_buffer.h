/* Copyright (C) 2002 Jean-Marc Valin */
/**
   @file speex_jitter_buffer.h
   @brief Adaptive jitter buffer for Speex packets only
*/

#include <speex/speex_jitter.h>
#include <speex/speex.h>

/** @defgroup SpeexJitter SpeexJitter: Adaptive jitter buffer specifically for Speex
 *  This is the jitter buffer that reorders UDP/RTP packets and adjusts the buffer size
 * to maintain good quality and low latency. This is a simplified version that works only
 * with Speex, but is much easier to use.
 *  @{
*/

#ifdef __cplusplus
extern "C" {
#endif

/** Speex jitter-buffer state. Never use it directly! */
typedef struct SpeexJitter {
   SpeexBits current_packet;         /**< Current Speex packet */
   int valid_bits;                   /**< True if Speex bits are valid */
   JitterBuffer *packets;            /**< Generic jitter buffer state */
   void *dec;                        /**< Pointer to Speex decoder */
   spx_int32_t frame_size;           /**< Frame size of Speex decoder */
   int activity_threshold;           /**< Activity threshold below which to jitter_buffer_update_delay() */
} SpeexJitter;

/** Initialise jitter buffer 
 * 
 * @param jitter State of the Speex jitter buffer
 * @param decoder Speex decoder to call
 * @param sampling_rate Sampling rate used by the decoder
*/
void speex_jitter_init(SpeexJitter *jitter, void *decoder, int sampling_rate);

/** Destroy jitter buffer */
void speex_jitter_destroy(SpeexJitter *jitter);

/** Put one packet into the jitter buffer */
void speex_jitter_put(SpeexJitter *jitter, char *packet, int len, int timestamp, void* userData);

/** Get one packet from the jitter buffer.  Return:
 *    0 - if a valid packet was decoded from the JB
 *    1 - if there was enough data already in the current SpeexBits to decode
 *        another frame without needing to remove one from the jitter-buffer
 *    2 - if no packet was available, so we had to extrapolate one
 *    3 - if packet was obtained, but Speex decode failed (so 'out' is filled with 0s)
*/
int speex_jitter_get(SpeexJitter *jitter, spx_int16_t *out, int *start_offset, void** userData);

/** Get pointer timestamp of jitter buffer */
int speex_jitter_get_pointer_timestamp(SpeexJitter *jitter);

#ifdef __cplusplus
}
#endif

/* @} */

#include <speex/speex_jitter.h>
#include "speex_jitter_buffer.h"

#ifndef NULL
#define NULL 0
#endif


void speex_jitter_init(SpeexJitter *jitter, void *decoder, int sampling_rate)
{
   jitter->dec = decoder;
   speex_decoder_ctl(decoder, SPEEX_GET_FRAME_SIZE, &jitter->frame_size);

   jitter->packets = jitter_buffer_init(jitter->frame_size);

   speex_bits_init(&jitter->current_packet);
   jitter->valid_bits = 0;
   jitter->activity_threshold = 0;
}

void speex_jitter_destroy(SpeexJitter *jitter)
{
   jitter_buffer_destroy(jitter->packets);
   speex_bits_destroy(&jitter->current_packet);
}

void speex_jitter_put(SpeexJitter *jitter, char *packet, int len, int timestamp, void* userData)
{
   JitterBufferPacket p;
   p.data = packet;
   p.len = len;
   p.timestamp = timestamp;
   p.span = jitter->frame_size;
   p.user_data = (spx_uint32_t)userData;
   
   jitter_buffer_put(jitter->packets, &p);
}

int speex_jitter_get(SpeexJitter *jitter, spx_int16_t *out, int *current_timestamp, void** userData)
{
   int i;
   int jbRet;	/* results returned by the JB */
   int ourRet;	/* result that we will return */
   spx_int32_t activity;
   char data[2048];
   JitterBufferPacket packet;
   packet.data = data;
   packet.len = 2048;  /* AAAAARGH: it took a week to find and add this missing line */
   
   if (jitter->valid_bits)
   {
      /* Try decoding last received packet */
      jbRet = speex_decode_int(jitter->dec, &jitter->current_packet, out);
      if (jbRet == 0)
      {
         jitter_buffer_tick(jitter->packets);
         return 1;
      } else {
         jitter->valid_bits = 0;
      }
   }

   jbRet = jitter_buffer_get(jitter->packets, &packet, jitter->frame_size, NULL);
   
   if (jbRet != JITTER_BUFFER_OK)
   {
	  /* no packet found, so no corresponding user-data */
      *userData = NULL;

      /* No packet found... extrapolate one */

      /*fprintf (stderr, "lost/late frame\n");*/
      /*Packet is late or lost*/
      speex_decode_int(jitter->dec, NULL, out);
	  
	  ourRet = 2;
   } else {
	  /* found a packet, so there is corresponding user-data */
	  *userData = (void*)(packet.user_data);
   
      speex_bits_read_from(&jitter->current_packet, packet.data, packet.len);
      /* Decode packet */
      jbRet = speex_decode_int(jitter->dec, &jitter->current_packet, out);
      if (jbRet == 0) {
         ourRet = 0;		
         jitter->valid_bits = 1;
      } else {
         /* Error while decoding */
         ourRet = 3;
         for (i=0;i<jitter->frame_size;i++) out[i]=0;
      }
   }
   speex_decoder_ctl(jitter->dec, SPEEX_GET_ACTIVITY, &activity);
   if (activity < jitter->activity_threshold) 
      jitter_buffer_update_delay(jitter->packets, &packet, NULL); 
   jitter_buffer_tick(jitter->packets);
   return ourRet;
}

int speex_jitter_get_pointer_timestamp(SpeexJitter *jitter)
{
   return jitter_buffer_get_pointer_timestamp(jitter->packets);
}

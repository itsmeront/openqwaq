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

// XXXX: This file contains temporary stubs that will eventually be replaced by real functions;
// this file will then be deleted.  (I'm just trying to get stuff working after ignoring Windoze 
// for a few months.

#include "QAudioPlugin.h"
#include "iaxclient.h"

#ifdef WIN32

// iaxclient functions declared in QAudioPlugin.h
int iaxc_initialize(int num_calls) { return 0; }
int iaxc_call(const char * num) { return 0; }
long iaxc_write_output_buffer(int index, void * data, long len) { return 0; }
int iaxc_select_call(int callNo) { return 0; }
int iaxc_selected_call() { return 0; }
void iaxc_dump_call(void) { }
int iaxc_register(const char * user, const char * pass, const char * host) { return 0; }
int iaxc_unregister( int id ) { return 0; }
int iaxc_start_processing_thread() { return 0; }
int iaxc_stop_processing_thread() { return 0; }
void iaxc_shutdown() { }
void iaxc_set_event_callback(iaxc_event_callback_t func) { }
int iaxc_quelch(int callNo, int MOH) { return 0; }
int iaxc_unquelch(int call) { return 0; }
void iaxc_send_busy_on_incoming_call(int callNo) { }
void iaxc_reject_call_number(int callNo) { }
void iaxc_answer_call(int callNo) { }

void iaxc_set_gsm_audio_callback(iaxc_gsm_audio_callback_t func) { }
int iaxc_get_output_buffer() { return 0; }

#endif //WIN32

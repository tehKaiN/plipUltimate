/*
 * uartutil.h - serial utility routines
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
 *
 * This file is part of plipbox.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#ifndef UARTUTIL_H
#define UARTUTIL_H

#include <avr/pgmspace.h>
#include "../global.h"

// send a c string from PROGMEM
void uart_send_pstring(PGM_P data);
// send a c string
void uart_send_string(const char *data);
// send data
void uart_send_data(uint8_t *data,uint8_t size);
// send a CR+LF
void uart_send_crlf(void);
// send a Space
void uart_send_spc(void);
// send time stamp
void uart_send_time_stamp_spc(void);
void uart_send_time_stamp_spc_ext(uint32_t ts);
// send rate in KB/s
void uart_send_rate_kbs(uint16_t kbs);
// send a delta in decimal
void uart_send_delta(uint32_t delta);

// send a hex byte
void uart_send_hex_byte(uint8_t data);
// send a hex word
void uart_send_hex_word(uint16_t data);
// send a hex dword
void uart_send_hex_dword(uint32_t data);

#ifdef DEBUG
// send free stack
void uart_send_free_stack(void);
#endif

#endif


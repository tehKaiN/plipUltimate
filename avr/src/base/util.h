/*
 * util.h - utilities
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

#ifndef UTIL_H
#define UTIL_H

#include <avr/io.h>

#include "../global.h"

// ----- conversion functions -----
// convert nybble to hex char
extern char utilNibbleToHex(uint8_t ubIn);
// convert uint8_t to 2 hex chars
extern void utilByteToHex(uint8_t ubIn, char *out);
// convert word to 4 hex chars
extern void utilWordToHex(uint16_t uwIn, char *out);
// convert dword to 6 hex chars
extern void utilDwordToHex(uint32_t ulIn, char *out);
// convert a byte to 3 dec chars
extern void utilByteToDec(uint8_t value, uint8_t *out);
// convert a dword to <num_digits> (up to 10) with fixed point at <point_pos>
// if <point_pos> < <num_digits> then no point is printed
extern void utilDwordToDec(uint32_t value, uint8_t *out, uint8_t num_digits, uint8_t point_pos);

// ----- parse functions: 01=ok, 00=error -----
// parse a nybble
extern uint8_t utilParseNibbleHex(char in,uint8_t *value);
// parse a byte
extern uint8_t utilParseByteHex(const char *str,uint8_t *value);
// parse a word
extern uint8_t utilParseWordHex(const char *str,uint16_t *value);
// parse a 6 byte dword
extern uint8_t utilParseDwordHex(const char *str,uint32_t *value);
// parse a decimal byte value
extern uint8_t utilParseByteDec(const char *buf, uint8_t *out);

extern void utilReset(void);

#ifdef DEBUG
// ---- stack free -----
extern void *__heap_start;
inline uint16_t stack_free(void) { return SP - (uint16_t) &__heap_start; }
#endif

#endif


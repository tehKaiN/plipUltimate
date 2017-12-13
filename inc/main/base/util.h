/*
 * This file is part of PlipUltimate.
 * License: GPLv2
 * Full license: https://github.com/tehKaiN/plipUltimate/blob/master/LICENSE
 * Authors list: https://github.com/tehKaiN/plipUltimate/blob/master/AUTHORS
 */

#ifndef UTIL_H
#define UTIL_H

#include <avr/io.h>

#include <main/global.h>

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

extern void *__heap_start;
/**
 * Checks how many free bytes are left on stack.
 * Checks distance between current Stack Pointer position and heap's
 * start address.
 */
inline uint16_t utilStackRemaining(void) { return SP - (uint16_t) &__heap_start; }

#endif


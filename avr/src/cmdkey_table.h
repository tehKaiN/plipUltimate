/*
 * This file is part of PlipUltimate.
 * Based on PlipBox by Christian Vogelgsang <chris@vogelgsang.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef CMDKEY_TABLE_H
#define CMDKEY_TABLE_H

#include <avr/pgmspace.h>
#include "global.h"

#define COMMAND_KEY(x) static void x (void)
#define CMDKEY_HELP(x,y) static const uint8_t x ## _help[] PROGMEM = y
#define CMDKEY_ENTRY(x,y) { x, y, y ## _help }

typedef void (*cmdkey_func_t)(void);

struct cmdkey_table_s {
  uint8_t     key;
  cmdkey_func_t func;
  const uint8_t * help;
};
typedef struct cmdkey_table_s cmdkey_table_t;

extern const cmdkey_table_t PROGMEM cmdkey_table[];

#endif

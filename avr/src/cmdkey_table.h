/*
 * This file is part of PlipUltimate.
 * License: GPLv2
 * Full license: https://github.com/tehKaiN/plipUltimate/blob/master/LICENSE
 * Authors list: https://github.com/tehKaiN/plipUltimate/blob/master/AUTHORS
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

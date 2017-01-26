/*
 * This file is part of PlipUltimate.
 * License: GPLv2
 * Full license: https://github.com/tehKaiN/plipUltimate/blob/master/LICENSE
 * Authors list: https://github.com/tehKaiN/plipUltimate/blob/master/AUTHORS
 */

#ifndef CMD_TABLE_H
#define CMD_TABLE_H

#include <avr/pgmspace.h>
#include "global.h"

#define CMD_OK            0x00
#define CMD_QUIT          0x01
#define CMD_RESET         0x02
#define CMD_OK_RESTART    0x03
#define CMD_WRONG_ARGC    0x10
#define CMD_PARSE_ERROR   0x11

#define CMD_MASK          0xf0
#define CMD_MASK_OK       0x00
#define CMD_MASK_SYNTAX   0x10
#define CMD_MASK_ERROR    0x20

#define COMMAND(x) static uint8_t x (uint8_t argc, const uint8_t **argv)
#define CMD_NAME(y,x,z) static const uint8_t x ## _name[] PROGMEM = y ; static const uint8_t x ## _help[] PROGMEM = z
#define CMD_ENTRY(x) { x ## _name, x, x ## _help }
#define CMD_ENTRY_NAME(x,y) { y ## _name, x, y ## _help }

typedef uint8_t (*cmd_table_func_t)(uint8_t argc, const uint8_t **argv);

struct cmd_table_s {
  const uint8_t * name;
  cmd_table_func_t func;
  const uint8_t * help;
};
typedef struct cmd_table_s cmd_table_t;

extern const cmd_table_t PROGMEM cmd_table[];

#endif

/*
 * util.c - utilities
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

#include "util.h"

/**
 * Converts nibble (0..15) value to hex char.
 */
char utilNibbleToHex(uint8_t ubIn)
{
  if(ubIn < 10)
    return '0' + ubIn;
  else
    return 'A' + ubIn - 10;
}

/**
 * Converts byte value to hex chars.
 */
void utilByteToHex(uint8_t ubIn, char *pOut)
{
  pOut[0] = utilNibbleToHex(ubIn >> 4);
  pOut[1] = utilNibbleToHex(ubIn & 0xf);
}

void utilWordToHex(uint16_t uwIn, char *pOut)
{
  utilByteToHex((uint8_t)(uwIn>>8), pOut);
  utilByteToHex((uint8_t)(uwIn&0xff), pOut+2);
}

void utilDwordToHex(uint32_t ulIn, char *pOut)
{
  utilWordToHex((uint16_t)(ulIn>>16), pOut);
  utilWordToHex((uint16_t)(ulIn&0xffff), pOut+4);
}

void utilByteToDec(uint8_t value, uint8_t *out)
{
  uint8_t h = value / 100;
  uint8_t t = value % 100;
  uint8_t o = t % 10;
  t = t / 10;
  out[0] = '0' + h;
  out[1] = '0' + t;
  out[2] = '0' + o;
}

void utilDwordToDec(uint32_t value, uint8_t *out, uint8_t num_digits, uint8_t point_pos)
{
	uint8_t i;
  // start backwards
  uint8_t *pos = out + num_digits - 1;
  if(point_pos < num_digits) {
    pos++;
  }
  for(i=0;i<num_digits;i++) {
    if(i == point_pos) {
      *pos = '.';
      pos--;
    }
    uint8_t dec = value % 10;
    *pos = '0' + dec;
    pos--;
    value /= 10;
  }
}

// parse

uint8_t utilParseNibbleHex(char c,uint8_t *value)
{
  if((c>='a')&&(c<='f')) {
    *value = c + 10 - 'a';
    return 1;
  }
  else if((c>='A')&&(c<='F')) {
    *value = c + 10 - 'A';
    return 1;
  }
  else if((c>='0')&&(c<='9')) {
    *value = c - '0';
    return 1;
  }
  else
    return 0;
}

uint8_t utilParseByteHex(const char *str,uint8_t *value)
{
  uint8_t val;
  if(!utilParseNibbleHex(str[0],&val))
    return 0;
  val <<= 4;
  if(!utilParseNibbleHex(str[1],value))
    return 0;
  *value |= val;
  return 1;
}

uint8_t utilParseWordHex(const char *str,uint16_t *value)
{
  uint8_t val;
  if(!utilParseByteHex(&str[0],&val))
    return 0;
  uint8_t val2;
  if(!utilParseByteHex(&str[2],&val2))
    return 0;
  *value = (uint16_t)val << 8 | val2;
  return 1;
}

uint8_t utilParseDwordHex(const char *str,uint32_t *value)
{
  uint8_t val;
  if(!utilParseByteHex(&str[0],&val))
    return 0;
  uint8_t val2;
  if(!utilParseByteHex(&str[2],&val2))
    return 0;
  uint8_t val3;
  if(!utilParseByteHex(&str[4],&val3))
    return 0;
  uint8_t val4;
  if(!utilParseByteHex(&str[6],&val4))
    return 0;
  *value = (uint32_t)val << 24 | (uint32_t)val2 << 16 | (uint32_t)val3 << 8 | val4;
  return 1;
}

uint8_t utilParseByteDec(const char *buf, uint8_t *out)
{
  uint8_t value = 0;
  uint8_t digits = 0;
  while(digits < 3) {
    uint8_t c = buf[digits];
    if((c<'0')||(c>'9')) {
      break;
    }
    c -= '0';
    value *= 10;
    value += c;
    digits++;
  }
  if(digits > 0) {
    *out = value;
  }
  return digits;
}

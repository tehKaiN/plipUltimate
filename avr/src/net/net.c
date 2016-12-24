/*
 * net.c - tool functions for network protocols
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

#include "net.h"
#include "../base/util.h"
#include "../base/uartutil.h"

const uint8_t net_bcast_mac[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
const uint8_t net_zero_mac[6] = { 0,0,0,0,0,0 };
const uint8_t net_zero_ip[4] = { 0,0,0,0 };
const uint8_t net_ones_ip[4] = { 255,255,255,255 };

void net_copy_mac(const uint8_t *in, uint8_t *out) {
	uint8_t i;
  for(i=6; i--;)
    out[i] = in[i];
}

void net_copy_ip(const uint8_t *in, uint8_t *out) {
	uint8_t i;
  for(i = 4; i--;)
		out[i] = in[i];
}

uint16_t  net_get_word(const uint8_t *buf) {
  return (uint16_t)buf[0] << 8 | (uint16_t)buf[1];
}

void net_put_word(uint8_t *buf, uint16_t value)
{
  buf[0] = (uint8_t)(value >> 8);
  buf[1] = (uint8_t)(value & 0xff);
}

uint32_t  net_get_long(const uint8_t *buf)
{
  return (uint32_t)buf[0] << 24 | (uint32_t)buf[1] << 16 | (uint32_t)buf[2] << 8 | (uint32_t)buf[3];
}

void net_put_long(uint8_t *buf, uint32_t value)
{
  buf[0] = (uint8_t)(value >> 24);
  buf[1] = (uint8_t)((value >> 16) & 0xff);
  buf[2] = (uint8_t)((value >> 8) & 0xff);
  buf[3] = (uint8_t)(value & 0xff);
}

static char *mac_str = "00:00:00:00:00:00";
static char *ip_str = "000.000.000.000";

void net_dump_mac(const uint8_t *in) {
  int pos = 0;
  uint8_t i;
  for(i=0;i<6;i++) {
    utilByteToHex(in[i], mac_str+pos);
    pos += 3;
  }
  // NOTE: UART - mac_str
}

uint8_t net_parse_ip(const char *buf, uint8_t *ip) {
	uint8_t i;
  for(i=0;i<4;i++) {
    uint8_t value;
    uint8_t digits = utilParseByteDec(buf,&value);
    if(digits == 0)
      return 0;
    buf += digits + 1;
    ip[i] = value;
  }
  return 1;
}

uint8_t net_parse_mac(const char *buf, uint8_t *mac) {
	uint8_t i;
  for(i=0;i<6;i++) {
    uint8_t value;
    if(!utilParseByteHex(buf, &value)) {
      return 0;
    }
    buf += 3;
    mac[i] = value;
  }
  return 1;
}

void net_dump_ip(const uint8_t *in) {
  uint8_t pos = 0;
  uint8_t i;
  for(i=0;i<4;i++) {
    utilByteToDec(in[i],(uint8_t *)(ip_str+pos));
    pos += 4;
  }
  // NOTE: UART - ip_str
}

uint8_t  net_compare_mac(const uint8_t *a, const uint8_t *b) {
	uint8_t i;
  for(i=0;i<6;i++)
    if(a[i] != b[i])
      return 0;
  return 1;
}

uint8_t  net_compare_ip(const uint8_t *a, const uint8_t *b) {
	uint8_t i;
  for(i=0;i<4;i++)
    if(a[i] != b[i])
      return 0;
  return 1;
}
